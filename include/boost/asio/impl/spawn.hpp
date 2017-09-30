//
// impl/spawn.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2017 Oliver Kowalke (oliver dot kowalke at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IMPL_SPAWN_HPP
#define BOOST_ASIO_IMPL_SPAWN_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/detail/atomic_count.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_cont_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>
#include <boost/asio/detail/shared_ptr.hpp>
#include <boost/asio/handler_type.hpp>
#include <boost/context/stack_context.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

template <typename Handler, typename T>
class continuation_handler
{
public:
    continuation_handler(basic_yield_context<Handler> ctx) :
        yc_(ctx.yc_.lock()),
        handler_(ctx.handler_ ),
        ready_(0),
        ec_(ctx.ec_),
        value_(0)
    {
    }

    void operator()(boost::system::error_code ec, T value)
    {
        *ec_ = ec;
        *value_ = BOOST_ASIO_MOVE_CAST(T)(value);
        if (0 == --*ready_)
        {
            yc_->resume();
        }
    }

    void operator()(T value)
    {
        (*this)(boost::system::error_code(), value);
    }

//private:
    shared_ptr<continuation_context>    yc_;
    Handler&                            handler_;
    atomic_count*                       ready_;
    boost::system::error_code*          ec_;
    T*                                  value_;
};

template <typename Handler>
class continuation_handler<Handler, void>
{
public:
    continuation_handler(basic_yield_context<Handler> ctx) :
        yc_(ctx.yc_.lock()),
        handler_(ctx.handler_),
        ready_(0),
        ec_(ctx.ec_)
    {
    }

    void operator()(boost::system::error_code ec)
    {
        *ec_ = boost::system::error_code();
        if (0 == --*ready_)
        {
            yc_->resume();
        }
    }

    void operator()()
    {
        (*this)(boost::system::error_code());
    }

//private:
    shared_ptr<continuation_context>    yc_;
    Handler&                            handler_;
    atomic_count*                       ready_;
    boost::system::error_code*          ec_;
};

template <typename Handler, typename T>
inline void* asio_handler_allocate(std::size_t size,
                                   continuation_handler<Handler, T>* this_handler)
{
    return boost_asio_handler_alloc_helpers::allocate(size, this_handler->handler_);
}

template <typename Handler, typename T>
inline void asio_handler_deallocate(void* pointer,
                                    std::size_t size,
                                    continuation_handler<Handler, T>* this_handler)
{
    boost_asio_handler_alloc_helpers::deallocate(pointer, size, this_handler->handler_);
}

template <typename Handler, typename T>
inline bool asio_handler_is_continuation(continuation_handler< Handler, T >*)
{
    return true;
}

template <typename Function, typename Handler, typename T>
inline void asio_handler_invoke(Function& function,
                                continuation_handler<Handler, T>* this_handler)
{
    boost_asio_handler_invoke_helpers::invoke(function, this_handler->handler_);
}

template <typename Function, typename Handler, typename T>
inline void asio_handler_invoke(const Function& function,
                                continuation_handler<Handler, T>* this_handler)
{
    boost_asio_handler_invoke_helpers::invoke(function, this_handler->handler_);
}

} // namespace detail

#if !defined(GENERATING_DOCUMENTATION)

template <typename Handler, typename ReturnType>
struct handler_type<basic_yield_context<Handler>, ReturnType()>
{
  typedef detail::continuation_handler<Handler, void> type;
};

template <typename Handler, typename ReturnType, typename Arg1>
struct handler_type<basic_yield_context<Handler>, ReturnType(Arg1)>
{
  typedef detail::continuation_handler<Handler, Arg1> type;
};

template <typename Handler, typename ReturnType>
struct handler_type<basic_yield_context<Handler>, ReturnType(boost::system::error_code)>
{
  typedef detail::continuation_handler<Handler, void> type;
};

template <typename Handler, typename ReturnType, typename Arg2>
struct handler_type< basic_yield_context<Handler>, ReturnType(boost::system::error_code, Arg2)>
{
  typedef detail::continuation_handler<Handler, Arg2> type;
};

template <typename Handler, typename T>
class async_result<detail::continuation_handler<Handler, T> >
{
public:
    typedef T   type;

