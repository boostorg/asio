//
// experimental/co_composed.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
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
#include <boost/asio/experimental/co_composed.hpp>

#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include "../unit_test.hpp"

template <typename CompletionToken>
auto async_throw(CompletionToken&& token)
{
  return boost::asio::async_initiate<CompletionToken, void()>(
      [](auto) { throw 42; }, token);
}

template <typename CompletionToken>
auto throw_first(CompletionToken&& token)
{
  return boost::asio::async_initiate<CompletionToken, void()>(
      boost::asio::experimental::co_composed(
        [](auto state) -> void
        {
          throw 42;
          co_yield state.complete();
        }), token);
}

void test_throw_first()
{
  try
  {
    throw_first(boost::asio::detached);
    BOOST_ASIO_CHECK(0);
  }
  catch (int)
  {
  }
}

template <typename CompletionToken>
auto throw_after_await(boost::asio::io_context& ctx, CompletionToken&& token)
{
  return boost::asio::async_initiate<CompletionToken, void()>(
      boost::asio::experimental::co_composed(
        [](auto state, boost::asio::io_context& ctx) -> void
        {
          co_await boost::asio::post(ctx, boost::asio::deferred);
          throw 42;
          co_yield state.complete();
        }), token, std::ref(ctx));
}

void test_throw_after_await()
{
  try
  {
    boost::asio::io_context ctx(1);
    throw_after_await(ctx, boost::asio::detached);
    ctx.run();
    BOOST_ASIO_CHECK(0);
  }
  catch (int)
  {
  }
}

template <typename CompletionToken>
auto throw_in_first_suspend(CompletionToken&& token)
{
  return boost::asio::async_initiate<CompletionToken, void()>(
      boost::asio::experimental::co_composed(
        [](auto state) -> void
        {
          co_await async_throw(boost::asio::deferred);
          co_yield state.complete();
        }), token);
}

void test_throw_in_first_suspend()
{
  try
  {
    throw_in_first_suspend(boost::asio::detached);
    BOOST_ASIO_CHECK(0);
  }
  catch (int)
  {
  }
}

template <typename CompletionToken>
auto throw_in_suspend_after_await(
    boost::asio::io_context& ctx, CompletionToken&& token)
{
  return boost::asio::async_initiate<CompletionToken, void()>(
      boost::asio::experimental::co_composed(
        [](auto state, boost::asio::io_context& ctx) -> void
        {
          co_await boost::asio::post(ctx, boost::asio::deferred);
          co_await async_throw(boost::asio::deferred);
          co_yield state.complete();
        }), token, std::ref(ctx));
}

void test_throw_in_suspend_after_await()
{
  try
  {
    boost::asio::io_context ctx(1);
    throw_in_suspend_after_await(ctx, boost::asio::detached);
    ctx.run();
    BOOST_ASIO_CHECK(0);
  }
  catch (int)
  {
  }
}

template <typename CompletionToken>
auto post_loop(boost::asio::io_context& ctx, CompletionToken&& token)
{
  return boost::asio::async_initiate<CompletionToken, void(int)>(
      boost::asio::experimental::co_composed(
        [](auto state, boost::asio::io_context& ctx) -> void
        {
          int i = 0;
          for (; i < 100; ++i)
            co_await boost::asio::post(ctx, boost::asio::deferred);
          co_yield state.complete(i);
        }, ctx), token, std::ref(ctx));
}

void test_post_loop()
{
  boost::asio::io_context ctx(1);
  int count = 0;
  post_loop(ctx, [&](int i){ count = i; });
  ctx.run();
  BOOST_ASIO_CHECK(count == 100);
}

template <typename CompletionToken>
auto nested_post(boost::asio::io_context& ctx, CompletionToken&& token)
{
  return boost::asio::async_initiate<CompletionToken, void()>(
      boost::asio::experimental::co_composed(
        [](auto state, boost::asio::io_context& ctx) -> void
        {
          co_await boost::asio::post(ctx, boost::asio::deferred);
          co_yield state.complete();
        }, ctx), token, std::ref(ctx));
}

template <typename CompletionToken>
auto nested_post_loop(boost::asio::io_context& ctx, CompletionToken&& token)
{
  return boost::asio::async_initiate<CompletionToken, void(int)>(
      boost::asio::experimental::co_composed(
        [](auto state, boost::asio::io_context& ctx) -> void
        {
          int i = 0;
          for (; i < 100; ++i)
            co_await nested_post(ctx, boost::asio::deferred);
          co_yield state.complete(i);
        }, ctx), token, std::ref(ctx));
}

void test_nested_post_loop()
{
  boost::asio::io_context ctx(1);
  int count = 0;
  nested_post_loop(ctx, [&](int i){ count = i; });
  ctx.run();
  BOOST_ASIO_CHECK(count == 100);
}

