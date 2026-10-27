//
// trampoline.cpp
// ~~~~~~~~~~~~~~
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
#include <boost/asio/trampoline.hpp>

#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <boost/asio/associated_allocator.hpp>
#include <boost/asio/bind_allocator.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/defer.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/inline_or_executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/execution/any_executor.hpp>
#include "unit_test.hpp"

using namespace boost::asio;
namespace bindns = std;

namespace {

typedef boost::asio::execution::detail::any_executor_base::object_type
  any_executor_object_type;

typedef trampoline<io_context::executor_type> small_trampoline_type;

static_assert(
    sizeof(small_trampoline_type)
      <= sizeof(any_executor_object_type)
      && alignment_of<small_trampoline_type>::value
        <= alignment_of<any_executor_object_type>::value,
    "trampoline<io_context::executor_type> no longer fits "
    "any_executor's small-object storage");

} // namespace

void increment(int* count)
{
  ++(*count);
}

void trampoline_conversion_test()
{
  io_context ioc;
  trampoline<io_context::executor_type> s1
    = make_trampoline(ioc, 4);

  trampoline<any_io_executor> s2(s1);
  trampoline<any_io_executor> s3 =
    trampoline<io_context::executor_type>(s1);

  s3 = s1;
  s3 = trampoline<io_context::executor_type>(s1);
}

void trampoline_query_test()
{
  io_context ioc;
  trampoline<io_context::executor_type> s1
    = make_trampoline(ioc, 4);

  BOOST_ASIO_CHECK(
      &boost::asio::query(s1, boost::asio::execution::context) == &ioc);

  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::blocking)
        == boost::asio::execution::blocking.possibly);

  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::outstanding_work)
        == boost::asio::execution::outstanding_work.untracked);

  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::relationship)
        == boost::asio::execution::relationship.continuation);

  BOOST_ASIO_CHECK((boost::asio::can_require<
      trampoline<io_context::executor_type>,
      execution::relationship_t::continuation_t>::value));
  BOOST_ASIO_CHECK((boost::asio::can_prefer<
      trampoline<io_context::executor_type>,
      execution::relationship_t::continuation_t>::value));
  auto s1b = boost::asio::require(s1, boost::asio::execution::relationship.continuation);
  auto s1c = boost::asio::prefer(s1, boost::asio::execution::relationship.continuation);
  static_assert(std::is_same<decltype(s1b), decltype(s1)>::value,
      "require(relationship.continuation) must preserve type");
  static_assert(std::is_same<decltype(s1c), decltype(s1)>::value,
      "prefer(relationship.continuation) must preserve type");
  BOOST_ASIO_CHECK(s1b == s1);
  BOOST_ASIO_CHECK(s1c == s1);

  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::allocator)
        == std::allocator<void>());

  trampoline<io_context::executor_type,
      execution::blocking_t::never_t> s2 =
    boost::asio::require(s1, boost::asio::execution::blocking.never);
  BOOST_ASIO_CHECK(
      boost::asio::query(s2, boost::asio::execution::blocking)
        == boost::asio::execution::blocking.never);

  BOOST_ASIO_CHECK((!boost::asio::can_require<
      trampoline<io_context::executor_type>,
      execution::relationship_t::fork_t>::value));

  auto s3 = boost::asio::prefer(s1, boost::asio::execution::relationship.fork);
  static_assert(std::is_same<decltype(s3), decltype(s1)>::value,
      "prefer(relationship.fork) must fall back to identity");
  BOOST_ASIO_CHECK(s3 == s1);
  BOOST_ASIO_CHECK(
      boost::asio::query(s3, boost::asio::execution::relationship)
        == boost::asio::execution::relationship.continuation);
}

void trampoline_equality_test()
{
  io_context ioc;

  trampoline<io_context::executor_type> s1
    = make_trampoline(ioc, 4);

  trampoline<io_context::executor_type> s2 = s1;
  BOOST_ASIO_CHECK(s1 == s2);
  BOOST_ASIO_CHECK(!(s1 != s2));
  BOOST_ASIO_CHECK(s1.get_inner_executor() == ioc.get_executor());

  BOOST_ASIO_CHECK(make_trampoline(ioc, 2) != make_trampoline(ioc, 2));
  BOOST_ASIO_CHECK(!(make_trampoline(ioc, 2) == make_trampoline(ioc, 2)));
  BOOST_ASIO_CHECK(make_trampoline(ioc, 2) != make_trampoline(ioc, 3));
}

void trampoline_call_count_carried_test()
{
  io_context ioc;

  BOOST_ASIO_CHECK(make_trampoline(ioc, 0).max_trampolined_calls() == 0);
  BOOST_ASIO_CHECK(make_trampoline(ioc, 1).max_trampolined_calls() == 1);
  BOOST_ASIO_CHECK(make_trampoline(ioc, 2).max_trampolined_calls() == 2);
  BOOST_ASIO_CHECK(make_trampoline(ioc, 7).max_trampolined_calls() == 7);

  BOOST_ASIO_CHECK(
      make_trampoline(ioc.get_executor(), 2).max_trampolined_calls() == 2);
  BOOST_ASIO_CHECK(
      make_trampoline(ioc.get_executor(), 7).max_trampolined_calls() == 7);

  trampoline<io_context::executor_type> c2(ioc.get_executor(), 2);
  trampoline<io_context::executor_type> c7(ioc.get_executor(), 7);
  BOOST_ASIO_CHECK(c2.max_trampolined_calls() == 2);
  BOOST_ASIO_CHECK(c7.max_trampolined_calls() == 7);

  std::size_t n = 7;
  BOOST_ASIO_CHECK(make_trampoline(ioc, n).max_trampolined_calls() == 7);

  BOOST_ASIO_CHECK(boost::asio::require(c7,
        boost::asio::execution::blocking.never)
      .max_trampolined_calls() == 7);
  BOOST_ASIO_CHECK(boost::asio::require(c7,
        boost::asio::execution::outstanding_work.tracked)
      .max_trampolined_calls() == 7);
}

void trampoline_execute_test()
{
  io_context ioc;
  int count = 0;

  trampoline<io_context::executor_type> s
    = make_trampoline(ioc, 4);

  s.execute(bindns::bind(increment, &count));
  BOOST_ASIO_CHECK(count == 0);
  ioc.run();
  BOOST_ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  boost::asio::require(s, boost::asio::execution::blocking.never).execute(
      bindns::bind(increment, &count));
  BOOST_ASIO_CHECK(!ioc.stopped());
  BOOST_ASIO_CHECK(count == 0);
  ioc.run();
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(count == 1);

  std::string log;
  ioc.restart();
  boost::asio::post(ioc,
      [&]
      {
        s.execute(
            [&]
            {
              log += "A ";
              s.execute([&] { log += "B"; });
            });
      });
  ioc.run();
  BOOST_ASIO_CHECK(log == "A B");

  count = 0;
  ioc.restart();
  boost::asio::require(s,
      boost::asio::execution::blocking.never,
      boost::asio::execution::outstanding_work.tracked
    ).execute(bindns::bind(increment, &count));
  BOOST_ASIO_CHECK(!ioc.stopped());
  BOOST_ASIO_CHECK(count == 0);
  ioc.run();
  BOOST_ASIO_CHECK(ioc.stopped());
  BOOST_ASIO_CHECK(count == 1);
}

