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

#include <cstddef>
#include <complex>
#include <string>
#include <vector>
#include <hpx/plugins/parcel/coalescing_message_handler.hpp>


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

    for(std::size_t i=0; i<n; ++i)
    {
        vec[i]=hpx::async(act,other_locality);
    }

    hpx::wait_all(vec);
    hpx::evaluate_active_counters(false, "First Done");

    for(std::size_t i=0; i<n; ++i)
    {
        vec2[i]=hpx::async(act,other_locality);
    }


    hpx::wait_all(vec2) ;
    hpx::evaluate_active_counters(false," Second Done");

    for(std::size_t i=0; i<n; ++i)
    {
        recieved[i]=hpx::async(act,other_locality);
    }

    hpx::wait_all(recieved) ;
    hpx::evaluate_active_counters(false," Third Done");

    for(std::size_t i=0; i<n; ++i)
    {
        recieved2[i]=hpx::async(act,other_locality);
    }
    hpx::wait_all(recieved2) ;
    hpx::evaluate_active_counters(true, "All Done");

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // Configure application-specific options
    boost::program_options::options_description cmdline(
            "Usage: " HPX_APPLICATION_STRING " [options]");

    cmdline.add_options()
            ("nparcels,n",
             boost::program_options::value<std::size_t>()->default_value(100000),
             "the number of parcels to create");
    // Initialize and run HPX
    std::vector<std::string> cfg;
    cfg.push_back("hpx.run_hpx_main!=1");
    return hpx::init(cmdline,argc, argv, cfg);
}