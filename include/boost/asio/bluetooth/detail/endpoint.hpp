//
// bluetooth/detail/endpoint.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2019 Huang Qinjin (huangqinjin at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_BLUETOOTH_DETAIL_ENDPOINT_HPP
#define BOOST_ASIO_BLUETOOTH_DETAIL_ENDPOINT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_BLUETOOTH_SOCKETS)

#include <cstddef>
#include <string>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/detail/string_view.hpp>
#include <boost/asio/bluetooth/address.hpp>

#include <boost/asio/detail/push_options.hpp>

#if defined(BOOST_ASIO_WINDOWS) || defined(__CYGWIN__)
# include <ws2bth.h>
#endif

namespace boost {
namespace asio {
namespace detail {
#if defined(BOOST_ASIO_WINDOWS) || defined(__CYGWIN__)
typedef SOCKADDR_BTH sockaddr_rc_type;
# define BOOST_ASIO_OS_DEF_AF_BLUETOOTH AF_BTH
# define BOOST_ASIO_OS_DEF_BTPROTO_RFCOMM BTHPROTO_RFCOMM
#else
struct sockaddr_rc_type {
  sa_family_t addressFamily;
  unsigned char	btAddr[6];
  unsigned char port;
};
# define BOOST_ASIO_OS_DEF_AF_BLUETOOTH AF_BLUETOOTH
# ifdef BTPROTO_RFCOMM
#  define BOOST_ASIO_OS_DEF_BTPROTO_RFCOMM BTPROTO_RFCOMM
# else
#  define BOOST_ASIO_OS_DEF_BTPROTO_RFCOMM 3
# endif
#endif
}
namespace bluetooth {
namespace detail {

// Helper class for implementing a Bluetooth endpoint.
class endpoint
{
public:
  // Default constructor.
  BOOST_ASIO_DECL endpoint() BOOST_ASIO_NOEXCEPT;

  // Construct an endpoint using an address and channel number.
  BOOST_ASIO_DECL endpoint(const boost::asio::bluetooth::address& addr,
      unsigned short channel_num) BOOST_ASIO_NOEXCEPT;

  // Get the underlying endpoint in the native type.
  boost::asio::detail::socket_addr_type* data() BOOST_ASIO_NOEXCEPT
  {
    return &data_.base;
  }

  // Get the underlying endpoint in the native type.
  const boost::asio::detail::socket_addr_type* data() const BOOST_ASIO_NOEXCEPT
  {
    return &data_.base;
  }

  // Get the underlying size of the endpoint in the native type.
  std::size_t size() const BOOST_ASIO_NOEXCEPT
  {
    return sizeof(boost::asio::detail::sockaddr_rc_type);
  }

  // Set the underlying size of the endpoint in the native type.
  BOOST_ASIO_DECL void resize(std::size_t size);

  // Get the capacity of the endpoint in the native type.
  std::size_t capacity() const BOOST_ASIO_NOEXCEPT
  {
    return sizeof(data_);
  }

  // Get the channel associated with the endpoint.
  BOOST_ASIO_DECL unsigned short channel() const BOOST_ASIO_NOEXCEPT;

  // Set the channel associated with the endpoint.
  BOOST_ASIO_DECL void channel(unsigned short channel_num) BOOST_ASIO_NOEXCEPT;

  // Get the Bluetooth address associated with the endpoint.
  BOOST_ASIO_DECL boost::asio::bluetooth::address address() const BOOST_ASIO_NOEXCEPT;

  // Set the Bluetooth address associated with the endpoint.
  BOOST_ASIO_DECL void address(const boost::asio::bluetooth::address& addr) BOOST_ASIO_NOEXCEPT;

  // Compare two endpoints for equality.
  BOOST_ASIO_DECL friend bool operator==(
      const endpoint& e1, const endpoint& e2) BOOST_ASIO_NOEXCEPT;

  // Compare endpoints for ordering.
  BOOST_ASIO_DECL friend bool operator<(
      const endpoint& e1, const endpoint& e2) BOOST_ASIO_NOEXCEPT;

  #if !defined(BOOST_ASIO_NO_IOSTREAM)
    // Convert to a string.
    BOOST_ASIO_DECL std::string to_string() const;
  #endif // !defined(BOOST_ASIO_NO_IOSTREAM)

private:
  // The underlying Bluetooth socket address.
  union data_union
  {
    boost::asio::detail::socket_addr_type base;
    boost::asio::detail::sockaddr_rc_type bt;
  } data_;
};

} // namespace detail
} // namespace bluetooth
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_ASIO_HEADER_ONLY)
# include <boost/asio/bluetooth/detail/impl/endpoint.ipp>
#endif // defined(BOOST_ASIO_HEADER_ONLY)

#endif // defined(BOOST_ASIO_HAS_BLUETOOTH_SOCKETS)

#endif // BOOST_ASIO_BLUETOOTH_DETAIL_ENDPOINT_HPP
