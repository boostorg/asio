//
// thread_pool.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_THREAD_POOL_HPP
#define BOOST_ASIO_THREAD_POOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/detail/atomic_count.hpp>
#include <boost/asio/detail/scheduler.hpp>
#include <boost/asio/detail/thread_group.hpp>
#include <boost/asio/execution.hpp>
#include <boost/asio/execution_context.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {
  struct thread_pool_bits
  {
    BOOST_ASIO_STATIC_CONSTEXPR(unsigned int, blocking_never = 1);
    BOOST_ASIO_STATIC_CONSTEXPR(unsigned int, blocking_always = 2);
    BOOST_ASIO_STATIC_CONSTEXPR(unsigned int, blocking_mask = 3);
    BOOST_ASIO_STATIC_CONSTEXPR(unsigned int, relationship_continuation = 4);
    BOOST_ASIO_STATIC_CONSTEXPR(unsigned int, outstanding_work_tracked = 8);
  };
} // namespace detail

/// A simple fixed-size thread pool.
/**
 * The thread pool class is an execution context where functions are permitted
 * to run on one of a fixed number of threads.
 *
 * @par Submitting tasks to the pool
 *
 * To submit functions to the thread pool, use the @ref boost::asio::dispatch,
 * @ref boost::asio::post or @ref boost::asio::defer free functions.
 *
 * For example:
 *
 * @code void my_task()
 * {
 *   ...
 * }
 *
 * ...
 *
 * // Launch the pool with four threads.
 * boost::asio::thread_pool pool(4);
 *
 * // Submit a function to the pool.
 * boost::asio::post(pool, my_task);
 *
 * // Submit a lambda object to the pool.
 * boost::asio::post(pool,
 *     []()
 *     {
 *       ...
 *     });
 *
 * // Wait for all tasks in the pool to complete.
 * pool.join(); @endcode
 */
class thread_pool
  : public execution_context
{
public:
  template <typename Allocator, unsigned int Bits>
  class basic_executor_type;

  template <typename Allocator, unsigned int Bits>
  friend class basic_executor_type;

  /// Executor used to submit functions to a thread pool.
  typedef basic_executor_type<std::allocator<void>, 0> executor_type;

#if !defined(BOOST_ASIO_NO_TS_EXECUTORS)
  /// Constructs a pool with an automatically determined number of threads.
  BOOST_ASIO_DECL thread_pool();
#endif // !defined(BOOST_ASIO_NO_TS_EXECUTORS)

  /// Constructs a pool with a specified number of threads.
  BOOST_ASIO_DECL thread_pool(std::size_t num_threads);

  /// Destructor.
  /**
   * Automatically stops and joins the pool, if not explicitly done beforehand.
   */
  BOOST_ASIO_DECL ~thread_pool();

  /// Obtains the executor associated with the pool.
  executor_type get_executor() BOOST_ASIO_NOEXCEPT;

  /// Obtains the executor associated with the pool.
  executor_type executor() BOOST_ASIO_NOEXCEPT;

  /// Stops the threads.
  /**
   * This function stops the threads as soon as possible. As a result of calling
   * @c stop(), pending function objects may be never be invoked.
   */
  BOOST_ASIO_DECL void stop();

  /// Attaches the current thread to the pool.
  /**
   * This function attaches the current thread to the pool so that it may be
   * used for executing submitted function objects. Blocks the calling thread
   * until the pool is stopped or joined and has no outstanding work.
   */
  BOOST_ASIO_DECL void attach();

  /// Joins the threads.
  /**
   * This function blocks until the threads in the pool have completed. If @c
   * stop() is not called prior to @c join(), the @c join() call will wait
   * until the pool has no more outstanding work.
   */
  BOOST_ASIO_DECL void join();

  /// Waits for threads to complete.
  /**
   * This function blocks until the threads in the pool have completed. If @c
   * stop() is not called prior to @c wait(), the @c wait() call will wait
   * until the pool has no more outstanding work.
   */
  BOOST_ASIO_DECL void wait();

private:
  thread_pool(const thread_pool&) BOOST_ASIO_DELETED;
  thread_pool& operator=(const thread_pool&) BOOST_ASIO_DELETED;

  struct thread_function;

  // Helper function to create the underlying scheduler.
  BOOST_ASIO_DECL detail::scheduler& add_scheduler(detail::scheduler* s);

  // The underlying scheduler.
  detail::scheduler& scheduler_;

  // The threads in the pool.
  detail::thread_group threads_;

  // The current number of threads in the pool.
  detail::atomic_count num_threads_;
};

