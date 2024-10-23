//
// strand.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <boost/asio/strand.hpp>

#include <functional>
#include <sstream>
#include <boost/asio/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/detail/thread.hpp>
#include <boost/asio/steady_timer.hpp>
#include "unit_test.hpp"

using namespace boost::asio;
namespace bindns = std;

typedef steady_timer timer;
namespace chronons = boost::asio::chrono;

void increment(int* count)
{
  ++(*count);
}

void increment_without_lock(strand<io_context::executor_type>* s, int* count)
{
  BOOST_ASIO_CHECK(!s->running_in_this_thread());

  int original_count = *count;

  dispatch(*s, bindns::bind(increment, count));

  // No other functions are currently executing through the locking dispatcher,
  // so the previous call to dispatch should have successfully nested.
  BOOST_ASIO_CHECK(*count == original_count + 1);
}

void increment_with_lock(strand<io_context::executor_type>* s, int* count)
{
  BOOST_ASIO_CHECK(s->running_in_this_thread());

  int original_count = *count;

  dispatch(*s, bindns::bind(increment, count));

  // The current function already holds the strand's lock, so the
  // previous call to dispatch should have successfully nested.
  BOOST_ASIO_CHECK(*count == original_count + 1);
}

void sleep_increment(io_context* ioc, int* count)
{
  timer t(*ioc, chronons::seconds(2));
  t.wait();

  ++(*count);
}

void increment_by_a(int* count, int a)
{
  (*count) += a;
}

void increment_by_a_b(int* count, int a, int b)
{
  (*count) += a + b;
}

void increment_by_a_b_c(int* count, int a, int b, int c)
{
  (*count) += a + b + c;
}

void increment_by_a_b_c_d(int* count, int a, int b, int c, int d)
{
  (*count) += a + b + c + d;
}

void start_sleep_increments(io_context* ioc,
    strand<io_context::executor_type>* s, int* count)
{
  // Give all threads a chance to start.
  timer t(*ioc, chronons::seconds(2));
  t.wait();

  // Start three increments.
  post(*s, bindns::bind(sleep_increment, ioc, count));
  post(*s, bindns::bind(sleep_increment, ioc, count));
  post(*s, bindns::bind(sleep_increment, ioc, count));
}

void throw_exception()
{
  throw 1;
}

void io_context_run(io_context* ioc)
{
  ioc->run();
}

void strand_test()
{
  io_context ioc;
  strand<io_context::executor_type> s = make_strand(ioc);
  int count = 0;

  post(ioc, bindns::bind(increment_without_lock, &s, &count));

  // No handlers can be called until run() is called.
  BOOST_ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  BOOST_ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  post(s, bindns::bind(increment_with_lock, &s, &count));

  // No handlers can be called until run() is called.
  BOOST_ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  BOOST_ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  post(ioc, bindns::bind(start_sleep_increments, &ioc, &s, &count));
  boost::asio::detail::thread thread1(bindns::bind(io_context_run, &ioc));
  boost::asio::detail::thread thread2(bindns::bind(io_context_run, &ioc));

  // Check all events run one after another even though there are two threads.
  timer timer1(ioc, chronons::seconds(3));
  timer1.wait();
  BOOST_ASIO_CHECK(count == 0);
  timer1.expires_at(timer1.expiry() + chronons::seconds(2));
  timer1.wait();
  BOOST_ASIO_CHECK(count == 1);
  timer1.expires_at(timer1.expiry() + chronons::seconds(2));
  timer1.wait();
  BOOST_ASIO_CHECK(count == 2);

  thread1.join();
  thread2.join();

  // The run() calls will not return until all work has finished.
  BOOST_ASIO_CHECK(count == 3);

  count = 0;
  int exception_count = 0;
  ioc.restart();
  post(s, throw_exception);
  post(s, bindns::bind(increment, &count));
  post(s, bindns::bind(increment, &count));
  post(s, throw_exception);
  post(s, bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  BOOST_ASIO_CHECK(count == 0);
  BOOST_ASIO_CHECK(exception_count == 0);

  for (;;)
  {
    try
    {
      ioc.run();
      break;
    }
    catch (int)
    {
      ++exception_count;
    }
  }

  // The run() calls will not return until all work has finished.
  BOOST_ASIO_CHECK(count == 3);
  BOOST_ASIO_CHECK(exception_count == 2);

  count = 0;
  ioc.restart();

  // Check for clean shutdown when handlers posted through an orphaned strand
  // are abandoned.
  {
    strand<io_context::executor_type> s2 = make_strand(ioc.get_executor());
    post(s2, bindns::bind(increment, &count));
    post(s2, bindns::bind(increment, &count));
    post(s2, bindns::bind(increment, &count));
  }

  // No handlers can be called until run() is called.
  BOOST_ASIO_CHECK(count == 0);
}

void strand_conversion_test()
{
  io_context ioc;
  strand<io_context::executor_type> s1 = make_strand(ioc);

  // Converting constructors.

  strand<executor> s2(s1);
  strand<executor> s3 = strand<io_context::executor_type>(s1);

  // Converting assignment.

  s3 = s1;
  s3 = strand<io_context::executor_type>(s1);
}

void strand_query_test()
{
  io_context ioc;
  strand<io_context::executor_type> s1 = make_strand(ioc);

  BOOST_ASIO_CHECK(
      &boost::asio::query(s1, boost::asio::execution::context)
        == &ioc);

  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::blocking)
        == boost::asio::execution::blocking.possibly);

  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::blocking.possibly)
        == boost::asio::execution::blocking.possibly);

  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::outstanding_work)
        == boost::asio::execution::outstanding_work.untracked);

  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::outstanding_work.untracked)
        == boost::asio::execution::outstanding_work.untracked);

  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::relationship)
        == boost::asio::execution::relationship.fork);

  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::relationship.fork)
        == boost::asio::execution::relationship.fork);

  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::mapping)
        == boost::asio::execution::mapping.thread);

  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::allocator)
        == std::allocator<void>());
}

