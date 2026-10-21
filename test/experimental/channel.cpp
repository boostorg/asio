//
// experimental/channel.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <boost/asio/experimental/channel.hpp>

#include <utility>
#include <boost/asio/any_completion_handler.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/bind_immediate_executor.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/inline_executor.hpp>
#include <boost/asio/io_context.hpp>
#include "../unit_test.hpp"

using namespace boost::asio;
using namespace boost::asio::experimental;

void unbuffered_channel_test()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx);

  BOOST_ASIO_CHECK(ch1.is_open());
  BOOST_ASIO_CHECK(!ch1.can_receive());

  bool b1 = ch1.try_send(boost::asio::error::eof, "hello");

  BOOST_ASIO_CHECK(!b1);

  std::string s1 = "abcdefghijklmnopqrstuvwxyz";
  bool b2 = ch1.try_send(boost::asio::error::eof, std::move(s1));

  BOOST_ASIO_CHECK(!b2);
  BOOST_ASIO_CHECK(!s1.empty());

  boost::system::error_code ec1;
  std::string s2;
  ch1.async_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec1 = ec;
        s2 = std::move(s);
      });

  bool b3 = ch1.try_send(boost::asio::error::eof, std::move(s1));

  BOOST_ASIO_CHECK(b3);
  BOOST_ASIO_CHECK(s1.empty());

  ctx.run();

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s2 == "abcdefghijklmnopqrstuvwxyz");

  bool b4 = ch1.try_receive([](boost::system::error_code, std::string){});

  BOOST_ASIO_CHECK(!b4);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s3 = "zyxwvutsrqponmlkjihgfedcba";
  ch1.async_send(boost::asio::error::eof, std::move(s3),
      [&](boost::system::error_code ec)
      {
        ec2 = ec;
      });

  boost::system::error_code ec3;
  std::string s4;
  bool b5 = ch1.try_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec3 = ec;
        s4 = s;
      });

  BOOST_ASIO_CHECK(b5);
  BOOST_ASIO_CHECK(ec3 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s4 == "zyxwvutsrqponmlkjihgfedcba");

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(!ec2);
}

void buffered_channel_test()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx, 1);

  BOOST_ASIO_CHECK(ch1.is_open());
  BOOST_ASIO_CHECK(!ch1.can_receive());

  bool b1 = ch1.try_send(boost::asio::error::eof, "hello");

  BOOST_ASIO_CHECK(b1);

  std::string s1 = "abcdefghijklmnopqrstuvwxyz";
  bool b2 = ch1.try_send(boost::asio::error::eof, std::move(s1));

  BOOST_ASIO_CHECK(!b2);
  BOOST_ASIO_CHECK(!s1.empty());

  boost::system::error_code ec1;
  std::string s2;
  ch1.async_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec1 = ec;
        s2 = std::move(s);
      });

  ctx.run();

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s2 == "hello");

  bool b4 = ch1.try_receive([](boost::system::error_code, std::string){});

  BOOST_ASIO_CHECK(!b4);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s3 = "zyxwvutsrqponmlkjihgfedcba";
  ch1.async_send(boost::asio::error::eof, std::move(s3),
      [&](boost::system::error_code ec)
      {
        ec2 = ec;
      });

  boost::system::error_code ec3;
  std::string s4;
  bool b5 = ch1.try_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec3 = ec;
        s4 = s;
      });

  BOOST_ASIO_CHECK(b5);
  BOOST_ASIO_CHECK(ec3 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s4 == "zyxwvutsrqponmlkjihgfedcba");

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(!ec2);

  bool b6 = ch1.try_send(boost::system::error_code(), "goodbye");

  BOOST_ASIO_CHECK(b6);

  ch1.close();

  boost::system::error_code ec4;
  std::string s5;
  ch1.async_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec4 = ec;
        s5 = std::move(s);
      });

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(!ec4);
  BOOST_ASIO_CHECK(s5 == "goodbye");

  boost::system::error_code ec5;
  std::string s6;
  ch1.async_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec5 = ec;
        s6 = std::move(s);
      });

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(ec5 == boost::asio::experimental::channel_errc::channel_closed);
  BOOST_ASIO_CHECK(s6.empty());
}

void buffered_error_channel_test()
{
  io_context ctx;

  channel<void(boost::system::error_code)> ch1(ctx, 1);

  BOOST_ASIO_CHECK(ch1.is_open());
  BOOST_ASIO_CHECK(!ch1.can_receive());

  bool b1 = ch1.try_send(boost::asio::error::eof);

  BOOST_ASIO_CHECK(b1);

  bool b2 = ch1.try_send(boost::asio::error::eof);

  BOOST_ASIO_CHECK(!b2);

  boost::system::error_code ec1;
  ch1.async_receive(
      [&](boost::system::error_code ec)
      {
        ec1 = ec;
      });

  ctx.run();

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::eof);

  bool b4 = ch1.try_receive([](boost::system::error_code){});

  BOOST_ASIO_CHECK(!b4);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  ch1.async_send(boost::asio::error::eof,
      [&](boost::system::error_code ec)
      {
        ec2 = ec;
      });

  boost::system::error_code ec3;
  bool b5 = ch1.try_receive(
      [&](boost::system::error_code ec)
      {
        ec3 = ec;
      });

  BOOST_ASIO_CHECK(b5);
  BOOST_ASIO_CHECK(ec3 == boost::asio::error::eof);

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(!ec2);
}

