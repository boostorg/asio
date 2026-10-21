//
// eager.cpp
// ~~~~~~~~~
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
#include <boost/asio/eager.hpp>

#include <atomic>
#include <functional>
#include <vector>
#include <boost/asio/associator.hpp>
#include <boost/asio/bind_allocator.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/detail/thread.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include "unit_test.hpp"

template <typename Handler>
class completion_observer
{
public:
  template <typename H>
  completion_observer(boost::system::error_code& observed, H&& handler)
    : observed_(&observed),
      handler_(static_cast<H&&>(handler))
  {
  }

  void operator()(boost::system::error_code ec)
  {
    *observed_ = ec;
    static_cast<Handler&&>(handler_)(ec);
  }

  boost::system::error_code* observed_;
  Handler handler_;
};

namespace boost {
namespace asio {

template <template <typename, typename> class Associator,
    typename Handler, typename DefaultCandidate>
struct associator<Associator,
    ::completion_observer<Handler>, DefaultCandidate>
  : Associator<Handler, DefaultCandidate>
{
  static typename Associator<Handler, DefaultCandidate>::type get(
      const ::completion_observer<Handler>& h) noexcept
  {
    return Associator<Handler, DefaultCandidate>::get(h.handler_);
  }

  static auto get(const ::completion_observer<Handler>& h,
      const DefaultCandidate& c) noexcept
    -> decltype(Associator<Handler, DefaultCandidate>::get(h.handler_, c))
  {
    return Associator<Handler, DefaultCandidate>::get(h.handler_, c);
  }
};

} // namespace asio
} // namespace boost

class initiate_observed_wait
{
public:
  typedef boost::asio::steady_timer::executor_type executor_type;

  initiate_observed_wait(boost::asio::steady_timer& timer,
      boost::system::error_code& observed)
    : timer_(&timer),
      observed_(&observed)
  {
  }

  executor_type get_executor() const noexcept
  {
    return timer_->get_executor();
  }

  template <typename Handler>
  void operator()(Handler&& handler) const
  {
    timer_->async_wait(
        completion_observer<boost::asio::decay_t<Handler>>(
          *observed_, static_cast<Handler&&>(handler)));
  }

private:
  boost::asio::steady_timer* timer_;
  boost::system::error_code* observed_;
};

template <typename CompletionToken>
auto observed_wait(boost::asio::steady_timer& timer,
    boost::system::error_code& observed, CompletionToken&& token)
  -> decltype(
      boost::asio::async_initiate<CompletionToken,
        void(boost::system::error_code)>(
          initiate_observed_wait(timer, observed), token))
{
  return boost::asio::async_initiate<CompletionToken,
      void(boost::system::error_code)>(
        initiate_observed_wait(timer, observed), token);
}

struct cancel_recorder
{
  bool* cancelled;

  void operator()(boost::asio::cancellation_type_t)
  {
    *cancelled = true;
  }
};

class initiate_never_completes
{
public:
  typedef boost::asio::io_context::executor_type executor_type;

  initiate_never_completes(boost::asio::io_context& ctx, bool& cancelled)
    : ctx_(&ctx),
      cancelled_(&cancelled)
  {
  }

  executor_type get_executor() const noexcept
  {
    return ctx_->get_executor();
  }

  template <typename Handler>
  void operator()(Handler&& handler) const
  {
    typedef boost::asio::decay_t<Handler> handler_type;
    boost::asio::associated_cancellation_slot_t<handler_type> slot
      = boost::asio::get_associated_cancellation_slot(handler);
    if (slot.is_connected())
      slot.assign(cancel_recorder{cancelled_});
  }

private:
  boost::asio::io_context* ctx_;
  bool* cancelled_;
};

template <typename CompletionToken>
auto never_completes(boost::asio::io_context& ctx,
    bool& cancelled, CompletionToken&& token)
  -> decltype(
      boost::asio::async_initiate<CompletionToken,
        void(boost::system::error_code)>(
          initiate_never_completes(ctx, cancelled), token))
{
  return boost::asio::async_initiate<CompletionToken,
      void(boost::system::error_code)>(
        initiate_never_completes(ctx, cancelled), token);
}

template <typename Handler>
struct multi_sig_completion
{
  Handler handler;
  bool with_value;

