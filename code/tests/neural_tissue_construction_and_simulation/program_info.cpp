#include "./program_info.hpp"

std::string  get_program_name()
{
    return "neural_tissue_construction_and_simulation";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return "This program tests correctness of creation of a neural tissue instance.\n"
           "There are 4 constructors in 'struct cellab::neural_tissue' so the program\n"
           "has 4 corresponding variants. Moreover, for each variant a simulation is\n"
           "performed, i.e. all the simulation algorithms are called. A neuron, synapse,\n"
           "and singnalling is implemented as a separate data type comprising just one\n"
           "integer counter of updates issued from tissue algorithms. The test checks\n"
           "for correct values in the counters during exection of each tissue algorithm\n"
           "and also in between calls two different tissue algorithms.\n"
           ;
}
