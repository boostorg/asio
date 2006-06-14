//
// placeholders.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_ARG_HPP
#define BOOST_ASIO_ARG_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/bind/arg.hpp>
#include <boost/asio/detail/pop_options.hpp>

namespace boost {
namespace asio {

namespace placeholders {

namespace {

#if defined(__BORLANDC__)

static inline boost::arg<1> error()
{
  return boost::arg<1>();
}

static inline boost::arg<2> bytes_transferred()
{
  return boost::arg<2>();
}

static inline boost::arg<2> iterator()
{
  return boost::arg<2>();
}

#elif defined(_MSC_VER) && (_MSC_VER < 1400)

static boost::arg<1> error;
static boost::arg<2> bytes_transferred;
static boost::arg<2> iterator;

#else

/// An argument placeholder, for use with @ref boost_bind, that corresponds to
/// the error argument of a handler for any of the asynchronous functions.
boost::arg<1> error;

/// An argument placeholder, for use with @ref boost_bind, that corresponds to
/// the bytes_transferred argument of a handler for asynchronous functions such
/// as boost::asio::basic_stream_socket::async_write_some or
/// boost::asio::async_write.
boost::arg<2> bytes_transferred;

/// An argument placeholder, for use with @ref boost_bind, that corresponds to
/// the iterator argument of a handler for asynchronous functions such as
/// boost::asio::basic_resolver::resolve.
boost::arg<2> iterator;

#endif

} // namespace

} // namespace placeholders

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_ARG_HPP
