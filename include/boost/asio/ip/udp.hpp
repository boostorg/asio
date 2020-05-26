//
// ip/udp.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IP_UDP_HPP
#define BOOST_ASIO_IP_UDP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/basic_datagram_socket.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>
#include <boost/asio/ip/basic_resolver.hpp>
#include <boost/asio/ip/basic_resolver_iterator.hpp>
#include <boost/asio/ip/basic_resolver_query.hpp>

#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
# include <boost/asio/detail/apple_nw_ptr.hpp>
# include <Network/Network.h>
#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace ip {

/// Encapsulates the flags needed for UDP.
/**
 * The boost::asio::ip::udp class contains flags necessary for UDP sockets.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe.
 *
 * @par Concepts:
 * Protocol, InternetProtocol.
 */
class udp
{
public:
  /// The type of a UDP endpoint.
  typedef basic_endpoint<udp> endpoint;

  /// Construct to represent the IPv4 UDP protocol.
  static udp v4() BOOST_ASIO_NOEXCEPT
  {
    return udp(BOOST_ASIO_OS_DEF(AF_INET));
  }

  /// Construct to represent the IPv6 UDP protocol.
  static udp v6() BOOST_ASIO_NOEXCEPT
  {
    return udp(BOOST_ASIO_OS_DEF(AF_INET6));
  }

  /// Construct to represent an unspecified TCP protocol.
  static udp any() BOOST_ASIO_NOEXCEPT
  {
    return udp(BOOST_ASIO_OS_DEF(AF_UNSPEC));
  }

  /// Obtain an identifier for the type of the protocol.
  int type() const BOOST_ASIO_NOEXCEPT
  {
    return BOOST_ASIO_OS_DEF(SOCK_DGRAM);
  }

  /// Obtain an identifier for the protocol.
  int protocol() const BOOST_ASIO_NOEXCEPT
  {
    return BOOST_ASIO_OS_DEF(IPPROTO_UDP);
  }

  /// Obtain an identifier for the protocol family.
  int family() const BOOST_ASIO_NOEXCEPT
  {
    return family_;
  }

#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
  // The following functions comprise the extensible interface for the Protocol
  // concept when targeting the Apple Network Framework.

  // Obtain parameters to be used when creating a new connection or listener.
  BOOST_ASIO_DECL boost::asio::detail::apple_nw_ptr<nw_parameters_t>
  apple_nw_create_parameters() const;

  // Obtain the override value for the maximum receive size.
  std::size_t apple_nw_max_receive_size() const BOOST_ASIO_NOEXCEPT
  {
    return 65535;
  }
#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

  /// The UDP socket type.
  typedef basic_datagram_socket<udp> socket;

  /// The UDP resolver type.
  typedef basic_resolver<udp> resolver;

  /// Compare two protocols for equality.
  friend bool operator==(const udp& p1, const udp& p2)
  {
    return p1.family_ == p2.family_;
  }

  /// Compare two protocols for inequality.
  friend bool operator!=(const udp& p1, const udp& p2)
  {
    return p1.family_ != p2.family_;
  }

private:
  // Construct with a specific family.
  explicit udp(int protocol_family) BOOST_ASIO_NOEXCEPT
    : family_(protocol_family)
  {
  }

  int family_;
};

} // namespace ip
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_ASIO_HEADER_ONLY)
# include <boost/asio/ip/impl/udp.ipp>
#endif // defined(BOOST_ASIO_HEADER_ONLY)

#endif // BOOST_ASIO_IP_UDP_HPP
