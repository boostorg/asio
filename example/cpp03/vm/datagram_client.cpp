//
// blocking_udp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

#if defined(BOOST_ASIO_HAS_VM_SOCKETS)

using boost::asio::vm::datagram_protocol;
using boost::asio::vm::cid;

enum { max_length = 1024 };

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: dgram_client <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;

    datagram_protocol::socket s(io_context, datagram_protocol::endpoint(cid::local(), 2001));
    datagram_protocol::endpoint endpoint(cid::local(), atoi(argv[1]));

    using namespace std; // For strlen.
    std::cout << "Enter message: ";
    char request[max_length];
    std::cin.getline(request, max_length);
    size_t request_length = strlen(request);
    s.send_to(boost::asio::buffer(request, request_length), endpoint);

    char reply[max_length];
    datagram_protocol::endpoint sender_endpoint;
    size_t reply_length = s.receive_from(
        boost::asio::buffer(reply, max_length), sender_endpoint);
    std::cout << "Reply is: ";
    std::cout.write(reply, reply_length);
    std::cout << "\n";
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
