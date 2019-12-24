//
// bluetooth/address.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2019 Huang Qinjin (huangqinjin at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_BLUETOOTH_ADDRESS_HPP
#define BOOST_ASIO_BLUETOOTH_ADDRESS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <string>
#include <boost/asio/detail/string_view.hpp>
#include <boost/system/error_code.hpp>

#if !defined(BOOST_ASIO_NO_IOSTREAM)
# include <iosfwd>
#endif // !defined(BOOST_ASIO_NO_IOSTREAM)

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace bluetooth {

/**
 * Implements Bluetooth addresses.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
class address
{
public:
  /// Get the address as a string.
  BOOST_ASIO_DECL std::string to_string() const;

  /// Compare two addresses for equality.
  BOOST_ASIO_DECL friend bool operator==(const address& a1,
      const address& a2) BOOST_ASIO_NOEXCEPT;

  /// Compare two addresses for inequality.
  friend bool operator!=(const address& a1,
      const address& a2) BOOST_ASIO_NOEXCEPT
  {
    return !(a1 == a2);
  }

  /// Compare addresses for ordering.
  BOOST_ASIO_DECL friend bool operator<(const address& a1,
      const address& a2) BOOST_ASIO_NOEXCEPT;

  /// Compare addresses for ordering.
  friend bool operator>(const address& a1,
      const address& a2) BOOST_ASIO_NOEXCEPT
  {
    return a2 < a1;
  }

  /// Compare addresses for ordering.
  friend bool operator<=(const address& a1,
      const address& a2) BOOST_ASIO_NOEXCEPT
  {
    return !(a2 < a1);
  }

  /// Compare addresses for ordering.
  friend bool operator>=(const address& a1,
      const address& a2) BOOST_ASIO_NOEXCEPT
  {
    return !(a1 < a2);
  }

public:
  unsigned char data[6];
};

/// Create an address from an Bluetooth address string in colon-separated or hyphen-separated form.
/**
 * @relates address
 */
BOOST_ASIO_DECL address make_address(const char* str);

/// Create an address from an Bluetooth address string in colon-separated or hyphen-separated form.
/**
 * @relates address
 */
BOOST_ASIO_DECL address make_address(const char* str,
    boost::system::error_code& ec) BOOST_ASIO_NOEXCEPT;

/// Create an address from an Bluetooth address string in colon-separated or hyphen-separated form.
/**
 * @relates address
 */
BOOST_ASIO_DECL address make_address(const std::string& str);

/// Create an address from an Bluetooth address string in colon-separated or hyphen-separated form.
/**
 * @relates address
 */
BOOST_ASIO_DECL address make_address(const std::string& str,
    boost::system::error_code& ec) BOOST_ASIO_NOEXCEPT;

#if defined(BOOST_ASIO_HAS_STRING_VIEW) \
  || defined(GENERATING_DOCUMENTATION)

/// Create an address from an Bluetooth address string in colon-separated or hyphen-separated form.
/**
 * @relates address
 */
BOOST_ASIO_DECL address make_address(string_view str);

/// Create an address from an Bluetooth address string in colon-separated or hyphen-separated form.
/**
 * @relates address
 */
BOOST_ASIO_DECL address make_address(string_view str,
    boost::system::error_code& ec) BOOST_ASIO_NOEXCEPT;

#endif // defined(BOOST_ASIO_HAS_STRING_VIEW)
       //  || defined(GENERATING_DOCUMENTATION)

#if !defined(BOOST_ASIO_NO_IOSTREAM)

/// Output an address as a string.
/**
 * Used to output a human-readable string for a specified address.
 *
 * @param os The output stream to which the string will be written.
 *
 * @param addr The address to be written.
 *
 * @return The output stream.
 *
 * @relates boost::asio::bluetooth::address
 */
template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(
    std::basic_ostream<Elem, Traits>& os, const address& addr)
{
  os << addr.to_string();
  return os;
}

#endif // !defined(BOOST_ASIO_NO_IOSTREAM)

} // namespace bluetooth
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_ASIO_HEADER_ONLY)
# include <boost/asio/bluetooth/detail/impl/address.ipp>
#endif // defined(BOOST_ASIO_HEADER_ONLY)

#endif // BOOST_ASIO_BLUETOOTH_ADDRESS_HPP
