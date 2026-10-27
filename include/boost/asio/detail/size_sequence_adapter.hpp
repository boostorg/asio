//
// detail/size_sequence_adapter.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_SIZE_SEQUENCE_ADAPTER_HPP
#define BOOST_ASIO_DETAIL_SIZE_SEQUENCE_ADAPTER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if !defined(BOOST_ASIO_WINDOWS_RUNTIME)

#include <cstddef>
#include <boost/asio/detail/empty_range.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/detail/unwrap_ref.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
BOOST_ASIO_INLINE_NAMESPACE_BEGIN
namespace detail {

// Helper class to record the size of each received message.
template <typename SizeSequence>
class size_sequence_adapter
{
public:
  // Set each size to the number of bytes in the corresponding message.
  static void set_sizes(const mmsghdr* msgs,
      std::size_t count, SizeSequence& sizes)
  {
    auto it = unwrap_ref(sizes).begin();
    for (std::size_t i = 0; i < count; ++i, ++it)
      unwrap_ref(*it) = BOOST_ASIO_MMSGHDR_LEN(msgs[i]);
  }
};

template <>
class size_sequence_adapter<empty_range>
{
public:
  static void set_sizes(const mmsghdr*, std::size_t, const empty_range&)
  {
  }
};

template <>
class size_sequence_adapter<const empty_range>
  : public size_sequence_adapter<empty_range>
{
};

} // namespace detail
BOOST_ASIO_INLINE_NAMESPACE_END
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // !defined(BOOST_ASIO_WINDOWS_RUNTIME)

#endif // BOOST_ASIO_DETAIL_SIZE_SEQUENCE_ADAPTER_HPP