  void operator()()
  {
    if (with_value)
      static_cast<Handler&&>(handler)(boost::system::error_code(), 42);
    else
      static_cast<Handler&&>(handler)();
  }
};

struct initiate_multi_sig
{
  boost::asio::io_context* ctx;
  bool with_value;

  typedef boost::asio::io_context::executor_type executor_type;

  executor_type get_executor() const noexcept
  {
    return ctx->get_executor();
  }

  template <typename Handler>
  void operator()(Handler&& handler) const
  {
    boost::asio::post(*ctx, multi_sig_completion<boost::asio::decay_t<Handler>>{
        static_cast<Handler&&>(handler), with_value});
  }
};

template <typename CompletionToken>
auto multi_sig(boost::asio::io_context& ctx, bool with_value,
    CompletionToken&& token)
  -> decltype(
      boost::asio::async_initiate<CompletionToken,
        void(boost::system::error_code, int), void()>(
          initiate_multi_sig{&ctx, with_value}, token))
{
  return boost::asio::async_initiate<CompletionToken,
      void(boost::system::error_code, int), void()>(
        initiate_multi_sig{&ctx, with_value}, token);
}

struct signature_collector
{
  int* value;
  bool* empty;

  void operator()(boost::system::error_code, int v) const { *value = v; }
  void operator()() const { *empty = true; }
};

struct destroy_tracking_handler
{
  bool* called;
  bool* destroyed;

  destroy_tracking_handler(bool* c, bool* d) : called(c), destroyed(d) {}
  destroy_tracking_handler(const destroy_tracking_handler&) = delete;

  destroy_tracking_handler(destroy_tracking_handler&& other) noexcept
    : called(other.called), destroyed(other.destroyed)
  {
    other.destroyed = 0;
  }

  ~destroy_tracking_handler() { if (destroyed) *destroyed = true; }

  void operator()(boost::system::error_code) { *called = true; }
};

struct initiate_inline_completion
{
  boost::asio::io_context* ctx;

  typedef boost::asio::io_context::executor_type executor_type;

  executor_type get_executor() const noexcept
  {
    return ctx->get_executor();
  }

  template <typename Handler>
  void operator()(Handler&& handler) const
  {
    typedef boost::asio::decay_t<Handler> handler_type;
    handler_type h(static_cast<Handler&&>(handler));
    static_cast<handler_type&&>(h)(boost::system::error_code(), 42);
  }
};

template <typename CompletionToken>
auto inline_completion(boost::asio::io_context& ctx, CompletionToken&& token)
  -> decltype(
      boost::asio::async_initiate<CompletionToken,
        void(boost::system::error_code, int)>(
          initiate_inline_completion{&ctx}, token))
{
  return boost::asio::async_initiate<CompletionToken,
      void(boost::system::error_code, int)>(
        initiate_inline_completion{&ctx}, token);
}

template <typename Handler>
struct posted_invoker
{
  Handler handler;

  void operator()()
  {
    static_cast<Handler&&>(handler)(boost::system::error_code(), 99);
  }
};

struct initiate_moving
{
  boost::asio::io_context* ctx;

  typedef boost::asio::io_context::executor_type executor_type;

  executor_type get_executor() const noexcept
  {
    return ctx->get_executor();
  }

  template <typename Handler>
  void operator()(Handler&& handler) const
  {
    typedef boost::asio::decay_t<Handler> handler_type;
    handler_type h(static_cast<Handler&&>(handler));
    boost::asio::post(*ctx, posted_invoker<handler_type>{
        static_cast<handler_type&&>(h)});
  }
};

template <typename CompletionToken>
auto moving_init(boost::asio::io_context& ctx, CompletionToken&& token)
  -> decltype(
      boost::asio::async_initiate<CompletionToken,
        void(boost::system::error_code, int)>(initiate_moving{&ctx}, token))
{
  return boost::asio::async_initiate<CompletionToken,
      void(boost::system::error_code, int)>(initiate_moving{&ctx}, token);
}

template <typename T>
struct counting_allocator
{
  typedef T value_type;

  std::size_t* count;

  explicit counting_allocator(std::size_t* c) : count(c) {}

  template <typename U>
  counting_allocator(const counting_allocator<U>& other)
    : count(other.count)
  {
  }

  T* allocate(std::size_t n)
  {
    ++*count;
    return static_cast<T*>(::operator new(n * sizeof(T)));
  }