/// Executor implementation type used to submit functions to a thread pool.
template <typename Allocator, unsigned int Bits>
class thread_pool::basic_executor_type : detail::thread_pool_bits
{
public:
  /// Copy construtor.
  basic_executor_type(
      const basic_executor_type& other) BOOST_ASIO_NOEXCEPT
    : pool_(other.pool_),
      allocator_(other.allocator_),
      bits_(other.bits_)
  {
    if (Bits & outstanding_work_tracked)
      if (pool_)
        pool_->scheduler_.work_started();
  }

#if defined(BOOST_ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move construtor.
  basic_executor_type(basic_executor_type&& other) BOOST_ASIO_NOEXCEPT
    : pool_(other.pool_),
      allocator_(BOOST_ASIO_MOVE_CAST(Allocator)(other.allocator_)),
      bits_(other.bits_)
  {
    if (Bits & outstanding_work_tracked)
      other.pool_ = 0;
  }
#endif // defined(BOOST_ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Destructor.
  ~basic_executor_type()
  {
    if (Bits & outstanding_work_tracked)
      if (pool_)
        pool_->scheduler_.work_finished();
  }

  /// Assignment operator.
  basic_executor_type& operator=(
      const basic_executor_type& other) BOOST_ASIO_NOEXCEPT;

#if defined(BOOST_ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move assignment operator.
  basic_executor_type& operator=(
      basic_executor_type&& other) BOOST_ASIO_NOEXCEPT;
#endif // defined(BOOST_ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Obtain an executor with the @c blocking.possibly property.
  BOOST_ASIO_CONSTEXPR basic_executor_type<Allocator,
      BOOST_ASIO_UNSPECIFIED(Bits & ~blocking_mask)>
  require(execution::blocking_t::possibly_t) const
  {
    return basic_executor_type<Allocator, Bits & ~blocking_mask>(
        pool_, allocator_, bits_ & ~blocking_mask);
  }

  /// Obtain an executor with the @c blocking.always property.
  BOOST_ASIO_CONSTEXPR basic_executor_type<Allocator,
      BOOST_ASIO_UNSPECIFIED((Bits & ~blocking_mask) | blocking_always)>
  require(execution::blocking_t::always_t) const
  {
    return basic_executor_type<Allocator,
        BOOST_ASIO_UNSPECIFIED((Bits & ~blocking_mask) | blocking_always)>(
          pool_, allocator_, bits_ & ~blocking_mask);
  }

  /// Obtain an executor with the @c blocking.never property.
  BOOST_ASIO_CONSTEXPR basic_executor_type<Allocator,
      BOOST_ASIO_UNSPECIFIED(Bits & ~blocking_mask)>
  require(execution::blocking_t::never_t) const
  {
    return basic_executor_type<Allocator, Bits & ~blocking_mask>(
        pool_, allocator_, (bits_ & ~blocking_mask) | blocking_never);
  }

  /// Obtain an executor with the @c relationship.fork property.
  BOOST_ASIO_CONSTEXPR basic_executor_type require(
      execution::relationship_t::fork_t) const
  {
    return basic_executor_type(pool_,
        allocator_, bits_ & ~relationship_continuation);
  }

  /// Obtain an executor with the @c relationship.continuation property.
  BOOST_ASIO_CONSTEXPR basic_executor_type require(
      execution::relationship_t::continuation_t) const
  {
    return basic_executor_type(pool_,
        allocator_, bits_ | relationship_continuation);
  }

