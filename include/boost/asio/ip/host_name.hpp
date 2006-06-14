//
// host_name.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IP_HOST_NAME_HPP
#define BOOST_ASIO_IP_HOST_NAME_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <string>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error_handler.hpp>
#include <boost/asio/detail/socket_ops.hpp>

namespace boost {
namespace asio {
namespace ip {

/// Get the current host name.
std::string host_name();

/// Get the current host name.
template <typename Error_Handler>
std::string host_name(Error_Handler error_handler);

inline std::string host_name()
{
  return host_name(boost::asio::throw_error());
}

template <typename Error_Handler>
std::string host_name(Error_Handler error_handler)
{
  char name[1024];
  if (boost::asio::detail::socket_ops::gethostname(name, sizeof(name)) != 0)
  {
    boost::asio::error error(boost::asio::detail::socket_ops::get_error());
    error_handler(error);
    return std::string();
  }
  return std::string(name);
}

} // namespace ip
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_IP_HOST_NAME_HPP
