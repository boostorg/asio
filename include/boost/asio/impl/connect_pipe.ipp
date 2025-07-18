//
// impl/connect_pipe.ipp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2021 Klemens D. Morgenstern
//                    (klemens dot morgenstern at gmx dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IMPL_CONNECT_PIPE_IPP
#define BOOST_ASIO_IMPL_CONNECT_PIPE_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_PIPE)

#include <boost/asio/connect_pipe.hpp>

#if defined(BOOST_ASIO_HAS_IOCP)
# include <cstdio>
# if _WIN32_WINNT >= 0x601
#  include <bcrypt.h>
#  if !defined(BOOST_ASIO_NO_DEFAULT_LINKED_LIBS)
#   if defined(_MSC_VER)
#    pragma comment(lib, "bcrypt.lib")
#   endif // defined(_MSC_VER)
#  endif // !defined(BOOST_ASIO_NO_DEFAULT_LINKED_LIBS)
# endif // _WIN32_WINNT >= 0x601
#else // defined(BOOST_ASIO_HAS_IOCP)
# include <boost/asio/detail/descriptor_ops.hpp>
#endif // defined(BOOST_ASIO_HAS_IOCP)

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

void create_pipe(native_pipe_handle p[2], boost::system::error_code& ec)
{
#if defined(BOOST_ASIO_HAS_IOCP)
  using namespace std; // For sprintf and memcmp.

  static long counter1 = 0;
  static long counter2 = 0;

  long n1 = ::InterlockedIncrement(&counter1);
  long n2 = (static_cast<unsigned long>(n1) % 0x10000000) == 0
    ? ::InterlockedIncrement(&counter2)
    : ::InterlockedExchangeAdd(&counter2, 0);

  wchar_t pipe_name[128];
#if defined(BOOST_ASIO_HAS_SECURE_RTL)
  swprintf_s(
#else // defined(BOOST_ASIO_HAS_SECURE_RTL)
  _snwprintf(
#endif // defined(BOOST_ASIO_HAS_SECURE_RTL)
      pipe_name, 128,
      // Include address of static to discriminate asio instances in DLLs.
      L"\\\\.\\pipe\\asio-A0812896-741A-484D-AF23-BE51BF620E22-%u-%p-%ld-%ld",
      static_cast<unsigned int>(::GetCurrentProcessId()), &counter1, n1, n2);

  p[0] = ::CreateNamedPipeW(pipe_name,
      PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
      0, 1, 8192, 8192, 0, 0);

  if (p[0] == INVALID_HANDLE_VALUE)
  {
    DWORD last_error = ::GetLastError();
    ec.assign(last_error, boost::asio::error::get_system_category());
    return;
  }

  p[1] = ::CreateFileW(pipe_name, GENERIC_WRITE, 0,
    0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

  if (p[1] == INVALID_HANDLE_VALUE)
  {
    DWORD last_error = ::GetLastError();
    ::CloseHandle(p[0]);
    ec.assign(last_error, boost::asio::error::get_system_category());
    return;
  }

# if _WIN32_WINNT >= 0x601
  unsigned char nonce[16];
  if (::BCryptGenRandom(0, nonce, sizeof(nonce),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0)
  {
    ec = boost::asio::error::connection_aborted;
    ::CloseHandle(p[0]);
    ::CloseHandle(p[1]);
    return;
  }

  DWORD bytes_written = 0;
  BOOL ok = ::WriteFile(p[1], nonce, sizeof(nonce), &bytes_written, 0);
  if (!ok || bytes_written != sizeof(nonce))
  {
    ec = boost::asio::error::connection_aborted;
    ::CloseHandle(p[0]);
    ::CloseHandle(p[1]);
    return;
  }

  unsigned char nonce_check[sizeof(nonce)];
  DWORD bytes_read = 0;
  ok = ::ReadFile(p[0], nonce_check, sizeof(nonce), &bytes_read, 0);
  if (!ok || bytes_read != sizeof(nonce)
      || memcmp(nonce, nonce_check, sizeof(nonce)) != 0)
  {
    ec = boost::asio::error::connection_aborted;
    ::CloseHandle(p[0]);
    ::CloseHandle(p[1]);
    return;
  }
#endif // _WIN32_WINNT >= 0x601

  boost::asio::error::clear(ec);
#else // defined(BOOST_ASIO_HAS_IOCP)
  int result = ::pipe(p);
  detail::descriptor_ops::get_last_error(ec, result != 0);
#endif // defined(BOOST_ASIO_HAS_IOCP)
}

void close_pipe(native_pipe_handle p)
{
#if defined(BOOST_ASIO_HAS_IOCP)
  ::CloseHandle(p);
#else // defined(BOOST_ASIO_HAS_IOCP)
  boost::system::error_code ignored_ec;
  detail::descriptor_ops::state_type state = 0;
  detail::descriptor_ops::close(p, state, ignored_ec);
#endif // defined(BOOST_ASIO_HAS_IOCP)
}

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_HAS_PIPE)

#endif // BOOST_ASIO_IMPL_CONNECT_PIPE_IPP
