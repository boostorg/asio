//
// ip/impl/tcp.ipp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IP_IMPL_TCP_IPP
#define BOOST_ASIO_IP_IMPL_TCP_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace ip {

#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)
boost::asio::detail::apple_nw_ptr<nw_parameters_t>
tcp::apple_nw_create_parameters() const
{
  boost::asio::detail::apple_nw_ptr<nw_parameters_t> parameters(
      nw_parameters_create_secure_tcp(
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

void tcp::no_delay::apple_nw_set(const void* self,
    nw_parameters_t parameters, nw_connection_t connection,
    boost::system::error_code& ec)
{
  const no_delay* option = static_cast<const no_delay*>(self);

  if (connection)
  {
    ec = boost::asio::error::already_connected;
    return;
  }

  boost::asio::detail::apple_nw_ptr<nw_protocol_stack_t> protocol_stack(
      nw_parameters_copy_default_protocol_stack(parameters));

  boost::asio::detail::apple_nw_ptr<nw_protocol_options_t> transport_options(
      nw_protocol_stack_copy_transport_protocol(protocol_stack));

  nw_tcp_options_set_no_delay(transport_options, option->value());

  ec = boost::system::error_code();
}

void tcp::no_delay::apple_nw_set(const void* self,
    nw_parameters_t parameters, nw_listener_t listener,
    boost::system::error_code& ec)
{
  const no_delay* option = static_cast<const no_delay*>(self);

  if (listener)
  {
    ec = boost::asio::error::already_open;
    return;
  }

  boost::asio::detail::apple_nw_ptr<nw_protocol_stack_t> protocol_stack(
      nw_parameters_copy_default_protocol_stack(parameters));

  boost::asio::detail::apple_nw_ptr<nw_protocol_options_t> transport_options(
      nw_protocol_stack_copy_transport_protocol(protocol_stack));

  nw_tcp_options_set_no_delay(transport_options, option->value());

  ec = boost::system::error_code();
}

void tcp::no_delay::apple_nw_get(void*, nw_parameters_t,
    nw_connection_t, boost::system::error_code& ec)
{
  ec = boost::asio::error::operation_not_supported;
}

void tcp::no_delay::apple_nw_get(void*, nw_parameters_t,
    nw_listener_t, boost::system::error_code& ec)
{
  ec = boost::asio::error::operation_not_supported;
}
#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

} // namespace ip
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_IP_IMPL_TCP_IPP
