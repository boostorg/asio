//
// lazy_1.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio/ts/net.hpp>
#include <boost/asio/lazy.hpp>
#include <iostream>

int main()
{
  boost::asio::io_context ctx;
  boost::asio::steady_timer timer(ctx);
  timer.expires_after(std::chrono::seconds(1));
  auto x = timer.async_wait(boost::asio::lazy);
  x([](auto){ std::cout << "timer done\n"; });
  ctx.run();
}
