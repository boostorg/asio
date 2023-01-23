//
// vm/detail/endpoint.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2023 OLiver Kowalke (oliver dot kowalke at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_VM_DETAIL_ENDPOINT_HPP
#define BOOST_ASIO_VM_DETAIL_ENDPOINT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_VM_SOCKETS)

#include <cstddef>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/detail/push_options.hpp>
#include <boost/asio/vm/cid.hpp>

namespace boost {
namespace asio {
namespace vm {
namespace detail {

// Helper class for implementing a VSOCK endpoint.
class endpoint
{
public:
  // Default constructor.
  BOOST_ASIO_DECL endpoint() BOOST_ASIO_NOEXCEPT;

  // Construct an endpoint using the specified path name.
  BOOST_ASIO_DECL endpoint(unsigned short port_num) BOOST_ASIO_NOEXCEPT;

  // Construct an endpoint using the specified path name.
  BOOST_ASIO_DECL endpoint(const boost::asio::vm::cid& cid, unsigned short port_num) BOOST_ASIO_NOEXCEPT;

  // Copy constructor.
  endpoint(const endpoint& other) BOOST_ASIO_NOEXCEPT
    : data_(other.data_)
  {
  }

  // Assign from another endpoint.
  endpoint& operator=(const endpoint& other) BOOST_ASIO_NOEXCEPT
  {
    data_ = other.data_;
    return *this;
  }

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
    return sizeof(boost::asio::detail::sockaddr_vm_type);
  }

  // Set the underlying size of the endpoint in the native type.
  BOOST_ASIO_DECL void resize(std::size_t size);

  // Get the capacity of the endpoint in the native type.
  std::size_t capacity() const BOOST_ASIO_NOEXCEPT
  {
    return sizeof(data_);
  }

  // Get the port associated with the endpoint.
  BOOST_ASIO_DECL unsigned short port() const BOOST_ASIO_NOEXCEPT;

  // Set the port associated with the endpoint.
  BOOST_ASIO_DECL void port(unsigned short port_num) BOOST_ASIO_NOEXCEPT;

  // Get the CID associated with the endpoint.
  BOOST_ASIO_DECL boost::asio::vm::cid cid() const BOOST_ASIO_NOEXCEPT;

  // Set the CID associated with the endpoint.
  BOOST_ASIO_DECL void cid(
      const boost::asio::vm::cid& addr) BOOST_ASIO_NOEXCEPT;

  // Compare two endpoints for equality.
  BOOST_ASIO_DECL friend bool operator==(const endpoint& e1,
      const endpoint& e2) BOOST_ASIO_NOEXCEPT;

  // Compare endpoints for ordering.
  BOOST_ASIO_DECL friend bool operator<(
      const endpoint& e1, const endpoint& e2) BOOST_ASIO_NOEXCEPT;

private:
  // The underlying VSOCK socket address.
  union data_union
  {
    boost::asio::detail::socket_addr_type base;
    boost::asio::detail::sockaddr_vm_type vm;
  } data_;

  // The length of the path associated with the endpoint.
  std::size_t path_length_;

  // Initialise with a specified path.
  BOOST_ASIO_DECL void init(const char* path, std::size_t path_length);
};

} // namespace detail
} // namespace vm
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_ASIO_HEADER_ONLY)
# include <boost/asio/vm/detail/impl/endpoint.ipp>
#endif // defined(BOOST_ASIO_HEADER_ONLY)

#endif // defined(BOOST_ASIO_HAS_VM_SOCKETS)

#endif // BOOST_ASIO_VM_DETAIL_ENDPOINT_HPP
