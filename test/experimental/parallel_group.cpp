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
#include <vector>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
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

struct record_strand_condition
{
  boost::asio::strand<boost::asio::io_context::executor_type>* strand_;
  int* on_strand_;
  int* off_strand_;

  boost::asio::cancellation_type_t operator()(const boost::system::error_code&) const
  {
    ++*(strand_->running_in_this_thread() ? on_strand_ : off_strand_);
    return boost::asio::cancellation_type::none;
  }
};

void variadic_group_io_executor_test()
{
  boost::asio::io_context ioc;
  boost::asio::strand<boost::asio::io_context::executor_type> s =
    boost::asio::make_strand(ioc);

  boost::asio::steady_timer t0(s, boost::asio::steady_timer::clock_type::now());
  boost::asio::steady_timer t1(s, boost::asio::steady_timer::clock_type::now());

  int on_strand = 0;
  int off_strand = 0;
  int called = 0;

  boost::asio::experimental::make_parallel_group(
      t0.async_wait(boost::asio::deferred),
      t1.async_wait(boost::asio::deferred)
    ).async_wait(
        record_strand_condition{&s, &on_strand, &off_strand},
        [&](std::array<std::size_t, 2>, boost::system::error_code, boost::system::error_code)
        {
          ++called;
        }
      );

  ioc.run();

  // Operations with no associated executor must complete on their I/O
  // object's executor.
  BOOST_ASIO_CHECK(called == 1);
  BOOST_ASIO_CHECK(on_strand == 2);
  BOOST_ASIO_CHECK(off_strand == 0);
}

void ranged_group_io_executor_test()
{
  boost::asio::io_context ioc;
  boost::asio::strand<boost::asio::io_context::executor_type> s =
    boost::asio::make_strand(ioc);

  boost::asio::steady_timer t0(s, boost::asio::steady_timer::clock_type::now());
  boost::asio::steady_timer t1(s, boost::asio::steady_timer::clock_type::now());

  typedef decltype(t0.async_wait(boost::asio::deferred)) op_type;
  std::vector<op_type> ops;
  ops.push_back(t0.async_wait(boost::asio::deferred));
  ops.push_back(t1.async_wait(boost::asio::deferred));

  int on_strand = 0;
  int off_strand = 0;
  int called = 0;

  boost::asio::experimental::make_parallel_group(std::move(ops)).async_wait(
      record_strand_condition{&s, &on_strand, &off_strand},
      [&](std::vector<std::size_t>, std::vector<boost::system::error_code>)
      {
        ++called;
      });

  ioc.run();

  // Operations with no associated executor must complete on their I/O
  // object's executor.
  BOOST_ASIO_CHECK(called == 1);
  BOOST_ASIO_CHECK(on_strand == 2);
  BOOST_ASIO_CHECK(off_strand == 0);
}

BOOST_ASIO_TEST_SUITE
(
  "experimental/parallel_group",
  BOOST_ASIO_TEST_CASE(non_terminal_group_cancellation_test)
  BOOST_ASIO_TEST_CASE(variadic_group_io_executor_test)
  BOOST_ASIO_TEST_CASE(ranged_group_io_executor_test)
)
