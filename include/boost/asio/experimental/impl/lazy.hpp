//
// experimental/impl/lazy.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_EXPERIMENTAL_IMPL_LAZY_HPP
#define BOOST_ASIO_EXPERIMENTAL_IMPL_LAZY_HPP

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
class lazy_init
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
  explicit lazy_init(BOOST_ASIO_MOVE_ARG(I) initiation,
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

} // namespace detail
} // namespace experimental

#if !defined(GENERATING_DOCUMENTATION)

template <typename Signature>
class async_result<experimental::lazy_t, Signature>
{
public:
  template <typename Initiation, typename... InitArgs>
  static experimental::detail::lazy_init<Signature, Initiation, InitArgs...>
  initiate(BOOST_ASIO_MOVE_ARG(Initiation) initiation,
      experimental::lazy_t, BOOST_ASIO_MOVE_ARG(InitArgs)... args)
  {
    return experimental::detail::lazy_init<Signature, Initiation, InitArgs...>(
        BOOST_ASIO_MOVE_CAST(Initiation)(initiation),
        BOOST_ASIO_MOVE_CAST(InitArgs)(args)...);
  }
};

#endif // !defined(GENERATING_DOCUMENTATION)

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_EXPERIMENTAL_IMPL_LAZY_HPP
