//
// detail/blocking_executor_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_BLOCKING_EXECUTOR_OP_HPP
#define BOOST_ASIO_DETAIL_BLOCKING_EXECUTOR_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/detail/event.hpp>
#include <boost/asio/detail/fenced_block.hpp>
#include <boost/asio/detail/mutex.hpp>
#include <boost/asio/detail/operation.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
BOOST_ASIO_INLINE_NAMESPACE_BEGIN
namespace detail {

class blocking_executor_op_base : public operation
{
public:
  blocking_executor_op_base(operation::func_type complete_func)
    : operation(complete_func),
      is_complete_(false)
  {
  }

  void wait()
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    while (!is_complete_)
      event_.wait(lock);
  }

protected:
  struct do_complete_cleanup
  {
    ~do_complete_cleanup()
    {
      boost::asio::detail::mutex::scoped_lock lock(op_->mutex_);
      op_->is_complete_ = true;
      op_->event_.unlock_and_signal_one_for_destruction(lock);
    }

    blocking_executor_op_base* op_;
  };

private:
  boost::asio::detail::mutex mutex_;
  boost::asio::detail::event event_;
  bool is_complete_;
};

template <typename Handler>
class blocking_executor_op : public blocking_executor_op_base
{
public:
  blocking_executor_op(Handler& h)
    : blocking_executor_op_base(&blocking_executor_op::do_complete),
      handler_(h)
  {
  }

  static void do_complete(void* owner, operation* base,
      int /*error_value*/, std::size_t /*bytes_transferred*/)
  {
    BOOST_ASIO_ASSUME(base != 0);
    blocking_executor_op* o(static_cast<blocking_executor_op*>(base));

    blocking_executor_op_base::do_complete_cleanup
      on_exit = { o };
    (void)on_exit;

    BOOST_ASIO_HANDLER_COMPLETION((*o));

    // Make the upcall if required.
    if (owner)
    {
      fenced_block b(fenced_block::half);
      BOOST_ASIO_HANDLER_INVOCATION_BEGIN(());
      static_cast<Handler&&>(o->handler_)();
      BOOST_ASIO_HANDLER_INVOCATION_END;
    }
  }

private:
  Handler& handler_;
};

} // namespace detail
BOOST_ASIO_INLINE_NAMESPACE_END
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_BLOCKING_EXECUTOR_OP_HPP
