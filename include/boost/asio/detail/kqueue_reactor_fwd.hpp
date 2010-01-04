//
// kqueue_reactor_fwd.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2005 Stefan Arentz (stefan at soze dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_KQUEUE_REACTOR_FWD_HPP
#define BOOST_ASIO_DETAIL_KQUEUE_REACTOR_FWD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#if !defined(BOOST_ASIO_DISABLE_KQUEUE)

#if (defined(__MACH__) && defined(__APPLE__)) \
  || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)

// Define this to indicate that kqueue is supported on the target platform.
#define BOOST_ASIO_HAS_KQUEUE 1

namespace boost {
namespace asio {
namespace detail {

template <bool Own_Thread>
class kqueue_reactor;

} // namespace detail
} // namespace asio
} // namespace boost

#endif // (defined(__MACH__) && defined(__APPLE__))
       // || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)

#endif // !defined(BOOST_ASIO_DISABLE_KQUEUE)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_KQUEUE_REACTOR_FWD_HPP