    explicit async_result(detail::continuation_handler<Handler, T>& h) :
            handler_(h),
            ready_(2)
    {
        h.ready_ = &ready_;
        out_ec_ = h.ec_;
        if (!out_ec_)
        {
            h.ec_ = &ec_;
        }
        h.value_ = &value_;
    }

    type get()
    {
        if (0 != --ready_)
        {
            handler_.yc_->suspend();
        }
        if (!out_ec_ && ec_)
        {
            throw boost::system::system_error(ec_);
        }
        return BOOST_ASIO_MOVE_CAST(type)(value_);
    }

private:
    detail::continuation_handler<Handler, T>    handler_;
    detail::atomic_count                        ready_;
    boost::system::error_code*                  out_ec_;
    boost::system::error_code                   ec_;
    type                                        value_;
};

template <typename Handler>
class async_result<detail::continuation_handler<Handler, void> >
{
public:
    typedef void    type;

    explicit async_result(detail::continuation_handler<Handler, void>& h) :
            handler_(h),
            ready_(2)
    {
        h.ready_ = &ready_;
        out_ec_ = h.ec_;
        if (!out_ec_) {
            h.ec_ = &ec_;
        }
    }

    void get()
    {
        if (0 != --ready_)
        {
            handler_.yc_->suspend();
        }
        if (!out_ec_ && ec_)
        {
            throw boost::system::system_error(ec_);
        }
    }

private:
    detail::continuation_handler<Handler, void> handler_;
    detail::atomic_count                        ready_;
    boost::system::error_code*                  out_ec_;
    boost::system::error_code                   ec_;
};

namespace detail {

template <typename Handler, typename Function, typename StackAllocator>
struct spawn_data : private noncopyable
{
    spawn_data(BOOST_ASIO_MOVE_ARG(Handler) handler,
               bool call_handler,
               BOOST_ASIO_MOVE_ARG(Function) function,
               BOOST_ASIO_MOVE_ARG(StackAllocator) salloc) :
        handler_(BOOST_ASIO_MOVE_CAST(Handler)(handler)),
        call_handler_(call_handler),
        function_(BOOST_ASIO_MOVE_CAST(Function)(function)),
        salloc_(BOOST_ASIO_MOVE_CAST(StackAllocator)(salloc))
    {
    }

    Handler                             handler_;
    bool                                call_handler_;
    Function                            function_;
    StackAllocator                      salloc_;
#if defined(BOOST_CONTEXT_NO_CXX11)
    shared_ptr<continuation_context>    yc_;
#endif
};

#if !defined(BOOST_CONTEXT_NO_CXX11)
template <typename Handler, typename Function, typename StackAllocator>
struct spawn_helper
{
    void operator()()
    {
        shared_ptr<continuation_context> yc(new continuation_context());
        yc->callee_ = boost::context::callcc(
                    std::allocator_arg, BOOST_ASIO_MOVE_CAST(StackAllocator)(data_->salloc_),
                    [yc,this](BOOST_ASIO_MOVE_ARG(boost::context::continuation) c)
                    {
                        yc->caller_ = BOOST_ASIO_MOVE_CAST(boost::context::continuation)(c);
                        shared_ptr<spawn_data<Handler, Function, StackAllocator> > data = data_;
                        const basic_yield_context<Handler> yh(data->handler_, yc);
                        (data->function_)(yh);
                        if (data->call_handler_)
                        {
                            (data->handler_)();
                        }
                        return BOOST_ASIO_MOVE_CAST(boost::context::continuation)(yc->caller_);
                    });
    }

    shared_ptr<spawn_data<Handler, Function, StackAllocator> >   data_;
};
#else
template <typename StackAllocator>
struct fcontext_continuation_context : public continuation_context
{
    fcontext_continuation_context(BOOST_ASIO_MOVE_ARG(StackAllocator) salloc,
                                  boost::context::stack_context const& sctx) :
        salloc_(BOOST_ASIO_MOVE_CAST(StackAllocator)(salloc)),
        sctx_(sctx)
    {
    }

    ~fcontext_continuation_context() BOOST_NOEXCEPT
    {
        salloc_.deallocate(sctx_);
    }