void unbuffered_non_immediate_receive()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(boost::asio::error::eof, std::move(s1),
      [&](boost::system::error_code ec)
      {
        ec1 = ec;
      });

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec2 = ec;
        s2 = std::move(s);
      });

  BOOST_ASIO_CHECK(ec2 == boost::asio::error::would_block);

  ctx.run();

  BOOST_ASIO_CHECK(!ec1);
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s2 == "0123456789");
}

void unbuffered_immediate_receive()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(boost::asio::error::eof, std::move(s1),
      [&](boost::system::error_code ec)
      {
        ec1 = ec;
      });

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_immediate_executor(inline_executor(),
        [&](boost::system::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s2 == "0123456789");

  ctx.run();

  BOOST_ASIO_CHECK(!ec1);
}

void unbuffered_executor_receive()
{
  io_context ctx;
  io_context ctx2;

  channel<void(boost::system::error_code, std::string)> ch1(ctx);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(boost::asio::error::eof, std::move(s1),
      [&](boost::system::error_code ec)
      {
        ec1 = ec;
      });

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_executor(ctx2,
        [&](boost::system::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::would_block);

  ctx.run();

  BOOST_ASIO_CHECK(!ec1);
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::would_block);

  ctx2.run();

  BOOST_ASIO_CHECK(ec2 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s2 == "0123456789");
}

void unbuffered_non_immediate_send()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1;
  ch1.async_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec1 = ec;
        s1 = std::move(s);
      });

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s2 = "0123456789";
  ch1.async_send(boost::asio::error::eof, std::move(s2),
      [&](boost::system::error_code ec)
      {
        ec2 = ec;
      });

  BOOST_ASIO_CHECK(ec2 == boost::asio::error::would_block);

  ctx.run();

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s1 == "0123456789");
  BOOST_ASIO_CHECK(!ec2);
}

void unbuffered_immediate_send()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1;
  ch1.async_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec1 = ec;
        s1 = std::move(s);
      });

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s2 = "0123456789";
  ch1.async_send(boost::asio::error::eof, std::move(s2),
      bind_immediate_executor(inline_executor(),
        [&](boost::system::error_code ec)
        {
          ec2 = ec;
        }));

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(!ec2);

  ctx.run();

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s1 == "0123456789");
}

void unbuffered_executor_send()
{
  io_context ctx;
  io_context ctx2;

  channel<void(boost::system::error_code, std::string)> ch1(ctx);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1;
  ch1.async_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec1 = ec;
        s1 = std::move(s);
      });

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s2 = "0123456789";
  ch1.async_send(boost::asio::error::eof, std::move(s2),
      bind_executor(ctx2,
        [&](boost::system::error_code ec)
        {
          ec2 = ec;
        }));

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::would_block);

  ctx.run();

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s1 == "0123456789");
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::would_block);

  ctx2.run();

  BOOST_ASIO_CHECK(!ec2);
}

void buffered_non_immediate_receive()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx, 1);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(boost::asio::error::eof, std::move(s1),
      [&](boost::system::error_code ec)
      {
        ec1 = ec;
      });

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  ctx.run();

  BOOST_ASIO_CHECK(!ec1);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec2 = ec;
        s2 = std::move(s);
      });

  BOOST_ASIO_CHECK(ec2 == boost::asio::error::would_block);

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(ec2 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s2 == "0123456789");
}

void buffered_immediate_receive()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx, 1);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(boost::asio::error::eof, std::move(s1),
      [&](boost::system::error_code ec)
      {
        ec1 = ec;
      });

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  ctx.run();

  BOOST_ASIO_CHECK(!ec1);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_immediate_executor(inline_executor(),
        [&](boost::system::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  BOOST_ASIO_CHECK(ec2 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s2 == "0123456789");

  ctx.restart();
  ctx.run();
}

void buffered_executor_receive()
{
  io_context ctx;
  io_context ctx2;

  channel<void(boost::system::error_code, std::string)> ch1(ctx, 1);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(boost::asio::error::eof, std::move(s1),
      [&](boost::system::error_code ec)
      {
        ec1 = ec;
      });

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  ctx.run();

  BOOST_ASIO_CHECK(!ec1);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_executor(ctx2,
        [&](boost::system::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  BOOST_ASIO_CHECK(ec2 == boost::asio::error::would_block);

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(ec2 == boost::asio::error::would_block);

  ctx2.run();

  BOOST_ASIO_CHECK(ec2 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s2 == "0123456789");
}

void buffered_non_immediate_send()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx, 1);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(boost::asio::error::eof, std::move(s1),
      [&](boost::system::error_code ec)
      {
        ec1 = ec;
      });

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  ctx.run();

  BOOST_ASIO_CHECK(!ec1);
}

void buffered_immediate_send()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx, 1);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(boost::asio::error::eof, std::move(s1),
      bind_immediate_executor(inline_executor(),
        [&](boost::system::error_code ec)
        {
          ec1 = ec;
        }));

  BOOST_ASIO_CHECK(!ec1);

  ctx.run();
}

