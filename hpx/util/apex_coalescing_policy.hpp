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

#ifdef HPX_HAVE_APEX
#include "apex_api.hpp"
#include "apex_policies.hpp"
#include <memory>
#include <mutex>
#include <cstdint>
#include <string>
#endif

namespace hpx { namespace util
{
#if defined(HPX_HAVE_APEX) && defined(HPX_HAVE_PARCEL_COALESCING)
    struct apex_parcel_coalescing_policy
    {
        apex_policy_handle* policy_handle;
        apex_tuning_request* request;
        int tuning_window;
        int send_count;

        std::string counter_name;
        std::string name;

        HPX_API_EXPORT static apex_parcel_coalescing_policy* instance;
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

            hpx::set_config_entry(
                "hpx.plugins.coalescing_message_handler.num_messages",
                parcel_count);
            hpx::set_config_entry(
                "hpx.plugins.coalescing_message_handler.interval", buffer_time);
        }

        static int policy(const apex_context context)
        {
            if (instance->send_count < 5000)
            {
                if(instance->count_mutex.try_lock())
                {
                    instance->send_count++;
                    instance->count_mutex.unlock();
                }
                return APEX_NOERROR;
            }
            else
            {
                if(instance->policy_mutex.try_lock())
                {
                    instance->send_count=0;
                    apex_profile* profile = apex::get_profile(instance->counter_name);
                    if (profile != nullptr && profile->calls >= instance->tuning_window)
                    {
                        apex::custom_event(instance->request->get_trigger(), NULL);
                        instance->set_coalescing_params();
                        apex::reset(instance->counter_name);
                    }
                    instance->policy_mutex.unlock();
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

        apex_parcel_coalescing_policy()
          : tuning_window(1)
          , name("HPX parcel coalescing")
        {
            std::stringstream ss;
            ss << "/threads{locality#" << hpx::get_locality_id();
            ss << "/total}/time/average-overhead";
            counter_name = std::string(ss.str());
            apex::sample_runtime_counter(500000, counter_name);
            std::function<double(void)> metric = [=]() -> double {
                apex_profile* profile = apex::get_profile(counter_name);
                if (profile == nullptr || profile->calls == 0)
                {
                    return 0.0;
                }
                double result = profile->accumulated / profile->calls;
                return result;
            };
            request = new apex_tuning_request(name);
            request->set_metric(metric);
            request->set_strategy(apex_ah_tuning_strategy::EXHAUSTIVE);
            request->add_param_long("parcel_count", 2, 2, 256, 2);
            request->add_param_long("buffer_time", 1000, 1000, 5000, 1000);
            request->set_trigger(apex::register_custom_event(name));
            apex::setup_custom_tuning(*request);

            policy_handle = apex::register_policy(APEX_SEND, policy);
            if (policy_handle == nullptr)
            {
                std::cerr << "Error registering policy!" << std::endl;
            }
        }

        static void initialize()
        {
            if (instance == nullptr)
            {
                instance = new apex_parcel_coalescing_policy();
            }
        }
        static void finalize()
        {
            if (instance != nullptr)
            {
                delete instance;
                instance = nullptr;
            }
        }
    };
#endif //HPX_HAVE_APEX && HPX_HAVE_PARCEL_COALESCING
}}

