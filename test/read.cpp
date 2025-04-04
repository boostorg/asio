//
// read.cpp
// ~~~~~~~~
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

// Test that header file is self-contained.
#include <boost/asio/read.hpp>

#include <array>
#include <cstring>
#include <functional>
#include <vector>
#include "archetypes/async_result.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/streambuf.hpp>
#include "unit_test.hpp"

#if defined(BOOST_ASIO_HAS_BOOST_ARRAY)
#include <boost/array.hpp>
#endif // defined(BOOST_ASIO_HAS_BOOST_ARRAY)

using namespace std; // For memcmp, memcpy and memset.

class test_stream
{
public:
  typedef boost::asio::io_context::executor_type executor_type;

  test_stream(boost::asio::io_context& io_context)
    : io_context_(io_context),
      length_(0),
      position_(0),
      next_read_length_(0)
  {
  }

  executor_type get_executor() noexcept
  {
    return io_context_.get_executor();
  }

  void reset(const void* data, size_t length)
  {
    BOOST_ASIO_CHECK(length <= max_length);

    memcpy(data_, data, length);
    length_ = length;
    position_ = 0;
    next_read_length_ = length;
  }

  void next_read_length(size_t length)
  {
    next_read_length_ = length;
  }

  template <typename Iterator>
  bool check_buffers(Iterator begin, Iterator end, size_t length)
  {
    if (length != position_)
      return false;

    Iterator iter = begin;
    size_t checked_length = 0;
    for (; iter != end && checked_length < length; ++iter)
    {
      size_t buffer_length = boost::asio::buffer_size(*iter);
      if (buffer_length > length - checked_length)
        buffer_length = length - checked_length;
      if (memcmp(data_ + checked_length, iter->data(), buffer_length) != 0)
        return false;
      checked_length += buffer_length;
    }

    return true;
  }

  template <typename Const_Buffers>
  bool check_buffers(const Const_Buffers& buffers, size_t length)
  {
    return check_buffers(boost::asio::buffer_sequence_begin(buffers),
        boost::asio::buffer_sequence_end(buffers), length);
  }

  template <typename Mutable_Buffers>
  size_t read_some(const Mutable_Buffers& buffers)
  {
    size_t n = boost::asio::buffer_copy(buffers,
        boost::asio::buffer(data_, length_) + position_,
        next_read_length_);
    position_ += n;
    return n;
  }

  template <typename Mutable_Buffers>
  size_t read_some(const Mutable_Buffers& buffers,
      boost::system::error_code& ec)
  {
    ec = boost::system::error_code();
    return read_some(buffers);
  }

  template <typename Mutable_Buffers, typename Handler>
  void async_read_some(const Mutable_Buffers& buffers,
      Handler&& handler)
  {
    size_t bytes_transferred = read_some(buffers);
    boost::asio::post(get_executor(),
        boost::asio::detail::bind_handler(
          static_cast<Handler&&>(handler),
          boost::system::error_code(), bytes_transferred));
  }

private:
  boost::asio::io_context& io_context_;
  enum { max_length = 8192 };
  char data_[max_length];
  size_t length_;
  size_t position_;
  size_t next_read_length_;
};

static const char read_data[]
  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void test_2_arg_zero_buffers_read()
{
  boost::asio::io_context ioc;
  test_stream s(ioc);
  std::vector<boost::asio::mutable_buffer> buffers;

  size_t bytes_transferred = boost::asio::read(s, buffers);
  BOOST_ASIO_CHECK(bytes_transferred == 0);
}

void test_2_arg_mutable_buffer_read()
{
  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  boost::asio::mutable_buffer buffers
    = boost::asio::buffer(read_buf, sizeof(read_buf));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  size_t bytes_transferred = boost::asio::read(s, buffers);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
}

void test_2_arg_vector_buffers_read()
{
  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  std::vector<boost::asio::mutable_buffer> buffers;
  buffers.push_back(boost::asio::buffer(read_buf, 32));
  buffers.push_back(boost::asio::buffer(read_buf, 39) + 32);
  buffers.push_back(boost::asio::buffer(read_buf) + 39);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  size_t bytes_transferred = boost::asio::read(s, buffers);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
}

