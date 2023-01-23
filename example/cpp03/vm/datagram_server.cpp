//
// async_udp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

#if defined(BOOST_ASIO_HAS_VM_SOCKETS)

using boost::asio::vm::datagram_protocol;

class server
{
public:
  server(boost::asio::io_context& io_context, short port)
    : socket_(io_context, datagram_protocol::endpoint(port))
  {
    socket_.async_receive_from(
        boost::asio::buffer(data_, max_length), sender_endpoint_,
        boost::bind(&server::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

  void handle_receive_from(const boost::system::error_code& error,
      size_t bytes_recvd)
  {
    if (!error && bytes_recvd > 0)
    {
      socket_.async_send_to(
          boost::asio::buffer(data_, bytes_recvd), sender_endpoint_,
          boost::bind(&server::handle_send_to, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      socket_.async_receive_from(
          boost::asio::buffer(data_, max_length), sender_endpoint_,
          boost::bind(&server::handle_receive_from, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
  }

  void handle_send_to(const boost::system::error_code& /*error*/,
      size_t /*bytes_sent*/)
  {
    socket_.async_receive_from(
        boost::asio::buffer(data_, max_length), sender_endpoint_,
        boost::bind(&server::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

private:
  datagram_protocol::socket socket_;
  datagram_protocol::endpoint sender_endpoint_;
  enum { max_length = 1024 };
  char data_[max_length];
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: dgram_server <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;
    using namespace std; // For atoi.
    server s(io_context, atoi(argv[1]));
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

#else // defined(BOOST_ASIO_HAS_VM_SOCKETS)
# error VSOCK sockets not available on this platform.
#endif // defined(BOOST_ASIO_HAS_VM_SOCKETS)