void buffered_executor_send()
{
  io_context ctx;
  io_context ctx2;

  channel<void(boost::system::error_code, std::string)> ch1(ctx, 1);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(boost::asio::error::eof, std::move(s1),
      bind_executor(ctx2,
        [&](boost::system::error_code ec)
        {
          ec1 = ec;
        }));

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  ctx.run();

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  ctx2.run();

  BOOST_ASIO_CHECK(!ec1);
}

void try_send_via_dispatch()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1;
  ch1.async_receive(
      bind_executor(boost::asio::inline_executor(),
        [&](boost::system::error_code ec, std::string s)
        {
          ec1 = ec;
          s1 = std::move(s);
        }));

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  ctx.poll();

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  std::string s2 = "0123456789";
  ch1.try_send_via_dispatch(boost::asio::error::eof, std::move(s2));

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s1 == "0123456789");
  BOOST_ASIO_CHECK(s2.empty());
}

void try_send_n_via_dispatch()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1;
  ch1.async_receive(
      bind_executor(boost::asio::inline_executor(),
        [&](boost::system::error_code ec, std::string s)
        {
          ec1 = ec;
          s1 = std::move(s);
        }));

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_executor(boost::asio::inline_executor(),
        [&](boost::system::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  ctx.poll();

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::would_block);

  std::string s3 = "0123456789";
  ch1.try_send_n_via_dispatch(2, boost::asio::error::eof, std::move(s3));

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s1 == "0123456789");
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s2 == "0123456789");
  BOOST_ASIO_CHECK(s3.empty());
}

struct counting_handler
{
  int& count_;
  template <typename... Args>
  void operator()(Args...) { ++count_; }
};

void try_peek_test()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx, 2);

  bool b1 = ch1.try_peek([](boost::system::error_code, std::string){});

  BOOST_ASIO_CHECK(!b1);

  bool b2 = ch1.try_send(boost::asio::error::eof, "hello");

  BOOST_ASIO_CHECK(b2);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1;
  bool b3 = ch1.try_peek(
      [&](boost::system::error_code ec, std::string s)
      {
        ec1 = ec;
        s1 = s;
      });

  BOOST_ASIO_CHECK(b3);
  BOOST_ASIO_CHECK(ec1 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s1 == "hello");

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s2;
  bool b4 = ch1.try_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec2 = ec;
        s2 = s;
      });

  BOOST_ASIO_CHECK(b4);
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s2 == "hello");

  bool b5 = ch1.try_peek([](boost::system::error_code, std::string){});

  BOOST_ASIO_CHECK(!b5);

  boost::system::error_code ec3 = boost::asio::error::would_block;
  std::string s3 = "zyxwvutsrqponmlkjihgfedcba";
  ch1.async_send(boost::asio::error::eof, std::move(s3),
      [&](boost::system::error_code ec)
      {
        ec3 = ec;
      });

  boost::system::error_code ec4 = boost::asio::error::would_block;
  std::string s4;
  bool b6 = ch1.try_peek(
      [&](boost::system::error_code ec, std::string s)
      {
        ec4 = ec;
        s4 = s;
      });

  BOOST_ASIO_CHECK(b6);
  BOOST_ASIO_CHECK(ec4 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s4 == "zyxwvutsrqponmlkjihgfedcba");

  BOOST_ASIO_CHECK(ec3 == boost::asio::error::would_block);

  boost::system::error_code ec5 = boost::asio::error::would_block;
  std::string s5;
  bool b7 = ch1.try_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec5 = ec;
        s5 = s;
      });

  BOOST_ASIO_CHECK(b7);
  BOOST_ASIO_CHECK(ec5 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s5 == "zyxwvutsrqponmlkjihgfedcba");

  ctx.run();

  BOOST_ASIO_CHECK(!ec3);

  channel<void()> ch2(ctx, 2);
  int zero = 0;

  bool b8 = ch2.try_peek(counting_handler{zero});

  BOOST_ASIO_CHECK(!b8);
  BOOST_ASIO_CHECK(zero == 0);

  bool b9 = ch2.try_send();

  BOOST_ASIO_CHECK(b9);

  int peek_count = 0;
  bool b10 = ch2.try_peek(counting_handler{peek_count});

  BOOST_ASIO_CHECK(b10);
  BOOST_ASIO_CHECK(peek_count == 1);

  int receive_count = 0;
  bool b11 = ch2.try_receive(counting_handler{receive_count});

  BOOST_ASIO_CHECK(b11);
  BOOST_ASIO_CHECK(receive_count == 1);

  bool b12 = ch2.try_peek(counting_handler{zero});

  BOOST_ASIO_CHECK(!b12);
  BOOST_ASIO_CHECK(zero == 0);
}