template <typename CompletionToken>
auto post_loop_return_1_0(boost::asio::io_context& ctx, CompletionToken&& token)
{
  return boost::asio::async_initiate<CompletionToken, void()>(
      boost::asio::experimental::co_composed<void()>(
        [](auto, boost::asio::io_context& ctx) -> void
        {
          int i = 0;
          for (; i < 100; ++i)
            co_await boost::asio::post(ctx, boost::asio::deferred);
          co_return {};
        }, ctx), token, std::ref(ctx));
}

void test_post_loop_return_1_0()
{
  boost::asio::io_context ctx(1);
  bool done = false;
  post_loop_return_1_0(ctx, [&]{ done = true; });
  ctx.run();
  BOOST_ASIO_CHECK(done);
}

template <typename CompletionToken>
auto post_loop_return_1_1(boost::asio::io_context& ctx, CompletionToken&& token)
{
  return boost::asio::async_initiate<CompletionToken, void(int)>(
      boost::asio::experimental::co_composed<void(int)>(
        [](auto, boost::asio::io_context& ctx) -> void
        {
          int i = 0;
          for (; i < 100; ++i)
            co_await boost::asio::post(ctx, boost::asio::deferred);
          co_return {i};
        }, ctx), token, std::ref(ctx));
}

void test_post_loop_return_1_1()
{
  boost::asio::io_context ctx(1);
  int count = 0;
  post_loop_return_1_1(ctx, [&](int i){ count = i; });
  ctx.run();
  BOOST_ASIO_CHECK(count == 100);
}

template <typename CompletionToken>
auto post_loop_return_1_2(boost::asio::io_context& ctx, CompletionToken&& token)
{
  return boost::asio::async_initiate<CompletionToken, void(int, char)>(
      boost::asio::experimental::co_composed<void(int, char)>(
        [](auto, boost::asio::io_context& ctx) -> void
        {
          int i = 0;
          for (; i < 100; ++i)
            co_await boost::asio::post(ctx, boost::asio::deferred);
          co_return {i, 'A'};
        }, ctx), token, std::ref(ctx));
}

void test_post_loop_return_1_2()
{
  boost::asio::io_context ctx(1);
  int count = 0;
  char ch = 0;
  post_loop_return_1_2(ctx, [&](int i, char c){ count = i, ch = c; });
  ctx.run();
  BOOST_ASIO_CHECK(count == 100);
  BOOST_ASIO_CHECK(ch == 'A');
}

template <typename CompletionToken>
auto post_loop_return_2(boost::asio::io_context& ctx, CompletionToken&& token)
{
  return boost::asio::async_initiate<CompletionToken, void(), void(int)>(
      boost::asio::experimental::co_composed<void(), void(int)>(
        [](auto, boost::asio::io_context& ctx) -> void
        {
          int i = 0;
          for (; i < 100; ++i)
            co_await boost::asio::post(ctx, boost::asio::deferred);
          co_return {i};
        }, ctx), token, std::ref(ctx));
}

void test_post_loop_return_2()
{
  boost::asio::io_context ctx(1);
  int count = 0;
  post_loop_return_2(ctx, [&](int i = 0){ count = i; });
  ctx.run();
  BOOST_ASIO_CHECK(count == 100);
}

template <typename CompletionToken>
auto complete_on_cancel(boost::asio::io_context& ctx, CompletionToken&& token)
{
  return boost::asio::async_initiate<
    CompletionToken, void(boost::system::error_code, int)>(
      boost::asio::experimental::co_composed<
        void(boost::system::error_code, int)>(
          [](auto state, boost::asio::io_context& ctx) -> void
          {
            state.on_cancellation_complete_with(
                boost::asio::error::invalid_argument, 42);
            int i = 0;
            for (; i < 100; ++i)
              co_await boost::asio::post(ctx, boost::asio::deferred);
            co_return {boost::asio::error::eof, i};
          }, ctx), token, std::ref(ctx));
}

void test_complete_on_cancel()
{
  boost::asio::io_context ctx(1);
  int count = 0;
  boost::system::error_code ec;
  boost::asio::cancellation_signal cancel;

  complete_on_cancel(ctx,
      [&](boost::system::error_code e, int i)
      {
        ec = e;
        count = i;
      });

  ctx.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::eof);
  BOOST_ASIO_CHECK(count == 100);

  complete_on_cancel(ctx,
      boost::asio::bind_cancellation_slot(cancel.slot(),
        [&](boost::system::error_code e, int i)
        {
          ec = e;
          count = i;
        }));

  ctx.restart();
  ctx.run_one();
  cancel.emit(boost::asio::cancellation_type::all);
  ctx.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::invalid_argument);
  BOOST_ASIO_CHECK(count == 42);

  complete_on_cancel(ctx,
      boost::asio::bind_cancellation_slot(cancel.slot(),
        [&](boost::system::error_code e, int i)
        {
          ec = e;
          count = i;
        }));

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::eof);
  BOOST_ASIO_CHECK(count == 100);
}

