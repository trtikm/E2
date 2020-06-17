#include <random_spikes_distribution_in_time_step/program_info.hpp>
#include <random_spikes_distribution_in_time_step/program_options.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/random.hpp>
#include <utility/msgstream.hpp>
#include <utility/canonical_path.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>


using  spikes_distribution = std::map<integer_32_bit, natural_32_bit>;


struct  experiment_setup
{
    natural_32_bit  NUM_TRIALS;
    natural_32_bit  NUM_SECONDS_TO_SIMULATE;
    natural_32_bit  SIMULATION_FREQUENCY;
    natural_32_bit  SPIKING_FREQUENCY_EXCITATORY;
    natural_32_bit  SPIKING_FREQUENCY_INHIBITORY;
    natural_32_bit  NUM_SPIKE_TRAINS_EXCITATORY;
    natural_32_bit  NUM_SPIKE_TRAINS_INHIBITORY;
    integer_32_bit  SPIKE_MAGNITUDE_EXCITATORY;
    integer_32_bit  SPIKE_MAGNITUDE_INHIBITORY;
    natural_32_bit  MIN_SPIKE_DISTANCE;
    natural_32_bit  SEED;
    natural_32_bit  ID;
};


void  extend_spikes_history(
        std::vector<integer_32_bit>& output_spike_history,
        natural_32_bit const  NUM_TIME_STEPS,
        natural_32_bit const  NUM_SPIKE_TRAINS,
        natural_32_bit const  NUM_SPIKES_PER_TRAIN,
        natural_32_bit const  MIN_SPIKE_DISTANCE,
        integer_32_bit const  SPIKE_AMPLITUDE,
        natural_32_bit const  SEED
        )
{
    std::vector<bool>  spike_history(NUM_TIME_STEPS);
    for (natural_32_bit  i = 0U; i != NUM_SPIKE_TRAINS; ++i)
    {
        std::fill(spike_history.begin(), spike_history.end(), false);
        random_generator_for_natural_32_bit  generator;
        reset(generator, SEED + i);
        for (natural_32_bit  j = 0U; j != NUM_SPIKES_PER_TRAIN; ++j)
        {
            while (true)
            {
                natural_32_bit  k = get_random_natural_32_bit_in_range(0U, NUM_TIME_STEPS - 1U, generator);

                if (spike_history.at(k))
                    continue;
                bool  ok = true;
                for (natural_32_bit  u = k, v = 0U; u > 0U && v < MIN_SPIKE_DISTANCE; --u, ++v)
                    if (spike_history.at(u))
                    {
                        ok = false;
                        break;
                    }
                if (!ok)
                    continue;
                for (natural_32_bit  u = k + 1U, v = 1U; u < NUM_TIME_STEPS - 1U && v < MIN_SPIKE_DISTANCE; ++u, ++v)
                    if (spike_history.at(u))
                    {
                        ok = false;
                        break;
                    }
                if (!ok)
                    continue;

                spike_history.at(k) = true;
                break;
            }
        }
        for (natural_32_bit  j = 0U; j != NUM_TIME_STEPS; ++j)
            if (spike_history.at(j))
                output_spike_history.at(j) += SPIKE_AMPLITUDE;
    }
}


void  perform_trial(spikes_distribution&  distribution, natural_32_bit const  trial_id, experiment_setup const&  setup)
{
    TMPROF_BLOCK();

    natural_32_bit const  NUM_TIME_STEPS = setup.NUM_SECONDS_TO_SIMULATE * setup.SIMULATION_FREQUENCY;
    natural_32_bit const  NUM_SPIKE_TRAINS = setup.NUM_SPIKE_TRAINS_EXCITATORY + setup.NUM_SPIKE_TRAINS_INHIBITORY;
    natural_32_bit const  SEED = setup.SEED + trial_id * NUM_SPIKE_TRAINS;

    std::vector<integer_32_bit>  spike_history(NUM_TIME_STEPS, 0U);
    extend_spikes_history(
            spike_history,
            NUM_TIME_STEPS,
            setup.NUM_SPIKE_TRAINS_EXCITATORY,
            setup.NUM_SECONDS_TO_SIMULATE * setup.SPIKING_FREQUENCY_EXCITATORY,
            setup.MIN_SPIKE_DISTANCE,
            setup.SPIKE_MAGNITUDE_EXCITATORY,
            SEED
            );
    extend_spikes_history(
            spike_history,
            NUM_TIME_STEPS,
            setup.NUM_SPIKE_TRAINS_INHIBITORY,
            setup.NUM_SECONDS_TO_SIMULATE * setup.SPIKING_FREQUENCY_INHIBITORY,
            setup.MIN_SPIKE_DISTANCE,
            setup.SPIKE_MAGNITUDE_INHIBITORY,
            SEED + setup.NUM_SPIKE_TRAINS_EXCITATORY
            );

    for (natural_32_bit  j = 0U; j != NUM_TIME_STEPS; ++j)
    {
        auto  it = distribution.find(spike_history.at(j));
        if (it == distribution.end())
            distribution[spike_history.at(j)] = 1;
        else
            ++it->second;
    }
}