  void deallocate(T* p, std::size_t)
  {
    --*count;
    ::operator delete(p);
  }

  template <typename U>
  bool operator==(const counting_allocator<U>&) const { return true; }

  template <typename U>
  bool operator!=(const counting_allocator<U>&) const { return false; }
};

struct deferred_link
{
  auto operator()(boost::system::error_code ec) const
    -> decltype(boost::asio::deferred.values(ec))
  {
    return boost::asio::deferred.values(ec);
  }
};

#if !defined(BOOST_ASIO_NO_EXCEPTIONS)

struct allocation_failure {};

template <typename T>
struct throwing_allocator
{
  typedef T value_type;

  throwing_allocator() {}

  template <typename U>
  throwing_allocator(const throwing_allocator<U>&) {}

  T* allocate(std::size_t)
  {
    throw allocation_failure();
  }

  void deallocate(T*, std::size_t) {}

  template <typename U>
  bool operator==(const throwing_allocator<U>&) const { return true; }

  template <typename U>
  bool operator!=(const throwing_allocator<U>&) const { return false; }
};

#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)

void eager_test()
{
  boost::asio::io_context ctx;
  boost::asio::steady_timer timer(ctx);
  timer.expires_after(boost::asio::chrono::seconds(0));

  auto e = timer.async_wait(boost::asio::eager);

  BOOST_ASIO_CHECK(boost::asio::is_async_operation<decltype(e)>::value);
  BOOST_ASIO_CHECK(!e.ready());

  ctx.run();

  BOOST_ASIO_CHECK(e.ready());

  ctx.restart();

  boost::system::error_code ec = boost::asio::error::would_block;
  bool called = false;
  std::move(e)(
      [&](boost::system::error_code ec_)
      {
        ec = ec_;
        called = true;
      });

  BOOST_ASIO_CHECK(!called);

  ctx.run();

  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK_MESSAGE(!ec, ec.message());
}

void eager_multi_signature_test()
{
  {
    boost::asio::io_context ctx;
    int value = -1;
    bool empty = false;

    auto e = multi_sig(ctx, true, boost::asio::eager);
    std::move(e)(signature_collector{&value, &empty});

    ctx.run();

    BOOST_ASIO_CHECK(value == 42);
    BOOST_ASIO_CHECK(!empty);
  }

  {
    boost::asio::io_context ctx;
    int value = -1;
    bool empty = false;

    auto e = multi_sig(ctx, false, boost::asio::eager);
    std::move(e)(signature_collector{&value, &empty});

    ctx.run();

    BOOST_ASIO_CHECK(value == -1);
    BOOST_ASIO_CHECK(empty);
  }
}

void eager_detach_test()
{
  // Dropping the handle detaches: the operation is not cancelled, and runs
  // to completion with its result discarded.
  {
    boost::asio::io_context ctx;
    boost::asio::steady_timer timer(ctx);
    timer.expires_after(boost::asio::chrono::seconds(0));

    boost::system::error_code ec = boost::asio::error::would_block;
    {
      auto e = observed_wait(timer, ec, boost::asio::eager);
      (void)e;
    }

    ctx.run();

    BOOST_ASIO_CHECK_MESSAGE(!ec, ec.message());
  }

  // The handle may be dropped once the wait has been started.
  {
    boost::asio::io_context ctx;
    boost::asio::steady_timer timer(ctx);
    timer.expires_after(boost::asio::chrono::seconds(0));

    boost::system::error_code ec = boost::asio::error::would_block;
    bool called = false;
    {
      auto e = timer.async_wait(boost::asio::eager);
      std::move(e)(
          [&](boost::system::error_code ec_)
          {
            ec = ec_;
            called = true;
          });
    }

    ctx.run();

    BOOST_ASIO_CHECK(called);
    BOOST_ASIO_CHECK_MESSAGE(!ec, ec.message());
  }

  // An abandoned wait destroys the handler without invoking it.
  {
    bool called = false;
    bool destroyed = false;
    {
      boost::asio::io_context ctx;
      boost::asio::steady_timer timer(ctx, boost::asio::chrono::seconds(10));
      auto e = timer.async_wait(boost::asio::eager);
      std::move(e)(destroy_tracking_handler(&called, &destroyed));
    }

    BOOST_ASIO_CHECK(!called);
    BOOST_ASIO_CHECK(destroyed);
  }
}

