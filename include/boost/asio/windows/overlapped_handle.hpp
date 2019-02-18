//
// windows/overlapped_handle.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_WINDOWS_OVERLAPPED_HANDLE_HPP
#define BOOST_ASIO_WINDOWS_OVERLAPPED_HANDLE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_WINDOWS_RANDOM_ACCESS_HANDLE) \
  || defined(BOOST_ASIO_HAS_WINDOWS_STREAM_HANDLE) \
  || defined(GENERATING_DOCUMENTATION)

#include <cstddef>
#include <boost/asio/async_result.hpp>
#include <boost/asio/detail/io_object_impl.hpp>
#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/detail/win_iocp_handle_service.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>

#if defined(BOOST_ASIO_HAS_MOVE)
# include <utility>
#endif // defined(BOOST_ASIO_HAS_MOVE)

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace windows {

/// Provides Windows handle functionality for objects that support
/// overlapped I/O.
/**
 * The windows::overlapped_handle class provides the ability to wrap a Windows
 * handle. The underlying object referred to by the handle must support
 * overlapped I/O.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
class overlapped_handle
{
public:
  /// The type of the executor associated with the object.
  typedef io_context::executor_type executor_type;

  /// The native representation of a handle.
#if defined(GENERATING_DOCUMENTATION)
  typedef implementation_defined native_handle_type;
#else
  typedef boost::asio::detail::win_iocp_handle_service::native_handle_type
    native_handle_type;
#endif

  /// An overlapped_handle is always the lowest layer.
  typedef overlapped_handle lowest_layer_type;

  /// Construct an overlapped_handle without opening it.
  /**
   * This constructor creates a handle without opening it.
   *
   * @param io_context The io_context object that the handle will use to
   * dispatch handlers for any asynchronous operations performed on the handle.
   */
  explicit overlapped_handle(boost::asio::io_context& io_context)
    : impl_(io_context)
  {
  }

  /// Construct an overlapped_handle on an existing native handle.
  /**
   * This constructor creates a handle object to hold an existing native handle.
   *
   * @param io_context The io_context object that the handle will use to
   * dispatch handlers for any asynchronous operations performed on the handle.
   *
   * @param handle A native handle.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  overlapped_handle(boost::asio::io_context& io_context,
      const native_handle_type& handle)
    : impl_(io_context)
  {
    boost::system::error_code ec;
    impl_.get_service().assign(impl_.get_implementation(), handle, ec);
    boost::asio::detail::throw_error(ec, "assign");
  }

#if defined(BOOST_ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move-construct an overlapped_handle from another.
  /**
   * This constructor moves a handle from one object to another.
   *
   * @param other The other overlapped_handle object from which the move will
   * occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c overlapped_handle(io_context&) constructor.
   */
  overlapped_handle(overlapped_handle&& other)
    : impl_(std::move(other.impl_))
  {
  }

  /// Move-assign an overlapped_handle from another.
  /**
   * This assignment operator moves a handle from one object to another.
   *
   * @param other The other overlapped_handle object from which the move will
   * occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c overlapped_handle(io_context&) constructor.
   */
  overlapped_handle& operator=(overlapped_handle&& other)
  {
    impl_ = std::move(other.impl_);
    return *this;
  }
#endif // defined(BOOST_ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

#if !defined(BOOST_ASIO_NO_DEPRECATED)
  /// (Deprecated: Use get_executor().) Get the io_context associated with the
  /// object.
  /**
   * This function may be used to obtain the io_context object that the I/O
   * object uses to dispatch handlers for asynchronous operations.
   *
   * @return A reference to the io_context object that the I/O object will use
   * to dispatch handlers. Ownership is not transferred to the caller.
   */
  boost::asio::io_context& get_io_context()
  {
    return impl_.get_io_context();
  }

  /// (Deprecated: Use get_executor().) Get the io_context associated with the
  /// object.
  /**
   * This function may be used to obtain the io_context object that the I/O
   * object uses to dispatch handlers for asynchronous operations.
   *
   * @return A reference to the io_context object that the I/O object will use
   * to dispatch handlers. Ownership is not transferred to the caller.
   */
  boost::asio::io_context& get_io_service()
  {
    return impl_.get_io_service();
  }
#endif // !defined(BOOST_ASIO_NO_DEPRECATED)

  /// Get the executor associated with the object.
  executor_type get_executor() BOOST_ASIO_NOEXCEPT
  {
    return impl_.get_executor();
  }

  /// Get a reference to the lowest layer.
  /**
   * This function returns a reference to the lowest layer in a stack of
   * layers. Since an overlapped_handle cannot contain any further layers, it
   * simply returns a reference to itself.
   *
   * @return A reference to the lowest layer in the stack of layers. Ownership
   * is not transferred to the caller.
   */
  lowest_layer_type& lowest_layer()
  {
    return *this;
  }

  /// Get a const reference to the lowest layer.
  /**
   * This function returns a const reference to the lowest layer in a stack of
   * layers. Since an overlapped_handle cannot contain any further layers, it
   * simply returns a reference to itself.
   *
   * @return A const reference to the lowest layer in the stack of layers.
   * Ownership is not transferred to the caller.
   */
  const lowest_layer_type& lowest_layer() const
  {
    return *this;
  }

  /// Assign an existing native handle to the handle.
  /*
   * This function opens the handle to hold an existing native handle.
   *
   * @param handle A native handle.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void assign(const native_handle_type& handle)
  {
    boost::system::error_code ec;
    impl_.get_service().assign(impl_.get_implementation(), handle, ec);
    boost::asio::detail::throw_error(ec, "assign");
  }

  /// Assign an existing native handle to the handle.
  /*
   * This function opens the handle to hold an existing native handle.
   *
   * @param handle A native handle.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  BOOST_ASIO_SYNC_OP_VOID assign(const native_handle_type& handle,
      boost::system::error_code& ec)
  {
    impl_.get_service().assign(impl_.get_implementation(), handle, ec);
    BOOST_ASIO_SYNC_OP_VOID_RETURN(ec);
  }

  /// Determine whether the handle is open.
  bool is_open() const
  {
    return impl_.get_service().is_open(impl_.get_implementation());
  }

  /// Close the handle.
  /**
   * This function is used to close the handle. Any asynchronous read or write
   * operations will be cancelled immediately, and will complete with the
   * boost::asio::error::operation_aborted error.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void close()
  {
    boost::system::error_code ec;
    impl_.get_service().close(impl_.get_implementation(), ec);
    boost::asio::detail::throw_error(ec, "close");
  }

  /// Close the handle.
  /**
   * This function is used to close the handle. Any asynchronous read or write
   * operations will be cancelled immediately, and will complete with the
   * boost::asio::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  BOOST_ASIO_SYNC_OP_VOID close(boost::system::error_code& ec)
  {
    impl_.get_service().close(impl_.get_implementation(), ec);
    BOOST_ASIO_SYNC_OP_VOID_RETURN(ec);
  }

  /// Get the native handle representation.
  /**
   * This function may be used to obtain the underlying representation of the
   * handle. This is intended to allow access to native handle functionality
   * that is not otherwise provided.
   */
  native_handle_type native_handle()
  {
    return impl_.get_service().native_handle(impl_.get_implementation());
  }

  /// Cancel all asynchronous operations associated with the handle.
  /**
   * This function causes all outstanding asynchronous read or write operations
   * to finish immediately, and the handlers for cancelled operations will be
   * passed the boost::asio::error::operation_aborted error.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void cancel()
  {
    boost::system::error_code ec;
    impl_.get_service().cancel(impl_.get_implementation(), ec);
    boost::asio::detail::throw_error(ec, "cancel");
  }

  /// Cancel all asynchronous operations associated with the handle.
  /**
   * This function causes all outstanding asynchronous read or write operations
   * to finish immediately, and the handlers for cancelled operations will be
   * passed the boost::asio::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  BOOST_ASIO_SYNC_OP_VOID cancel(boost::system::error_code& ec)
  {
    impl_.get_service().cancel(impl_.get_implementation(), ec);
    BOOST_ASIO_SYNC_OP_VOID_RETURN(ec);
  }

protected:
  /// Protected destructor to prevent deletion through this type.
  /**
   * This function destroys the handle, cancelling any outstanding asynchronous
   * wait operations associated with the handle as if by calling @c cancel.
   */
  ~overlapped_handle()
  {
  }

  boost::asio::detail::io_object_impl<
    boost::asio::detail::win_iocp_handle_service> impl_;

private:
  // Disallow copying and assignment.
  overlapped_handle(const overlapped_handle&) BOOST_ASIO_DELETED;
  overlapped_handle& operator=(const overlapped_handle&) BOOST_ASIO_DELETED;
};

} // namespace windows
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_WINDOWS_RANDOM_ACCESS_HANDLE)
       //   || defined(BOOST_ASIO_HAS_WINDOWS_STREAM_HANDLE)
       //   || defined(GENERATING_DOCUMENTATION)

#endif // BOOST_ASIO_WINDOWS_OVERLAPPED_HANDLE_HPP
