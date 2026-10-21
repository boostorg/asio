//
// experimental/basic_channel.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_EXPERIMENTAL_BASIC_CHANNEL_HPP
#define BOOST_ASIO_EXPERIMENTAL_BASIC_CHANNEL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/basic_channel.hpp>
#include <boost/asio/channel_traits.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
BOOST_ASIO_INLINE_NAMESPACE_BEGIN
namespace experimental {

template <typename Executor,
    typename Traits = boost::asio::channel_traits<>,
    typename... Signatures>
using basic_channel =
  boost::asio::basic_channel<Executor, Traits, Signatures...>;

} // namespace experimental
BOOST_ASIO_INLINE_NAMESPACE_END
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_EXPERIMENTAL_BASIC_CHANNEL_HPP
