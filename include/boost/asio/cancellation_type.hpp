//
// cancellation_type.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_CANCELLATION_TYPE_HPP
#define BOOST_ASIO_CANCELLATION_TYPE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {

#if defined(BOOST_ASIO_HAS_ENUM_CLASS) \
  || defined(GENERATING_DOCUMENTATION)

/// Enumeration representing the different types of cancellation that may
/// be requested from or implemented by an asynchronous operation.
enum class cancellation_type : unsigned int
{
  /// Bitmask representing no types of cancellation.
  none = 0,

  /// Following cancellation, the only safe operation on the I/O object is
  /// destruction.
  terminal = 1,

  /// Following cancellation, the I/O object is in a well-known state, and
  /// may be used for further operations.
  interrupt = 2,

  /// Following cancellation, the operation has had no side effects and may be
  /// restarted.
  restartable = 4,

  /// Bitmask representing all types of cancellation.
  all = 0xFFFFFFFF
};

typedef cancellation_type cancellation_type_t;

#else // defined(BOOST_ASIO_HAS_ENUM_CLASS)
      //  || defined(GENERATING_DOCUMENTATION)

namespace cancellation_type {

enum cancellation_type_t
{
  none = 0,
  terminal = 1,
  interrupt = 2,
  restartable = 4,
  all = 0xFFFFFFFF
};

} // namespace cancellation_type

typedef cancellation_type::cancellation_type_t cancellation_type_t;

#endif // defined(BOOST_ASIO_HAS_ENUM_CLASS)
       //  || defined(GENERATING_DOCUMENTATION)

inline BOOST_ASIO_CONSTEXPR bool operator!(cancellation_type_t x)
{
  return static_cast<unsigned int>(x) == 0;
}

inline BOOST_ASIO_CONSTEXPR cancellation_type_t operator&(
    cancellation_type_t x, cancellation_type_t y)
{
  return static_cast<cancellation_type_t>(
      static_cast<unsigned int>(x) & static_cast<unsigned int>(y));
}

inline BOOST_ASIO_CONSTEXPR cancellation_type_t operator|(
    cancellation_type_t x, cancellation_type_t y)
{
  return static_cast<cancellation_type_t>(
      static_cast<unsigned int>(x) | static_cast<unsigned int>(y));
}

inline BOOST_ASIO_CONSTEXPR cancellation_type_t operator^(
    cancellation_type_t x, cancellation_type_t y)
{
  return static_cast<cancellation_type_t>(
      static_cast<unsigned int>(x) ^ static_cast<unsigned int>(y));
}

inline BOOST_ASIO_CONSTEXPR cancellation_type_t operator~(cancellation_type_t x)
{
  return static_cast<cancellation_type_t>(~static_cast<unsigned int>(x));
}

inline cancellation_type_t& operator&=(
    cancellation_type_t& x, cancellation_type_t y)
{
  x = x & y;
  return x;
}

inline cancellation_type_t& operator|=(
    cancellation_type_t& x, cancellation_type_t y)
{
  x = x | y;
  return x;
}

inline cancellation_type_t& operator^=(
    cancellation_type_t& x, cancellation_type_t y)
{
  x = x ^ y;
  return x;
}

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_CANCELLATION_TYPE_HPP
