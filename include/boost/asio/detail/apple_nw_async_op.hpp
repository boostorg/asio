//
// detail/apple_nw_async_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_APPLE_NW_ASYNC_OP_HPP
#define BOOST_ASIO_DETAIL_APPLE_NW_ASYNC_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#include <boost/asio/error.hpp>
#include <boost/asio/detail/operation.hpp>
#include <Network/Network.h>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename T>
class apple_nw_async_op
  : public operation
{
public:
  void set(const boost::system::error_code& ec, T result)
  {
    ec_ = ec;
    result_ = BOOST_ASIO_MOVE_CAST(T)(result);
  }

  void set(nw_error_t error, T result)
  {
    set(boost::system::error_code(error ? nw_error_get_error_code(error) : 0,
          boost::system::system_category()), BOOST_ASIO_MOVE_CAST(T)(result));
  }

protected:
  apple_nw_async_op(func_type complete_func)
    : operation(complete_func),
      result_()
  {
  }

  // The error code to be passed to the completion handler.
  boost::system::error_code ec_;

  // The result of the operation, to be passed to the completion handler.
  T result_;
};

template <>
class apple_nw_async_op<void>
  : public operation
{
public:
  void set(const boost::system::error_code& ec)
  {
    ec_ = ec;
  }

  void set(nw_error_t error)
  {
    set(boost::system::error_code(
          error ? nw_error_get_error_code(error) : 0,
          boost::system::system_category()));
  }

protected:
  apple_nw_async_op(func_type complete_func)
    : operation(complete_func)
  {
  }

  // The error code to be passed to the completion handler.
  boost::system::error_code ec_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_APPLE_NETWORK_FRAMEWORK)

#endif // BOOST_ASIO_DETAIL_APPLE_NW_ASYNC_OP_HPP
