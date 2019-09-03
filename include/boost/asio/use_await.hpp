//
// use_await.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_USE_AWAIT_HPP
#define BOOST_ASIO_USE_AWAIT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_CO_AWAIT) || defined(GENERATING_DOCUMENTATION)

#include <functional>
#include <tuple>
#include <utility>
#include <boost/asio/async_result.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {

struct use_await_t {};

template <typename R, typename... Args>
class async_result<use_await_t, R(Args...)>
{
public:
  template <typename Initiation, typename... InitArgs>
  static auto initiate(Initiation initiation,
      use_await_t, InitArgs... init_args)
  {
    struct awaitable
    {
      Initiation initiation_;
      std::tuple<InitArgs...> init_args_;
      void* result_ = nullptr;

      bool await_ready() const noexcept
      {
        return false;
      }

      void await_suspend(std::experimental::coroutine_handle<> h) noexcept
      {
        std::apply(
            [&](auto&&... a)
            {
              initiation_(
                  [this, h](Args... results) mutable
                  {
                    std::tuple<Args...> result(std::move(results)...);
                    result_ = &result;
                    h.resume();
                  },
                  std::forward<decltype(a)>(a)...
                );
            },
            init_args_
          );
      }

      std::tuple<Args...> await_resume()
      {
        return std::move(*static_cast<std::tuple<Args...>*>(result_));
      }
    };

    return awaitable{std::move(initiation),
        std::forward_as_tuple(std::move(init_args)...)};
  }
};

/// A completion token object that indicates that an Awaitable should be returned.
#if defined(BOOST_ASIO_HAS_CONSTEXPR) || defined(GENERATING_DOCUMENTATION)
constexpr use_await_t use_await;
#elif defined(BOOST_ASIO_MSVC)
__declspec(selectany) use_await_t use_await;
#endif

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_CO_AWAIT) || defined(GENERATING_DOCUMENTATION)

#endif // BOOST_ASIO_USE_AWAIT_HPP