void test_2_arg_dynamic_string_read()
{
  boost::asio::io_context ioc;
  test_stream s(ioc);
  std::string data;
  boost::asio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb
      = boost::asio::dynamic_buffer(data, sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  size_t bytes_transferred = boost::asio::read(s, sb);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
}

void test_2_arg_streambuf_read()
{
#if !defined(BOOST_ASIO_NO_DYNAMIC_BUFFER_V1)
  boost::asio::io_context ioc;
  test_stream s(ioc);
  boost::asio::streambuf sb(sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  size_t bytes_transferred = boost::asio::read(s, sb);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
#endif // !defined(BOOST_ASIO_NO_DYNAMIC_BUFFER_V1)
}

void test_3_arg_nothrow_zero_buffers_read()
{
  boost::asio::io_context ioc;
  test_stream s(ioc);
  std::vector<boost::asio::mutable_buffer> buffers;

  boost::system::error_code error;
  size_t bytes_transferred = boost::asio::read(s, buffers, error);
  BOOST_ASIO_CHECK(bytes_transferred == 0);
  BOOST_ASIO_CHECK(!error);
}

void test_3_arg_nothrow_mutable_buffer_read()
{
  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  boost::asio::mutable_buffer buffers
    = boost::asio::buffer(read_buf, sizeof(read_buf));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  boost::system::error_code error;
  size_t bytes_transferred = boost::asio::read(s, buffers, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);
}

void test_3_arg_nothrow_vector_buffers_read()
{
  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  std::vector<boost::asio::mutable_buffer> buffers;
  buffers.push_back(boost::asio::buffer(read_buf, 32));
  buffers.push_back(boost::asio::buffer(read_buf, 39) + 32);
  buffers.push_back(boost::asio::buffer(read_buf) + 39);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  boost::system::error_code error;
  size_t bytes_transferred = boost::asio::read(s, buffers, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);
}

void test_3_arg_nothrow_dynamic_string_read()
{
  boost::asio::io_context ioc;
  test_stream s(ioc);
  std::string data;
  boost::asio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb
      = boost::asio::dynamic_buffer(data, sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  boost::system::error_code error;
  size_t bytes_transferred = boost::asio::read(s, sb, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);
}

void test_3_arg_nothrow_streambuf_read()
{
#if !defined(BOOST_ASIO_NO_DYNAMIC_BUFFER_V1)
  boost::asio::io_context ioc;
  test_stream s(ioc);
  boost::asio::streambuf sb(sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  boost::system::error_code error;
  size_t bytes_transferred = boost::asio::read(s, sb, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);
#endif // !defined(BOOST_ASIO_NO_DYNAMIC_BUFFER_V1)
}

bool old_style_transfer_all(const boost::system::error_code& ec,
    size_t /*bytes_transferred*/)
{
  return !!ec;
}

struct short_transfer
{
  short_transfer() {}
  short_transfer(short_transfer&&) {}
  size_t operator()(const boost::system::error_code& ec,
      size_t /*bytes_transferred*/)
  {
    return !!ec ? 0 : 3;
  }
};

void test_3_arg_mutable_buffer_read()
{
  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  boost::asio::mutable_buffer buffers
    = boost::asio::buffer(read_buf, sizeof(read_buf));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  size_t bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_all());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_all());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_all());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(1));
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(1));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(10));
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(42));
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(42));
  BOOST_ASIO_CHECK(bytes_transferred == 50);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, old_style_transfer_all);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, old_style_transfer_all);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, old_style_transfer_all);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, short_transfer());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, short_transfer());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, short_transfer());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
}

void test_3_arg_vector_buffers_read()
{
  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  std::vector<boost::asio::mutable_buffer> buffers;
  buffers.push_back(boost::asio::buffer(read_buf, 32));
  buffers.push_back(boost::asio::buffer(read_buf, 39) + 32);
  buffers.push_back(boost::asio::buffer(read_buf) + 39);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  size_t bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_all());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_all());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_all());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(1));
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(1));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(10));
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(42));
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(42));
  BOOST_ASIO_CHECK(bytes_transferred == 50);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, old_style_transfer_all);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, old_style_transfer_all);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, old_style_transfer_all);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, short_transfer());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, short_transfer());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, short_transfer());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
}