    StackAllocator                  salloc_;
    boost::context::stack_context   sctx_;
};

template <typename Handler, typename Function, typename StackAllocator>
void context_entry( boost::context::detail::transfer_t t) BOOST_NOEXCEPT
{
    shared_ptr<spawn_data<Handler, Function, StackAllocator> > data(*static_cast<shared_ptr<spawn_data<Handler, Function, StackAllocator> >*>(t.data));
    data->yc_->caller_ = t.fctx;
    const basic_yield_context<Handler> yh(data->handler_, data->yc_);
    (data->function_)(yh);
    if (data->call_handler_)
    {
        (data->handler_)();
    }
    boost::context::detail::fcontext_t caller = data->yc_->caller_;
    data.reset();
    boost::context::detail::jump_fcontext(caller, 0);
}

template <typename Handler, typename Function, typename StackAllocator>
struct spawn_helper
{
    void operator()()
    {
        boost::context::stack_context sctx = data_->salloc_.allocate();
        shared_ptr<continuation_context> yc(new fcontext_continuation_context<StackAllocator>(BOOST_ASIO_MOVE_CAST(StackAllocator)(data_->salloc_), sctx));
        data_->yc_ = yc;
        yc->callee_ = boost::context::detail::make_fcontext(sctx.sp, sctx.size, &context_entry<Handler, Function, StackAllocator>);
        yc->callee_ = boost::context::detail::jump_fcontext(data_->yc_->callee_, & data_).fctx;
    }

    shared_ptr<spawn_data<Handler, Function, StackAllocator> >  data_;
};
#endif

inline void default_spawn_handler() {}

} // namespace detail

template <typename Handler, typename Function, typename StackAllocator>
void spawn(BOOST_ASIO_MOVE_ARG(Handler) handler,
           BOOST_ASIO_MOVE_ARG(Function) function,
           BOOST_ASIO_MOVE_ARG(StackAllocator) salloc)
{
    detail::spawn_helper<Handler, Function, StackAllocator> helper;
    helper.data_.reset(
            new detail::spawn_data<Handler, Function, StackAllocator>(
                BOOST_ASIO_MOVE_CAST(Handler)(handler),
                true,
                BOOST_ASIO_MOVE_CAST(Function)(function),
                BOOST_ASIO_MOVE_CAST(StackAllocator)(salloc)));
    boost_asio_handler_invoke_helpers::invoke(helper, helper.data_->handler_);
}

template <typename Handler, typename Function, typename StackAllocator>
void spawn(basic_yield_context< Handler > ctx,
           BOOST_ASIO_MOVE_ARG(Function) function,
           BOOST_ASIO_MOVE_ARG(StackAllocator) salloc)
{
    Handler handler = ctx.handler_;
    detail::spawn_helper<Handler, Function, StackAllocator> helper;
    helper.data_.reset(
            new detail::spawn_data<Handler, Function, StackAllocator>(
                BOOST_ASIO_MOVE_CAST(Handler)(handler),
                false,
                BOOST_ASIO_MOVE_CAST(Function)(function),
                BOOST_ASIO_MOVE_CAST(StackAllocator)(salloc)));
    boost_asio_handler_invoke_helpers::invoke(helper, helper.data_->handler_);
}

template <typename Function, typename StackAllocator>
void spawn(io_service::strand strand,
           BOOST_ASIO_MOVE_ARG(Function) function,
           BOOST_ASIO_MOVE_ARG(StackAllocator) salloc)
{
    spawn(strand.wrap(&detail::default_spawn_handler),
          BOOST_ASIO_MOVE_CAST(Function)(function),
          BOOST_ASIO_MOVE_CAST(StackAllocator)(salloc));
}

template <typename Function, typename StackAllocator>
void spawn(io_service& io_service,
           BOOST_ASIO_MOVE_ARG(Function) function,
           BOOST_ASIO_MOVE_ARG(StackAllocator) salloc)
{
    spawn(io_service::strand(io_service),
          BOOST_ASIO_MOVE_CAST(Function)(function),
          BOOST_ASIO_MOVE_CAST(StackAllocator)(salloc));
}

#endif // !defined(GENERATING_DOCUMENTATION)

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_IMPL_SPAWN_HPP
