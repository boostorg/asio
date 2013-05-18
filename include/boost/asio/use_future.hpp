//
// use_future.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_USE_FUTURE_HPP
#define BOOST_ASIO_USE_FUTURE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <memory>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {

template <typename Allocator = std::allocator<void> >
class use_future_t
{
public:
  typedef Allocator allocator_type;

  constexpr use_future_t()
  {
  }

  explicit use_future_t(const Allocator& allocator)
    : allocator_(allocator)
  {
  }

  template <typename OtherAllocator>
  use_future_t<OtherAllocator> operator[](const OtherAllocator& allocator) const
  {
    return use_future_t<OtherAllocator>(allocator);
  }

  allocator_type get_allocator() const
  {
    return allocator_;
  }

private:
  Allocator allocator_;
};

// A special value, similar to std::nothrow.
constexpr use_future_t<> use_future;

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/impl/use_future.hpp>

#endif // BOOST_ASIO_USE_FUTURE_HPP
