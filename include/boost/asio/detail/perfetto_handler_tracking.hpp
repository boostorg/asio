//
// detail/perfetto_handler_tracking.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_PERFETTO_HANDLER_TRACKING_HPP
#define BOOST_ASIO_DETAIL_PERFETTO_HANDLER_TRACKING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_PERFETTO_HANDLER_TRACKING)

#include <cstddef>
#include <string>
#include <boost/system/error_code.hpp>
#include <boost/asio/detail/cstdint.hpp>
#include <boost/asio/detail/perfetto_writer.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
BOOST_ASIO_INLINE_NAMESPACE_BEGIN

class execution_context;

namespace detail {
namespace perfetto_protocol {

struct annotation;

} // namespace perfetto_protocol

class perfetto_handler_tracking
{
public:
  class completion;

  // Base class for objects containing tracked handlers.
  class tracked_handler
  {
  private:
    friend class perfetto_handler_tracking;
    friend class completion;
    uint64_t id_;
    const void* object_;
    const char* object_type_;
    const char* op_name_;
    bool reactor_linked_;

  protected:
    // Constructor initialises with no id.
    tracked_handler()
      : id_(0), object_(0), object_type_(0), op_name_(0), reactor_linked_(false)
    {
    }

    // Prevent deletion through this type.
    ~tracked_handler() {}
  };

  // Initialise the tracking system.
  BOOST_ASIO_DECL static void init();

  class location
  {
  public:
    // Constructor emits a slice_begin for the enclosing call site, nested
    // within the current handler-invocation slice on this thread track.
    BOOST_ASIO_DECL explicit location(const char* file,
        int line, const char* func);

    // Destructor emits the matching slice_end.
    BOOST_ASIO_DECL ~location();

  private:
    // Disallow copying and assignment.
    location(const location&) = delete;
    location& operator=(const location&) = delete;

    const char* file_;
    int line_;
    const char* func_;
    bool emitted_;
  };

  // Record the creation of a tracked handler.
  BOOST_ASIO_DECL static void creation(tracked_handler& h,
      const char* object_type, void* object,
      uintmax_t native_handle, const char* op_name);

  // Record the creation of a tracked handler.
  BOOST_ASIO_DECL static void creation(
      execution_context& context, tracked_handler& h,
      const char* object_type, void* object,
      uintmax_t native_handle, const char* op_name);

  class completion
  {
  public:
    // Constructor records that handler is to be invoked with no arguments.
    BOOST_ASIO_DECL explicit completion(const tracked_handler& h);

    // Destructor records only when an exception is thrown from the handler, or
    // if the memory is being freed without the handler having been invoked.
    BOOST_ASIO_DECL ~completion();

    // Records that handler is to be invoked with no arguments.
    BOOST_ASIO_DECL void invocation_begin();

    // Records that handler is to be invoked with one argument.
    BOOST_ASIO_DECL void invocation_begin(const boost::system::error_code& ec);

    // Constructor records that handler is to be invoked with two arguments.
    BOOST_ASIO_DECL void invocation_begin(
        const boost::system::error_code& ec, std::size_t bytes_transferred);

    // Constructor records that handler is to be invoked with two arguments.
    BOOST_ASIO_DECL void invocation_begin(
        const boost::system::error_code& ec, int signal_number);

    // Constructor records that handler is to be invoked with two arguments.
    BOOST_ASIO_DECL void invocation_begin(
        const boost::system::error_code& ec, const char* arg);

    // Record that handler invocation has ended.
    BOOST_ASIO_DECL void invocation_end();

  private:
    friend class perfetto_handler_tracking;
    uint64_t id_;
    const void* object_;
    const char* object_type_;
    const char* op_name_;
    bool reactor_linked_;
    bool invoked_;

    // Shared body of the invocation_begin overloads.
    BOOST_ASIO_DECL static void do_invocation_begin(
        uint64_t id, bool reactor_linked,
        const char* object_type, const void* object, const char* op_name,
        const perfetto_protocol::annotation* annotations,
        std::size_t annotation_count);

    // Close the invocation slice on the calling thread.
    BOOST_ASIO_DECL static void do_invocation_end(uint64_t id);

    // Handle destruction of a completion that was either invoked-and-threw or
    // abandoned before its handler ran.
    BOOST_ASIO_DECL static void completion_end(uint64_t id, bool invoked,
        bool reactor_linked);
  };

  // Record an operation that is not directly associated with a handler.
  BOOST_ASIO_DECL static void operation(execution_context& context,
      const char* object_type, void* object,
      uintmax_t native_handle, const char* op_name);

  // Record that a descriptor has been registered with the reactor.
  BOOST_ASIO_DECL static void reactor_registration(execution_context& context,
      uintmax_t native_handle, uintmax_t registration);

  // Record that a descriptor has been deregistered from the reactor.
  BOOST_ASIO_DECL static void reactor_deregistration(execution_context& context,
      uintmax_t native_handle, uintmax_t registration);

  // Record a reactor-based operation that is associated with a handler.
  BOOST_ASIO_DECL static void reactor_events(execution_context& context,
      uintmax_t registration, unsigned events);

  // Record a reactor-based operation that is associated with a handler.
  BOOST_ASIO_DECL static void reactor_operation(
      tracked_handler& h, const char* op_name,
      const boost::system::error_code& ec);

