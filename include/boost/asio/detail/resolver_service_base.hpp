//
// detail/resolver_service_base.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_RESOLVER_SERVICE_BASE_HPP
#define BOOST_ASIO_DETAIL_RESOLVER_SERVICE_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/resolve_op.hpp>
#include <boost/asio/detail/resolver_thread_pool.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/socket_types.hpp>

#if defined(BOOST_ASIO_HAS_IOCP)
# include <boost/asio/detail/win_iocp_io_context.hpp>
#else // defined(BOOST_ASIO_HAS_IOCP)
# include <boost/asio/detail/scheduler.hpp>
#endif // defined(BOOST_ASIO_HAS_IOCP)

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

class resolver_service_base
{
public:
  // The implementation type of the resolver. A cancellation token is used to
  // indicate to the background thread that the operation has been cancelled.
  typedef socket_ops::shared_cancel_token_type implementation_type;

  // Constructor.
  BOOST_ASIO_DECL resolver_service_base(execution_context& context);

  // Destructor.
  BOOST_ASIO_DECL ~resolver_service_base();

  // Construct a new resolver implementation.
  BOOST_ASIO_DECL void construct(implementation_type& impl);

  // Destroy a resolver implementation.
  BOOST_ASIO_DECL void destroy(implementation_type&);

  // Move-construct a new resolver implementation.
  BOOST_ASIO_DECL void move_construct(implementation_type& impl,
      implementation_type& other_impl);

  // Move-assign from another resolver implementation.
  BOOST_ASIO_DECL void move_assign(implementation_type& impl,
      resolver_service_base& other_service,
      implementation_type& other_impl);

  // Move-construct a new timer implementation.
  void converting_move_construct(implementation_type& impl,
      resolver_service_base&, implementation_type& other_impl)
  {
    move_construct(impl, other_impl);
  }

  // Move-assign from another timer implementation.
  void converting_move_assign(implementation_type& impl,
      resolver_service_base& other_service,
      implementation_type& other_impl)
  {
    move_assign(impl, other_service, other_impl);
  }

  // Cancel pending asynchronous operations.
  BOOST_ASIO_DECL void cancel(implementation_type& impl);

protected:
#if !defined(BOOST_ASIO_WINDOWS_RUNTIME)
  // Helper class to perform exception-safe cleanup of addrinfo objects.
  class auto_addrinfo
    : private boost::asio::detail::noncopyable
  {
  public:
    explicit auto_addrinfo(boost::asio::detail::addrinfo_type* ai)
      : ai_(ai)
    {
    }

    ~auto_addrinfo()
    {
      if (ai_)
        socket_ops::freeaddrinfo(ai_);
    }

    operator boost::asio::detail::addrinfo_type*()
    {
      return ai_;
    }

  private:
    boost::asio::detail::addrinfo_type* ai_;
  };
#endif // !defined(BOOST_ASIO_WINDOWS_RUNTIME)

  // Private thread pool used for performing asynchronous host resolution.
  resolver_thread_pool& thread_pool_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_ASIO_HEADER_ONLY)
# include <boost/asio/detail/impl/resolver_service_base.ipp>
#endif // defined(BOOST_ASIO_HEADER_ONLY)

#endif // BOOST_ASIO_DETAIL_RESOLVER_SERVICE_BASE_HPP
