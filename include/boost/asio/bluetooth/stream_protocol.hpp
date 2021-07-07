//
// bluetooth/stream_protocol.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2019 Huang Qinjin (huangqinjin at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_BLUETOOTH_STREAM_PROTOCOL_HPP
#define BOOST_ASIO_BLUETOOTH_STREAM_PROTOCOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_BLUETOOTH_SOCKETS) \
  || defined(GENERATING_DOCUMENTATION)

#include <boost/asio/basic_socket_acceptor.hpp>
#include <boost/asio/basic_socket_iostream.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/bluetooth/basic_endpoint.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace bluetooth {

/// Encapsulates the flags needed for stream-oriented Bluetooth sockets.
/**
 * The boost::asio::bluetooth::stream_protocol class contains flags necessary for
 * stream-oriented Bluetooth sockets.
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
  /// Obtain an identifier for the type of the protocol.
  int type() const
  {
    return SOCK_STREAM;
  }

  /// Obtain an identifier for the protocol.
  int protocol() const
  {
    return BOOST_ASIO_OS_DEF(BTPROTO_RFCOMM);
  }

  /// Obtain an identifier for the protocol family.
  int family() const
  {
    return BOOST_ASIO_OS_DEF(AF_BLUETOOTH);
  }

  /// The type of a Bluetooth endpoint.
  typedef basic_endpoint<stream_protocol> endpoint;

  /// The Bluetooth socket type.
  typedef basic_stream_socket<stream_protocol> socket;

  /// The Bluetooth acceptor type.
  typedef basic_socket_acceptor<stream_protocol> acceptor;

#if !defined(BOOST_ASIO_NO_IOSTREAM)
  /// The Bluetooth iostream type.
  typedef basic_socket_iostream<stream_protocol> iostream;
#endif // !defined(BOOST_ASIO_NO_IOSTREAM)
};

} // namespace bluetooth
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_BLUETOOTH_SOCKETS)
       //   || defined(GENERATING_DOCUMENTATION)

#endif // BOOST_ASIO_BLUETOOTH_STREAM_PROTOCOL_HPP
