//
// detail/io_uring_service.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_IO_URING_SERVICE_HPP
#define BOOST_ASIO_DETAIL_IO_URING_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_IO_URING)

#include <liburing.h>
#include <boost/asio/detail/atomic_count.hpp>
#include <boost/asio/detail/buffer_sequence_adapter.hpp>
#include <boost/asio/detail/conditionally_enabled_mutex.hpp>
#include <boost/asio/detail/io_uring_operation.hpp>
#include <boost/asio/detail/limits.hpp>
#include <boost/asio/detail/object_pool.hpp>
#include <boost/asio/detail/op_queue.hpp>
#include <boost/asio/detail/reactor.hpp>
#include <boost/asio/detail/scheduler_task.hpp>
#include <boost/asio/detail/timer_queue_base.hpp>
#include <boost/asio/detail/timer_queue_set.hpp>
#include <boost/asio/detail/wait_op.hpp>
#include <boost/asio/execution_context.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

class io_uring_service
  : public execution_context_service_base<io_uring_service>,
    public scheduler_task
{
private:
  // The mutex type used by this reactor.
  typedef conditionally_enabled_mutex mutex;

public:
  enum op_types { read_op = 0, write_op = 1, except_op = 2, max_ops = 3 };

  class io_object;

  // An I/O queue stores operations that must run serially.
  class io_queue : operation
  {
    friend class io_uring_service;

    io_object* io_object_;
    op_queue<io_uring_operation> op_queue_;
    bool cancel_requested_;

    BOOST_ASIO_DECL io_queue();
    void set_result(int r) { task_result_ = static_cast<unsigned>(r); }
    BOOST_ASIO_DECL operation* perform_io(int result);
    BOOST_ASIO_DECL static void do_complete(void* owner, operation* base,
        const boost::system::error_code& ec, std::size_t bytes_transferred);
  };

  // Per I/O object state.
  struct io_object
  {
    io_object* next_;
    io_object* prev_;

    mutex mutex_;
    io_uring_service* service_;
    io_queue queues_[max_ops];
    bool shutdown_;

    BOOST_ASIO_DECL io_object(bool locking, int spin_count);
  };

  // Per I/O object data.
  typedef io_object* per_io_object_data;

  // Constructor.
  BOOST_ASIO_DECL io_uring_service(boost::asio::execution_context& ctx);

  // Destructor.
  BOOST_ASIO_DECL ~io_uring_service();

  // Destroy all user-defined handler objects owned by the service.
  BOOST_ASIO_DECL void shutdown();

  // Recreate internal state following a fork.
  BOOST_ASIO_DECL void notify_fork(
      boost::asio::execution_context::fork_event fork_ev);

  // Initialise the task.
  BOOST_ASIO_DECL void init_task();

  // Register an I/O object with io_uring.
  BOOST_ASIO_DECL void register_io_object(io_object*& io_obj);

  // Register an internal I/O object with io_uring.
  BOOST_ASIO_DECL void register_internal_io_object(
      io_object*& io_obj, int op_type, io_uring_operation* op);

  // Register buffers with io_uring.
  BOOST_ASIO_DECL void register_buffers(const ::iovec* v, unsigned n);

  // Unregister buffers from io_uring.
  BOOST_ASIO_DECL void unregister_buffers();

  // Post an operation for immediate completion.
  void post_immediate_completion(operation* op, bool is_continuation);

  // Start a new operation. The operation will be prepared and submitted to the
  // io_uring when it is at the head of its I/O operation queue.
  BOOST_ASIO_DECL void start_op(int op_type, per_io_object_data& io_obj,
      io_uring_operation* op, bool is_continuation);

  // Cancel all operations associated with the given I/O object. The handlers
  // associated with the I/O object will be invoked with the operation_aborted
  // error.
  BOOST_ASIO_DECL void cancel_ops(per_io_object_data& io_obj);

  // Cancel all operations associated with the given I/O object and key. The
  // handlers associated with the object and key will be invoked with the
  // operation_aborted error.
  BOOST_ASIO_DECL void cancel_ops_by_key(per_io_object_data& io_obj,
      int op_type, void* cancellation_key);

  // Cancel any operations that are running against the I/O object and remove
  // its registration from the service. The service resources associated with
  // the I/O object must be released by calling cleanup_io_object.
  BOOST_ASIO_DECL void deregister_io_object(per_io_object_data& io_obj);

  // Perform any post-deregistration cleanup tasks associated with the I/O
  // object.
  BOOST_ASIO_DECL void cleanup_io_object(per_io_object_data& io_obj);

  // Add a new timer queue to the reactor.
  template <typename TimeTraits, typename Allocator>
  void add_timer_queue(timer_queue<TimeTraits, Allocator>& timer_queue);

  // Remove a timer queue from the reactor.
  template <typename TimeTraits, typename Allocator>
  void remove_timer_queue(timer_queue<TimeTraits, Allocator>& timer_queue);

  // Schedule a new operation in the given timer queue to expire at the
  // specified absolute time.
  template <typename TimeTraits, typename Allocator>
  void schedule_timer(timer_queue<TimeTraits, Allocator>& queue,
      const typename TimeTraits::time_type& time,
      typename timer_queue<TimeTraits, Allocator>::per_timer_data& timer,
      wait_op* op);

  // Cancel the timer operations associated with the given token. Returns the
  // number of operations that have been posted or dispatched.
  template <typename TimeTraits, typename Allocator>
  std::size_t cancel_timer(timer_queue<TimeTraits, Allocator>& queue,
      typename timer_queue<TimeTraits, Allocator>::per_timer_data& timer,
      std::size_t max_cancelled = (std::numeric_limits<std::size_t>::max)());

  // Cancel the timer operations associated with the given key.
  template <typename TimeTraits, typename Allocator>
  void cancel_timer_by_key(timer_queue<TimeTraits, Allocator>& queue,
      typename timer_queue<TimeTraits, Allocator>::per_timer_data* timer,
      void* cancellation_key);

  // Move the timer operations associated with the given timer.
  template <typename TimeTraits, typename Allocator>
  void move_timer(timer_queue<TimeTraits, Allocator>& queue,
      typename timer_queue<TimeTraits, Allocator>::per_timer_data& target,
      typename timer_queue<TimeTraits, Allocator>::per_timer_data& source);

  // Wait on io_uring once until interrupted or events are ready to be
  // dispatched.
  BOOST_ASIO_DECL void run(long usec, op_queue<operation>& ops);

  // Interrupt the io_uring wait.
  BOOST_ASIO_DECL void interrupt();

private:
  // The hint to pass to io_uring_queue_init to size its data structures.
  enum { ring_size = 16384 };

  // The number of operations to submit in a batch.
  enum { submit_batch_size = 128 };

  // The number of operations to complete in a batch.
  enum { complete_batch_size = 128 };

  // The type used for processing eventfd readiness notifications.
  class event_fd_read_op;

  // Initialise the ring.
  BOOST_ASIO_DECL void init_ring();

  // Register the eventfd descriptor for readiness notifications.
  BOOST_ASIO_DECL void register_with_reactor();

  // Allocate a new I/O object.
  BOOST_ASIO_DECL io_object* allocate_io_object();

  // Free an existing I/O object.
  BOOST_ASIO_DECL void free_io_object(io_object* s);

  // Helper function to cancel all operations associated with the given I/O
  // object. This function must be called while the I/O object's mutex is held.
  // Returns true if there are operations for which cancellation is pending.
  BOOST_ASIO_DECL bool do_cancel_ops(
      per_io_object_data& io_obj, op_queue<operation>& ops);

  // Helper function to add a new timer queue.
  BOOST_ASIO_DECL void do_add_timer_queue(timer_queue_base& queue);

  // Helper function to remove a timer queue.
  BOOST_ASIO_DECL void do_remove_timer_queue(timer_queue_base& queue);

  // Called to recalculate and update the timeout.
  BOOST_ASIO_DECL void update_timeout();

  // Get the current timeout value.
  BOOST_ASIO_DECL __kernel_timespec get_timeout() const;

  // Get a new submission queue entry, flushing the queue if necessary.
  BOOST_ASIO_DECL ::io_uring_sqe* get_sqe();

  // Submit pending submission queue entries.
  BOOST_ASIO_DECL void submit_sqes();

  // Post an operation to submit the pending submission queue entries.
  BOOST_ASIO_DECL void post_submit_sqes_op(mutex::scoped_lock& lock);

  // Push an operation to submit the pending submission queue entries.
  BOOST_ASIO_DECL void push_submit_sqes_op(op_queue<operation>& ops);

  // Helper operation to submit pending submission queue entries.
  class submit_sqes_op : operation
  {
    friend class io_uring_service;

    io_uring_service* service_;

    BOOST_ASIO_DECL submit_sqes_op(io_uring_service* s);
    BOOST_ASIO_DECL static void do_complete(void* owner, operation* base,
        const boost::system::error_code& ec, std::size_t bytes_transferred);
  };

  // The scheduler implementation used to post completions.
  scheduler& scheduler_;

  // Mutex to protect access to internal data.
  mutex mutex_;

  // The ring.
  ::io_uring ring_;

  // The count of unfinished work.
  atomic_count outstanding_work_;

  // The operation used to submit the pending submission queue entries.
  submit_sqes_op submit_sqes_op_;

  // The number of pending submission queue entries_.
  int pending_sqes_;

  // Whether there is a pending submission operation.
  bool pending_submit_sqes_op_;

  // Whether the service has been shut down.
  bool shutdown_;

  // Whether I/O locking is enabled.
  const bool io_locking_;

  // How any times to spin waiting for the I/O mutex.
  const int io_locking_spin_count_;

  // The timer queues.
  timer_queue_set timer_queues_;

  // The timespec for the pending timeout operation. Must remain valid while the
  // operation is outstanding.
  __kernel_timespec timeout_;

  // Mutex to protect access to the registered I/O objects.
  mutex registration_mutex_;

  // Keep track of all registered I/O objects.
  object_pool<io_object, execution_context::allocator<void>>
    registered_io_objects_;

  // Helper class to do post-perform_io cleanup.
  struct perform_io_cleanup_on_block_exit;
  friend struct perform_io_cleanup_on_block_exit;

  // The reactor used to register for eventfd readiness.
  reactor& reactor_;

  // The per-descriptor reactor data used for the eventfd.
  reactor::per_descriptor_data reactor_data_;

  // The eventfd descriptor used to wait for readiness.
  int event_fd_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/detail/impl/io_uring_service.hpp>
#if defined(BOOST_ASIO_HEADER_ONLY)
# include <boost/asio/detail/impl/io_uring_service.ipp>
#endif // defined(BOOST_ASIO_HEADER_ONLY)

#endif // defined(BOOST_ASIO_HAS_IO_URING)

#endif // BOOST_ASIO_DETAIL_IO_URING_SERVICE_HPP
