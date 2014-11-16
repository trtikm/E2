#include "./program_info.hpp"

std::string  get_program_name()
{
    return "synchronisation_of_tissue_algorithms";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return "This program tests correctness of synchronisation of tissue algorithms.\n"
           "It is done such that each neuron, synapse, and teritorie contains just\n"
           "one unsigned number which is initialised to zero and it is incremented\n"
           "by one each time its update is issued from a tissue algorithm. Whenever\n"
           "a tissue algorithm finishes its job the test checks for correct values\n"
           "in the unsigned numbers in all tissue elements. The test is applied to\n"
           "several tissues differing in properties like number of cells, synapses,\n"
           "kinds of cells, etc."
           ;
}
