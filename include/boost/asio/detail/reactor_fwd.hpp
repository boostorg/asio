//
// reactor_fwd.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_REACTOR_FWD_HPP
#define BOOST_ASIO_DETAIL_REACTOR_FWD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/dev_poll_reactor_fwd.hpp>
#include <boost/asio/detail/epoll_reactor_fwd.hpp>
#include <boost/asio/detail/kqueue_reactor_fwd.hpp>
#include <boost/asio/detail/select_reactor_fwd.hpp>
#include <boost/asio/detail/win_iocp_io_service_fwd.hpp>

namespace boost {
namespace asio {
namespace detail {

#if defined(BOOST_ASIO_HAS_IOCP)
typedef select_reactor<true> reactor;
#elif defined(BOOST_ASIO_HAS_EPOLL)
typedef epoll_reactor reactor;
#elif defined(BOOST_ASIO_HAS_KQUEUE)
typedef kqueue_reactor reactor;
#elif defined(BOOST_ASIO_HAS_DEV_POLL)
typedef dev_poll_reactor reactor;
#else
typedef select_reactor<false> reactor;
#endif

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_REACTOR_FWD_HPP
