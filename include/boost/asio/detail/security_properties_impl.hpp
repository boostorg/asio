//
// security_properties_impl.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2019 Apple Inc.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_SECURITY_PROPERTIES_IMPL_HPP
#define BOOST_ASIO_DETAIL_SECURITY_PROPERTIES_IMPL_HPP

#include <boost/asio/detail/config.hpp>

#include <functional>
#include <string_view>
#include <utility>
#include <vector>

#if defined(__APPLE__)
#include <Security/Security.h>
#endif

#include <boost/asio/detail/push_options.hpp>

extern "C" {

// These are forward declarations of functions available in BoringSSL.

enum ssl_verify_result_t {
  ssl_verify_ok,
  ssl_verify_invalid,
  ssl_verify_retry,
};

struct BIO;
struct CRYPTO_BUFFER;
struct CRYPTO_BUFFER_POOL;
struct EVP_PKEY;
struct pem_password_cb;
struct SSL* SSL_new(struct SSL_CTX*);
struct SSL;
struct SSL_CTX* SSL_CTX_new(const struct SSL_METHOD*);
struct SSL_CTX;
struct SSL_METHOD;
struct _STACK;
BIO* BIO_new_mem_buf(const void*, int);
const SSL_METHOD* TLS_with_buffers_method(void);
void SSL_set_custom_verify(SSL *ssl, int mode, enum ssl_verify_result_t (*callback)(SSL *ssl, uint8_t *out_alert));
int SSL_set_ex_data(SSL *ssl, int idx, void *data);
void *SSL_get_ex_data(const SSL *ssl, int idx);
EVP_PKEY* PEM_read_bio_PrivateKey(BIO*, EVP_PKEY**, pem_password_cb*, void*);
int SSL_accept(SSL*);
const uint8_t *CRYPTO_BUFFER_data(const CRYPTO_BUFFER *buf);
size_t CRYPTO_BUFFER_len(const CRYPTO_BUFFER *buf);
int SSL_connect(struct SSL*);
int SSL_read(struct SSL*, void*, int);
int SSL_set_fd(struct SSL*, int);
int SSL_write(struct SSL*, const void*, int);
size_t sk_num(const _STACK *sk);
SSL* SSL_new(SSL_CTX*);
SSL_CTX* SSL_CTX_new(const SSL_METHOD*);
const struct stack_st_CRYPTO_BUFFER* SSL_get0_peer_certificates(const SSL *ssl);
void *sk_value(const _STACK *sk, size_t i);
void BIO_free(BIO*);
void EVP_PKEY_free(EVP_PKEY*);
void SSL_CTX_free(struct SSL_CTX*);
void SSL_free(struct SSL*);
typedef struct ssl_private_key_method_st SSL_PRIVATE_KEY_METHOD;
int SSL_set_chain_and_key(SSL *ssl, CRYPTO_BUFFER *const *certs, size_t num_certs, EVP_PKEY *privkey, const SSL_PRIVATE_KEY_METHOD *privkey_method);
CRYPTO_BUFFER *CRYPTO_BUFFER_new(const uint8_t *data, size_t len, CRYPTO_BUFFER_POOL *pool);
void CRYPTO_BUFFER_free(CRYPTO_BUFFER *buf);

}

namespace boost {
namespace asio {
namespace detail {

template <typename> class reactive_socket_service;

#if defined(__APPLE__)
template<typename T> struct CFPtr {
    CFPtr(T pointer) : pointer_(pointer) { };
    CFPtr(CFPtr&& other) : pointer_(std::exchange(other.pointer_, nullptr)) { }
    ~CFPtr() { if (pointer_) CFRelease(pointer_); }
    T get() { return pointer_; }
    bool operator!() { return !pointer_; }
    operator bool() { return !!pointer_; }
private:
    T pointer_ { nullptr };
};
#endif

namespace ssl {
template<typename> struct deleter;
template<> struct deleter<BIO> { void operator() (BIO* bio) { BIO_free(bio); } };
template<> struct deleter<SSL> { void operator() (SSL* ssl) { SSL_free(ssl); } };
template<> struct deleter<SSL_CTX> { void operator() (SSL_CTX* ctx) { SSL_CTX_free(ctx); } };
template<> struct deleter<EVP_PKEY> { void operator() (EVP_PKEY* ctx) { EVP_PKEY_free(ctx); } };
template<> struct deleter<CRYPTO_BUFFER> { void operator() (CRYPTO_BUFFER* buffer) { CRYPTO_BUFFER_free(buffer); } };
template <typename T> using unique_ptr = std::unique_ptr<T, deleter<T>>;
}

class security_properties_impl {
public:

    void use_private_key(std::string_view key)
    {
        if (auto private_key_bio = ssl::unique_ptr<BIO>(BIO_new_mem_buf(key.data(), key.length())))
            private_key_ = ssl::unique_ptr<EVP_PKEY>(PEM_read_bio_PrivateKey(private_key_bio.get(), nullptr, nullptr, nullptr));
    }

    bool accept(int new_socket)
    {
        if (security_disabled_)
            return true;

        ssl::unique_ptr<SSL_CTX> ctx(SSL_CTX_new(TLS_with_buffers_method()));
        if (!ctx)
            return false;

        ssl_ = ssl::unique_ptr<SSL>(SSL_new(ctx.get()));
        if (!ssl_)
            return false;

        if (!private_key_ || !certificates_.size())
            return false;

        if (!SSL_set_chain_and_key(ssl_.get(), reinterpret_cast<CRYPTO_BUFFER * const *>(certificates_.data()), certificates_.size(), private_key_.get(), nullptr))
            return false;

        SSL_set_fd(ssl_.get(), new_socket);

        return SSL_accept(ssl_.get()) > 0;
    }

