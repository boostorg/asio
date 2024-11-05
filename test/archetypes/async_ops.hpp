//
// archetypes/async_ops.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ARCHETYPES_ASYNC_OPS_HPP
#define ARCHETYPES_ASYNC_OPS_HPP

#include <functional>
#include <boost/asio/associated_allocator.hpp>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/bind_allocator.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/post.hpp>

namespace archetypes {

namespace bindns = std;

struct initiate_op_0
{
  template <typename Handler>
  void operator()(Handler&& handler)
  {
    typename boost::asio::associated_allocator<Handler>::type a
      = boost::asio::get_associated_allocator(handler);

    typename boost::asio::associated_executor<Handler>::type ex
      = boost::asio::get_associated_executor(handler);

    boost::asio::post(ex,
        boost::asio::bind_allocator(a, static_cast<Handler&&>(handler)));
  }
};

template <typename CompletionToken>
auto async_op_0(CompletionToken&& token)
  -> decltype(
    boost::asio::async_initiate<CompletionToken, void()>(
      initiate_op_0(), token))
{
  return boost::asio::async_initiate<CompletionToken, void()>(
      initiate_op_0(), token);
}

struct initiate_op_ec_0
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename boost::asio::associated_allocator<Handler>::type a
      = boost::asio::get_associated_allocator(handler);

    typename boost::asio::associated_executor<Handler>::type ex
      = boost::asio::get_associated_executor(handler);

    if (ok)
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              boost::system::error_code())));
    }
    else
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              boost::system::error_code(
                boost::asio::error::operation_aborted))));
    }
  }
};

template <typename CompletionToken>
auto async_op_ec_0(bool ok, CompletionToken&& token)
  -> decltype(
    boost::asio::async_initiate<CompletionToken,
      void(boost::system::error_code)>(
        initiate_op_ec_0(), token, ok))
{
  return boost::asio::async_initiate<CompletionToken,
    void(boost::system::error_code)>(
      initiate_op_ec_0(), token, ok);
}

struct initiate_op_ex_0
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename boost::asio::associated_allocator<Handler>::type a
      = boost::asio::get_associated_allocator(handler);

    typename boost::asio::associated_executor<Handler>::type ex
      = boost::asio::get_associated_executor(handler);

    if (ok)
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::exception_ptr())));
    }
    else
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::make_exception_ptr(std::runtime_error("blah")))));
    }
  }
};

template <typename CompletionToken>
auto async_op_ex_0(bool ok, CompletionToken&& token)
  -> decltype(
    boost::asio::async_initiate<CompletionToken,
      void(std::exception_ptr)>(
        initiate_op_ex_0(), token, ok))
{
  return boost::asio::async_initiate<CompletionToken,
    void(std::exception_ptr)>(
      initiate_op_ex_0(), token, ok);
}

struct initiate_op_1
{
  template <typename Handler>
  void operator()(Handler&& handler)
  {
    typename boost::asio::associated_allocator<Handler>::type a
      = boost::asio::get_associated_allocator(handler);

    typename boost::asio::associated_executor<Handler>::type ex
      = boost::asio::get_associated_executor(handler);

    boost::asio::post(ex,
        boost::asio::bind_allocator(a,
          bindns::bind(static_cast<Handler&&>(handler), 42)));
  }
};

template <typename CompletionToken>
auto async_op_1(CompletionToken&& token)
  -> decltype(
    boost::asio::async_initiate<CompletionToken, void(int)>(
      initiate_op_1(), token))
{
  return boost::asio::async_initiate<CompletionToken, void(int)>(
      initiate_op_1(), token);
}

struct initiate_op_ec_1
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename boost::asio::associated_allocator<Handler>::type a
      = boost::asio::get_associated_allocator(handler);

    typename boost::asio::associated_executor<Handler>::type ex
      = boost::asio::get_associated_executor(handler);

    if (ok)
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              boost::system::error_code(), 42)));
    }
    else
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              boost::system::error_code(
                boost::asio::error::operation_aborted), 0)));
    }
  }
};

template <typename CompletionToken>
auto async_op_ec_1(bool ok, CompletionToken&& token)
  -> decltype(
    boost::asio::async_initiate<CompletionToken,
      void(boost::system::error_code, int)>(
        initiate_op_ec_1(), token, ok))
{
  return boost::asio::async_initiate<CompletionToken,
    void(boost::system::error_code, int)>(
      initiate_op_ec_1(), token, ok);
}

struct initiate_op_ex_1
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename boost::asio::associated_allocator<Handler>::type a
      = boost::asio::get_associated_allocator(handler);

    typename boost::asio::associated_executor<Handler>::type ex
      = boost::asio::get_associated_executor(handler);

    if (ok)
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::exception_ptr(), 42)));
    }
    else
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::make_exception_ptr(std::runtime_error("blah")), 0)));
    }
  }
};

template <typename CompletionToken>
auto async_op_ex_1(bool ok, CompletionToken&& token)
  -> decltype(
    boost::asio::async_initiate<CompletionToken,
      void(std::exception_ptr, int)>(
        initiate_op_ex_1(), token, ok))
{
  return boost::asio::async_initiate<CompletionToken,
    void(std::exception_ptr, int)>(
      initiate_op_ex_1(), token, ok);
}