  /// Obtain an executor with the @c outstanding_work.tracked property.
  BOOST_ASIO_CONSTEXPR basic_executor_type<Allocator,
      BOOST_ASIO_UNSPECIFIED(Bits | outstanding_work_tracked)>
  require(execution::outstanding_work_t::tracked_t) const
  {
    return basic_executor_type<Allocator, Bits | outstanding_work_tracked>(
        pool_, allocator_, bits_);
  }

  /// Obtain an executor with the @c outstanding_work.untracked property.
  BOOST_ASIO_CONSTEXPR basic_executor_type<Allocator,
      BOOST_ASIO_UNSPECIFIED(Bits & ~outstanding_work_tracked)>
  require(execution::outstanding_work_t::untracked_t) const
  {
    return basic_executor_type<Allocator, Bits & ~outstanding_work_tracked>(
        pool_, allocator_, bits_);
  }

  /// Obtain an executor with the specified @c allocator property.
  template <typename OtherAllocator>
  BOOST_ASIO_CONSTEXPR basic_executor_type<OtherAllocator, Bits>
  require(execution::allocator_t<OtherAllocator> a) const
  {
    return basic_executor_type<OtherAllocator, Bits>(
        pool_, a.value(), bits_);
  }

  /// Obtain an executor with the default @c allocator property.
  BOOST_ASIO_CONSTEXPR basic_executor_type<std::allocator<void>, Bits>
  require(execution::allocator_t<void>) const
  {
    return basic_executor_type<std::allocator<void>, Bits>(
        pool_, std::allocator<void>(), bits_);
  }

  /// Query the current value of the @c bulk_guarantee property.
  static BOOST_ASIO_CONSTEXPR execution::bulk_guarantee_t query(
      execution::bulk_guarantee_t) BOOST_ASIO_NOEXCEPT
  {
    return execution::bulk_guarantee.parallel;
  }

  /// Query the current value of the @c mapping property.
  static BOOST_ASIO_CONSTEXPR execution::mapping_t query(
      execution::mapping_t) BOOST_ASIO_NOEXCEPT
  {
    return execution::mapping.thread;
  }

  /// Query the current value of the @c context property.
  thread_pool& query(execution::context_t) const BOOST_ASIO_NOEXCEPT
  {
    return *pool_;
  }

  /// Query the current value of the @c blocking property.
  BOOST_ASIO_CONSTEXPR execution::blocking_t query(
      execution::blocking_t) const BOOST_ASIO_NOEXCEPT
  {
    return (bits_ & blocking_never)
      ? execution::blocking_t(execution::blocking.never)
      : ((Bits & blocking_always)
          ? execution::blocking_t(execution::blocking.always)
          : execution::blocking_t(execution::blocking.possibly));
  }

  /// Query the current value of the @c relationship property.
  BOOST_ASIO_CONSTEXPR execution::relationship_t query(
      execution::relationship_t) const BOOST_ASIO_NOEXCEPT
  {
    return (bits_ & relationship_continuation)
      ? execution::relationship_t(execution::relationship.continuation)
      : execution::relationship_t(execution::relationship.fork);
  }

  /// Query the current value of the @c outstanding_work property.
  static BOOST_ASIO_CONSTEXPR execution::outstanding_work_t query(
      execution::outstanding_work_t) BOOST_ASIO_NOEXCEPT
  {
    return (Bits & outstanding_work_tracked)
      ? execution::outstanding_work_t(execution::outstanding_work.tracked)
      : execution::outstanding_work_t(execution::outstanding_work.untracked);
  }

  /// Query the current value of the @c allocator property.
  template <typename OtherAllocator>
  BOOST_ASIO_CONSTEXPR Allocator query(
      execution::allocator_t<OtherAllocator>) const BOOST_ASIO_NOEXCEPT
  {
    return allocator_;
  }

