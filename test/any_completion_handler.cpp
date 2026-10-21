//
// any_completion_handler.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
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
#include <boost/asio/any_completion_handler.hpp>

#include "unit_test.hpp"

#include <functional>
#include <boost/asio/bind_allocator.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/bind_immediate_executor.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/inline_executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/thread_pool.hpp>

namespace bindns = std;

void increment(int* count)
{
  ++(*count);
}

void any_completion_handler_construction_test()
{
  int count = 0;
  boost::asio::nullptr_t null_ptr = boost::asio::nullptr_t();

  boost::asio::any_completion_handler<void()> h1;

  BOOST_ASIO_CHECK(!h1);
  BOOST_ASIO_CHECK(h1 == null_ptr);

  boost::asio::any_completion_handler<void()> h2(null_ptr);

  BOOST_ASIO_CHECK(!h2);
  BOOST_ASIO_CHECK(h2 == null_ptr);

  boost::asio::any_completion_handler<void()> h3(
      bindns::bind(&increment, &count));

  BOOST_ASIO_CHECK(!!h3);
  BOOST_ASIO_CHECK(h3 != null_ptr);

  boost::asio::any_completion_handler<void()> h4(std::move(h1));

  BOOST_ASIO_CHECK(!h4);
  BOOST_ASIO_CHECK(h4 == null_ptr);
  BOOST_ASIO_CHECK(!h1);
  BOOST_ASIO_CHECK(h1 == null_ptr);

  boost::asio::any_completion_handler<void()> h5(std::move(h3));

  BOOST_ASIO_CHECK(!!h5);
  BOOST_ASIO_CHECK(h5 != null_ptr);
  BOOST_ASIO_CHECK(!h3);
  BOOST_ASIO_CHECK(h3 == null_ptr);

  boost::asio::any_completion_handler<void()> h6(std::move(h5));

  BOOST_ASIO_CHECK(!!h6);
  BOOST_ASIO_CHECK(h6 != null_ptr);
  BOOST_ASIO_CHECK(!h5);
  BOOST_ASIO_CHECK(h5 == null_ptr);
}

void any_completion_handler_assignment_test()
{
  int count = 0;
  boost::asio::nullptr_t null_ptr = boost::asio::nullptr_t();

  boost::asio::any_completion_handler<void()> h1;

  boost::asio::any_completion_handler<void()> h2;
  h2 = null_ptr;

  BOOST_ASIO_CHECK(!h2);

  boost::asio::any_completion_handler<void()> h3;
  h3 = bindns::bind(&increment, &count);

  BOOST_ASIO_CHECK(!!h3);

  boost::asio::any_completion_handler<void()> h4;
  h4 = std::move(h1);

  BOOST_ASIO_CHECK(!h4);
  BOOST_ASIO_CHECK(!h1);

  h4 = std::move(h3);

  BOOST_ASIO_CHECK(!!h4);
  BOOST_ASIO_CHECK(!h3);
}

template <typename T>
class handler_allocator
{
public:
  using value_type = T;

  explicit handler_allocator(int* count)
    : count_(count)
  {
  }

  template <typename U>
  handler_allocator(const handler_allocator<U>& other) noexcept
    : count_(other.count_)
  {
  }

  bool operator==(const handler_allocator& other) const noexcept
  {
    return &count_ == &other.count_;
  }

  bool operator!=(const handler_allocator& other) const noexcept
  {
    return &count_ != &other.count_;
  }

  T* allocate(std::size_t n) const
  {
    ++(*count_);
    return static_cast<T*>(::operator new(sizeof(T) * n));
  }

  void deallocate(T* p, std::size_t /*n*/) const
  {
    ::operator delete(p);
  }

private:
  template <typename> friend class handler_allocator;

  int* count_;
};

class cancel_handler
{
public:
  explicit cancel_handler(int* count)
    : count_(count)
  {
  }

  void operator()(boost::asio::cancellation_type_t)
  {
    ++(*count_);
  }

private:
  int* count_;
};

