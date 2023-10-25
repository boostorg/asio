//
// scheduler.cpp
// ~~~~~~~~~~~~~
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
#include <boost/asio/execution/scheduler.hpp>

#include "../unit_test.hpp"

#if !defined(BOOST_ASIO_NO_DEPRECATED)

namespace exec = boost::asio::execution;

struct not_a_scheduler
{
};

struct executor
{
  executor()
  {
  }

  executor(const executor&) noexcept
  {
  }

  executor(executor&&) noexcept
  {
  }

  template <typename F>
  void execute(F&& f) const noexcept
  {
    (void)f;
  }

  bool operator==(const executor&) const noexcept
  {
    return true;
  }

  bool operator!=(const executor&) const noexcept
  {
    return false;
  }
};

namespace boost {
namespace asio {
namespace traits {

#if !defined(BOOST_ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

template <typename F>
struct execute_member<executor, F>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
  typedef void result_type;
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_SET_ERROR_MEMBER_TRAIT)
#if !defined(BOOST_ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

template <>
struct equality_comparable<executor>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
};

#endif // !defined(BOOST_ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

} // namespace traits
} // namespace asio
} // namespace boost

void test_is_scheduler()
{
  BOOST_ASIO_CHECK(!exec::is_scheduler<void>::value);
  BOOST_ASIO_CHECK(!exec::is_scheduler<not_a_scheduler>::value);
  BOOST_ASIO_CHECK(exec::is_scheduler<executor>::value);
}

BOOST_ASIO_TEST_SUITE
(
  "scheduler",
  BOOST_ASIO_TEST_CASE(test_is_scheduler)
)

#else // !defined(BOOST_ASIO_NO_DEPRECATED)

BOOST_ASIO_TEST_SUITE
(
  "scheduler",
  BOOST_ASIO_TEST_CASE(null_test)
)

#endif // !defined(BOOST_ASIO_NO_DEPRECATED)
