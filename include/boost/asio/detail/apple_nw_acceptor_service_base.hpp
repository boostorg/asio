//
// detail/apple_nw_acceptor_service_base.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_APPLE_NW_ACCEPTOR_SERVICE_BASE_HPP
#define BOOST_ASIO_DETAIL_APPLE_NW_ACCEPTOR_SERVICE_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/detail/apple_nw_async_op.hpp>
#include <boost/asio/detail/apple_nw_ptr.hpp>
#include <boost/asio/detail/apple_nw_socket_accept_op.hpp>
#include <boost/asio/detail/apple_nw_sync_result.hpp>
#include <boost/asio/detail/atomic_count.hpp>
#include <boost/asio/detail/memory.hpp>
#include <boost/asio/detail/scheduler.hpp>
#include <boost/asio/detail/scoped_ptr.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <Network/Network.h>
#include <deque>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

class apple_nw_acceptor_service_base
{
public:
  // The native type of an acceptor.
  struct native_handle_type
  {
    explicit native_handle_type(nw_listener_t l)
      : parameters(0),
        listener(l)
    {
    }

    native_handle_type(nw_parameters_t p, nw_listener_t l)
      : parameters(p),
        listener(l)
    {
    }

    nw_parameters_t parameters;
    nw_listener_t listener;
  };

  // The queues used for managing unaccepted connections.
  struct accept_queues
  {
    // Default constructor.
    accept_queues()
      : mutex_(),
        unclaimed_connections_(),
        pending_sync_(0)
    {
    }

    // A mutex used to protect access to the connection and accept queues.
    std::mutex mutex_;

    // The queue of unclaimed connections.
    std::deque<apple_nw_ptr<nw_connection_t> > unclaimed_connections_;

    // A pending synchronous accept operation (there can be only one).
    apple_nw_sync_result<apple_nw_ptr<nw_connection_t> >* pending_sync_;

    // Pending asynchronous accept operations.
    op_queue<apple_nw_async_op<apple_nw_ptr<nw_connection_t> > > pending_async_;
  };

  // The implementation type of the acceptor.
  struct base_implementation_type
  {
    // Default constructor.
    base_implementation_type()
      : parameters_(),
        listener_(),
        accept_queues_(),
        next_(0),
        prev_(0)
    {
    }

    // The parameters to be used to create the listener.
    apple_nw_ptr<nw_parameters_t> parameters_;

    // The underlying native listener.
    apple_nw_ptr<nw_listener_t> listener_;

    // The accept queues associated with the listener.
    std::shared_ptr<accept_queues> accept_queues_;

    // Pointers to adjacent acceptor implementations in linked list.
    base_implementation_type* next_;
    base_implementation_type* prev_;
  };

  // Constructor.
  BOOST_ASIO_DECL apple_nw_acceptor_service_base(execution_context& context);

  // Destroy all user-defined handler objects owned by the service.
  BOOST_ASIO_DECL void base_shutdown();

  // Construct a new acceptor implementation.
  BOOST_ASIO_DECL void construct(base_implementation_type&);

  // Move-construct a new acceptor implementation.
  BOOST_ASIO_DECL void base_move_construct(base_implementation_type& impl,
      base_implementation_type& other_impl) BOOST_ASIO_NOEXCEPT;

  // Move-assign from another acceptor implementation.
  BOOST_ASIO_DECL void base_move_assign(base_implementation_type& impl,
      apple_nw_acceptor_service_base& other_service,
      base_implementation_type& other_impl);

  // Destroy an acceptor implementation.
  BOOST_ASIO_DECL void destroy(base_implementation_type& impl);

  // Determine whether the acceptor is open.
  bool is_open(const base_implementation_type& impl) const
  {
    return !!impl.parameters_;
  }

  // Destroy an acceptor implementation.
  BOOST_ASIO_DECL boost::system::error_code close(
      base_implementation_type& impl, boost::system::error_code& ec);

  // Release ownership of the acceptor.
  BOOST_ASIO_DECL native_handle_type release(
      base_implementation_type& impl, boost::system::error_code& ec);

  // Get the native acceptor representation.
  native_handle_type native_handle(base_implementation_type& impl)
  {
    return native_handle_type(impl.parameters_, impl.listener_);
  }

  // Cancel all operations associated with the acceptor.
  BOOST_ASIO_DECL boost::system::error_code cancel(
      base_implementation_type&, boost::system::error_code& ec);

  // Place the socket into the state where it will listen for new connections.
  BOOST_ASIO_DECL boost::system::error_code listen(
      base_implementation_type& impl,
      int backlog, boost::system::error_code& ec);

