//
// spawn.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
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

#if defined(BOOST_ASIO_HAS_BOOST_CONTEXT_FIBER) \
  || defined(BOOST_ASIO_HAS_ASM_FIBER)

#include <memory>
#include <stdexcept>
#include <vector>
#include "archetypes/async_ops.hpp"
#include <boost/asio/any_completion_handler.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>

#if defined(BOOST_ASIO_HAS_ASM_FIBER)
# include <boost/asio/detail/default_stack_allocator.hpp>
#endif // defined(BOOST_ASIO_HAS_ASM_FIBER)

#if defined(BOOST_ASIO_HAS_FIBER_GUARD_PAGE)
# include <cerrno>
# include <unistd.h>
#endif // defined(BOOST_ASIO_HAS_FIBER_GUARD_PAGE)

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
#if !defined(BOOST_ASIO_NO_EXCEPTIONS)
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
#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)
}

#if !defined(BOOST_ASIO_NO_EXCEPTIONS)

void throwing_coroutine(boost::asio::yield_context)
{
  throw std::runtime_error("oops");
}

#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)

void test_spawn_exception()
{
#if !defined(BOOST_ASIO_NO_EXCEPTIONS)
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
#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)
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

#if !defined(BOOST_ASIO_NO_EXCEPTIONS)

int coroutine_using_async_ops_0(boost::asio::yield_context yield)
{
  using namespace archetypes;

  try
  {
    async_op_0(yield);
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    async_op_ec_0(true, yield);
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    async_op_ec_0(false, yield);
    BOOST_ASIO_CHECK(false);
  }
  catch (boost::system::system_error& e)
  {
    BOOST_ASIO_CHECK(e.code() == boost::asio::error::operation_aborted);
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    async_op_ex_0(true, yield);
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    async_op_ex_0(false, yield);
    BOOST_ASIO_CHECK(false);
  }
  catch (std::exception& e)
  {
    BOOST_ASIO_CHECK(e.what() == std::string("blah"));
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  return 123;
}

int coroutine_using_async_ops_1(boost::asio::yield_context yield)
{
  using namespace archetypes;

  try
  {
    int i = async_op_1(yield);
    BOOST_ASIO_CHECK(i == 42);
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    int i = async_op_ec_1(true, yield);
    BOOST_ASIO_CHECK(i == 42);
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    int i = async_op_ec_1(false, yield);
    BOOST_ASIO_CHECK(false);
    (void)i;
  }
  catch (boost::system::system_error& e)
  {
    BOOST_ASIO_CHECK(e.code() == boost::asio::error::operation_aborted);
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    int i = async_op_ex_1(true, yield);
    BOOST_ASIO_CHECK(i == 42);
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    int i = async_op_ex_1(false, yield);
    BOOST_ASIO_CHECK(false);
    (void)i;
  }
  catch (std::exception& e)
  {
    BOOST_ASIO_CHECK(e.what() == std::string("blah"));
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  return 234;
}

int coroutine_using_async_ops_2(boost::asio::yield_context yield)
{
  using namespace archetypes;

  try
  {
    int i;
    double d;
    std::tie(i, d) = async_op_2(yield);
    BOOST_ASIO_CHECK(i == 42);
    BOOST_ASIO_CHECK(d == 2.0);
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    int i;
    double d;
    std::tie(i, d) = async_op_ec_2(true, yield);
    BOOST_ASIO_CHECK(i == 42);
    BOOST_ASIO_CHECK(d == 2.0);
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    std::tuple<int, double> t = async_op_ec_2(false, yield);
    BOOST_ASIO_CHECK(false);
    (void)t;
  }
  catch (boost::system::system_error& e)
  {
    BOOST_ASIO_CHECK(e.code() == boost::asio::error::operation_aborted);
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    int i;
    double d;
    std::tie(i, d) = async_op_ex_2(true, yield);
    BOOST_ASIO_CHECK(i == 42);
    BOOST_ASIO_CHECK(d == 2.0);
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    std::tuple<int, double> t = async_op_ex_2(false, yield);
    BOOST_ASIO_CHECK(false);
    (void)t;
  }
  catch (std::exception& e)
  {
    BOOST_ASIO_CHECK(e.what() == std::string("blah"));
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  return 345;
}

int coroutine_using_async_ops_3(boost::asio::yield_context yield)
{
  using namespace archetypes;

  try
  {
    int i;
    double d;
    char c;
    std::tie(i, d, c) = async_op_3(yield);
    BOOST_ASIO_CHECK(i == 42);
    BOOST_ASIO_CHECK(d == 2.0);
    BOOST_ASIO_CHECK(c == 'a');
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    int i;
    double d;
    char c;
    std::tie(i, d, c) = async_op_ec_3(true, yield);
    BOOST_ASIO_CHECK(i == 42);
    BOOST_ASIO_CHECK(d == 2.0);
    BOOST_ASIO_CHECK(c == 'a');
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    std::tuple<int, double, char> t = async_op_ec_3(false, yield);
    BOOST_ASIO_CHECK(false);
    (void)t;
  }
  catch (boost::system::system_error& e)
  {
    BOOST_ASIO_CHECK(e.code() == boost::asio::error::operation_aborted);
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    int i;
    double d;
    char c;
    std::tie(i, d, c) = async_op_ex_3(true, yield);
    BOOST_ASIO_CHECK(i == 42);
    BOOST_ASIO_CHECK(d == 2.0);
    BOOST_ASIO_CHECK(c == 'a');
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  try
  {
    std::tuple<int, double, char> t = async_op_ex_3(false, yield);
    BOOST_ASIO_CHECK(false);
    (void)t;
  }
  catch (std::exception& e)
  {
    BOOST_ASIO_CHECK(e.what() == std::string("blah"));
  }
  catch (...)
  {
    BOOST_ASIO_CHECK(false);
  }

  return 456;
}

#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)

void test_spawn_async_ops()
{
#if !defined(BOOST_ASIO_NO_EXCEPTIONS)
  boost::asio::io_context ctx;

  bool called = false;
  boost::asio::spawn(ctx, coroutine_using_async_ops_0,
      [&](std::exception_ptr, int i)
      {
        BOOST_ASIO_CHECK(i == 123);
        called = true;
      });

  BOOST_ASIO_CHECK(!called);

  ctx.run();

  BOOST_ASIO_CHECK(called);

  called = false;
  boost::asio::spawn(ctx, coroutine_using_async_ops_1,
      [&](std::exception_ptr, int i)
      {
        BOOST_ASIO_CHECK(i == 234);
        called = true;
      });

  BOOST_ASIO_CHECK(!called);

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(called);

  called = false;
  boost::asio::spawn(ctx, coroutine_using_async_ops_2,
      [&](std::exception_ptr, int i)
      {
        BOOST_ASIO_CHECK(i == 345);
        called = true;
      });

  BOOST_ASIO_CHECK(!called);

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(called);

  called = false;
  boost::asio::spawn(ctx, coroutine_using_async_ops_3,
      [&](std::exception_ptr, int i)
      {
        BOOST_ASIO_CHECK(i == 456);
        called = true;
      });

  BOOST_ASIO_CHECK(!called);

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(called);
#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)
}

int stack_object_count = 0;

struct stack_object
{
  stack_object() { ++stack_object_count; }
  stack_object(const stack_object&) { ++stack_object_count; }
  ~stack_object() { --stack_object_count; }
};

void test_spawn_io_context_destroyed()
{
  std::exception_ptr result = nullptr;
  bool called = false;

  BOOST_ASIO_CHECK(stack_object_count == 0);

  {
    boost::asio::io_context ctx;

    boost::asio::spawn(ctx,
        [](boost::asio::yield_context yield)
        {
          stack_object a;
          stack_object b;

          boost::asio::steady_timer timer(yield.get_executor());
          timer.expires_at(boost::asio::steady_timer::time_point::max());
          timer.async_wait(yield);

          BOOST_ASIO_CHECK(false);
        },
        [&](std::exception_ptr e)
        {
          result = e;
          called = true;
        });

    ctx.poll();
    BOOST_ASIO_CHECK(!ctx.stopped());

    BOOST_ASIO_CHECK(stack_object_count == 2);
    BOOST_ASIO_CHECK(!called);

  }

#if !defined(BOOST_ASIO_NO_EXCEPTIONS)
  BOOST_ASIO_CHECK(stack_object_count == 0);
#else // !defined(BOOST_ASIO_NO_EXCEPTIONS)
  BOOST_ASIO_CHECK(stack_object_count == 2);
  stack_object_count = 0;
#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)

  BOOST_ASIO_CHECK(!called);
  BOOST_ASIO_CHECK(result == nullptr);
}

void test_spawn_io_context_destroyed_in_catch()
{
#if !defined(BOOST_ASIO_NO_EXCEPTIONS)
  BOOST_ASIO_CHECK(stack_object_count == 0);

  bool called = false;
  bool reached_catch = false;

  {
    boost::asio::io_context ctx;

    boost::asio::spawn(ctx,
        [&](boost::asio::yield_context yield)
        {
          try
          {
            throw std::runtime_error("test");
          }
          catch (const std::runtime_error&)
          {
            reached_catch = true;
            stack_object a;

            boost::asio::steady_timer timer(yield.get_executor());
            timer.expires_at(boost::asio::steady_timer::time_point::max());
            timer.async_wait(yield);

            BOOST_ASIO_CHECK(false);
          }
        },
        [&](std::exception_ptr) { called = true; });

    ctx.poll();
    BOOST_ASIO_CHECK(reached_catch);
    BOOST_ASIO_CHECK(stack_object_count == 1);
    BOOST_ASIO_CHECK(!called);
  }

  BOOST_ASIO_CHECK(stack_object_count == 0);
  BOOST_ASIO_CHECK(!called);
#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)
}

void test_spawn_nested()
{
  boost::asio::io_context ctx;

  int order = 0;
  int outer_at = 0;
  int inner_at = 0;

  boost::asio::spawn(ctx,
      [&](boost::asio::yield_context yield)
      {
        outer_at = ++order;

        boost::asio::spawn(yield,
            [&](boost::asio::yield_context)
            {
              inner_at = ++order;
            }, yield);
      }, boost::asio::detached);

  ctx.run();

  BOOST_ASIO_CHECK(outer_at == 1);
  BOOST_ASIO_CHECK(inner_at == 2);
}

void test_spawn_many()
{
  const int n = 16;
  const int rounds = 4;

  boost::asio::io_context ctx;
  std::vector<int> counts(n, 0);

  for (int i = 0; i < n; ++i)
  {
    boost::asio::spawn(ctx,
        [&counts, i](boost::asio::yield_context yield)
        {
          for (int j = 0; j < rounds; ++j)
          {
            ++counts[i];
            boost::asio::post(yield);
          }
        }, boost::asio::detached);
  }

  ctx.run();

  for (int i = 0; i < n; ++i)
    BOOST_ASIO_CHECK(counts[i] == rounds);
}

void test_spawn_completes_out_of_line()
{
  boost::asio::io_context ctx;

  int depth = 0;
  int max_depth = 0;
  bool called = false;

  boost::asio::post(ctx,
      [&]
      {
        ++depth;
        max_depth = depth > max_depth ? depth : max_depth;

        boost::asio::spawn(ctx,
            [](boost::asio::yield_context)
            {
            },
            [&](std::exception_ptr)
            {
              ++depth;
              max_depth = depth > max_depth ? depth : max_depth;
              called = true;
              --depth;
            });

        --depth;
      });

  ctx.run();

  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(max_depth == 1);
}

void test_spawn_inline_completions()
{
  boost::asio::io_context ctx;

  int completed = 0;
  int destroyed = 0;

  struct tracker
  {
    int& destroyed_;
    ~tracker() { ++destroyed_; }
  };

  boost::asio::spawn(ctx,
      [&](boost::asio::yield_context yield)
      {
        tracker t{destroyed};
        for (int i = 0; i < 16; ++i)
          boost::asio::dispatch(yield);
        completed = 1;
      }, boost::asio::detached);

  ctx.run();

  BOOST_ASIO_CHECK(completed == 1);
  BOOST_ASIO_CHECK(destroyed == 1);
}

void test_spawn_inline_completions_abandoned()
{
#if !defined(BOOST_ASIO_NO_EXCEPTIONS)
  int destroyed = 0;

  struct tracker
  {
    int& destroyed_;
    ~tracker() { ++destroyed_; }
  };

  {
    boost::asio::io_context ctx;
    boost::asio::steady_timer timer(ctx);

    boost::asio::spawn(ctx,
        [&](boost::asio::yield_context yield)
        {
          tracker t{destroyed};
          for (int i = 0; i < 8; ++i)
            boost::asio::dispatch(yield);
          timer.expires_after(boost::asio::chrono::hours(1));
          timer.async_wait(yield);
        }, boost::asio::detached);

    ctx.run_one();
  }

  BOOST_ASIO_CHECK(destroyed == 1);
#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)
}

struct test_stack_allocator
{
  struct context
  {
    void* sp;
    std::size_t size;
    void* base;

    context(void* sp_arg, std::size_t size_arg, void* base_arg)
      : sp(sp_arg), size(size_arg), base(base_arg)
    {
    }

    context(const context&) = default;
    context() = delete;
    context& operator=(const context&) = delete;
  };

  int* allocated;
  int* deallocated;

  context allocate() const
  {
    const std::size_t size = BOOST_ASIO_DEFAULT_FIBER_STACK_SIZE;
    char* base = new char[size];
    ++*allocated;
    return context(base + size, size, base);
  }

  void deallocate(context& c) const
  {
    ++*deallocated;
    delete[] static_cast<char*>(c.base);
  }
};

void test_spawn_stack_allocator()
{
  int allocated = 0;
  int deallocated = 0;
  int count = 0;

  {
    boost::asio::io_context ctx;
    test_stack_allocator allocator = { &allocated, &deallocated };

    boost::asio::spawn(ctx, boost::asio::allocator_arg_t(), allocator,
        [&](boost::asio::yield_context yield)
        {
          for (int i = 0; i < 4; ++i)
          {
            boost::asio::post(yield);
            ++count;
          }
        }, boost::asio::detached);

    BOOST_ASIO_CHECK(allocated == 1);
    BOOST_ASIO_CHECK(deallocated == 0);

    ctx.run();
  }

  BOOST_ASIO_CHECK(count == 4);
  BOOST_ASIO_CHECK(allocated == 1);
  BOOST_ASIO_CHECK(deallocated == 1);
}

void test_default_stack_allocator()
{
#if defined(BOOST_ASIO_HAS_ASM_FIBER)
  typedef boost::asio::detail::default_stack_allocator allocator_type;
  allocator_type allocator(BOOST_ASIO_DEFAULT_FIBER_STACK_SIZE);

  // Successive stacks start at different offsets, so try every one of them.
  for (int i = 0; i < allocator_type::colour_count; ++i)
  {
    allocator_type::stack_context stack = allocator.allocate();
    char* sp = static_cast<char*>(stack.sp);

    BOOST_ASIO_CHECK(sp - stack.size == static_cast<char*>(stack.base));
    BOOST_ASIO_CHECK(stack.size >= BOOST_ASIO_DEFAULT_FIBER_STACK_SIZE);
    sp[-1] = 0;

    allocator.deallocate(stack);
  }
#endif // defined(BOOST_ASIO_HAS_ASM_FIBER)
}

void test_stack_guard_page()
{
#if defined(BOOST_ASIO_HAS_FIBER_GUARD_PAGE)
  boost::asio::detail::default_stack_allocator allocator(
      BOOST_ASIO_DEFAULT_FIBER_STACK_SIZE);
  boost::asio::detail::default_stack_allocator::stack_context stack =
    allocator.allocate();

  int fds[2];
  BOOST_ASIO_CHECK(::pipe(fds) == 0);

  errno = 0;
  BOOST_ASIO_CHECK(::write(fds[1], stack.base, 1) == -1);
  BOOST_ASIO_CHECK(errno == EFAULT);

  BOOST_ASIO_CHECK(::write(fds[1],
        static_cast<char*>(stack.base) + allocator.page_size(), 1) == 1);

  BOOST_ASIO_CHECK(::close(fds[0]) == 0);
  BOOST_ASIO_CHECK(::close(fds[1]) == 0);
  allocator.deallocate(stack);
#endif // defined(BOOST_ASIO_HAS_FIBER_GUARD_PAGE)
}

BOOST_ASIO_TEST_SUITE
(
  "spawn",
  BOOST_ASIO_TEST_CASE(test_spawn_with_any_completion_handler)
  BOOST_ASIO_TEST_CASE(test_spawn_deferred)
  BOOST_ASIO_TEST_CASE(test_spawn_cancel)
  BOOST_ASIO_TEST_CASE(test_spawn_exception)
  BOOST_ASIO_TEST_CASE(test_spawn_return_move_only)
  BOOST_ASIO_TEST_CASE(test_spawn_async_ops)
  BOOST_ASIO_TEST_CASE(test_spawn_completes_out_of_line)
  BOOST_ASIO_TEST_CASE(test_spawn_io_context_destroyed)
  BOOST_ASIO_TEST_CASE(test_spawn_io_context_destroyed_in_catch)
  BOOST_ASIO_TEST_CASE(test_spawn_nested)
  BOOST_ASIO_TEST_CASE(test_spawn_many)
  BOOST_ASIO_TEST_CASE(test_spawn_inline_completions)
  BOOST_ASIO_TEST_CASE(test_spawn_inline_completions_abandoned)
  BOOST_ASIO_TEST_CASE(test_spawn_stack_allocator)
  BOOST_ASIO_TEST_CASE(test_default_stack_allocator)
  BOOST_ASIO_TEST_CASE(test_stack_guard_page)
)

#else // defined(BOOST_ASIO_HAS_BOOST_CONTEXT_FIBER)

BOOST_ASIO_TEST_SUITE
(
  "spawn",
  BOOST_ASIO_TEST_CASE(null_test)
)

#endif // defined(BOOST_ASIO_HAS_BOOST_CONTEXT_FIBER)
