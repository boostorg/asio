//
// bluetooth/detail/impl/endpoint.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2019 Huang Qinjin (huangqinjin at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_BLUETOOTH_DETAIL_IMPL_ENDPOINT_IPP
#define BOOST_ASIO_BLUETOOTH_DETAIL_IMPL_ENDPOINT_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_BLUETOOTH_SOCKETS)

#include <cstring>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/bluetooth/detail/endpoint.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace bluetooth {
namespace detail {

endpoint::endpoint() BOOST_ASIO_NOEXCEPT
  : data_()
{
  data_.bt.addressFamily = BOOST_ASIO_OS_DEF(AF_BLUETOOTH);
}

endpoint::endpoint(const boost::asio::bluetooth::address& addr,
    unsigned short channel_num) BOOST_ASIO_NOEXCEPT
  : data_()
{
  data_.bt.addressFamily = BOOST_ASIO_OS_DEF(AF_BLUETOOTH);
  address(addr);
  channel(channel_num);
}

void endpoint::resize(std::size_t new_size)
{
  if (new_size > sizeof(data_))
  {
    boost::system::error_code ec(boost::asio::error::invalid_argument);
    boost::asio::detail::throw_error(ec);
  }
}

unsigned short endpoint::channel() const BOOST_ASIO_NOEXCEPT
{
  return data_.bt.port;
}

void endpoint::channel(unsigned short channel_num) BOOST_ASIO_NOEXCEPT
{
  data_.bt.port = channel_num;
}

boost::asio::bluetooth::address endpoint::address() const BOOST_ASIO_NOEXCEPT
{
  boost::asio::bluetooth::address addr;
  memcpy(&addr.data, &data_.bt.btAddr, sizeof(addr.data));
  return addr;
}

void endpoint::address(const boost::asio::bluetooth::address& addr) BOOST_ASIO_NOEXCEPT
{
  memcpy(&data_.bt.btAddr, &addr.data, sizeof(addr.data));
}

bool operator==(const endpoint& e1, const endpoint& e2) BOOST_ASIO_NOEXCEPT
{
  return e1.address() == e2.address() && e1.channel() == e2.channel();
}

bool operator<(const endpoint& e1, const endpoint& e2) BOOST_ASIO_NOEXCEPT
{
  if (e1.address() < e2.address())
    return true;
  if (e1.address() != e2.address())
    return false;
  return (e1.channel() < e2.channel());
}

#if !defined(BOOST_ASIO_NO_IOSTREAM)
std::string endpoint::to_string() const
{
  std::ostringstream tmp_os;
  tmp_os.imbue(std::locale::classic());
  tmp_os << '[' << address() << ']' << ':' << channel();

  return tmp_os.str();
}
#endif // !defined(BOOST_ASIO_NO_IOSTREAM)

} // namespace detail
} // namespace bluetooth
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_BLUETOOTH_SOCKETS)

#endif // BOOST_ASIO_BLUETOOTH_DETAIL_IMPL_ENDPOINT_IPP
