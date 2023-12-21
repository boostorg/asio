//
// bind_executor.cpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2023 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <boost/asio/bind_executor.hpp>

#include <functional>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include "unit_test.hpp"

#if defined(BOOST_ASIO_HAS_BOOST_DATE_TIME)
# include <boost/asio/deadline_timer.hpp>
#else // defined(BOOST_ASIO_HAS_BOOST_DATE_TIME)
# include <boost/asio/steady_timer.hpp>
#endif // defined(BOOST_ASIO_HAS_BOOST_DATE_TIME)

using namespace boost::asio;
namespace bindns = std;

#if defined(BOOST_ASIO_HAS_BOOST_DATE_TIME)
typedef deadline_timer timer;
namespace chronons = boost::posix_time;
#else // defined(BOOST_ASIO_HAS_BOOST_DATE_TIME)
typedef steady_timer timer;
namespace chronons = boost::asio::chrono;
#endif // defined(BOOST_ASIO_HAS_BOOST_DATE_TIME)

void increment(int* count)
{
  ++(*count);
}

void bind_executor_to_function_object_test()
{
  io_context ioc1;
  io_context ioc2;

  int count = 0;

  timer t(ioc1, chronons::seconds(1));
  t.async_wait(
      bind_executor(
        ioc2.get_executor(),
        bindns::bind(&increment, &count)));

  ioc1.run();

  BOOST_ASIO_CHECK(count == 0);

  ioc2.run();

  BOOST_ASIO_CHECK(count == 1);
}

struct incrementer_token_v1
{
  explicit incrementer_token_v1(int* c) : count(c) {}
  int* count;
};

struct incrementer_handler_v1
{
  explicit incrementer_handler_v1(incrementer_token_v1 t) : count(t.count) {}
  void operator()(boost::system::error_code){ increment(count); }
  int* count;
};

namespace boost {
namespace asio {

template <>
class async_result<incrementer_token_v1, void(boost::system::error_code)>
{
public:
  typedef incrementer_handler_v1 completion_handler_type;
  typedef void return_type;
  explicit async_result(completion_handler_type&) {}
  return_type get() {}
};

} // namespace asio
} // namespace boost

void bind_executor_to_completion_token_v1_test()
{
  io_context ioc1;
  io_context ioc2;

  int count = 0;

  timer t(ioc1, chronons::seconds(1));
  t.async_wait(
      bind_executor(
        ioc2.get_executor(),
        incrementer_token_v1(&count)));

  ioc1.run();

  BOOST_ASIO_CHECK(count == 0);

  ioc2.run();

  BOOST_ASIO_CHECK(count == 1);
}

struct incrementer_token_v2
{
  explicit incrementer_token_v2(int* c) : count(c) {}
  int* count;
};

namespace boost {
namespace asio {

template <>
class async_result<incrementer_token_v2, void(boost::system::error_code)>
{
public:
#if !defined(BOOST_ASIO_HAS_RETURN_TYPE_DEDUCTION)
  typedef void return_type;
#endif // !defined(BOOST_ASIO_HAS_RETURN_TYPE_DEDUCTION)

  template <typename Initiation, typename... Args>
  static void initiate(Initiation initiation,
      incrementer_token_v2 token, Args&&... args)
  {
    initiation(bindns::bind(&increment, token.count),
        static_cast<Args&&>(args)...);
  }
};

} // namespace asio
} // namespace boost

void bind_executor_to_completion_token_v2_test()
{
  io_context ioc1;
  io_context ioc2;

  int count = 0;

  timer t(ioc1, chronons::seconds(1));
  t.async_wait(
      bind_executor(
        ioc2.get_executor(),
        incrementer_token_v2(&count)));

  ioc1.run();

  BOOST_ASIO_CHECK(count == 0);

  ioc2.run();

  BOOST_ASIO_CHECK(count == 1);
}

BOOST_ASIO_TEST_SUITE
(
  "bind_executor",
  BOOST_ASIO_TEST_CASE(bind_executor_to_function_object_test)
  BOOST_ASIO_TEST_CASE(bind_executor_to_completion_token_v1_test)
  BOOST_ASIO_TEST_CASE(bind_executor_to_completion_token_v2_test)
)