struct multi_signature_handler
{
  std::string* s_;
  boost::system::error_code* ec_;

  void operator()(std::string s)
  {
    *s_ = s;
  }

  void operator()(boost::system::error_code ec)
  {
    *ec_ = ec;
  }
};

void implicit_error_signature_channel_test()
{
  io_context ctx;

  channel<void(std::string)> ch1(ctx);

  BOOST_ASIO_CHECK(ch1.is_open());
  BOOST_ASIO_CHECK(!ch1.can_receive());

  bool b1 = ch1.try_send("hello");

  BOOST_ASIO_CHECK(!b1);

  std::string s1 = "abcdefghijklmnopqrstuvwxyz";
  bool b2 = ch1.try_send(std::move(s1));

  BOOST_ASIO_CHECK(!b2);
  BOOST_ASIO_CHECK(!s1.empty());

  std::string s2;
  boost::system::error_code ec1 = boost::asio::error::would_block;
  multi_signature_handler h1 = {&s2, &ec1};
  ch1.async_receive(h1);

  bool b3 = ch1.try_send(std::move(s1));

  BOOST_ASIO_CHECK(b3);
  BOOST_ASIO_CHECK(s1.empty());

  ctx.run();

  BOOST_ASIO_CHECK(s2 == "abcdefghijklmnopqrstuvwxyz");
  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  std::string s3;
  boost::system::error_code ec2;
  multi_signature_handler h2 = {&s3, &ec2};
  bool b4 = ch1.try_receive(h2);

  BOOST_ASIO_CHECK(!b4);

  std::string s4 = "zyxwvutsrqponmlkjihgfedcba";
  boost::system::error_code ec3;
  ch1.async_send(std::move(s4),
      [&](boost::system::error_code ec)
      {
        ec3 = ec;
      });

  std::string s5;
  boost::system::error_code ec4 = boost::asio::error::would_block;
  multi_signature_handler h3 = {&s5, &ec4};
  bool b5 = ch1.try_receive(h3);

  BOOST_ASIO_CHECK(b5);
  BOOST_ASIO_CHECK(ec4 == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(s5 == "zyxwvutsrqponmlkjihgfedcba");

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(!ec3);

  std::string s6;
  boost::system::error_code ec5 = boost::asio::error::would_block;
  multi_signature_handler h4 = {&s6, &ec5};
  ch1.async_receive(h4);

  ch1.close();

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(s6.empty());
  BOOST_ASIO_CHECK(ec5 == boost::asio::experimental::channel_errc::channel_closed);
}

void channel_with_any_completion_handler_test()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1;
  ch1.async_receive(
      boost::asio::any_completion_handler<
        void(boost::system::error_code, std::string)>(
          [&](boost::system::error_code ec, std::string s)
          {
            ec1 = ec;
            s1 = std::move(s);
          }));

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s2 = "zyxwvutsrqponmlkjihgfedcba";
  ch1.async_send(boost::asio::error::eof, std::move(s2),
      boost::asio::any_completion_handler<void(boost::system::error_code)>(
        [&](boost::system::error_code ec)
        {
          ec2 = ec;
        }));

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::would_block);

  ctx.run();

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s1 == "zyxwvutsrqponmlkjihgfedcba");
  BOOST_ASIO_CHECK(!ec2);
}

void unbuffered_cancel_after_close()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  ch1.async_send(boost::asio::error::eof, "hello",
      [&](boost::system::error_code ec)
      {
        ec1 = ec;
      });

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  ch1.close();
  ch1.cancel();

  ctx.run();

  BOOST_ASIO_CHECK(ec1 == boost::asio::experimental::channel_errc::channel_cancelled);
}

void buffered_cancel_after_close()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx, 1);

  bool b1 = ch1.try_send(boost::asio::error::eof, "first");

  BOOST_ASIO_CHECK(b1);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  ch1.async_send(boost::asio::error::eof, "second",
      [&](boost::system::error_code ec)
      {
        ec1 = ec;
      });

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  ch1.close();
  ch1.cancel();

  ctx.run();

  BOOST_ASIO_CHECK(ec1 == boost::asio::experimental::channel_errc::channel_cancelled);
}

void unbuffered_destroy_after_close()
{
  io_context ctx;

  boost::system::error_code ec1 = boost::asio::error::would_block;

  {
    channel<void(boost::system::error_code, std::string)> ch1(ctx);

    ch1.async_send(boost::asio::error::eof, "hello",
        [&](boost::system::error_code ec)
        {
          ec1 = ec;
        });

    BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

    ch1.close();
  }

  ctx.run();

  BOOST_ASIO_CHECK(ec1 == boost::asio::experimental::channel_errc::channel_cancelled);
}

