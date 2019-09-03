//
// post_1.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio/ts/net.hpp>
#include <boost/asio/use_fiber.hpp>

namespace ctx = boost::context;

boost::asio::io_context io_ctx(1);

ctx::fiber post_loop(ctx::fiber f)
{
  for (int i = 0; i < 1000000; ++i)
  {
    boost::asio::post(io_ctx, boost::asio::use_fiber(f));
  }
  return f;
}

int main()
{
  ctx::fiber(post_loop).resume();
  io_ctx.run();
}
