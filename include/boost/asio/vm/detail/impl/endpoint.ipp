//
// vm/detail/impl/endpoint.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2023 OLiver Kowalke (oliver dot kowalke at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_VM_DETAIL_IMPL_ENDPOINT_IPP
#define BOOST_ASIO_VM_DETAIL_IMPL_ENDPOINT_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_VM_SOCKETS)

#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/vm/detail/endpoint.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace vm {
namespace detail {

endpoint::endpoint() BOOST_ASIO_NOEXCEPT
{
  memset(&data_, 0, sizeof(data_));
  data_.vm.svm_family = AF_VSOCK;
  data_.vm.svm_port = 0;
  data_.vm.svm_cid = VMADDR_CID_ANY;
}

endpoint::endpoint(unsigned short port_num) BOOST_ASIO_NOEXCEPT
{
  memset(&data_, 0, sizeof(data_));
  data_.vm.svm_family = AF_VSOCK;
  data_.vm.svm_port = port_num;
  data_.vm.svm_cid = VMADDR_CID_ANY;
}

endpoint::endpoint(const boost::asio::vm::cid& cid, unsigned short port_num) BOOST_ASIO_NOEXCEPT
{
  memset(&data_, 0, sizeof(data_));
  data_.vm.svm_family = AF_VSOCK;
  data_.vm.svm_port = port_num;
  data_.vm.svm_cid = cid.to_uint();
}

void endpoint::resize(std::size_t new_size)
{
  if (new_size > sizeof(boost::asio::detail::sockaddr_storage_type))
  {
    boost::system::error_code ec(boost::asio::error::invalid_argument);
    boost::asio::detail::throw_error(ec);
  }
}

unsigned short endpoint::port() const BOOST_ASIO_NOEXCEPT
{
  return data_.vm.svm_port;
}

void endpoint::port(unsigned short port_num) BOOST_ASIO_NOEXCEPT
{
    data_.vm.svm_port = port_num;
}

boost::asio::vm::cid endpoint::cid() const BOOST_ASIO_NOEXCEPT
{
    return boost::asio::vm::cid(data_.vm.svm_cid);
}

void endpoint::cid(const boost::asio::vm::cid& cid) BOOST_ASIO_NOEXCEPT
{
  endpoint tmp_endpoint(cid, port());
  data_ = tmp_endpoint.data_;
}

bool operator==(const endpoint& e1, const endpoint& e2) BOOST_ASIO_NOEXCEPT
{
  return e1.cid() == e2.cid() && e1.port() == e2.port();
}

bool operator<(const endpoint& e1, const endpoint& e2) BOOST_ASIO_NOEXCEPT
{
  if (e1.cid() < e2.cid())
    return true;
  if (e1.cid() != e2.cid())
    return false;
  return e1.port() < e2.port();
}

} // namespace detail
} // namespace vm
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_VM_SOCKETS)

#endif // BOOST_ASIO_VM_DETAIL_IMPL_ENDPOINT_IPP
