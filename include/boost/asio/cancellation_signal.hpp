//
// cancellation_signal.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_CANCELLATION_SIGNAL_HPP
#define BOOST_ASIO_CANCELLATION_SIGNAL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <cassert>
#include <new>
#include <utility>
#include <boost/asio/cancellation_type.hpp>
#include <boost/asio/detail/cstddef.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

class cancellation_handler_base
{
public:
  virtual void call(cancellation_type_t) = 0;
  virtual std::pair<void*, std::size_t> destroy() BOOST_ASIO_NOEXCEPT = 0;

protected:
  ~cancellation_handler_base() {}
};

template <typename Handler>
class cancellation_handler
  : public cancellation_handler_base
{
public:
#if defined(BOOST_ASIO_HAS_VARIADIC_TEMPLATES)
  template <typename... Args>
  cancellation_handler(std::size_t size, BOOST_ASIO_MOVE_ARG(Args)... args)
    : handler_(BOOST_ASIO_MOVE_CAST(Args)(args)...),
      size_(size)
  {
  }
#else // defined(BOOST_ASIO_HAS_VARIADIC_TEMPLATES)
  cancellation_handler(std::size_t size)
    : handler_(),
      size_(size)
  {
  }

#define BOOST_ASIO_PRIVATE_HANDLER_CTOR_DEF(n) \
  template <BOOST_ASIO_VARIADIC_TPARAMS(n)> \
  cancellation_handler(std::size_t size, BOOST_ASIO_VARIADIC_MOVE_PARAMS(n)) \
    : handler_(BOOST_ASIO_VARIADIC_MOVE_ARGS(n)), \
      size_(size) \
  { \
  } \
  /**/
  BOOST_ASIO_VARIADIC_GENERATE(BOOST_ASIO_PRIVATE_HANDLER_CTOR_DEF)
#undef BOOST_ASIO_PRIVATE_HANDLER_CTOR_DEF
#endif // defined(BOOST_ASIO_HAS_VARIADIC_TEMPLATES)

  void call(cancellation_type_t type)
  {
    handler_(type);
  }

  std::pair<void*, std::size_t> destroy() BOOST_ASIO_NOEXCEPT
  {
    std::pair<void*, std::size_t> mem(this, size_);
    this->cancellation_handler::~cancellation_handler();
    return mem;
  }

  Handler& handler() BOOST_ASIO_NOEXCEPT
  {
    return handler_;
  }

private:
  ~cancellation_handler()
  {
  }

  Handler handler_;
  std::size_t size_;
};

} // namespace detail

class cancellation_slot;

/// A cancellation signal with a single slot.
class cancellation_signal
{
public:
  cancellation_signal()
    : handler_(0)
  {
  }

  ~cancellation_signal()
  {
    if (handler_)
      ::operator delete(handler_->destroy().first);
  }

  /// Emits the signal and causes invocation of the slot's handler, if any.
  void emit(cancellation_type_t type)
  {
    if (handler_)
      handler_->call(type);
  }

  /// Returns the single slot associated with the signal.
  /**
   * The signal object must remain valid for as long the slot may be used.
   * Destruction of the signal invalidates the slot.
   */
  cancellation_slot slot();

private:
  cancellation_signal(const cancellation_signal&) BOOST_ASIO_DELETED;
  cancellation_signal& operator=(const cancellation_signal&) BOOST_ASIO_DELETED;

  detail::cancellation_handler_base* handler_;
};

