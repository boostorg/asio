//
// stream_file.cpp
// ~~~~~~~~~~~~~~~
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
#include <boost/asio/stream_file.hpp>

#include <cstdio>
#include <cstring>
#include <vector>
#include "archetypes/async_result.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include "unit_test.hpp"

// stream_file_compile test
// ~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that all public member functions on the class
// stream_file compile and link correctly. Runtime failures are ignored.

namespace stream_file_compile {

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
#if defined(BOOST_ASIO_HAS_FILE)
  using namespace boost::asio;

  try
  {
    io_context ioc;
    const io_context::executor_type ioc_ex = ioc.get_executor();
    char mutable_char_buffer[128] = "";
    const char const_char_buffer[128] = "";
    archetypes::lazy_handler lazy;
    boost::system::error_code ec;
    const std::string path;

    // basic_stream_file constructors.

    stream_file file1(ioc);
    stream_file file2(ioc, "", stream_file::read_only);
    stream_file file3(ioc, path, stream_file::read_only);
    stream_file::native_handle_type native_file1 = file1.native_handle();
    stream_file file4(ioc, native_file1);

    stream_file file5(ioc_ex);
    stream_file file6(ioc_ex, "", stream_file::read_only);
    stream_file file7(ioc_ex, path, stream_file::read_only);
    stream_file::native_handle_type native_file2 = file1.native_handle();
    stream_file file8(ioc_ex, native_file2);

    stream_file file9(std::move(file8));

    basic_stream_file<io_context::executor_type> file10(ioc);
    stream_file file11(std::move(file10));

    // basic_stream_file operators.

    file1 = stream_file(ioc);
    file1 = std::move(file2);
    file1 = std::move(file10);

    // I/O object functions.

    stream_file::executor_type ex = file1.get_executor();
    (void)ex;

    // basic_stream_file functions.

    file1.open("", stream_file::read_only);
    file1.open("", stream_file::read_only, ec);

    file1.open(path, stream_file::read_only);
    file1.open(path, stream_file::read_only, ec);

    stream_file::native_handle_type native_file3 = file1.native_handle();
    file1.assign(native_file3);
    stream_file::native_handle_type native_file4 = file1.native_handle();
    file1.assign(native_file4, ec);

    bool is_open = file1.is_open();
    (void)is_open;

    file1.close();
    file1.close(ec);

    stream_file::native_handle_type native_file5 = file1.native_handle();
    (void)native_file5;

    stream_file::native_handle_type native_file6 = file1.release();
    (void)native_file6;
    stream_file::native_handle_type native_file7 = file1.release(ec);
    (void)native_file7;

    file1.cancel();
    file1.cancel(ec);

    boost::asio::uint64_t s1 = file1.size();
    (void)s1;
    boost::asio::uint64_t s2 = file1.size(ec);
    (void)s2;

    file1.resize(boost::asio::uint64_t(0));
    file1.resize(boost::asio::uint64_t(0), ec);

    file1.sync_all();
    file1.sync_all(ec);

    file1.sync_data();
    file1.sync_data(ec);

    boost::asio::uint64_t s3 = file1.seek(0, stream_file::seek_set);
    (void)s3;
    boost::asio::uint64_t s4 = file1.seek(0, stream_file::seek_set, ec);
    (void)s4;

    file1.write_some(buffer(mutable_char_buffer));
    file1.write_some(buffer(const_char_buffer));
    file1.write_some(buffer(mutable_char_buffer), ec);
    file1.write_some(buffer(const_char_buffer), ec);

    file1.async_write_some(buffer(mutable_char_buffer), write_some_handler());
    file1.async_write_some(buffer(const_char_buffer), write_some_handler());
    int i1 = file1.async_write_some(buffer(mutable_char_buffer), lazy);
    (void)i1;
    int i2 = file1.async_write_some(buffer(const_char_buffer), lazy);
    (void)i2;

    file1.read_some(buffer(mutable_char_buffer));
    file1.read_some(buffer(mutable_char_buffer), ec);

    file1.async_read_some(buffer(mutable_char_buffer), read_some_handler());
    int i3 = file1.async_read_some(buffer(mutable_char_buffer), lazy);
    (void)i3;
  }
  catch (std::exception&)
  {
  }
#endif // defined(BOOST_ASIO_HAS_FILE)
}

} // namespace stream_file_compile

// stream_file_runtime test
// ~~~~~~~~~~~~~~~~~~~~~~~~
// The following tests check that the file position advances by the number of
// bytes transferred when using multi-buffer sequences.

