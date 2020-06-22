//
// detail/handler_work.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_HANDLER_WORK_HPP
#define BOOST_ASIO_DETAIL_HANDLER_WORK_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

// A helper class template to allow completion handlers to be dispatched
// through either the new executors framework or the old invocaton hook. The
// primary template uses the new executors framework.
template <typename Handler,
    typename IoExecutor = system_executor, typename HandlerExecutor
      = typename associated_executor<Handler, IoExecutor>::type>
class handler_work
{
public:
  explicit handler_work(Handler& handler) BOOST_ASIO_NOEXCEPT
    : io_executor_(),
      executor_(boost::asio::get_associated_executor(handler, io_executor_)),
      owns_work_(true)
  {
    io_executor_.on_work_started();
    executor_.on_work_started();
  }

  handler_work(Handler& handler, const IoExecutor& io_ex) BOOST_ASIO_NOEXCEPT
    : io_executor_(io_ex),
      executor_(boost::asio::get_associated_executor(handler, io_executor_)),
      owns_work_(true)
  {
    io_executor_.on_work_started();
    executor_.on_work_started();
  }

  handler_work(const handler_work& other)
    : io_executor_(other.io_executor_),
      executor_(other.executor_),
      owns_work_(other.owns_work_)
  {
    if (owns_work_)
    {
      io_executor_.on_work_started();
      executor_.on_work_started();
    }
  }

#if defined(BOOST_ASIO_HAS_MOVE)
  handler_work(handler_work&& other)
    : io_executor_(BOOST_ASIO_MOVE_CAST(IoExecutor)(other.io_executor_)),
      executor_(BOOST_ASIO_MOVE_CAST(HandlerExecutor)(other.executor_)),
      owns_work_(other.owns_work_)
  {
    other.owns_work_ = false;
  }
#endif // defined(BOOST_ASIO_HAS_MOVE)

  ~handler_work()
  {
    if (owns_work_)
    {
      io_executor_.on_work_finished();
      executor_.on_work_finished();
    }
  }

  template <typename Function>
  void complete(Function& function, Handler& handler)
  {
    executor_.dispatch(BOOST_ASIO_MOVE_CAST(Function)(function),
        boost::asio::get_associated_allocator(handler));
  }

private:
  // Disallow assignment.
  handler_work& operator=(const handler_work&);

  IoExecutor io_executor_;
  HandlerExecutor executor_;
  bool owns_work_;
};

// This specialisation dispatches a handler through the old invocation hook.
// The specialisation is not strictly required for correctness, as the
// system_executor will dispatch through the hook anyway. However, by doing
// this we avoid an extra copy of the handler.
template <typename Handler>
class handler_work<Handler, system_executor, system_executor>
{
public:
  explicit handler_work(Handler&) BOOST_ASIO_NOEXCEPT {}
  handler_work(Handler&, const system_executor&) BOOST_ASIO_NOEXCEPT {}

  template <typename Function>
  void complete(Function& function, Handler& handler)
  {
    boost_asio_handler_invoke_helpers::invoke(function, handler);
  }

private:
  // Disallow assignment.
  handler_work& operator=(const handler_work&);
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_HANDLER_WORK_HPP
