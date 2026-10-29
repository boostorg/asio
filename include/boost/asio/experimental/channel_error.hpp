//
// experimental/channel_error.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_EXPERIMENTAL_CHANNEL_ERROR_HPP
#define BOOST_ASIO_EXPERIMENTAL_CHANNEL_ERROR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/channel_error.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
BOOST_ASIO_INLINE_NAMESPACE_BEGIN
namespace experimental {
namespace error {
  using boost::asio::channel_errc::channel_errors;
  using boost::asio::channel_errc::channel_closed;
  using boost::asio::channel_errc::channel_cancelled;
  using boost::asio::channel_errc::get_channel_category;
  BOOST_ASIO_INLINE_OR_STATIC_VARIABLE const boost::system::error_category&
    channel_category BOOST_ASIO_UNUSED_VARIABLE
    = boost::asio::channel_errc::get_channel_category();
  using boost::asio::channel_errc::make_error_code;
} // namespace error
namespace channel_errc {
  using boost::asio::channel_errc::channel_closed;
  using boost::asio::channel_errc::channel_cancelled;
} // namespace channel_errc
} // namespace experimental
BOOST_ASIO_INLINE_NAMESPACE_END
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_EXPERIMENTAL_CHANNEL_ERROR_HPP