  /// Query the current value of the @c allocator property.
  BOOST_ASIO_CONSTEXPR Allocator query(
      execution::allocator_t<void>) const BOOST_ASIO_NOEXCEPT
  {
    return allocator_;
  }

  /// Query the occupancy (recommended number of work items) for the pool.
  std::size_t query(execution::occupancy_t) const BOOST_ASIO_NOEXCEPT
  {
    return static_cast<std::size_t>(pool_->num_threads_);
  }

  /// Determine whether the thread pool is running in the current thread.
  /**
   * @return @c true if the current thread is running the thread pool. Otherwise
   * returns @c false.
   */
  bool running_in_this_thread() const BOOST_ASIO_NOEXCEPT;

  /// Compare two executors for equality.
  /**
   * Two executors are equal if they refer to the same underlying thread pool.
   */
  friend bool operator==(const basic_executor_type& a,
      const basic_executor_type& b) BOOST_ASIO_NOEXCEPT
  {
    return a.pool_ == b.pool_
      && a.allocator_ == b.allocator_
      && a.bits_ == b.bits_;
  }

  /// Compare two executors for inequality.
  /**
   * Two executors are equal if they refer to the same underlying thread pool.
   */
  friend bool operator!=(const basic_executor_type& a,
      const basic_executor_type& b) BOOST_ASIO_NOEXCEPT
  {
    return a.pool_ != b.pool_
      || a.allocator_ != b.allocator_
      || a.bits_ != b.bits_;
  }

  /// Execution function.
  template <typename Function>
  void execute(BOOST_ASIO_MOVE_ARG(Function) f) const
  {
    this->do_execute(BOOST_ASIO_MOVE_CAST(Function)(f),
        integral_constant<bool, (Bits & blocking_always) != 0>());
  }

#if !defined(BOOST_ASIO_NO_TS_EXECUTORS)
  /// Obtain the underlying execution context.
  thread_pool& context() const BOOST_ASIO_NOEXCEPT;

  /// Inform the thread pool that it has some outstanding work to do.
  /**
   * This function is used to inform the thread pool that some work has begun.
   * This ensures that the thread pool's join() function will not return while
   * the work is underway.
   */
  void on_work_started() const BOOST_ASIO_NOEXCEPT;

  /// Inform the thread pool that some work is no longer outstanding.
  /**
   * This function is used to inform the thread pool that some work has
   * finished. Once the count of unfinished work reaches zero, the thread
   * pool's join() function is permitted to exit.
   */
  void on_work_finished() const BOOST_ASIO_NOEXCEPT;

