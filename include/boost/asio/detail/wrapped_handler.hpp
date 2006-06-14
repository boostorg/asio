//
// wrapped_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_WRAPPED_HANDLER_HPP
#define BOOST_ASIO_DETAIL_WRAPPED_HANDLER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename Dispatcher, typename Handler>
class wrapped_handler
{
public:
  typedef void result_type;

  wrapped_handler(Dispatcher& dispatcher, Handler handler)
    : dispatcher_(dispatcher),
      handler_(handler)
  {
  }

  void operator()()
  {
    dispatcher_.dispatch(handler_);
  }

  void operator()() const
  {
    dispatcher_.dispatch(handler_);
  }

  template <typename Arg1>
  void operator()(Arg1 arg1)
  {
    dispatcher_.dispatch(detail::bind_handler(handler_, arg1));
  }

  template <typename Arg1>
  void operator()(Arg1 arg1) const
  {
    dispatcher_.dispatch(detail::bind_handler(handler_, arg1));
  }

  template <typename Arg1, typename Arg2>
  void operator()(Arg1 arg1, Arg2 arg2)
  {
    dispatcher_.dispatch(detail::bind_handler(handler_, arg1, arg2));
  }

  template <typename Arg1, typename Arg2>
  void operator()(Arg1 arg1, Arg2 arg2) const
  {
    dispatcher_.dispatch(detail::bind_handler(handler_, arg1, arg2));
  }

  template <typename Arg1, typename Arg2, typename Arg3>
  void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3)
  {
    dispatcher_.dispatch(detail::bind_handler(handler_, arg1, arg2, arg3));
  }

  template <typename Arg1, typename Arg2, typename Arg3>
  void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3) const
  {
    dispatcher_.dispatch(detail::bind_handler(handler_, arg1, arg2, arg3));
  }

  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
  void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
  {
    dispatcher_.dispatch(
        detail::bind_handler(handler_, arg1, arg2, arg3, arg4));
  }

  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
  void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) const
  {
    dispatcher_.dispatch(
        detail::bind_handler(handler_, arg1, arg2, arg3, arg4));
  }

  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
      typename Arg5>
  void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
  {
    dispatcher_.dispatch(
        detail::bind_handler(handler_, arg1, arg2, arg3, arg4, arg5));
  }

  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4,
      typename Arg5>
  void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5) const
  {
    dispatcher_.dispatch(
        detail::bind_handler(handler_, arg1, arg2, arg3, arg4, arg5));
  }

  friend void* asio_handler_allocate(std::size_t size,
      wrapped_handler<Dispatcher, Handler>* this_handler)
  {
    return boost_asio_handler_alloc_helpers::allocate(
        size, &this_handler->handler_);
  }

  friend void asio_handler_deallocate(void* pointer, std::size_t size,
      wrapped_handler<Dispatcher, Handler>* this_handler)
  {
    boost_asio_handler_alloc_helpers::deallocate(
        pointer, size, &this_handler->handler_);
  }

private:
  Dispatcher& dispatcher_;
  Handler handler_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_WRAPPED_HANDLER_HPP
