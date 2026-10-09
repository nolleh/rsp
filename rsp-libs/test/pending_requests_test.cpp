/** Copyright (C) 2026  nolleh (nolleh7707@gmail.com) **/
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <thread>

#include "rsplib/message/pending_requests.hpp"

namespace {
using requests = rsp::libs::message::pending_requests<int>;
using clock_type = requests::clock;
using namespace std::chrono_literals;

TEST(PendingRequests, TimerExpiresWithoutAnotherRequest) {
  requests pending(5ms);
  std::promise<void> timed_out;
  auto result = timed_out.get_future();
  std::atomic<int> responses{0};
  std::atomic<int> timeouts{0};
  pending.add(1, [&](const auto&) { ++responses; },
              [&] {
                if (++timeouts == 1) timed_out.set_value();
              },
              clock_type::now() + 20ms);
  pending.start();
  EXPECT_EQ(std::future_status::ready, result.wait_for(2s));
  pending.stop();
  EXPECT_EQ(0, responses.load());
  EXPECT_EQ(1, timeouts.load());
}

TEST(PendingRequests, ResponseBeforeDeadlineSuppressesTimeout) {
  requests pending;
  const auto now = clock_type::now();
  int responses = 0;
  int timeouts = 0;
  pending.add(1, [&](const auto&) { ++responses; }, [&] { ++timeouts; },
              now + 5s);
  pending.complete(1, std::make_shared<int>(42), now);
  pending.expire(now + 6s);
  pending.complete(1, std::make_shared<int>(42), now + 6s);
  EXPECT_EQ(1, responses);
  EXPECT_EQ(0, timeouts);
}

TEST(PendingRequests, LateResponseTriggersTimeoutBeforeNextTick) {
  requests pending;
  const auto deadline = clock_type::now();
  int responses = 0;
  int timeouts = 0;
  pending.add(1, [&](const auto&) { ++responses; }, [&] { ++timeouts; },
              deadline);
  pending.complete(1, std::make_shared<int>(42), deadline);
  pending.expire(deadline);
  EXPECT_EQ(0, responses);
  EXPECT_EQ(1, timeouts);
}

TEST(PendingRequests, CancelSuppressesBothCallbacks) {
  requests pending;
  const auto now = clock_type::now();
  int callbacks = 0;
  pending.add(1, [&](const auto&) { ++callbacks; }, [&] { ++callbacks; },
              now + 5s);
  EXPECT_TRUE(pending.cancel(1));
  EXPECT_FALSE(pending.cancel(1));
  pending.complete(1, std::make_shared<int>(42), now);
  pending.expire(now + 6s);
  EXPECT_EQ(0, callbacks);
}

TEST(PendingRequests, UsersExpireIndependentlyOfRequestIdOrder) {
  requests pending;
  const auto now = clock_type::now();
  int first_timeouts = 0;
  int second_timeouts = 0;
  int first_responses = 0;
  pending.add(1, [&](const auto&) { ++first_responses; },
              [&] { ++first_timeouts; }, now + 10s);
  pending.add(2, [](const auto&) {}, [&] { ++second_timeouts; }, now + 5s);
  pending.expire(now + 6s);
  pending.complete(1, std::make_shared<int>(42), now + 7s);
  EXPECT_EQ(0, first_timeouts);
  EXPECT_EQ(1, second_timeouts);
  EXPECT_EQ(1, first_responses);
}

TEST(PendingRequests, ResponseAndExpirationRaceCompletesExactlyOnce) {
  const auto response = std::make_shared<int>(42);
  for (int iteration = 0; iteration < 100; ++iteration) {
    requests pending;
    const auto deadline = clock_type::now();
    std::atomic<int> callbacks{0};
    pending.add(1, [&](const auto&) { ++callbacks; }, [&] { ++callbacks; },
                deadline);
    std::thread receiver([&] {
      pending.complete(1, response, deadline - 1ms);
    });
    std::thread timer([&] { pending.expire(deadline); });
    receiver.join();
    timer.join();
    EXPECT_EQ(1, callbacks.load());
  }
}

TEST(PendingRequests, TimeoutCallbackCanRegisterAnotherRequest) {
  requests pending;
  const auto now = clock_type::now();
  int timeouts = 0;
  pending.add(1, [](const auto&) {},
              [&] {
                ++timeouts;
                pending.add(2, [](const auto&) {}, [&] { ++timeouts; },
                            now + 1s);
              },
              now);
  pending.expire(now);
  EXPECT_EQ(1, timeouts);
  pending.expire(now + 1s);
  EXPECT_EQ(2, timeouts);
}

TEST(PendingRequests, StopClearsRequestsAndTimerCanRestart) {
  requests pending(5ms);
  int cancelled_callbacks = 0;
  std::weak_ptr<int> retained;
  {
    auto token = std::make_shared<int>(42);
    retained = token;
    pending.add(1, [token](const auto&) {},
                [token, &cancelled_callbacks] { ++cancelled_callbacks; },
                clock_type::now() + 10s);
  }
  pending.start();
  pending.stop();
  EXPECT_TRUE(retained.expired());
  pending.expire(clock_type::now() + 20s);
  EXPECT_EQ(0, cancelled_callbacks);

  std::promise<void> timed_out;
  auto result = timed_out.get_future();
  pending.add(2, [](const auto&) {}, [&] { timed_out.set_value(); },
              clock_type::now() + 20ms);
  pending.start();
  EXPECT_EQ(std::future_status::ready, result.wait_for(2s));
  pending.stop();
}
}  // namespace
