//
// any_executor.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <boost/asio/execution/any_executor.hpp>

#include <boost/asio/thread_pool.hpp>
#include "../unit_test.hpp"

#if defined(BOOST_ASIO_HAS_BOOST_BIND)
# include <boost/bind/bind.hpp>
#else // defined(BOOST_ASIO_HAS_BOOST_BIND)
# include <functional>
#endif // defined(BOOST_ASIO_HAS_BOOST_BIND)

using namespace boost::asio;

#if defined(BOOST_ASIO_HAS_BOOST_BIND)
namespace bindns = boost;
#else // defined(BOOST_ASIO_HAS_BOOST_BIND)
namespace bindns = std;
#endif

void increment(int* count)
{
  ++(*count);
}

void any_executor_executor_query_test()
{
  thread_pool pool(1);
  execution::any_executor<
      execution::blocking_t,
      execution::outstanding_work_t,
      execution::relationship_t,
      execution::mapping_t::thread_t,
      execution::occupancy_t>
    ex(pool.executor());

  BOOST_ASIO_CHECK(
      boost::asio::query(ex, boost::asio::execution::blocking)
        == boost::asio::execution::blocking.possibly);

  BOOST_ASIO_CHECK(
      boost::asio::query(ex, boost::asio::execution::blocking.possibly)
        == boost::asio::execution::blocking.possibly);

  BOOST_ASIO_CHECK(
      boost::asio::query(ex, boost::asio::execution::outstanding_work)
        == boost::asio::execution::outstanding_work.untracked);

  BOOST_ASIO_CHECK(
      boost::asio::query(ex, boost::asio::execution::outstanding_work.untracked)
        == boost::asio::execution::outstanding_work.untracked);

  BOOST_ASIO_CHECK(
      boost::asio::query(ex, boost::asio::execution::relationship)
        == boost::asio::execution::relationship.fork);

  BOOST_ASIO_CHECK(
      boost::asio::query(ex, boost::asio::execution::relationship.fork)
        == boost::asio::execution::relationship.fork);

  BOOST_ASIO_CHECK(
      boost::asio::query(ex, boost::asio::execution::mapping)
        == boost::asio::execution::mapping.thread);

  BOOST_ASIO_CHECK(
      boost::asio::query(ex, boost::asio::execution::occupancy)
        == 1);
}

void any_executor_executor_execute_test()
{
  int count = 0;
  thread_pool pool(1);
  execution::any_executor<
      execution::blocking_t::possibly_t,
      execution::blocking_t::never_t,
      execution::outstanding_work_t::untracked_t,
      execution::outstanding_work_t::tracked_t,
      execution::relationship_t::continuation_t>
    ex(pool.executor());

  boost::asio::execution::execute(pool.executor(),
      bindns::bind(increment, &count));

  boost::asio::execution::execute(
      boost::asio::require(pool.executor(),
        boost::asio::execution::blocking.possibly),
      bindns::bind(increment, &count));

  boost::asio::execution::execute(
      boost::asio::require(pool.executor(),
        boost::asio::execution::blocking.never),
      bindns::bind(increment, &count));

  boost::asio::execution::execute(
      boost::asio::require(pool.executor(),
        boost::asio::execution::blocking.never,
        boost::asio::execution::outstanding_work.tracked),
      bindns::bind(increment, &count));

  boost::asio::execution::execute(
      boost::asio::require(pool.executor(),
        boost::asio::execution::blocking.never,
        boost::asio::execution::outstanding_work.untracked),
      bindns::bind(increment, &count));

  boost::asio::execution::execute(
      boost::asio::require(pool.executor(),
        boost::asio::execution::blocking.never,
        boost::asio::execution::outstanding_work.untracked,
        boost::asio::execution::relationship.continuation),
      bindns::bind(increment, &count));

  pool.wait();

  BOOST_ASIO_CHECK(count == 6);
}

BOOST_ASIO_TEST_SUITE
(
  "any_executor",
  BOOST_ASIO_TEST_CASE(any_executor_executor_query_test)
  BOOST_ASIO_TEST_CASE(any_executor_executor_execute_test)
)
