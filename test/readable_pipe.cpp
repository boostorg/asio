//
// readable_pipe.cpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header pipe is self-contained.
#include <boost/asio/readable_pipe.hpp>

#include "archetypes/async_result.hpp"
#include <boost/asio/io_context.hpp>
#include "unit_test.hpp"

// readable_pipe_compile test
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that all public member functions on the class
// readable_pipe compile and link correctly. Runtime failures are ignored.

namespace readable_pipe_compile {

struct write_some_handler
{
  write_some_handler() {}
  void operator()(const boost::system::error_code&, std::size_t) {}
  write_some_handler(write_some_handler&&) {}
private:
  write_some_handler(const write_some_handler&);
};

struct read_some_handler
{
  read_some_handler() {}
  void operator()(const boost::system::error_code&, std::size_t) {}
  read_some_handler(read_some_handler&&) {}
private:
  read_some_handler(const read_some_handler&);
};

void test()
{
#if defined(BOOST_ASIO_HAS_PIPE)
  using namespace boost::asio;

  try
  {
    io_context ioc;
    const io_context::executor_type ioc_ex = ioc.get_executor();
    char mutable_char_buffer[128] = "";
    archetypes::lazy_handler lazy;
    boost::system::error_code ec;
    const std::string path;

    // basic_readable_pipe constructors.

    readable_pipe pipe1(ioc);
    readable_pipe::native_handle_type native_pipe1 = pipe1.native_handle();
    readable_pipe pipe2(ioc, native_pipe1);

    readable_pipe pipe3(ioc_ex);
    readable_pipe::native_handle_type native_pipe2 = pipe1.native_handle();
    readable_pipe pipe4(ioc_ex, native_pipe2);

    readable_pipe pipe5(std::move(pipe4));

    basic_readable_pipe<io_context::executor_type> pipe6(ioc);
    readable_pipe pipe7(std::move(pipe6));

    // basic_readable_pipe operators.

    pipe1 = readable_pipe(ioc);
    pipe1 = std::move(pipe2);
    pipe1 = std::move(pipe6);

    // I/O object functions.

    readable_pipe::executor_type ex = pipe1.get_executor();
    (void)ex;

    // basic_readable_pipe functions.

    readable_pipe::native_handle_type native_pipe3 = pipe1.native_handle();
    pipe1.assign(native_pipe3);
    readable_pipe::native_handle_type native_pipe4 = pipe1.native_handle();
    pipe1.assign(native_pipe4, ec);

    bool is_open = pipe1.is_open();
    (void)is_open;

    pipe1.close();
    pipe1.close(ec);

    readable_pipe::native_handle_type native_pipe5 = pipe1.release();
    (void)native_pipe5;
    readable_pipe::native_handle_type native_pipe6 = pipe1.release(ec);
    (void)native_pipe6;

    readable_pipe::native_handle_type native_pipe7 = pipe1.native_handle();
    (void)native_pipe7;

    pipe1.cancel();
    pipe1.cancel(ec);

    pipe1.read_some(buffer(mutable_char_buffer));
    pipe1.read_some(buffer(mutable_char_buffer), ec);

    pipe1.async_read_some(buffer(mutable_char_buffer), read_some_handler());
    int i3 = pipe1.async_read_some(buffer(mutable_char_buffer), lazy);
    (void)i3;
  }
  catch (std::exception&)
  {
  }
#endif // defined(BOOST_ASIO_HAS_PIPE)
}

} // namespace readable_pipe_compile

BOOST_ASIO_TEST_SUITE
(
  "readable_pipe",
  BOOST_ASIO_COMPILE_TEST_CASE(readable_pipe_compile::test)
)
