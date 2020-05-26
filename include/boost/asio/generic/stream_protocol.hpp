//
// generic/stream_protocol.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_GENERIC_STREAM_PROTOCOL_HPP
#define BOOST_ASIO_GENERIC_STREAM_PROTOCOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#include <typeinfo>
#include <boost/asio/basic_socket_acceptor.hpp>
#include <boost/asio/basic_socket_iostream.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/detail/throw_exception.hpp>
#include <boost/asio/detail/type_traits.hpp>
#include <boost/asio/generic/basic_endpoint.hpp>

#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
# include <boost/asio/detail/apple_nw_ptr.hpp>
# include <Network/Network.h>
#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace generic {

/// Encapsulates the flags needed for a generic stream-oriented socket.
/**
 * The boost::asio::generic::stream_protocol class contains flags necessary for
 * stream-oriented sockets of any address family and protocol.
 *
 * @par Examples
 * Constructing using a native address family and socket protocol:
 * @code stream_protocol p(AF_INET, IPPROTO_TCP); @endcode
 * Constructing from a specific protocol type:
 * @code stream_protocol p(boost::asio::ip::tcp::v4()); @endcode
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe.
 *
 * @par Concepts:
 * Protocol.
 */
class stream_protocol
{
public:
#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
  /// Construct a protocol object from parameters and max size.
  stream_protocol(
      boost::asio::detail::apple_nw_ptr<nw_parameters_t> parameters,
      std::size_t max_receive_size)
    : parameters_(
        BOOST_ASIO_MOVE_CAST(
          boost::asio::detail::apple_nw_ptr<nw_parameters_t>)(parameters)),
      max_receive_size_(max_receive_size)
  {
  }
#else // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
  /// Construct a protocol object for a specific address family and protocol.
  stream_protocol(int address_family, int socket_protocol)
    : family_(address_family),
      protocol_(socket_protocol)
  {
  }
#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

  /// Construct a generic protocol object from a specific protocol.
  /**
   * @throws @c bad_cast Thrown if the source protocol is not stream-oriented.
   */
  template <typename Protocol>
  stream_protocol(const Protocol& source_protocol,
      typename enable_if<
        is_same<Protocol, typename Protocol::endpoint::protocol_type>::value
      >::type* = 0)
#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
    : parameters_(source_protocol.apple_nw_create_parameters()),
      max_receive_size_(source_protocol.apple_nw_max_receive_size())
#else // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
    : family_(source_protocol.family()),
      protocol_(source_protocol.protocol())
#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
  {
    if (source_protocol.type() != type())
    {
      std::bad_cast ex;
      boost::asio::detail::throw_exception(ex);
    }
  }

  /// Obtain an identifier for the type of the protocol.
  int type() const BOOST_ASIO_NOEXCEPT
  {
    return BOOST_ASIO_OS_DEF(SOCK_STREAM);
  }

#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
  // The following functions comprise the extensible interface for the Protocol
  // concept when targeting the Apple Network Framework.

  // Obtain parameters to be used when creating a new connection or listener.
  boost::asio::detail::apple_nw_ptr<nw_parameters_t>
  apple_nw_create_parameters() const
  {
    return boost::asio::detail::apple_nw_ptr<nw_parameters_t>(
        nw_parameters_copy(parameters_));
  }

  // Obtain the override value for the maximum receive size.
  std::size_t apple_nw_max_receive_size() const
  {
    return max_receive_size_;
  }
#else // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
  // The following functions are only available with the standard extensible
  // interface.

  /// Obtain an identifier for the protocol.
  int protocol() const BOOST_ASIO_NOEXCEPT
  {
    return protocol_;
  }

  /// Obtain an identifier for the protocol family.
  int family() const BOOST_ASIO_NOEXCEPT
  {
    return family_;
  }
#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

  /// Compare two protocols for equality.
  friend bool operator==(const stream_protocol& p1, const stream_protocol& p2)
  {
#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
    return p1.parameters_ == p2.parameters_
      && p1.max_receive_size_ == p2.max_receive_size_;
#else // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
    return p1.family_ == p2.family_ && p1.protocol_ == p2.protocol_;
#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
  }

  /// Compare two protocols for inequality.
  friend bool operator!=(const stream_protocol& p1, const stream_protocol& p2)
  {
    return !(p1 == p2);
  }

  /// The type of an endpoint.
  typedef basic_endpoint<stream_protocol> endpoint;

  /// The generic socket type.
  typedef basic_stream_socket<stream_protocol> socket;

  /// The generic acceptor type.
  typedef basic_socket_acceptor<stream_protocol> acceptor;

#if !defined(BOOST_ASIO_NO_IOSTREAM) \
  && !defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
  /// The generic socket iostream type.
  typedef basic_socket_iostream<stream_protocol> iostream;
#endif // !defined(BOOST_ASIO_NO_IOSTREAM)
       //   && !defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

private:
#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
  boost::asio::detail::apple_nw_ptr<nw_parameters_t> parameters_;
  std::size_t max_receive_size_;
#else // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
  int family_;
  int protocol_;
#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
};

} // namespace generic
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_GENERIC_STREAM_PROTOCOL_HPP