void test_3_arg_dynamic_string_read()
{
  boost::asio::io_context ioc;
  test_stream s(ioc);
  std::string data;
  boost::asio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb
      = boost::asio::dynamic_buffer(data, sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  size_t bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_all());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_all());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_all());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(1));
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(1));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(10));
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(42));
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(42));
  BOOST_ASIO_CHECK(bytes_transferred == 50);
  BOOST_ASIO_CHECK(sb.size() == 50);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 50));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 1));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 42));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, old_style_transfer_all);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, old_style_transfer_all);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, old_style_transfer_all);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, short_transfer());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, short_transfer());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, short_transfer());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
}

void test_3_arg_streambuf_read()
{
#if !defined(BOOST_ASIO_NO_DYNAMIC_BUFFER_V1)
  boost::asio::io_context ioc;
  test_stream s(ioc);
  boost::asio::streambuf sb(sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  size_t bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_all());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_all());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_all());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(1));
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(1));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(10));
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(42));
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(42));
  BOOST_ASIO_CHECK(bytes_transferred == 50);
  BOOST_ASIO_CHECK(sb.size() == 50);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 50));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(1));
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(10));
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(42));
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, old_style_transfer_all);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, old_style_transfer_all);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, old_style_transfer_all);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, short_transfer());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, short_transfer());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, short_transfer());
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
#endif // !defined(BOOST_ASIO_NO_DYNAMIC_BUFFER_V1)
}

