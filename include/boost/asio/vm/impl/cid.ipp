//
// vm/impl/cid.ipp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2023 OLiver Kowalke (oliver dot kowalke at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_VM_IMPL_CID_IPP
#define BOOST_ASIO_VM_IMPL_CID_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/vm/cid.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace vm {

cid::cid(cid::uint_type c)
{
  addr_.svm_cid = c;
}

cid::uint_type cid::to_uint() const BOOST_ASIO_NOEXCEPT
{
  return addr_.svm_cid;
}

bool cid::is_local() const BOOST_ASIO_NOEXCEPT
{
  return VMADDR_CID_LOCAL == to_uint();
}

bool cid::is_hypervisor() const BOOST_ASIO_NOEXCEPT
{
  return VMADDR_CID_HYPERVISOR == to_uint();
}

bool cid::is_any() const BOOST_ASIO_NOEXCEPT
{
  return VMADDR_CID_ANY == to_uint();
}

} // namespace vm
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_VM_IMPL_CID_IPP