  /// Request the thread pool to invoke the given function object.
  /**
   * This function is used to ask the thread pool to execute the given function
   * object. If the current thread belongs to the pool, @c dispatch() executes
   * the function before returning. Otherwise, the function will be scheduled
   * to run on the thread pool.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
  template <typename Function, typename OtherAllocator>
  void dispatch(BOOST_ASIO_MOVE_ARG(Function) f,
      const OtherAllocator& a) const;

  /// Request the thread pool to invoke the given function object.
  /**
   * This function is used to ask the thread pool to execute the given function
   * object. The function object will never be executed inside @c post().
   * Instead, it will be scheduled to run on the thread pool.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
  template <typename Function, typename OtherAllocator>
  void post(BOOST_ASIO_MOVE_ARG(Function) f,
      const OtherAllocator& a) const;

  /// Request the thread pool to invoke the given function object.
  /**
   * This function is used to ask the thread pool to execute the given function
   * object. The function object will never be executed inside @c defer().
   * Instead, it will be scheduled to run on the thread pool.
   *
   * If the current thread belongs to the thread pool, @c defer() will delay
   * scheduling the function object until the current thread returns control to
   * the pool.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
  template <typename Function, typename OtherAllocator>
  void defer(BOOST_ASIO_MOVE_ARG(Function) f,
      const OtherAllocator& a) const;
#endif // !defined(BOOST_ASIO_NO_TS_EXECUTORS)

private:
  friend class thread_pool;
  template <typename, unsigned int> friend class basic_executor_type;

  // Constructor used by thread_pool::get_executor().
  explicit basic_executor_type(thread_pool& p) BOOST_ASIO_NOEXCEPT
    : pool_(&p),
      allocator_(),
      bits_(0)
  {
    if (Bits & outstanding_work_tracked)
      pool_->scheduler_.work_started();
  }

  // Constructor used by require().
  basic_executor_type(thread_pool* p,
      const Allocator& a, unsigned int bits) BOOST_ASIO_NOEXCEPT
    : pool_(p),
      allocator_(a),
      bits_(bits)
  {
    if (Bits & outstanding_work_tracked)
      if (pool_)
        pool_->scheduler_.work_started();
  }

  /// Execution helper implementation for possibly and never blocking.
  template <typename Function>
  void do_execute(BOOST_ASIO_MOVE_ARG(Function) f, false_type) const;

  /// Execution helper implementation for always blocking.
  template <typename Function>
  void do_execute(BOOST_ASIO_MOVE_ARG(Function) f, true_type) const;

  // The underlying thread pool.
  thread_pool* pool_;

  // The allocator used for execution functions.
  Allocator allocator_;

  // The runtime-switched properties of the thread pool executor.
  unsigned int bits_;
};

#if !defined(GENERATING_DOCUMENTATION)

namespace execution {

#if !defined(BOOST_ASIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

template <typename Allocator, unsigned int Bits>
struct is_executor<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>
  > : true_type
{
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

} // namespace execution
namespace traits {

#if !defined(BOOST_ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

template <typename Allocator, unsigned int Bits, typename Function>
struct execute_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    Function
  >
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef void result_type;
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

#if !defined(BOOST_ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

template <typename Allocator, unsigned int Bits>
struct require_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    boost::asio::execution::blocking_t::possibly_t
  > : boost::asio::detail::thread_pool_bits
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef boost::asio::thread_pool::basic_executor_type<
      Allocator, Bits & ~blocking_mask> result_type;
};

template <typename Allocator, unsigned int Bits>
struct require_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    boost::asio::execution::blocking_t::always_t
  > : boost::asio::detail::thread_pool_bits
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef boost::asio::thread_pool::basic_executor_type<Allocator,
      (Bits & ~blocking_mask) | blocking_always> result_type;
};

template <typename Allocator, unsigned int Bits>
struct require_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    boost::asio::execution::blocking_t::never_t
  > : boost::asio::detail::thread_pool_bits
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef boost::asio::thread_pool::basic_executor_type<
      Allocator, Bits & ~blocking_mask> result_type;
};

template <typename Allocator, unsigned int Bits>
struct require_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    boost::asio::execution::relationship_t::fork_t
  >
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef boost::asio::thread_pool::basic_executor_type<
      Allocator, Bits> result_type;
};

template <typename Allocator, unsigned int Bits>
struct require_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    boost::asio::execution::relationship_t::continuation_t
  >
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef boost::asio::thread_pool::basic_executor_type<
      Allocator, Bits> result_type;
};

template <typename Allocator, unsigned int Bits>
struct require_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    boost::asio::execution::outstanding_work_t::tracked_t
  > : boost::asio::detail::thread_pool_bits
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef boost::asio::thread_pool::basic_executor_type<
      Allocator, Bits | outstanding_work_tracked> result_type;
};

template <typename Allocator, unsigned int Bits>
struct require_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    boost::asio::execution::outstanding_work_t::untracked_t
  > : boost::asio::detail::thread_pool_bits
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef boost::asio::thread_pool::basic_executor_type<
      Allocator, Bits & ~outstanding_work_tracked> result_type;
};

template <typename Allocator, unsigned int Bits>
struct require_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    boost::asio::execution::allocator_t<void>
  >
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef boost::asio::thread_pool::basic_executor_type<
      std::allocator<void>, Bits> result_type;
};

template <unsigned int Bits,
    typename Allocator, typename OtherAllocator>
struct require_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    boost::asio::execution::allocator_t<OtherAllocator>
  >
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
  typedef boost::asio::thread_pool::basic_executor_type<
      OtherAllocator, Bits> result_type;
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

#if !defined(BOOST_ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

template <typename Allocator, unsigned int Bits, typename Property>
struct query_static_constexpr_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    Property,
    typename boost::asio::enable_if<
      boost::asio::is_convertible<
        Property,
        boost::asio::execution::bulk_guarantee_t
      >::value
    >::type
  >
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef boost::asio::execution::bulk_guarantee_t::parallel_t result_type;

  static BOOST_ASIO_CONSTEXPR result_type value() BOOST_ASIO_NOEXCEPT
  {
    return result_type();
  }
};

template <typename Allocator, unsigned int Bits, typename Property>
struct query_static_constexpr_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    Property,
    typename boost::asio::enable_if<
      boost::asio::is_convertible<
        Property,
        boost::asio::execution::outstanding_work_t
      >::value
    >::type
  > : boost::asio::detail::thread_pool_bits
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef boost::asio::execution::outstanding_work_t result_type;

  static BOOST_ASIO_CONSTEXPR result_type value() BOOST_ASIO_NOEXCEPT
  {
    return (Bits & outstanding_work_tracked)
      ? execution::outstanding_work_t(execution::outstanding_work.tracked)
      : execution::outstanding_work_t(execution::outstanding_work.untracked);
  }
};

template <typename Allocator, unsigned int Bits, typename Property>
struct query_static_constexpr_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    Property,
    typename boost::asio::enable_if<
      boost::asio::is_convertible<
        Property,
        boost::asio::execution::mapping_t
      >::value
    >::type
  >
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef boost::asio::execution::mapping_t::thread_t result_type;

  static BOOST_ASIO_CONSTEXPR result_type value() BOOST_ASIO_NOEXCEPT
  {
    return result_type();
  }
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

#if !defined(BOOST_ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

template <typename Allocator, unsigned int Bits, typename Property>
struct query_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    Property,
    typename boost::asio::enable_if<
      boost::asio::is_convertible<
        Property,
        boost::asio::execution::blocking_t
      >::value
    >::type
  >
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef boost::asio::execution::blocking_t result_type;
};

template <typename Allocator, unsigned int Bits, typename Property>
struct query_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    Property,
    typename boost::asio::enable_if<
      boost::asio::is_convertible<
        Property,
        boost::asio::execution::relationship_t
      >::value
    >::type
  >
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef boost::asio::execution::relationship_t result_type;
};

template <typename Allocator, unsigned int Bits>
struct query_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    boost::asio::execution::occupancy_t
  >
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef std::size_t result_type;
};

template <typename Allocator, unsigned int Bits>
struct query_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    boost::asio::execution::context_t
  >
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef boost::asio::thread_pool& result_type;
};

template <typename Allocator, unsigned int Bits>
struct query_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    boost::asio::execution::allocator_t<void>
  >
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef Allocator result_type;
};

template <typename Allocator, unsigned int Bits, typename OtherAllocator>
struct query_member<
    boost::asio::thread_pool::basic_executor_type<Allocator, Bits>,
    boost::asio::execution::allocator_t<OtherAllocator>
  >
{
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_valid = true);
  BOOST_ASIO_STATIC_CONSTEXPR(bool, is_noexcept = true);
  typedef Allocator result_type;
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

} // namespace traits

#endif // !defined(GENERATING_DOCUMENTATION)

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/impl/thread_pool.hpp>
#if defined(BOOST_ASIO_HEADER_ONLY)
# include <boost/asio/impl/thread_pool.ipp>
#endif // defined(BOOST_ASIO_HEADER_ONLY)

#endif // BOOST_ASIO_THREAD_POOL_HPP