/// A slot associated with a cancellation signal.
class cancellation_slot
{
public:
  /// Creates a slot that is not connected to any cancellation signal.
  BOOST_ASIO_CONSTEXPR cancellation_slot()
    : handler_(0)
  {
  }

#if defined(BOOST_ASIO_HAS_VARIADIC_TEMPLATES) \
  || defined(GENERATING_DOCUMENTATION)
  /// Installs a handler into the slot.
  /**
   * Destroys any existing handler slot, then installs the new handler,
   * constructing it with the supplied @c args.
   *
   * The handler is a function object to be called when the signal is emitted.
   * The signature of the handler must be @code void handler(); @endcode
   *
   * @param args Arguments to be passed to the @c Handler object's constructor.
   *
   * @returns A reference to the newly installed handler.
   */
  template <typename Handler, typename... Args>
  Handler& emplace(BOOST_ASIO_MOVE_ARG(Args)... args)
  {
    typedef detail::cancellation_handler<Handler> cancellation_handler_type;
    std::pair<void*, std::size_t> mem =
      prepare_memory(sizeof(cancellation_handler_type));
    auto_delete del = { mem.first };
    cancellation_handler_type* handler_obj =
      new (del.p) cancellation_handler_type(
        mem.second, BOOST_ASIO_MOVE_CAST(Args)(args)...);
    del.p = 0;
    *handler_ = handler_obj;
    return handler_obj->handler();
  }
#else // defined(BOOST_ASIO_HAS_VARIADIC_TEMPLATES)
      //   || defined(GENERATING_DOCUMENTATION)
  template <typename Handler>
  Handler& emplace()
  {
    typedef detail::cancellation_handler<Handler> cancellation_handler_type;
    std::pair<void*, std::size_t> mem =
      prepare_memory(sizeof(cancellation_handler_type));
    auto_delete del = { mem.first };
    cancellation_handler_type* handler_obj =
      new (del.p) cancellation_handler_type(mem.second);
    del.p = 0;
    *handler_ = handler_obj;
    return handler_obj->handler();
  }

#define BOOST_ASIO_PRIVATE_HANDLER_EMPLACE_DEF(n) \
  template <typename Handler, BOOST_ASIO_VARIADIC_TPARAMS(n)> \
  Handler& emplace(BOOST_ASIO_VARIADIC_MOVE_PARAMS(n)) \
  { \
    typedef detail::cancellation_handler<Handler> cancellation_handler_type; \
    std::pair<void*, std::size_t> mem = \
      prepare_memory(sizeof(cancellation_handler_type)); \
    auto_delete del = { mem.first }; \
    cancellation_handler_type* handler_obj = \
      new (del.p) cancellation_handler_type( \
        mem.second, BOOST_ASIO_VARIADIC_MOVE_ARGS(n)); \
    del.p = 0; \
    *handler_ = handler_obj; \
    return handler_obj->handler(); \
  } \
  /**/
  BOOST_ASIO_VARIADIC_GENERATE(BOOST_ASIO_PRIVATE_HANDLER_EMPLACE_DEF)
#undef BOOST_ASIO_PRIVATE_HANDLER_EMPLACE_DEF
#endif // defined(BOOST_ASIO_HAS_VARIADIC_TEMPLATES)

  /// Clears the slot.
  /**
   * Destroys any existing handler in the slot.
   */
  void clear()
  {
    if (handler_ != 0 && *handler_ != 0)
    {
      ::operator delete((*handler_)->destroy().first);
      *handler_ = 0;
    }
  }

  /// Returns whether the slot is connected to a signal.
  BOOST_ASIO_CONSTEXPR bool is_connected() const BOOST_ASIO_NOEXCEPT
  {
    return handler_ != 0;
  }

  /// Returns whether the slow is connected and has an installed handler.
  BOOST_ASIO_CONSTEXPR bool has_handler() const BOOST_ASIO_NOEXCEPT
  {
    return handler_ != 0 && *handler_ != 0;
  }

  /// Compare two slots for equality.
  friend BOOST_ASIO_CONSTEXPR bool operator==(const cancellation_slot& lhs,
      const cancellation_slot& rhs) BOOST_ASIO_NOEXCEPT
  {
    return lhs.handler_ == rhs.handler_;
  }

  /// Compare two slots for inequality.
  friend BOOST_ASIO_CONSTEXPR bool operator!=(const cancellation_slot& lhs,
      const cancellation_slot& rhs) BOOST_ASIO_NOEXCEPT
  {
    return lhs.handler_ != rhs.handler_;
  }

private:
  friend class cancellation_signal;