void  compute_distribution(spikes_distribution&  distribution, experiment_setup const&  setup)
{
    TMPROF_BLOCK();

    ASSUMPTION(setup.NUM_TRIALS > 0U);
    for (natural_32_bit  i = 0U; i != setup.NUM_TRIALS; ++i)
    {
        spikes_distribution  trial_distribution;
        perform_trial(trial_distribution, i, setup);
        for (auto const&  elem : trial_distribution)
        {
            auto  it = distribution.find(elem.first);
            if (it == distribution.end())
                distribution[elem.first] = elem.second;
            else
                it->second += elem.second;
        }
    }
    for (auto&  elem : distribution)
        elem.second = (natural_32_bit)((float_32_bit)elem.second / (float_32_bit)setup.NUM_TRIALS + 0.5f);
}


void  save_distribution(spikes_distribution const&  distribution, experiment_setup const&  setup)
{
    boost::filesystem::path  dir = canonical_path(get_program_options()->outputDir());
    boost::filesystem::create_directories(dir);

    std::string const  fname =
        msgstream() << "exp" << setup.ID
                    << "_trains_" << setup.NUM_SPIKE_TRAINS_EXCITATORY << "e_" << setup.NUM_SPIKE_TRAINS_INHIBITORY << "i"
                    << "_spiking_" << setup.SPIKING_FREQUENCY_EXCITATORY << "eHz_" << setup.SPIKING_FREQUENCY_INHIBITORY << "iHz"
                    << "_simul_" << setup.SIMULATION_FREQUENCY << "Hz"
                    << ".txt"
                    ;
    std::string const  pathname = (dir / fname).string();

    std::ofstream  ofile(pathname.c_str(), std::ios_base::out);
    ofile << "# The file contains a distribution of expected spiking current" << std::endl;
    ofile << "# arriving to a neuron in a simulation time step." << std::endl;
    ofile << "# EXPERIMENT SETUP:" << std::endl;
    ofile << "#     NUM_TRIALS=" << setup.NUM_TRIALS << std::endl;
    ofile << "#     NUM_SECONDS_TO_SIMULATE=" << setup.NUM_SECONDS_TO_SIMULATE << std::endl;
    ofile << "#     SIMULATION_FREQUENCY=" << setup.SIMULATION_FREQUENCY << std::endl;
    ofile << "#     SPIKING_FREQUENCY_EXCITATORY=" << setup.SPIKING_FREQUENCY_EXCITATORY << std::endl;
    ofile << "#     SPIKING_FREQUENCY_INHIBITORY=" << setup.SPIKING_FREQUENCY_INHIBITORY << std::endl;
    ofile << "#     NUM_SPIKE_TRAINS_EXCITATORY=" << setup.NUM_SPIKE_TRAINS_EXCITATORY << std::endl;
    ofile << "#     NUM_SPIKE_TRAINS_INHIBITORY=" << setup.NUM_SPIKE_TRAINS_INHIBITORY << std::endl;
    ofile << "#     SPIKE_MAGNITUDE_EXCITATORY=" << setup.SPIKE_MAGNITUDE_EXCITATORY << std::endl;
    ofile << "#     SPIKE_MAGNITUDE_INHIBITORY=" << setup.SPIKE_MAGNITUDE_INHIBITORY << std::endl;
    ofile << "#     MIN_SPIKE_DISTANCE=" << setup.MIN_SPIKE_DISTANCE << std::endl;
    ofile << "#     SEED=" << setup.SEED << std::endl;
    ofile << "#     ID=" << setup.ID << std::endl;
    ofile << "# DISTRIBUTION:" << std::endl;
    ofile << "#     First column:  What spiking current arrived to a neuron in a simulation time step." << std::endl;
    ofile << "#     Second column: How many times that happened during the entire simulation." << std::endl;
    ofile << "#     NOTE: The distribution does not contain rows whose second column is smaller" << std::endl;
    ofile << "#           than 1% of a maximal value in the second column in the file." << std::endl;

    natural_32_bit  max_count = 1U;
    for (auto const&  elem : distribution)
        if (max_count < elem.second)
            max_count = elem.second;

    for (auto const&  elem : distribution)
        if ((float_32_bit)elem.second / (float_32_bit)max_count >= 0.01f)
            ofile << elem.first << ' ' << elem.second << std::endl;

}


