//
// echo_server_3.cpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio/ts/net.hpp>
#include <boost/asio/use_fiber.hpp>

namespace ctx = boost::context;
using boost::asio::ip::tcp;

boost::asio::io_context io_ctx;

void echo(tcp::socket socket, ctx::fiber& f)
{
  for (;;)
  {
    char data[1024];
    auto [e1, n] = socket.async_read_some(boost::asio::buffer(data), boost::asio::use_fiber(f));
    if (e1) break;
    auto [e2, _] = boost::asio::async_write(socket, boost::asio::buffer(data, n), boost::asio::use_fiber(f));
    if (e2) break;
  }
}

ctx::fiber listener(ctx::fiber f)
{
  tcp::acceptor acceptor(io_ctx, {tcp::v4(), 54321});
  for (;;)
  {
    if (auto [_, socket] = acceptor.async_accept(boost::asio::use_fiber(f)); socket.is_open())
    {
      echo(std::move(socket), f);
    }
  }
}

int main()
{
  ctx::fiber(listener).resume();
  io_ctx.run();
}
