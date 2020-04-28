//
// security_properties.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2019 Apple Inc.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_SECURITY_PROPERTIES_HPP
#define BOOST_ASIO_SECURITY_PROPERTIES_HPP

#include <string_view>
#include <vector>
#include <boost/asio/detail/security_properties_impl.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {

namespace detail {
template <typename> class reactive_socket_service;
class security_properties_impl;
}

class security_properties {
public:
    using certificate_chain = std::vector<std::string_view>;

    security_properties& disable_security() { impl_.disable_security(); impl_.disable_security(); return *this; }
    security_properties& set_host(std::string_view host) { impl_.set_host(host); return *this; }
    security_properties& use_private_key(std::string_view key) { impl_.use_private_key(key); return *this; }
    security_properties& use_certificates(const certificate_chain& chain) { impl_.use_certificates(chain); return *this; }

    template <typename Verifier, typename = std::enable_if_t<std::is_invocable_r_v<bool, Verifier, const certificate_chain&>>>
    security_properties& use_certificate_verifier(Verifier verifier) { impl_.use_certificate_verifier(std::forward<Verifier>(verifier)); return *this; }

    security_properties(detail::security_properties_impl& impl) : impl_(impl) { }

private:
    template <typename> friend class detail::reactive_socket_service;
    detail::security_properties_impl& impl_;
};

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_SECURITY_PROPERTIES_HPP
