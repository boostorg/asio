//
// config.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <boost/asio/config.hpp>

#include <boost/asio/io_context.hpp>
#include <cstdlib>
#include "unit_test.hpp"

void config_from_string_test()
{
  boost::asio::io_context ctx1(
      boost::asio::config_from_string(
        "scheduler.concurrency_hint=123\n"
        " scheduler.locking = 1 \n"
        "# comment\n"
        "garbage\n"
        "reactor.registration_locking= 0 # comment\n"
        "reactor.io_locking=1"));

  boost::asio::config cfg1(ctx1);
  BOOST_ASIO_CHECK(cfg1.get("scheduler", "concurrency_hint", 0) == 123);
  BOOST_ASIO_CHECK(cfg1.get("scheduler", "locking", false) == true);
  BOOST_ASIO_CHECK(cfg1.get("reactor", "registration_locking", true) == false);
  BOOST_ASIO_CHECK(cfg1.get("reactor", "io_locking", false) == true);

  boost::asio::io_context ctx2(
      boost::asio::config_from_string(
        "prefix.scheduler.concurrency_hint=456\n"
        " prefix.scheduler.locking = 1 \n"
        "# comment\n"
        "garbage\n"
        "prefix.reactor.registration_locking= 0 # comment\n"
        "prefix.reactor.io_locking=1",
        "prefix"));

  boost::asio::config cfg2(ctx2);
  BOOST_ASIO_CHECK(cfg2.get("scheduler", "concurrency_hint", 0) == 456);
  BOOST_ASIO_CHECK(cfg2.get("scheduler", "locking", false) == true);
  BOOST_ASIO_CHECK(cfg2.get("reactor", "registration_locking", true) == false);
  BOOST_ASIO_CHECK(cfg2.get("reactor", "io_locking", false) == true);
}

BOOST_ASIO_TEST_SUITE
(
  "config",
  BOOST_ASIO_TEST_CASE(config_from_string_test)
)