void unbuffered_drain_after_close()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  ch1.async_send(boost::asio::error::eof, "first",
      [&](boost::system::error_code ec)
      {
        ec1 = ec;
      });

  boost::system::error_code ec2 = boost::asio::error::would_block;
  ch1.async_send(boost::asio::error::eof, "second",
      [&](boost::system::error_code ec)
      {
        ec2 = ec;
      });

  ch1.close();

  BOOST_ASIO_CHECK(!ch1.is_open());
  BOOST_ASIO_CHECK(ch1.ready());

  boost::system::error_code ec3;
  std::string s3;
  bool b1 = ch1.try_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec3 = ec;
        s3 = std::move(s);
      });

  BOOST_ASIO_CHECK(b1);
  BOOST_ASIO_CHECK(ec3 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s3 == "first");

  boost::system::error_code ec4 = boost::asio::error::would_block;
  std::string s4;
  ch1.async_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec4 = ec;
        s4 = std::move(s);
      });

  boost::system::error_code ec5 = boost::asio::error::would_block;
  std::string s5;
  ch1.async_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec5 = ec;
        s5 = std::move(s);
      });

  ctx.run();

  BOOST_ASIO_CHECK(!ec1);
  BOOST_ASIO_CHECK(!ec2);
  BOOST_ASIO_CHECK(ec4 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s4 == "second");
  BOOST_ASIO_CHECK(ec5 == boost::asio::experimental::channel_errc::channel_closed);
  BOOST_ASIO_CHECK(s5.empty());
}

void buffered_drain_after_close()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx, 1);

  bool b1 = ch1.try_send(boost::asio::error::eof, "first");

  BOOST_ASIO_CHECK(b1);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  ch1.async_send(boost::asio::error::eof, "second",
      [&](boost::system::error_code ec)
      {
        ec1 = ec;
      });

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  ch1.close();

  BOOST_ASIO_CHECK(!ch1.is_open());
  BOOST_ASIO_CHECK(ch1.ready());

  boost::system::error_code ec2;
  std::string s2;
  bool b2 = ch1.try_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec2 = ec;
        s2 = std::move(s);
      });

  BOOST_ASIO_CHECK(b2);
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s2 == "first");

  boost::system::error_code ec3 = boost::asio::error::would_block;
  std::string s3;
  ch1.async_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec3 = ec;
        s3 = std::move(s);
      });

  boost::system::error_code ec4 = boost::asio::error::would_block;
  std::string s4;
  ch1.async_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec4 = ec;
        s4 = std::move(s);
      });

  ctx.run();

  BOOST_ASIO_CHECK(!ec1);
  BOOST_ASIO_CHECK(ec3 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s3 == "second");
  BOOST_ASIO_CHECK(ec4 == boost::asio::experimental::channel_errc::channel_closed);
  BOOST_ASIO_CHECK(s4.empty());
}

void send_after_close()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  ch1.async_send(boost::asio::error::eof, "hello",
      [&](boost::system::error_code ec)
      {
        ec1 = ec;
      });

  ch1.close();

  bool b1 = ch1.try_send(boost::asio::error::eof, "rejected");

  BOOST_ASIO_CHECK(!b1);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  ch1.async_send(boost::asio::error::eof, "rejected",
      [&](boost::system::error_code ec)
      {
        ec2 = ec;
      });

  boost::system::error_code ec3;
  std::string s3;
  bool b2 = ch1.try_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec3 = ec;
        s3 = std::move(s);
      });

  BOOST_ASIO_CHECK(b2);
  BOOST_ASIO_CHECK(ec3 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s3 == "hello");

  ctx.run();

  BOOST_ASIO_CHECK(!ec1);
  BOOST_ASIO_CHECK(ec2 == boost::asio::experimental::channel_errc::channel_closed);
}

void can_send_can_receive_test()
{
  io_context ctx;

  channel<void(boost::system::error_code, std::string)> ch1(ctx);

  BOOST_ASIO_CHECK(!ch1.can_send());
  BOOST_ASIO_CHECK(!ch1.can_receive());

  boost::system::error_code ec1 = boost::asio::error::would_block;
  std::string s1 = "hello";
  ch1.async_send(boost::asio::error::eof, std::move(s1),
      [&](boost::system::error_code ec) { ec1 = ec; });

  BOOST_ASIO_CHECK(!ch1.can_send());
  BOOST_ASIO_CHECK(ch1.can_receive());

  bool b1 = ch1.try_receive([](boost::system::error_code, std::string){});

  BOOST_ASIO_CHECK(b1);
  BOOST_ASIO_CHECK(!ch1.can_send());
  BOOST_ASIO_CHECK(!ch1.can_receive());

  ctx.run();

  BOOST_ASIO_CHECK(!ec1);

  boost::system::error_code ec2 = boost::asio::error::would_block;
  std::string s2;
  ch1.async_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec2 = ec;
        s2 = std::move(s);
      });

  BOOST_ASIO_CHECK(ch1.can_send());
  BOOST_ASIO_CHECK(!ch1.can_receive());

  bool b2 = ch1.try_send(boost::asio::error::eof, "world");

  BOOST_ASIO_CHECK(b2);
  BOOST_ASIO_CHECK(!ch1.can_send());
  BOOST_ASIO_CHECK(!ch1.can_receive());

  ctx.restart();
  ctx.run();

  BOOST_ASIO_CHECK(ec2 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s2 == "world");

  channel<void(boost::system::error_code, std::string)> ch2(ctx, 2);

  BOOST_ASIO_CHECK(ch2.can_send());
  BOOST_ASIO_CHECK(!ch2.can_receive());

  BOOST_ASIO_CHECK(ch2.try_send(boost::system::error_code(), "first"));

  BOOST_ASIO_CHECK(ch2.can_send());
  BOOST_ASIO_CHECK(ch2.can_receive());

  BOOST_ASIO_CHECK(ch2.try_send(boost::system::error_code(), "second"));

  BOOST_ASIO_CHECK(!ch2.can_send());
  BOOST_ASIO_CHECK(ch2.can_receive());

  boost::system::error_code ec3;
  std::string s3;
  BOOST_ASIO_CHECK(ch2.try_receive(
      [&](boost::system::error_code ec, std::string s)
      {
        ec3 = ec;
        s3 = std::move(s);
      }));

  BOOST_ASIO_CHECK(!ec3);
  BOOST_ASIO_CHECK(s3 == "first");
  BOOST_ASIO_CHECK(ch2.can_send());
  BOOST_ASIO_CHECK(ch2.can_receive());
}

