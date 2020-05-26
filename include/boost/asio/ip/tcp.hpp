//
// ip/tcp.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IP_TCP_HPP
#define BOOST_ASIO_IP_TCP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/basic_socket_acceptor.hpp>
#include <boost/asio/basic_socket_iostream.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/detail/socket_option.hpp>
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

/// Encapsulates the flags needed for TCP.
/**
 * The boost::asio::ip::tcp class contains flags necessary for TCP sockets.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe.
 *
 * @par Concepts:
 * Protocol, InternetProtocol.
 */
class tcp
{
public:
  /// The type of a TCP endpoint.
  typedef basic_endpoint<tcp> endpoint;

  /// Construct to represent the IPv4 TCP protocol.
  static tcp v4() BOOST_ASIO_NOEXCEPT
  {
    return tcp(BOOST_ASIO_OS_DEF(AF_INET));
  }

  /// Construct to represent the IPv6 TCP protocol.
  static tcp v6() BOOST_ASIO_NOEXCEPT
  {
    return tcp(BOOST_ASIO_OS_DEF(AF_INET6));
  }

  /// Construct to represent an unspecified TCP protocol.
  static tcp any() BOOST_ASIO_NOEXCEPT
  {
    return tcp(BOOST_ASIO_OS_DEF(AF_UNSPEC));
  }

  /// Obtain an identifier for the type of the protocol.
  int type() const BOOST_ASIO_NOEXCEPT
  {
    return BOOST_ASIO_OS_DEF(SOCK_STREAM);
  }

  /// Obtain an identifier for the protocol.
  int protocol() const BOOST_ASIO_NOEXCEPT
  {
    return BOOST_ASIO_OS_DEF(IPPROTO_TCP);
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
    return 0;
  }
#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

  /// The TCP socket type.
  typedef basic_stream_socket<tcp> socket;

  /// The TCP acceptor type.
  typedef basic_socket_acceptor<tcp> acceptor;

  /// The TCP resolver type.
  typedef basic_resolver<tcp> resolver;

#if !defined(BOOST_ASIO_NO_IOSTREAM) \
  && !defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
  /// The TCP iostream type.
  typedef basic_socket_iostream<tcp> iostream;
#endif // !defined(BOOST_ASIO_NO_IOSTREAM)
       //   && !defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

  /// Socket option for disabling the Nagle algorithm.
  /**
   * Implements the IPPROTO_TCP/TCP_NODELAY socket option.
   *
   * @par Examples
   * Setting the option:
   * @code
   * boost::asio::ip::tcp::socket socket(my_context);
   * ...
   * boost::asio::ip::tcp::no_delay option(true);
   * socket.set_option(option);
   * @endcode
   *
   * @par
   * Getting the current option value:
   * @code
   * boost::asio::ip::tcp::socket socket(my_context);
   * ...
   * boost::asio::ip::tcp::no_delay option;
   * socket.get_option(option);
   * bool is_set = option.value();
   * @endcode
   *
   * @par Concepts:
   * Socket_Option, Boolean_Socket_Option.
   */
#if defined(GENERATING_DOCUMENTATION)
  typedef implementation_defined no_delay;
#else
  class no_delay :
    public boost::asio::detail::socket_option::boolean<
      BOOST_ASIO_OS_DEF(IPPROTO_TCP), BOOST_ASIO_OS_DEF(TCP_NODELAY)>
  {
  public:
    no_delay()
    {
    }

    explicit no_delay(bool b)
      : boost::asio::detail::socket_option::boolean<
          BOOST_ASIO_OS_DEF(IPPROTO_TCP), BOOST_ASIO_OS_DEF(TCP_NODELAY)>(b)
    {
    }

    no_delay& operator=(bool b)
    {
      boost::asio::detail::socket_option::boolean<
          BOOST_ASIO_OS_DEF(IPPROTO_TCP),
          BOOST_ASIO_OS_DEF(TCP_NODELAY)>::operator=(b);
      return *this;
    }

#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
  // The following functions comprise the extensible interface for the
  // SettableSocketOption and GettableSocketOption concepts when targeting the
  // Apple Network Framework.

  // Set the socket option on the specified connection.
  BOOST_ASIO_DECL static void apple_nw_set(const void* self,
      nw_parameters_t parameters, nw_connection_t connection,
      boost::system::error_code& ec);

  // Set the socket option on the specified connection.
  BOOST_ASIO_DECL static void apple_nw_set(const void* self,
      nw_parameters_t parameters, nw_listener_t listener,
      boost::system::error_code& ec);

  // Get the socket option from the specified connection.
  BOOST_ASIO_DECL static void apple_nw_get(void* self,
      nw_parameters_t parameters, nw_connection_t connection,
      boost::system::error_code& ec);

  // Get the socket option from the specified connection.
  BOOST_ASIO_DECL static void apple_nw_get(void* self,
      nw_parameters_t parameters, nw_listener_t listener,
      boost::system::error_code& ec);
#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
  };
#endif

  /// Compare two protocols for equality.
  friend bool operator==(const tcp& p1, const tcp& p2)
  {
    return p1.family_ == p2.family_;
  }

  /// Compare two protocols for inequality.
  friend bool operator!=(const tcp& p1, const tcp& p2)
  {
    return p1.family_ != p2.family_;
  }

private:
  // Construct with a specific family.
  explicit tcp(int protocol_family) BOOST_ASIO_NOEXCEPT
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
# include <boost/asio/ip/impl/tcp.ipp>
#endif // defined(BOOST_ASIO_HEADER_ONLY)

#endif // BOOST_ASIO_IP_TCP_HPP