    bool connect(int new_socket)
    {
        if (security_disabled_)
            return true;

        // Perform SSL handshake.  If it fails, mimic ::connect's failure.
        ssl::unique_ptr<SSL_CTX> ctx(SSL_CTX_new(TLS_with_buffers_method()));
        if (!ctx)
            return false;

        ssl_ = ssl::unique_ptr<SSL>(SSL_new(ctx.get()));
        if (!ssl_)
            return false;

        const int SSL_VERIFY_PEER = 0x01;
        const int SSL_VERIFY_FAIL_IF_NO_PEER_CERT = 0x02;
        SSL_set_custom_verify(ssl_.get(), SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, &security_properties_impl::verifyCertificateChain);
        SSL_set_ex_data(ssl_.get(), 0, this);
        SSL_set_fd(ssl_.get(), new_socket);
        if (SSL_connect(ssl_.get()) != 1)
            return false;

        return true;
    }

    bool security_disabled() const { return security_disabled_; }
    SSL* ssl() const { return ssl_.get(); }
    void disable_security() { security_disabled_ = true; }

    void use_certificates(const std::vector<std::string_view>& chain)
    {
        certificates_.clear();
        certificates_.reserve(chain.size());
        for (const auto& cert : chain)
            certificates_.push_back(ssl::unique_ptr<CRYPTO_BUFFER>(CRYPTO_BUFFER_new(reinterpret_cast<const uint8_t*>(cert.data()), cert.length(), nullptr)));
    }

    void set_host(std::string_view host) { host_ = host; }

    template <typename Verifier, typename = std::enable_if_t<std::is_invocable_r_v<bool, Verifier, const std::vector<std::string_view>&>>>
    void use_certificate_verifier(Verifier verifier) { certificate_verifier_ = std::forward<Verifier>(verifier); }

private:

    static bool verifyCertificateChainWithOperatingSystem(const std::vector<std::string_view> chain, const char* host)
    {
#if defined(__APPLE__)
        std::vector<CFPtr<SecCertificateRef>> certs;
        for (const auto& cert : chain) {
            CFPtr<CFDataRef> data(CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, reinterpret_cast<const UInt8 *>(cert.data()), cert.size(), kCFAllocatorNull));
            if (!data)
                return false;
            CFPtr<SecCertificateRef> certificate(SecCertificateCreateWithData(nullptr, data.get()));
            if (!certificate)
                return false;
            certs.push_back(std::move(certificate));
        }
        CFPtr<CFArrayRef> certsArray(CFArrayCreate(kCFAllocatorDefault, (const void**)certs.data(), certs.size(), nullptr));
        if (!certsArray)
            return false;

        CFPtr<CFStringRef> hostName(CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, (const char*)host, kCFStringEncodingASCII, kCFAllocatorNull));
        if (!hostName)
            return false;

        const Boolean serverCertificates = true;
        CFPtr<SecPolicyRef> policy(SecPolicyCreateSSL(serverCertificates, hostName.get()));
        SecTrustRef trust = nullptr;
        if (SecTrustCreateWithCertificates(certsArray.get(), policy.get(), &trust) == noErr) {
            CFPtr<SecTrustRef> trustPtr(trust);
            if (SecTrustEvaluateWithError(trust, NULL))
                return true;
        }
#else
        (void)chain;
        (void)host;
#endif
        return false;
    }

    static enum ssl_verify_result_t verifyCertificateChain(struct SSL* ssl, uint8_t* out_alert)
    {
        auto sk_CRYPTO_BUFFER_num = [] (const struct stack_st_CRYPTO_BUFFER* sk) -> size_t {
            return sk_num(reinterpret_cast<const _STACK *>(sk));
        };
        auto sk_CRYPTO_BUFFER_value = [] (const struct stack_st_CRYPTO_BUFFER* sk, size_t i) -> CRYPTO_BUFFER* {
            return reinterpret_cast<CRYPTO_BUFFER*>(sk_value(reinterpret_cast<const _STACK*>(sk), i));
        };

        const auto& properties = *static_cast<security_properties_impl*>(SSL_get_ex_data(ssl, 0));
        const char* host = properties.host_.c_str();
        const struct stack_st_CRYPTO_BUFFER* chain = SSL_get0_peer_certificates(ssl);
        size_t length = sk_CRYPTO_BUFFER_num(chain);
        if (!length)
            return ssl_verify_invalid;

        std::vector<std::string_view> chainDER;
        for (size_t i = 0; i < length; ++i) {
            CRYPTO_BUFFER *der_buffer = sk_CRYPTO_BUFFER_value(chain, i);
            const char *der_data = reinterpret_cast<const char*>(CRYPTO_BUFFER_data(der_buffer));
            size_t der_length = CRYPTO_BUFFER_len(der_buffer);
            chainDER.push_back({ der_data, der_length });
        }
        if (properties.certificate_verifier_)
            return properties.certificate_verifier_(chainDER) ? ssl_verify_ok : ssl_verify_invalid;
        return verifyCertificateChainWithOperatingSystem(chainDER, host) ? ssl_verify_ok : ssl_verify_invalid;
    }

    template <typename> friend class detail::reactive_socket_service;

    bool security_disabled_ { false };
    ssl::unique_ptr<SSL> ssl_;
    ssl::unique_ptr<EVP_PKEY> private_key_;
    std::vector<ssl::unique_ptr<CRYPTO_BUFFER>> certificates_;
    std::string host_;
    std::function<bool(const std::vector<std::string_view>&)> certificate_verifier_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_SECURITY_PROPERTIES_IMPL_HPP