void eager_cancel_test()
{
  // Cancellation is emitted on the calling thread.
  {
    boost::asio::io_context ctx;
    bool cancelled = false;

    auto e = never_completes(ctx, cancelled, boost::asio::eager);
    BOOST_ASIO_CHECK(!cancelled);

    e.cancel();
    BOOST_ASIO_CHECK(cancelled);
  }

  // A cancelled operation completes with operation_aborted.
  {
    boost::asio::io_context ctx;
    boost::asio::steady_timer timer(ctx, boost::asio::chrono::seconds(10));

    auto e = timer.async_wait(boost::asio::eager);
    e.cancel();

    boost::system::error_code ec = boost::asio::error::would_block;
    bool called = false;
    std::move(e)(
        [&](boost::system::error_code ec_)
        {
          ec = ec_;
          called = true;
        });

    ctx.run();

    BOOST_ASIO_CHECK(called);
    BOOST_ASIO_CHECK(ec == boost::asio::error::operation_aborted);
  }

  // The waiting handler's cancellation slot reaches the operation.
  {
    boost::asio::io_context ctx;
    boost::asio::steady_timer timer(ctx, boost::asio::chrono::seconds(10));

    auto e = timer.async_wait(boost::asio::eager);

    boost::system::error_code ec = boost::asio::error::would_block;
    bool called = false;
    boost::asio::cancellation_signal sig;

    std::move(e)(
        boost::asio::bind_cancellation_slot(sig.slot(),
          [&](boost::system::error_code ec_)
          {
            ec = ec_;
            called = true;
          }));

    BOOST_ASIO_CHECK(!called);

    sig.emit(boost::asio::cancellation_type::all);
    ctx.run();

    BOOST_ASIO_CHECK(called);
    BOOST_ASIO_CHECK(ec == boost::asio::error::operation_aborted);
  }
}

void eager_detached_test()
{
  // Waiting with the detached token discards the result.
  {
    boost::asio::io_context ctx;
    boost::asio::steady_timer timer(ctx);
    timer.expires_after(boost::asio::chrono::seconds(0));

    boost::system::error_code ec = boost::asio::error::would_block;
    {
      auto e = observed_wait(timer, ec, boost::asio::eager);
      std::move(e)(boost::asio::detached);
    }

    ctx.run();

    BOOST_ASIO_CHECK_MESSAGE(!ec, ec.message());
  }

  // A bound cancellation slot is still honoured.
  {
    boost::asio::io_context ctx;
    boost::asio::steady_timer timer(ctx, boost::asio::chrono::seconds(10));

    boost::system::error_code ec = boost::asio::error::would_block;
    boost::asio::cancellation_signal sig;
    {
      auto e = observed_wait(timer, ec, boost::asio::eager);
      std::move(e)(
          boost::asio::bind_cancellation_slot(
            sig.slot(), boost::asio::detached));
    }

    sig.emit(boost::asio::cancellation_type::all);
    ctx.run();

    BOOST_ASIO_CHECK(ec == boost::asio::error::operation_aborted);
  }
}

void eager_executor_test()
{
  // The waiting handler runs on its associated executor.
  {
    boost::asio::io_context ctx;
    boost::asio::strand<boost::asio::io_context::executor_type> st
      = boost::asio::make_strand(ctx);
    boost::asio::steady_timer timer(ctx);
    timer.expires_after(boost::asio::chrono::seconds(0));

    bool called = false;
    bool on_strand = false;

    auto e = timer.async_wait(boost::asio::eager);
    std::move(e)(
        boost::asio::bind_executor(st,
          [&](boost::system::error_code)
          {
            called = true;
            on_strand = st.running_in_this_thread();
          }));

    ctx.run();

    BOOST_ASIO_CHECK(called);
    BOOST_ASIO_CHECK(on_strand);
  }

  // The handler may belong to a different execution context.
  {
    boost::asio::io_context ctx1;
    boost::asio::io_context ctx2;
    boost::asio::steady_timer timer(ctx1);
    timer.expires_after(boost::asio::chrono::seconds(0));

    bool called = false;

    auto e = timer.async_wait(boost::asio::eager);
    std::move(e)(
        boost::asio::bind_executor(ctx2.get_executor(),
          [&](boost::system::error_code){ called = true; }));

    ctx1.run();

    BOOST_ASIO_CHECK(!called);

    ctx2.run();

    BOOST_ASIO_CHECK(called);
  }
}

