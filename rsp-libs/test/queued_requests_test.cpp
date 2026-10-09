/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/
#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include "rsplib/message/queued_requests.hpp"

namespace {
using registry = rsp::libs::message::queued_requests;
using namespace std::chrono_literals;

TEST(QueuedRequests, CancelBeforeStartSkipsWork) {
  registry requests;
  const registry::key key{"user-a", 1};
  auto ticket = requests.admit(key);
  ASSERT_TRUE(ticket);
  requests.cancel(key);
  EXPECT_FALSE(ticket->try_start());
  EXPECT_FALSE(ticket->started());
  EXPECT_TRUE(requests.finish(key, ticket));
  EXPECT_FALSE(requests.admit(key));
}

TEST(QueuedRequests, StartedWorkIgnoresCancellation) {
  registry requests;
  const registry::key key{"user-a", 1};
  auto ticket = requests.admit(key);
  ASSERT_TRUE(ticket);
  ASSERT_TRUE(ticket->try_start());
  requests.cancel(key);
  EXPECT_TRUE(ticket->started());
  EXPECT_TRUE(requests.finish(key, ticket));
  requests.cancel(key);
  EXPECT_FALSE(requests.admit(key));
}

TEST(QueuedRequests, UnknownCancellationExpiresWithoutExtendingOnDuplicates) {
  registry requests(4, 30s);
  const auto now = registry::clock::now();
  const registry::key key{"user-a", 1};
  requests.cancel(key, now);
  requests.cancel(key, now + 20s);
  EXPECT_FALSE(requests.admit(key, now + 29s));
  EXPECT_TRUE(requests.admit(key, now + 30s));
}

TEST(QueuedRequests, CancellationIsScopedByRouteAndRequestId) {
  registry requests;
  const registry::key a{"user-a", 1};
  const registry::key b{"user-b", 1};
  auto first = requests.admit(a);
  auto second = requests.admit(b);
  ASSERT_TRUE(first);
  ASSERT_TRUE(second);
  requests.cancel(a);
  EXPECT_FALSE(first->try_start());
  EXPECT_TRUE(second->try_start());
}

TEST(QueuedRequests, DuplicateQueuedRequestIsNotScheduledTwice) {
  registry requests;
  auto ticket = requests.admit({"user-a", 1});
  ASSERT_TRUE(ticket);
  EXPECT_FALSE(requests.admit({"user-a", 1}));
  EXPECT_TRUE(ticket->try_start());
}

TEST(QueuedRequests, CancelledActiveTicketDoesNotExpireWhileStillQueued) {
  registry requests(4, 30s);
  const auto now = registry::clock::now();
  const registry::key key{"user-a", 1};
  auto ticket = requests.admit(key, now);
  ASSERT_TRUE(ticket);
  requests.cancel(key, now + 60s);
  EXPECT_FALSE(ticket->try_start());
  EXPECT_FALSE(requests.admit(key, now + 120s));
  EXPECT_TRUE(requests.finish(key, ticket, now + 120s));
}

TEST(QueuedRequests, ActiveCapacityDoesNotEvictQueuedWork) {
  registry requests(1);
  auto ticket = requests.admit({"user-a", 1});
  ASSERT_TRUE(ticket);
  EXPECT_FALSE(requests.admit({"user-b", 1}));
  requests.cancel({"user-a", 1});
  EXPECT_FALSE(requests.admit({"user-b", 1}));
  EXPECT_TRUE(requests.finish({"user-a", 1}, ticket));
  EXPECT_TRUE(requests.admit({"user-b", 1}));
}

TEST(QueuedRequests, TerminalCapacityEvictsOldestRecord) {
  registry requests(2, 30s);
  const auto now = registry::clock::now();
  requests.cancel({"user", 1}, now);
  requests.cancel({"user", 2}, now + 1s);
  requests.cancel({"user", 3}, now + 2s);
  EXPECT_TRUE(requests.admit({"user", 1}, now + 3s));
  EXPECT_FALSE(requests.admit({"user", 2}, now + 3s));
  EXPECT_FALSE(requests.admit({"user", 3}, now + 3s));
}

TEST(QueuedRequests, StartAndCancelRaceHasOneWinner) {
  for (int iteration = 0; iteration < 100; ++iteration) {
    registry requests;
    const registry::key key{"user", 1};
    auto ticket = requests.admit(key);
    ASSERT_TRUE(ticket);
    bool started = false;
    std::thread worker([&] { started = ticket->try_start(); });
    std::thread canceller([&] { requests.cancel(key); });
    worker.join();
    canceller.join();
    EXPECT_EQ(started, ticket->started());
    EXPECT_TRUE(requests.finish(key, ticket));
    EXPECT_FALSE(requests.admit(key));
  }
}

TEST(QueuedRequests, OldCompletionCannotRemoveNewTicketAfterRetention) {
  registry requests(4, 30s);
  const auto now = registry::clock::now();
  const registry::key key{"user", 1};
  auto first = requests.admit(key, now);
  ASSERT_TRUE(first);
  ASSERT_TRUE(first->try_start());
  ASSERT_TRUE(requests.finish(key, first, now));
  auto second = requests.admit(key, now + 31s);
  ASSERT_TRUE(second);
  EXPECT_FALSE(requests.finish(key, first, now + 31s));
  EXPECT_FALSE(requests.admit(key, now + 31s));
  EXPECT_TRUE(second->try_start());
}
}  // namespace
