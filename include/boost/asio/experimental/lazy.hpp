//
// experimental/lazy.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_EXPERIMENTAL_LAZY_HPP
#define BOOST_ASIO_EXPERIMENTAL_LAZY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace experimental {

/// Class used to specify that an asynchronous operation should return a
/// function object to lazily launch the operation.
/**
 * The lazy_t class is used to indicate that an asynchronous operation should
 * return a function object which is itself an initiation function. A lazy_t
 * object may be passed as a completion token to an asynchronous operation,
 * typically using the special value @c boost::asio::lazy. For example:
 *
 * @code auto my_sender
 *   = my_socket.async_read_some(my_buffer,
 *       boost::asio::experimental::lazy); @endcode
 *
 * The initiating function (async_read_some in the above example) returns a
 * function object that will lazily initiate the operation.
 */
class lazy_t
{
public:
  /// Default constructor.
  BOOST_ASIO_CONSTEXPR lazy_t()
  {
  }

  /// Adapts an executor to add the @c lazy_t completion token as the
  /// default.
  template <typename InnerExecutor>
  struct executor_with_default : InnerExecutor
  {
    /// Specify @c lazy_t as the default completion token type.
    typedef lazy_t default_completion_token_type;

    /// Construct the adapted executor from the inner executor type.
    template <typename InnerExecutor1>
    executor_with_default(const InnerExecutor1& ex,
        typename constraint<
          conditional<
            !is_same<InnerExecutor1, executor_with_default>::value,
            is_convertible<InnerExecutor1, InnerExecutor>,
            false_type
          >::type::value
        >::type = 0) BOOST_ASIO_NOEXCEPT
      : InnerExecutor(ex)
    {
    }
  };

  /// Type alias to adapt an I/O object to use @c lazy_t as its
  /// default completion token type.
#if defined(BOOST_ASIO_HAS_ALIAS_TEMPLATES) \
  || defined(GENERATING_DOCUMENTATION)
  template <typename T>
  using as_default_on_t = typename T::template rebind_executor<
      executor_with_default<typename T::executor_type> >::other;
#endif // defined(BOOST_ASIO_HAS_ALIAS_TEMPLATES)
       //   || defined(GENERATING_DOCUMENTATION)

  /// Function helper to adapt an I/O object to use @c lazy_t as its
  /// default completion token type.
  template <typename T>
  static typename decay<T>::type::template rebind_executor<
      executor_with_default<typename decay<T>::type::executor_type>
    >::other
  as_default_on(BOOST_ASIO_MOVE_ARG(T) object)
  {
    return typename decay<T>::type::template rebind_executor<
        executor_with_default<typename decay<T>::type::executor_type>
      >::other(BOOST_ASIO_MOVE_CAST(T)(object));
  }
};

/// A special value, similar to std::nothrow.
/**
 * See the documentation for boost::asio::experimental::lazy_t for a usage example.
 */
#if defined(BOOST_ASIO_HAS_CONSTEXPR) || defined(GENERATING_DOCUMENTATION)
constexpr lazy_t lazy;
#elif defined(BOOST_ASIO_MSVC)
__declspec(selectany) lazy_t lazy;
#endif

} // namespace experimental
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/experimental/impl/lazy.hpp>

#endif // BOOST_ASIO_EXPERIMENTAL_LAZY_HPP
