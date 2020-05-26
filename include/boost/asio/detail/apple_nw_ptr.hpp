//
// detail/apple_nw_ptr.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_APPLE_NW_PTR_HPP
#define BOOST_ASIO_DETAIL_APPLE_NW_PTR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#include <cstring>
#include <cstddef>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename T>
class apple_nw_ptr
{
public:
  apple_nw_ptr() BOOST_ASIO_NOEXCEPT
    : ptr_(0)
  {
  }

  explicit apple_nw_ptr(T ptr) BOOST_ASIO_NOEXCEPT
    : ptr_(ptr)
  {
  }

  apple_nw_ptr(const apple_nw_ptr& other) BOOST_ASIO_NOEXCEPT
    : ptr_(other.ptr_)
  {
    nw_retain(ptr_);
  }

#if defined(BOOST_ASIO_HAS_MOVE)
  apple_nw_ptr(apple_nw_ptr&& other) BOOST_ASIO_NOEXCEPT
    : ptr_(other.ptr_)
  {
    other.ptr_ = 0;
  }
#endif // defined(BOOST_ASIO_HAS_MOVE)

  ~apple_nw_ptr()
  {
    nw_release(ptr_);
  }

  apple_nw_ptr& operator=(const apple_nw_ptr& other) BOOST_ASIO_NOEXCEPT
  {
    reset();
    ptr_ = other.ptr_;
    nw_retain(ptr_);
    return *this;
  }

#if defined(BOOST_ASIO_HAS_MOVE)
  apple_nw_ptr& operator=(apple_nw_ptr&& other) BOOST_ASIO_NOEXCEPT
  {
    reset();
    ptr_ = other.ptr_;
    other.ptr_ = 0;
    return *this;
  }
#endif // defined(BOOST_ASIO_HAS_MOVE)

  T get() BOOST_ASIO_NOEXCEPT
  {
    return ptr_;
  }

  operator T() const BOOST_ASIO_NOEXCEPT
  {
    return ptr_;
  }

  bool operator!() const BOOST_ASIO_NOEXCEPT
  {
    return ptr_ == 0;
  }

  void reset() BOOST_ASIO_NOEXCEPT
  {
    nw_release(ptr_);
    ptr_ = 0;
  }

  void reset(T ptr) BOOST_ASIO_NOEXCEPT
  {
    nw_release(ptr_);
    ptr_ = ptr;
  }

  T release() BOOST_ASIO_NOEXCEPT
  {
    T tmp = ptr_;
    ptr_ = 0;
    return tmp;
  }

  void swap(apple_nw_ptr& other) BOOST_ASIO_NOEXCEPT
  {
    T tmp = ptr_;
    ptr_ = other.ptr_;
    other.ptr_ = tmp;
  }

  friend bool operator==(const apple_nw_ptr& a,
      const apple_nw_ptr& b) BOOST_ASIO_NOEXCEPT
  {
    return a.ptr_ == b.ptr_;
  }

  friend bool operator!=(const apple_nw_ptr& a,
      const apple_nw_ptr& b) BOOST_ASIO_NOEXCEPT
  {
    return a.ptr_ != b.ptr_;
  }

private:
  T ptr_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#endif // BOOST_ASIO_DETAIL_APPLE_NW_PTR_HPP
