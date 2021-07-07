//
// bluetooth/basic_endpoint.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2019 Huang Qinjin (huangqinjin at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_BLUETOOTH_BASIC_ENDPOINT_HPP
#define BOOST_ASIO_BLUETOOTH_BASIC_ENDPOINT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_BLUETOOTH_SOCKETS) \
  || defined(GENERATING_DOCUMENTATION)

#include <boost/asio/bluetooth/detail/endpoint.hpp>

#if !defined(BOOST_ASIO_NO_IOSTREAM)
# include <iosfwd>
#endif // !defined(BOOST_ASIO_NO_IOSTREAM)

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace bluetooth {

/// Describes an endpoint for a Bluetooth socket.
/**
 * The boost::asio::bluetooth::basic_endpoint class template describes an endpoint
 * that may be associated with a particular Bluetooth socket.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 *
 * @par Concepts:
 * Endpoint.
 */
template <typename Protocol>
class basic_endpoint
{
public:
  /// The protocol type associated with the endpoint.
  typedef Protocol protocol_type;

  /// The type of the endpoint structure. This type is dependent on the
  /// underlying implementation of the socket layer.
#if defined(GENERATING_DOCUMENTATION)
  typedef implementation_defined data_type;
#else
  typedef boost::asio::detail::socket_addr_type data_type;
#endif

  /// Default constructor.
  basic_endpoint() BOOST_ASIO_NOEXCEPT
    : impl_()
  {
  }

  // Construct an endpoint using an address and channel number.
  basic_endpoint(const boost::asio::bluetooth::address& addr,
      unsigned short channel_num) BOOST_ASIO_NOEXCEPT
    : impl_(addr, channel_num)
  {
  }

  /// The protocol associated with the endpoint.
  protocol_type protocol() const BOOST_ASIO_NOEXCEPT
  {
    return protocol_type();
  }

  /// Get the underlying endpoint in the native type.
  data_type* data() BOOST_ASIO_NOEXCEPT
  {
    return impl_.data();
  }

  /// Get the underlying endpoint in the native type.
  const data_type* data() const BOOST_ASIO_NOEXCEPT
  {
    return impl_.data();
  }

  /// Get the underlying size of the endpoint in the native type.
  std::size_t size() const BOOST_ASIO_NOEXCEPT
  {
    return impl_.size();
  }

  /// Set the underlying size of the endpoint in the native type.
  void resize(std::size_t new_size)
  {
    impl_.resize(new_size);
  }

  /// Get the capacity of the endpoint in the native type.
  std::size_t capacity() const BOOST_ASIO_NOEXCEPT
  {
    return impl_.capacity();
  }

  // Get the channel associated with the endpoint.
  unsigned short channel() const BOOST_ASIO_NOEXCEPT
  {
    return impl_.channel();
  }

  // Set the channel associated with the endpoint.
  void channel(unsigned short channel_num) BOOST_ASIO_NOEXCEPT
  {
    return impl_.channel(channel_num);
  }

  // Get the Bluetooth address associated with the endpoint.
  boost::asio::bluetooth::address address() const BOOST_ASIO_NOEXCEPT
  {
    return impl_.address();
  }

  // Set the Bluetooth address associated with the endpoint.
  void address(const boost::asio::bluetooth::address& addr) BOOST_ASIO_NOEXCEPT
  {
    return impl_.address(addr);
  }

  /// Compare two endpoints for equality.
  friend bool operator==(const basic_endpoint<Protocol>& e1,
      const basic_endpoint<Protocol>& e2)
  {
    return e1.impl_ == e2.impl_;
  }

  /// Compare two endpoints for inequality.
  friend bool operator!=(const basic_endpoint<Protocol>& e1,
      const basic_endpoint<Protocol>& e2)
  {
    return !(e1.impl_ == e2.impl_);
  }

  /// Compare endpoints for ordering.
  friend bool operator<(const basic_endpoint<Protocol>& e1,
      const basic_endpoint<Protocol>& e2)
  {
    return e1.impl_ < e2.impl_;
  }

  /// Compare endpoints for ordering.
  friend bool operator>(const basic_endpoint<Protocol>& e1,
      const basic_endpoint<Protocol>& e2)
  {
    return e2.impl_ < e1.impl_;
  }

  /// Compare endpoints for ordering.
  friend bool operator<=(const basic_endpoint<Protocol>& e1,
      const basic_endpoint<Protocol>& e2)
  {
    return !(e2 < e1);
  }

  /// Compare endpoints for ordering.
  friend bool operator>=(const basic_endpoint<Protocol>& e1,
      const basic_endpoint<Protocol>& e2)
  {
    return !(e1 < e2);
  }

private:
  // The underlying Bluetooth endpoint.
  boost::asio::bluetooth::detail::endpoint impl_;
};

/// Output an endpoint as a string.
/**
 * Used to output a human-readable string for a specified endpoint.
 *
 * @param os The output stream to which the string will be written.
 *
 * @param endpoint The endpoint to be written.
 *
 * @return The output stream.
 *
 * @relates boost::asio::bluetooth::basic_endpoint
 */
template <typename Elem, typename Traits, typename Protocol>
std::basic_ostream<Elem, Traits>& operator<<(
    std::basic_ostream<Elem, Traits>& os,
    const basic_endpoint<Protocol>& endpoint)
{
  os << boost::asio::bluetooth::detail::endpoint(endpoint.address(), endpoint.channel()).to_string();
  return os;
}

} // namespace bluetooth
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_BLUETOOTH_SOCKETS)
       //   || defined(GENERATING_DOCUMENTATION)

#endif // BOOST_ASIO_BLUETOOTH_BASIC_ENDPOINT_HPP
