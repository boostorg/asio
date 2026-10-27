//
// detail/reactive_socket_recvmmsg_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_REACTIVE_SOCKET_RECVMMSG_OP_HPP
#define BOOST_ASIO_DETAIL_REACTIVE_SOCKET_RECVMMSG_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if !defined(BOOST_ASIO_HAS_IOCP) \
  && !defined(BOOST_ASIO_WINDOWS_RUNTIME)

#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/detail/buffer_sequences_adapter.hpp>
#include <boost/asio/detail/empty_range.hpp>
#include <boost/asio/detail/fenced_block.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_work.hpp>
#include <boost/asio/detail/memory.hpp>
#include <boost/asio/detail/reactor_op.hpp>
#include <boost/asio/detail/socket_ops.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
BOOST_ASIO_INLINE_NAMESPACE_BEGIN
namespace detail {

template <typename BufferSequences, typename EndpointSequence,
    typename SizeSequence, typename FlagsSequence>
class reactive_socket_recvmmsg_op_base : public reactor_op
{
public:
  template <typename B, typename E, typename S, typename F>
  reactive_socket_recvmmsg_op_base(const boost::system::error_code& success_ec,
      socket_type socket, B&& buffer_sequences, E&& endpoints, S&& sizes,
      F&& out_flags, socket_base::message_flags flags,
      func_type complete_func)
    : reactor_op(success_ec,
          &reactive_socket_recvmmsg_op_base::do_perform, complete_func),
      socket_(socket),
      buffer_sequences_(static_cast<B&&>(buffer_sequences)),
      endpoints_(static_cast<E&&>(endpoints)),
      sizes_(static_cast<S&&>(sizes)),
      out_flags_(static_cast<F&&>(out_flags)),
      flags_(flags)
  {
  }

  static BOOST_ASIO_ALWAYS_INLINE status do_perform(reactor_op* base)
  {
    BOOST_ASIO_ASSUME(base != 0);
    reactive_socket_recvmmsg_op_base* o(
        static_cast<reactive_socket_recvmmsg_op_base*>(base));

    status result = non_blocking_receive_multiple(o->socket_,
        o->buffer_sequences_, o->endpoints_, o->sizes_, o->out_flags_,
        o->flags_, o->ec_, o->bytes_transferred_) ? done : not_done;

    BOOST_ASIO_HANDLER_REACTOR_OPERATION((*o, "non_blocking_recvmmsg",
          o->ec_, o->bytes_transferred_));

    return result;
  }

private:
  socket_type socket_;
  BufferSequences buffer_sequences_;
  EndpointSequence endpoints_;
  SizeSequence sizes_;
  FlagsSequence out_flags_;
  socket_base::message_flags flags_;
};

template <typename BufferSequences, typename EndpointSequence,
    typename SizeSequence, typename FlagsSequence,
    typename Handler, typename IoExecutor>
class reactive_socket_recvmmsg_op
  : public reactive_socket_recvmmsg_op_base<
        BufferSequences, EndpointSequence, SizeSequence, FlagsSequence>
{
public:
  typedef Handler handler_type;
  typedef IoExecutor io_executor_type;

  BOOST_ASIO_DEFINE_HANDLER_PTR(reactive_socket_recvmmsg_op);

  template <typename B, typename E, typename S, typename F>
  reactive_socket_recvmmsg_op(const boost::system::error_code& success_ec,
      socket_type socket, B&& buffer_sequences, E&& endpoints, S&& sizes,
      F&& out_flags, socket_base::message_flags flags, Handler& handler,
      const IoExecutor& io_ex)
    : reactive_socket_recvmmsg_op_base<BufferSequences,
          EndpointSequence, SizeSequence, FlagsSequence>(success_ec,
          socket, static_cast<B&&>(buffer_sequences),
          static_cast<E&&>(endpoints), static_cast<S&&>(sizes),
          static_cast<F&&>(out_flags), flags,
          &reactive_socket_recvmmsg_op::do_complete),
      handler_(static_cast<Handler&&>(handler)),
      work_(handler_, io_ex)
  {
  }

  static void do_complete(void* owner, operation* base,
      int /*error_value*/, std::size_t /*bytes_transferred*/)
  {
    // Take ownership of the handler object.
    BOOST_ASIO_ASSUME(base != 0);
    reactive_socket_recvmmsg_op* o(
        static_cast<reactive_socket_recvmmsg_op*>(base));
    ptr p = { boost::asio::detail::addressof(o->handler_), o, o };

    BOOST_ASIO_HANDLER_COMPLETION((*o));

    // Take ownership of the operation's outstanding work.
    handler_work<Handler, IoExecutor> w(
        static_cast<handler_work<Handler, IoExecutor>&&>(o->work_));

    BOOST_ASIO_ERROR_LOCATION(o->ec_);

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    detail::binder2<Handler, boost::system::error_code, std::size_t>
      handler(o->handler_, o->ec_, o->bytes_transferred_);
    p.h = boost::asio::detail::addressof(handler.handler_);
    p.reset();

    // Make the upcall if required.
    if (owner)
    {
      fenced_block b(fenced_block::half);
      BOOST_ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_, handler.arg2_));
      w.complete(handler, handler.handler_);
      BOOST_ASIO_HANDLER_INVOCATION_END;
    }
  }

  static void do_immediate(operation* base, bool, const void* io_ex)
  {
    // Take ownership of the handler object.
    BOOST_ASIO_ASSUME(base != 0);
    reactive_socket_recvmmsg_op* o(
        static_cast<reactive_socket_recvmmsg_op*>(base));
    ptr p = { boost::asio::detail::addressof(o->handler_), o, o };

    BOOST_ASIO_HANDLER_COMPLETION((*o));

    // Take ownership of the operation's outstanding work.
    immediate_handler_work<Handler, IoExecutor> w(
        static_cast<handler_work<Handler, IoExecutor>&&>(o->work_));

    BOOST_ASIO_ERROR_LOCATION(o->ec_);

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    detail::binder2<Handler, boost::system::error_code, std::size_t>
      handler(o->handler_, o->ec_, o->bytes_transferred_);
    p.h = boost::asio::detail::addressof(handler.handler_);
    p.reset();

    BOOST_ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_, handler.arg2_));
    w.complete(handler, handler.handler_, io_ex);
    BOOST_ASIO_HANDLER_INVOCATION_END;
  }

private:
  Handler handler_;
  handler_work<Handler, IoExecutor> work_;
};

} // namespace detail
BOOST_ASIO_INLINE_NAMESPACE_END
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // !defined(BOOST_ASIO_HAS_IOCP) && !defined(BOOST_ASIO_WINDOWS_RUNTIME)

#endif // BOOST_ASIO_DETAIL_REACTIVE_SOCKET_RECVMMSG_OP_HPP
