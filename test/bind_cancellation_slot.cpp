//
// bind_cancellation_slot.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <boost/asio/bind_cancellation_slot.hpp>

#include <functional>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include "unit_test.hpp"

using namespace boost::asio;
namespace bindns = std;
typedef steady_timer timer;
namespace chronons = boost::asio::chrono;

void increment_on_cancel(int* count, const boost::system::error_code& error)
{
  if (error == boost::asio::error::operation_aborted)
    ++(*count);
}

void bind_cancellation_slot_to_function_object_test()
{
  io_context ioc;
  cancellation_signal sig;

  int count = 0;

  timer t(ioc, chronons::seconds(5));
  t.async_wait(
      bind_cancellation_slot(sig.slot(),
        bindns::bind(&increment_on_cancel,
          &count, bindns::placeholders::_1)));

  ioc.poll();

  BOOST_ASIO_CHECK(count == 0);

  sig.emit(boost::asio::cancellation_type::all);

  ioc.run();

  BOOST_ASIO_CHECK(count == 1);

  t.async_wait(
      bind_cancellation_slot(sig.slot(),
        bind_cancellation_slot(sig.slot(),
          bindns::bind(&increment_on_cancel,
            &count, bindns::placeholders::_1))));

  ioc.restart();
  ioc.poll();

  BOOST_ASIO_CHECK(count == 1);

  sig.emit(boost::asio::cancellation_type::all);

  ioc.run();

  BOOST_ASIO_CHECK(count == 2);
}

struct incrementer_token_v1
{
  explicit incrementer_token_v1(int* c) : count(c) {}
  int* count;
};

struct incrementer_handler_v1
{
  explicit incrementer_handler_v1(incrementer_token_v1 t) : count(t.count) {}

  void operator()(boost::system::error_code error)
  {
    increment_on_cancel(count, error);
  }

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

void bind_cancellation_slot_to_completion_token_v1_test()
{
  io_context ioc;
  cancellation_signal sig;

  int count = 0;

  timer t(ioc, chronons::seconds(5));
  t.async_wait(
      bind_cancellation_slot(sig.slot(),
        incrementer_token_v1(&count)));

  ioc.poll();

  BOOST_ASIO_CHECK(count == 0);

  sig.emit(boost::asio::cancellation_type::all);

  ioc.run();

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
    initiation(
        bindns::bind(&increment_on_cancel,
          token.count, bindns::placeholders::_1),
        static_cast<Args&&>(args)...);
  }
};

} // namespace asio
} // namespace boost

void bind_cancellation_slot_to_completion_token_v2_test()
{
  io_context ioc;
  cancellation_signal sig;

  int count = 0;

  timer t(ioc, chronons::seconds(5));
  t.async_wait(
      bind_cancellation_slot(sig.slot(),
        incrementer_token_v2(&count)));

  ioc.poll();

  BOOST_ASIO_CHECK(count == 0);

  sig.emit(boost::asio::cancellation_type::all);

  ioc.run();

  BOOST_ASIO_CHECK(count == 1);
}

void partial_bind_cancellation_slot()
{
  io_context ioc;
  cancellation_signal sig;

  int count = 0;

  timer t(ioc, chronons::seconds(5));
  t.async_wait(bind_cancellation_slot(sig.slot()))(
      bindns::bind(&increment_on_cancel,
        &count, bindns::placeholders::_1));

  ioc.poll();

  BOOST_ASIO_CHECK(count == 0);

  sig.emit(boost::asio::cancellation_type::all);

  ioc.run();

  BOOST_ASIO_CHECK(count == 1);

  t.async_wait()(
      bind_cancellation_slot(sig.slot()))(
        incrementer_token_v2(&count));

  ioc.restart();
  ioc.poll();

  BOOST_ASIO_CHECK(count == 1);

  sig.emit(boost::asio::cancellation_type::all);

  ioc.run();

  BOOST_ASIO_CHECK(count == 2);
}

BOOST_ASIO_TEST_SUITE
(
  "bind_cancellation_slot",
  BOOST_ASIO_TEST_CASE(bind_cancellation_slot_to_function_object_test)
  BOOST_ASIO_TEST_CASE(bind_cancellation_slot_to_completion_token_v1_test)
  BOOST_ASIO_TEST_CASE(bind_cancellation_slot_to_completion_token_v2_test)
  BOOST_ASIO_TEST_CASE(partial_bind_cancellation_slot)
)
