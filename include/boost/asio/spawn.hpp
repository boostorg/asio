//
// spawn.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2017 Oliver Kowalke (oliver dot kowalke at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_SPAWN_HPP
#define BOOST_ASIO_SPAWN_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/shared_ptr.hpp>
#include <boost/asio/detail/weak_ptr.hpp>
#include <boost/asio/detail/wrapped_handler.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/context/detail/config.hpp>
#if !defined(BOOST_CONTEXT_NO_CXX11)
# include <boost/context/continuation.hpp>
#else
# include <boost/context/detail/fcontext.hpp>
# include <boost/context/fixedsize_stack.hpp>
# include <boost/context/segmented_stack.hpp>
#endif

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

#if !defined(BOOST_CONTEXT_NO_CXX11)
class continuation_context : private noncopyable
{
public:
    boost::context::continuation    callee_;
    boost::context::continuation    caller_;

    continuation_context() :
        callee_(),
        caller_()
    {
    }

    void resume()
    {
        callee_ = callee_.resume();
    }

    void suspend()
    {
        caller_ = caller_.resume();
    }
};
#else
class continuation_context : private noncopyable
{
public:
    boost::context::detail::fcontext_t  callee_;
    boost::context::detail::fcontext_t  caller_;

    continuation_context() :
        callee_( 0),
        caller_( 0)
    {
    }

    virtual ~continuation_context()
    {
    }

    void resume()
    {
        callee_ = boost::context::detail::jump_fcontext( callee_, 0).fctx;
    }

    void suspend()
    {
        caller_ = boost::context::detail::jump_fcontext( caller_, 0).fctx;
    }
};
#endif

}

/// Context object represents the current execution context.
/**
 * The basic_yield_context class is used to represent the current execution
 * context. A basic_yield_context may be passed as a handler to an
 * asynchronous operation. For example:
 *
 * @code template <typename Handler>
 * void my_continuation(basic_yield_context<Handler> yield)
 * {
 *   ...
 *   std::size_t n = my_socket.async_read_some(buffer, yield);
 *   ...
 * } @endcode
 *
 * The initiating function (async_read_some in the above example) suspends the
 * current execution context, e.g. reifies a continuation. The continuation
 * is resumed when the asynchronous operation * completes, and the result of
 * the operation is returned.
 */
template <typename Handler>
class basic_yield_context
{
public:
  /// Construct a yield context to represent the specified execution context.
  /**
   * Most applications do not need to use this constructor. Instead, the
   * spawn() function passes a yield context as an argument to the continuation
   * function.
   */
  basic_yield_context(Handler& handler, const detail::shared_ptr<detail::continuation_context>& yc) :
      yc_(yc),
      handler_(handler),
      ec_(0)
  {
  }

  /// Return a yield context that sets the specified error_code.
  /**
   * By default, when a yield context is used with an asynchronous operation, a
   * non-success error_code is converted to system_error and thrown. This
   * operator may be used to specify an error_code object that should instead be
   * set with the asynchronous operation's result. For example:
   *
   * @code template <typename Handler>
   * void my_continuation(basic_yield_context<Handler> yield)
   * {
   *   ...
   *   std::size_t n = my_socket.async_read_some(buffer, yield[ec]);
   *   if (ec)
   *   {
   *     // An error occurred.
   *   }
   *   ...
   * } @endcode
   */
  basic_yield_context operator[](boost::system::error_code& ec) const
  {
    basic_yield_context tmp(*this);
    tmp.ec_ = &ec;
    return tmp;
  }

#if defined(GENERATING_DOCUMENTATION)
private:
#endif // defined(GENERATING_DOCUMENTATION)
  detail::weak_ptr<detail::continuation_context>    yc_;
  Handler&                                          handler_;
  boost::system::error_code*                        ec_;
};

#if defined(GENERATING_DOCUMENTATION)
/// Context object that represents the current execution context.
typedef basic_yield_context<unspecified> yield_context;
#else // defined(GENERATING_DOCUMENTATION)
typedef basic_yield_context<
  detail::wrapped_handler<
    io_service::strand, void(*)(),
    detail::is_continuation_if_running> > yield_context;
#endif // defined(GENERATING_DOCUMENTATION)

/**
 * @defgroup spawn boost::asio::spawn
 *
 * @brief Start a new execution context with a new stack.
 *
 * The spawn() function is a high-level wrapper over the Boost.Context
 * library (callcc()/continuation). This function enables programs to
 * implement asynchronous logic in a * synchronous manner, as illustrated
 * by the following example:
 *
 * @code boost::asio::spawn(my_strand, do_echo);
 *
 * // ...
 *
 * void do_echo(boost::asio::yield_context yield)
 * {
 *   try
 *   {
 *     char data[128];
 *     for (;;)
 *     {
 *       std::size_t length =
 *         my_socket.async_read_some(
 *           boost::asio::buffer(data), yield);
 *
 *       boost::asio::async_write(my_socket,
 *           boost::asio::buffer(data, length), yield);
 *     }
 *   }
 *   catch (std::exception& e)
 *   {
 *     // ...
 *   }
 * } @endcode
 */
