//
// execution/execute.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2023 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_EXECUTION_EXECUTE_HPP
#define BOOST_ASIO_EXECUTION_EXECUTE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if !defined(BOOST_ASIO_NO_DEPRECATED)

#include <boost/asio/detail/type_traits.hpp>
#include <boost/asio/execution/detail/as_invocable.hpp>
#include <boost/asio/execution/detail/as_receiver.hpp>
#include <boost/asio/traits/execute_member.hpp>
#include <boost/asio/traits/execute_free.hpp>

#include <boost/asio/detail/push_options.hpp>

#if defined(GENERATING_DOCUMENTATION)

namespace boost {
namespace asio {
namespace execution {

/// (Deprecated: Use @c execute member function.) A customisation point that
/// executes a function on an executor.
/**
 * The name <tt>execution::execute</tt> denotes a customisation point object.
 *
 * For some subexpressions <tt>e</tt> and <tt>f</tt>, let <tt>E</tt> be a type
 * such that <tt>decltype((e))</tt> is <tt>E</tt> and let <tt>F</tt> be a type
 * such that <tt>decltype((f))</tt> is <tt>F</tt>. The expression
 * <tt>execution::execute(e, f)</tt> is ill-formed if <tt>F</tt> does not model
 * <tt>invocable</tt>, or if <tt>E</tt> does not model either <tt>executor</tt>
 * or <tt>sender</tt>. Otherwise, it is expression-equivalent to:
 *
 * @li <tt>e.execute(f)</tt>, if that expression is valid. If the function
 *   selected does not execute the function object <tt>f</tt> on the executor
 *   <tt>e</tt>, the program is ill-formed with no diagnostic required.
 *
 * @li Otherwise, <tt>execute(e, f)</tt>, if that expression is valid, with
 *   overload resolution performed in a context that includes the declaration
 *   <tt>void execute();</tt> and that does not include a declaration of
 *   <tt>execution::execute</tt>. If the function selected by overload
 *   resolution does not execute the function object <tt>f</tt> on the executor
 *   <tt>e</tt>, the program is ill-formed with no diagnostic required.
 */
inline constexpr unspecified execute = unspecified;

/// (Deprecated.) A type trait that determines whether an @c execute expression
/// is well-formed.
/**
 * Class template @c can_execute is a trait that is derived from
 * @c true_type if the expression <tt>execution::execute(std::declval<T>(),
 * std::declval<F>())</tt> is well formed; otherwise @c false_type.
 */
template <typename T, typename F>
struct can_execute :
  integral_constant<bool, automatically_determined>
{
};

} // namespace execution
} // namespace asio
} // namespace boost

#else // defined(GENERATING_DOCUMENTATION)

