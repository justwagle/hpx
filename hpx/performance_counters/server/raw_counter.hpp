//  Copyright (c) 2007-2010 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(HPX_PERFORMANCE_COUNTERS_SERVER_RAW_COUNTER_MAR_03_2009_0743M)
#define HPX_PERFORMANCE_COUNTERS_SERVER_RAW_COUNTER_MAR_03_2009_0743M

#include <hpx/hpx_fwd.hpp>
#include <hpx/performance_counters/server/base_performance_counter.hpp>

#include <boost/function.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace hpx { namespace performance_counters { namespace server
{
    class HPX_EXPORT raw_counter 
      : public base_performance_counter,
        public components::detail::managed_component_base<raw_counter> 
    {
        typedef components::detail::managed_component_base<raw_counter> base_type;

    public:
        typedef raw_counter type_holder;

        raw_counter() {}
        raw_counter(counter_info const& info, boost::function<boost::int64_t()> f);

        void get_counter_info(counter_info& info);
        void get_counter_value(counter_value& value);

        /// \brief finalize() will be called just before the instance gets 
        ///        destructed
        void finalize() 
        {
            base_performance_counter::finalize();
            base_type::finalize();
        }

        // This is the component id. Every component needs to have a function
        // \a get_component_type() which is used by the generic action 
        // implementation to associate this component with a given action.
        static components::component_type get_component_type() 
        { 
            return base_type::get_component_type(); 
        }
        static void set_component_type(components::component_type t) 
        { 
            base_type::set_component_type(t);
        }

    private:
        counter_info info_;
        boost::function<boost::int64_t()> f_;
    };

}}}

#endif
