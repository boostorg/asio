//
// vm/cid.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2023 OLiver Kowalke (oliver dot kowalke at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_VM_CID_HPP
#define BOOST_ASIO_VM_CID_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_VM_SOCKETS)

#include <boost/asio/detail/cstdint.hpp>
#include <boost/asio/detail/socket_types.hpp>

#if !defined(BOOST_ASIO_NO_IOSTREAM)
# include <iosfwd>
#endif // !defined(BOOST_ASIO_NO_IOSTREAM)

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace vm {

/// Implements VSOCK style context ID.
/**
 * The boost::asio::vm::cid class provides the ability to use and
 * manipulate VSOCK context IDs.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
class cid
{
public:
  /// The type used to represent an address as an unsigned integer.
  typedef uint_least32_t uint_type;

  /// Default constructor.
  cid() BOOST_ASIO_NOEXCEPT
  {
    addr_.svm_cid = VMADDR_CID_ANY;
  }

  /// Construct an address from an unsigned integer.
  BOOST_ASIO_DECL explicit cid(uint_type c);

  /// Copy constructor.
  cid(const cid& other) BOOST_ASIO_NOEXCEPT
    : addr_(other.addr_)
  {
  }

#if defined(BOOST_ASIO_HAS_MOVE)
  /// Move constructor.
  cid(cid&& other) BOOST_ASIO_NOEXCEPT
    : addr_(other.addr_)
  {
  }
#endif // defined(BOOST_ASIO_HAS_MOVE)

  /// Assign from another address.
  cid& operator=(const cid& other) BOOST_ASIO_NOEXCEPT
  {
    addr_ = other.addr_;
    return *this;
  }

#if defined(BOOST_ASIO_HAS_MOVE)
  /// Move-assign from another address.
  cid& operator=(cid&& other) BOOST_ASIO_NOEXCEPT
  {
    addr_ = other.addr_;
    return *this;
  }
#endif // defined(BOOST_ASIO_HAS_MOVE)

  /// Get the address as an unsigned integer
  BOOST_ASIO_DECL uint_type to_uint() const BOOST_ASIO_NOEXCEPT;

  /// Determine whether the address is a loopback address.
  BOOST_ASIO_DECL bool is_local() const BOOST_ASIO_NOEXCEPT;

  /// Determine whether the address is unspecified.
  BOOST_ASIO_DECL bool is_hypervisor() const BOOST_ASIO_NOEXCEPT;

  /// Determine whether the address is a any address.
  BOOST_ASIO_DECL bool is_any() const BOOST_ASIO_NOEXCEPT;

  /// Compare two addresses for equality.
  friend bool operator==(const cid& a1,
      const cid& a2) BOOST_ASIO_NOEXCEPT
  {
    return a1.addr_.svm_cid == a2.addr_.svm_cid;
  }

  /// Compare two addresses for inequality.
  friend bool operator!=(const cid& a1,
      const cid& a2) BOOST_ASIO_NOEXCEPT
  {
    return a1.addr_.svm_cid != a2.addr_.svm_cid;
  }

  /// Compare addresses for ordering.
  friend bool operator<(const cid& a1,
      const cid& a2) BOOST_ASIO_NOEXCEPT
  {
    return a1.to_uint() < a2.to_uint();
  }

  /// Compare addresses for ordering.
  friend bool operator>(const cid& a1,
      const cid& a2) BOOST_ASIO_NOEXCEPT
  {
    return a1.to_uint() > a2.to_uint();
  }

  /// Compare addresses for ordering.
  friend bool operator<=(const cid& a1,
      const cid& a2) BOOST_ASIO_NOEXCEPT
  {
    return a1.to_uint() <= a2.to_uint();
  }

  /// Compare addresses for ordering.
  friend bool operator>=(const cid& a1,
      const cid& a2) BOOST_ASIO_NOEXCEPT
  {
    return a1.to_uint() >= a2.to_uint();
  }

  /// Obtain an address object that represents any address.
  static cid any() BOOST_ASIO_NOEXCEPT
  {
    return cid();
  }

  /// Obtain an address object that represents the loopback address.
  static cid local() BOOST_ASIO_NOEXCEPT
  {
    return cid(VMADDR_CID_LOCAL);
  }

  /// Obtain an address object that represents the broadcast address.
  static cid hypervisor() BOOST_ASIO_NOEXCEPT
  {
    return cid(VMADDR_CID_HYPERVISOR);
  }

private:
  // The underlying VSOCK address.
  boost::asio::detail::sockaddr_vm_type addr_;
};

/// Create an IPv4 address from an unsigned integer in host byte order.
/**
 * @relates cid
 */
inline cid make_cid(cid::uint_type id)
{
  return cid(id);
}

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
 * @relates boost::asio::vm::cid
 */
template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(
    std::basic_ostream<Elem, Traits>& os, const cid& addr);

#endif // !defined(BOOST_ASIO_NO_IOSTREAM)

} // namespace vm
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/vm/impl/cid.hpp>
#if defined(BOOST_ASIO_HEADER_ONLY)
# include <boost/asio/vm/impl/cid.ipp>
#endif // defined(BOOST_ASIO_HEADER_ONLY)
 
#endif // defined(BOOST_ASIO_HAS_VM_SOCKETS)

#endif // BOOST_ASIO_VM_CID_HPP
