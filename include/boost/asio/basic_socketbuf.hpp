//
// basic_socketbuf.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_BASIC_SOCKETBUF_HPP
#define BOOST_ASIO_BASIC_SOCKETBUF_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <streambuf>
#include <boost/array.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/utility/base_from_member.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/basic_socket.hpp>
#include <boost/asio/error_handler.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/stream_socket_service.hpp>

#if !defined(BOOST_ASIO_SOCKETBUF_MAX_ARITY)
#define BOOST_ASIO_SOCKETBUF_MAX_ARITY 5
#endif // !defined(BOOST_ASIO_SOCKETBUF_MAX_ARITY)

// A macro that should expand to:
//   template < typename T1, ..., typename Tn >
//   explicit basic_socketbuf( T1 x1, ..., Tn xn )
//     : basic_socket<Protocol, Service>(
//         boost::base_from_member<io_service>::member)
//   {
//     init_buffers();
//     typedef typename Protocol::resolver_query resolver_query;
//     resolver_query query( x1, ..., xn );
//     resolve_and_connect(query);
//   }
// This macro should only persist within this file.

#define BOOST_ASIO_PRIVATE_CTR_DEF( z, n, data ) \
  template < BOOST_PP_ENUM_PARAMS(n, typename T) > \
  explicit basic_socketbuf( BOOST_PP_ENUM_BINARY_PARAMS(n, T, x) ) \
    : basic_socket<Protocol, Service>( \
        boost::base_from_member<io_service>::member) \
  { \
    init_buffers(); \
    typedef typename Protocol::resolver_query resolver_query; \
    resolver_query query( BOOST_PP_ENUM_PARAMS(n, x) ); \
    resolve_and_connect(query); \
  } \
  /**/

// A macro that should expand to:
//   template < typename T1, ..., typename Tn >
//   void connect( T1 x1, ..., Tn xn )
//   {
//     this->basic_socket<Protocol, Service>::close();
//     init_buffers();
//     typedef typename Protocol::resolver_query resolver_query;
//     resolver_query query( x1, ..., xn );
//     resolve_and_connect(query);
//   }
// This macro should only persist within this file.

#define BOOST_ASIO_PRIVATE_CONNECT_DEF( z, n, data ) \
  template < BOOST_PP_ENUM_PARAMS(n, typename T) > \
  void connect( BOOST_PP_ENUM_BINARY_PARAMS(n, T, x) ) \
  { \
    this->basic_socket<Protocol, Service>::close(); \
    init_buffers(); \
    typedef typename Protocol::resolver_query resolver_query; \
    resolver_query query( BOOST_PP_ENUM_PARAMS(n, x) ); \
    resolve_and_connect(query); \
  } \
  /**/

