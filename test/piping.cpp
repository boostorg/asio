//
// piping.cpp
// ~~~~~~~~~~
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
#include <boost/asio/piping.hpp>

#include <boost/asio/deferred.hpp>
#include <boost/asio/eager.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include "unit_test.hpp"

namespace piping_test {
struct not_pipeable
{
};

struct user_op
{
  int* result;

  template <typename Token>
  int operator()(Token&& token) const
  {
    *result = token(7);
    return *result;
  }
};

struct user_token
{
  int operator()(int value) const { return value * 2; }
};
} // namespace piping_test

namespace boost {
namespace asio {
template <>
struct supports_piping<piping_test::user_op> : true_type
{
};
} // namespace asio
} // namespace boost

namespace piping_test {
void piping_trait()
{
  using namespace boost::asio;

  io_context ctx;
  steady_timer timer(ctx);

  typedef decltype(timer.async_wait(deferred)) deferred_op;
  BOOST_ASIO_CHECK(supports_piping<deferred_op>::value);
  BOOST_ASIO_CHECK(supports_piping<deferred_values<int> >::value);

  BOOST_ASIO_CHECK(!supports_piping<deferred_t>::value);

  typedef decltype(timer.async_wait(eager)) eager_op;
  BOOST_ASIO_CHECK(supports_piping<eager_op>::value);

  BOOST_ASIO_CHECK(!is_deferred<eager_op>::value);

  BOOST_ASIO_CHECK(!supports_piping<not_pipeable>::value);
  BOOST_ASIO_CHECK(!supports_piping<int>::value);
  BOOST_ASIO_CHECK(!supports_piping<steady_timer>::value);

  BOOST_ASIO_CHECK(supports_piping<user_op>::value);

#if defined(BOOST_ASIO_HAS_VARIABLE_TEMPLATES)
  BOOST_ASIO_CHECK(supports_piping_v<deferred_op>);
  BOOST_ASIO_CHECK(supports_piping_v<eager_op>);
  BOOST_ASIO_CHECK(!supports_piping_v<not_pipeable>);
#endif // defined(BOOST_ASIO_HAS_VARIABLE_TEMPLATES)
}

void piping_operator()
{
  using namespace boost::asio;

  int result = 0;
  int returned = user_op{&result} | user_token();
  BOOST_ASIO_CHECK(result == 14);
  BOOST_ASIO_CHECK(returned == 14);
}

void piping_deferred_chain()
{
  using namespace boost::asio;

  io_context ctx;
  steady_timer timer(ctx);
  timer.expires_after(std::chrono::milliseconds(0));

  int got = -1;
  timer.async_wait(deferred)
    | deferred([](boost::system::error_code){ return deferred.values(42); })
    | [&](int value){ got = value; };

  ctx.run();

  BOOST_ASIO_CHECK(got == 42);
}

void piping_eager_through_deferred()
{
  using namespace boost::asio;

  io_context ctx;
  steady_timer timer(ctx);
  timer.expires_after(std::chrono::milliseconds(0));

  int got = -1;
  timer.async_wait(eager)
    | deferred([](boost::system::error_code){ return deferred.values(99); })
    | [&](int value){ got = value; };

  ctx.run();

  BOOST_ASIO_CHECK(got == 99);
}
} // namespace piping_test

BOOST_ASIO_TEST_SUITE
(
  "piping",
  BOOST_ASIO_TEST_CASE(piping_test::piping_trait)
  BOOST_ASIO_TEST_CASE(piping_test::piping_operator)
  BOOST_ASIO_TEST_CASE(piping_test::piping_deferred_chain)
  BOOST_ASIO_TEST_CASE(piping_test::piping_eager_through_deferred)
)