void eager_type_erasure_test()
{
  typedef boost::asio::basic_waitable_timer<boost::asio::chrono::steady_clock,
      boost::asio::wait_traits<boost::asio::chrono::steady_clock>,
      boost::asio::io_context::executor_type> concrete_timer;

  typedef boost::asio::eager_async_operation<
      void(boost::system::error_code)> erased_op;

  // Operations with the same signature and executor share a handle type.
  {
    boost::asio::io_context ctx;
    boost::asio::steady_timer timer(ctx);

    BOOST_ASIO_CHECK((boost::asio::is_same<
          decltype(timer.async_wait(boost::asio::eager)), erased_op>::value));
  }

  // A concrete executor yields a different handle type, which converts.
  {
    boost::asio::io_context ctx;
    concrete_timer timer(ctx);
    timer.expires_after(boost::asio::chrono::seconds(0));

    BOOST_ASIO_CHECK((!boost::asio::is_same<
          decltype(timer.async_wait(boost::asio::eager)), erased_op>::value));

    bool called = false;
    erased_op e(timer.async_wait(boost::asio::eager));
    std::move(e)([&](boost::system::error_code){ called = true; });

    ctx.run();

    BOOST_ASIO_CHECK(called);
  }

  // Operations of differing kinds may be held in one container.
  {
    boost::asio::io_context ctx;
    boost::asio::steady_timer plain(ctx);
    concrete_timer concrete(ctx);
    plain.expires_after(boost::asio::chrono::seconds(0));
    concrete.expires_after(boost::asio::chrono::seconds(0));

    std::vector<erased_op> ops;
    ops.push_back(plain.async_wait(boost::asio::eager));
    ops.push_back(concrete.async_wait(boost::asio::eager));

    int called = 0;
    for (std::size_t i = 0; i < ops.size(); ++i)
      std::move(ops[i])([&](boost::system::error_code){ ++called; });

    ctx.run();

    BOOST_ASIO_CHECK(called == 2);
  }
}

void eager_move_assign_test()
{
  typedef boost::asio::eager_async_operation<
      void(boost::system::error_code)> erased_op;

  BOOST_ASIO_CHECK((boost::asio::is_nothrow_move_assignable<erased_op>::value));

  boost::asio::io_context ctx;
  boost::asio::steady_timer timer1(ctx);
  boost::asio::steady_timer timer2(ctx);
  timer1.expires_after(boost::asio::chrono::seconds(0));
  timer2.expires_after(boost::asio::chrono::seconds(0));

  boost::system::error_code ec1 = boost::asio::error::would_block;
  boost::system::error_code ec2 = boost::asio::error::would_block;

  auto e = observed_wait(timer1, ec1, boost::asio::eager);
  e = observed_wait(timer2, ec2, boost::asio::eager);

  bool called = false;
  std::move(e)([&](boost::system::error_code){ called = true; });

  ctx.run();

  BOOST_ASIO_CHECK_MESSAGE(!ec1, ec1.message());
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK_MESSAGE(!ec2, ec2.message());
}

void eager_initiation_test()
{
  // The initiation may complete before it returns.
  {
    boost::asio::io_context ctx;

    auto e = inline_completion(ctx, boost::asio::eager);
    BOOST_ASIO_CHECK(e.ready());

    int got = -1;
    std::move(e)([&](boost::system::error_code, int v){ got = v; });

    ctx.run();

    BOOST_ASIO_CHECK(got == 42);
  }

  // The initiation may move the handler it is given.
  {
    boost::asio::io_context ctx;

    auto e = moving_init(ctx, boost::asio::eager);
    BOOST_ASIO_CHECK(!e.ready());

    int got = -1;
    std::move(e)([&](boost::system::error_code, int v){ got = v; });

    ctx.run();

    BOOST_ASIO_CHECK(got == 99);
  }
}