void any_completion_handler_associator_test()
{
  typedef boost::asio::any_completion_handler<void()> handler_type;

  int count = 0;
  int alloc_count = 0;
  int cancel_count = 0;
  boost::asio::thread_pool pool(1);
  boost::asio::cancellation_signal sig;

  boost::asio::any_completion_handler<void()> h1(
      boost::asio::bind_allocator(handler_allocator<char>(&alloc_count),
        boost::asio::bind_cancellation_slot(sig.slot(),
          boost::asio::bind_executor(pool.get_executor(),
            boost::asio::bind_immediate_executor(boost::asio::inline_executor(),
              bindns::bind(&increment, &count))))));

  BOOST_ASIO_CHECK(alloc_count == 1);

  BOOST_ASIO_REBIND_ALLOC(boost::asio::associated_allocator<handler_type>::type,
      char) alloc1(boost::asio::get_associated_allocator(h1));
  alloc1.deallocate(alloc1.allocate(1), 1);

  BOOST_ASIO_CHECK(alloc_count == 2);

  boost::asio::associated_cancellation_slot<handler_type>::type slot1
    = boost::asio::get_associated_cancellation_slot(h1);

  BOOST_ASIO_CHECK(slot1.is_connected());

  slot1.emplace<cancel_handler>(&cancel_count);

  BOOST_ASIO_CHECK(cancel_count == 0);

  sig.emit(boost::asio::cancellation_type::terminal);

  BOOST_ASIO_CHECK(cancel_count == 1);

  boost::asio::associated_executor<handler_type>::type ex1
    = boost::asio::get_associated_executor(h1);

  BOOST_ASIO_CHECK(ex1 == pool.get_executor());

  boost::asio::associated_immediate_executor<
    handler_type, boost::asio::thread_pool::executor_type>::type ex2
      = boost::asio::get_associated_immediate_executor(h1, pool.get_executor());

  BOOST_ASIO_CHECK(ex2 == boost::asio::inline_executor());
}

void increment_with_error(boost::system::error_code ec,
    boost::system::error_code* out_ec, int* count)
{
  *out_ec = ec;
  ++(*count);
}

void any_completion_handler_invocation_test()
{
  int count = 0;
  boost::system::error_code ec;

  boost::asio::any_completion_handler<void()> h1(
      bindns::bind(&increment, &count));

  BOOST_ASIO_CHECK(count == 0);

  std::move(h1)();

  BOOST_ASIO_CHECK(count == 1);

  boost::asio::any_completion_handler<void(boost::system::error_code)> h2(
      bindns::bind(&increment_with_error,
        bindns::placeholders::_1, &ec, &count));

  BOOST_ASIO_CHECK(!ec);
  BOOST_ASIO_CHECK(count == 1);

  std::move(h2)(boost::asio::error::eof);

  BOOST_ASIO_CHECK(ec == boost::asio::error::eof);
  BOOST_ASIO_CHECK(count == 2);
}

// Regression tests for handler_work's cross-erased-type fast path
// (detail/handler_work.hpp): an any_completion_handler's associated
// executor is reported as any_completion_executor, a different C++ type
// from an I/O object's any_io_executor even when both erase the identical
// underlying target, so the equality fast path that skips a redundant
// tracked-work executor must still be reachable in that (very common)
// case, and must NOT be taken when the two executors differ.

