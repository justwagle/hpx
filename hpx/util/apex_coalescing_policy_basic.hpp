//  Copyright (c) 2007-2016 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once // prevent multiple inclusions of this header file.

#include <hpx/config.hpp>
#include <hpx/runtime/get_locality_id.hpp>
#include <hpx/runtime/get_num_localities.hpp>
#include <hpx/runtime/startup_function.hpp>
#include <hpx/runtime/config_entry.hpp>
#include <hpx/util/thread_description.hpp>
#include <iostream>

#ifdef HPX_HAVE_APEX
#include "apex_api.hpp"
#include "apex_policies.hpp"
#include <memory>
#include <mutex>
#include <cstdint>
#include <string>
#include <stdio.h>
#endif

namespace hpx { namespace util
{
#if defined(HPX_HAVE_APEX) && defined(HPX_HAVE_PARCEL_COALESCING)

    struct apex_parcel_coalescing_policy_basic
    {
        //std::shared_ptr<apex_policy_handle> policy_handle;
        //std::shared_ptr<apex_policy_handle> policy_handle_sample_counter;
        //std::shared_ptr<apex_tuning_request> request;
        HPX_API_EXPORT static apex_policy_handle* policy_handle;
        apex_policy_handle* policy_handle_sample_counter;
        apex_tuning_request* request;
        HPX_API_EXPORT static apex_tuning_session_handle tuning_session_handle;
        int tuning_window;
        int send_count;
	double * duration_;

        std::string counter_name;
        std::string name;

        HPX_API_EXPORT static apex_parcel_coalescing_policy_basic* instance_basic;

        HPX_API_EXPORT static apex_event_type custom_coalescing_event;

        static std::mutex params_mutex;

        std::mutex count_mutex;
        std::mutex policy_mutex;

        void set_coalescing_params()
        {
            std::shared_ptr<apex_param_long> parcel_count_param =
                std::static_pointer_cast<apex_param_long>(
                    request->get_param("parcel_count"));

            std::shared_ptr<apex_param_long> buffer_time_param =
                std::static_pointer_cast<apex_param_long>(
                    request->get_param("buffer_time"));

            const int parcel_count = parcel_count_param->get_value();
            const int buffer_time = buffer_time_param->get_value();


            apex::sample_value(
                "hpx.plugins.coalescing_message_handler.num_messages",
                parcel_count);
            apex::sample_value(
                "hpx.plugins.coalescing_message_handler.interval", buffer_time);
            std::cout<<"now setting coalescing values Parcel Count: " << parcel_count << "  Buffer_time: " << buffer_time << "\n";
            hpx::set_config_entry(
                "hpx.plugins.coalescing_message_handler.num_messages",
                parcel_count);
            hpx::set_config_entry(
                "hpx.plugins.coalescing_message_handler.interval", buffer_time);
        }

        static int direct_policy(const apex_context context)
        {
	    if (!apex::has_session_converged(tuning_session_handle)){
                	apex::custom_event(instance_basic->request->get_trigger(), NULL);
                	instance_basic->set_coalescing_params();
                	//apex::reset(instance->counter_name);
            }
            else {
 		//printf("As session is converged, policy is deregistered\n"); fflush(stdout);
 		//apex::deregister_policy(policy_handle);

	    }
            return APEX_NOERROR;
	
        }

        static int count_based_policy(const apex_context context)
        {
            if (instance_basic->send_count < 50000)
            {
                if(instance_basic->count_mutex.try_lock())
                {
                    instance_basic->send_count++;
                    instance_basic->count_mutex.unlock();
                }
                return APEX_NOERROR;
            }
            else
            {
                if(instance_basic->policy_mutex.try_lock())
                {
                    instance_basic->send_count=0;
                    instance_basic->direct_policy(context);
                    instance_basic->policy_mutex.unlock();
                }
                return APEX_NOERROR;
            }
        }


        static apex_event_type apex_parcel_coalescing_event(
            apex_event_type in_type = APEX_INVALID_EVENT)
        {
            static apex_event_type event_type;
             if (in_type != APEX_INVALID_EVENT)
             {
                 event_type = in_type;
             }
             return event_type;
        }

        apex_parcel_coalescing_policy_basic(double & duration)
          : tuning_window(1)
          , duration_ (&duration)
        {
            std::stringstream ss;
            ss << "/threads{locality#" << hpx::get_locality_id();
            ss << "/total}/background-overhead";

	    //ss << "/statistics{/threads{locality#" << hpx::get_locality_id();
            //ss << "/total}/time/background-overhead}/rolling_average@1000,5000,1";
            counter_name = std::string(ss.str());
            name = "basic_policy";
            //policy_handle_sample_counter = apex::sample_runtime_counter(50000, counter_name);
            std::function<double(void)> metric = [=]() -> double {
		std::cout << "duration : " << *duration_ << std::endl;
                return *duration_;
            };
            request = new apex_tuning_request(name);
            request->set_metric(metric);
            //request->set_strategy(apex_ah_tuning_strategy::EXHAUSTIVE);
            request->set_strategy(apex_ah_tuning_strategy::PARALLEL_RANK_ORDER);
            //request->add_param_long("parcel_count", 20, 20, 26, 2);
            request->add_param_long("parcel_count", 20, 2, 256, 2);
            //request->add_param_long("buffer_time", 1000, 1000, 2000, 1000);
            request->add_param_long("buffer_time", 1000, 1000, 5000, 1000);
            request->set_trigger(apex::register_custom_event(name));
            tuning_session_handle = apex::setup_custom_tuning(*request);

	    // To register the policy using the send event: uncomment the following line
            //policy_handle = apex::register_policy(APEX_SEND, count_based_policy);

	    // To register a periodic policy: uncomment the following line
            //policy_handle = apex::register_periodic_policy(500000, direct_policy);

	    // To register a custom event : uncomment the following two line
            custom_coalescing_event = apex_parcel_coalescing_event(apex::register_custom_event("APEX parcel coalescing event"));
            policy_handle = apex::register_policy(custom_coalescing_event, direct_policy);


	    // To call the custom event include this header file and use this following commented line
	    // Do not uncomment the below line. It is just an example how to use it elsewhere
	    //apex::custom_event(hpx::util::apex_parcel_coalescing_policy_basic::return_apex_parcel_coalescing_event(), NULL);

            if (policy_handle == nullptr)
            {
                std::cerr << "Error registering policy!" << std::endl;
            }
            else std::cout<<" Done registering policy.\n";
        }

        static apex_event_type return_apex_parcel_coalescing_event()
        {
             return apex_parcel_coalescing_event(custom_coalescing_event);
        }


        static void initialize(double & duration)
        {
            if (instance_basic == nullptr)
            {
                instance_basic = new apex_parcel_coalescing_policy_basic(duration);
            }
        }
        static void finalize()
        {
            if (instance_basic != nullptr)
            {
                delete instance_basic;
                instance_basic = nullptr;
            }
        }
    };
#endif //HPX_HAVE_APEX && HPX_HAVE_PARCEL_COALESCING
}}