  // Record a reactor-based operation that is associated with a handler.
  BOOST_ASIO_DECL static void reactor_operation(
      tracked_handler& h, const char* op_name,
      const boost::system::error_code& ec, std::size_t bytes_transferred);

  // Scoped record of an execution context run call (run, run_one, poll, etc.).
  class scheduler_operation
  {
  public:
    BOOST_ASIO_DECL scheduler_operation(
        const void* scheduler, const char* name);
    BOOST_ASIO_DECL ~scheduler_operation();

  private:
    scheduler_operation(const scheduler_operation&) = delete;
    scheduler_operation& operator=(const scheduler_operation&) = delete;
    bool active_;
  };

private:
  // Append s to [p, end), returning the new position.
  BOOST_ASIO_DECL static char* append(char* p, char* end, const char* s);

  // Append the pointer as "0x" followed by lower-case hex with no leading
  // zeros.
  BOOST_ASIO_DECL static char* append_ptr(char* p, char* end, const void* ptr);

  // Append value as decimal digits, returning the new position.
  BOOST_ASIO_DECL static char* append_uint(char* p, char* end, unsigned value);

  // Format "object_type@0xADDR.op_name", optionally followed by a space and a
  // suffix, into buf.
  BOOST_ASIO_DECL static uint32_t object_label(char* buf, std::size_t n,
      const char* object_type, const void* object, const char* op_name,
      const char* suffix);

  // Format "object_type@0xADDR.op_name" into buf.
  BOOST_ASIO_DECL static uint32_t op_label(char* buf, std::size_t n,
      const char* object_type, const void* object, const char* op_name);

  // Titles a handler's invocation slice.
  BOOST_ASIO_DECL static uint32_t completion_label(char* buf, std::size_t n,
      const char* object_type, const void* object, const char* op_name);

  // Names a reactor-operation instant.
  BOOST_ASIO_DECL static void reactor_label(char* buf, std::size_t n,
      const char* object_type, const void* object, const char* op_name,
      const char* reactor_op);

  // Names a reactor lifecycle instant with the reactor's address.
  BOOST_ASIO_DECL static void reactor_event_label(char* buf, std::size_t n,
      const void* reactor, const char* suffix);

  // Formats the source location as "file:line" into buf.
  BOOST_ASIO_DECL static void location_label(char* buf, std::size_t n,
      const char* file, int line);

  // Used to distinguish an async op's intermediate reactor events.
  static const uint64_t reactor_flow_bit = 1ULL << 63;
};

# define BOOST_ASIO_INHERIT_TRACKED_HANDLER \
  : public boost::asio::detail::perfetto_handler_tracking::tracked_handler

# define BOOST_ASIO_ALSO_INHERIT_TRACKED_HANDLER \
  , public boost::asio::detail::perfetto_handler_tracking::tracked_handler

# define BOOST_ASIO_HANDLER_TRACKING_INIT \
  boost::asio::detail::perfetto_handler_tracking::init()

# define BOOST_ASIO_HANDLER_LOCATION(args) \
  boost::asio::detail::perfetto_handler_tracking::location tracked_location args

# define BOOST_ASIO_HANDLER_CREATION(args) \
  boost::asio::detail::perfetto_handler_tracking::creation args

# define BOOST_ASIO_HANDLER_COMPLETION(args) \
  boost::asio::detail::perfetto_handler_tracking::completion \
    tracked_completion args

# define BOOST_ASIO_HANDLER_INVOCATION_BEGIN(args) \
  tracked_completion.invocation_begin args

# define BOOST_ASIO_HANDLER_INVOCATION_END \
  tracked_completion.invocation_end()

# define BOOST_ASIO_HANDLER_OPERATION(args) \
  boost::asio::detail::perfetto_handler_tracking::operation args

# define BOOST_ASIO_HANDLER_REACTOR_REGISTRATION(args) \
  boost::asio::detail::perfetto_handler_tracking::reactor_registration args

# define BOOST_ASIO_HANDLER_REACTOR_DEREGISTRATION(args) \
  boost::asio::detail::perfetto_handler_tracking::reactor_deregistration args

# define BOOST_ASIO_HANDLER_REACTOR_READ_EVENT 1
# define BOOST_ASIO_HANDLER_REACTOR_WRITE_EVENT 2
# define BOOST_ASIO_HANDLER_REACTOR_ERROR_EVENT 4

# define BOOST_ASIO_HANDLER_REACTOR_EVENTS(args) \
  boost::asio::detail::perfetto_handler_tracking::reactor_events args

# define BOOST_ASIO_HANDLER_REACTOR_OPERATION(args) \
  boost::asio::detail::perfetto_handler_tracking::reactor_operation args

# define BOOST_ASIO_HANDLER_SCHEDULER_RUN(args) \
  boost::asio::detail::perfetto_handler_tracking::scheduler_operation \
    tracked_scheduler args

} // namespace detail
BOOST_ASIO_INLINE_NAMESPACE_END
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // defined(BOOST_ASIO_PERFETTO_HANDLER_TRACKING)

#if defined(BOOST_ASIO_HEADER_ONLY)
# include <boost/asio/detail/impl/perfetto_handler_tracking.ipp>
#endif // defined(BOOST_ASIO_HEADER_ONLY)

#endif // BOOST_ASIO_DETAIL_PERFETTO_HANDLER_TRACKING_HPP