  // Perform an IO control command on the acceptor.
  template <typename IO_Control_Command>
  boost::system::error_code io_control(base_implementation_type&,
      IO_Control_Command&, boost::system::error_code& ec)
  {
    ec = boost::asio::error::operation_not_supported;
    return ec;
  }

  // Gets the non-blocking mode of the acceptor.
  bool non_blocking(const base_implementation_type&) const
  {
    return false;
  }

  // Sets the non-blocking mode of the acceptor.
  boost::system::error_code non_blocking(base_implementation_type&,
      bool, boost::system::error_code& ec)
  {
    ec = boost::asio::error::operation_not_supported;
    return ec;
  }

  // Gets the non-blocking mode of the native acceptor implementation.
  bool native_non_blocking(const base_implementation_type&) const
  {
    return false;
  }

  // Sets the non-blocking mode of the native acceptor implementation.
  boost::system::error_code native_non_blocking(base_implementation_type&,
      bool, boost::system::error_code& ec)
  {
    ec = boost::asio::error::operation_not_supported;
    return ec;
  }

  // Wait for the acceptor to become ready to read, ready to write, or to have
  // pending error conditions.
  boost::system::error_code wait(base_implementation_type&,
      socket_base::wait_type, boost::system::error_code& ec)
  {
    ec = boost::asio::error::operation_not_supported;
    return ec;
  }

  // Asynchronously wait for the acceptor to become ready to read, ready to
  // write, or to have pending error conditions.
  template <typename Handler, typename IoExecutor>
  void async_wait(base_implementation_type&,
      socket_base::wait_type, Handler& handler, const IoExecutor& io_ex)
  {
    boost::system::error_code ec = boost::asio::error::operation_not_supported;
    boost::asio::post(io_ex, detail::bind_handler(
          BOOST_ASIO_MOVE_CAST(Handler)(handler), ec));
  }

protected:
  // Open a new acceptor implementation.
  BOOST_ASIO_DECL boost::system::error_code do_open(base_implementation_type& impl,
      apple_nw_ptr<nw_parameters_t> parameters, boost::system::error_code& ec);

  // Assign a native acceptor to an acceptor implementation.
  BOOST_ASIO_DECL boost::system::error_code do_assign(base_implementation_type& impl,
      const native_handle_type& native_acceptor, boost::system::error_code& ec);

  // Helper function to obtain local endpoint associated with the connection.
  BOOST_ASIO_DECL apple_nw_ptr<nw_endpoint_t> do_get_local_endpoint(
      const base_implementation_type& impl,
      boost::system::error_code& ec) const;

  // Helper function to set an acceptor option.
  BOOST_ASIO_DECL boost::system::error_code do_set_option(
      base_implementation_type& impl, const void* option,
      void (*set_fn)(const void*, nw_parameters_t,
        nw_listener_t, boost::system::error_code&),
      boost::system::error_code& ec);

  // Helper function to get an acceptor option.
  BOOST_ASIO_DECL boost::system::error_code do_get_option(
      const base_implementation_type& impl, void* option,
      void (*get_fn)(void*, nw_parameters_t,
        nw_listener_t, boost::system::error_code&),
      boost::system::error_code& ec) const;

  // Helper function to perform a synchronous bind.
  BOOST_ASIO_DECL boost::system::error_code do_bind(
      base_implementation_type& impl,
      apple_nw_ptr<nw_endpoint_t> endpoint,
      boost::system::error_code& ec);

  // Helper function to perform a synchronous accept.
  BOOST_ASIO_DECL apple_nw_ptr<nw_connection_t> do_accept(
      base_implementation_type& impl,
      boost::system::error_code& ec);

  // Helper function to start an asynchronous accept.
  BOOST_ASIO_DECL void start_accept_op(base_implementation_type& impl,
      apple_nw_async_op<apple_nw_ptr<nw_connection_t> >* op,
      bool is_continuation, bool peer_is_open);

  // The scheduler implementation used for delivering completions.
  scheduler& scheduler_;

  // Mutex to protect access to the linked list of implementations.
  boost::asio::detail::mutex mutex_;

  // The head of a linked list of all implementations.
  base_implementation_type* impl_list_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_ASIO_HEADER_ONLY)
# include <boost/asio/detail/impl/apple_nw_acceptor_service_base.ipp>
#endif // defined(BOOST_ASIO_HEADER_ONLY)

#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#endif // BOOST_ASIO_DETAIL_APPLE_NW_ACCEPTOR_SERVICE_BASE_HPP
