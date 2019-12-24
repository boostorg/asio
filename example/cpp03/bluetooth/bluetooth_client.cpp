//
// bluetooth_client.cpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2019 Huang Qinjin (huangqinjin at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdio>
#include <iostream>
#include <boost/asio/bluetooth/stream_protocol.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>


#if defined(BOOST_ASIO_HAS_BLUETOOTH_SOCKETS)

using boost::asio::bluetooth::stream_protocol;

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: bluetooth_client <addr> <channel>\n";
      return 1;
    }

    boost::asio::io_context io_context;

    stream_protocol::endpoint ep(boost::asio::bluetooth::make_address(argv[1]), atoi(argv[2]));
    stream_protocol::socket s(io_context);
    s.connect(ep);
    std::cout << "connect to " << s.remote_endpoint() << std::endl;
    
    char buf[6] = "hello";
    boost::asio::write(s, boost::asio::buffer(buf, 5));
    
    int n = boost::asio::read(s, boost::asio::buffer(buf, 5));
    buf[n] = '\0';
    std::cout << "receive [" << buf << "]" << std::endl;

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

#else // defined(BOOST_ASIO_HAS_BLUETOOTH_SOCKETS)
# error Bluetooth sockets not available on this platform.
#endif // defined(BOOST_ASIO_HAS_BLUETOOTH_SOCKETS)