void test_4_arg_mutable_buffer_read()
{
  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  boost::asio::mutable_buffer buffers
    = boost::asio::buffer(read_buf, sizeof(read_buf));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  boost::system::error_code error;
  size_t bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_all(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_all(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_all(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 50);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 50));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      old_style_transfer_all, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      old_style_transfer_all, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      old_style_transfer_all, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, short_transfer(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers, short_transfer(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers, short_transfer(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);
}

void test_4_arg_vector_buffers_read()
{
  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  std::vector<boost::asio::mutable_buffer> buffers;
  buffers.push_back(boost::asio::buffer(read_buf, 32));
  buffers.push_back(boost::asio::buffer(read_buf, 39) + 32);
  buffers.push_back(boost::asio::buffer(read_buf) + 39);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  boost::system::error_code error;
  size_t bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_all(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_all(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_all(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_at_least(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 50);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 50));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      boost::asio::transfer_exactly(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers,
      old_style_transfer_all, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      old_style_transfer_all, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers,
      old_style_transfer_all, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = boost::asio::read(s, buffers, short_transfer(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers, short_transfer(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, buffers, short_transfer(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);
}

void test_4_arg_dynamic_string_read()
{
  boost::asio::io_context ioc;
  test_stream s(ioc);
  std::string data;
  boost::asio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb
      = boost::asio::dynamic_buffer(data, sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  boost::system::error_code error;
  size_t bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_all(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_all(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_all(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 50);
  BOOST_ASIO_CHECK(sb.size() == 50);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 50));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      old_style_transfer_all, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      old_style_transfer_all, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      old_style_transfer_all, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, short_transfer(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb, short_transfer(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb, short_transfer(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);
}

void test_4_arg_streambuf_read()
{
#if !defined(BOOST_ASIO_NO_DYNAMIC_BUFFER_V1)
  boost::asio::io_context ioc;
  test_stream s(ioc);
  boost::asio::streambuf sb(sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  boost::system::error_code error;
  size_t bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_all(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_all(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_all(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_at_least(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 50);
  BOOST_ASIO_CHECK(sb.size() == 50);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 50));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(1), error);
  BOOST_ASIO_CHECK(bytes_transferred == 1);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 1));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(10), error);
  BOOST_ASIO_CHECK(bytes_transferred == 10);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      boost::asio::transfer_exactly(42), error);
  BOOST_ASIO_CHECK(bytes_transferred == 42);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 42));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb,
      old_style_transfer_all, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      old_style_transfer_all, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb,
      old_style_transfer_all, error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = boost::asio::read(s, sb, short_transfer(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb, short_transfer(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = boost::system::error_code();
  bytes_transferred = boost::asio::read(s, sb, short_transfer(), error);
  BOOST_ASIO_CHECK(bytes_transferred == sizeof(read_data));
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
  BOOST_ASIO_CHECK(!error);
#endif // !defined(BOOST_ASIO_NO_DYNAMIC_BUFFER_V1)
}

void async_read_handler(const boost::system::error_code& e,
    size_t bytes_transferred, size_t expected_bytes_transferred, bool* called)
{
  *called = true;
  BOOST_ASIO_CHECK(!e);
  BOOST_ASIO_CHECK(bytes_transferred == expected_bytes_transferred);
}

void test_3_arg_mutable_buffer_async_read()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  boost::asio::mutable_buffer buffers
    = boost::asio::buffer(read_buf, sizeof(read_buf));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bool called = false;
  boost::asio::async_read(s, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  int i = boost::asio::async_read(s, buffers, archetypes::lazy_handler());
  BOOST_ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
    boost::asio::async_read(s, buffers)(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
}

void test_3_arg_boost_array_buffers_async_read()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

#if defined(BOOST_ASIO_HAS_BOOST_ARRAY)
  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  boost::array<boost::asio::mutable_buffer, 2> buffers = { {
    boost::asio::buffer(read_buf, 32),
    boost::asio::buffer(read_buf) + 32 } };

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bool called = false;
  boost::asio::async_read(s, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  int i = boost::asio::async_read(s, buffers, archetypes::lazy_handler());
  BOOST_ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers)(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
#endif // defined(BOOST_ASIO_HAS_BOOST_ARRAY)
}

void test_3_arg_std_array_buffers_async_read()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  std::array<boost::asio::mutable_buffer, 2> buffers = { {
    boost::asio::buffer(read_buf, 32),
    boost::asio::buffer(read_buf) + 32 } };

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bool called = false;
  boost::asio::async_read(s, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  int i = boost::asio::async_read(s, buffers, archetypes::lazy_handler());
  BOOST_ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers)(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
}

void test_3_arg_vector_buffers_async_read()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  std::vector<boost::asio::mutable_buffer> buffers;
  buffers.push_back(boost::asio::buffer(read_buf, 32));
  buffers.push_back(boost::asio::buffer(read_buf, 39) + 32);
  buffers.push_back(boost::asio::buffer(read_buf) + 39);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bool called = false;
  boost::asio::async_read(s, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  int i = boost::asio::async_read(s, buffers, archetypes::lazy_handler());
  BOOST_ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers)(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
}

void test_3_arg_dynamic_string_async_read()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  boost::asio::io_context ioc;
  test_stream s(ioc);
  std::string data;
  boost::asio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb
      = boost::asio::dynamic_buffer(data, sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bool called = false;
  boost::asio::async_read(s, sb,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  int i = boost::asio::async_read(s, sb, archetypes::lazy_handler());
  BOOST_ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb)(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
}

void test_3_arg_streambuf_async_read()
{
#if !defined(BOOST_ASIO_NO_DYNAMIC_BUFFER_V1)
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  boost::asio::io_context ioc;
  test_stream s(ioc);
  boost::asio::streambuf sb(sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bool called = false;
  boost::asio::async_read(s, sb,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  int i = boost::asio::async_read(s, sb, archetypes::lazy_handler());
  BOOST_ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb)(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
#endif // !defined(BOOST_ASIO_NO_DYNAMIC_BUFFER_V1)
}

void test_4_arg_mutable_buffer_async_read()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  boost::asio::mutable_buffer buffers
    = boost::asio::buffer(read_buf, sizeof(read_buf));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bool called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  int i = boost::asio::async_read(s, buffers,
      short_transfer(), archetypes::lazy_handler());
  BOOST_ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer())(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
}

void test_4_arg_boost_array_buffers_async_read()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

#if defined(BOOST_ASIO_HAS_BOOST_ARRAY)
  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  boost::array<boost::asio::mutable_buffer, 2> buffers = { {
    boost::asio::buffer(read_buf, 32),
    boost::asio::buffer(read_buf) + 32 } };

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bool called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  int i = boost::asio::async_read(s, buffers,
      short_transfer(), archetypes::lazy_handler());
  BOOST_ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer())(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
#endif // defined(BOOST_ASIO_HAS_BOOST_ARRAY)
}

void test_4_arg_std_array_buffers_async_read()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  std::array<boost::asio::mutable_buffer, 2> buffers = { {
    boost::asio::buffer(read_buf, 32),
    boost::asio::buffer(read_buf) + 32 } };

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bool called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  int i = boost::asio::async_read(s, buffers,
      short_transfer(), archetypes::lazy_handler());
  BOOST_ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer())(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
}

void test_4_arg_vector_buffers_async_read()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  boost::asio::io_context ioc;
  test_stream s(ioc);
  char read_buf[sizeof(read_data)];
  std::vector<boost::asio::mutable_buffer> buffers;
  buffers.push_back(boost::asio::buffer(read_buf, 32));
  buffers.push_back(boost::asio::buffer(read_buf, 39) + 32);
  buffers.push_back(boost::asio::buffer(read_buf) + 39);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bool called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  int i = boost::asio::async_read(s, buffers,
      short_transfer(), archetypes::lazy_handler());
  BOOST_ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  boost::asio::async_read(s, buffers, short_transfer())(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(s.check_buffers(buffers, sizeof(read_data)));
}

void test_4_arg_dynamic_string_async_read()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  boost::asio::io_context ioc;
  test_stream s(ioc);
  std::string data;
  boost::asio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb
      = boost::asio::dynamic_buffer(data, sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bool called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 50);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 50));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 1));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), 42));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  int i = boost::asio::async_read(s, sb,
      short_transfer(), archetypes::lazy_handler());
  BOOST_ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, short_transfer())(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(0, sb.size()), sizeof(read_data)));
}

