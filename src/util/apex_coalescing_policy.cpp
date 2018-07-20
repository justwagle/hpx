//  Copyright (c) 2007-2013 Kevin Huck
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <hpx/config.hpp>
#include <hpx/util/apex_coalescing_policy.hpp>

namespace hpx { namespace util
{
#if defined(HPX_HAVE_APEX)

    apex_parcel_coalescing_policy* apex_parcel_coalescing_policy::instance = nullptr;

#endif
}}

