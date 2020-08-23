#include <netlab_cortex_functions/program_info.hpp>
#include <netlab_cortex_functions/program_options.hpp>
#include <netlab_cortex_functions/exp_inhibitory_weight.hpp>
#include <netlab_cortex_functions/exp_spiking_const_input_signal.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <iostream>
#include <memory>


void run(int argc, char* argv[])
{
    std::string const  experiment = get_program_options()->experiment();
    if (experiment == "inhibitory_weight")
        osi::run(std::make_unique<exp_inhibitory_weight>());
    else if (experiment == "spiking_const_input_signal")
        osi::run(std::make_unique<exp_spiking_const_input_signal>());
    else
        std::cout << "ERROR: unknown experiment name: " << experiment << std::endl
                  << "Choose one of these (option --exp):" << std::endl
                  << "    " << "inhibitory_weight" << std::endl
                  << "    " << "spiking_const_input_signal" << std::endl
                  ;
}