void channel_move_test()
{
  io_context ctx;

  channel<void(boost::system::error_code)> ch1(ctx);
  channel<void(boost::system::error_code)> ch2 = std::move(ch1);
  (void)ch2;

  channel<void(boost::system::error_code, std::string)> ch3(ctx);
  channel<void(boost::system::error_code, std::string)> ch4 = std::move(ch3);
  (void)ch4;
}

struct int_or_string_handler
{
  boost::system::error_code* ec_;
  int* i_;
  std::string* s_;

  void operator()(boost::system::error_code ec, int i)
  {
    *ec_ = ec;
    *i_ = i;
  }

  void operator()(boost::system::error_code ec, std::string s)
  {
    *ec_ = ec;
    *s_ = std::move(s);
  }
};

struct three_signature_handler
{
  boost::system::error_code* ec_;
  int* i_;
  std::string* s1_;
  std::string* s2_;

  void operator()(boost::system::error_code ec, int i)
  {
    *ec_ = ec;
    *i_ = i;
  }

  void operator()(boost::system::error_code ec, std::string s)
  {
    *ec_ = ec;
    *s1_ = std::move(s);
  }

  void operator()(boost::system::error_code ec, std::string s1, std::string s2)
  {
    *ec_ = ec;
    *s1_ = std::move(s1);
    *s2_ = std::move(s2);
  }
};

void multi_signature_try_send()
{
  io_context ctx;

  channel<void(boost::system::error_code, int),
      void(boost::system::error_code, std::string)> ch1(ctx, 2);

  bool b1 = ch1.try_send(boost::system::error_code(), 42);
  bool b2 = ch1.try_send(boost::asio::error::eof, std::string("second"));

  BOOST_ASIO_CHECK(b1);
  BOOST_ASIO_CHECK(b2);

  boost::system::error_code ec1;
  int i1 = 0;
  std::string s1;
  int_or_string_handler h1 = {&ec1, &i1, &s1};

  bool b3 = ch1.try_receive(h1);

  BOOST_ASIO_CHECK(b3);
  BOOST_ASIO_CHECK(!ec1);
  BOOST_ASIO_CHECK(i1 == 42);

  bool b4 = ch1.try_receive(h1);

  BOOST_ASIO_CHECK(b4);
  BOOST_ASIO_CHECK(ec1 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s1 == "second");

  bool b5 = ch1.try_send(boost::system::error_code(), 0);
  bool b6 = ch1.try_send(boost::system::error_code(), 0);

  BOOST_ASIO_CHECK(b5);
  BOOST_ASIO_CHECK(b6);

  bool b7 = ch1.try_send(boost::asio::error::eof, std::string("full"));

  BOOST_ASIO_CHECK(!b7);

#if defined(BOOST_ASIO_HAS_STD_VARIANT)
  channel<void(boost::system::error_code, int),
      void(boost::system::error_code, std::string),
      void(boost::system::error_code, std::string, std::string)> ch2(ctx, 3);

  bool b8 = ch2.try_send(boost::system::error_code(), 42);
  bool b9 = ch2.try_send(boost::asio::error::eof, std::string("second"));
  bool b10 = ch2.try_send(boost::asio::error::eof,
      std::string("left"), std::string("right"));

  BOOST_ASIO_CHECK(b8);
  BOOST_ASIO_CHECK(b9);
  BOOST_ASIO_CHECK(b10);

  boost::system::error_code ec2;
  int i2 = 0;
  std::string s2;
  std::string s3;
  three_signature_handler h2 = {&ec2, &i2, &s2, &s3};

  bool b11 = ch2.try_receive(h2);

  BOOST_ASIO_CHECK(b11);
  BOOST_ASIO_CHECK(!ec2);
  BOOST_ASIO_CHECK(i2 == 42);

  bool b12 = ch2.try_receive(h2);

  BOOST_ASIO_CHECK(b12);
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s2 == "second");

  bool b13 = ch2.try_receive(h2);

  BOOST_ASIO_CHECK(b13);
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s2 == "left");
  BOOST_ASIO_CHECK(s3 == "right");

  bool b14 = ch2.try_send(boost::asio::error::eof,
      std::string("buffer"), std::string("full"));
  bool b15 = ch2.try_send(boost::asio::error::eof,
      std::string("buffer"), std::string("full"));
  bool b16 = ch2.try_send(boost::asio::error::eof,
      std::string("buffer"), std::string("full"));
  bool b17 = ch2.try_send(boost::asio::error::eof,
      std::string("buffer"), std::string("full"));

  BOOST_ASIO_CHECK(b14);
  BOOST_ASIO_CHECK(b15);
  BOOST_ASIO_CHECK(b16);
  BOOST_ASIO_CHECK(!b17);
