//
// redirect_error.cpp
// ~~~~~~~~~~~~~~~~~~
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
#include <boost/asio/redirect_error.hpp>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/use_future.hpp>
#include "unit_test.hpp"

struct redirect_error_handler
{
  int* count_;

  explicit redirect_error_handler(int* c)
    : count_(c)
  {
  }

  void operator()()
  {
    ++(*count_);
  }
};

void redirect_error_test()
{
  boost::asio::io_context io1;
  boost::asio::io_context io2;
  boost::asio::system_timer timer1(io1);
  boost::system::error_code ec = boost::asio::error::would_block;
  int count = 0;

  timer1.expires_after(boost::asio::chrono::seconds(0));
  timer1.async_wait(
      boost::asio::redirect_error(
        boost::asio::bind_executor(io2.get_executor(),
          redirect_error_handler(&count)), ec));

  BOOST_ASIO_CHECK(ec == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(count == 0);

  io1.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(count == 0);

  io2.run();

  BOOST_ASIO_CHECK(!ec);
  BOOST_ASIO_CHECK(count == 1);

  ec = boost::asio::error::would_block;
  timer1.async_wait(
      boost::asio::redirect_error(
        boost::asio::bind_executor(io2.get_executor(),
          boost::asio::deferred), ec))(redirect_error_handler(&count));

  BOOST_ASIO_CHECK(ec == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(count == 1);

  io1.restart();
  io1.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(count == 1);

  io2.restart();
  io2.run();

  BOOST_ASIO_CHECK(!ec);
  BOOST_ASIO_CHECK(count == 2);

#if defined(BOOST_ASIO_HAS_STD_FUTURE_CLASS)
  ec = boost::asio::error::would_block;
  std::future<void> f = timer1.async_wait(
      boost::asio::redirect_error(
        boost::asio::bind_executor(io2.get_executor(),
          boost::asio::use_future), ec));

  BOOST_ASIO_CHECK(ec == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::timeout);

  io1.restart();
  io1.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::timeout);

  io2.restart();
  io2.run();

  BOOST_ASIO_CHECK(!ec);
  BOOST_ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::ready);
#endif // defined(BOOST_ASIO_HAS_STD_FUTURE_CLASS)
}

void partial_redirect_error_test()
{
  boost::asio::io_context io1;
  boost::asio::io_context io2;
  boost::asio::system_timer timer1(io1);
  boost::system::error_code ec = boost::asio::error::would_block;
  int count = 0;

  timer1.expires_after(boost::asio::chrono::seconds(0));
  timer1.async_wait(boost::asio::redirect_error(ec))(
      boost::asio::bind_executor(io2.get_executor(),
        redirect_error_handler(&count)));

  BOOST_ASIO_CHECK(ec == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(count == 0);

  io1.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(count == 0);

  io2.run();

  BOOST_ASIO_CHECK(!ec);
  BOOST_ASIO_CHECK(count == 1);

  ec = boost::asio::error::would_block;
  timer1.async_wait(boost::asio::redirect_error(ec))(
      boost::asio::bind_executor(io2.get_executor(),
        boost::asio::deferred))(redirect_error_handler(&count));

  BOOST_ASIO_CHECK(ec == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(count == 1);

  io1.restart();
  io1.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(count == 1);

  io2.restart();
  io2.run();

  BOOST_ASIO_CHECK(!ec);
  BOOST_ASIO_CHECK(count == 2);

  ec = boost::asio::error::would_block;
  timer1.async_wait()(boost::asio::redirect_error(ec))(
      boost::asio::bind_executor(io2.get_executor(),
        boost::asio::deferred))(redirect_error_handler(&count));

  BOOST_ASIO_CHECK(ec == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(count == 2);

  io1.restart();
  io1.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(count == 2);

  io2.restart();
  io2.run();

  BOOST_ASIO_CHECK(!ec);
  BOOST_ASIO_CHECK(count == 3);

#if defined(BOOST_ASIO_HAS_STD_FUTURE_CLASS)
  ec = boost::asio::error::would_block;
  std::future<void> f = timer1.async_wait(boost::asio::redirect_error(ec))(
      boost::asio::bind_executor(io2.get_executor(), boost::asio::use_future));

  BOOST_ASIO_CHECK(ec == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::timeout);

  io1.restart();
  io1.run();

  BOOST_ASIO_CHECK(ec == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::timeout);

  io2.restart();
  io2.run();

  BOOST_ASIO_CHECK(!ec);
  BOOST_ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::ready);
#endif // defined(BOOST_ASIO_HAS_STD_FUTURE_CLASS)
}

BOOST_ASIO_TEST_SUITE
(
  "redirect_error",
  BOOST_ASIO_TEST_CASE(redirect_error_test)
  BOOST_ASIO_TEST_CASE(partial_redirect_error_test)
)