void eager_deferred_test()
{
  // Dropping a deferred operation detaches, as dropping a handle does.
  {
    boost::asio::io_context ctx;
    boost::asio::steady_timer timer(ctx);
    timer.expires_after(boost::asio::chrono::seconds(0));

    boost::system::error_code ec = boost::asio::error::would_block;
    {
      auto d = observed_wait(timer, ec, boost::asio::eager)
        | boost::asio::deferred(deferred_link());
      (void)d;
    }

    ctx.run();

    BOOST_ASIO_CHECK_MESSAGE(!ec, ec.message());
  }

  // A deferred operation may be held and invoked later.
  {
    boost::asio::io_context ctx;
    boost::asio::steady_timer timer(ctx);
    timer.expires_after(boost::asio::chrono::milliseconds(50));

    boost::system::error_code ec = boost::asio::error::would_block;
    bool called = false;

    auto d = observed_wait(timer, ec, boost::asio::eager)
      | boost::asio::deferred(deferred_link());

    boost::asio::steady_timer later(ctx);
    later.expires_after(boost::asio::chrono::milliseconds(5));
    later.async_wait(
        [&](boost::system::error_code)
        {
          std::move(d)([&](boost::system::error_code){ called = true; });
        });

    ctx.run();

    BOOST_ASIO_CHECK(called);
    BOOST_ASIO_CHECK_MESSAGE(!ec, ec.message());
  }
}

void eager_idempotent_test()
{
  boost::asio::io_context ctx;
  boost::asio::steady_timer timer(ctx);

  BOOST_ASIO_CHECK((boost::asio::is_same<
        decltype(timer.async_wait(boost::asio::eager)),
        decltype(timer.async_wait(boost::asio::eager)(boost::asio::eager))
      >::value));

  BOOST_ASIO_CHECK((boost::asio::is_same<
        decltype(timer.async_wait(boost::asio::eager)),
        decltype(timer.async_wait(boost::asio::eager) | boost::asio::eager)
      >::value));

  // Re-applying the token delivers the result exactly once.
  {
    timer.expires_after(boost::asio::chrono::seconds(0));

    int count = 0;
    auto e = timer.async_wait(boost::asio::eager)(boost::asio::eager);
    std::move(e)([&](boost::system::error_code){ ++count; });

    ctx.run();

    BOOST_ASIO_CHECK(count == 1);
  }

  // Re-applying the token allocates no additional shared state.
  {
    ctx.restart();

    std::size_t one = 0;
    std::size_t four = 0;

    {
      boost::asio::steady_timer timer1(ctx, boost::asio::chrono::seconds(10));
      boost::asio::steady_timer timer2(ctx, boost::asio::chrono::seconds(10));

      auto e = timer1.async_wait(
          boost::asio::eager.rebind(counting_allocator<void>(&one)));

      auto f = timer2.async_wait(
          boost::asio::eager.rebind(counting_allocator<void>(&four)))(
            boost::asio::eager.rebind(counting_allocator<void>(&four)))(
            boost::asio::eager.rebind(counting_allocator<void>(&four)))(
            boost::asio::eager.rebind(counting_allocator<void>(&four)));

      BOOST_ASIO_CHECK(one != 0);
      BOOST_ASIO_CHECK(one == four);

      std::move(e)([](boost::system::error_code){});
      std::move(f)([](boost::system::error_code){});
    }

    ctx.run();

    BOOST_ASIO_CHECK(one == 0);
    BOOST_ASIO_CHECK(four == 0);
  }

  // An operation with an initiation of its own is not idempotent.
  {
    ctx.restart();

    std::size_t count = 0;

    {
      boost::asio::steady_timer timer3(ctx, boost::asio::chrono::seconds(10));

      auto d = timer3.async_wait(boost::asio::deferred);
      std::size_t before = count;

      auto e = std::move(d)(
          boost::asio::eager.rebind(counting_allocator<void>(&count)));

      BOOST_ASIO_CHECK(count > before);

      std::move(e)([](boost::system::error_code){});
    }

    ctx.run();

    BOOST_ASIO_CHECK(count == 0);
  }
}

void eager_allocation_failure_test()
{
#if !defined(BOOST_ASIO_NO_EXCEPTIONS)
  boost::asio::io_context ctx;
  boost::asio::steady_timer timer(ctx);
  timer.expires_after(boost::asio::chrono::seconds(0));

  boost::system::error_code ec = boost::asio::error::would_block;
  bool threw = false;

  try
  {
    auto e = observed_wait(timer, ec, boost::asio::eager);
    std::move(e)(
        boost::asio::bind_allocator(throwing_allocator<void>(),
          [](boost::system::error_code){}));
  }
  catch (const allocation_failure&)
  {
    threw = true;
  }

  ctx.run();

  BOOST_ASIO_CHECK(threw);
  BOOST_ASIO_CHECK_MESSAGE(!ec, ec.message());
#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)
}

