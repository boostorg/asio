//
// vm/datagram_protocol.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2023 OLiver Kowalke (oliver dot kowalke at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_VM_DATAGRAM_PROTOCOL_HPP
#define BOOST_ASIO_VM_DATAGRAM_PROTOCOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_VM_SOCKETS)

#include <boost/asio/basic_datagram_socket.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/vm/basic_endpoint.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace vm {

/// Encapsulates the flags needed for datagram-oriented VSOCK sockets.
/**
 * The boost::asio::vm::datagram_protocol class contains flags necessary for
 * datagram-oriented VSOCK sockets.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe.
 *
 * @par Concepts:
 * Protocol.
 */
class datagram_protocol
{
public:
  /// Obtain an identifier for the type of the protocol.
  int type() const BOOST_ASIO_NOEXCEPT
  {
    return SOCK_DGRAM;
  }

  /// Obtain an identifier for the protocol.
  int protocol() const BOOST_ASIO_NOEXCEPT
  {
    return 0;
  }

  /// Obtain an identifier for the protocol family.
  int family() const BOOST_ASIO_NOEXCEPT
  {
    return AF_VSOCK;
  }

  /// The type of a VSOCK endpoint.
  typedef basic_endpoint<datagram_protocol> endpoint;

  /// The VSOCK socket type.
  typedef basic_datagram_socket<datagram_protocol> socket;
};

} // namespace vm
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_VM_SOCKETS)

#endif // BOOST_ASIO_VM_DATAGRAM_PROTOCOL_HPP
