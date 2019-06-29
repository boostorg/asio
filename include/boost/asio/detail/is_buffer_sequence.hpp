//
// detail/is_buffer_sequence.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2019 Alexander Karzhenkov
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_IS_BUFFER_SEQUENCE_HPP
#define BOOST_ASIO_DETAIL_IS_BUFFER_SEQUENCE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/detail/type_traits.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {

class mutable_buffer;
class const_buffer;

namespace detail {

template <typename C, typename T>
struct buffer_sequence_check
  : integral_constant<bool,
      sizeof(C::template detector<T>(0)) == 1>
{
};

struct buffer_sequence_helper
{
  template <int N> struct result { };

  template <typename>
  static char (&detector(...))[2];

  template <typename T>
  static typename enable_if<
    is_member_function_pointer<T>::value>::type
  is_memfn(T);

  template <typename To, typename From>
  static typename enable_if<
    is_convertible<From, To>::value>::type
  is_convertible_to(From);

  template <typename T>
  static void check();
};

template <typename Buffer>
struct is_buffer_sequence_class : buffer_sequence_helper
{
  using buffer_sequence_helper::detector;

  template <typename T>
  static result<sizeof(

    // Basic checks

    is_convertible_to<Buffer>(*buffer_sequence_begin(declval<T&>())),
    is_convertible_to<Buffer>(*buffer_sequence_end(declval<T&>())),

#if 1

    // Check additional details of the inspected type

    // Perhaps the additional checks make sense when using C++98.
    // It would be better to implement them by the means
    // of 'buffer_sequence_begin' and 'buffer_sequence_end'
    // (or simply to remove).

    is_convertible_to<Buffer>(*declval<T>().begin()),
    is_convertible_to<Buffer>(*declval<T>().end()),

    is_convertible_to<Buffer>(declval<typename T::value_type>()),

    check<typename T::const_iterator>(),

#endif

  0)> detector(int);
};

template <typename T, typename Buffer>
struct is_buffer_sequence
  : buffer_sequence_check<is_buffer_sequence_class<Buffer>, T>
{
};

template <>
struct is_buffer_sequence<mutable_buffer, mutable_buffer>
  : true_type
{
};

template <>
struct is_buffer_sequence<mutable_buffer, const_buffer>
  : true_type
{
};

template <>
struct is_buffer_sequence<const_buffer, const_buffer>
  : true_type
{
};

template <>
struct is_buffer_sequence<const_buffer, mutable_buffer>
  : false_type
{
};

struct is_dynamic_buffer_class_v1 : buffer_sequence_helper
{
  using buffer_sequence_helper::detector;

  template <typename T, typename Buffer>
  static result<sizeof(

    is_memfn(&T::size),
    is_memfn(&T::max_size),
    is_memfn(&T::capacity),
    is_memfn(&T::data),
    is_memfn(&T::consume),
    is_memfn(&T::prepare),
    is_memfn(&T::commit),

    check<typename T::const_buffers_type>(),
    check<typename T::mutable_buffers_type>(),

  0)> detector(int);
};

template <typename T>
struct is_dynamic_buffer_v1
  : buffer_sequence_check<is_dynamic_buffer_class_v1, T>
{
};

struct is_dynamic_buffer_class_v2 : buffer_sequence_helper
{
  using buffer_sequence_helper::detector;

  template <typename T, typename Buffer>
  static result<sizeof(

    is_memfn(&T::size),
    is_memfn(&T::max_size),
    is_memfn(&T::capacity),
    is_memfn(&T::data),
    is_memfn(&T::consume),
    is_memfn(&T::grow),
    is_memfn(&T::shrink),

    check<typename T::const_buffers_type>(),
    check<typename T::mutable_buffers_type>(),

  0)> detector(int);
};

template <typename T>
struct is_dynamic_buffer_v2
  : buffer_sequence_check<is_dynamic_buffer_class_v2, T>
{
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_IS_BUFFER_SEQUENCE_HPP
