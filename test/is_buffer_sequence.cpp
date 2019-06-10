//
// is_buffer_sequence.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2019 Alexander Karzhenkov
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <string>
#include <boost/asio/buffer.hpp>
#include "unit_test.hpp"

using namespace boost::asio;

namespace case1 {

struct no_buffers
{
  typedef const_buffer value_type;
  typedef const_buffer* const_iterator;
  const_iterator begin();
};

struct buffers : public no_buffers
{
  const_iterator end();
};

void run()
{
  BOOST_ASIO_CHECK(is_const_buffer_sequence<no_buffers>::value == 0);
  BOOST_ASIO_CHECK(is_const_buffer_sequence<buffers>::value == 1);
}

} // namespace case1

namespace case2 {

#ifdef BOOST_ASIO_HAS_DECLTYPE
# define DECL_TYPE_MSG "[has decltype] "
#else
# define DECL_TYPE_MSG "[has no decltype] "
#endif

#define TEST_BUFFER_SEQUENCE(Buffer, Condition, Enabled) \
  test<Buffer, Condition, Enabled>::run(DECL_TYPE_MSG \
    "test<" #Buffer ", " #Condition ", " #Enabled ">::")

#define DO_CHECK(Name, Expected) \
  BOOST_ASIO_CHECK_MESSAGE( \
    Condition<Name>::value == Expected, \
    msg_prefix + #Name ": " + result[Expected])

const char* result[] = {"false positive", "false negative"};

template <
  typename Buffer,
  template <typename> typename Condition,
  bool Enabled>
struct test
{
  struct typedefs
  {
    typedef Buffer value_type;
    typedef Buffer* const_iterator;
  };

  struct methods
  {
    Buffer* begin();
    Buffer* end();
  };
  
  struct fake : typedefs
  {
    Buffer* begin;
    Buffer* end;
  };

  struct incomplete : typedefs
  {
    Buffer* begin();
  };

  struct complete : incomplete
  {
    Buffer* end();
  };
  
  static void run(const std::string& msg_prefix)
  {
    DO_CHECK(typedefs  , false);
    DO_CHECK(methods   , false);
    DO_CHECK(fake      , false);
    DO_CHECK(incomplete, false);
    DO_CHECK(complete  , Enabled);
  }
};

void run()
{
  TEST_BUFFER_SEQUENCE(mutable_buffer, is_mutable_buffer_sequence, true);
  TEST_BUFFER_SEQUENCE(mutable_buffer, is_const_buffer_sequence  , true);
  TEST_BUFFER_SEQUENCE(const_buffer  , is_mutable_buffer_sequence, false);
  TEST_BUFFER_SEQUENCE(const_buffer  , is_const_buffer_sequence  , true);
}

} // namespace case2

BOOST_ASIO_TEST_SUITE
(
  "is_buffer_sequence",
  BOOST_ASIO_TEST_CASE(case1::run)
  BOOST_ASIO_TEST_CASE(case2::run)
)
