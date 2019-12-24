//
// bluetooth/impl/address.ipp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2019 Huang Qinjin (huangqinjin at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_BLUETOOTH_DETAIL_IMPL_ADDRESS_IPP
#define BOOST_ASIO_BLUETOOTH_DETAIL_IMPL_ADDRESS_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <typeinfo>
#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/bluetooth/address.hpp>
#include <boost/system/system_error.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace bluetooth {

address make_address(const char* str)
{
  boost::system::error_code ec;
  address addr = make_address(str, ec);
  boost::asio::detail::throw_error(ec);
  return addr;
}

address make_address(const char* str,
    boost::system::error_code& ec) BOOST_ASIO_NOEXCEPT
{
  unsigned d[6];
  int r = sscanf(str, "%02X%*[:-]%02X%*[:-]%02X%*[:-]%02X%*[:-]%02X%*[:-]%02X", 
    &d[5], &d[4], &d[3], &d[2], &d[1], &d[0]);

  address addr;
  if (r == 6)
  {
    while (--r >= 0)
    {
      if (d[r] > 0xffu)
        break;
      addr.data[r] = d[r];
    }
  }
  else
  {
    r = 0;
  }
  
  if (r == -1)
  {
    ec = boost::system::error_code();
    return addr;
  }

  ec = boost::asio::error::invalid_argument;
  return address();
}

address make_address(const std::string& str)
{
  return make_address(str.c_str());
}

address make_address(const std::string& str,
    boost::system::error_code& ec) BOOST_ASIO_NOEXCEPT
{
  return make_address(str.c_str(), ec);
}

#if defined(BOOST_ASIO_HAS_STRING_VIEW)

address make_address(string_view str)
{
  return make_address(static_cast<std::string>(str));
}

address make_address(string_view str,
    boost::system::error_code& ec) BOOST_ASIO_NOEXCEPT
{
  return make_address(static_cast<std::string>(str), ec);
}

#endif // defined(BOOST_ASIO_HAS_STRING_VIEW)

std::string address::to_string() const
{
  std::string s(17, '\0');
  sprintf((char*)s.c_str(), "%02X:%02X:%02X:%02X:%02X:%02X",
    data[5], data[4], data[3], data[2], data[1], data[0]);
    return s;
}

bool operator==(const address& a1, const address& a2) BOOST_ASIO_NOEXCEPT
{
  return memcmp(&a1.data, &a2.data, sizeof(a1.data)) == 0;
}

bool operator<(const address& a1, const address& a2) BOOST_ASIO_NOEXCEPT
{
  return memcmp(&a1.data, &a2.data, sizeof(a1.data)) < 0;
}

} // namespace bluetooth
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_BLUETOOTH_DETAIL_IMPL_ADDRESS_IPP
