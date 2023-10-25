//
// traits/connect_free.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2023 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_TRAITS_CONNECT_FREE_HPP
#define BOOST_ASIO_TRAITS_CONNECT_FREE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/detail/type_traits.hpp>

#if defined(BOOST_ASIO_HAS_WORKING_EXPRESSION_SFINAE)
# define BOOST_ASIO_HAS_DEDUCED_CONNECT_FREE_TRAIT 1
#endif // defined(BOOST_ASIO_HAS_WORKING_EXPRESSION_SFINAE)

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace traits {

template <typename S, typename R, typename = void>
struct connect_free_default;

template <typename S, typename R, typename = void>
struct connect_free;

} // namespace traits
namespace detail {

struct no_connect_free
{
  static constexpr bool is_valid = false;
  static constexpr bool is_noexcept = false;
};

#if defined(BOOST_ASIO_HAS_DEDUCED_CONNECT_FREE_TRAIT)

template <typename S, typename R, typename = void>
struct connect_free_trait : no_connect_free
{
};

template <typename S, typename R>
struct connect_free_trait<S, R,
  void_t<
    decltype(connect(declval<S>(), declval<R>()))
  >>
{
  static constexpr bool is_valid = true;

  using result_type = decltype(
    connect(declval<S>(), declval<R>()));

  static constexpr bool is_noexcept =
    noexcept(connect(declval<S>(), declval<R>()));
};

#else // defined(BOOST_ASIO_HAS_DEDUCED_CONNECT_FREE_TRAIT)

template <typename S, typename R, typename = void>
struct connect_free_trait :
  conditional_t<
    is_same<S, remove_reference_t<S>>::value
      && is_same<R, decay_t<R>>::value,
    conditional_t<
      is_same<S, add_const_t<S>>::value,
      no_connect_free,
      traits::connect_free<add_const_t<S>, R>
    >,
    traits::connect_free<
      remove_reference_t<S>,
      decay_t<R>>
  >
{
};

#endif // defined(BOOST_ASIO_HAS_DEDUCED_CONNECT_FREE_TRAIT)

} // namespace detail
namespace traits {

template <typename S, typename R, typename>
struct connect_free_default :
  detail::connect_free_trait<S, R>
{
};

template <typename S, typename R, typename>
struct connect_free :
  connect_free_default<S, R>
{
};

} // namespace traits
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_TRAITS_CONNECT_FREE_HPP
