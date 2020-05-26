//
// ip/impl/udp.ipp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IP_IMPL_UDP_IPP
#define BOOST_ASIO_IP_IMPL_UDP_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/ip/udp.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace ip {

#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
boost::asio::detail::apple_nw_ptr<nw_parameters_t>
udp::apple_nw_create_parameters() const
{
  boost::asio::detail::apple_nw_ptr<nw_parameters_t> parameters(
      nw_parameters_create_secure_udp(
        NW_PARAMETERS_DISABLE_PROTOCOL,
        NW_PARAMETERS_DEFAULT_CONFIGURATION));

  boost::asio::detail::apple_nw_ptr<nw_protocol_stack_t> protocol_stack(
      nw_parameters_copy_default_protocol_stack(parameters));

  boost::asio::detail::apple_nw_ptr<nw_protocol_options_t> ip_options(
      nw_protocol_stack_copy_internet_protocol(protocol_stack));

  if (family_ == BOOST_ASIO_OS_DEF(AF_INET))
    nw_ip_options_set_version(ip_options, nw_ip_version_4);
  else if (family_ == BOOST_ASIO_OS_DEF(AF_INET6))
    nw_ip_options_set_version(ip_options, nw_ip_version_6);

  return parameters;
}
#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

} // namespace ip
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_IP_IMPL_UDP_IPP
