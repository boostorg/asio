//
// piping.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_PIPING_HPP
#define BOOST_ASIO_PIPING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/detail/type_traits.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
BOOST_ASIO_INLINE_NAMESPACE_BEGIN

/// Trait for detecting objects usable on the left of the pipe operator.
template <typename T, typename = void>
struct supports_piping : false_type
{
};

#if defined(BOOST_ASIO_HAS_VARIABLE_TEMPLATES)

template <typename T>
constexpr const bool supports_piping_v = supports_piping<T>::value;

#endif // defined(BOOST_ASIO_HAS_VARIABLE_TEMPLATES)

/// Pipe operator used to chain operations that support piping.
template <typename Head, typename Tail>
inline auto operator|(Head head, Tail&& tail)
  -> constraint_t<
      supports_piping<Head>::value,
      decltype(static_cast<Head&&>(head)(static_cast<Tail&&>(tail)))
    >
{
  return static_cast<Head&&>(head)(static_cast<Tail&&>(tail));
}

BOOST_ASIO_INLINE_NAMESPACE_END
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_PIPING_HPP
