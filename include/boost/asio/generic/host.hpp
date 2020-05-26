//
// generic/host.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_GENERIC_HOST_HPP
#define BOOST_ASIO_GENERIC_HOST_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#include <boost/asio/detail/apple_nw_ptr.hpp>
#include <Network/Network.h>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace generic {

/// Describes a host-based endpoint for any socket type.
/**

 * The boost::asio::generic::host class template describes a
 * host-based endpoint that may be associated with any socket type.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 *
 * @par Concepts:
 * Endpoint.
 */
template <typename Protocol>
class host
{
public:
  /// The protocol type associated with the endpoint.
  typedef Protocol protocol_type;

  /// Construct an endpoint for the specified protocol, host and port.
  host(const protocol_type& proto,
      const char* host_name, const char* port)
    : protocol_(proto),
      endpoint_(nw_endpoint_create_host(host_name, port))
  {
  }

  /// Copy constructor.
  host(const host& other)
    : protocol_(other.protocol_),
      endpoint_(other.endpoint_)
  {
  }

#if defined(BOOST_ASIO_HAS_MOVE)
  /// Move constructor.
  host(host&& other)
    : protocol_(BOOST_ASIO_MOVE_CAST(protocol_type)(other.protocol_)),
      endpoint_(BOOST_ASIO_MOVE_CAST(
          boost::asio::detail::apple_nw_ptr<nw_endpoint_t>)(
            other.endpoint_))
  {
  }
#endif // defined(BOOST_ASIO_HAS_MOVE)

  /// Assign from another endpoint.
  host& operator=(const host& other)
  {
    protocol_ = other.protocol_;
    endpoint_ = other.endpoint_;
    return *this;
  }

#if defined(BOOST_ASIO_HAS_MOVE)
  /// Move-assign from another endpoint.
  host& operator=(host&& other)
  {
    protocol_ = BOOST_ASIO_MOVE_CAST(protocol_type)(other.protocol_);
    endpoint_ = BOOST_ASIO_MOVE_CAST(
        boost::asio::detail::apple_nw_ptr<nw_endpoint_t>)(
          other.endpoint_);
    return *this;
  }
#endif // defined(BOOST_ASIO_HAS_MOVE)

  /// The protocol associated with the endpoint.
  protocol_type protocol() const
  {
    return protocol_;
  }

  // Create a new native object corresponding to the endpoint.
  boost::asio::detail::apple_nw_ptr<nw_endpoint_t>
  apple_nw_create_endpoint() const
  {
    return endpoint_;
  }

  // Set the endpoint from the native object.
  void apple_nw_set_endpoint(
      boost::asio::detail::apple_nw_ptr<nw_endpoint_t> new_ep)
  {
    endpoint_ = new_ep;
  }

  // Set the protocol.
  void apple_nw_set_protocol(protocol_type new_protocol)
  {
    protocol_ = new_protocol;
  }

  /// Compare two endpoints for equality.
  friend bool operator==(const host<Protocol>& e1, const host<Protocol>& e2)
  {
    return e1.protocol_ == e2.protocol_ && e1.endpoint_ == e2.endpoint_;
  }

  /// Compare two endpoints for inequality.
  friend bool operator!=(const host<Protocol>& e1, const host<Protocol>& e2)
  {
    return !(e1 == e2);
  }

private:
  // The associated protocol object.
  Protocol protocol_;

  // The underlying native endpoint.
  boost::asio::detail::apple_nw_ptr<nw_endpoint_t> endpoint_;
};

} // namespace generic
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#endif // BOOST_ASIO_GENERIC_HOST_HPP