  BOOST_ASIO_CONSTEXPR cancellation_slot(int,
      detail::cancellation_handler_base** handler)
    : handler_(handler)
  {
  }

  std::pair<void*, std::size_t> prepare_memory(std::size_t size)
  {
    assert(handler_);
    std::pair<void*, std::size_t> mem;
    if (*handler_)
    {
      mem = (*handler_)->destroy();
      *handler_ = 0;
    }
    if (size > mem.second)
    {
      ::operator delete(mem.first);
      mem.first = ::operator new(size);
      mem.second = size;
    }
    return mem;
  }

  struct auto_delete
  {
    void* p;

    ~auto_delete()
    {
      if (p)
        ::operator delete(p);
    }
  };

  detail::cancellation_handler_base** handler_;
};

inline cancellation_slot cancellation_signal::slot()
{
  return cancellation_slot(0, &handler_);
}

/// Default cancellation signal propagation filter.
template <cancellation_type_t Mask>
struct cancellation_filter
{
  cancellation_type_t operator()(
      cancellation_type_t type) const BOOST_ASIO_NOEXCEPT
  {
    return type & Mask;
  }
};

/// A cancellation state is used for chaining signals and slots in compositions.
class cancellation_state
{
public:
  /// Construct and attach to a parent slot to create a new child slot.
  template <typename CancellationSlot>
  explicit cancellation_state(CancellationSlot slot)
    : impl_(slot.is_connected() ? &slot.template emplace<impl<> >() : 0)
  {
  }

  /// Construct and attach to a parent slot to create a new child slot.
  template <typename CancellationSlot, typename FilterIn, typename FilterOut>
  cancellation_state(CancellationSlot slot,
      FilterIn filter_in, FilterOut filter_out)
    : impl_(slot.is_connected()
        ? &slot.template emplace<impl<FilterIn, FilterOut> >(
            BOOST_ASIO_MOVE_CAST(FilterIn)(filter_in),
            BOOST_ASIO_MOVE_CAST(FilterOut)(filter_out))
        : 0)
  {
  }

  /// Returns the single child slot associated with the state.
  /**
   * This sub-slot is used with the operations that are being composed.
   */
  cancellation_slot slot() const BOOST_ASIO_NOEXCEPT
  {
    return impl_ ? impl_->signal_.slot() : cancellation_slot();
  }

  /// Returns whether specified cancellation types have been triggered.
  cancellation_type_t cancelled() const BOOST_ASIO_NOEXCEPT
  {
    return impl_ ? impl_->cancelled_ : cancellation_type_t();
  }

private:
  struct impl_base
  {
    impl_base()
      : cancelled_()
    {
    }

    cancellation_signal signal_;
    cancellation_type_t cancelled_;
  };

  typedef cancellation_filter<
    static_cast<cancellation_type_t>(
      static_cast<unsigned int>(cancellation_type::terminal)
        | static_cast<unsigned int>(cancellation_type::interrupt))>
    default_filter;

  template <
      typename FilterIn = default_filter,
      typename FilterOut = default_filter>
  struct impl : impl_base
  {
    impl()
      : filter_in_(),
        filter_out_()
    {
    }

    impl(FilterIn filter_in, FilterOut filter_out)
      : filter_in_(BOOST_ASIO_MOVE_CAST(FilterIn)(filter_in)),
        filter_out_(BOOST_ASIO_MOVE_CAST(FilterOut)(filter_out))
    {
    }

    void operator()(cancellation_type_t type)
    {
      this->cancelled_ = filter_in_(type);
      cancellation_type_t propagate = filter_out_(this->cancelled_);
      if (propagate != cancellation_type::none)
        this->signal_.emit(propagate);
    }

    FilterIn filter_in_;
    FilterOut filter_out_;
  };

  impl_base* impl_;
};

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_CANCELLATION_SIGNAL_HPP
