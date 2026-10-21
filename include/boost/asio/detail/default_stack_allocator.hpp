//
// detail/default_stack_allocator.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_DEFAULT_STACK_ALLOCATOR_HPP
#define BOOST_ASIO_DETAIL_DEFAULT_STACK_ALLOCATOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_ASM_FIBER)

#include <cstddef>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
BOOST_ASIO_INLINE_NAMESPACE_BEGIN
namespace detail {

class default_stack_allocator
{
public:
  struct stack_context
  {
    // One past the last usable byte.
    void* sp;

    // The size of the region below sp, so that sp - size is its low end.
    std::size_t size;

    // The low end of the region. The guard page if there is one.
    void* base;
  };

  explicit default_stack_allocator(std::size_t size) noexcept
    : size_(size)
  {
  }

  // Distinct starting offsets handed out to successive stacks, and their
  // spacing. Every mapping is page-aligned, so the same offsets in each stack
  // would otherwise fall in the same cache sets and evict one another.
  // Positioning each stack a little further down spreads them out.
  enum
  {
    colour_count = 64,
    colour_step = 128
  };

  // The system page size, and the size of the guard page.
  BOOST_ASIO_DECL static std::size_t page_size();

  // Obtains a stack, adding a guard page if required. Throws std::bad_alloc on
  // failure.
  BOOST_ASIO_DECL stack_context allocate() const;

  // Release a stack obtained from allocate().
  BOOST_ASIO_DECL void deallocate(stack_context& stack) const;

private:
  // The size of each stack's memory, including any guard page.
  BOOST_ASIO_DECL std::size_t region_size() const;

  std::size_t size_;
};

} // namespace detail
BOOST_ASIO_INLINE_NAMESPACE_END
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_ASIO_HEADER_ONLY)
# include <boost/asio/detail/impl/default_stack_allocator.ipp>
#endif // defined(BOOST_ASIO_HEADER_ONLY)

#endif // defined(BOOST_ASIO_HAS_ASM_FIBER)

#endif // BOOST_ASIO_DETAIL_DEFAULT_STACK_ALLOCATOR_HPP
