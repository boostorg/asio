//
// promise.cpp
// ~~~~~~~~~~~
//
// Copyright (c) 2021-2022 Klemens D. Morgenstern
//                         (klemens dot morgenstern at gmx dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <boost/asio/experimental/promise.hpp>

#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/experimental/use_promise.hpp>
#include <boost/asio/steady_timer.hpp>
#include "../unit_test.hpp"

namespace promise {

void promise_tester()
{
  using namespace boost::asio;
  using namespace std::chrono;

  io_context ctx;

  steady_timer timer1{ctx}, timer2{ctx};

  const auto started_when = steady_clock::now();
  timer1.expires_at(started_when + milliseconds(5000));
  timer2.expires_at(started_when + milliseconds(1000));
  auto p = timer1.async_wait(experimental::use_promise);

  steady_clock::time_point completed_when;
  boost::system::error_code ec;
  bool called = false;

  p([&](boost::system::error_code ec_)
      {
        ec = ec_;
        called = true;
        completed_when = steady_clock::now();
      });

  steady_clock::time_point timer2_done;
  timer2.async_wait(
      [&](boost::system::error_code)
      {
        timer2_done = steady_clock::now();
        p.cancel();
      });

  ctx.run();

  static_assert(
      boost::asio::is_async_operation<decltype(p)>::value,
      "promise is async_op");

  BOOST_ASIO_CHECK(timer2_done + milliseconds(1) > started_when);
  BOOST_ASIO_CHECK(completed_when > timer2_done);
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(ec == error::operation_aborted);
}

void promise_slot_tester()
{
  using namespace boost::asio;
  using namespace std::chrono;

  io_context ctx;

  steady_timer timer1{ctx}, timer2{ctx};

  const auto started_when = steady_clock::now();
  timer1.expires_at(started_when + milliseconds(2500));
  timer2.expires_at(started_when + milliseconds(1000));
  auto p = timer1.async_wait(experimental::use_promise);

  steady_clock::time_point completed_when;
  boost::system::error_code ec;
  bool called = false;

  boost::asio::cancellation_signal sig;

  p(boost::asio::bind_cancellation_slot(
        sig.slot(),
        [&](boost::system::error_code ec_)
        {
          ec = ec_;
          called = true;
          completed_when = steady_clock::now();
        }));

  steady_clock::time_point timer2_done;
  timer2.async_wait(
      [&](boost::system::error_code)
      {
        timer2_done = steady_clock::now();
        sig.emit(boost::asio::cancellation_type::all);
      });

  ctx.run();

  static_assert(
      boost::asio::is_async_operation<decltype(p)>::value,
      "promise is async_op");

  BOOST_ASIO_CHECK(timer2_done + milliseconds(1) > started_when);
  BOOST_ASIO_CHECK(completed_when > timer2_done);
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(ec == error::operation_aborted);
}

void early_completion()
{
  using namespace boost::asio;
  using namespace std::chrono;

  io_context ctx;
  auto p = boost::asio::post(ctx, boost::asio::experimental::use_promise);
  ctx.run();

  BOOST_ASIO_CHECK(p.completed());

  bool completed = false;
  p([&]{completed = true;});
  BOOST_ASIO_CHECK(!completed);
  ctx.restart();
  ctx.run();
  BOOST_ASIO_CHECK(completed);
}

struct test_cancel_impl_op
{
  boost::asio::steady_timer & tim;
  boost::system::error_code &ec;
  template<typename Self>
  void operator()(Self& self)
  {
    tim.async_wait(std::forward<Self>(self));
  }

  template<typename Self>
  void operator()(Self& self, boost::system::error_code ec_)
  {
    ec = ec_;
    self.complete(ec_);
  }
};

template <typename CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(
    CompletionToken, void(boost::system::error_code))
test_cancel_impl(boost::asio::steady_timer & tim,
                 boost::system::error_code &ec,
                 CompletionToken&& token)
{
  return boost::asio::async_compose<CompletionToken, void(boost::system::error_code)>(
      test_cancel_impl_op{tim, ec}, token, tim);
}

void test_cancel()
{
  boost::asio::io_context ctx;
  boost::asio::steady_timer tim{ctx, std::chrono::seconds(10)};
  boost::system::error_code ec;

  {
    auto p = test_cancel_impl(
        tim, ec, boost::asio::experimental::use_promise);
  }

  ctx.run();

  BOOST_ASIO_CHECK_MESSAGE(
      ec == boost::asio::error::operation_aborted,
      ec.message());
}

} // namespace promise

BOOST_ASIO_TEST_SUITE
(
  "promise",
  BOOST_ASIO_TEST_CASE(promise::promise_tester)
  BOOST_ASIO_TEST_CASE(promise::promise_slot_tester)
  BOOST_ASIO_TEST_CASE(promise::early_completion)
  BOOST_ASIO_TEST_CASE(promise::test_cancel)
)
