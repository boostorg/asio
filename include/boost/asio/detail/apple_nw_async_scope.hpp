//
// detail/apple_nw_async_scope.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_APPLE_NW_ASYNC_SCOPE_HPP
#define BOOST_ASIO_DETAIL_APPLE_NW_ASYNC_SCOPE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#include <boost/asio/error.hpp>
#include <atomic>
#include <condition_variable>
#include <mutex>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

class apple_nw_async_scope
{
public:
  apple_nw_async_scope()
    : outstanding_ops_(1)
  {
  }

  void wait()
  {
    if (--outstanding_ops_ > 0)
    {
      std::unique_lock<std::mutex> lock(mutex_);
      while (outstanding_ops_ != 0)
        condition_.wait(lock);
    }
  }

  void work_started()
  {
    ++outstanding_ops_;
  }

  void work_finished()
  {
    if (--outstanding_ops_ == 0)
    {
      std::unique_lock<std::mutex> lock(mutex_);
      condition_.notify_all();
    }
  }

private:
  apple_nw_async_scope(
      const apple_nw_async_scope& other) BOOST_ASIO_DELETED;
  apple_nw_async_scope& operator=(
      const apple_nw_async_scope& other) BOOST_ASIO_DELETED;

  std::atomic<long> outstanding_ops_;
  std::mutex mutex_;
  std::condition_variable condition_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#endif // BOOST_ASIO_DETAIL_APPLE_NW_ASYNC_SCOPE_HPP