// Same target on both sides (the common case: no executor explicitly
// bound to the completion handler, so it defaults to the timer's own
// I/O executor). If the fast path's equality check under-tracked, by
// treating this timer's own outstanding work as already covered when it
// was not, the io_context could stop as soon as the first timer's
// wait operation issues, before either handler has actually run: the
// second timer is not even created, let alone outstanding, until the
// first handler runs. Checking that both handlers ran (not stopped
// early) and that run() returned (not stopped late/hung, since a unit
// test that never returns fails as a timeout) together demonstrate the
// work count stayed correct.
void any_completion_handler_outstanding_work_test()
{
  boost::asio::io_context ioc;
  boost::asio::any_io_executor io_ex(ioc.get_executor());

  int fire_count = 0;
  boost::asio::steady_timer timer2(io_ex);

  boost::asio::steady_timer timer1(io_ex);
  timer1.expires_after(boost::asio::chrono::milliseconds(1));
  timer1.async_wait(
      boost::asio::any_completion_handler<void(boost::system::error_code)>(
        [&](boost::system::error_code)
        {
          ++fire_count;
          timer2.expires_after(boost::asio::chrono::milliseconds(1));
          timer2.async_wait(
              boost::asio::any_completion_handler<void(boost::system::error_code)>(
                [&](boost::system::error_code)
                {
                  ++fire_count;
                }));
        }));

  ioc.run();

  BOOST_ASIO_CHECK(fire_count == 2);
}

// Different targets: the handler's associated executor is explicitly
// bound to a second, distinct io_context's executor, so the fast path
// must NOT apply. The completion must still be dispatched THROUGH that
// second executor rather than run inline by the first. That
// is checked directly and structurally, not just by a work count: since
// ioc2 is never run until after ioc has finished, an inline (wrongly
// untracked/undispatched) completion would show up as the handler having
// already fired before ioc2.run() is ever called.
void any_completion_handler_different_bound_executor_test()
{
  boost::asio::io_context ioc;
  boost::asio::any_io_executor io_ex(ioc.get_executor());

  boost::asio::io_context ioc2;
  boost::asio::any_completion_executor io_ex2(ioc2.get_executor());

  int fire_count = 0;
  boost::asio::steady_timer timer(io_ex);
  timer.expires_after(boost::asio::chrono::milliseconds(1));
  timer.async_wait(
      boost::asio::any_completion_handler<void(boost::system::error_code)>(
        boost::asio::bind_executor(io_ex2,
          [&](boost::system::error_code)
          {
            ++fire_count;
          })));

  ioc.run();

  // The wait completed, but since the handler's executor (ioc2's) differs
  // from the timer's own I/O executor (ioc's), handler_work must have
  // dispatched it there rather than run it inline, so it must not have
  // fired yet.
  BOOST_ASIO_CHECK(fire_count == 0);

  std::size_t handlers_run = ioc2.run();

  BOOST_ASIO_CHECK(fire_count == 1);
  BOOST_ASIO_CHECK(handlers_run == 1);
}

inline std::pair<boost::asio::ip::tcp::socket, boost::asio::ip::tcp::socket>
make_connected_pair(boost::asio::io_context& ioc, const boost::asio::any_io_executor& ex)
{
  boost::asio::ip::tcp::acceptor acceptor(ex,
      boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 0));
  boost::asio::ip::tcp::socket client(ex);
  auto port = acceptor.local_endpoint().port();
  client.connect(boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address_v4::loopback(), port));
  boost::asio::ip::tcp::socket server = acceptor.accept();
  (void)ioc;
  return { std::move(client), std::move(server) };
}

// A handler that names its own immediate executor, so that
// associated_immediate_executor is specialised for it and the erased handler
// must ask the handler rather than assume the default association.
struct handler_with_immediate_executor
{
  typedef boost::asio::inline_executor immediate_executor_type;

  immediate_executor_type get_immediate_executor() const noexcept
  {
    return boost::asio::inline_executor();
  }

  void operator()()
  {
  }
};

