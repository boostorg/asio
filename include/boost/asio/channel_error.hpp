//
// channel_error.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_CHANNEL_ERROR_HPP
#define BOOST_ASIO_CHANNEL_ERROR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/system/error_code.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
BOOST_ASIO_INLINE_NAMESPACE_BEGIN
namespace channel_errc {

enum channel_errors
{
  /// The channel was closed.
  channel_closed = 1,

  /// The channel was cancelled.
  channel_cancelled = 2
};

extern BOOST_ASIO_DECL
const boost::system::error_category& get_channel_category();

BOOST_ASIO_INLINE_OR_STATIC_VARIABLE const boost::system::error_category&
  channel_category BOOST_ASIO_UNUSED_VARIABLE
  = boost::asio::channel_errc::get_channel_category();

} // namespace channel_errc
BOOST_ASIO_INLINE_NAMESPACE_END
} // namespace asio
} // namespace boost

namespace boost {
namespace system {

template<> struct is_error_code_enum<
    boost::asio::channel_errc::channel_errors>
{
  static const bool value = true;
};

} // namespace system
} // namespace boost

namespace boost {
namespace asio {
BOOST_ASIO_INLINE_NAMESPACE_BEGIN
namespace channel_errc {

inline boost::system::error_code make_error_code(channel_errors e)
{
  return boost::system::error_code(
      static_cast<int>(e), get_channel_category());
}

} // namespace channel_errc
BOOST_ASIO_INLINE_NAMESPACE_END
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_ASIO_HEADER_ONLY)
# include <boost/asio/impl/channel_error.ipp>
#endif // defined(BOOST_ASIO_HEADER_ONLY)

#endif // BOOST_ASIO_CHANNEL_ERROR_HPP
