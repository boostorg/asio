//
// detail/fiber.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_FIBER_HPP
#define BOOST_ASIO_DETAIL_FIBER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_BOOST_CONTEXT_FIBER) \
  && (!defined(BOOST_ASIO_STANDALONE) || !defined(BOOST_ASIO_HAS_ASM_FIBER))
# include <boost/asio/detail/boost_fiber.hpp>
#elif defined(BOOST_ASIO_HAS_ASM_FIBER)
# include <boost/asio/detail/asm_fiber.hpp>
#else
# error No spawn() implementation available
#endif

namespace boost {
namespace asio {
BOOST_ASIO_INLINE_NAMESPACE_BEGIN
namespace detail {

#if defined(BOOST_ASIO_HAS_BOOST_CONTEXT_FIBER) \
  && (!defined(BOOST_ASIO_STANDALONE) || !defined(BOOST_ASIO_HAS_ASM_FIBER))
using fiber = boost_fiber;
using forced_unwind = boost_forced_unwind;
#elif defined(BOOST_ASIO_HAS_ASM_FIBER)
using fiber = asm_fiber;
using forced_unwind = asm_fiber_forced_unwind;
#endif

} // namespace detail
BOOST_ASIO_INLINE_NAMESPACE_END
} // namespace asio
} // namespace boost

#endif // BOOST_ASIO_DETAIL_FIBER_HPP