void any_completion_handler_immediate_executor_association_test()
{
  typedef boost::asio::any_completion_handler<void()> handler_type;

  int count = 0;
  boost::asio::io_context ioc;

  // A handler with the default association: the immediate executor is
  // derived from the candidate, and is the candidate made non-blocking.
  handler_type h1(bindns::bind(&increment, &count));

  boost::asio::associated_immediate_executor<handler_type,
    boost::asio::io_context::executor_type>::type ex1
      = boost::asio::get_associated_immediate_executor(h1, ioc.get_executor());

  BOOST_ASIO_CHECK(ex1 == boost::asio::any_completion_executor(std::nothrow,
        boost::asio::require(boost::asio::any_io_executor(ioc.get_executor()),
          boost::asio::execution::blocking.never)));

  // A handler that specialises the association: the erased handler must
  // report the handler's own immediate executor.
  handler_type h2(handler_with_immediate_executor{});

  boost::asio::associated_immediate_executor<handler_type,
    boost::asio::io_context::executor_type>::type ex2
      = boost::asio::get_associated_immediate_executor(h2, ioc.get_executor());

  BOOST_ASIO_CHECK(ex2 == boost::asio::inline_executor());

  // The same, reached through a binder rather than a nested type.
  handler_type h3(boost::asio::bind_immediate_executor(
        boost::asio::inline_executor(), bindns::bind(&increment, &count)));

  boost::asio::associated_immediate_executor<handler_type,
    boost::asio::io_context::executor_type>::type ex3
      = boost::asio::get_associated_immediate_executor(h3, ioc.get_executor());

  BOOST_ASIO_CHECK(ex3 == boost::asio::inline_executor());
}

// The I/O object's executor is concrete while the handler's association is
// erased. The two now compare equal when they designate the same executor,
// and must still compare unequal when they do not.
void any_completion_handler_concrete_io_executor_test()
{
  typedef boost::asio::basic_waitable_timer<boost::asio::chrono::steady_clock,
      boost::asio::wait_traits<boost::asio::chrono::steady_clock>,
      boost::asio::io_context::executor_type> timer_type;

  boost::asio::io_context ioc;
  boost::asio::io_context ioc2;

  int fire_count = 0;

  // Same executor, reached through an erased association.
  timer_type timer1(ioc.get_executor());
  timer1.expires_after(boost::asio::chrono::milliseconds(1));
  timer1.async_wait(
      boost::asio::any_completion_handler<void(boost::system::error_code)>(
        boost::asio::bind_executor(boost::asio::any_completion_executor(ioc.get_executor()),
          [&](boost::system::error_code)
          {
            ++fire_count;
          })));

  // A different executor, of the same concrete type as the I/O object's. If
  // the comparison wrongly succeeded, this completion would be run on ioc
  // rather than dispatched to ioc2, and the check below fails.
  timer_type timer2(ioc.get_executor());
  timer2.expires_after(boost::asio::chrono::milliseconds(1));
  timer2.async_wait(
      boost::asio::any_completion_handler<void(boost::system::error_code)>(
        boost::asio::bind_executor(boost::asio::any_completion_executor(ioc2.get_executor()),
          [&](boost::system::error_code)
          {
            ++fire_count;
          })));

  ioc.run();

  BOOST_ASIO_CHECK(fire_count == 1);

  std::size_t handlers_run = ioc2.run();

  BOOST_ASIO_CHECK(fire_count == 2);
  BOOST_ASIO_CHECK(handlers_run == 1);
}

int counting_allocations = 0;
int counting_deallocations = 0;

// An allocator that records its use, so that a handler carrying one can be
// seen to keep it. Deliberately no more than ordinarily aligned: an
// over-aligned value type is the general path's business, not the
// allocator's, since that path rebinds to bytes and recovers the alignment
// by hand on top of whatever this returns.
template <typename T>
class counting_allocator
{
public:
  typedef T value_type;

  counting_allocator() noexcept
  {
  }

  template <typename U>
  counting_allocator(const counting_allocator<U>&) noexcept
  {
  }

  T* allocate(std::size_t n)
  {
    ++counting_allocations;
    return static_cast<T*>(::operator new(sizeof(T) * n));
  }

  void deallocate(T* p, std::size_t) noexcept
  {
    ++counting_deallocations;
    ::operator delete(p);
  }

  template <typename U>
  bool operator==(const counting_allocator<U>&) const noexcept
  {
    return true;
  }

  template <typename U>
  bool operator!=(const counting_allocator<U>&) const noexcept
  {
    return false;
  }
};

// Stands in for an operation's state: ordinarily aligned, so the recycling
// allocator's own blocks already satisfy it.
struct op_state
{
  std::size_t value;
  char data[24];
};

