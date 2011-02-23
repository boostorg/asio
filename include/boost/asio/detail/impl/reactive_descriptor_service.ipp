//
// detail/impl/reactive_descriptor_service.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_IMPL_REACTIVE_DESCRIPTOR_SERVICE_IPP
#define BOOST_ASIO_DETAIL_IMPL_REACTIVE_DESCRIPTOR_SERVICE_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)

#include <boost/asio/error.hpp>
#include <boost/asio/detail/reactive_descriptor_service.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

reactive_descriptor_service::reactive_descriptor_service(
    boost::asio::io_service& io_service)
  : reactor_(boost::asio::use_service<reactor>(io_service))
{
  reactor_.init_task();
}

void reactive_descriptor_service::shutdown_service()
{
}

void reactive_descriptor_service::construct(
    reactive_descriptor_service::implementation_type& impl)
{
  impl.descriptor_ = -1;
  impl.state_ = 0;
}

void reactive_descriptor_service::destroy(
    reactive_descriptor_service::implementation_type& impl)
{
  if (is_open(impl))
  {
    reactor_.deregister_descriptor(impl.descriptor_, impl.reactor_data_,
        (impl.state_ & descriptor_ops::possible_dup) == 0);
  }

  boost::system::error_code ignored_ec;
  descriptor_ops::close(impl.descriptor_, impl.state_, ignored_ec);
}

boost::system::error_code reactive_descriptor_service::assign(
    reactive_descriptor_service::implementation_type& impl,
    const native_handle_type& native_descriptor, boost::system::error_code& ec)
{
  if (is_open(impl))
  {
    ec = boost::asio::error::already_open;
    return ec;
  }

  if (int err = reactor_.register_descriptor(
        native_descriptor, impl.reactor_data_))
  {
    ec = boost::system::error_code(err,
        boost::asio::error::get_system_category());
    return ec;
  }

  impl.descriptor_ = native_descriptor;
  impl.state_ = descriptor_ops::possible_dup;
  ec = boost::system::error_code();
  return ec;
}

boost::system::error_code reactive_descriptor_service::close(
    reactive_descriptor_service::implementation_type& impl,
    boost::system::error_code& ec)
{
  if (is_open(impl))
  {
    reactor_.deregister_descriptor(impl.descriptor_, impl.reactor_data_,
        (impl.state_ & descriptor_ops::possible_dup) == 0);
  }

  if (descriptor_ops::close(impl.descriptor_, impl.state_, ec) == 0)
    construct(impl);

  return ec;
}

reactive_descriptor_service::native_handle_type
reactive_descriptor_service::release(
    reactive_descriptor_service::implementation_type& impl)
{
  native_handle_type descriptor = impl.descriptor_;

  if (is_open(impl))
  {
    reactor_.deregister_descriptor(impl.descriptor_, impl.reactor_data_, false);
    construct(impl);
  }

  return descriptor;
}

boost::system::error_code reactive_descriptor_service::cancel(
    reactive_descriptor_service::implementation_type& impl,
    boost::system::error_code& ec)
{
  if (!is_open(impl))
  {
    ec = boost::asio::error::bad_descriptor;
    return ec;
  }

  reactor_.cancel_ops(impl.descriptor_, impl.reactor_data_);
  ec = boost::system::error_code();
  return ec;
}

void reactive_descriptor_service::start_op(
    reactive_descriptor_service::implementation_type& impl,
    int op_type, reactor_op* op, bool is_non_blocking, bool noop)
{
  if (!noop)
  {
    if ((impl.state_ & descriptor_ops::non_blocking) ||
        descriptor_ops::set_internal_non_blocking(
          impl.descriptor_, impl.state_, true, op->ec_))
    {
      reactor_.start_op(op_type, impl.descriptor_,
          impl.reactor_data_, op, is_non_blocking);
      return;
    }
  }

  reactor_.post_immediate_completion(op);
}

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)

#endif // BOOST_ASIO_DETAIL_IMPL_REACTIVE_DESCRIPTOR_SERVICE_IPP