namespace boost {
namespace asio {

/// Iostream streambuf for a socket.
template <typename Protocol,
    typename Service = stream_socket_service<Protocol> >
class basic_socketbuf
  : public std::streambuf,
    private boost::base_from_member<io_service>,
    public basic_socket<Protocol, Service>
{
public:
  /// The endpoint type.
  typedef typename Protocol::endpoint endpoint_type;

  /// Construct a basic_socketbuf without establishing a connection.
  basic_socketbuf()
    : basic_socket<Protocol, Service>(
        boost::base_from_member<boost::asio::io_service>::member)
  {
    init_buffers();
  }

  /// Establish a connection to the specified endpoint.
  explicit basic_socketbuf(const endpoint_type& endpoint)
    : basic_socket<Protocol, Service>(
        boost::base_from_member<boost::asio::io_service>::member)
  {
    init_buffers();
    this->basic_socket<Protocol, Service>::connect(endpoint);
  }

#if defined(GENERATING_DOCUMENTATION)
  /// Establish a connection to an endpoint corresponding to a resolver query.
  /**
   * This constructor automatically establishes a connection based on the
   * supplied resolver query parameters. The arguments are used to construct
   * a resolver query object.
   */
  template <typename T1, ..., typename TN>
  explicit basic_socketbuf(T1 t1, ..., TN tn);
#else
  BOOST_PP_REPEAT_FROM_TO(
      1, BOOST_PP_INC(BOOST_ASIO_SOCKETBUF_MAX_ARITY),
      BOOST_ASIO_PRIVATE_CTR_DEF, _ )
#endif

  /// Destructor flushes buffered data.
  ~basic_socketbuf()
  {
    sync();
  }

  /// Establish a connection to the specified endpoint.
  void connect(const endpoint_type& endpoint)
  {
    this->basic_socket<Protocol, Service>::close();
    init_buffers();
    this->basic_socket<Protocol, Service>::connect(endpoint);
  }

#if defined(GENERATING_DOCUMENTATION)
  /// Establish a connection to an endpoint corresponding to a resolver query.
  /**
   * This function automatically establishes a connection based on the supplied
   * resolver query parameters. The arguments are used to construct a resolver
   * query object.
   */
  template <typename T1, ..., typename TN>
  void connect(T1 t1, ..., TN tn);
#else
  BOOST_PP_REPEAT_FROM_TO(
      1, BOOST_PP_INC(BOOST_ASIO_SOCKETBUF_MAX_ARITY),
      BOOST_ASIO_PRIVATE_CONNECT_DEF, _ )
#endif

  /// Close the connection.
  void close()
  {
    sync();
    this->basic_socket<Protocol, Service>::close();
    init_buffers();
  }

protected:
  int_type underflow()
  {
    if (gptr() == egptr())
    {
      boost::asio::error error;
      std::size_t bytes_transferred = this->service.receive(
          this->implementation,
          boost::asio::buffer(boost::asio::buffer(get_buffer_) + putback_max),
          0, boost::asio::assign_error(error));
      if (error)
      {
        if (error != boost::asio::error::eof)
          throw error;
        return traits_type::eof();
      }
      setg(get_buffer_.begin(), get_buffer_.begin() + putback_max,
          get_buffer_.begin() + putback_max + bytes_transferred);
      return traits_type::to_int_type(*gptr());
    }
    else
    {
      return traits_type::eof();
    }
  }

  int_type overflow(int_type c)
  {
    if (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      if (pptr() == epptr())
      {
        boost::asio::const_buffer buffer =
          boost::asio::buffer(pbase(), pptr() - pbase());
        while (boost::asio::buffer_size(buffer) > 0)
        {
          std::size_t bytes_transferred = this->service.send(
              this->implementation, boost::asio::buffer(buffer),
              0, boost::asio::throw_error());
          buffer = buffer + bytes_transferred;
        }
        setp(put_buffer_.begin(), put_buffer_.end());
      }

      *pptr() = traits_type::to_char_type(c);
      pbump(1);
      return c;
    }

    return traits_type::not_eof(c);
  }

  int sync()
  {
    boost::asio::const_buffer buffer =
      boost::asio::buffer(pbase(), pptr() - pbase());
    while (boost::asio::buffer_size(buffer) > 0)
    {
      std::size_t bytes_transferred = this->service.send(
          this->implementation, boost::asio::buffer(buffer),
          0, boost::asio::throw_error());
      buffer = buffer + bytes_transferred;
    }
    setp(put_buffer_.begin(), put_buffer_.end());
    return 0;
  }

private:
  void init_buffers()
  {
    setg(get_buffer_.begin(),
        get_buffer_.begin() + putback_max,
        get_buffer_.begin() + putback_max);
    setp(put_buffer_.begin(), put_buffer_.end());
  }

  void resolve_and_connect(const typename Protocol::resolver_query& query)
  {
    typedef typename Protocol::resolver resolver_type;
    typedef typename Protocol::resolver_iterator iterator_type;
    resolver_type resolver(
        boost::base_from_member<boost::asio::io_service>::member);
    iterator_type iterator = resolver.resolve(query);
    boost::asio::error error(boost::asio::error::host_not_found);
    while (error && iterator != iterator_type())
    {
      this->basic_socket<Protocol, Service>::close();
      this->basic_socket<Protocol, Service>::connect(
          *iterator, boost::asio::assign_error(error));
      ++iterator;
    }
    if (error)
      throw error;
  }

  enum { putback_max = 8 };
  enum { buffer_size = 512 };
  boost::array<char, buffer_size> get_buffer_;
  boost::array<char, buffer_size> put_buffer_;
};

} // namespace asio
} // namespace boost

#undef BOOST_ASIO_PRIVATE_CTR_DEF
#undef BOOST_ASIO_PRIVATE_CONNECT_DEF

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_BASIC_SOCKETBUF_HPP
