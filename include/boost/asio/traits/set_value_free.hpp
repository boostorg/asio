//
// traits/set_value_free.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2023 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_TRAITS_SET_VALUE_FREE_HPP
#define BOOST_ASIO_TRAITS_SET_VALUE_FREE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/detail/type_traits.hpp>

#if defined(BOOST_ASIO_HAS_WORKING_EXPRESSION_SFINAE)
# define BOOST_ASIO_HAS_DEDUCED_SET_VALUE_FREE_TRAIT 1
#endif // defined(BOOST_ASIO_HAS_WORKING_EXPRESSION_SFINAE)

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace traits {

template <typename T, typename Vs, typename = void>
struct set_value_free_default;

template <typename T, typename Vs, typename = void>
struct set_value_free;

} // namespace traits
namespace detail {

struct no_set_value_free
{
  static constexpr bool is_valid = false;
  static constexpr bool is_noexcept = false;
};

#if defined(BOOST_ASIO_HAS_DEDUCED_SET_VALUE_FREE_TRAIT)

template <typename T, typename Vs, typename = void>
struct set_value_free_trait : no_set_value_free
{
};

template <typename T, typename... Vs>
struct set_value_free_trait<T, void(Vs...),
  void_t<
    decltype(set_value(declval<T>(), declval<Vs>()...))
  >>
{
  static constexpr bool is_valid = true;

  using result_type = decltype(
    set_value(declval<T>(), declval<Vs>()...));

  static constexpr bool is_noexcept =
    noexcept(set_value(declval<T>(), declval<Vs>()...));
};

#else // defined(BOOST_ASIO_HAS_DEDUCED_SET_VALUE_FREE_TRAIT)

template <typename T, typename Vs, typename = void>
struct set_value_free_trait;

template <typename T, typename... Vs>
struct set_value_free_trait<T, void(Vs...)> :
  conditional_t<
    is_same<T, remove_reference_t<T>>::value
      && conjunction<is_same<Vs, decay_t<Vs>>...>::value,
    conditional_t<
      is_same<T, add_const_t<T>>::value,
      no_set_value_free,
      traits::set_value_free<add_const_t<T>, void(Vs...)>
    >,
    traits::set_value_free<
      remove_reference_t<T>,
      void(decay_t<Vs>...)>
  >
{
};

#endif // defined(BOOST_ASIO_HAS_DEDUCED_SET_VALUE_FREE_TRAIT)

} // namespace detail
namespace traits {

template <typename T, typename Vs, typename>
struct set_value_free_default :
  detail::set_value_free_trait<T, Vs>
{
};

template <typename T, typename Vs, typename>
struct set_value_free :
  set_value_free_default<T, Vs>
{
};

} // namespace traits
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_TRAITS_SET_VALUE_FREE_HPP