#endif // defined(BOOST_ASIO_HAS_STD_VARIANT)
}

struct long_or_int_handler
{
  int* which_;

  void operator()(boost::system::error_code, long)
  {
    *which_ = 1;
  }

  void operator()(boost::system::error_code, int)
  {
    *which_ = 2;
  }
};

void multi_signature_try_send_first_match_wins()
{
  io_context ctx;

  channel<void(boost::system::error_code, long),
      void(boost::system::error_code, int)> ch1(ctx, 1);

  bool b1 = ch1.try_send(boost::system::error_code(), 42);

  BOOST_ASIO_CHECK(b1);

  int which = 0;
  long_or_int_handler h1 = {&which};
  bool b2 = ch1.try_receive(h1);

  BOOST_ASIO_CHECK(b2);
  BOOST_ASIO_CHECK(which == 1);
}

struct movable_payload
{
  int* moves_;
  int* copies_;
  int value_;

  movable_payload(int* moves, int* copies, int value)
    : moves_(moves),
      copies_(copies),
      value_(value)
  {
  }

  movable_payload(const movable_payload& other)
    : moves_(other.moves_),
      copies_(other.copies_),
      value_(other.value_)
  {
    ++*copies_;
  }

  movable_payload(movable_payload&& other)
    : moves_(other.moves_),
      copies_(other.copies_),
      value_(other.value_)
  {
    ++*moves_;
  }
};

void multi_signature_try_send_failure_preserves_arguments()
{
  io_context ctx;

  channel<void(boost::system::error_code, int),
      void(boost::system::error_code, movable_payload)> ch1(ctx);

  int moves = 0;
  int copies = 0;
  movable_payload p1(&moves, &copies, 42);

  bool b1 = ch1.try_send(boost::asio::error::eof, std::move(p1));

  BOOST_ASIO_CHECK(!b1);
  BOOST_ASIO_CHECK(moves == 0);
  BOOST_ASIO_CHECK(copies == 0);

  channel<void(boost::system::error_code, int),
      void(boost::system::error_code, movable_payload)> ch2(ctx, 1);

  bool b2 = ch2.try_send(boost::asio::error::eof, std::move(p1));

  BOOST_ASIO_CHECK(b2);
  BOOST_ASIO_CHECK(moves > 0);
  BOOST_ASIO_CHECK(copies == 0);
}

void multi_signature_try_send_n()
{
#if defined(BOOST_ASIO_HAS_STD_VARIANT)
  io_context ctx;

  channel<void(boost::system::error_code, int),
      void(boost::system::error_code, std::string)> ch1(ctx, 4);

  std::size_t n1 = ch1.try_send_n(2, boost::asio::error::eof, std::string("copied"));

  BOOST_ASIO_CHECK(n1 == 2);

  boost::system::error_code ec1;
  int i1 = 0;
  std::string s1;
  int_or_string_handler h1 = {&ec1, &i1, &s1};

  bool b1 = ch1.try_receive(h1);

  BOOST_ASIO_CHECK(b1);
  BOOST_ASIO_CHECK(ec1 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s1 == "copied");

  s1.clear();
  bool b2 = ch1.try_receive(h1);

  BOOST_ASIO_CHECK(b2);
  BOOST_ASIO_CHECK(ec1 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s1 == "copied");
#endif // defined(BOOST_ASIO_HAS_STD_VARIANT)
}

