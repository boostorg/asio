//
// thread_pool.cpp
// ~~~~~~~~~~~~~~~
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
#include <boost/asio/thread_pool.hpp>

#include <boost/asio/dispatch.hpp>
#include <boost/asio/post.hpp>
#include "unit_test.hpp"

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

void decrement_to_zero(thread_pool* pool, int* count)
{
  if (*count > 0)
  {
    --(*count);

    int before_value = *count;
    boost::asio::post(*pool, bindns::bind(decrement_to_zero, pool, count));

    // Handler execution cannot nest, so count value should remain unchanged.
    BOOST_ASIO_CHECK(*count == before_value);
  }
}

void nested_decrement_to_zero(thread_pool* pool, int* count)
{
  if (*count > 0)
  {
    --(*count);

    boost::asio::dispatch(*pool,
        bindns::bind(nested_decrement_to_zero, pool, count));

    // Handler execution is nested, so count value should now be zero.
    BOOST_ASIO_CHECK(*count == 0);
  }
}

void thread_pool_test()
{
  thread_pool pool(1);

  int count1 = 0;
  boost::asio::post(pool, bindns::bind(increment, &count1));

  int count2 = 10;
  boost::asio::post(pool, bindns::bind(decrement_to_zero, &pool, &count2));

  int count3 = 10;
  boost::asio::post(pool, bindns::bind(nested_decrement_to_zero, &pool, &count3));

  pool.join();

  BOOST_ASIO_CHECK(count1 == 1);
  BOOST_ASIO_CHECK(count2 == 0);
  BOOST_ASIO_CHECK(count3 == 0);
}

class test_service : public boost::asio::execution_context::service
{
public:
  typedef test_service key_type;

  test_service(boost::asio::execution_context& ctx)
    : boost::asio::execution_context::service(ctx)
  {
  }

private:
  virtual void shutdown() {}
};

void thread_pool_service_test()
{
  boost::asio::thread_pool pool1(1);
  boost::asio::thread_pool pool2(1);
  boost::asio::thread_pool pool3(1);

  // Implicit service registration.

  boost::asio::use_service<test_service>(pool1);

  BOOST_ASIO_CHECK(boost::asio::has_service<test_service>(pool1));

  test_service* svc1 = new test_service(pool1);
  try
  {
    boost::asio::add_service(pool1, svc1);
    BOOST_ASIO_ERROR("add_service did not throw");
  }
  catch (boost::asio::service_already_exists&)
  {
  }
  delete svc1;

  // Explicit service registration.

  test_service& svc2 = boost::asio::make_service<test_service>(pool2);

  BOOST_ASIO_CHECK(boost::asio::has_service<test_service>(pool2));
  BOOST_ASIO_CHECK(&boost::asio::use_service<test_service>(pool2) == &svc2);

  test_service* svc3 = new test_service(pool2);
  try
  {
    boost::asio::add_service(pool2, svc3);
    BOOST_ASIO_ERROR("add_service did not throw");
  }
  catch (boost::asio::service_already_exists&)
  {
  }
  delete svc3;

  // Explicit registration with invalid owner.

  test_service* svc4 = new test_service(pool2);
  try
  {
    boost::asio::add_service(pool3, svc4);
    BOOST_ASIO_ERROR("add_service did not throw");
  }
  catch (boost::asio::invalid_service_owner&)
  {
  }
  delete svc4;

  BOOST_ASIO_CHECK(!boost::asio::has_service<test_service>(pool3));
}

BOOST_ASIO_TEST_SUITE
(
  "thread_pool",
  BOOST_ASIO_TEST_CASE(thread_pool_test)
  BOOST_ASIO_TEST_CASE(thread_pool_service_test)
)
