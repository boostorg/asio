//
// detail/base_from_cancellation_state.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_BASE_FROM_CANCELLATION_STATE_HPP
#define BOOST_ASIO_DETAIL_BASE_FROM_CANCELLATION_STATE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/associated_cancellation_slot.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/detail/type_traits.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename Handler, typename = void>
class base_from_cancellation_state
{
public:
  typedef cancellation_slot cancellation_slot_type;

  cancellation_slot_type get_cancellation_slot() const BOOST_ASIO_NOEXCEPT
  {
    return cancellation_state_.slot();
  }

protected:
  explicit base_from_cancellation_state(const Handler& handler)
    : cancellation_state_(
        boost::asio::get_associated_cancellation_slot(handler))
  {
  }

  template <typename FilterIn, typename FilterOut>
  base_from_cancellation_state(const Handler& handler,
      FilterIn filter_in, FilterOut filter_out)
    : cancellation_state_(
        boost::asio::get_associated_cancellation_slot(handler),
        BOOST_ASIO_MOVE_CAST(FilterIn)(filter_in),
        BOOST_ASIO_MOVE_CAST(FilterOut)(filter_out))
  {
  }

  bool cancelled() const
  {
    return cancellation_state_.cancelled() != cancellation_type_t();
  }

private:
  cancellation_state cancellation_state_;
};

template <typename Handler>
class base_from_cancellation_state<Handler,
    typename enable_if<
      is_same<
        typename associated_cancellation_slot<
          Handler, cancellation_slot
        >::asio_associated_cancellation_slot_is_unspecialised,
        void
      >::value
    >::type>
{
protected:
  explicit base_from_cancellation_state(const Handler&)
  {
  }

  template <typename FilterIn, typename FilterOut>
  base_from_cancellation_state(const Handler&, FilterIn, FilterOut)
  {
  }

  BOOST_ASIO_CONSTEXPR bool cancelled() const
  {
    return false;
  }
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_BASE_FROM_CANCELLATION_STATE_HPP