void test_4_arg_streambuf_async_read()
{
#if !defined(BOOST_ASIO_NO_DYNAMIC_BUFFER_V1)
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  boost::asio::io_context ioc;
  test_stream s(ioc);
  boost::asio::streambuf sb(sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bool called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 50);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 50));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 1);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 10);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, boost::asio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == 42);
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  int i = boost::asio::async_read(s, sb,
      short_transfer(), archetypes::lazy_handler());
  BOOST_ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  boost::asio::async_read(s, sb, short_transfer())(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  BOOST_ASIO_CHECK(called);
  BOOST_ASIO_CHECK(sb.size() == sizeof(read_data));
  BOOST_ASIO_CHECK(s.check_buffers(sb.data(), sizeof(read_data)));
#endif // !defined(BOOST_ASIO_NO_DYNAMIC_BUFFER_V1)
}

BOOST_ASIO_TEST_SUITE
(
  "read",
  BOOST_ASIO_TEST_CASE(test_2_arg_zero_buffers_read)
  BOOST_ASIO_TEST_CASE(test_2_arg_mutable_buffer_read)
  BOOST_ASIO_TEST_CASE(test_2_arg_vector_buffers_read)
  BOOST_ASIO_TEST_CASE(test_2_arg_dynamic_string_read)
  BOOST_ASIO_TEST_CASE(test_2_arg_streambuf_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_nothrow_zero_buffers_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_nothrow_mutable_buffer_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_nothrow_vector_buffers_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_nothrow_dynamic_string_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_nothrow_streambuf_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_mutable_buffer_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_vector_buffers_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_dynamic_string_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_streambuf_read)
  BOOST_ASIO_TEST_CASE(test_4_arg_mutable_buffer_read)
  BOOST_ASIO_TEST_CASE(test_4_arg_vector_buffers_read)
  BOOST_ASIO_TEST_CASE(test_4_arg_dynamic_string_read)
  BOOST_ASIO_TEST_CASE(test_4_arg_streambuf_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_mutable_buffer_async_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_boost_array_buffers_async_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_std_array_buffers_async_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_vector_buffers_async_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_dynamic_string_async_read)
  BOOST_ASIO_TEST_CASE(test_3_arg_streambuf_async_read)
  BOOST_ASIO_TEST_CASE(test_4_arg_mutable_buffer_async_read)
  BOOST_ASIO_TEST_CASE(test_4_arg_vector_buffers_async_read)
  BOOST_ASIO_TEST_CASE(test_4_arg_boost_array_buffers_async_read)
  BOOST_ASIO_TEST_CASE(test_4_arg_std_array_buffers_async_read)
  BOOST_ASIO_TEST_CASE(test_4_arg_dynamic_string_async_read)
  BOOST_ASIO_TEST_CASE(test_4_arg_streambuf_async_read)
)
