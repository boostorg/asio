//
// spawn.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2024 Casey Bodley (cbodley at redhat dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <boost/asio/spawn.hpp>

#include "unit_test.hpp"

#if defined(BOOST_ASIO_HAS_BOOST_CONTEXT_FIBER)

#include <memory>
#include <stdexcept>
#include <boost/asio/any_completion_handler.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>

void void_returning_coroutine(boost::asio::yield_context)
{
}

int int_returning_coroutine(boost::asio::yield_context)
{
  return 42;
}

void test_spawn_with_any_completion_handler()
{
  boost::asio::io_context ctx;

  bool called = false;
  boost::asio::spawn(ctx, void_returning_coroutine,
      boost::asio::any_completion_handler<void(std::exception_ptr)>(
        [&](std::exception_ptr)
        {
          called = true;
        }));

  BOOST_ASIO_CHECK(!called);

  ctx.run();

  BOOST_ASIO_CHECK(called);

  int result = 0;
  boost::asio::spawn(ctx, int_returning_coroutine,
      boost::asio::any_completion_handler<void(std::exception_ptr, int)>(
        [&](std::exception_ptr, int i)
        {
          result = i;
        }));

  BOOST_ASIO_CHECK(result == 0);

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(result == 42);
}

void test_spawn_deferred()
{
  boost::asio::io_context ctx;

  {
    bool called = false;
    auto fn = boost::asio::spawn(ctx, void_returning_coroutine, boost::asio::deferred);

    fn([&](std::exception_ptr)
        {
          called = true;
        });

    BOOST_ASIO_CHECK(!called);

    ctx.poll();

    BOOST_ASIO_CHECK(ctx.stopped());
    BOOST_ASIO_CHECK(called);
  }
  {
    int result = 0;
    auto fn = boost::asio::spawn(ctx, int_returning_coroutine, boost::asio::deferred);

    fn([&](std::exception_ptr, int i)
        {
          result = i;
        });

    BOOST_ASIO_CHECK(result == 0);

    ctx.restart();
    ctx.poll();

    BOOST_ASIO_CHECK(ctx.stopped());
    BOOST_ASIO_CHECK(result == 42);
  }
}

void sleeping_coroutine(boost::asio::yield_context yield)
{
  boost::asio::steady_timer timer(yield.get_executor(),
      boost::asio::steady_timer::time_point::max());
  timer.async_wait(yield);
}

void test_spawn_cancel()
{
  boost::asio::cancellation_signal sig;
  boost::asio::io_context ctx;

  std::exception_ptr result = nullptr;
  bool called = false;
  boost::asio::spawn(ctx, sleeping_coroutine,
      boost::asio::bind_cancellation_slot(sig.slot(),
        [&](std::exception_ptr e)
        {
          result = e;
          called = true;
        }));

  ctx.poll();
  BOOST_ASIO_CHECK(!ctx.stopped());

  BOOST_ASIO_CHECK(!called);
  BOOST_ASIO_CHECK(result == nullptr);

  sig.emit(boost::asio::cancellation_type::all);

  ctx.poll();
  BOOST_ASIO_CHECK(ctx.stopped());

  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(result != nullptr);
  try
  {
    std::rethrow_exception(result);
  }
  catch (const boost::system::system_error& e)
  {
    BOOST_ASIO_CHECK(e.code() == boost::asio::error::operation_aborted);
  }
  catch (...)
  {
    BOOST_ASIO_ERROR("expected system_error");
  }
}

void throwing_coroutine(boost::asio::yield_context)
{
  throw std::runtime_error("oops");
}

void test_spawn_exception()
{
  boost::asio::io_context ctx;

  std::exception_ptr result = nullptr;
  bool called = false;
  boost::asio::spawn(ctx, throwing_coroutine,
      [&](std::exception_ptr e)
      {
        result = e;
        called = true;
      });

  ctx.poll();
  BOOST_ASIO_CHECK(ctx.stopped());

  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(result != nullptr);
  try
  {
    std::rethrow_exception(result);
  }
  catch (const std::runtime_error&)
  {
    // ok
  }
  catch (...)
  {
    BOOST_ASIO_ERROR("expected runtime_error");
  }
}

std::unique_ptr<int> factory_coroutine(boost::asio::yield_context)
{
  return std::unique_ptr<int>(new int(42));
}

void test_spawn_return_move_only()
{
  boost::asio::io_context ctx;

  std::unique_ptr<int> result;
  bool called = false;
  boost::asio::spawn(ctx, factory_coroutine,
      [&](std::exception_ptr, std::unique_ptr<int> r)
      {
        result = std::move(r);
        called = true;
      });

  ctx.poll();
  BOOST_ASIO_CHECK(ctx.stopped());

  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(result);
  BOOST_ASIO_CHECK(*result == 42);
}

BOOST_ASIO_TEST_SUITE
(
  "spawn",
  BOOST_ASIO_TEST_CASE(test_spawn_with_any_completion_handler)
  BOOST_ASIO_TEST_CASE(test_spawn_deferred)
  BOOST_ASIO_TEST_CASE(test_spawn_cancel)
  BOOST_ASIO_TEST_CASE(test_spawn_exception)
  BOOST_ASIO_TEST_CASE(test_spawn_return_move_only)
)

#else // defined(BOOST_ASIO_HAS_BOOST_CONTEXT_FIBER)

BOOST_ASIO_TEST_SUITE
(
  "spawn",
  BOOST_ASIO_TEST_CASE(null_test)
)

#endif // defined(BOOST_ASIO_HAS_BOOST_CONTEXT_FIBER)
