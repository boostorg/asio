//
// experimental/impl/deferred.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_EXPERIMENTAL_IMPL_DEFERRED_HPP
#define BOOST_ASIO_EXPERIMENTAL_IMPL_DEFERRED_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <tuple>
#include <utility>
#include <boost/asio/async_result.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace experimental {
namespace detail {

template <typename Signature, typename Initiation, typename... InitArgs>
class deferred_init
{
private:
  typename decay<Initiation>::type initiation_;
  typedef std::tuple<typename decay<InitArgs>::type...> init_args_t;
  init_args_t init_args_;

  template <typename CompletionToken, std::size_t... I>
  decltype(auto) invoke(BOOST_ASIO_MOVE_ARG(CompletionToken) token,
      std::index_sequence<I...>)
  {
    return boost::asio::async_initiate<CompletionToken, Signature>(
        BOOST_ASIO_MOVE_CAST(typename decay<Initiation>::type)(initiation_),
        token, std::get<I>(BOOST_ASIO_MOVE_CAST(init_args_t)(init_args_))...);
  }

public:
  template <typename I, typename... A>
  explicit deferred_init(BOOST_ASIO_MOVE_ARG(I) initiation,
      BOOST_ASIO_MOVE_ARG(A)... init_args)
    : initiation_(BOOST_ASIO_MOVE_CAST(I)(initiation)),
      init_args_(BOOST_ASIO_MOVE_CAST(A)(init_args)...)
  {
  }

  template <BOOST_ASIO_COMPLETION_TOKEN_FOR(Signature) CompletionToken>
  decltype(auto) operator()(BOOST_ASIO_MOVE_ARG(CompletionToken) token)
  {
    return this->invoke(
        BOOST_ASIO_MOVE_CAST(CompletionToken)(token),
        std::index_sequence_for<InitArgs...>());
  }
};

struct deferred_signature_probe {};

template <typename T>
struct deferred_signature_probe_result
{
  typedef T type;
};

template <typename T>
struct deferred_signature
{
  typedef typename decltype(
      declval<T>()(declval<deferred_signature_probe>()))::type type;
};

template <typename HeadSignature, typename Tail>
struct deferred_link_signature;

template <typename R, typename... Args, typename Tail>
struct deferred_link_signature<R(Args...), Tail>
{
  typedef typename decltype(
      declval<Tail>()(declval<Args>()...)(
        declval<deferred_signature_probe>()))::type type;
};

template <typename Handler, typename Tail>
class deferred_link_handler
{
public:
  template <typename H, typename T>
  explicit deferred_link_handler(
      BOOST_ASIO_MOVE_ARG(H) handler, BOOST_ASIO_MOVE_ARG(T) tail)
    : handler_(BOOST_ASIO_MOVE_CAST(H)(handler)),
      tail_(BOOST_ASIO_MOVE_CAST(T)(tail))
  {
  }

  template <typename... Args>
  void operator()(BOOST_ASIO_MOVE_ARG(Args)... args)
  {
    BOOST_ASIO_MOVE_OR_LVALUE(Tail)(tail_)(
        BOOST_ASIO_MOVE_CAST(Args)(args)...)(
          BOOST_ASIO_MOVE_OR_LVALUE(Handler)(handler_));
  }

//private:
  Handler handler_;
  Tail tail_;
};

struct deferred_link_initiate
{
  template <typename Handler, typename Head, typename Tail>
  void operator()(BOOST_ASIO_MOVE_ARG(Handler) handler,
      Head head, BOOST_ASIO_MOVE_ARG(Tail) tail)
  {
    BOOST_ASIO_MOVE_OR_LVALUE(Head)(head)(
        deferred_link_handler<typename decay<Handler>::type,
          typename decay<Tail>::type>(
            BOOST_ASIO_MOVE_CAST(Handler)(handler),
            BOOST_ASIO_MOVE_CAST(Tail)(tail)));
  }
};

template <typename Head, typename Tail>
class deferred_link
{
public:
  typedef typename deferred_link_signature<
    typename deferred_signature<Head>::type, Tail>::type
      signature;