// More aligned than the blocks the recycling allocator hands out.
struct alignas(64) over_aligned_op_state
{
  std::size_t value;
};

// A handler that leaves associated_allocator unspecialised, which its
// function table records, so an operation's allocations go straight to the
// recycling allocator. Repeating the cycle also exercises a recycled block,
// which the fast path must hand back usable and correctly aligned.
void any_completion_handler_default_allocator_test()
{
  int count = 0;
  boost::asio::any_completion_handler<void()> h(
      bindns::bind(&increment, &count));

  typedef std::allocator_traits<
    boost::asio::any_completion_handler<void()>::allocator_type>::
      rebind_alloc<op_state> alloc_type;

  alloc_type alloc(h.get_allocator());

  for (int i = 0; i < 4; ++i)
  {
    op_state* p = alloc.allocate(1);

    BOOST_ASIO_CHECK(p != 0);
    BOOST_ASIO_CHECK(reinterpret_cast<std::size_t>(p) % alignof(op_state) == 0);

    p->value = 42;

    BOOST_ASIO_CHECK(p->value == 42);

    alloc.deallocate(p, 1);
  }

  op_state* p = alloc.allocate(3);

  BOOST_ASIO_CHECK(p != 0);
  BOOST_ASIO_CHECK(reinterpret_cast<std::size_t>(p) % alignof(op_state) == 0);

  alloc.deallocate(p, 3);
}

// A handler with its own associated allocator, applied BEFORE erasure. The
// function table records that the association is not the default one, so
// every allocation must still reach that allocator, by the general path --
// both when made directly through the erased handler's allocator and when
// made by a real operation completing through it.
void any_completion_handler_custom_allocator_test()
{
  int count = 0;
  boost::asio::any_completion_handler<void()> h(
      boost::asio::bind_allocator(counting_allocator<void>(),
        bindns::bind(&increment, &count)));

  // The erased implementation itself was allocated through the same
  // allocator, so count from here rather than from zero.
  int allocations_before = counting_allocations;
  int deallocations_before = counting_deallocations;

  typedef std::allocator_traits<
    boost::asio::any_completion_handler<void()>::allocator_type>::
      rebind_alloc<op_state> alloc_type;

  alloc_type alloc(h.get_allocator());

  op_state* p = alloc.allocate(1);

  BOOST_ASIO_CHECK(p != 0);
  BOOST_ASIO_CHECK(reinterpret_cast<std::size_t>(p) % alignof(op_state) == 0);
  BOOST_ASIO_CHECK(counting_allocations == allocations_before + 1);

  alloc.deallocate(p, 1);

  BOOST_ASIO_CHECK(counting_deallocations == deallocations_before + 1);

  // The same association, reached by an operation rather than by hand.
  boost::asio::io_context ioc;
  boost::asio::any_io_executor io_ex(ioc.get_executor());
  auto pair = make_connected_pair(ioc, io_ex);
  boost::asio::ip::tcp::socket& client = pair.first;

  char data[4] = { 'a', 'b', 'c', 'd' };
  int fire_count = 0;
  int op_allocations_before = counting_allocations;
  int op_deallocations_before = counting_deallocations;

  client.async_write_some(boost::asio::buffer(data),
      boost::asio::any_completion_handler<void(boost::system::error_code, std::size_t)>(
        boost::asio::bind_allocator(counting_allocator<void>(),
          [&](boost::system::error_code, std::size_t)
          {
            ++fire_count;
          })));

  ioc.run();

  // Everything the operation allocated through the handler came back: the
  // handler still held by h above is deliberately not counted, being older
  // than this operation and still alive.
  BOOST_ASIO_CHECK(fire_count == 1);
  BOOST_ASIO_CHECK(counting_allocations > op_allocations_before);
  BOOST_ASIO_CHECK(counting_allocations - op_allocations_before
      == counting_deallocations - op_deallocations_before);
}

