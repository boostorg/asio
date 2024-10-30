//
// disposition.cpp
// ~~~~~~~~~~~~~~~
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
#include <boost/asio/disposition.hpp>

#include <boost/asio/error.hpp>

#include "unit_test.hpp"

void no_error_test()
{
  using boost::asio::no_error;

  BOOST_ASIO_CHECK(no_error == no_error);
  BOOST_ASIO_CHECK(!(no_error != no_error));
}

void error_code_disposition_test()
{
  using boost::asio::no_error;

  boost::system::error_code ec1;

  BOOST_ASIO_CHECK(ec1 == no_error);
  BOOST_ASIO_CHECK(no_error == ec1);
  BOOST_ASIO_CHECK(!(ec1 != no_error));
  BOOST_ASIO_CHECK(!(no_error != ec1));

  std::exception_ptr ep1 = boost::asio::to_exception_ptr(ec1);
  BOOST_ASIO_CHECK(ep1 == nullptr);

  boost::system::error_code ec2 = boost::asio::error::eof;

  BOOST_ASIO_CHECK(!(ec2 == no_error));
  BOOST_ASIO_CHECK(!(no_error == ec2));
  BOOST_ASIO_CHECK(ec2 != no_error);
  BOOST_ASIO_CHECK(no_error != ec2);

#if !defined(BOOST_ASIO_NO_EXCEPTIONS)
  bool caught;
  try
  {
    boost::asio::throw_exception(ec2);
    caught = false;
  }
  catch (const boost::system::system_error& ex)
  {
    caught = true;
    BOOST_ASIO_CHECK(ex.code() == boost::asio::error::eof);
  }
  BOOST_ASIO_CHECK(caught);
#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)

  std::exception_ptr ep2 = boost::asio::to_exception_ptr(ec2);
  BOOST_ASIO_CHECK(ep2 != nullptr);
}

void exception_ptr_disposition_test()
{
  using boost::asio::no_error;

  std::exception_ptr ep1;

  BOOST_ASIO_CHECK(ep1 == no_error);
  BOOST_ASIO_CHECK(no_error == ep1);
  BOOST_ASIO_CHECK(!(ep1 != no_error));
  BOOST_ASIO_CHECK(!(no_error != ep1));

  std::exception_ptr ep2 = boost::asio::to_exception_ptr(ep1);
  BOOST_ASIO_CHECK(ep1 == nullptr);

  std::exception_ptr ep3 = std::make_exception_ptr(
      boost::system::system_error(boost::asio::error::eof));

  BOOST_ASIO_CHECK(!(ep3 == no_error));
  BOOST_ASIO_CHECK(!(no_error == ep3));
  BOOST_ASIO_CHECK(ep3 != no_error);
  BOOST_ASIO_CHECK(no_error != ep3);

#if !defined(BOOST_ASIO_NO_EXCEPTIONS)
  bool caught;
  try
  {
    boost::asio::throw_exception(ep3);
    caught = false;
  }
  catch (const boost::system::system_error& ex)
  {
    caught = true;
    BOOST_ASIO_CHECK(ex.code() == boost::asio::error::eof);
  }
  BOOST_ASIO_CHECK(caught);
#endif // !defined(BOOST_ASIO_NO_EXCEPTIONS)

  std::exception_ptr ep4 = boost::asio::to_exception_ptr(ep3);
  BOOST_ASIO_CHECK(ep4 != nullptr);
}

BOOST_ASIO_TEST_SUITE
(
  "disposition",
  BOOST_ASIO_TEST_CASE(no_error_test)
  BOOST_ASIO_TEST_CASE(error_code_disposition_test)
  BOOST_ASIO_TEST_CASE(exception_ptr_disposition_test)
)
