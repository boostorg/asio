//
// detail/win_signal_blocker.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_WIN_SIGNAL_BLOCKER_HPP
#define BOOST_ASIO_DETAIL_WIN_SIGNAL_BLOCKER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)

#include <boost/asio/detail/noncopyable.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

class win_signal_blocker
  : private noncopyable
{
public:
  // Constructor blocks all signals for the calling thread.
  win_signal_blocker()
  {
    // No-op.
  }

  // Destructor restores the previous signal mask.
  ~win_signal_blocker()
  {
    // No-op.
  }

  // Block all signals for the calling thread.
  void block()
  {
    // No-op.
  }

  // Restore the previous signal mask.
  void unblock()
  {
    // No-op.
  }
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)

#endif // BOOST_ASIO_DETAIL_WIN_SIGNAL_BLOCKER_HPP