// An operation type more aligned than the blocks the recycling allocator
// hands out. The fast path cannot satisfy it, so the general path, which
// recovers the alignment by hand, must still be taken, and must still
// produce a correctly aligned block, for a handler with the default
// allocator association as much as for one without.
void any_completion_handler_over_aligned_allocation_test()
{
  int count = 0;
  boost::asio::any_completion_handler<void()> h(
      bindns::bind(&increment, &count));

  typedef std::allocator_traits<
    boost::asio::any_completion_handler<void()>::allocator_type>::
      rebind_alloc<over_aligned_op_state> alloc_type;

  alloc_type alloc(h.get_allocator());

  for (int i = 0; i < 4; ++i)
  {
    over_aligned_op_state* p = alloc.allocate(1);

    BOOST_ASIO_CHECK(p != 0);
    BOOST_ASIO_CHECK(reinterpret_cast<std::size_t>(p)
        % alignof(over_aligned_op_state) == 0);

    p->value = 7;

    BOOST_ASIO_CHECK(p->value == 7);

    alloc.deallocate(p, 1);
  }

  over_aligned_op_state* p = alloc.allocate(3);

  BOOST_ASIO_CHECK(p != 0);
  BOOST_ASIO_CHECK(reinterpret_cast<std::size_t>(p)
      % alignof(over_aligned_op_state) == 0);

  alloc.deallocate(p, 3);

  // The same over-aligned type, through a handler whose allocator is its
  // own: over-aligned and custom together still land on the general path.
  boost::asio::any_completion_handler<void()> h2(
      boost::asio::bind_allocator(counting_allocator<void>(),
        bindns::bind(&increment, &count)));

  int allocations_before = counting_allocations;

  alloc_type alloc2(h2.get_allocator());

  over_aligned_op_state* p2 = alloc2.allocate(1);

  BOOST_ASIO_CHECK(p2 != 0);
  BOOST_ASIO_CHECK(reinterpret_cast<std::size_t>(p2)
      % alignof(over_aligned_op_state) == 0);
  BOOST_ASIO_CHECK(counting_allocations == allocations_before + 1);

  alloc2.deallocate(p2, 1);
}

// Feeds the read started by each test below. Issued only once that read is
// already outstanding, so the completion is delivered by the reactor and
// goes through handler_work rather than the immediate path.
inline void write_four_bytes(boost::asio::ip::tcp::socket& client)
{
  char send_data[4] = { 'a', 'b', 'c', 'd' };
  client.write_some(boost::asio::buffer(send_data));
}

// The default association, reached through a composed operation: async_read's
// own continuation wraps the erased handler, so whether the associated
// executor is the I/O executor is answered for that wrapper, which forwards
// the question to the handler it wraps. The answer lets base2 be left empty
// without the association ever being derived. This path has to preserve
// completion exactly once, with the right byte count.
void any_completion_handler_composed_operation_test()
{
  boost::asio::io_context ioc;
  boost::asio::any_io_executor io_ex(ioc.get_executor());
  auto pair = make_connected_pair(ioc, io_ex);
  boost::asio::ip::tcp::socket& client = pair.first;
  boost::asio::ip::tcp::socket& server = pair.second;

  char recv_data[4];
  int fire_count = 0;
  std::size_t transferred = 0;

  boost::asio::async_read(server, boost::asio::buffer(recv_data),
      boost::asio::any_completion_handler<void(boost::system::error_code, std::size_t)>(
        [&](boost::system::error_code ec, std::size_t n)
        {
          ++fire_count;
          if (!ec)
            transferred = n;
        }));

  write_four_bytes(client);

  ioc.run();

  BOOST_ASIO_CHECK(fire_count == 1);
  BOOST_ASIO_CHECK(transferred == 4);
}

