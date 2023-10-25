//
// execution/detail/submit_receiver.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2023 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_EXECUTION_DETAIL_SUBMIT_RECEIVER_HPP
#define BOOST_ASIO_EXECUTION_DETAIL_SUBMIT_RECEIVER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/detail/type_traits.hpp>
#include <boost/asio/execution/connect.hpp>
#include <boost/asio/execution/receiver.hpp>
#include <boost/asio/execution/set_done.hpp>
#include <boost/asio/execution/set_error.hpp>
#include <boost/asio/execution/set_value.hpp>
#include <boost/asio/traits/set_done_member.hpp>
#include <boost/asio/traits/set_error_member.hpp>
#include <boost/asio/traits/set_value_member.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace execution {
namespace detail {

template <typename Sender, typename Receiver>
struct submit_receiver;

template <typename Sender, typename Receiver>
struct submit_receiver_wrapper
{
  submit_receiver<Sender, Receiver>* p_;

  explicit submit_receiver_wrapper(submit_receiver<Sender, Receiver>* p)
    : p_(p)
  {
  }

  template <typename... Args>
  enable_if_t<is_receiver_of<Receiver, Args...>::value>
  set_value(Args&&... args) &&
    noexcept(is_nothrow_receiver_of<Receiver, Args...>::value)
  {
    execution::set_value(
        static_cast<remove_cvref_t<Receiver>&&>(p_->r_),
        static_cast<Args&&>(args)...);
    delete p_;
  }

  template <typename E>
  void set_error(E&& e) && noexcept
  {
    execution::set_error(
        static_cast<remove_cvref_t<Receiver>&&>(p_->r_),
        static_cast<E&&>(e));
    delete p_;
  }

  void set_done() && noexcept
  {
    execution::set_done(static_cast<remove_cvref_t<Receiver>&&>(p_->r_));
    delete p_;
  }
};

template <typename Sender, typename Receiver>
struct submit_receiver
{
  remove_cvref_t<Receiver> r_;
  connect_result_t<Sender, submit_receiver_wrapper<Sender, Receiver>> state_;

  template <typename S, typename R>
  explicit submit_receiver(S&& s, R&& r)
    : r_(static_cast<R&&>(r)),
      state_(execution::connect(static_cast<S&&>(s),
            submit_receiver_wrapper<Sender, Receiver>(this)))
  {
  }
};

} // namespace detail
} // namespace execution
namespace traits {

#if !defined(BOOST_ASIO_HAS_DEDUCED_SET_VALUE_MEMBER_TRAIT)

template <typename Sender, typename Receiver, typename... Args>
struct set_value_member<
    boost::asio::execution::detail::submit_receiver_wrapper<
      Sender, Receiver>,
    void(Args...)>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept =
    boost::asio::execution::is_nothrow_receiver_of<Receiver, Args...>::value;
  typedef void result_type;
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_SET_VALUE_MEMBER_TRAIT)

#if !defined(BOOST_ASIO_HAS_DEDUCED_SET_ERROR_MEMBER_TRAIT)

template <typename Sender, typename Receiver, typename E>
struct set_error_member<
    boost::asio::execution::detail::submit_receiver_wrapper<
      Sender, Receiver>, E>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
  typedef void result_type;
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_SET_ERROR_MEMBER_TRAIT)

#if !defined(BOOST_ASIO_HAS_DEDUCED_SET_DONE_MEMBER_TRAIT)

template <typename Sender, typename Receiver>
struct set_done_member<
    boost::asio::execution::detail::submit_receiver_wrapper<
      Sender, Receiver>>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
  typedef void result_type;
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_SET_DONE_MEMBER_TRAIT)

} // namespace traits
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_EXECUTION_DETAIL_SUBMIT_RECEIVER_HPP
