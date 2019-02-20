//
// detail/win_iocp_overlapped_ptr.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_WIN_IOCP_OVERLAPPED_PTR_HPP
#define BOOST_ASIO_DETAIL_WIN_IOCP_OVERLAPPED_PTR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_IOCP)

#include <boost/asio/io_context.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/memory.hpp>
#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/win_iocp_overlapped_op.hpp>
#include <boost/asio/detail/win_iocp_io_context.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

// Wraps a handler to create an OVERLAPPED object for use with overlapped I/O.
class win_iocp_overlapped_ptr
  : private noncopyable
{
public:
  // Construct an empty win_iocp_overlapped_ptr.
  win_iocp_overlapped_ptr()
    : ptr_(0),
      iocp_service_(0)
  {
  }

  // Construct a win_iocp_overlapped_ptr to contain the specified handler.
  template <typename Executor, typename Handler>
  explicit win_iocp_overlapped_ptr(
      const Executor& ex, BOOST_ASIO_MOVE_ARG(Handler) handler)
    : ptr_(0),
      iocp_service_(0)
  {
    this->reset(ex, BOOST_ASIO_MOVE_CAST(Handler)(handler));
  }

  // Construct a win_iocp_overlapped_ptr to contain the specified handler.
  template <typename ExecutionContext, typename Handler>
  explicit win_iocp_overlapped_ptr(
      ExecutionContext& context, BOOST_ASIO_MOVE_ARG(Handler) handler,
      typename enable_if<
        is_convertible<ExecutionContext&, execution_context&>::value
      >::type* = 0)
    : ptr_(0),
      iocp_service_(0)
  {
    this->reset(context.get_executor(), BOOST_ASIO_MOVE_CAST(Handler)(handler));
  }

  // Destructor automatically frees the OVERLAPPED object unless released.
  ~win_iocp_overlapped_ptr()
  {
    reset();
  }

  // Reset to empty.
  void reset()
  {
    if (ptr_)
    {
      ptr_->destroy();
      ptr_ = 0;
      iocp_service_->work_finished();
      iocp_service_ = 0;
    }
  }

  // Reset to contain the specified handler, freeing any current OVERLAPPED
  // object.
  template <typename Executor, typename Handler>
  void reset(const Executor& ex, Handler handler)
  {
    detail::win_iocp_io_context* svc =
        &boost::asio::use_service<detail::win_iocp_io_context>(ex.context());
    typedef win_iocp_overlapped_op<Executor, Handler> op;
    typename op::ptr p = { boost::asio::detail::addressof(handler),
      op::ptr::allocate(handler), 0 };
    p.p = new (p.v) op(ex, handler);

    BOOST_ASIO_HANDLER_CREATION((io_context, *p.p,
          "io_context", svc, 0, "overlapped"));

    svc->work_started();
    reset();
    ptr_ = p.p;
    p.v = p.p = 0;
    iocp_service_ = svc;
  }

  // Get the contained OVERLAPPED object.
  OVERLAPPED* get()
  {
    return ptr_;
  }

  // Get the contained OVERLAPPED object.
  const OVERLAPPED* get() const
  {
    return ptr_;
  }

  // Release ownership of the OVERLAPPED object.
  OVERLAPPED* release()
  {
    if (ptr_)
      iocp_service_->on_pending(ptr_);

    OVERLAPPED* tmp = ptr_;
    ptr_ = 0;
    iocp_service_ = 0;
    return tmp;
  }

  // Post completion notification for overlapped operation. Releases ownership.
  void complete(const boost::system::error_code& ec,
      std::size_t bytes_transferred)
  {
    if (ptr_)
    {
      iocp_service_->on_completion(ptr_, ec,
          static_cast<DWORD>(bytes_transferred));
      ptr_ = 0;
      iocp_service_ = 0;
    }
  }

private:
  win_iocp_operation* ptr_;
  win_iocp_io_context* iocp_service_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_IOCP)

#endif // BOOST_ASIO_DETAIL_WIN_IOCP_OVERLAPPED_PTR_HPP
