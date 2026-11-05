//
// basic_resolver_results.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
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
#include <boost/asio/ip/basic_resolver_results.hpp>

#include <cstring>
#include <boost/asio/ip/tcp.hpp>
#include "../unit_test.hpp"

//------------------------------------------------------------------------------

// ip_basic_resolver_results_runtime test
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that results created from an addrinfo list with
// no usable entries are empty and can be iterated.

namespace ip_basic_resolver_results_runtime {

void test()
{
  using namespace std; // For memset.
  using boost::asio::ip::tcp;

  // An IPv4 entry whose address is too short to be used.
  boost::asio::detail::addrinfo_type info;
  memset(&info, 0, sizeof(info));
  info.ai_family = BOOST_ASIO_OS_DEF(AF_INET);
  info.ai_addrlen = 0;

  tcp::resolver::results_type results =
    tcp::resolver::results_type::create(&info, "host", "service");

  BOOST_ASIO_CHECK(results.size() == 0);
  BOOST_ASIO_CHECK(results.empty());
  BOOST_ASIO_CHECK(results.begin() == results.end());
  BOOST_ASIO_CHECK(results.cbegin() == results.cend());
  BOOST_ASIO_CHECK(results == tcp::resolver::results_type());

  std::size_t count = 0;
  for (tcp::resolver::results_type::const_iterator i = results.begin();
      i != results.end() && count < 2; ++i)
    ++count;
  BOOST_ASIO_CHECK(count == 0);
}

} // namespace ip_basic_resolver_results_runtime

//------------------------------------------------------------------------------

BOOST_ASIO_TEST_SUITE
(
  "ip/basic_resolver_results",
  BOOST_ASIO_TEST_CASE(ip_basic_resolver_results_runtime::test)
)
