//
// inline_executor.cpp
// ~~~~~~~~~~~~~~~~~~~
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

// Prevent link dependency on the Boost.System library.
#if !defined(BOOST_SYSTEM_NO_DEPRECATED)
#define BOOST_SYSTEM_NO_DEPRECATED
#endif // !defined(BOOST_SYSTEM_NO_DEPRECATED)

// Test that header file is self-contained.
#include <boost/asio/inline_executor.hpp>

#include <functional>
#include <boost/asio/any_completion_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include "unit_test.hpp"

using namespace boost::asio;

namespace bindns = std;

void increment(boost::asio::detail::atomic_count* count)
{
  ++(*count);
}

void inline_executor_query_test()
{
  BOOST_ASIO_CHECK(
      boost::asio::query(inline_executor(),
        boost::asio::execution::blocking)
      == boost::asio::execution::blocking.always);

  BOOST_ASIO_CHECK(
      boost::asio::query(inline_executor(),
        boost::asio::execution::blocking.possibly)
      == boost::asio::execution::blocking.always);

  BOOST_ASIO_CHECK(
      boost::asio::query(inline_executor(),
        boost::asio::execution::outstanding_work)
      == boost::asio::execution::outstanding_work.untracked);

  BOOST_ASIO_CHECK(
      boost::asio::query(inline_executor(),
        boost::asio::execution::outstanding_work.untracked)
      == boost::asio::execution::outstanding_work.untracked);

  BOOST_ASIO_CHECK(
      boost::asio::query(inline_executor(),
        boost::asio::execution::relationship)
      == boost::asio::execution::relationship.fork);

  BOOST_ASIO_CHECK(
      boost::asio::query(inline_executor(),
        boost::asio::execution::relationship.fork)
      == boost::asio::execution::relationship.fork);

  BOOST_ASIO_CHECK(
      boost::asio::query(inline_executor(),
        boost::asio::execution::mapping)
      == boost::asio::execution::mapping.thread);

  BOOST_ASIO_CHECK(
      boost::asio::query(inline_executor(),
        boost::asio::execution::inline_exception_handling)
      == boost::asio::execution::inline_exception_handling.propagate);

  BOOST_ASIO_CHECK(
      boost::asio::query(inline_executor(),
        boost::asio::execution::inline_exception_handling.propagate)
      == boost::asio::execution::inline_exception_handling.propagate);

  BOOST_ASIO_CHECK(
      boost::asio::query(
        boost::asio::require(inline_executor(),
          boost::asio::execution::inline_exception_handling.terminate),
        boost::asio::execution::inline_exception_handling)
      == boost::asio::execution::inline_exception_handling.terminate);

  BOOST_ASIO_CHECK(
      boost::asio::query(
        boost::asio::require(inline_executor(),
          boost::asio::execution::inline_exception_handling.terminate),
        boost::asio::execution::inline_exception_handling.propagate)
      == boost::asio::execution::inline_exception_handling.terminate);
}

void inline_executor_execute_test()
{
  boost::asio::detail::atomic_count count(0);

  inline_executor().execute(bindns::bind(increment, &count));

  boost::asio::require(inline_executor(),
      boost::asio::execution::blocking.always
    ).execute(bindns::bind(increment, &count));

  boost::asio::prefer(inline_executor(),
      boost::asio::execution::blocking.possibly
    ).execute(bindns::bind(increment, &count));

  boost::asio::any_completion_executor ex = inline_executor();

  ex.execute(bindns::bind(increment, &count));

  BOOST_ASIO_CHECK(count == 4);
}

void inline_executor_dispatch_test()
{
  boost::asio::detail::atomic_count count(0);

  boost::asio::dispatch(inline_executor(),
      bindns::bind(increment, &count));

  boost::asio::dispatch(
      boost::asio::require(inline_executor(),
        boost::asio::execution::inline_exception_handling.terminate),
      bindns::bind(increment, &count));

  BOOST_ASIO_CHECK(count == 2);
}

void throw_exception()
{
  throw 42;
}

void inline_executor_exception_test()
{
#if !defined(BOOST_ASIO_NO_EXCEPTIONS)
  bool caught = false;

  try
  {
    inline_executor().execute(throw_exception);
  }
  catch (...)
  {
    caught = true;
  }

  BOOST_ASIO_CHECK(caught);
#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)
}

BOOST_ASIO_TEST_SUITE
(
  "inline_executor",
  BOOST_ASIO_TEST_CASE(inline_executor_query_test)
  BOOST_ASIO_TEST_CASE(inline_executor_execute_test)
  BOOST_ASIO_TEST_CASE(inline_executor_dispatch_test)
  BOOST_ASIO_TEST_CASE(inline_executor_exception_test)
)