namespace boost {
namespace asio {
namespace execution {

template <typename T, typename R>
struct is_sender_to;

namespace detail {

template <typename S, typename R>
void submit_helper(S&& s, R&& r);

} // namespace detail
} // namespace execution
} // namespace asio
} // namespace boost
namespace boost_asio_execution_execute_fn {

using boost::asio::conditional_t;
using boost::asio::decay_t;
using boost::asio::declval;
using boost::asio::enable_if_t;
using boost::asio::execution::detail::as_receiver;
using boost::asio::execution::detail::is_as_invocable;
using boost::asio::execution::is_sender_to;
using boost::asio::false_type;
using boost::asio::result_of_t;
using boost::asio::traits::execute_free;
using boost::asio::traits::execute_member;
using boost::asio::true_type;
using boost::asio::void_t;

void execute();

enum overload_type
{
  call_member,
  call_free,
  adapter,
  ill_formed
};

template <typename Impl, typename T, typename F, typename = void,
    typename = void, typename = void, typename = void, typename = void>
struct call_traits
{
  static constexpr overload_type overload = ill_formed;
};

template <typename Impl, typename T, typename F>
struct call_traits<Impl, T, void(F),
  enable_if_t<
    execute_member<typename Impl::template proxy<T>::type, F>::is_valid
  >> :
  execute_member<typename Impl::template proxy<T>::type, F>
{
  static constexpr overload_type overload = call_member;
};

template <typename Impl, typename T, typename F>
struct call_traits<Impl, T, void(F),
  enable_if_t<
    !execute_member<typename Impl::template proxy<T>, F>::is_valid
  >,
  enable_if_t<
    execute_free<T, F>::is_valid
  >> :
  execute_free<T, F>
{
  static constexpr overload_type overload = call_free;
};

template <typename Impl, typename T, typename F>
struct call_traits<Impl, T, void(F),
  enable_if_t<
    !execute_member<typename Impl::template proxy<T>::type, F>::is_valid
  >,
  enable_if_t<
    !execute_free<T, F>::is_valid
  >,
  void_t<
   result_of_t<decay_t<F>&()>
  >,
  enable_if_t<
    !is_as_invocable<decay_t<F>>::value
  >,
  enable_if_t<
    is_sender_to<T, as_receiver<decay_t<F>, T>>::value
  >>
{
  static constexpr overload_type overload = adapter;
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = false;
  typedef void result_type;
};

struct impl
{
  template <typename T>
  struct proxy
  {
#if defined(BOOST_ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)
    struct type
    {
      template <typename F>
      auto execute(F&& f)
        noexcept(
          noexcept(
            declval<conditional_t<true, T, F>>().execute(static_cast<F&&>(f))
          )
        )
        -> decltype(
          declval<conditional_t<true, T, F>>().execute(static_cast<F&&>(f))
        );
    };
#else // defined(BOOST_ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)
    typedef T type;
#endif // defined(BOOST_ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)
  };

  template <typename T, typename F>
  constexpr enable_if_t<
    call_traits<impl, T, void(F)>::overload == call_member,
    typename call_traits<impl, T, void(F)>::result_type
  >
  operator()(T&& t, F&& f) const
    noexcept(call_traits<impl, T, void(F)>::is_noexcept)
  {
    return static_cast<T&&>(t).execute(static_cast<F&&>(f));
  }

  template <typename T, typename F>
  constexpr enable_if_t<
    call_traits<impl, T, void(F)>::overload == call_free,
    typename call_traits<impl, T, void(F)>::result_type
  >
  operator()(T&& t, F&& f) const
    noexcept(call_traits<impl, T, void(F)>::is_noexcept)
  {
    return execute(static_cast<T&&>(t), static_cast<F&&>(f));
  }

  template <typename T, typename F>
  constexpr enable_if_t<
    call_traits<impl, T, void(F)>::overload == adapter,
    typename call_traits<impl, T, void(F)>::result_type
  >
  operator()(T&& t, F&& f) const
    noexcept(call_traits<impl, T, void(F)>::is_noexcept)
  {
    return boost::asio::execution::detail::submit_helper(static_cast<T&&>(t),
        as_receiver<decay_t<F>, T>(static_cast<F&&>(f), 0));
  }
};

template <typename T = impl>
struct static_instance
{
  static const T instance;
};

template <typename T>
const T static_instance<T>::instance = {};

} // namespace boost_asio_execution_execute_fn
namespace boost {
namespace asio {
namespace execution {
namespace {

static constexpr const boost_asio_execution_execute_fn::impl&
  execute = boost_asio_execution_execute_fn::static_instance<>::instance;

} // namespace

typedef boost_asio_execution_execute_fn::impl execute_t;

template <typename T, typename F>
struct can_execute :
  integral_constant<bool,
    boost_asio_execution_execute_fn::call_traits<
      execute_t, T, void(F)>::overload !=
        boost_asio_execution_execute_fn::ill_formed>
{
};

#if defined(BOOST_ASIO_HAS_VARIABLE_TEMPLATES)

template <typename T, typename F>
constexpr bool can_execute_v = can_execute<T, F>::value;

#endif // defined(BOOST_ASIO_HAS_VARIABLE_TEMPLATES)

} // namespace execution
} // namespace asio
} // namespace boost

#endif // defined(GENERATING_DOCUMENTATION)

#include <boost/asio/detail/pop_options.hpp>

#endif // !defined(BOOST_ASIO_NO_DEPRECATED)

#endif // BOOST_ASIO_EXECUTION_EXECUTE_HPP