void multi_signature_try_send_via_dispatch()
{
  io_context ctx;

  channel<void(boost::system::error_code, int),
      void(boost::system::error_code, std::string)> ch1(ctx);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  int i1 = 0;
  std::string s1;
  int_or_string_handler h1 = {&ec1, &i1, &s1};
  ch1.async_receive(bind_executor(boost::asio::inline_executor(), h1));

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  ctx.poll();

  BOOST_ASIO_CHECK(ec1 == boost::asio::error::would_block);

  std::string s2 = "0123456789";
  bool b1 = ch1.try_send_via_dispatch(boost::asio::error::eof, std::move(s2));

  BOOST_ASIO_CHECK(b1);
  BOOST_ASIO_CHECK(ec1 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s1 == "0123456789");
  BOOST_ASIO_CHECK(s2.empty());

#if defined(BOOST_ASIO_HAS_STD_VARIANT)
  boost::system::error_code ec2 = boost::asio::error::would_block;
  int i2 = 0;
  std::string s3;
  int_or_string_handler h2 = {&ec2, &i2, &s3};
  ch1.async_receive(bind_executor(boost::asio::inline_executor(), h2));

  boost::system::error_code ec3 = boost::asio::error::would_block;
  int i3 = 0;
  std::string s4;
  int_or_string_handler h3 = {&ec3, &i3, &s4};
  ch1.async_receive(bind_executor(boost::asio::inline_executor(), h3));

  ctx.poll();

  BOOST_ASIO_CHECK(ec2 == boost::asio::error::would_block);
  BOOST_ASIO_CHECK(ec3 == boost::asio::error::would_block);

  std::string s5 = "9876543210";
  std::size_t n1 = ch1.try_send_n_via_dispatch(
      2, boost::asio::error::eof, std::move(s5));

  BOOST_ASIO_CHECK(n1 == 2);
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s3 == "9876543210");
  BOOST_ASIO_CHECK(ec3 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s4 == "9876543210");
  BOOST_ASIO_CHECK(s5.empty());
#endif // defined(BOOST_ASIO_HAS_STD_VARIANT)
}

void multi_signature_async_send()
{
  io_context ctx;

  channel<void(boost::system::error_code, int),
      void(boost::system::error_code, std::string)> ch1(ctx, 1);

  boost::system::error_code ec1 = boost::asio::error::would_block;
  ch1.async_send(boost::asio::error::eof, std::string("hello"),
      [&](boost::system::error_code ec)
      {
        ec1 = ec;
      });

  boost::system::error_code ec2;
  int i1 = 0;
  std::string s1;
  int_or_string_handler h1 = {&ec2, &i1, &s1};
  bool b1 = ch1.try_receive(h1);

  BOOST_ASIO_CHECK(b1);
  BOOST_ASIO_CHECK(ec2 == boost::asio::error::eof);
  BOOST_ASIO_CHECK(s1 == "hello");

  ctx.run();

  BOOST_ASIO_CHECK(!ec1);
}

BOOST_ASIO_TEST_SUITE
(
  "experimental/channel",
  BOOST_ASIO_TEST_CASE(unbuffered_channel_test)
  BOOST_ASIO_TEST_CASE(buffered_channel_test)
  BOOST_ASIO_TEST_CASE(buffered_error_channel_test)
  BOOST_ASIO_TEST_CASE(unbuffered_non_immediate_receive)
  BOOST_ASIO_TEST_CASE(unbuffered_immediate_receive)
  BOOST_ASIO_TEST_CASE(unbuffered_executor_receive)
  BOOST_ASIO_TEST_CASE(unbuffered_non_immediate_send)
  BOOST_ASIO_TEST_CASE(unbuffered_immediate_send)
  BOOST_ASIO_TEST_CASE(unbuffered_executor_send)
  BOOST_ASIO_TEST_CASE(buffered_non_immediate_receive)
  BOOST_ASIO_TEST_CASE(buffered_immediate_receive)
  BOOST_ASIO_TEST_CASE(buffered_executor_receive)
  BOOST_ASIO_TEST_CASE(buffered_non_immediate_send)
  BOOST_ASIO_TEST_CASE(buffered_immediate_send)
  BOOST_ASIO_TEST_CASE(buffered_executor_send)
  BOOST_ASIO_TEST_CASE(try_send_via_dispatch)
  BOOST_ASIO_TEST_CASE(try_send_n_via_dispatch)
  BOOST_ASIO_TEST_CASE(try_peek_test)
  BOOST_ASIO_TEST_CASE(can_send_can_receive_test)
  BOOST_ASIO_TEST_CASE(implicit_error_signature_channel_test)
  BOOST_ASIO_TEST_CASE(channel_with_any_completion_handler_test)
  BOOST_ASIO_TEST_CASE(unbuffered_cancel_after_close)
  BOOST_ASIO_TEST_CASE(buffered_cancel_after_close)
  BOOST_ASIO_TEST_CASE(unbuffered_destroy_after_close)
  BOOST_ASIO_TEST_CASE(unbuffered_drain_after_close)
  BOOST_ASIO_TEST_CASE(buffered_drain_after_close)
  BOOST_ASIO_TEST_CASE(send_after_close)
  BOOST_ASIO_COMPILE_TEST_CASE(channel_move_test)
  BOOST_ASIO_TEST_CASE(multi_signature_try_send)
  BOOST_ASIO_TEST_CASE(multi_signature_try_send_first_match_wins)
  BOOST_ASIO_TEST_CASE(multi_signature_try_send_failure_preserves_arguments)
  BOOST_ASIO_TEST_CASE(multi_signature_try_send_n)
  BOOST_ASIO_TEST_CASE(multi_signature_try_send_via_dispatch)
  BOOST_ASIO_TEST_CASE(multi_signature_async_send)
)