#if defined(BOOST_ASIO_HAS_THREADS)

template <typename Handler>
struct manual_invoker
{
  Handler handler;

  void operator()(boost::system::error_code ec, int value)
  {
    static_cast<Handler&&>(handler)(ec, value);
  }
};

struct initiate_manual
{
  boost::asio::io_context* ctx;
  std::function<void(boost::system::error_code, int)>* sink;

  typedef boost::asio::io_context::executor_type executor_type;

  executor_type get_executor() const noexcept
  {
    return ctx->get_executor();
  }

  template <typename Handler>
  void operator()(Handler&& handler) const
  {
    *sink = manual_invoker<boost::asio::decay_t<Handler>>{
        static_cast<Handler&&>(handler)};
  }
};

template <typename CompletionToken>
auto manual_async(boost::asio::io_context& ctx,
    std::function<void(boost::system::error_code, int)>& sink,
    CompletionToken&& token)
  -> decltype(
      boost::asio::async_initiate<CompletionToken,
        void(boost::system::error_code, int)>(
          initiate_manual{&ctx, &sink}, token))
{
  return boost::asio::async_initiate<CompletionToken,
      void(boost::system::error_code, int)>(
        initiate_manual{&ctx, &sink}, token);
}

struct context_runner
{
  boost::asio::io_context* ctx;

  void operator()() { ctx->run(); }
};

struct completer
{
  std::atomic<int>* gate;
  std::function<void(boost::system::error_code, int)>* sink;
  int spin;

  void operator()()
  {
    gate->fetch_add(1);
    while (gate->load() < 2) {}
    for (int i = 0; i < spin; ++i)
      static_cast<void>(gate->load());
    (*sink)(boost::system::error_code(), 42);
  }
};

#endif // defined(BOOST_ASIO_HAS_THREADS)

void eager_concurrent_test()
{
#if defined(BOOST_ASIO_HAS_THREADS)
  const int iterations = 2000;

  boost::asio::io_context ctx;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work
    = boost::asio::make_work_guard(ctx);
  boost::asio::detail::thread runner(context_runner{&ctx});

  int completed = 0;

  for (int i = 0; i < iterations; ++i)
  {
    std::function<void(boost::system::error_code, int)> sink;
    auto op = manual_async(ctx, sink, boost::asio::eager);

    std::atomic<int> gate(0);
    std::atomic<bool> done(false);
    int got = -1;

    boost::asio::detail::thread other(completer{&gate, &sink, i % 8});

    gate.fetch_add(1);
    while (gate.load() < 2) {}
    for (int j = 0; j < i % 5; ++j)
      static_cast<void>(gate.load());

    std::move(op)(
        [&](boost::system::error_code, int value)
        {
          got = value;
          done.store(true);
        });

    other.join();
    while (!done.load()) {}

    if (got == 42)
      ++completed;
  }

  work.reset();
  runner.join();

  BOOST_ASIO_CHECK(completed == iterations);
#endif // defined(BOOST_ASIO_HAS_THREADS)
}

BOOST_ASIO_TEST_SUITE
(
  "eager",
  BOOST_ASIO_TEST_CASE(eager_test)
  BOOST_ASIO_TEST_CASE(eager_multi_signature_test)
  BOOST_ASIO_TEST_CASE(eager_detach_test)
  BOOST_ASIO_TEST_CASE(eager_cancel_test)
  BOOST_ASIO_TEST_CASE(eager_detached_test)
  BOOST_ASIO_TEST_CASE(eager_executor_test)
  BOOST_ASIO_TEST_CASE(eager_type_erasure_test)
  BOOST_ASIO_TEST_CASE(eager_move_assign_test)
  BOOST_ASIO_TEST_CASE(eager_initiation_test)
  BOOST_ASIO_TEST_CASE(eager_deferred_test)
  BOOST_ASIO_TEST_CASE(eager_idempotent_test)
  BOOST_ASIO_TEST_CASE(eager_allocation_failure_test)
  BOOST_ASIO_TEST_CASE(eager_concurrent_test)
)
