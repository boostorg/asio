//
// detail/handler_tracking.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_HANDLER_TRACKING_HPP
#define BOOST_ASIO_DETAIL_HANDLER_TRACKING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

namespace boost {
namespace asio {
BOOST_ASIO_INLINE_NAMESPACE_BEGIN

class execution_context;

BOOST_ASIO_INLINE_NAMESPACE_END
} // namespace asio
} // namespace boost

#if defined(BOOST_ASIO_CUSTOM_HANDLER_TRACKING)

# include BOOST_ASIO_CUSTOM_HANDLER_TRACKING

// The user-specified header must define the following macros:
// - BOOST_ASIO_INHERIT_TRACKED_HANDLER
// - BOOST_ASIO_ALSO_INHERIT_TRACKED_HANDLER
// - BOOST_ASIO_HANDLER_TRACKING_INIT
// - BOOST_ASIO_HANDLER_CREATION(args)
// - BOOST_ASIO_HANDLER_COMPLETION(args)
// - BOOST_ASIO_HANDLER_INVOCATION_BEGIN(args)
// - BOOST_ASIO_HANDLER_INVOCATION_END
// - BOOST_ASIO_HANDLER_OPERATION(args)
// - BOOST_ASIO_HANDLER_REACTOR_REGISTRATION(args)
// - BOOST_ASIO_HANDLER_REACTOR_DEREGISTRATION(args)
// - BOOST_ASIO_HANDLER_REACTOR_READ_EVENT
// - BOOST_ASIO_HANDLER_REACTOR_WRITE_EVENT
// - BOOST_ASIO_HANDLER_REACTOR_ERROR_EVENT
// - BOOST_ASIO_HANDLER_REACTOR_EVENTS(args)
// - BOOST_ASIO_HANDLER_REACTOR_OPERATION(args)
// - BOOST_ASIO_HANDLER_SCHEDULER_RUN(args)


# if !defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
#  define BOOST_ASIO_ENABLE_HANDLER_TRACKING 1
# endif // !defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)

#elif defined(BOOST_ASIO_PERFETTO_HANDLER_TRACKING)

# include <boost/asio/detail/perfetto_handler_tracking.hpp>

#elif defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)

# include <boost/asio/detail/stderr_handler_tracking.hpp>

#else // defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)

# define BOOST_ASIO_INHERIT_TRACKED_HANDLER
# define BOOST_ASIO_ALSO_INHERIT_TRACKED_HANDLER
# define BOOST_ASIO_HANDLER_TRACKING_INIT (void)0
# define BOOST_ASIO_HANDLER_LOCATION(loc) (void)0
# define BOOST_ASIO_HANDLER_CREATION(args) (void)0
# define BOOST_ASIO_HANDLER_COMPLETION(args) (void)0
# define BOOST_ASIO_HANDLER_INVOCATION_BEGIN(args) (void)0
# define BOOST_ASIO_HANDLER_INVOCATION_END (void)0
# define BOOST_ASIO_HANDLER_OPERATION(args) (void)0
# define BOOST_ASIO_HANDLER_REACTOR_REGISTRATION(args) (void)0
# define BOOST_ASIO_HANDLER_REACTOR_DEREGISTRATION(args) (void)0
# define BOOST_ASIO_HANDLER_REACTOR_READ_EVENT 0
# define BOOST_ASIO_HANDLER_REACTOR_WRITE_EVENT 0
# define BOOST_ASIO_HANDLER_REACTOR_ERROR_EVENT 0
# define BOOST_ASIO_HANDLER_REACTOR_EVENTS(args) (void)0
# define BOOST_ASIO_HANDLER_REACTOR_OPERATION(args) (void)0
# define BOOST_ASIO_HANDLER_SCHEDULER_RUN(args) (void)0

#endif // defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)

#endif // BOOST_ASIO_DETAIL_HANDLER_TRACKING_HPP
