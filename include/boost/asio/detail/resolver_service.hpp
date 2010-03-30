//
// resolver_service.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_RESOLVER_SERVICE_HPP
#define BOOST_ASIO_DETAIL_RESOLVER_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <cstring>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/basic_resolver_iterator.hpp>
#include <boost/asio/ip/basic_resolver_query.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/detail/fenced_block.hpp>
#include <boost/asio/detail/mutex.hpp>
#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/operation.hpp>
#include <boost/asio/detail/service_base.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/detail/thread.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename Protocol>
class resolver_service
  : public boost::asio::detail::service_base<resolver_service<Protocol> >
{
private:
  // Helper class to perform exception-safe cleanup of addrinfo objects.
  class auto_addrinfo
    : private boost::asio::detail::noncopyable
  {
  public:
    explicit auto_addrinfo(boost::asio::detail::addrinfo_type* ai)
      : ai_(ai)
    {
    }

    ~auto_addrinfo()
    {
      if (ai_)
        socket_ops::freeaddrinfo(ai_);
    }

    operator boost::asio::detail::addrinfo_type*()
    {
      return ai_;
    }

  private:
    boost::asio::detail::addrinfo_type* ai_;
  };

public:
  // The implementation type of the resolver. The shared pointer is used as a
  // cancellation token to indicate to the background thread that the operation
  // has been cancelled.
  typedef boost::shared_ptr<void> implementation_type;
  struct noop_deleter { void operator()(void*) {} };

  // The endpoint type.
  typedef typename Protocol::endpoint endpoint_type;

  // The query type.
  typedef boost::asio::ip::basic_resolver_query<Protocol> query_type;

  // The iterator type.
  typedef boost::asio::ip::basic_resolver_iterator<Protocol> iterator_type;

  // Constructor.
  resolver_service(boost::asio::io_service& io_service)
    : boost::asio::detail::service_base<
        resolver_service<Protocol> >(io_service),
      mutex_(),
      io_service_impl_(boost::asio::use_service<io_service_impl>(io_service)),
      work_io_service_(new boost::asio::io_service),
      work_io_service_impl_(boost::asio::use_service<
          io_service_impl>(*work_io_service_)),
      work_(new boost::asio::io_service::work(*work_io_service_)),
      work_thread_(0)
  {
  }

  // Destructor.
  ~resolver_service()
  {
    shutdown_service();
  }

  // Destroy all user-defined handler objects owned by the service.
  void shutdown_service()
  {
    work_.reset();
    if (work_io_service_)
    {
      work_io_service_->stop();
      if (work_thread_)
      {
        work_thread_->join();
        work_thread_.reset();
      }
      work_io_service_.reset();
    }
  }

  // Construct a new resolver implementation.
  void construct(implementation_type& impl)
  {
    impl.reset(static_cast<void*>(0), noop_deleter());
  }

  // Destroy a resolver implementation.
  void destroy(implementation_type&)
  {
  }

  // Cancel pending asynchronous operations.
  void cancel(implementation_type& impl)
  {
    impl.reset(static_cast<void*>(0), noop_deleter());
  }

  // Resolve a query to a list of entries.
  iterator_type resolve(implementation_type&, const query_type& query,
      boost::system::error_code& ec)
  {
    boost::asio::detail::addrinfo_type* address_info = 0;
    std::string host_name = query.host_name();
    std::string service_name = query.service_name();
    boost::asio::detail::addrinfo_type hints = query.hints();

    socket_ops::getaddrinfo(!host_name.empty() ? host_name.c_str() : 0,
        service_name.c_str(), &hints, &address_info, ec);
    auto_addrinfo auto_address_info(address_info);

    if (ec)
      return iterator_type();

    return iterator_type::create(address_info, host_name, service_name);
  }

  template <typename Handler>
  class resolve_op
    : public operation
  {
  public:
    resolve_op(implementation_type impl, const query_type& query,
        io_service_impl& io_service_impl, Handler handler)
      : operation(&resolve_op::do_complete),
        impl_(impl),
        query_(query),
        io_service_impl_(io_service_impl),
        handler_(handler)
    {
    }

    static void do_complete(io_service_impl* owner, operation* base,
        boost::system::error_code /*ec*/, std::size_t /*bytes_transferred*/)
    {
      // Take ownership of the operation object.
      resolve_op* o(static_cast<resolve_op*>(base));
      typedef handler_alloc_traits<Handler, resolve_op> alloc_traits;
      handler_ptr<alloc_traits> ptr(o->handler_, o);

      if (owner)
      {
        if (owner != &o->io_service_impl_)
        {
          // The operation is being run on the worker io_service. Time to
          // perform the resolver operation.
        
          if (o->impl_.expired())
          {
            // THe operation has been cancelled.
            o->ec_ = boost::asio::error::operation_aborted;
          }
          else
          {
            // Perform the blocking host resolution operation.
            boost::asio::detail::addrinfo_type* address_info = 0;
            std::string host_name = o->query_.host_name();
            std::string service_name = o->query_.service_name();
            boost::asio::detail::addrinfo_type hints = o->query_.hints();
            socket_ops::getaddrinfo(!host_name.empty() ? host_name.c_str() : 0,
                service_name.c_str(), &hints, &address_info, o->ec_);
            auto_addrinfo auto_address_info(address_info);
            o->iter_ = iterator_type::create(
              address_info, host_name, service_name);
          }

          o->io_service_impl_.post_deferred_completion(o);
          ptr.release();
        }
        else
        {
          // The operation has been returned to the main io_serice. The
          // completion handler is ready to be delivered.

          // Make a copy of the handler so that the memory can be deallocated
          // before the upcall is made. Even if we're not about to make an
          // upcall, a sub-object of the handler may be the true owner of the
          // memory associated with the handler. Consequently, a local copy of
          // the handler is required to ensure that any owning sub-object
          // remains valid until after we have deallocated the memory here.
          detail::binder2<Handler, boost::system::error_code, iterator_type>
            handler(o->handler_, o->ec_, o->iter_);
          ptr.reset();
          boost::asio::detail::fenced_block b;
          boost_asio_handler_invoke_helpers::invoke(handler, handler);
        }
      }
    }

  private:
    boost::weak_ptr<void> impl_;
    query_type query_;
    io_service_impl& io_service_impl_;
    Handler handler_;
    boost::system::error_code ec_;
    iterator_type iter_;
  };

  // Asynchronously resolve a query to a list of entries.
  template <typename Handler>
  void async_resolve(implementation_type& impl, const query_type& query,
      Handler handler)
  {
    // Allocate and construct an operation to wrap the handler.
    typedef resolve_op<Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    handler_ptr<alloc_traits> ptr(raw_ptr,
        impl, query, io_service_impl_, handler);

    if (work_io_service_)
    {
      start_work_thread();
      io_service_impl_.work_started();
      work_io_service_impl_.post_immediate_completion(ptr.get());
      ptr.release();
    }
  }

  // Resolve an endpoint to a list of entries.
  iterator_type resolve(implementation_type&,
      const endpoint_type& endpoint, boost::system::error_code& ec)
  {
    // First try resolving with the service name. If that fails try resolving
    // but allow the service to be returned as a number.
    char host_name[NI_MAXHOST];
    char service_name[NI_MAXSERV];
    int flags = endpoint.protocol().type() == SOCK_DGRAM ? NI_DGRAM : 0;
    socket_ops::getnameinfo(endpoint.data(), endpoint.size(),
        host_name, NI_MAXHOST, service_name, NI_MAXSERV, flags, ec);
    if (ec)
    {
      flags |= NI_NUMERICSERV;
      socket_ops::getnameinfo(endpoint.data(), endpoint.size(),
          host_name, NI_MAXHOST, service_name, NI_MAXSERV, flags, ec);
    }

    if (ec)
      return iterator_type();

    return iterator_type::create(endpoint, host_name, service_name);
  }

  template <typename Handler>
  class resolve_endpoint_op
    : public operation
  {
  public:
    resolve_endpoint_op(implementation_type impl, const endpoint_type& ep,
        io_service_impl& io_service_impl, Handler handler)
      : operation(&resolve_endpoint_op::do_complete),
        impl_(impl),
        ep_(ep),
        io_service_impl_(io_service_impl),
        handler_(handler)
    {
    }

    static void do_complete(io_service_impl* owner, operation* base,
        boost::system::error_code /*ec*/, std::size_t /*bytes_transferred*/)
    {
      // Take ownership of the operation object.
      resolve_endpoint_op* o(static_cast<resolve_endpoint_op*>(base));
      typedef handler_alloc_traits<Handler, resolve_endpoint_op> alloc_traits;
      handler_ptr<alloc_traits> ptr(o->handler_, o);

      if (owner)
      {
        if (owner != &o->io_service_impl_)
        {
          // The operation is being run on the worker io_service. Time to
          // perform the resolver operation.
        
          if (o->impl_.expired())
          {
            // THe operation has been cancelled.
            o->ec_ = boost::asio::error::operation_aborted;
          }
          else
          {
            // Perform the blocking endoint resolution operation.
            char host_name[NI_MAXHOST];
            char service_name[NI_MAXSERV];
            int flags = o->ep_.protocol().type() == SOCK_DGRAM ? NI_DGRAM : 0;
            socket_ops::getnameinfo(o->ep_.data(), o->ep_.size(),
                host_name, NI_MAXHOST, service_name,
                NI_MAXSERV, flags, o->ec_);
            if (o->ec_)
            {
              flags |= NI_NUMERICSERV;
              socket_ops::getnameinfo(o->ep_.data(), o->ep_.size(),
                  host_name, NI_MAXHOST, service_name,
                  NI_MAXSERV, flags, o->ec_);
            }
            o->iter_ = iterator_type::create(o->ep_, host_name, service_name);
          }

          o->io_service_impl_.post_deferred_completion(o);
          ptr.release();
        }
        else
        {
          // The operation has been returned to the main io_serice. The
          // completion handler is ready to be delivered.

          // Make a copy of the handler so that the memory can be deallocated
          // before the upcall is made. Even if we're not about to make an
          // upcall, a sub-object of the handler may be the true owner of the
          // memory associated with the handler. Consequently, a local copy of
          // the handler is required to ensure that any owning sub-object
          // remains valid until after we have deallocated the memory here.
          detail::binder2<Handler, boost::system::error_code, iterator_type>
            handler(o->handler_, o->ec_, o->iter_);
          ptr.reset();
          boost::asio::detail::fenced_block b;
          boost_asio_handler_invoke_helpers::invoke(handler, handler);
        }
      }
    }

  private:
    boost::weak_ptr<void> impl_;
    endpoint_type ep_;
    io_service_impl& io_service_impl_;
    Handler handler_;
    boost::system::error_code ec_;
    iterator_type iter_;
  };

  // Asynchronously resolve an endpoint to a list of entries.
  template <typename Handler>
  void async_resolve(implementation_type& impl, const endpoint_type& endpoint,
      Handler handler)
  {
    // Allocate and construct an operation to wrap the handler.
    typedef resolve_endpoint_op<Handler> value_type;
    typedef handler_alloc_traits<Handler, value_type> alloc_traits;
    raw_handler_ptr<alloc_traits> raw_ptr(handler);
    handler_ptr<alloc_traits> ptr(raw_ptr,
        impl, endpoint, io_service_impl_, handler);

    if (work_io_service_)
    {
      start_work_thread();
      io_service_impl_.work_started();
      work_io_service_impl_.post_immediate_completion(ptr.get());
      ptr.release();
    }
  }

private:
  // Helper class to run the work io_service in a thread.
  class work_io_service_runner
  {
  public:
    work_io_service_runner(boost::asio::io_service& io_service)
      : io_service_(io_service) {}
    void operator()() { io_service_.run(); }
  private:
    boost::asio::io_service& io_service_;
  };

  // Start the work thread if it's not already running.
  void start_work_thread()
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);
    if (!work_thread_)
    {
      work_thread_.reset(new boost::asio::detail::thread(
            work_io_service_runner(*work_io_service_)));
    }
  }

  // Mutex to protect access to internal data.
  boost::asio::detail::mutex mutex_;

  // The io_service implementation used to post completions.
  io_service_impl& io_service_impl_;

  // Private io_service used for performing asynchronous host resolution.
  boost::scoped_ptr<boost::asio::io_service> work_io_service_;

  // The work io_service implementation used to post completions.
  io_service_impl& work_io_service_impl_;

  // Work for the private io_service to perform.
  boost::scoped_ptr<boost::asio::io_service::work> work_;

  // Thread used for running the work io_service's run loop.
  boost::scoped_ptr<boost::asio::detail::thread> work_thread_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_RESOLVER_SERVICE_HPP
