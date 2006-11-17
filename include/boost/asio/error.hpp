//
// error.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_ERROR_HPP
#define BOOST_ASIO_ERROR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <cerrno>
#include <boost/config.hpp>
#include <boost/cerrno.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/detail/socket_types.hpp>

#if defined(GENERATING_DOCUMENTATION)
/// INTERNAL ONLY.
# define BOOST_ASIO_NATIVE_ERROR(e) implementation_defined
/// INTERNAL ONLY.
# define BOOST_ASIO_SOCKET_ERROR(e) implementation_defined
/// INTERNAL ONLY.
# define BOOST_ASIO_NETDB_ERROR(e) implementation_defined
/// INTERNAL ONLY.
# define BOOST_ASIO_GETADDRINFO_ERROR(e) implementation_defined
/// INTERNAL ONLY.
# define BOOST_ASIO_WIN_OR_POSIX(e_win, e_posix) implementation_defined
#elif defined(BOOST_WINDOWS) || defined(__CYGWIN__)
# define BOOST_ASIO_NATIVE_ERROR(e) \
    boost::system::error_code(e, boost::system::native_ecat)
# define BOOST_ASIO_SOCKET_ERROR(e) \
    boost::system::error_code(WSA ## e, boost::system::native_ecat)
# define BOOST_ASIO_NETDB_ERROR(e) \
    boost::system::error_code(WSA ## e, boost::system::native_ecat)
# define BOOST_ASIO_GETADDRINFO_ERROR(e) \
    boost::system::error_code(WSA ## e, boost::system::native_ecat)
# define BOOST_ASIO_EOF_ERROR(e) \
    boost::system::error_code(e, boost::system::native_ecat)
# define BOOST_ASIO_WIN_OR_POSIX(e_win, e_posix) e_win
#else
# define BOOST_ASIO_NATIVE_ERROR(e) \
    boost::system::error_code(e, boost::system::native_ecat)
# define BOOST_ASIO_SOCKET_ERROR(e) \
    boost::system::error_code(e, boost::system::native_ecat)
# define BOOST_ASIO_NETDB_ERROR(e) \
    boost::system::error_code(e, boost::asio::error_base<T>::netdb_ecat)
# define BOOST_ASIO_GETADDRINFO_ERROR(e) \
    boost::system::error_code(e, boost::asio::error_base<T>::addrinfo_ecat)
# define BOOST_ASIO_EOF_ERROR(e) \
    boost::system::error_code(e, boost::asio::error_base<T>::eof_ecat)
# define BOOST_ASIO_WIN_OR_POSIX(e_win, e_posix) e_posix
#endif

namespace boost {
namespace asio {

/// Hack to keep asio library header-file-only.
template <typename T>
class error_base
{
public:
#if !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)
  static boost::system::error_category netdb_ecat;
  static int netdb_ed(const boost::system::error_code& ec);
  static std::string netdb_md(const boost::system::error_code& ec);
  static boost::system::wstring_t netdb_wmd(
      const boost::system::error_code& ec);

  static boost::system::error_category addrinfo_ecat;
  static int addrinfo_ed(const boost::system::error_code& ec);
  static std::string addrinfo_md(const boost::system::error_code& ec);
  static boost::system::wstring_t addrinfo_wmd(
      const boost::system::error_code& ec);

  static boost::system::error_category eof_ecat;
  static int eof_ed(const boost::system::error_code& ec);
  static std::string eof_md(const boost::system::error_code& ec);
  static boost::system::wstring_t eof_wmd(const boost::system::error_code& ec);
#endif // !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)

  static boost::system::error_category ssl_ecat;
  static int ssl_ed(const boost::system::error_code& ec);
  static std::string ssl_md(const boost::system::error_code& ec);
  static boost::system::wstring_t ssl_wmd(const boost::system::error_code& ec);

  /// Permission denied.
  static const boost::system::error_code access_denied;

  /// Address family not supported by protocol.
  static const boost::system::error_code address_family_not_supported;

  /// Address already in use.
  static const boost::system::error_code address_in_use;

  /// Transport endpoint is already connected.
  static const boost::system::error_code already_connected;

  /// Operation already in progress.
  static const boost::system::error_code already_started;

  /// A connection has been aborted.
  static const boost::system::error_code connection_aborted;

  /// Connection refused.
  static const boost::system::error_code connection_refused;

  /// Connection reset by peer.
  static const boost::system::error_code connection_reset;

  /// Bad file descriptor.
  static const boost::system::error_code bad_descriptor;

  /// End of file or stream.
  static const boost::system::error_code eof;

  /// Bad address.
  static const boost::system::error_code fault;

  /// Host not found (authoritative).
  static const boost::system::error_code host_not_found;

  /// Host not found (non-authoritative).
  static const boost::system::error_code host_not_found_try_again;

  /// No route to host.
  static const boost::system::error_code host_unreachable;

  /// Operation now in progress.
  static const boost::system::error_code in_progress;

  /// Interrupted system call.
  static const boost::system::error_code interrupted;

  /// Invalid argument.
  static const boost::system::error_code invalid_argument;

  /// Message too long.
  static const boost::system::error_code message_size;

  /// Network is down.
  static const boost::system::error_code network_down;

  /// Network dropped connection on reset.
  static const boost::system::error_code network_reset;

  /// Network is unreachable.
  static const boost::system::error_code network_unreachable;

  /// Too many open files.
  static const boost::system::error_code no_descriptors;

  /// No buffer space available.
  static const boost::system::error_code no_buffer_space;

  /// The query is valid but does not have associated address data.
  static const boost::system::error_code no_data;

  /// Cannot allocate memory.
  static const boost::system::error_code no_memory;

  /// Operation not permitted.
  static const boost::system::error_code no_permission;

  /// Protocol not available.
  static const boost::system::error_code no_protocol_option;

  /// A non-recoverable error occurred.
  static const boost::system::error_code no_recovery;

  /// Transport endpoint is not connected.
  static const boost::system::error_code not_connected;

  /// Socket operation on non-socket.
  static const boost::system::error_code not_socket;

  /// Operation not supported.
  static const boost::system::error_code not_supported;

  /// Operation cancelled.
  static const boost::system::error_code operation_aborted;

  /// The service is not supported for the given socket type.
  static const boost::system::error_code service_not_found;

  /// The socket type is not supported.
  static const boost::system::error_code socket_type_not_supported;

  /// Cannot send after transport endpoint shutdown.
  static const boost::system::error_code shut_down;

  /// Connection timed out.
  static const boost::system::error_code timed_out;

  /// Resource temporarily unavailable.
  static const boost::system::error_code try_again;

  /// The socket is marked non-blocking and the requested operation would block.
  static const boost::system::error_code would_block;

private:
  error_base();
};

#if !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)

template <typename T>
boost::system::error_category error_base<T>::netdb_ecat(
    boost::system::error_code::new_category(&error_base<T>::netdb_ed,
      &error_base<T>::netdb_md, &error_base<T>::netdb_wmd));

template <typename T>
int error_base<T>::netdb_ed(const boost::system::error_code& ec)
{
  return EOTHER;
}

template <typename T>
std::string error_base<T>::netdb_md(const boost::system::error_code& ec)
{
  if (ec == error_base<T>::host_not_found)
    return "Host not found (authoritative)";
  if (ec == error_base<T>::host_not_found_try_again)
    return "Host not found (non-authoritative), try again later";
  if (ec == error_base<T>::no_data)
    return "The query is valid, but it does not have associated data";
  if (ec == error_base<T>::no_recovery)
    return "A non-recoverable error occurred during database lookup";
  return "EINVAL";
}

template <typename T>
boost::system::wstring_t error_base<T>::netdb_wmd(
    const boost::system::error_code& ec)
{
  if (ec == error_base<T>::host_not_found)
    return L"Host not found (authoritative)";
  if (ec == error_base<T>::host_not_found_try_again)
    return L"Host not found (non-authoritative), try again later";
  if (ec == error_base<T>::no_data)
    return L"The query is valid, but it does not have associated data";
  if (ec == error_base<T>::no_recovery)
    return L"A non-recoverable error occurred during database lookup";
  return L"EINVAL";
}

template <typename T>
boost::system::error_category error_base<T>::addrinfo_ecat(
    boost::system::error_code::new_category(&error_base<T>::addrinfo_ed,
      &error_base<T>::addrinfo_md, &error_base<T>::addrinfo_wmd));

template <typename T>
int error_base<T>::addrinfo_ed(const boost::system::error_code& ec)
{
  return EOTHER;
}

template <typename T>
std::string error_base<T>::addrinfo_md(const boost::system::error_code& ec)
{
  if (ec == error_base<T>::service_not_found)
    return "Service not found";
  if (ec == error_base<T>::socket_type_not_supported)
    return "Socket type not supported";
  return "EINVAL";
}

template <typename T>
boost::system::wstring_t error_base<T>::addrinfo_wmd(
    const boost::system::error_code& ec)
{
  if (ec == error_base<T>::service_not_found)
    return L"Service not found";
  if (ec == error_base<T>::socket_type_not_supported)
    return L"Socket type not supported";
  return L"EINVAL";
}

template <typename T>
boost::system::error_category error_base<T>::eof_ecat(
    boost::system::error_code::new_category(&error_base<T>::eof_ed,
      &error_base<T>::eof_md, &error_base<T>::eof_wmd));

template <typename T>
int error_base<T>::eof_ed(const boost::system::error_code& ec)
{
  return EOTHER;
}

template <typename T>
std::string error_base<T>::eof_md(const boost::system::error_code& ec)
{
  if (ec == error_base<T>::eof)
    return "End of file";
  return "EINVAL";
}

template <typename T>
boost::system::wstring_t error_base<T>::eof_wmd(
    const boost::system::error_code& ec)
{
  if (ec == error_base<T>::eof)
    return L"End of file";
  return L"EINVAL";
}

#endif // !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)

template <typename T>
boost::system::error_category error_base<T>::ssl_ecat(
    boost::system::error_code::new_category(&error_base<T>::ssl_ed,
      &error_base<T>::ssl_md, &error_base<T>::ssl_wmd));

template <typename T>
int error_base<T>::ssl_ed(const boost::system::error_code& ec)
{
  return EOTHER;
}

template <typename T>
std::string error_base<T>::ssl_md(const boost::system::error_code& ec)
{
  return "SSL error";
}

template <typename T>
boost::system::wstring_t error_base<T>::ssl_wmd(
    const boost::system::error_code& ec)
{
  return L"SSL error";
}

template <typename T> const boost::system::error_code
error_base<T>::access_denied = BOOST_ASIO_SOCKET_ERROR(EACCES);

template <typename T> const boost::system::error_code
error_base<T>::address_family_not_supported = BOOST_ASIO_SOCKET_ERROR(
    EAFNOSUPPORT);

template <typename T> const boost::system::error_code
error_base<T>::address_in_use = BOOST_ASIO_SOCKET_ERROR(EADDRINUSE);

template <typename T> const boost::system::error_code
error_base<T>::already_connected = BOOST_ASIO_SOCKET_ERROR(EISCONN);

template <typename T> const boost::system::error_code
error_base<T>::already_started = BOOST_ASIO_SOCKET_ERROR(EALREADY);

template <typename T> const boost::system::error_code
error_base<T>::connection_aborted = BOOST_ASIO_SOCKET_ERROR(ECONNABORTED);

template <typename T> const boost::system::error_code
error_base<T>::connection_refused = BOOST_ASIO_SOCKET_ERROR(ECONNREFUSED);

template <typename T> const boost::system::error_code
error_base<T>::connection_reset = BOOST_ASIO_SOCKET_ERROR(ECONNRESET);

template <typename T> const boost::system::error_code
error_base<T>::bad_descriptor = BOOST_ASIO_SOCKET_ERROR(EBADF);

template <typename T> const boost::system::error_code
error_base<T>::eof = BOOST_ASIO_WIN_OR_POSIX(
    BOOST_ASIO_EOF_ERROR(ERROR_HANDLE_EOF),
    BOOST_ASIO_EOF_ERROR(-1));

template <typename T> const boost::system::error_code
error_base<T>::fault = BOOST_ASIO_SOCKET_ERROR(EFAULT);

template <typename T> const boost::system::error_code
error_base<T>::host_not_found = BOOST_ASIO_NETDB_ERROR(HOST_NOT_FOUND);

template <typename T> const boost::system::error_code
error_base<T>::host_not_found_try_again = BOOST_ASIO_NETDB_ERROR(TRY_AGAIN);

template <typename T> const boost::system::error_code
error_base<T>::host_unreachable = BOOST_ASIO_SOCKET_ERROR(EHOSTUNREACH);

template <typename T> const boost::system::error_code
error_base<T>::in_progress = BOOST_ASIO_SOCKET_ERROR(EINPROGRESS);

template <typename T> const boost::system::error_code
error_base<T>::interrupted = BOOST_ASIO_SOCKET_ERROR(EINTR);

template <typename T> const boost::system::error_code
error_base<T>::invalid_argument = BOOST_ASIO_SOCKET_ERROR(EINVAL);

template <typename T> const boost::system::error_code
error_base<T>::message_size = BOOST_ASIO_SOCKET_ERROR(EMSGSIZE);

template <typename T> const boost::system::error_code
error_base<T>::network_down = BOOST_ASIO_SOCKET_ERROR(ENETDOWN);

template <typename T> const boost::system::error_code
error_base<T>::network_reset = BOOST_ASIO_SOCKET_ERROR(ENETRESET);

template <typename T> const boost::system::error_code
error_base<T>::network_unreachable = BOOST_ASIO_SOCKET_ERROR(ENETUNREACH);

template <typename T> const boost::system::error_code
error_base<T>::no_descriptors = BOOST_ASIO_SOCKET_ERROR(EMFILE);

template <typename T> const boost::system::error_code
error_base<T>::no_buffer_space = BOOST_ASIO_SOCKET_ERROR(ENOBUFS);

template <typename T> const boost::system::error_code
error_base<T>::no_data = BOOST_ASIO_NETDB_ERROR(NO_DATA);

template <typename T> const boost::system::error_code
error_base<T>::no_memory = BOOST_ASIO_WIN_OR_POSIX(
    BOOST_ASIO_NATIVE_ERROR(ERROR_OUTOFMEMORY),
    BOOST_ASIO_NATIVE_ERROR(ENOMEM));

template <typename T> const boost::system::error_code
error_base<T>::no_permission = BOOST_ASIO_WIN_OR_POSIX(
    BOOST_ASIO_NATIVE_ERROR(ERROR_ACCESS_DENIED),
    BOOST_ASIO_NATIVE_ERROR(EPERM));

template <typename T> const boost::system::error_code
error_base<T>::no_protocol_option = BOOST_ASIO_SOCKET_ERROR(ENOPROTOOPT);

template <typename T> const boost::system::error_code
error_base<T>::no_recovery = BOOST_ASIO_NETDB_ERROR(NO_RECOVERY);

template <typename T> const boost::system::error_code
error_base<T>::not_connected = BOOST_ASIO_SOCKET_ERROR(ENOTCONN);

template <typename T> const boost::system::error_code
error_base<T>::not_socket = BOOST_ASIO_SOCKET_ERROR(ENOTSOCK);

template <typename T> const boost::system::error_code
error_base<T>::not_supported = BOOST_ASIO_SOCKET_ERROR(EOPNOTSUPP);

template <typename T> const boost::system::error_code
error_base<T>::operation_aborted = BOOST_ASIO_WIN_OR_POSIX(
    BOOST_ASIO_NATIVE_ERROR(ERROR_OPERATION_ABORTED),
    BOOST_ASIO_NATIVE_ERROR(ECANCELED));

template <typename T> const boost::system::error_code
error_base<T>::service_not_found = BOOST_ASIO_WIN_OR_POSIX(
    BOOST_ASIO_NATIVE_ERROR(WSATYPE_NOT_FOUND),
    BOOST_ASIO_GETADDRINFO_ERROR(EAI_SERVICE));

template <typename T> const boost::system::error_code
error_base<T>::socket_type_not_supported = BOOST_ASIO_WIN_OR_POSIX(
    BOOST_ASIO_NATIVE_ERROR(WSAESOCKTNOSUPPORT),
    BOOST_ASIO_GETADDRINFO_ERROR(EAI_SOCKTYPE));

template <typename T> const boost::system::error_code
error_base<T>::shut_down = BOOST_ASIO_SOCKET_ERROR(ESHUTDOWN);

template <typename T> const boost::system::error_code
error_base<T>::timed_out = BOOST_ASIO_SOCKET_ERROR(ETIMEDOUT);

template <typename T> const boost::system::error_code
error_base<T>::try_again = BOOST_ASIO_WIN_OR_POSIX(
    BOOST_ASIO_NATIVE_ERROR(ERROR_RETRY),
    BOOST_ASIO_NATIVE_ERROR(EAGAIN));

template <typename T> const boost::system::error_code
error_base<T>::would_block = BOOST_ASIO_SOCKET_ERROR(EWOULDBLOCK);

/// Contains error constants.
class error : public error_base<error>
{
private:
  error();
};

} // namespace asio
} // namespace boost

#undef BOOST_ASIO_NATIVE_ERROR
#undef BOOST_ASIO_SOCKET_ERROR
#undef BOOST_ASIO_NETDB_ERROR
#undef BOOST_ASIO_GETADDRINFO_ERROR
#undef BOOST_ASIO_EOF_ERROR
#undef BOOST_ASIO_WIN_OR_POSIX


#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_ERROR_HPP