/*@{*/

/// Start a new execution context (with new stack), calling the specified handler
/// when it completes.
/**
 * This function is used to launch a new execution context on behalf of callcc()
 * and continuation.
 *
 * @param handler A handler to be called when the continuation exits. More
 * importantly, the handler provides an execution context (via the the handler
 * invocation hook) for the continuation. The handler must have the signature:
 * @code void handler(); @endcode
 *
 * @param function The continuation function. The function must have the signature:
 * @code void function(basic_yield_context<Handler> yield); @endcode
 *
 * @param salloc Boost.Context uses stack allocators to create stacks.
 */
template <typename Handler, typename Function, typename StackAllocator>
void spawn(BOOST_ASIO_MOVE_ARG(Handler) handler,
           BOOST_ASIO_MOVE_ARG(Function) function,
           BOOST_ASIO_MOVE_ARG(StackAllocator) salloc);
template <typename Handler, typename Function>
void spawn(BOOST_ASIO_MOVE_ARG(Handler) handler,
           BOOST_ASIO_MOVE_ARG(Function) function) {
  spawn(BOOST_ASIO_MOVE_CAST(Handler)(handler),
        BOOST_ASIO_MOVE_CAST(Function)(function),
        boost::context::default_stack());
}

/// Start a new exeuciton context (with new stack), inheriting the execution context of another.
/**
 * This function is used to launch a new execution context on behalf of callcc()
 * and continuation.
 *
 * @param ctx Identifies the current execution context as a parent of the new
 * continuation. This specifies that the new continuation should inherit the
 * execution context of the parent. For example, if the parent continuation is
 * executing in a particular strand, then the new continuation will execute in the
 * same strand.
 *
 * @param function The continuation function. The function must have the signature:
 * @code void function(basic_yield_context<Handler> yield); @endcode
 *
 * @param salloc Boost.Context uses stack allocators to create stacks.
 */
template <typename Handler, typename Function, typename StackAllocator>
void spawn(basic_yield_context< Handler > ctx,
           BOOST_ASIO_MOVE_ARG(Function) function,
           BOOST_ASIO_MOVE_ARG(StackAllocator) salloc);
template <typename Handler, typename Function>
void spawn(basic_yield_context< Handler > ctx,
           BOOST_ASIO_MOVE_ARG(Function) function) {
  spawn(ctx,
        BOOST_ASIO_MOVE_CAST(Function)(function),
        boost::context::default_stack());
}

/// Start a new execution context (with new stack) that executes in the context of a strand.
/**
 * This function is used to launch a new execution context on behalf of callcc()
 * and continuation.
 *
 * @param strand Identifies a strand. By starting multiple continuations on the
 * same strand, the implementation ensures that none of those continuations can
 * execute simultaneously.
 *
 * @param function The continuations function. The function must have the signature:
 * @code void function(yield_context yield); @endcode
 *
 * @param salloc Boost.Context uses stack allocators to create stacks.
 */
template <typename Function, typename StackAllocator>
void spawn(io_service::strand strand,
           BOOST_ASIO_MOVE_ARG(Function) function,
           BOOST_ASIO_MOVE_ARG(StackAllocator) salloc);
template <typename Function>
void spawn(io_service::strand strand,
           BOOST_ASIO_MOVE_ARG(Function) function) {
  spawn(strand,
        BOOST_ASIO_MOVE_CAST(Function)(function),
        boost::context::default_stack());
}

/// Start a new execution context (with new stack) that executes on a given io_service.
/**
 * This function is used to launch a new execution context on behalf of callcc()
 * and continuation.
 *
 * @param io_service Identifies the io_service that will run the continuation. The
 * new ccontinuation is implicitly given its own strand within this io_service.
 *
 * @param function The continuation function. The function must have the signature:
 * @code void function(yield_context yield); @endcode
 *
 * @param salloc Boost.Context uses stack allocators to create stacks.
 */
template <typename Function, typename StackAllocator>
void spawn(io_service & io_service,
           BOOST_ASIO_MOVE_ARG(Function) function,
           BOOST_ASIO_MOVE_ARG(StackAllocator) salloc);
template <typename Function>
void spawn(io_service & io_service,
           BOOST_ASIO_MOVE_ARG(Function) function) {
  spawn(io_service,
        BOOST_ASIO_MOVE_CAST(Function)(function),
        boost::context::default_stack());
}

/*@}*/

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/impl/spawn.hpp>

#endif // BOOST_ASIO_SPAWN_HPP
