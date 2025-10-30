//
// inline_or_executor.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <boost/asio/inline_or_executor.hpp>

#include <functional>
#include <sstream>
#include <boost/asio/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/post.hpp>
#include "unit_test.hpp"

using namespace boost::asio;
namespace bindns = std;

void increment(int* count)
{
  ++(*count);
}

void inline_or_executor_conversion_test()
{
  io_context ioc;
  inline_or_executor<io_context::executor_type> s1 = inline_or(ioc);

  // Converting constructors.

  inline_or_executor<executor> s2(s1);
  inline_or_executor<executor> s3 =
    inline_or_executor<io_context::executor_type>(s1);

  // Converting assignment.

  s3 = s1;
  s3 = inline_or_executor<io_context::executor_type>(s1);
}

void inline_or_executor_query_test()
{
  io_context ioc;
  inline_or_executor<io_context::executor_type> s1 = inline_or(ioc);

  BOOST_ASIO_CHECK(
      &boost::asio::query(s1, boost::asio::execution::context)
        == &ioc);

  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::blocking)
        == boost::asio::execution::blocking.always);

  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::blocking.always)
        == boost::asio::execution::blocking.always);

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

void inline_or_executor_execute_test()
{
  io_context ioc;
  inline_or_executor<io_context::executor_type> s1 = inline_or(ioc);
  int count = 0;

  s1.execute(bindns::bind(increment, &count));

  // Handler is run inline.
  BOOST_ASIO_CHECK(count == 1);

  count = 0;
  boost::asio::require(s1, boost::asio::execution::blocking.possibly).execute(
      bindns::bind(increment, &count));

  // Handler is run inline.
  BOOST_ASIO_CHECK(count == 1);

  count = 0;
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
  "inline_or_executor",
  BOOST_ASIO_COMPILE_TEST_CASE(inline_or_executor_conversion_test)
  BOOST_ASIO_TEST_CASE(inline_or_executor_query_test)
  BOOST_ASIO_TEST_CASE(inline_or_executor_execute_test)
)
