//
// detail/apple_nw_sync_result.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_APPLE_NW_SYNC_RESULT_HPP
#define BOOST_ASIO_DETAIL_APPLE_NW_SYNC_RESULT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#include <boost/asio/error.hpp>
#include <condition_variable>
#include <mutex>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename T>
class apple_nw_sync_result
{
public:
  apple_nw_sync_result()
    : ec_(boost::asio::error::would_block),
      result_()
  {
  }

  void set(const boost::system::error_code& ec, T result)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    ec_ = ec;
    result_ = BOOST_ASIO_MOVE_CAST(T)(result);
    condition_.notify_all();
  }

  void set(nw_error_t error, T result)
  {
    set(boost::system::error_code(error ? nw_error_get_error_code(error) : 0,
          boost::system::system_category()), BOOST_ASIO_MOVE_CAST(T)(result));
  }

  T get(boost::system::error_code& ec)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (ec_ == boost::asio::error::would_block)
      condition_.wait(lock);
    ec = ec_;
    return BOOST_ASIO_MOVE_CAST(T)(result_);
  }

private:
  apple_nw_sync_result(
      const apple_nw_sync_result& other) BOOST_ASIO_DELETED;
  apple_nw_sync_result& operator=(
      const apple_nw_sync_result& other) BOOST_ASIO_DELETED;

  std::mutex mutex_;
  std::condition_variable condition_;
  boost::system::error_code ec_;
  T result_;
};

template <>
class apple_nw_sync_result<void>
{
public:
  apple_nw_sync_result()
    : ec_(boost::asio::error::would_block)
  {
  }

  void set(const boost::system::error_code& ec)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    ec_ = ec;
    condition_.notify_all();
  }

  void set(nw_error_t error)
  {
    set(boost::system::error_code(
          error ? nw_error_get_error_code(error) : 0,
          boost::system::system_category()));
  }

  boost::system::error_code get(boost::system::error_code& ec)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (ec_ == boost::asio::error::would_block)
      condition_.wait(lock);
    ec = ec_;
    return ec;
  }

private:
  apple_nw_sync_result(
      const apple_nw_sync_result& other) BOOST_ASIO_DELETED;
  apple_nw_sync_result& operator=(
      const apple_nw_sync_result& other) BOOST_ASIO_DELETED;

  std::mutex mutex_;
  std::condition_variable condition_;
  boost::system::error_code ec_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#endif // BOOST_ASIO_DETAIL_APPLE_NW_SYNC_RESULT_HPP
