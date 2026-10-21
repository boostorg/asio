//
// eager_2.cpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <cstring>
#include <iostream>
#include <vector>

using boost::asio::eager;
using boost::asio::ip::tcp;

// An eager operation's type records only the executor and the completion
// signature, so operations of different kinds share a single type and may be
// stored together.
typedef boost::asio::eager_async_operation<
    void(boost::system::error_code, std::size_t)> read_operation;

// An adapted executor is not recorded in the operation's type, so this
// socket's reads have the same type as the others below.
typedef boost::asio::eager_t<>::as_default_on_t<tcp::socket> eager_socket;

int main()
{
  boost::asio::io_context ctx;

  tcp::endpoint local(boost::asio::ip::make_address("127.0.0.1"), 0);
  tcp::acceptor acceptor(ctx, local);
  std::vector<tcp::socket> readers;
  std::vector<tcp::socket> writers;
  for (int i = 0; i < 3; ++i)
  {
    tcp::socket writer(ctx);
    writer.connect(acceptor.local_endpoint());
    readers.push_back(acceptor.accept());
    writers.push_back(std::move(writer));
  }

  char buffer1[16];
  char buffer2[16];
  char buffer3[16];

  eager_socket reader3(std::move(readers[2]));

  std::vector<read_operation> operations;
  operations.push_back(
      readers[0].async_read_some(boost::asio::buffer(buffer1), eager));
  operations.push_back(boost::asio::async_read(readers[1],
        boost::asio::buffer(buffer2, 4), eager));
  operations.push_back(reader3.async_read_some(boost::asio::buffer(buffer3)));

  std::cout << "started " << operations.size()
    << " operations of different kinds, all held as "
    << sizeof(read_operation) << " bytes each\n";

  for (std::size_t i = 0; i < writers.size(); ++i)
    boost::asio::write(writers[i], boost::asio::buffer("data", 4));

  for (std::size_t i = 0; i < operations.size(); ++i)
  {
    std::size_t index = i;
    std::move(operations[i])(
        [index](boost::system::error_code ec, std::size_t n)
        {
          std::cout << "operation " << index << " read " << n
            << " bytes: " << ec.message() << "\n";
        }
      );
  }

  ctx.run();

  return 0;
}
