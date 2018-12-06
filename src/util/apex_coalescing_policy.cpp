//  Copyright (c) 2007-2013 Kevin Huck
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <hpx/config.hpp>
#include <hpx/util/apex_coalescing_policy.hpp>

namespace hpx { namespace util
{
#if defined(HPX_HAVE_APEX) && defined(HPX_HAVE_PARCEL_COALESCING)

    apex_parcel_coalescing_policy* apex_parcel_coalescing_policy::instance = nullptr;
    apex_event_type apex_parcel_coalescing_policy::custom_coalescing_event;
    apex_tuning_session_handle apex_parcel_coalescing_policy::tuning_session_handle;
    apex_policy_handle* apex_parcel_coalescing_policy::policy_handle = nullptr;
#endif
}}

