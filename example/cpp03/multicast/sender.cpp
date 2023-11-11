//
// sender.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2023 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <sstream>
#include <string>
#include <boost/asio.hpp>
#include "boost/bind/bind.hpp"

const short multicast_port = 30001;
const int max_message_count = 10;

class sender
{
public:
  sender(boost::asio::io_context& io_context,
      const boost::asio::ip::address& listen_address,
      const boost::asio::ip::address& multicast_address)
    : endpoint_(multicast_address, multicast_port),
      socket_(io_context, endpoint_.protocol()),
      timer_(io_context),
      message_count_(0)
  {
    if (listen_address.is_v6())
      socket_.set_option(boost::asio::ip::multicast::outbound_interface(listen_address.to_v6().scope_id()));
    else
      socket_.set_option(boost::asio::ip::multicast::outbound_interface(listen_address.to_v4()));

    std::ostringstream os;
    os << "Message " << message_count_++;
    message_ = os.str();

    socket_.async_send_to(
        boost::asio::buffer(message_), endpoint_,
        boost::bind(&sender::handle_send_to, this,
          boost::asio::placeholders::error));
  }

  void handle_send_to(const boost::system::error_code& error)
  {
    if (!error && message_count_ < max_message_count)
    {
      timer_.expires_after(boost::asio::chrono::seconds(1));
      timer_.async_wait(
          boost::bind(&sender::handle_timeout, this,
            boost::asio::placeholders::error));
    }
  }

  void handle_timeout(const boost::system::error_code& error)
  {
    if (!error)
    {
      std::ostringstream os;
      os << "Message " << message_count_++;
      message_ = os.str();

      socket_.async_send_to(
          boost::asio::buffer(message_), endpoint_,
          boost::bind(&sender::handle_send_to, this,
            boost::asio::placeholders::error));
    }
  }

private:
  boost::asio::ip::udp::endpoint endpoint_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::steady_timer timer_;
  int message_count_;
  std::string message_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: sender <multicast_address>\n";
      std::cerr << "  For IPv4, try:\n";
      std::cerr << "    sender 0.0.0.0 239.255.0.1\n";
      std::cerr << "  For IPv6, try:\n";
      std::cerr << "    sender 0::0 ff31::8000:1234\n";
      return 1;
    }

    boost::asio::io_context io_context;
    boost::asio::ip::address listen_address(boost::asio::ip::make_address(argv[1]));
    boost::asio::ip::address multicast_address(boost::asio::ip::make_address(argv[2]));
    if ((listen_address.is_v4() && !multicast_address.is_v4())
      || (!listen_address.is_v4() && multicast_address.is_v4()))
    {
      std::cerr << "Error: both addresses have to be IPv4 or IPv6." << std::endl;
      return 1;
    }

    sender s(io_context, listen_address, multicast_address);
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