// A different executor, bound BEFORE erasure, so that the binder is behind
// the erasure and only the erased handler's function table can report it.
// The table records that the association is not the default one,
// so the association must still be derived and the completion dispatched
// through it. Structurally decisive: ioc2 is not run until after ioc's run()
// returns, so a wrongly skipped association shows up as having already fired.
void any_completion_handler_composed_bound_executor_test()
{
  boost::asio::io_context ioc;
  boost::asio::any_io_executor io_ex(ioc.get_executor());
  auto pair = make_connected_pair(ioc, io_ex);
  boost::asio::ip::tcp::socket& client = pair.first;
  boost::asio::ip::tcp::socket& server = pair.second;

  boost::asio::io_context ioc2;
  boost::asio::any_completion_executor io_ex2(ioc2.get_executor());

  char recv_data[4];
  int fire_count = 0;

  boost::asio::async_read(server, boost::asio::buffer(recv_data),
      boost::asio::any_completion_handler<void(boost::system::error_code, std::size_t)>(
        boost::asio::bind_executor(io_ex2,
          [&](boost::system::error_code, std::size_t)
          {
            ++fire_count;
          })));

  write_four_bytes(client);

  ioc.run();

  BOOST_ASIO_CHECK(fire_count == 0);

  std::size_t handlers_run = ioc2.run();

  BOOST_ASIO_CHECK(fire_count == 1);
  BOOST_ASIO_CHECK(handlers_run == 1);
}

// The binder OUTSIDE the erasure, wrapping a handler that does have the
// default association. This is what the associator-forwarding guard exists
// for: the binder forwards every association except the executor, which it
// overrides, so its own associated_executor does not derive from its
// associator and the question must stop there. Were it forwarded past the
// binder to the erased handler, that handler would answer "default", base2
// would be left empty, and the completion would run inline on ioc instead of
// being dispatched to ioc2.
void any_completion_handler_binder_over_erased_executor_test()
{
  boost::asio::io_context ioc;
  boost::asio::any_io_executor io_ex(ioc.get_executor());
  auto pair = make_connected_pair(ioc, io_ex);
  boost::asio::ip::tcp::socket& client = pair.first;
  boost::asio::ip::tcp::socket& server = pair.second;

  boost::asio::io_context ioc2;
  boost::asio::any_io_executor io_ex2(ioc2.get_executor());

  char recv_data[4];
  int fire_count = 0;

  boost::asio::async_read(server, boost::asio::buffer(recv_data),
      boost::asio::bind_executor(io_ex2,
        boost::asio::any_completion_handler<void(boost::system::error_code, std::size_t)>(
          [&](boost::system::error_code, std::size_t)
          {
            ++fire_count;
          })));

  write_four_bytes(client);

  ioc.run();

  BOOST_ASIO_CHECK(fire_count == 0);

  std::size_t handlers_run = ioc2.run();

  BOOST_ASIO_CHECK(fire_count == 1);
  BOOST_ASIO_CHECK(handlers_run == 1);
}

BOOST_ASIO_TEST_SUITE
(
  "any_completion_handler",
  BOOST_ASIO_TEST_CASE(any_completion_handler_construction_test)
  BOOST_ASIO_TEST_CASE(any_completion_handler_assignment_test)
  BOOST_ASIO_TEST_CASE(any_completion_handler_associator_test)
  BOOST_ASIO_TEST_CASE(any_completion_handler_immediate_executor_association_test)
  BOOST_ASIO_TEST_CASE(any_completion_handler_invocation_test)
  BOOST_ASIO_TEST_CASE(any_completion_handler_outstanding_work_test)
  BOOST_ASIO_TEST_CASE(any_completion_handler_different_bound_executor_test)
  BOOST_ASIO_TEST_CASE(any_completion_handler_concrete_io_executor_test)
  BOOST_ASIO_TEST_CASE(any_completion_handler_composed_operation_test)
  BOOST_ASIO_TEST_CASE(any_completion_handler_composed_bound_executor_test)
  BOOST_ASIO_TEST_CASE(any_completion_handler_binder_over_erased_executor_test)
  BOOST_ASIO_TEST_CASE(any_completion_handler_default_allocator_test)
  BOOST_ASIO_TEST_CASE(any_completion_handler_custom_allocator_test)
  BOOST_ASIO_TEST_CASE(any_completion_handler_over_aligned_allocation_test)
)
