/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "rsplib/job/job_scheduler.hpp"

namespace {

class manual_job : public rsp::libs::job::job {
 public:
  manual_job(std::string name, std::vector<std::string>* events,
             bool expires = false)
      : name_(std::move(name)), events_(events), expires_(expires) {}

  void run(completion done) override {
    events_->push_back("run " + name_);
    done_ = std::move(done);
  }

  bool expired(std::chrono::steady_clock::time_point) const override {
    return expires_;
  }

  void cancel(cancel_reason reason) override {
    events_->push_back("cancel " + name_ +
                       (reason == cancel_reason::kTimeout ? " timeout"
                                                          : " shutdown"));
    if (done_) done_();
  }

  void complete() { done_(); }

 private:
  std::string name_;
  std::vector<std::string>* events_;
  bool expires_;
  completion done_;
};

TEST(JobScheduler, WaitsForAsyncJobCompletion) {
  rsp::libs::job::job_scheduler scheduler;
  std::vector<std::string> events;
  auto first = std::make_shared<manual_job>("first", &events);
  auto second = std::make_shared<manual_job>("second", &events);

  scheduler.push_and_run(first);
  scheduler.push_and_run(second);

  EXPECT_EQ((std::vector<std::string>{"run first"}), events);

  first->complete();

  EXPECT_EQ((std::vector<std::string>{"run first", "run second"}), events);
}

TEST(JobScheduler, ExternalTimeoutCompletesCurrentJobBeforeNextJob) {
  rsp::libs::job::job_scheduler scheduler;
  std::vector<std::string> events;
  auto first = std::make_shared<manual_job>("first", &events, true);
  auto second = std::make_shared<manual_job>("second", &events);

  scheduler.push_and_run(first);
  scheduler.push_and_run(second);
  EXPECT_EQ((std::vector<std::string>{"run first"}), events);

  first->cancel(rsp::libs::job::job::cancel_reason::kTimeout);

  EXPECT_EQ((std::vector<std::string>{"run first", "cancel first timeout",
                                       "run second"}),
            events);
}

TEST(JobScheduler, ShutdownCancelsActiveAndQueuedJobsWithoutRunningNext) {
  rsp::libs::job::job_scheduler scheduler;
  std::vector<std::string> events;
  auto first = std::make_shared<manual_job>("first", &events);
  auto second = std::make_shared<manual_job>("second", &events);

  scheduler.push_and_run(first);
  scheduler.push_and_run(second);
  scheduler.shutdown();

  EXPECT_EQ((std::vector<std::string>{"run first", "cancel first shutdown",
                                      "cancel second shutdown"}),
            events);
}

TEST(JobScheduler, RejectsJobsAfterShutdown) {
  rsp::libs::job::job_scheduler scheduler;
  std::vector<std::string> events;
  auto rejected = std::make_shared<manual_job>("rejected", &events);

  scheduler.shutdown();
  scheduler.push_and_run(rejected);

  EXPECT_EQ((std::vector<std::string>{"cancel rejected shutdown"}), events);
}

}  // namespace
