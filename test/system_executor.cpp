//
// system_executor.cpp
// ~~~~~~~~~~~~~~~~~~~
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

// Prevent link dependency on the Boost.System library.
#if !defined(BOOST_SYSTEM_NO_DEPRECATED)
#define BOOST_SYSTEM_NO_DEPRECATED
#endif // !defined(BOOST_SYSTEM_NO_DEPRECATED)

// Test that header file is self-contained.
#include <boost/asio/system_executor.hpp>

#include <functional>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/post.hpp>
#include "unit_test.hpp"

using namespace boost::asio;

namespace bindns = std;

void increment(boost::asio::detail::atomic_count* count)
{
  ++(*count);
}

void system_executor_query_test()
{
  BOOST_ASIO_CHECK(
      &boost::asio::query(system_executor(),
        boost::asio::execution::context)
      != static_cast<const system_context*>(0));

  BOOST_ASIO_CHECK(
      boost::asio::query(system_executor(),
        boost::asio::execution::blocking)
      == boost::asio::execution::blocking.possibly);

  BOOST_ASIO_CHECK(
      boost::asio::query(system_executor(),
        boost::asio::execution::blocking.possibly)
      == boost::asio::execution::blocking.possibly);

  BOOST_ASIO_CHECK(
      boost::asio::query(system_executor(),
        boost::asio::execution::outstanding_work)
      == boost::asio::execution::outstanding_work.untracked);

  BOOST_ASIO_CHECK(
      boost::asio::query(system_executor(),
        boost::asio::execution::outstanding_work.untracked)
      == boost::asio::execution::outstanding_work.untracked);

  BOOST_ASIO_CHECK(
      boost::asio::query(system_executor(),
        boost::asio::execution::relationship)
      == boost::asio::execution::relationship.fork);

  BOOST_ASIO_CHECK(
      boost::asio::query(system_executor(),
        boost::asio::execution::relationship.fork)
      == boost::asio::execution::relationship.fork);

  BOOST_ASIO_CHECK(
      boost::asio::query(system_executor(),
        boost::asio::execution::mapping)
      == boost::asio::execution::mapping.thread);

  BOOST_ASIO_CHECK(
      boost::asio::query(system_executor(),
        boost::asio::execution::allocator)
      == std::allocator<void>());
}

void system_executor_execute_test()
{
  boost::asio::detail::atomic_count count(0);

  system_executor().execute(bindns::bind(increment, &count));

  boost::asio::require(system_executor(),
      boost::asio::execution::blocking.possibly
    ).execute(bindns::bind(increment, &count));

  boost::asio::require(system_executor(),
      boost::asio::execution::blocking.always
    ).execute(bindns::bind(increment, &count));

  boost::asio::require(system_executor(),
      boost::asio::execution::blocking.never
    ).execute(bindns::bind(increment, &count));

  boost::asio::require(system_executor(),
      boost::asio::execution::blocking.never,
      boost::asio::execution::outstanding_work.untracked
    ).execute(bindns::bind(increment, &count));

  boost::asio::require(system_executor(),
      boost::asio::execution::blocking.never,
      boost::asio::execution::outstanding_work.untracked,
      boost::asio::execution::relationship.fork
    ).execute(bindns::bind(increment, &count));

  boost::asio::require(system_executor(),
      boost::asio::execution::blocking.never,
      boost::asio::execution::outstanding_work.untracked,
      boost::asio::execution::relationship.continuation
    ).execute(bindns::bind(increment, &count));

  boost::asio::prefer(
      boost::asio::require(system_executor(),
        boost::asio::execution::blocking.never,
        boost::asio::execution::outstanding_work.untracked,
        boost::asio::execution::relationship.continuation),
      boost::asio::execution::allocator(std::allocator<void>())
    ).execute(bindns::bind(increment, &count));

  boost::asio::prefer(
      boost::asio::require(system_executor(),
        boost::asio::execution::blocking.never,
        boost::asio::execution::outstanding_work.untracked,
        boost::asio::execution::relationship.continuation),
      boost::asio::execution::allocator
    ).execute(bindns::bind(increment, &count));

  boost::asio::query(system_executor(), execution::context).join();

  BOOST_ASIO_CHECK(count == 9);
}

BOOST_ASIO_TEST_SUITE
(
  "system_executor",
  BOOST_ASIO_TEST_CASE(system_executor_query_test)
  BOOST_ASIO_TEST_CASE(system_executor_execute_test)
)
