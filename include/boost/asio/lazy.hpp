//
// lazy.hpp
// ~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_LAZY_HPP
#define BOOST_ASIO_LAZY_HPP

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

struct lazy_t {};

template <BOOST_ASIO_COMPLETION_SIGNATURE Signature>
class async_result<lazy_t, Signature>
{
public:
  template <typename Initiation, typename... InitArgs>
  static auto initiate(Initiation initiation, lazy_t, InitArgs... init_args)
  {
    return [
        initiation = std::move(initiation),
        init_arg_pack = std::make_tuple(std::move(init_args)...)
      ](auto&& token) mutable
    {
      return std::apply(
          [&](auto&&... args)
          {
            return async_initiate<decltype(token), Signature>(
                std::move(initiation), token,
                std::forward<decltype(args)>(args)...);
          },
          std::move(init_arg_pack)
        );
    };
  }
};

/// A completion token object that indicates that an Awaitable should be returned.
#if defined(BOOST_ASIO_HAS_CONSTEXPR) || defined(GENERATING_DOCUMENTATION)
constexpr lazy_t lazy;
#elif defined(BOOST_ASIO_MSVC)
__declspec(selectany) lazy_t lazy;
#endif

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_LAZY_HPP
