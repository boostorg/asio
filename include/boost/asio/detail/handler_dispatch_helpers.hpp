//
// handler_dispatch_helpers.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_HANDLER_DISPATCH_HELPERS_HPP
#define BOOST_ASIO_DETAIL_HANDLER_DISPATCH_HELPERS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/handler_dispatch_hook.hpp>

// Calls to asio_handler_dispatch must be made from a namespace that does not
// contain overloads of this function. The boost_asio_handler_dispatch_helpers
// namespace is defined here for that purpose.
namespace boost_asio_handler_dispatch_helpers {

template <typename Handler, typename Context>
inline void dispatch_handler(const Handler& handler, Context* context)
{
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
  Handler tmp(handler);
  tmp();
#else
  using namespace boost::asio;
  asio_handler_dispatch(handler, context);
#endif
}

} // namespace boost_asio_handler_dispatch_helpers

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_HANDLER_DISPATCH_HELPERS_HPP