void trampoline_call_count_test()
{
  io_context ioc;
  std::string log;

  auto s = make_trampoline(ioc, 2);
  s.execute(
      [&]
      {
        log += "A ";
        s.execute([&] { log += "B "; });
        s.execute([&] { log += "C "; });
        s.execute([&] { log += "D "; });
      });
  BOOST_ASIO_CHECK(log == "");
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A B C ");
  ioc.run();
  BOOST_ASIO_CHECK(log == "A B C D ");

  log.clear();
  ioc.restart();
  boost::asio::post(ioc,
      [&]
      {
        s.execute(
            [&]
            {
              log += "A ";
              s.execute([&] { log += "B "; });
              s.execute([&] { log += "C "; });
            });
        s.execute(
            [&]
            {
              log += "D ";
              s.execute([&] { log += "E "; });
              s.execute([&] { log += "F "; });
            });
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A B C D E F ");
  BOOST_ASIO_CHECK(ioc.poll() == 0);
}

void trampoline_zero_and_one_test()
{
  io_context ioc;
  std::string log;

  auto z = make_trampoline(ioc, 0);
  z.execute(
      [&]
      {
        log += "A ";
        z.execute([&] { log += "B "; });
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A ");
  ioc.run();
  BOOST_ASIO_CHECK(log == "A B ");

  log.clear();
  ioc.restart();
  auto o = make_trampoline(ioc, 1);
  o.execute(
      [&]
      {
        log += "A ";
        o.execute([&] { log += "B "; });
        o.execute([&] { log += "C "; });
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A B ");
  ioc.run();
  BOOST_ASIO_CHECK(log == "A B C ");
}

void trampoline_unbounded_test()
{
  io_context ioc;
  std::string log;

  auto s = make_trampoline(ioc, (std::numeric_limits<std::size_t>::max)());
  BOOST_ASIO_CHECK(s.max_trampolined_calls()
      == (std::numeric_limits<std::size_t>::max)());

  s.execute(
      [&]
      {
        log += "A ";
        s.execute([&] { log += "B "; });
        s.execute([&] { log += "C "; });
        s.execute([&] { log += "D "; });
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A B C D ");
  BOOST_ASIO_CHECK(ioc.poll() == 0);
}

void trampoline_identity_match_test()
{
  io_context ioc;
  std::string log;

  auto v = make_trampoline(ioc, 8);
  auto n = boost::asio::require(v, boost::asio::execution::blocking.never);

  v.execute(
      [&]
      {
        log += "A( ";
        v.execute([&] { log += "B "; });
        n.execute([&] { log += "C "; });
        log += ") ";
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A( ) B C ");
  BOOST_ASIO_CHECK(ioc.poll() == 0);

  log.clear();
  ioc.restart();
  auto w = make_trampoline(ioc, 3);
  v.execute(
      [&]
      {
        log += "A( ";
        w.execute([&] { log += "B "; });
        log += ") ";
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A( ) ");
  BOOST_ASIO_CHECK(ioc.poll() == 1);
  BOOST_ASIO_CHECK(log == "A( ) B ");
  BOOST_ASIO_CHECK(ioc.poll() == 0);
}

void trampoline_identity_mismatch_test()
{
  io_context ioc1;
  io_context ioc2;
  std::string log;

  auto v1 = make_trampoline(ioc1, 8);
  auto v2 = make_trampoline(ioc2, 8);

  post(ioc1,
      [&]
      {
        v1.execute(
            [&]
            {
              log += "A( ";
              v2.execute([&] { log += "x "; });
              v1.execute([&] { log += "B "; });
              log += ") ";
            });
      });
  BOOST_ASIO_CHECK(ioc1.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A( ) B ");
  BOOST_ASIO_CHECK(ioc2.poll() == 1);
  BOOST_ASIO_CHECK(log == "A( ) B x ");
}

void trampoline_executor_function_test()
{
  io_context ioc;
  std::string log;

  {
    auto v = make_trampoline(ioc, 8);
    any_io_executor erased(v);
    bool called = false;
    post(erased, [&]{ called = true; });
    BOOST_ASIO_CHECK(!called);
    BOOST_ASIO_CHECK(ioc.poll_one() == 1);
    BOOST_ASIO_CHECK(called);
    ioc.restart();
  }

  {
    log.clear();
    auto v = make_trampoline(ioc, 8);
    any_io_executor erased(v);
    v.execute(
        [&]
        {
          log += "A( ";
          post(erased, [&] { log += "B "; });
          log += ") ";
        });
    BOOST_ASIO_CHECK(ioc.poll_one() == 1);
    BOOST_ASIO_CHECK(log == "A( ) B ");
    BOOST_ASIO_CHECK(ioc.poll() == 0);
    ioc.restart();
  }

  {
    log.clear();
    auto v1 = make_trampoline(ioc, 8);
    auto v2 = make_trampoline(ioc, 8);
    any_io_executor erased2(v2);

    v1.execute(
        [&]
        {
          log += "A( ";
          post(erased2, [&] { log += "x "; });
          log += ") ";
        });
    BOOST_ASIO_CHECK(ioc.poll_one() == 1);
    BOOST_ASIO_CHECK(log == "A( ) ");
    BOOST_ASIO_CHECK(ioc.poll() == 1);
    BOOST_ASIO_CHECK(log == "A( ) x ");
    ioc.restart();
  }
}

void trampoline_carries_test()
{
  io_context ioc;
  std::string log;

  auto v = make_trampoline(ioc, 1);
  v.execute(
      [&]
      {
        log += "A ";
        v.execute([&] { log += "B "; });
        v.execute(
            [&]
            {
              log += "C( ";
              v.execute([&] { log += "D "; });
              log += ") ";
            });
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A B ");
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A B C( ) D ");
  BOOST_ASIO_CHECK(ioc.poll() == 0);
}

void trampoline_inline_depth_default_test()
{
  io_context ioc;
  std::string log;

  auto v = make_trampoline(ioc, 8);
  BOOST_ASIO_CHECK(v.max_inline_depth() == 0);

  v.execute(
      [&]
      {
        log += "A( ";
        v.execute([&] { log += "B "; });
        log += ") ";
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A( ) B ");
  BOOST_ASIO_CHECK(ioc.poll() == 0);
}

void trampoline_inline_depth_test()
{
  io_context ioc;
  std::string log;

  auto v = make_trampoline(ioc, 8, 2);
  auto n = boost::asio::require(v, boost::asio::execution::blocking.never);
  BOOST_ASIO_CHECK(v.max_inline_depth() == 2);

  v.execute(
      [&]
      {
        log += "A( ";
        v.execute([&] { log += "B "; });
        n.execute([&] { log += "C "; });
        log += ") ";
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A( B ) C ");
  BOOST_ASIO_CHECK(ioc.poll() == 0);
}

void trampoline_inline_depth_limit_test()
{
  io_context ioc;
  std::string log;

  auto v = make_trampoline(ioc, 8, 1);
  v.execute(
      [&]
      {
        log += "A( ";
        v.execute(
            [&]
            {
              log += "B( ";
              v.execute([&] { log += "D "; });
              log += ") ";
            });
        log += ") ";
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A( B( ) ) D ");
  BOOST_ASIO_CHECK(ioc.poll() == 0);
}

void trampoline_inline_depth_budget_test()
{
  io_context ioc;
  std::string log;

  auto v = make_trampoline(ioc, 1, 8);
  v.execute(
      [&]
      {
        log += "A( ";
        v.execute([&] { log += "B "; });
        v.execute([&] { log += "C "; });
        log += ") ";
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A( B ) ");
  BOOST_ASIO_CHECK(ioc.poll() == 1);
  BOOST_ASIO_CHECK(log == "A( B ) C ");
  BOOST_ASIO_CHECK(ioc.poll() == 0);
}

struct trampoline_inline_depth_counter
{
  int* shared_calls;
  int own_calls;

  void operator()()
  {
    ++*shared_calls;
    ++own_calls;
  }
};

void trampoline_inline_depth_copy_test()
{
  io_context ioc;
  int calls = 0;

  auto v = make_trampoline(ioc, 8, 2);
  trampoline_inline_depth_counter counter = { &calls, 0 };
  v.execute(
      [&]
      {
        v.execute(counter);
        BOOST_ASIO_CHECK(calls == 1);
        BOOST_ASIO_CHECK(counter.own_calls == 0);

        v.execute(trampoline_inline_depth_counter{ &calls, 0 });
        BOOST_ASIO_CHECK(calls == 2);
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(calls == 2);
  BOOST_ASIO_CHECK(counter.own_calls == 0);
  BOOST_ASIO_CHECK(ioc.poll() == 0);
}

void trampoline_inline_depth_exception_test()
{
  io_context ioc;
  std::string log;

  auto v = make_trampoline(ioc, 8, 1);
  v.execute(
      [&]
      {
        log += "A( ";
        try
        {
          v.execute(
              [&]
              {
                log += "B ";
                throw std::runtime_error("x");
              });
        }
        catch (...)
        {
          log += "caught ";
        }
        v.execute([&] { log += "C "; });
        log += ") ";
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A( B caught C ) ");
  BOOST_ASIO_CHECK(ioc.poll() == 0);
}

void trampoline_inline_depth_carried_test()
{
  io_context ioc;

  BOOST_ASIO_CHECK(make_trampoline(ioc, 4).max_inline_depth() == 0);
  BOOST_ASIO_CHECK(make_trampoline(ioc, 4, 0).max_inline_depth() == 0);
  BOOST_ASIO_CHECK(make_trampoline(ioc, 4, 3).max_inline_depth() == 3);
  BOOST_ASIO_CHECK(make_trampoline(ioc, 4, 8).max_inline_depth() == 8);
  BOOST_ASIO_CHECK(make_trampoline(ioc, 4, 255).max_inline_depth() == 255);

#if !defined(BOOST_ASIO_NO_EXCEPTIONS)
  bool out_of_range_thrown = false;
  try
  {
    make_trampoline(ioc, 4, 256);
  }
  catch (const std::out_of_range&)
  {
    out_of_range_thrown = true;
  }
  BOOST_ASIO_CHECK(out_of_range_thrown);
#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)

  BOOST_ASIO_CHECK(
      make_trampoline(ioc.get_executor(), 4, 3).max_inline_depth() == 3);

  trampoline<io_context::executor_type> c3(ioc.get_executor(), 4, 3);
  BOOST_ASIO_CHECK(c3.max_inline_depth() == 3);
  BOOST_ASIO_CHECK(c3.max_trampolined_calls() == 4);

  trampoline<io_context::executor_type> c3_copy = c3;
  BOOST_ASIO_CHECK(c3_copy.max_inline_depth() == 3);
  BOOST_ASIO_CHECK(c3 == c3_copy);

  trampoline<any_io_executor> c3_erased(c3);
  BOOST_ASIO_CHECK(c3_erased.max_inline_depth() == 3);
  BOOST_ASIO_CHECK(c3_erased.get_inner_executor() == c3.get_inner_executor());

  BOOST_ASIO_CHECK(boost::asio::require(c3,
        boost::asio::execution::blocking.never).max_inline_depth() == 3);
  BOOST_ASIO_CHECK(boost::asio::require(c3,
        boost::asio::execution::outstanding_work.tracked).max_inline_depth() == 3);
  BOOST_ASIO_CHECK(boost::asio::prefer(c3,
        boost::asio::execution::relationship.continuation).max_inline_depth() == 3);

  BOOST_ASIO_CHECK(make_trampoline(ioc, 4, 3) != make_trampoline(ioc, 4, 3));
}

template <typename T>
struct marker_allocator
{
  typedef T value_type;

  marker_allocator()
  {
  }

  template <typename U>
  marker_allocator(const marker_allocator<U>&)
  {
  }

  T* allocate(std::size_t n)
  {
    return static_cast<T*>(::operator new(sizeof(T) * n));
  }

  void deallocate(T* p, std::size_t)
  {
    ::operator delete(p);
  }

  template <typename U>
  bool operator==(const marker_allocator<U>&) const
  {
    return true;
  }

  template <typename U>
  bool operator!=(const marker_allocator<U>&) const
  {
    return false;
  }
};

template <typename Installer, typename Probe>
bool joins_frame(io_context& ioc,
    const Installer& installer, const Probe& probe)
{
  std::string log;

  installer.execute(
      [&]
      {
        log += "A( ";
        boost::asio::post(ioc, [&] { log += "P "; });
        probe.execute([&] { log += "B "; });
        log += ") ";
      });

  ioc.restart();
  while (ioc.poll() > 0) {}

  return log == "A( ) B P ";
}

void trampoline_identity_property_test()
{
  namespace ex = boost::asio::execution;

  io_context ioc;
  auto v = make_trampoline(ioc, 8);

  BOOST_ASIO_CHECK(joins_frame(ioc, v, v));

  BOOST_ASIO_CHECK(joins_frame(ioc, v,
        boost::asio::require(v, ex::blocking.never)));
  BOOST_ASIO_CHECK(joins_frame(ioc,
        boost::asio::require(v, ex::blocking.never), v));
  BOOST_ASIO_CHECK(joins_frame(ioc, v,
        boost::asio::require(v, ex::blocking.possibly)));

  BOOST_ASIO_CHECK(joins_frame(ioc, v,
        boost::asio::prefer(v, ex::relationship.continuation)));
  BOOST_ASIO_CHECK(joins_frame(ioc,
        boost::asio::prefer(v, ex::relationship.continuation), v));
  BOOST_ASIO_CHECK(joins_frame(ioc, v,
        boost::asio::prefer(v, ex::relationship.fork)));

  BOOST_ASIO_CHECK(joins_frame(ioc, v,
        boost::asio::require(v, ex::outstanding_work.tracked)));
  BOOST_ASIO_CHECK(joins_frame(ioc,
        boost::asio::require(v, ex::outstanding_work.tracked), v));
  BOOST_ASIO_CHECK(joins_frame(ioc, v,
        boost::asio::require(v, ex::outstanding_work.untracked)));

  BOOST_ASIO_CHECK(joins_frame(ioc, v,
        boost::asio::require(v, ex::allocator(marker_allocator<void>()))));
  BOOST_ASIO_CHECK(joins_frame(ioc,
        boost::asio::require(v, ex::allocator(marker_allocator<void>())), v));
  BOOST_ASIO_CHECK(joins_frame(ioc, v,
        boost::asio::require(v, ex::allocator)));

  trampoline<any_io_executor> p(
      any_io_executor(ioc.get_executor()), 8);

  BOOST_ASIO_CHECK(joins_frame(ioc, p, p));
  BOOST_ASIO_CHECK(joins_frame(ioc, p,
        boost::asio::prefer(p, ex::relationship.continuation)));
  BOOST_ASIO_CHECK(joins_frame(ioc,
        boost::asio::prefer(p, ex::relationship.continuation), p));
  BOOST_ASIO_CHECK(joins_frame(ioc, p,
        boost::asio::prefer(p, ex::outstanding_work.tracked)));
  BOOST_ASIO_CHECK(joins_frame(ioc,
        boost::asio::prefer(p, ex::outstanding_work.tracked), p));
  BOOST_ASIO_CHECK(joins_frame(ioc, p,
        boost::asio::require(p, ex::blocking.never)));

  io_context other;

  BOOST_ASIO_CHECK(!joins_frame(ioc, v, make_trampoline(other, 8)));
  BOOST_ASIO_CHECK(!joins_frame(ioc, v, make_trampoline(ioc, 8)));
  BOOST_ASIO_CHECK(!joins_frame(ioc, make_trampoline(ioc, 8), v));
  BOOST_ASIO_CHECK(!joins_frame(ioc, v, make_trampoline(ioc, 3)));
  BOOST_ASIO_CHECK(!joins_frame(ioc, make_trampoline(ioc, 3), v));

  trampoline<io_context::executor_type> v_copy = v;
  BOOST_ASIO_CHECK(joins_frame(ioc, v, v_copy));
  BOOST_ASIO_CHECK(joins_frame(ioc, v_copy, v));

  trampoline<any_io_executor> v_erased(v);
  BOOST_ASIO_CHECK(joins_frame(ioc, v, v_erased));
  BOOST_ASIO_CHECK(joins_frame(ioc, v_erased, v));

  auto outer = make_trampoline(v, 8);
  BOOST_ASIO_CHECK(outer.get_inner_executor() == v);
}

void trampoline_allocator_forwarding_test()
{
  io_context ioc;

  trampoline<io_context::executor_type> s1
    = make_trampoline(ioc, 4);
  BOOST_ASIO_CHECK(
      boost::asio::query(s1, boost::asio::execution::allocator)
        == std::allocator<void>());

  auto s2 = boost::asio::require(s1,
      boost::asio::execution::allocator(marker_allocator<void>()));
  BOOST_ASIO_CHECK(
      boost::asio::query(s2, boost::asio::execution::allocator)
        == marker_allocator<void>());
  BOOST_ASIO_CHECK(
      boost::asio::query(s2.get_inner_executor(), boost::asio::execution::allocator)
        == marker_allocator<void>());

  auto s3 = boost::asio::require(s2, boost::asio::execution::allocator);
  BOOST_ASIO_CHECK(
      boost::asio::query(s3, boost::asio::execution::allocator)
        == std::allocator<void>());
}

void trampoline_count_from_installer_test()
{
  io_context ioc;
  std::string log;

  auto v = make_trampoline(ioc, 1);
  trampoline<io_context::executor_type> v_copy = v;
  trampoline<any_io_executor> v_erased(v);

  v.execute(
      [&]
      {
        log += "A( ";
        v_copy.execute([&] { log += "B "; });   // 1st extra call: admitted
        v_erased.execute([&] { log += "C "; }); // 2nd extra call: over budget
        log += ") ";
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A( ) B ");

  BOOST_ASIO_CHECK(ioc.poll() == 1);
  BOOST_ASIO_CHECK(log == "A( ) B C ");
}

void trampoline_running_in_this_thread_test()
{
  io_context ioc;

  auto v = make_trampoline(ioc, 8);
  trampoline<io_context::executor_type> v_copy = v;
  auto other = make_trampoline(ioc, 8); // Different trampoline ID.

  BOOST_ASIO_CHECK(!v.running_in_this_thread());
  BOOST_ASIO_CHECK(!v_copy.running_in_this_thread());
  BOOST_ASIO_CHECK(!other.running_in_this_thread());

  bool checked_in_installer = false;
  v.execute(
      [&]
      {
        BOOST_ASIO_CHECK(v.running_in_this_thread());
        BOOST_ASIO_CHECK(v_copy.running_in_this_thread());
        BOOST_ASIO_CHECK(!other.running_in_this_thread());
        checked_in_installer = true;
      });
  ioc.poll_one();
  BOOST_ASIO_CHECK(checked_in_installer);
  BOOST_ASIO_CHECK(!v.running_in_this_thread());

  ioc.restart();
  bool checked_in_drain = false;
  v.execute(
      [&]
      {
        v.execute(
            [&]
            {
              BOOST_ASIO_CHECK(v.running_in_this_thread());
              BOOST_ASIO_CHECK(v_copy.running_in_this_thread());
              checked_in_drain = true;
            });
      });
  ioc.poll_one();
  BOOST_ASIO_CHECK(checked_in_drain);
  BOOST_ASIO_CHECK(!v.running_in_this_thread());
}

class test_function
{
public:
  test_function() = default;

  template <typename Function,
      typename = enable_if_t<
        !is_same<decay_t<Function>, test_function>::value>>
  test_function(Function&& f)
    : impl_(new holder<decay_t<Function>>(static_cast<Function&&>(f)))
  {
  }

  explicit operator bool() const noexcept
  {
    return impl_.get() != 0;
  }

  void operator()()
  {
    impl_->call();
  }

private:
  struct base
  {
    virtual ~base()
    {
    }

    virtual void call() = 0;
  };

  template <typename Function>
  struct holder : base
  {
    template <typename F>
    explicit holder(F&& f)
      : function_(static_cast<F&&>(f))
    {
    }

    void call()
    {
      function_();
    }

    Function function_;
  };

  std::unique_ptr<base> impl_;
};

struct probe_state
{
  bool saw_marker = false;
  test_function pending;
};

class probe_executor
{
public:
  probe_executor(io_context& ioc, probe_state* s)
    : context_(&ioc),
      state_(s)
  {
  }

  execution_context& query(execution::context_t) const noexcept
  {
    return *context_;
  }

  static constexpr execution::blocking_t::never_t query(
      execution::blocking_t) noexcept
  {
    return execution::blocking_t::never_t();
  }

  std::allocator<void> query(execution::allocator_t<void>) const noexcept
  {
    return std::allocator<void>();
  }

  template <typename Function>
  void execute(Function&& f) const
  {
    state_->saw_marker = boost::asio::is_same<
        boost::asio::associated_allocator_t<boost::asio::decay_t<Function>>,
        marker_allocator<void>>::value;
    state_->pending = test_function(static_cast<Function&&>(f));
  }

  friend bool operator==(const probe_executor& a,
      const probe_executor& b) noexcept
  {
    return a.state_ == b.state_;
  }

  friend bool operator!=(const probe_executor& a,
      const probe_executor& b) noexcept
  {
    return a.state_ != b.state_;
  }

private:
  io_context* context_;
  probe_state* state_;
};

struct relay_target
{
  std::function<void(test_function)> submit;
};

class relay_executor
{
public:
  relay_executor(io_context& ioc, relay_target* t)
    : context_(&ioc),
      target_(t)
  {
  }

  execution_context& query(execution::context_t) const noexcept
  {
    return *context_;
  }

  static constexpr execution::blocking_t::never_t query(
      execution::blocking_t) noexcept
  {
    return execution::blocking_t::never_t();
  }

  std::allocator<void> query(execution::allocator_t<void>) const noexcept
  {
    return std::allocator<void>();
  }

  template <typename Function>
  void execute(Function&& f) const
  {
    target_->submit(test_function(static_cast<Function&&>(f)));
  }

  friend bool operator==(const relay_executor& a,
      const relay_executor& b) noexcept
  {
    return a.target_ == b.target_;
  }

  friend bool operator!=(const relay_executor& a,
      const relay_executor& b) noexcept
  {
    return a.target_ != b.target_;
  }

private:
  io_context* context_;
  relay_target* target_;
};

struct ts_state
{
  std::string log;
  test_function pending;
};

class ts_executor
{
public:
  ts_executor(io_context& ioc, ts_state* s)
    : context_(&ioc),
      state_(s)
  {
  }

  execution_context& context() const noexcept
  {
    return *context_;
  }

  void on_work_started() const noexcept
  {
  }

  void on_work_finished() const noexcept
  {
  }

  template <typename Function, typename Allocator>
  void dispatch(Function&& f, const Allocator&) const
  {
    state_->log += "dispatch ";
    decay_t<Function> g(static_cast<Function&&>(f));
    g();
  }

  template <typename Function, typename Allocator>
  void post(Function&& f, const Allocator&) const
  {
    state_->log += "post ";
    state_->pending = test_function(static_cast<Function&&>(f));
  }

  template <typename Function, typename Allocator>
  void defer(Function&& f, const Allocator&) const
  {
    state_->log += "defer ";
    state_->pending = test_function(static_cast<Function&&>(f));
  }

  friend bool operator==(const ts_executor& a,
      const ts_executor& b) noexcept
  {
    return a.state_ == b.state_;
  }

  friend bool operator!=(const ts_executor& a,
      const ts_executor& b) noexcept
  {
    return a.state_ != b.state_;
  }

private:
  io_context* context_;
  ts_state* state_;
};

namespace boost {
namespace asio {
template <>
struct is_executor<ts_executor> : boost::asio::true_type
{
};

} // namespace asio
} // namespace boost

#if !defined(BOOST_ASIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

namespace boost {
namespace asio {
namespace execution {
template <>
struct is_executor<probe_executor> : boost::asio::true_type
{
};

template <>
struct is_executor<relay_executor> : boost::asio::true_type
{
};

} // namespace execution
} // namespace asio
} // namespace boost

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

namespace boost {
namespace asio {
namespace traits {
#if !defined(BOOST_ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

template <>
struct equality_comparable<probe_executor>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
};

template <>
struct equality_comparable<ts_executor>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
};

template <>
struct equality_comparable<relay_executor>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

#if !defined(BOOST_ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

template <typename Function>
struct execute_member<probe_executor, Function>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = false;
  typedef void result_type;
};

template <typename Function>
struct execute_member<relay_executor, Function>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = false;
  typedef void result_type;
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

#if !defined(BOOST_ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

template <typename Param>
struct query_static_constexpr_member<probe_executor, Param,
    typename boost::asio::enable_if<
      boost::asio::is_convertible<Param,
        boost::asio::execution::blocking_t>::value
    >::type>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
  typedef boost::asio::execution::blocking_t::never_t result_type;

  static constexpr result_type value() noexcept
  {
    return result_type();
  }
};

template <typename Param>
struct query_static_constexpr_member<relay_executor, Param,
    typename boost::asio::enable_if<
      boost::asio::is_convertible<Param,
        boost::asio::execution::blocking_t>::value
    >::type>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
  typedef boost::asio::execution::blocking_t::never_t result_type;

  static constexpr result_type value() noexcept
  {
    return result_type();
  }
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

#if !defined(BOOST_ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

template <>
struct query_member<probe_executor, boost::asio::execution::context_t>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
  typedef boost::asio::execution_context& result_type;
};

template <>
struct query_member<probe_executor,
    boost::asio::execution::allocator_t<void>>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
  typedef std::allocator<void> result_type;
};

template <>
struct query_member<relay_executor, boost::asio::execution::context_t>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
  typedef boost::asio::execution_context& result_type;
};

template <>
struct query_member<relay_executor,
    boost::asio::execution::allocator_t<void>>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
  typedef std::allocator<void> result_type;
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

} // namespace traits
} // namespace asio
} // namespace boost

void trampoline_association_test()
{
  io_context ioc;
  probe_state st;
  probe_executor probe(ioc, &st);
  int count = 0;

  auto v = make_trampoline(probe, 4);
  v.execute(
      boost::asio::bind_allocator(marker_allocator<void>(),
        bindns::bind(increment, &count)));
  BOOST_ASIO_CHECK(st.saw_marker);
  BOOST_ASIO_CHECK(count == 0);
  st.pending();
  BOOST_ASIO_CHECK(count == 1);

  st = probe_state();
  count = 0;
  auto z = make_trampoline(probe, 0);
  z.execute(
      [&]
      {
        z.execute(
            boost::asio::bind_allocator(marker_allocator<void>(),
              bindns::bind(increment, &count)));
      });
  BOOST_ASIO_CHECK(st.pending);
  st.pending();
  BOOST_ASIO_CHECK(st.saw_marker);
  BOOST_ASIO_CHECK(count == 0);
  st.pending();
  BOOST_ASIO_CHECK(count == 1);
}

void trampoline_bypass_livelock_test()
{
  io_context ioc;
  std::string log;

  relay_target target;
  relay_executor relay(ioc, &target);
  auto a = make_trampoline(relay, 1);
  target.submit = [&a](test_function f) { a.execute(std::move(f)); };

  a.execute(
      [&]
      {
        log += "A ";
        a.execute([&] { log += "B "; });
        a.execute([&] { log += "C "; });
      });
  BOOST_ASIO_CHECK(log == "A C B ");
}

void trampoline_ts_executor_test()
{
  io_context ioc;
  ts_state st;
  ts_executor ts(ioc, &st);
  std::string log;

  trampoline<ts_executor> s(ts, 4);
  BOOST_ASIO_CHECK(s.get_inner_executor() == ts);
  BOOST_ASIO_CHECK(&s.context() == &ioc);
  BOOST_ASIO_CHECK(s.max_trampolined_calls() == 4);
  s.on_work_started();
  s.on_work_finished();

  s.dispatch(
      [&]
      {
        log += "A( ";
        s.dispatch([&] { log += "B "; }, std::allocator<void>());
        log += ") ";
      }, std::allocator<void>());
  BOOST_ASIO_CHECK(st.log == "dispatch ");
  BOOST_ASIO_CHECK(log == "A( ) B ");
  BOOST_ASIO_CHECK(!st.pending);

  st.log.clear();
  log.clear();
  s.dispatch(
      [&]
      {
        log += "A( ";
        s.dispatch([&] { log += "b "; }, std::allocator<void>());
        s.post([&] { log += "p "; }, std::allocator<void>());
        s.defer([&] { log += "d "; }, std::allocator<void>());
        log += ") ";
      }, std::allocator<void>());
  BOOST_ASIO_CHECK(st.log == "dispatch ");
  BOOST_ASIO_CHECK(log == "A( ) b p d ");
  BOOST_ASIO_CHECK(!st.pending);

  trampoline<ts_executor> z(ts, 0);

  st.log.clear();
  log.clear();
  z.dispatch(
      [&]
      {
        log += "A( ";
        z.dispatch([&] { log += "D "; }, std::allocator<void>());
        log += ") ";
      }, std::allocator<void>());
  BOOST_ASIO_CHECK(st.log == "dispatch dispatch ");
  BOOST_ASIO_CHECK(log == "A( D ) ");
  BOOST_ASIO_CHECK(!st.pending);

  st.log.clear();
  log.clear();
  z.dispatch(
      [&]
      {
        z.post([&] { log += "P "; }, std::allocator<void>());
      }, std::allocator<void>());
  BOOST_ASIO_CHECK(st.log == "dispatch post ");
  BOOST_ASIO_CHECK(log.empty());
  BOOST_ASIO_CHECK(st.pending);
  st.pending();
  BOOST_ASIO_CHECK(log == "P ");

  st.log.clear();
  st.pending = test_function();
  log.clear();
  z.dispatch(
      [&]
      {
        z.defer([&] { log += "F "; }, std::allocator<void>());
      }, std::allocator<void>());
  BOOST_ASIO_CHECK(st.log == "dispatch defer ");
  BOOST_ASIO_CHECK(log.empty());
  BOOST_ASIO_CHECK(st.pending);
  st.pending();
  BOOST_ASIO_CHECK(log == "F ");

  ts_state other_st;
  ts_executor other(ioc, &other_st);
  trampoline<ts_executor> t(other, 4);
  BOOST_ASIO_CHECK(s != t);

  st.log.clear();
  st.pending = test_function();
  other_st.log.clear();
  log.clear();
  s.dispatch(
      [&]
      {
        log += "A( ";
        t.dispatch([&] { log += "x "; }, std::allocator<void>());
        t.post([&] { log += "y "; }, std::allocator<void>());
        log += ") ";
      }, std::allocator<void>());
  BOOST_ASIO_CHECK(st.log == "dispatch ");
  BOOST_ASIO_CHECK(other_st.log == "dispatch post ");
  BOOST_ASIO_CHECK(log == "A( x ) ");
  BOOST_ASIO_CHECK(other_st.pending);
  other_st.pending();
  BOOST_ASIO_CHECK(log == "A( x ) y ");

  st.log.clear();
  st.pending = test_function();
  log.clear();
  s.post([&] { log += "P "; }, std::allocator<void>());
  BOOST_ASIO_CHECK(st.log == "post ");
  BOOST_ASIO_CHECK(log.empty());
  BOOST_ASIO_CHECK(st.pending);
  st.pending();
  BOOST_ASIO_CHECK(log == "P ");

  st.log.clear();
  st.pending = test_function();
  log.clear();
  s.defer([&] { log += "F "; }, std::allocator<void>());
  BOOST_ASIO_CHECK(st.log == "defer ");
  BOOST_ASIO_CHECK(log.empty());
  BOOST_ASIO_CHECK(st.pending);
  st.pending();
  BOOST_ASIO_CHECK(log == "F ");
}

struct counting_function
{
  counting_function(int* m, std::string* l)
    : moves_(m), log_(l)
  {
  }

  counting_function(const counting_function&) = delete;

  counting_function(counting_function&& o) noexcept
    : moves_(o.moves_), log_(o.log_)
  {
    ++*moves_;
  }

  void operator()()
  {
    *log_ += "F ";
  }

  int* moves_;
  std::string* log_;
};

struct throwing_ex
{
  static constexpr execution::blocking_t query(execution::blocking_t) noexcept
  {
    return execution::blocking_t::always_t();
  }

  template <typename P>
  throwing_ex require(P) const noexcept
  {
    return *this;
  }

  template <typename F>
  void execute(F&&) const
  {
    throw std::runtime_error("x");
  }

  friend bool operator==(throwing_ex, throwing_ex) noexcept
  {
    return true;
  }

  friend bool operator!=(throwing_ex, throwing_ex) noexcept
  {
    return false;
  }
};

#if !defined(BOOST_ASIO_HAS_WORKING_EXPRESSION_SFINAE)

namespace boost {
namespace asio {
BOOST_ASIO_INLINE_NAMESPACE_BEGIN
namespace traits {

template <typename F>
struct execute_member<throwing_ex, F>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = false;
  typedef void result_type;
};

template <typename P>
struct require_member<throwing_ex, P>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
  typedef throwing_ex result_type;
};

template <typename P>
struct query_static_constexpr_member<throwing_ex, P,
    enable_if_t<is_convertible<P, execution::blocking_t>::value>>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
  typedef execution::blocking_t result_type;

  static constexpr result_type value() noexcept
  {
    return execution::blocking_t::always_t();
  }
};

template <>
struct equality_comparable<throwing_ex>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
};

} // namespace traits
BOOST_ASIO_INLINE_NAMESPACE_END
} // namespace asio
} // namespace boost

#endif // !defined(BOOST_ASIO_HAS_WORKING_EXPRESSION_SFINAE)

void trampoline_fast_path_inline_or_test()
{
  io_context ioc;
  int moves = 0;
  std::string log;
  auto v = make_trampoline(inline_or(ioc), 4);
  v.execute(
      [&]
      {
        log += "A ";
        v.execute([&] { log += "B "; });
        v.execute([&] { log += "C "; });
      });
  BOOST_ASIO_CHECK(log == "A B C " && ioc.poll() == 0);
  v.execute(counting_function(&moves, &log));
  BOOST_ASIO_CHECK(moves == 0);
}

void trampoline_fast_path_thread_pool_test()
{
  thread_pool pool(1);
  io_context ioc;
  int moves = 0;
  std::string log;
  auto always_ex = boost::asio::require(
      pool.get_executor(), execution::blocking.always);
  make_trampoline(always_ex, 4).execute(counting_function(&moves, &log));
  BOOST_ASIO_CHECK(log == "F " && moves == 0);
  log.clear(); moves = 0;
  make_trampoline(pool.get_executor(), 4).execute(
      counting_function(&moves, &log));
  pool.join();
  BOOST_ASIO_CHECK(log == "F " && moves >= 1);
  moves = 0;
  make_trampoline(ioc.get_executor(), 4).execute(
      counting_function(&moves, &log));
  ioc.run();
  BOOST_ASIO_CHECK(moves >= 1);
}

void trampoline_fast_path_exception_test()
{
  trampoline<throwing_ex> v(throwing_ex(), 4);
  bool caught = false;
  try
  {
    v.execute([] {});
  }
  catch (const std::runtime_error&)
  {
    caught = true;
  }
  BOOST_ASIO_CHECK(caught && boost::asio::detail::trampoline_frame::current() == 0);
}

void trampoline_exception_policy_query_test()
{
  io_context ioc;
  auto v = make_trampoline(ioc, 4, 2);

  BOOST_ASIO_CHECK(boost::asio::query(v, execution::inline_exception_handling)
      == execution::inline_exception_handling.propagate);

  auto t = boost::asio::require(v, execution::inline_exception_handling.terminate);
  BOOST_ASIO_CHECK(boost::asio::query(t, execution::inline_exception_handling)
      == execution::inline_exception_handling.terminate);

  BOOST_ASIO_CHECK(t.max_trampolined_calls() == 4);
  BOOST_ASIO_CHECK(t.max_inline_depth() == 2);
  BOOST_ASIO_CHECK(t.get_inner_executor() == ioc.get_executor());

  auto p = boost::asio::require(t, execution::inline_exception_handling.propagate);
  BOOST_ASIO_CHECK(boost::asio::query(p, execution::inline_exception_handling)
      == execution::inline_exception_handling.propagate);
  BOOST_ASIO_CHECK(p == v);

  auto n = boost::asio::require(t, execution::blocking.never);
  BOOST_ASIO_CHECK(boost::asio::query(n, execution::blocking) == execution::blocking.never);
  BOOST_ASIO_CHECK(boost::asio::query(n, execution::inline_exception_handling)
      == execution::inline_exception_handling.terminate);

  auto q = boost::asio::prefer(v, execution::inline_exception_handling.terminate);
  BOOST_ASIO_CHECK(boost::asio::query(q, execution::inline_exception_handling)
      == execution::inline_exception_handling.terminate);
}

void trampoline_propagate_drain_test()
{
  io_context ioc;
  std::string log;
  auto v = make_trampoline(ioc, 8);
  bool caught = false;

  v.execute(
      [&]
      {
        log += "A ";
        v.execute([&] { log += "B "; });
        v.execute([&] { log += "C "; });
        throw std::runtime_error("x");
      });

  try
  {
    ioc.poll_one();
  }
  catch (const std::runtime_error&)
  {
    caught = true;
  }

  BOOST_ASIO_CHECK(caught);
  BOOST_ASIO_CHECK(log == "A ");
  BOOST_ASIO_CHECK(boost::asio::detail::trampoline_frame::current() == 0);
  BOOST_ASIO_CHECK(ioc.poll() == 2);
  BOOST_ASIO_CHECK(log == "A B C ");
}

void trampoline_propagate_drain_order_test()
{
  io_context ioc;
  std::string log;
  auto v = make_trampoline(ioc, 8);
  bool caught = false;

  v.execute(
      [&]
      {
        v.execute([&] { log += "B "; });
        v.execute([&] { log += "C "; });
        v.execute([&] { log += "D "; });
        throw std::runtime_error("x");
      });
  boost::asio::post(ioc, [&] { log += "P "; });

  try
  {
    ioc.poll_one();
  }
  catch (const std::runtime_error&)
  {
    caught = true;
  }

  BOOST_ASIO_CHECK(caught);
  BOOST_ASIO_CHECK(ioc.poll() == 4);
  BOOST_ASIO_CHECK(log == "P B C D ");
}

void trampoline_propagate_nested_drain_test()
{
  io_context ioc;
  std::string log;
  auto v = make_trampoline(ioc, 8, 2);
  auto q = boost::asio::require(v, execution::blocking.never);
  bool caught = false;

  v.execute(
      [&]
      {
        log += "A( ";
        q.execute([&] { log += "Q1 "; });
        v.execute(
            [&]
            {
              log += "N( ";
              q.execute([&] { log += "Q2 "; });
              throw std::runtime_error("x");
            });
        log += ") ";
      });

  try
  {
    ioc.poll_one();
  }
  catch (const std::runtime_error&)
  {
    caught = true;
  }

  BOOST_ASIO_CHECK(caught);
  BOOST_ASIO_CHECK(log == "A( N( ");
  BOOST_ASIO_CHECK(ioc.poll() == 2);
  BOOST_ASIO_CHECK(log == "A( N( Q1 Q2 ");
}

void trampoline_propagate_bypass_nesting_test()
{
  io_context ioc;
  std::string log;
  relay_target target;
  relay_executor relay(ioc, &target);
  target.submit = [](test_function f) { f(); };
  bool caught = false;

  auto a = make_trampoline(relay, 1);
  try
  {
    a.execute(
        [&]
        {
          log += "A( ";
          a.execute([&] { log += "B "; });
          a.execute(
              [&]
              {
                log += "C ";
                throw std::runtime_error("x");
              });
          log += ") ";
        });
  }
  catch (const std::runtime_error&)
  {
    caught = true;
  }

  BOOST_ASIO_CHECK(caught);
  BOOST_ASIO_CHECK(log == "A( C B ");
  BOOST_ASIO_CHECK(boost::asio::detail::trampoline_frame::current() == 0);
}

void trampoline_propagate_drain_submission_test()
{
  io_context ioc;
  std::string log;
  relay_target target;
  relay_executor relay(ioc, &target);

  std::vector<test_function> held;
  std::vector<bool> frame_clear_at_submit;
  target.submit =
    [&](test_function f)
    {
      frame_clear_at_submit.push_back(
          boost::asio::detail::trampoline_frame::current() == 0);
      held.push_back(static_cast<test_function&&>(f));
    };

  auto v = make_trampoline(relay, 8);

  v.execute(
      [&]
      {
        log += "A ";
        v.execute([&] { log += "B "; });
        v.execute([&] { log += "C "; });
        v.execute([&] { log += "D "; });
        throw std::runtime_error("x");
      });

  BOOST_ASIO_CHECK(held.size() == 1);
  BOOST_ASIO_CHECK(log.empty());

  test_function installer(static_cast<test_function&&>(held[0]));
  held.clear();
  frame_clear_at_submit.clear();

  bool caught = false;
  try
  {
    installer();
  }
  catch (const std::runtime_error&)
  {
    caught = true;
  }

  BOOST_ASIO_CHECK(caught);

  BOOST_ASIO_CHECK(log == "A ");
  BOOST_ASIO_CHECK(held.size() == 3);

  BOOST_ASIO_CHECK(frame_clear_at_submit.size() == 3);
  BOOST_ASIO_CHECK(frame_clear_at_submit[0]);
  BOOST_ASIO_CHECK(frame_clear_at_submit[1]);
  BOOST_ASIO_CHECK(frame_clear_at_submit[2]);
  BOOST_ASIO_CHECK(boost::asio::detail::trampoline_frame::current() == 0);

  for (std::size_t i = 0; i < held.size(); ++i)
    held[i]();
  BOOST_ASIO_CHECK(log == "A B C D ");
}

void trampoline_propagate_drain_from_operation_test()
{
  io_context ioc;
  std::string log;
  auto v = make_trampoline(ioc, 8);
  any_io_executor any_ex(v);
  bool caught = false;

  boost::asio::post(any_ex,
      [&]
      {
        log += "A ";
        v.execute([&] { log += "B "; });
        v.execute([&] { log += "C "; });
        throw std::runtime_error("x");
      });

  try
  {
    ioc.poll_one();
  }
  catch (const std::runtime_error&)
  {
    caught = true;
  }

  BOOST_ASIO_CHECK(caught);
  BOOST_ASIO_CHECK(log == "A ");
  BOOST_ASIO_CHECK(boost::asio::detail::trampoline_frame::current() == 0);
  BOOST_ASIO_CHECK(ioc.poll() == 2);
  BOOST_ASIO_CHECK(log == "A B C ");
}

void trampoline_propagate_immediate_drain_test()
{
  io_context ioc;
  std::string log;
  relay_target target;
  relay_executor relay(ioc, &target);

  std::vector<test_function> held;
  std::vector<bool> frame_clear_at_submit;
  target.submit =
    [&](test_function f)
    {
      frame_clear_at_submit.push_back(
          boost::asio::detail::trampoline_frame::current() == 0);
      held.push_back(static_cast<test_function&&>(f));
    };

  auto v = make_trampoline(relay, 8, 2);
  auto q = boost::asio::require(v, execution::blocking.never);
  bool caught = false;

  v.execute(
      [&]
      {
        log += "A ";
        q.execute([&] { log += "B "; });
        q.execute([&] { log += "C "; });
        q.execute([&] { log += "D "; });

        try
        {
          v.execute(
              [&]
              {
                log += "I ";
                throw std::runtime_error("x");
              });
        }
        catch (const std::runtime_error&)
        {
          caught = true;
        }

        BOOST_ASIO_CHECK(boost::asio::detail::trampoline_frame::current() != 0);
        q.execute([&] { log += "E "; });
      });

  BOOST_ASIO_CHECK(held.size() == 1);
  BOOST_ASIO_CHECK(log.empty());

  test_function installer(static_cast<test_function&&>(held[0]));
  held.clear();
  frame_clear_at_submit.clear();

  installer();

  BOOST_ASIO_CHECK(caught);

  BOOST_ASIO_CHECK(log == "A I E ");
  BOOST_ASIO_CHECK(held.size() == 3);

  BOOST_ASIO_CHECK(frame_clear_at_submit.size() == 3);
  BOOST_ASIO_CHECK(frame_clear_at_submit[0]);
  BOOST_ASIO_CHECK(frame_clear_at_submit[1]);
  BOOST_ASIO_CHECK(frame_clear_at_submit[2]);
  BOOST_ASIO_CHECK(boost::asio::detail::trampoline_frame::current() == 0);

  for (std::size_t i = 0; i < held.size(); ++i)
    held[i]();
  BOOST_ASIO_CHECK(log == "A I E B C D ");
}

void trampoline_propagate_immediate_drain_once_test()
{
  io_context ioc;
  int calls = 0;
  auto v = make_trampoline(ioc, 8, 2);
  auto q = boost::asio::require(v, execution::blocking.never);
  bool caught = false;

  v.execute(
      [&]
      {
        q.execute([&] { ++calls; });
        v.execute([&] { throw std::runtime_error("x"); });
      });

  try
  {
    ioc.poll_one();
  }
  catch (const std::runtime_error&)
  {
    caught = true;
  }

  BOOST_ASIO_CHECK(caught);
  BOOST_ASIO_CHECK(calls == 0);
  BOOST_ASIO_CHECK(ioc.poll() == 1);
  BOOST_ASIO_CHECK(calls == 1);
  BOOST_ASIO_CHECK(ioc.poll() == 0);
  BOOST_ASIO_CHECK(calls == 1);
}

void trampoline_terminate_policy_test()
{
  io_context ioc;
  std::string log;
  auto v = boost::asio::require(make_trampoline(ioc, 4),
      execution::inline_exception_handling.terminate);

  v.execute(
      [&]
      {
        log += "A ";
        v.execute([&] { log += "B "; });
      });
  BOOST_ASIO_CHECK(ioc.poll_one() == 1);
  BOOST_ASIO_CHECK(log == "A B ");
  BOOST_ASIO_CHECK(ioc.poll() == 0);
}

void trampoline_post_defer_dispatch_test()
{
  io_context ioc;
  auto tr = make_trampoline(ioc.get_executor(), 4);
  any_io_executor any_ex(tr);
  int calls = 0;

  boost::asio::post(tr, [&] { ++calls; });
  boost::asio::defer(tr, [&] { ++calls; });
  boost::asio::dispatch(tr, [&] { ++calls; });
  boost::asio::post(any_ex, [&] { ++calls; });
  boost::asio::defer(any_ex, [&] { ++calls; });
  boost::asio::dispatch(any_ex, [&] { ++calls; });

  boost::asio::post(boost::asio::bind_executor(tr, [&] { ++calls; }));
  boost::asio::defer(boost::asio::bind_executor(tr, [&] { ++calls; }));
  boost::asio::dispatch(boost::asio::bind_executor(tr, [&] { ++calls; }));
  boost::asio::post(boost::asio::bind_executor(any_ex, [&] { ++calls; }));
  boost::asio::defer(boost::asio::bind_executor(any_ex, [&] { ++calls; }));
  boost::asio::dispatch(boost::asio::bind_executor(any_ex, [&] { ++calls; }));

  ioc.run();
  BOOST_ASIO_CHECK(calls == 12);

#if defined(BOOST_ASIO_HAS_STD_FUTURE_CLASS)
  ioc.restart();
  std::future<void> f1 = boost::asio::post(tr, boost::asio::use_future);
  std::future<void> f2 = boost::asio::defer(tr, boost::asio::use_future);
  std::future<void> f3 = boost::asio::dispatch(tr, boost::asio::use_future);
  std::future<void> f4 = boost::asio::post(any_ex, boost::asio::use_future);
  std::future<void> f5 = boost::asio::defer(any_ex, boost::asio::use_future);
  std::future<void> f6 = boost::asio::dispatch(any_ex, boost::asio::use_future);
  ioc.run();
  f1.get(); f2.get(); f3.get(); f4.get(); f5.get(); f6.get();
#endif // defined(BOOST_ASIO_HAS_STD_FUTURE_CLASS)
}

struct destroy_tracker
{
  int* destroyed;
  int* ran;
  bool live;

  destroy_tracker(int* d, int* r)
    : destroyed(d),
      ran(r),
      live(true)
  {
  }

  destroy_tracker(destroy_tracker&& other)
    : destroyed(other.destroyed),
      ran(other.ran),
      live(true)
  {
    other.live = false;
  }

  ~destroy_tracker()
  {
    if (live)
      ++*destroyed;
  }

  void operator()()
  {
    ++*ran;
  }
};

void trampoline_shutdown_destroy_test()
{
  int destroyed = 0;
  int ran = 0;

  {
    io_context ioc;
    auto tr = make_trampoline(ioc, 4);
    any_io_executor any_ex(tr);

    boost::asio::post(any_ex, destroy_tracker(&destroyed, &ran));
    boost::asio::post(any_ex, destroy_tracker(&destroyed, &ran));

  }

  BOOST_ASIO_CHECK(ran == 0);
  BOOST_ASIO_CHECK(destroyed == 2);
}

BOOST_ASIO_TEST_SUITE
(
  "trampoline",
  BOOST_ASIO_COMPILE_TEST_CASE(trampoline_conversion_test)
  BOOST_ASIO_TEST_CASE(trampoline_query_test)
  BOOST_ASIO_TEST_CASE(trampoline_equality_test)
  BOOST_ASIO_TEST_CASE(trampoline_call_count_carried_test)
  BOOST_ASIO_TEST_CASE(trampoline_execute_test)
  BOOST_ASIO_TEST_CASE(trampoline_call_count_test)
  BOOST_ASIO_TEST_CASE(trampoline_zero_and_one_test)
  BOOST_ASIO_TEST_CASE(trampoline_unbounded_test)
  BOOST_ASIO_TEST_CASE(trampoline_identity_match_test)
  BOOST_ASIO_TEST_CASE(trampoline_identity_mismatch_test)
  BOOST_ASIO_TEST_CASE(trampoline_executor_function_test)
  BOOST_ASIO_TEST_CASE(trampoline_identity_property_test)
  BOOST_ASIO_TEST_CASE(trampoline_allocator_forwarding_test)
  BOOST_ASIO_TEST_CASE(trampoline_count_from_installer_test)
  BOOST_ASIO_TEST_CASE(trampoline_running_in_this_thread_test)
  BOOST_ASIO_TEST_CASE(trampoline_carries_test)
  BOOST_ASIO_TEST_CASE(trampoline_inline_depth_default_test)
  BOOST_ASIO_TEST_CASE(trampoline_inline_depth_test)
  BOOST_ASIO_TEST_CASE(trampoline_inline_depth_limit_test)
  BOOST_ASIO_TEST_CASE(trampoline_inline_depth_budget_test)
  BOOST_ASIO_TEST_CASE(trampoline_inline_depth_copy_test)
  BOOST_ASIO_TEST_CASE(trampoline_inline_depth_exception_test)
  BOOST_ASIO_TEST_CASE(trampoline_inline_depth_carried_test)
  BOOST_ASIO_TEST_CASE(trampoline_association_test)
  BOOST_ASIO_TEST_CASE(trampoline_bypass_livelock_test)
  BOOST_ASIO_TEST_CASE(trampoline_ts_executor_test)
  BOOST_ASIO_TEST_CASE(trampoline_fast_path_inline_or_test)
  BOOST_ASIO_TEST_CASE(trampoline_fast_path_thread_pool_test)
  BOOST_ASIO_TEST_CASE(trampoline_fast_path_exception_test)
  BOOST_ASIO_TEST_CASE(trampoline_exception_policy_query_test)
  BOOST_ASIO_TEST_CASE(trampoline_propagate_drain_test)
  BOOST_ASIO_TEST_CASE(trampoline_propagate_drain_order_test)
  BOOST_ASIO_TEST_CASE(trampoline_propagate_nested_drain_test)
  BOOST_ASIO_TEST_CASE(trampoline_propagate_bypass_nesting_test)
  BOOST_ASIO_TEST_CASE(trampoline_propagate_drain_submission_test)
  BOOST_ASIO_TEST_CASE(trampoline_propagate_drain_from_operation_test)
  BOOST_ASIO_TEST_CASE(trampoline_propagate_immediate_drain_test)
  BOOST_ASIO_TEST_CASE(trampoline_propagate_immediate_drain_once_test)
  BOOST_ASIO_TEST_CASE(trampoline_terminate_policy_test)
  BOOST_ASIO_TEST_CASE(trampoline_post_defer_dispatch_test)
  BOOST_ASIO_TEST_CASE(trampoline_shutdown_destroy_test)
)
