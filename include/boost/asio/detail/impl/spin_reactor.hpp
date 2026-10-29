//
// detail/impl/spin_reactor.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_IMPL_SPIN_REACTOR_HPP
#define BOOST_ASIO_DETAIL_IMPL_SPIN_REACTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(BOOST_ASIO_ENABLE_SPIN_REACTOR) \
  && !defined(BOOST_ASIO_HAS_IOCP) \
  && !defined(BOOST_ASIO_WINDOWS_RUNTIME) \
  && !defined(BOOST_ASIO_HAS_IO_URING_AS_DEFAULT)

#include <boost/asio/detail/scheduler.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
BOOST_ASIO_INLINE_NAMESPACE_BEGIN
namespace detail {

inline void spin_reactor::post_immediate_completion(
    operation* op, bool is_continuation) const
{
  scheduler_.post_immediate_completion(op, is_continuation);
}

template <int OpType, reactor_op::perform_func_type Perform>
inline bool spin_reactor::start_op(socket_type descriptor,
    spin_reactor::per_descriptor_data& descriptor_data, reactor_op* op)
{
  (void)descriptor;

  if (BOOST_ASIO_UNLIKELY(!descriptor_data))
    return start_bad_descriptor_op(op);

  descriptor_data->mutex_.lock();
  mutex::scoped_lock descriptor_lock(descriptor_data->mutex_,
      mutex::scoped_lock::adopt_lock);

  if (Perform != nullptr
      && descriptor_data->op_queue_[OpType].empty()
      && (OpType != read_op
        || descriptor_data->op_queue_[except_op].empty()))
  {
    if (Perform(op))
    {
      descriptor_lock.unlock();
      return true;
    }
  }

  return start_unready_op(OpType, descriptor_data,
      descriptor_lock, op, Perform);
}

template <typename TimeTraits, typename Allocator>
void spin_reactor::add_timer_queue(
    timer_queue<TimeTraits, Allocator>& queue)
{
  do_add_timer_queue(queue);
}

template <typename TimeTraits, typename Allocator>
void spin_reactor::remove_timer_queue(
    timer_queue<TimeTraits, Allocator>& queue)
{
  do_remove_timer_queue(queue);
}

template <typename TimeTraits, typename Allocator>
void spin_reactor::schedule_timer(timer_queue<TimeTraits, Allocator>& queue,
    const typename TimeTraits::time_type& time,
    typename timer_queue<TimeTraits, Allocator>::per_timer_data& timer,
    wait_op* op)
{
  mutex::scoped_lock lock(mutex_);

  if (shutdown_)
  {
    scheduler_.post_immediate_completion(op, false);
    return;
  }

  queue.enqueue_timer(time, timer, op);
  scheduler_.work_started();
}

template <typename TimeTraits, typename Allocator>
std::size_t spin_reactor::cancel_timer(
    timer_queue<TimeTraits, Allocator>& queue,
    typename timer_queue<TimeTraits, Allocator>::per_timer_data& timer,
    std::size_t max_cancelled)
{
  mutex::scoped_lock lock(mutex_);
  op_queue<operation> ops;
  std::size_t n = queue.cancel_timer(timer, ops, max_cancelled);
  lock.unlock();
  scheduler_.post_deferred_completions(ops);
  return n;
}

template <typename TimeTraits, typename Allocator>
void spin_reactor::cancel_timer_by_key(
    timer_queue<TimeTraits, Allocator>& queue,
    typename timer_queue<TimeTraits, Allocator>::per_timer_data* timer,
    void* cancellation_key)
{
  mutex::scoped_lock lock(mutex_);
  op_queue<operation> ops;
  queue.cancel_timer_by_key(timer, ops, cancellation_key);
  lock.unlock();
  scheduler_.post_deferred_completions(ops);
}

template <typename TimeTraits, typename Allocator>
void spin_reactor::move_timer(timer_queue<TimeTraits, Allocator>& queue,
    typename timer_queue<TimeTraits, Allocator>::per_timer_data& target,
    typename timer_queue<TimeTraits, Allocator>::per_timer_data& source)
{
  mutex::scoped_lock lock(mutex_);
  op_queue<operation> ops;
  queue.cancel_timer(target, ops);
  queue.move_timer(target, source);
  lock.unlock();
  scheduler_.post_deferred_completions(ops);
}

} // namespace detail
BOOST_ASIO_INLINE_NAMESPACE_END
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_ENABLE_SPIN_REACTOR)
       //   && !defined(BOOST_ASIO_HAS_IOCP)
       //   && !defined(BOOST_ASIO_WINDOWS_RUNTIME)
       //   && !defined(BOOST_ASIO_HAS_IO_URING_AS_DEFAULT)

#endif // BOOST_ASIO_DETAIL_IMPL_SPIN_REACTOR_HPP
