//
// detail/apple_nw_socket_accept_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_APPLE_NW_SOCKET_ACCEPT_OP_HPP
#define BOOST_ASIO_DETAIL_APPLE_NW_SOCKET_ACCEPT_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#include <boost/asio/detail/apple_nw_async_op.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/detail/fenced_block.hpp>
#include <boost/asio/detail/scheduler_operation.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename Protocol, typename Socket,
    typename Handler, typename IoExecutor>
class apple_nw_socket_accept_op :
  public apple_nw_async_op<apple_nw_ptr<nw_connection_t> >
{
public:
  BOOST_ASIO_DEFINE_HANDLER_PTR(apple_nw_socket_accept_op);

  apple_nw_socket_accept_op(Socket& peer, const Protocol& protocol,
      typename Protocol::endpoint* peer_endpoint, Handler& handler,
      const IoExecutor& io_ex)
    : apple_nw_async_op<apple_nw_ptr<nw_connection_t> >(
        &apple_nw_socket_accept_op::do_complete),
      peer_socket_(peer),
      protocol_(protocol),
      peer_endpoint_(peer_endpoint),
      handler_(BOOST_ASIO_MOVE_CAST(Handler)(handler)),
      io_executor_(io_ex)
  {
    handler_work<Handler, IoExecutor>::start(handler_, io_executor_);
  }

  static void do_complete(void* owner, operation* base,
      const boost::system::error_code& /*ec*/,
      std::size_t /*bytes_transferred*/)
  {
    // Take ownership of the handler object.
    apple_nw_socket_accept_op* o(static_cast<apple_nw_socket_accept_op*>(base));
    ptr p = { boost::asio::detail::addressof(o->handler_), o, o };
    handler_work<Handler, IoExecutor> w(o->handler_, o->io_executor_);

    BOOST_ASIO_HANDLER_COMPLETION((*o));

    if (!o->ec_)
    {
      typename Socket::native_handle_type handle(o->result_.get());
      o->peer_socket_.assign(o->protocol_, handle, o->ec_);
      if (!o->ec_)
      {
        o->result_.release();
        if (o->peer_endpoint_)
        {
          *o->peer_endpoint_ = o->peer_socket_.remote_endpoint(o->ec_);
          if (o->ec_)
            o->peer_socket_.close();
        }
      }
    }

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    detail::binder1<Handler, boost::system::error_code> handler(o->handler_, o->ec_);
    p.h = boost::asio::detail::addressof(handler.handler_);
    p.reset();

    // Make the upcall if required.
    if (owner)
    {
      fenced_block b(fenced_block::half);
      BOOST_ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_));
      w.complete(handler, handler.handler_);
      BOOST_ASIO_HANDLER_INVOCATION_END;
    }
  }

private:
  Socket& peer_socket_;
  Protocol protocol_;
  typename Protocol::endpoint* peer_endpoint_;
  Handler handler_;
  IoExecutor io_executor_;
};

#if defined(BOOST_ASIO_HAS_MOVE)

template <typename Protocol, typename PeerIoExecutor,
    typename Handler, typename IoExecutor>
class apple_nw_socket_move_accept_op :
  public apple_nw_async_op<apple_nw_ptr<nw_connection_t> >
{
public:
  BOOST_ASIO_DEFINE_HANDLER_PTR(apple_nw_socket_move_accept_op);

  apple_nw_socket_move_accept_op(const PeerIoExecutor& peer_io_executor,
      const Protocol& protocol, typename Protocol::endpoint* peer_endpoint,
      Handler& handler, const IoExecutor& io_ex)
    : apple_nw_async_op<apple_nw_ptr<nw_connection_t> >(
        &apple_nw_socket_move_accept_op::do_complete),
      peer_io_executor_(peer_io_executor),
      protocol_(protocol),
      peer_endpoint_(peer_endpoint),
      handler_(BOOST_ASIO_MOVE_CAST(Handler)(handler)),
      io_executor_(io_ex)
  {
    handler_work<Handler, IoExecutor>::start(handler_, io_executor_);
  }

  static void do_complete(void* owner, operation* base,
      const boost::system::error_code& /*ec*/,
      std::size_t /*bytes_transferred*/)
  {
    // Take ownership of the handler object.
    apple_nw_socket_move_accept_op* o(
        static_cast<apple_nw_socket_move_accept_op*>(base));
    ptr p = { boost::asio::detail::addressof(o->handler_), o, o };
    handler_work<Handler, IoExecutor> w(o->handler_, o->io_executor_);

    BOOST_ASIO_HANDLER_COMPLETION((*o));

    peer_socket_type peer_socket(o->peer_io_executor_);
    if (!o->ec_)
    {
      typename peer_socket_type::native_handle_type handle(o->result_.get());
      peer_socket.assign(o->protocol_, handle, o->ec_);
      if (!o->ec_)
      {
        o->result_.release();
        if (o->peer_endpoint_)
        {
          *o->peer_endpoint_ = peer_socket.remote_endpoint(o->ec_);
          if (o->ec_)
            peer_socket.close();
        }
      }
    }

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    detail::move_binder2<Handler,
      boost::system::error_code, peer_socket_type>
        handler(0, BOOST_ASIO_MOVE_CAST(Handler)(o->handler_), o->ec_,
          BOOST_ASIO_MOVE_CAST(peer_socket_type)(peer_socket));
    p.h = boost::asio::detail::addressof(handler.handler_);
    p.reset();

    // Make the upcall if required.
    if (owner)
    {
      fenced_block b(fenced_block::half);
      BOOST_ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_));
      w.complete(handler, handler.handler_);
      BOOST_ASIO_HANDLER_INVOCATION_END;
    }
  }

private:
  typedef typename Protocol::socket::template
    rebind_executor<PeerIoExecutor>::other peer_socket_type;

  PeerIoExecutor peer_io_executor_;
  Protocol protocol_;
  typename Protocol::endpoint* peer_endpoint_;
  Handler handler_;
  IoExecutor io_executor_;
};

#endif // defined(BOOST_ASIO_HAS_MOVE)

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#endif // BOOST_ASIO_DETAIL_APPLE_NW_SOCKET_ACCEPT_OP_HPP
