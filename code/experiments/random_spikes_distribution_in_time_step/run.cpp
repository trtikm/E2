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
#include <vector>
#include <ostream>
#include <algorithm>


using  spikes_distribution = std::vector<natural_32_bit>;


struct  experiment_setup
{
    natural_32_bit  NUM_TRIALS;
    natural_32_bit  NUM_SECONDS_TO_SIMULATE;
    natural_32_bit  SIMULATION_FREQUENCY;
    natural_32_bit  SPIKING_FREQUENCY;
    natural_32_bit  NUM_SPIKE_TRAINS;
    natural_32_bit  MIN_SPIKE_DISTANCE;
    natural_32_bit  SEED;
};


void  perform_trial(spikes_distribution&  distribution, natural_32_bit const  trial_id, experiment_setup const&  setup)
{
    TMPROF_BLOCK();

    natural_32_bit const  NUM_TIME_STEPS = setup.NUM_SECONDS_TO_SIMULATE * setup.SIMULATION_FREQUENCY;
    std::vector<natural_32_bit>  sum_spike_history(NUM_TIME_STEPS, 0U);
    {
        natural_32_bit const  NUM_SPIKES = setup.NUM_SECONDS_TO_SIMULATE * setup.SPIKING_FREQUENCY;
        std::vector<bool>  spike_history(NUM_TIME_STEPS);
        for (natural_32_bit  i = 0U; i != setup.NUM_SPIKE_TRAINS; ++i)
        {
            std::fill(spike_history.begin(), spike_history.end(), 0U);
            random_generator_for_natural_32_bit  generator;
            reset(generator, setup.SEED * (trial_id + 1U) * (i + 1U));
            for (natural_32_bit  j = 0U; j != NUM_SPIKES; ++j)
            {
                while (true)
                {
                    natural_32_bit  k = get_random_natural_32_bit_in_range(0U, NUM_TIME_STEPS - 1U, generator);

                    bool  ok = true;
                    for (natural_32_bit  u = k, v = 0U; u > 0U && v < setup.MIN_SPIKE_DISTANCE; --u, ++v)
                        if (spike_history.at(u))
                        {
                            ok = false;
                            break;
                        }
                    if (!ok)
                        continue;
                    for (natural_32_bit  u = k + 1U, v = 1U; u < NUM_TIME_STEPS - 1U && v < setup.MIN_SPIKE_DISTANCE; ++u, ++v)
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
                    ++sum_spike_history.at(j);
        }
    }

    distribution.clear();
    distribution.resize(setup.NUM_SPIKE_TRAINS + 1U, 0U);
    for (natural_32_bit  j = 0U; j != NUM_TIME_STEPS; ++j)
        ++distribution.at(sum_spike_history.at(j));
}


void  compute_distribution(spikes_distribution&  distribution, experiment_setup const&  setup)
{
    TMPROF_BLOCK();

    ASSUMPTION(setup.NUM_TRIALS > 0U);
    distribution.clear();
    distribution.resize(setup.NUM_SPIKE_TRAINS + 1U, 0U);
    spikes_distribution  trial_distribution;
    for (natural_32_bit  i = 0U; i != setup.NUM_TRIALS; ++i)
    {
        perform_trial(trial_distribution, i, setup);
        for (natural_32_bit  j = 0U; j != setup.NUM_SPIKE_TRAINS + 1U; ++j)
            distribution.at(j) += trial_distribution.at(j);
    }
    for (natural_32_bit  j = 0U; j != setup.NUM_SPIKE_TRAINS + 1U; ++j)
        distribution.at(j) = (natural_32_bit)((float_32_bit)distribution.at(j) / (float_32_bit)setup.NUM_TRIALS + 0.5f);
}


void  save_distribution(spikes_distribution const&  distribution, natural_32_bit const  ID, experiment_setup const&  setup)
{
    boost::filesystem::path  dir = canonical_path(get_program_options()->outputDir());
    boost::filesystem::create_directories(dir);

    std::string const  fname =
        msgstream() << "exp" << ID
                    << "_trains_" << setup.NUM_SPIKE_TRAINS
                    << "_spiking_" << setup.SPIKING_FREQUENCY << "Hz"
                    << "_simul_" << setup.SIMULATION_FREQUENCY << "Hz"
                    << ".txt"
                    ;
    std::string const  pathname = (dir / fname).string();

    std::ofstream  ofile(pathname.c_str(), std::ios_base::out);
    ofile << "# The file contains a distribution of expected counts of spikes" << std::endl;
    ofile << "# arriving to a neuron in a simulation time step." << std::endl;
    ofile << "# EXPERIMENT SETUP:" << std::endl;
    ofile << "#     NUM_TRIALS=" << setup.NUM_TRIALS << std::endl;
    ofile << "#     NUM_SECONDS_TO_SIMULATE=" << setup.NUM_SECONDS_TO_SIMULATE << std::endl;
    ofile << "#     SIMULATION_FREQUENCY=" << setup.SIMULATION_FREQUENCY << std::endl;
    ofile << "#     SPIKING_FREQUENCY=" << setup.SPIKING_FREQUENCY << std::endl;
    ofile << "#     NUM_SPIKE_TRAINS=" << setup.NUM_SPIKE_TRAINS << std::endl;
    ofile << "#     MIN_SPIKE_DISTANCE=" << setup.MIN_SPIKE_DISTANCE << std::endl;
    ofile << "#     SEED=" << setup.SEED << std::endl;
    ofile << "# DISTRIBUTION:" << std::endl;
    ofile << "#     First column:  How many spikes arrived to a neuron in a simulation time step." << std::endl;
    ofile << "#     Second column: How many times that happened during the entire simulation." << std::endl;
    ofile << "#     NOTE: The distribution does not contain rows whose second column is smaller" << std::endl;
    ofile << "#           than 1% of a maximal value in the second column in the file." << std::endl;
    natural_32_bit const  max_count = std::max(1U, *std::max_element(distribution.begin(), distribution.end()));
    for (natural_32_bit  i = 0U; i != distribution.size(); ++i)
        if ((float_32_bit)distribution.at(i) / (float_32_bit)max_count >= 0.01f)
            ofile << i << ' ' << distribution.at(i) << std::endl;

}


void run(int argc, char* argv[])
{
    TMPROF_BLOCK();

    std::vector<experiment_setup>  setups {
        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            100U,   // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            100U,   // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },
        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            100U,   // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            500U,   // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },
        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            100U,   // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            1500U,  // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },
        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            100U,   // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            10000U, // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },

        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            200U,   // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            100U,   // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },
        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            200U,   // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            500U,   // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },
        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            200U,   // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            1500U,  // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },
        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            200U,   // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            10000U, // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            151U    // SEED
        },

        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            500U,   // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            100U,   // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },
        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            500U,   // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            500U,   // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },
        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            500U,   // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            1500U,  // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },
        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            500U,   // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            10000U, // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },

        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            1000U,  // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            100U,   // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },
        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            1000U,  // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            500U,   // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },
        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            1000U,  // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            1500U,  // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },
        {
            10U,    // NUM_TRIALS
            10U,    // NUM_SECONDS_TO_SIMULATE
            1000U,  // SIMULATION_FREQUENCY
            20U,    // SPIKING_FREQUENCY
            10000U, // NUM_SPIKE_TRAINS
            2U,     // MIN_SPIKE_DISTANCE
            101U    // SEED
        },
    };
    for (natural_32_bit  i = 0U; i != setups.size(); ++i)
    {
        spikes_distribution  distribution;
        compute_distribution(distribution, setups.at(i));
        save_distribution(distribution, i, setups.at(i));
    }
}
