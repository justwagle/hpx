//  Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx_init.hpp>
#include <hpx/hpx.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/async.hpp>
#include <hpx/include/serialization.hpp>
#include <hpx/runtime/config_entry.hpp>
#include <hpx/util/apex_coalescing_policy_basic.hpp>

#include <chrono>
#include <cstddef>
#include <complex>
#include <string>
#include <vector>
#include <hpx/plugins/parcel/coalescing_message_handler.hpp>

double duration = 101;

namespace pingpong
{
    namespace server
    {
        std::complex<double> get_element()
        {
            return std::complex<double>(13.3,-23.8);
        }
    }
}

HPX_PLAIN_ACTION(pingpong::server::get_element, pingpong_get_element_action);
HPX_ACTION_USES_MESSAGE_COALESCING(pingpong_get_element_action);


int hpx_main(boost::program_options::variables_map& vm)
{
    //Exit if not run on two localities
    if(hpx::get_num_localities().get() != 2)
    {
        std::cout<<"ERROR 42: !!Need to run with exactly 2 localities!!"<<std::endl;
        return hpx::finalize();
    }

    //Commandline specific code
    std::size_t const n = vm["nparcels"].as<std::size_t>();

    if (0 == hpx::get_locality_id())
    {
        hpx::cout << "Running With nparcel = " << n << "\n" << hpx::flush;
    }
    
    hpx::util::apex_parcel_coalescing_policy_basic::initialize(duration);
 
    //Create instance of the actions
    pingpong_get_element_action act;
    std::vector<hpx::future<std::complex<double>>> recieved(n);
    std::vector<hpx::future<std::complex<double>>> vec(n);
    std::vector<hpx::future<std::complex<double>>> vec2(n);
    std::vector<hpx::future<std::complex<double>>> recieved2(n);

    //Find the other locality

    std::vector<hpx::naming::id_type> dummy = hpx::find_remote_localities();
    hpx::naming::id_type other_locality = dummy[0];
    hpx::evaluate_active_counters(true, "Finished Initialization");

for(std::size_t j = 0; j < 35; j++) {

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < n; ++i) {
        //calling the custom event
        //apex::custom_event(hpx::util::apex_parcel_coalescing_policy::return_apex_parcel_coalescing_event(), NULL);
            
        vec[i] = hpx::async(act, other_locality);
    }


    hpx::wait_all(vec);
    hpx::evaluate_active_counters(false, "First Done");

    std::chrono::steady_clock::time_point end= std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    //std::cout << "First Time difference = " << duration <<std::endl;
    apex::custom_event(hpx::util::apex_parcel_coalescing_policy_basic::return_apex_parcel_coalescing_event(), NULL);

    printf("first done \n");
    fflush(stdout);

    begin = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < n; ++i) {
        vec2[i] = hpx::async(act, other_locality);
    }


    hpx::wait_all(vec2);
    hpx::evaluate_active_counters(false, " Second Done");
    end = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    //std::cout << "Second Time difference = " << duration <<std::endl;
    apex::custom_event(hpx::util::apex_parcel_coalescing_policy_basic::return_apex_parcel_coalescing_event(), NULL);


    printf("Second done \n");
    fflush(stdout);

    begin = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < n; ++i) {
        recieved[i] = hpx::async(act, other_locality);
    }

    hpx::wait_all(recieved);
    hpx::evaluate_active_counters(false, " Third Done");
    end= std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    //std::cout << "third Time difference = " << duration <<std::endl;
    apex::custom_event(hpx::util::apex_parcel_coalescing_policy_basic::return_apex_parcel_coalescing_event(), NULL);



    printf("Third done \n");
    fflush(stdout);

    begin = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < n; ++i) {
        recieved2[i] = hpx::async(act, other_locality);
    }
    hpx::wait_all(recieved2);
    hpx::evaluate_active_counters(true, "All Done");
    end= std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    //std::cout << "fourth Time difference = " << duration <<std::endl;
    apex::custom_event(hpx::util::apex_parcel_coalescing_policy_basic::return_apex_parcel_coalescing_event(), NULL);


}
    printf("All done \n"); fflush(stdout);
    hpx::util::apex_parcel_coalescing_policy_basic::finalize();

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // Configure application-specific options
    boost::program_options::options_description cmdline(
            "Usage: " HPX_APPLICATION_STRING " [options]");

    cmdline.add_options()
            ("nparcels,n",
             boost::program_options::value<std::size_t>()->default_value(1000),
             "the number of parcels to create");
    // Initialize and run HPX
    std::vector<std::string> cfg;
    cfg.push_back("hpx.run_hpx_main!=1");
    return hpx::init(cmdline,argc, argv, cfg);
}

