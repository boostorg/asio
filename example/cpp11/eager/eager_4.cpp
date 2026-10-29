//
// eager_4.cpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <iostream>

using boost::asio::eager;

int main()
{
  boost::asio::io_context ctx;

  boost::asio::steady_timer timer(ctx);
  timer.expires_after(std::chrono::seconds(5));

  auto eager_op = timer.async_wait(eager);

  // A cancellation slot may be bound to the handler that awaits the result,
  // as for any other asynchronous operation.
  boost::asio::cancellation_signal sig;

  std::move(eager_op)(
      boost::asio::bind_cancellation_slot(sig.slot(),
        [](boost::system::error_code ec)
        {
          std::cout << "timer wait finished: " << ec.message() << "\n";
        }
      )
    );

  // The signal is emitted from within the io_context, where the wait was
  // launched.
  boost::asio::steady_timer trigger(ctx);
  trigger.expires_after(std::chrono::milliseconds(100));
  trigger.async_wait(
      [&](boost::system::error_code)
      {
        std::cout << "requesting cancellation\n";
        sig.emit(boost::asio::cancellation_type::all);
      }
    );

  ctx.run();

  return 0;
}