struct initiate_op_2
{
  template <typename Handler>
  void operator()(Handler&& handler)
  {
    typename boost::asio::associated_allocator<Handler>::type a
      = boost::asio::get_associated_allocator(handler);

    typename boost::asio::associated_executor<Handler>::type ex
      = boost::asio::get_associated_executor(handler);

    boost::asio::post(ex,
        boost::asio::bind_allocator(a,
          bindns::bind(static_cast<Handler&&>(handler), 42, 2.0)));
  }
};

template <typename CompletionToken>
auto async_op_2(CompletionToken&& token)
  -> decltype(
    boost::asio::async_initiate<CompletionToken, void(int, double)>(
      initiate_op_2(), token))
{
  return boost::asio::async_initiate<CompletionToken, void(int, double)>(
      initiate_op_2(), token);
}

struct initiate_op_ec_2
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename boost::asio::associated_allocator<Handler>::type a
      = boost::asio::get_associated_allocator(handler);

    typename boost::asio::associated_executor<Handler>::type ex
      = boost::asio::get_associated_executor(handler);

    if (ok)
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              boost::system::error_code(), 42, 2.0)));
    }
    else
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              boost::system::error_code(boost::asio::error::operation_aborted),
              0, 0.0)));
    }
  }
};

template <typename CompletionToken>
auto async_op_ec_2(bool ok, CompletionToken&& token)
  -> decltype(
    boost::asio::async_initiate<CompletionToken,
      void(boost::system::error_code, int, double)>(
        initiate_op_ec_2(), token, ok))
{
  return boost::asio::async_initiate<CompletionToken,
    void(boost::system::error_code, int, double)>(
      initiate_op_ec_2(), token, ok);
}

struct initiate_op_ex_2
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename boost::asio::associated_allocator<Handler>::type a
      = boost::asio::get_associated_allocator(handler);

    typename boost::asio::associated_executor<Handler>::type ex
      = boost::asio::get_associated_executor(handler);

    if (ok)
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::exception_ptr(), 42, 2.0)));
    }
    else
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::make_exception_ptr(std::runtime_error("blah")), 0, 0.0)));
    }
  }
};

template <typename CompletionToken>
auto async_op_ex_2(bool ok, CompletionToken&& token)
  -> decltype(
    boost::asio::async_initiate<CompletionToken,
      void(std::exception_ptr, int, double)>(
        initiate_op_ex_2(), token, ok))
{
  return boost::asio::async_initiate<CompletionToken,
    void(std::exception_ptr, int, double)>(
      initiate_op_ex_2(), token, ok);
}

struct initiate_op_3
{
  template <typename Handler>
  void operator()(Handler&& handler)
  {
    typename boost::asio::associated_allocator<Handler>::type a
      = boost::asio::get_associated_allocator(handler);

    typename boost::asio::associated_executor<Handler>::type ex
      = boost::asio::get_associated_executor(handler);

    boost::asio::post(ex,
        boost::asio::bind_allocator(a,
          bindns::bind(static_cast<Handler&&>(handler), 42, 2.0, 'a')));
  }
};

template <typename CompletionToken>
auto async_op_3(CompletionToken&& token)
  -> decltype(
    boost::asio::async_initiate<CompletionToken, void(int, double, char)>(
      initiate_op_3(), token))
{
  return boost::asio::async_initiate<CompletionToken, void(int, double, char)>(
      initiate_op_3(), token);
}

struct initiate_op_ec_3
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename boost::asio::associated_allocator<Handler>::type a
      = boost::asio::get_associated_allocator(handler);

    typename boost::asio::associated_executor<Handler>::type ex
      = boost::asio::get_associated_executor(handler);

    if (ok)
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              boost::system::error_code(), 42, 2.0, 'a')));
    }
    else
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              boost::system::error_code(boost::asio::error::operation_aborted),
              0, 0.0, 'z')));
    }
  }
};

template <typename CompletionToken>
auto async_op_ec_3(bool ok, CompletionToken&& token)
  -> decltype(
    boost::asio::async_initiate<CompletionToken,
      void(boost::system::error_code, int, double, char)>(
        initiate_op_ec_3(), token, ok))
{
  return boost::asio::async_initiate<CompletionToken,
    void(boost::system::error_code, int, double, char)>(
      initiate_op_ec_3(), token, ok);
}

struct initiate_op_ex_3
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename boost::asio::associated_allocator<Handler>::type a
      = boost::asio::get_associated_allocator(handler);

    typename boost::asio::associated_executor<Handler>::type ex
      = boost::asio::get_associated_executor(handler);

    if (ok)
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::exception_ptr(), 42, 2.0, 'a')));
    }
    else
    {
      boost::asio::post(ex,
          boost::asio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::make_exception_ptr(std::runtime_error("blah")),
              0, 0.0, 'z')));
    }
  }
};

template <typename CompletionToken>
auto async_op_ex_3(bool ok, CompletionToken&& token)
  -> decltype(
    boost::asio::async_initiate<CompletionToken,
      void(std::exception_ptr, int, double, char)>(
        initiate_op_ex_3(), token, ok))
{
  return boost::asio::async_initiate<CompletionToken,
    void(std::exception_ptr, int, double, char)>(
      initiate_op_ex_3(), token, ok);
}

} // namespace archetypes

#endif // ARCHETYPES_ASYNC_OPS_HPP
