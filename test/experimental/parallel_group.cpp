//
// experimental/parallel_group.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <boost/asio/experimental/parallel_group.hpp>

#include <array>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include "../unit_test.hpp"

struct wait_for_cancel_filter
{
  boost::asio::steady_timer* timer_;
  boost::asio::cancellation_type_t react_to_;

  void operator()(boost::asio::cancellation_type_t type)
  {
    if (!!(type & react_to_))
      timer_->cancel();
  }
};

struct wait_for_cancel_initiation
{
  boost::asio::cancellation_type_t react_to_;

  template <typename Handler>
  void operator()(Handler&& handler, boost::asio::steady_timer* timer) const
  {
    auto slot = boost::asio::get_associated_cancellation_slot(handler);
    timer->expires_after(boost::asio::chrono::hours(1));
    if (slot.is_connected())
      slot.assign(wait_for_cancel_filter{timer, react_to_});

    timer->async_wait(
        boost::asio::bind_cancellation_slot(boost::asio::cancellation_slot(),
          static_cast<Handler&&>(handler)));
  }
};

template <typename CompletionToken = boost::asio::deferred_t>
auto async_wait_for_cancel(boost::asio::steady_timer& timer,
    boost::asio::cancellation_type_t react_to,
    CompletionToken&& token = boost::asio::deferred_t())
  -> decltype(
      boost::asio::async_initiate<CompletionToken, void(boost::system::error_code)>(
        wait_for_cancel_initiation{react_to}, token, &timer))
{
  return boost::asio::async_initiate<CompletionToken, void(boost::system::error_code)>(
      wait_for_cancel_initiation{react_to}, token, &timer);
}

void non_terminal_group_cancellation_test()
{
  boost::asio::io_context ioc;
  boost::asio::steady_timer t0(ioc);
  boost::asio::steady_timer t1(ioc);
  boost::asio::cancellation_signal cancel_signal;

  int called = 0;
  std::array<std::size_t, 2> order = {{}};
  boost::system::error_code ec0 = boost::asio::error::would_block;
  boost::system::error_code ec1 = boost::asio::error::would_block;

  boost::asio::experimental::make_parallel_group(
      async_wait_for_cancel(t0, boost::asio::cancellation_type::partial),
      async_wait_for_cancel(t1, boost::asio::cancellation_type::terminal)
    ).async_wait(
        boost::asio::experimental::wait_for_one(),
        boost::asio::bind_cancellation_slot(
          cancel_signal.slot(),
          [&](std::array<std::size_t, 2> o,
            boost::system::error_code e0, boost::system::error_code e1)
          {
            ++called;
            order = o;
            ec0 = e0;
            ec1 = e1;
          }
        )
      );

  cancel_signal.emit(boost::asio::cancellation_type::partial);

  ioc.run_for(boost::asio::chrono::seconds(5));

  BOOST_ASIO_CHECK(called == 1);
  BOOST_ASIO_CHECK(order[0] == 0);
  BOOST_ASIO_CHECK(order[1] == 1);
  BOOST_ASIO_CHECK(ec0 == boost::asio::error::operation_aborted);
  BOOST_ASIO_CHECK(ec1 == boost::asio::error::operation_aborted);
}

BOOST_ASIO_TEST_SUITE
(
  "experimental/parallel_group",
  BOOST_ASIO_TEST_CASE(non_terminal_group_cancellation_test)
)