namespace stream_file_runtime {

#if defined(BOOST_ASIO_HAS_FILE)
const char path[] = "stream_file_runtime.tmp";
const char data[] = "abcdefgh";
#endif // defined(BOOST_ASIO_HAS_FILE)

void sync_test()
{
#if defined(BOOST_ASIO_HAS_FILE)
  using namespace boost::asio;

  io_context ioc;
  boost::system::error_code ec;

  std::vector<const_buffer> write_buffers;
  write_buffers.push_back(buffer(data, 0));
  write_buffers.push_back(buffer(data, 3));
  write_buffers.push_back(buffer(data + 3, 5));

  stream_file file(ioc, path, stream_file::write_only
      | stream_file::create | stream_file::truncate);
  std::size_t n = write(file, write_buffers);
  BOOST_ASIO_CHECK(n == 8);
  BOOST_ASIO_CHECK(file.seek(0, stream_file::seek_cur) == 8);
  file.close();

  char read_data[16] = "";
  std::vector<mutable_buffer> read_buffers;
  read_buffers.push_back(buffer(read_data, 0));
  read_buffers.push_back(buffer(read_data, 3));
  read_buffers.push_back(buffer(read_data + 3, 5));

  file.open(path, stream_file::read_only);
  BOOST_ASIO_CHECK(file.size() == 8);
  n = read(file, read_buffers);
  BOOST_ASIO_CHECK(n == 8);
  BOOST_ASIO_CHECK(memcmp(read_data, data, 8) == 0);
  BOOST_ASIO_CHECK(file.seek(0, stream_file::seek_cur) == 8);

  file.seek(0, stream_file::seek_set);
  n = file.read_some(buffer(read_data), ec);
  BOOST_ASIO_CHECK(n == 8);
  BOOST_ASIO_CHECK(!ec);
  BOOST_ASIO_CHECK(file.seek(0, stream_file::seek_cur) == 8);

  n = file.read_some(buffer(read_data), ec);
  BOOST_ASIO_CHECK(n == 0);
  BOOST_ASIO_CHECK(ec == boost::asio::error::eof);
  BOOST_ASIO_CHECK(file.seek(0, stream_file::seek_cur) == 8);

  file.close();
  std::remove(path);
#endif // defined(BOOST_ASIO_HAS_FILE)
}

void async_test()
{
#if defined(BOOST_ASIO_HAS_FILE)
  using namespace boost::asio;

  io_context ioc;
  boost::system::error_code ec;
  std::size_t n = 0;

  std::vector<const_buffer> write_buffers;
  write_buffers.push_back(buffer(data, 0));
  write_buffers.push_back(buffer(data, 3));
  write_buffers.push_back(buffer(data + 3, 5));

  stream_file file(ioc, path, stream_file::write_only
      | stream_file::create | stream_file::truncate);
  async_write(file, write_buffers,
      [&](const boost::system::error_code& e, std::size_t bytes)
      {
        ec = e;
        n = bytes;
      });
  ioc.run();
  BOOST_ASIO_CHECK(!ec);
  BOOST_ASIO_CHECK(n == 8);
  BOOST_ASIO_CHECK(file.seek(0, stream_file::seek_cur) == 8);
  file.close();

  char read_data[16] = "";
  std::vector<mutable_buffer> read_buffers;
  read_buffers.push_back(buffer(read_data, 0));
  read_buffers.push_back(buffer(read_data, 3));
  read_buffers.push_back(buffer(read_data + 3, 5));

  file.open(path, stream_file::read_only);
  BOOST_ASIO_CHECK(file.size() == 8);
  n = 0;
  async_read(file, read_buffers,
      [&](const boost::system::error_code& e, std::size_t bytes)
      {
        ec = e;
        n = bytes;
      });
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(!ec);
  BOOST_ASIO_CHECK(n == 8);
  BOOST_ASIO_CHECK(memcmp(read_data, data, 8) == 0);
  BOOST_ASIO_CHECK(file.seek(0, stream_file::seek_cur) == 8);

  file.close();
  std::remove(path);
#endif // defined(BOOST_ASIO_HAS_FILE)
}

} // namespace stream_file_runtime

BOOST_ASIO_TEST_SUITE
(
  "stream_file",
  BOOST_ASIO_COMPILE_TEST_CASE(stream_file_compile::test)
  BOOST_ASIO_TEST_CASE(stream_file_runtime::sync_test)
  BOOST_ASIO_TEST_CASE(stream_file_runtime::async_test)
)