void strand_execute_test()
{
  io_context ioc;
  strand<io_context::executor_type> s1 = make_strand(ioc);
  int count = 0;

  s1.execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  BOOST_ASIO_CHECK(!ioc.stopped());
  BOOST_ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  boost::asio::require(s1, boost::asio::execution::blocking.possibly).execute(
      bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  BOOST_ASIO_CHECK(!ioc.stopped());
  BOOST_ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  boost::asio::require(s1, boost::asio::execution::blocking.never).execute(
      bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  BOOST_ASIO_CHECK(!ioc.stopped());
  BOOST_ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  BOOST_ASIO_CHECK(!ioc.stopped());

  boost::asio::require(s1,
      boost::asio::execution::blocking.never,
      boost::asio::execution::outstanding_work.tracked
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  BOOST_ASIO_CHECK(!ioc.stopped());
  BOOST_ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  boost::asio::require(s1,
      boost::asio::execution::blocking.never,
      boost::asio::execution::outstanding_work.untracked
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  BOOST_ASIO_CHECK(!ioc.stopped());
  BOOST_ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  boost::asio::require(s1,
      boost::asio::execution::blocking.never,
      boost::asio::execution::outstanding_work.untracked,
      boost::asio::execution::relationship.fork
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  BOOST_ASIO_CHECK(!ioc.stopped());
  BOOST_ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  boost::asio::require(s1,
      boost::asio::execution::blocking.never,
      boost::asio::execution::outstanding_work.untracked,
      boost::asio::execution::relationship.continuation
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  BOOST_ASIO_CHECK(!ioc.stopped());
  BOOST_ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  boost::asio::prefer(
      boost::asio::require(s1,
        boost::asio::execution::blocking.never,
        boost::asio::execution::outstanding_work.untracked,
        boost::asio::execution::relationship.continuation),
      boost::asio::execution::allocator(std::allocator<void>())
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  BOOST_ASIO_CHECK(!ioc.stopped());
  BOOST_ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  boost::asio::prefer(
      boost::asio::require(s1,
        boost::asio::execution::blocking.never,
        boost::asio::execution::outstanding_work.untracked,
        boost::asio::execution::relationship.continuation),
      boost::asio::execution::allocator
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  BOOST_ASIO_CHECK(!ioc.stopped());
  BOOST_ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(count == 1);
}

BOOST_ASIO_TEST_SUITE
(
  "strand",
  BOOST_ASIO_TEST_CASE(strand_test)
  BOOST_ASIO_COMPILE_TEST_CASE(strand_conversion_test)
  BOOST_ASIO_TEST_CASE(strand_query_test)
  BOOST_ASIO_TEST_CASE(strand_execute_test)
)
