//
// context_service.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2005 Voipster / Indrek dot Juhani at voipster dot com
// Copyright (c) 2005 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_SSL_CONTEXT_SERVICE_HPP
#define BOOST_ASIO_SSL_CONTEXT_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl/detail/openssl_context_service.hpp>

namespace boost {
namespace asio {
namespace ssl {

/// Default service implementation for a context.
class context_service
  : public boost::asio::io_service::service
{
private:
  // The type of the platform-specific implementation.
  typedef detail::openssl_context_service service_impl_type;

public:
  /// The type of the context.
#if defined(GENERATING_DOCUMENTATION)
  typedef implementation_defined impl_type;
#else
  typedef service_impl_type::impl_type impl_type;
#endif

  /// Constructor.
  explicit context_service(boost::asio::io_service& io_service)
    : boost::asio::io_service::service(io_service),
      service_impl_(boost::asio::use_service<service_impl_type>(io_service))
  {
  }

  /// Destroy all user-defined handler objects owned by the service.
  void shutdown_service()
  {
  }

  /// Return a null context implementation.
  impl_type null() const
  {
    return service_impl_.null();
  }

  /// Create a new context implementation.
  void create(impl_type& impl, context_base::method m)
  {
    service_impl_.create(impl, m);
  }

  /// Destroy a context implementation.
  void destroy(impl_type& impl)
  {
    service_impl_.destroy(impl);
  }

  /// Set options on the context.
  template <typename Error_Handler>
  void set_options(impl_type& impl, context_base::options o,
      Error_Handler error_handler)
  {
    service_impl_.set_options(impl, o, error_handler);
  }

  /// Set peer verification mode.
  template <typename Error_Handler>
  void set_verify_mode(impl_type& impl, context_base::verify_mode v,
      Error_Handler error_handler)
  {
    service_impl_.set_verify_mode(impl, v, error_handler);
  }

  /// Load a certification authority file for performing verification.
  template <typename Error_Handler>
  void load_verify_file(impl_type& impl, const std::string& filename,
      Error_Handler error_handler)
  {
    service_impl_.load_verify_file(impl, filename, error_handler);
  }

  /// Add a directory containing certification authority files to be used for
  /// performing verification.
  template <typename Error_Handler>
  void add_verify_path(impl_type& impl, const std::string& path,
      Error_Handler error_handler)
  {
    service_impl_.add_verify_path(impl, path, error_handler);
  }

  /// Use a certificate from a file.
  template <typename Error_Handler>
  void use_certificate_file(impl_type& impl, const std::string& filename,
      context_base::file_format format, Error_Handler error_handler)
  {
    service_impl_.use_certificate_file(impl, filename, format, error_handler);
  }

  /// Use a certificate chain from a file.
  template <typename Error_Handler>
  void use_certificate_chain_file(impl_type& impl, const std::string& filename,
      Error_Handler error_handler)
  {
    service_impl_.use_certificate_chain_file(impl, filename, error_handler);
  }

  /// Use a private key from a file.
  template <typename Error_Handler>
  void use_private_key_file(impl_type& impl, const std::string& filename,
      context_base::file_format format, Error_Handler error_handler)
  {
    service_impl_.use_private_key_file(impl, filename, format, error_handler);
  }

  /// Use an RSA private key from a file.
  template <typename Error_Handler>
  void use_rsa_private_key_file(impl_type& impl, const std::string& filename,
      context_base::file_format format, Error_Handler error_handler)
  {
    service_impl_.use_rsa_private_key_file(impl, filename, format,
        error_handler);
  }

  /// Use the specified file to obtain the temporary Diffie-Hellman parameters.
  template <typename Error_Handler>
  void use_tmp_dh_file(impl_type& impl, const std::string& filename,
      Error_Handler error_handler)
  {
    service_impl_.use_tmp_dh_file(impl, filename, error_handler);
  }

private:
  // The service that provides the platform-specific implementation.
  service_impl_type& service_impl_;
};

} // namespace ssl
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_SSL_CONTEXT_SERVICE_HPP