template <typename CompletionToken>
auto complete_with_default_on_cancel(
    boost::asio::io_context& ctx, CompletionToken&& token)
{
  return boost::asio::async_initiate<
    CompletionToken, void(boost::system::error_code, int)>(
      boost::asio::experimental::co_composed<
        void(boost::system::error_code, int)>(
          [](auto, boost::asio::io_context& ctx) -> void
          {
            int i = 0;
            for (; i < 100; ++i)
              co_await boost::asio::post(ctx, boost::asio::deferred);
            co_return {boost::asio::error::eof, i};
          }, ctx), token, std::ref(ctx));
}

void test_complete_with_default_on_cancel()
{
  boost::asio::io_context ctx(1);
  int count = 0;
  boost::system::error_code ec;
  boost::asio::cancellation_signal cancel;

  complete_with_default_on_cancel(ctx,
      [&](boost::system::error_code e, int i)
      {
        ec = e;
        count = i;
      });

  ctx.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::eof);
  BOOST_ASIO_CHECK(count == 100);

  complete_with_default_on_cancel(ctx,
      boost::asio::bind_cancellation_slot(cancel.slot(),
        [&](boost::system::error_code e, int i)
        {
          ec = e;
          count = i;
        }));

  ctx.restart();
  ctx.run_one();
  cancel.emit(boost::asio::cancellation_type::all);
  ctx.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::operation_aborted);
  BOOST_ASIO_CHECK(count == 0);

  complete_with_default_on_cancel(ctx,
      boost::asio::bind_cancellation_slot(cancel.slot(),
        [&](boost::system::error_code e, int i)
        {
          ec = e;
          count = i;
        }));

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::eof);
  BOOST_ASIO_CHECK(count == 100);
}

template <typename CompletionToken>
auto throw_on_cancel(boost::asio::io_context& ctx, CompletionToken&& token)
{
  return boost::asio::async_initiate<
    CompletionToken, void(boost::system::error_code, int)>(
      boost::asio::experimental::co_composed<
        void(boost::system::error_code, int)>(
          [](auto state, boost::asio::io_context& ctx) -> void
          {
            try
            {
              state.throw_if_cancelled(true);
              int i = 0;
              for (; i < 100; ++i)
                co_await boost::asio::post(ctx, boost::asio::deferred);
              co_return {boost::asio::error::eof, i};
            }
            catch (...)
            {
              co_return {boost::asio::error::invalid_argument, 42};
            }
          }, ctx), token, std::ref(ctx));
}

void test_throw_on_cancel()
{
  boost::asio::io_context ctx(1);
  int count = 0;
  boost::system::error_code ec;
  boost::asio::cancellation_signal cancel;

  throw_on_cancel(ctx,
      [&](boost::system::error_code e, int i)
      {
        ec = e;
        count = i;
      });

  ctx.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::eof);
  BOOST_ASIO_CHECK(count == 100);

  throw_on_cancel(ctx,
      boost::asio::bind_cancellation_slot(cancel.slot(),
        [&](boost::system::error_code e, int i)
        {
          ec = e;
          count = i;
        }));

  ctx.restart();
  ctx.run_one();
  cancel.emit(boost::asio::cancellation_type::all);
  ctx.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::invalid_argument);
  BOOST_ASIO_CHECK(count == 42);

  throw_on_cancel(ctx,
      boost::asio::bind_cancellation_slot(cancel.slot(),
        [&](boost::system::error_code e, int i)
        {
          ec = e;
          count = i;
        }));

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::eof);
  BOOST_ASIO_CHECK(count == 100);
}

BOOST_ASIO_TEST_SUITE
(
  "experimental/co_composed",
  BOOST_ASIO_TEST_CASE(test_throw_first)
  BOOST_ASIO_TEST_CASE(test_throw_after_await)
  BOOST_ASIO_TEST_CASE(test_throw_in_first_suspend)
  BOOST_ASIO_TEST_CASE(test_throw_in_suspend_after_await)
  BOOST_ASIO_TEST_CASE(test_post_loop)
  BOOST_ASIO_TEST_CASE(test_nested_post_loop)
  BOOST_ASIO_TEST_CASE(test_post_loop_return_1_0)
  BOOST_ASIO_TEST_CASE(test_post_loop_return_1_1)
  BOOST_ASIO_TEST_CASE(test_post_loop_return_1_2)
  BOOST_ASIO_TEST_CASE(test_post_loop_return_2)
  BOOST_ASIO_TEST_CASE(test_complete_on_cancel)
  BOOST_ASIO_TEST_CASE(test_complete_with_default_on_cancel)
  BOOST_ASIO_TEST_CASE(test_throw_on_cancel)
)