  template <typename H, typename T>
  explicit deferred_link(BOOST_ASIO_MOVE_ARG(H) head, BOOST_ASIO_MOVE_ARG(T) tail)
    : head_(BOOST_ASIO_MOVE_CAST(H)(head)),
      tail_(BOOST_ASIO_MOVE_CAST(T)(tail))
  {
  }

  template <BOOST_ASIO_COMPLETION_TOKEN_FOR(signature) CompletionToken>
  decltype(auto) operator()(BOOST_ASIO_MOVE_ARG(CompletionToken) token)
  {
    return boost::asio::async_initiate<CompletionToken, signature>(
        deferred_link_initiate(), token,
        BOOST_ASIO_MOVE_OR_LVALUE(Head)(head_),
        BOOST_ASIO_MOVE_OR_LVALUE(Tail)(tail_));
  }

private:
  Head head_;
  Tail tail_;
};

} // namespace detail
} // namespace experimental

#if !defined(GENERATING_DOCUMENTATION)

template <typename Signature>
class async_result<experimental::deferred_t, Signature>
{
public:
  template <typename Initiation, typename... InitArgs>
  static experimental::deferred_operation<
    experimental::detail::deferred_init<Signature, Initiation, InitArgs...> >
  initiate(BOOST_ASIO_MOVE_ARG(Initiation) initiation,
      experimental::deferred_t, BOOST_ASIO_MOVE_ARG(InitArgs)... args)
  {
    return experimental::deferred_operation<
      experimental::detail::deferred_init<Signature, Initiation, InitArgs...> >(
        experimental::detail::deferred_init<Signature, Initiation, InitArgs...>(
          BOOST_ASIO_MOVE_CAST(Initiation)(initiation),
          BOOST_ASIO_MOVE_CAST(InitArgs)(args)...));
    }
};

template <typename R, typename... Args>
class async_result<experimental::detail::deferred_signature_probe, R(Args...)>
{
public:
  typedef experimental::detail::deferred_signature_probe_result<void(Args...)>
    return_type;

  template <typename Initiation, typename... InitArgs>
  static return_type initiate(
      BOOST_ASIO_MOVE_ARG(Initiation),
      experimental::detail::deferred_signature_probe,
      BOOST_ASIO_MOVE_ARG(InitArgs)...)
  {
    return return_type{};
  }
};

template <typename Tail, typename R, typename... Args>
  requires (experimental::is_deferred_operation<std::invoke_result_t<Tail, Args...>>::value)
class boost::asio::async_result<Tail, R(Args...)>
{
public:
  template <typename Initiation, typename... InitArgs>
  static auto initiate(BOOST_ASIO_MOVE_ARG(Initiation) initiation,
      Tail tail, BOOST_ASIO_MOVE_ARG(InitArgs)... init_args)
  {
    return experimental::deferred_operation<
      experimental::detail::deferred_link<
        experimental::detail::deferred_init<R(Args...), Initiation, InitArgs...>,
        Tail> >(
          experimental::detail::deferred_link<
            experimental::detail::deferred_init<R(Args...), Initiation, InitArgs...>,
            Tail>(
              experimental::detail::deferred_init<R(Args...), Initiation, InitArgs...>(
                BOOST_ASIO_MOVE_CAST(Initiation)(initiation),
                BOOST_ASIO_MOVE_CAST(InitArgs)(init_args)...),
              BOOST_ASIO_MOVE_CAST(Tail)(tail)));
  }
};

template <template <typename, typename> class Associator,
    typename Handler, typename Tail, typename DefaultCandidate>
struct associator<Associator,
    experimental::detail::deferred_link_handler<Handler, Tail>, DefaultCandidate>
  : Associator<Handler, DefaultCandidate>
{
  static typename Associator<Handler, DefaultCandidate>::type get(
      const experimental::detail::deferred_link_handler<Handler, Tail>& h,
      const DefaultCandidate& c = DefaultCandidate()) BOOST_ASIO_NOEXCEPT
  {
    return Associator<Handler, DefaultCandidate>::get(h.handler_, c);
  }
};

#endif // !defined(GENERATING_DOCUMENTATION)

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_EXPERIMENTAL_IMPL_DEFERRED_HPP