void run(int argc, char* argv[])
{
    TMPROF_BLOCK();

    if (get_program_options()->genJson())
    {
        boost::property_tree::ptree  ptree;
        ptree.put("NUM_TRIALS", 10U);
        ptree.put("NUM_SECONDS_TO_SIMULATE", 10U);
        ptree.put("SIMULATION_FREQUENCY", 100U);
        ptree.put("SPIKING_FREQUENCY_EXCITATORY", 20U);
        ptree.put("SPIKING_FREQUENCY_INHIBITORY", 20U);
        ptree.put("NUM_SPIKE_TRAINS_EXCITATORY", 100U);
        ptree.put("NUM_SPIKE_TRAINS_INHIBITORY", 100U);
        ptree.put("SPIKE_MAGNITUDE_EXCITATORY", 1);
        ptree.put("SPIKE_MAGNITUDE_INHIBITORY", -1);
        ptree.put("MIN_SPIKE_DISTANCE", 2U);
        ptree.put("SEED", 101U);
        ptree.put("ID", 0U);

        std::string const  pathname = "./" + get_program_name() + ".json";
        std::ofstream  ostr(pathname.c_str(), std::ios_base::out);

        boost::property_tree::write_json(ostr, ptree);
    }

    experiment_setup  setup;
    {
        boost::property_tree::ptree  ptree;
        {
            std::ifstream  istr(get_program_options()->inputJson().c_str(), std::ios_base::in);
            if (istr.is_open())
                boost::property_tree::read_json(istr, ptree);
            else
                std::cout << "Cannot read JSON setup file '" << get_program_options()->inputJson()
                          << "'. The default setup will be used." << std::endl;
        }

        setup.NUM_TRIALS = ptree.get<natural_32_bit>("NUM_TRIALS", 10U);
        setup.NUM_SECONDS_TO_SIMULATE = ptree.get<natural_32_bit>("NUM_SECONDS_TO_SIMULATE", 10U);
        setup.SIMULATION_FREQUENCY = ptree.get<natural_32_bit>("SIMULATION_FREQUENCY", 100U);
        setup.SPIKING_FREQUENCY_EXCITATORY = ptree.get<natural_32_bit>("SPIKING_FREQUENCY_EXCITATORY", 20U);
        setup.SPIKING_FREQUENCY_INHIBITORY = ptree.get<natural_32_bit>("SPIKING_FREQUENCY_INHIBITORY", 20U);
        setup.NUM_SPIKE_TRAINS_EXCITATORY = ptree.get<natural_32_bit>("NUM_SPIKE_TRAINS_EXCITATORY", 100U);
        setup.NUM_SPIKE_TRAINS_INHIBITORY = ptree.get<natural_32_bit>("NUM_SPIKE_TRAINS_INHIBITORY", 100U);
        setup.SPIKE_MAGNITUDE_EXCITATORY = ptree.get<integer_32_bit>("SPIKE_MAGNITUDE_EXCITATORY", 1);
        setup.SPIKE_MAGNITUDE_INHIBITORY = ptree.get<integer_32_bit>("SPIKE_MAGNITUDE_INHIBITORY", -1);
        setup.MIN_SPIKE_DISTANCE = ptree.get<natural_32_bit>("MIN_SPIKE_DISTANCE", 2U);
        setup.SEED = ptree.get<natural_32_bit>("SEED", 101U);
        setup.ID = ptree.get<natural_32_bit>("ID", 0U);
    }
    spikes_distribution  distribution;
    compute_distribution(distribution, setup);
    save_distribution(distribution, setup);
}
