//
// execution/detail/as_operation.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2023 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_EXECUTION_DETAIL_AS_OPERATION_HPP
#define BOOST_ASIO_EXECUTION_DETAIL_AS_OPERATION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/detail/memory.hpp>
#include <boost/asio/detail/type_traits.hpp>
#include <boost/asio/execution/detail/as_invocable.hpp>
#include <boost/asio/execution/execute.hpp>
#include <boost/asio/execution/set_error.hpp>
#include <boost/asio/traits/start_member.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace execution {
namespace detail {

template <typename Executor, typename Receiver>
struct as_operation
{
  remove_cvref_t<Executor> ex_;
  remove_cvref_t<Receiver> receiver_;

  template <typename E, typename R>
  explicit as_operation(E&& e, R&& r)
    : ex_(static_cast<E&&>(e)),
      receiver_(static_cast<R&&>(r))
  {
  }

  void start() noexcept
  {
#if !defined(BOOST_ASIO_NO_EXCEPTIONS)
    try
    {
#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)
#if defined(BOOST_ASIO_NO_DEPRECATED)
      ex_.execute(
#else // defined(BOOST_ASIO_NO_DEPRECATED)
      execution::execute(
          static_cast<remove_cvref_t<Executor>&&>(ex_),
#endif // defined(BOOST_ASIO_NO_DEPRECATED)
          as_invocable<remove_cvref_t<Receiver>, Executor>(receiver_));
#if !defined(BOOST_ASIO_NO_EXCEPTIONS)
    }
    catch (...)
    {
      execution::set_error(
          static_cast<remove_cvref_t<Receiver>&&>(receiver_),
          std::current_exception());
    }
#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)
  }
};

} // namespace detail
} // namespace execution
namespace traits {

#if !defined(BOOST_ASIO_HAS_DEDUCED_START_MEMBER_TRAIT)

template <typename Executor, typename Receiver>
struct start_member<
    boost::asio::execution::detail::as_operation<Executor, Receiver>>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
  typedef void result_type;
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_START_MEMBER_TRAIT)

} // namespace traits
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_EXECUTION_DETAIL_AS_OPERATION_HPP
