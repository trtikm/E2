#include <qtgl/simulation_time_config.hpp>
#include <qtgl/real_time_simulator.hpp>
#include <utility/invariants.hpp>
#include <algorithm>

namespace qtgl {


simulation_time_config::simulation_time_config()
    : m_pause_state(PAUSE_STATE::PAUSED)
    , m_fixed_time_step_use(FIXED_TIME_STEP_USE::FOR_SINGLE_STEP_ONLY)
    , m_fixed_time_step_in_seconds(1.0f / 30.0f)
    , m_longest_time_step_in_seconds(1.0f / 30.0f)
    , m_previous_pause_state(PAUSE_STATE::NOT_PAUSED)
{}

float_32_bit  simulation_time_config::get_clipped_simulation_time_step_in_seconds(
        float_32_bit const  duration_of_last_time_step_in_seconds
        ) const
{
    return std::min(m_longest_time_step_in_seconds, get_raw_simulation_time_step_in_seconds(duration_of_last_time_step_in_seconds));
}

float_32_bit  simulation_time_config::get_raw_simulation_time_step_in_seconds(
        float_32_bit const  duration_of_last_time_step_in_seconds
        ) const
{
    return m_fixed_time_step_use == FIXED_TIME_STEP_USE::ALWAYS ||
                (m_fixed_time_step_use == FIXED_TIME_STEP_USE::FOR_SINGLE_STEP_ONLY &&
                    m_pause_state == PAUSE_STATE::UNPAUSED_FOR_SINGLE_STEP_ONLY)
            ? m_fixed_time_step_in_seconds : duration_of_last_time_step_in_seconds;
}


simulation_time_config::auto_updater::auto_updater(simulation_time_config& cfg, qtgl::real_time_simulator* const  simulator_or_null)
    : config_ptr(&cfg)
    , config_working_copy(cfg)
    , simulator_ptr(simulator_or_null)
{
    INVARIANT(config_working_copy.m_previous_pause_state != PAUSE_STATE::UNPAUSED_FOR_SINGLE_STEP_ONLY);
    if (config_working_copy.m_pause_state != config_working_copy.m_previous_pause_state)
    {
        if (simulator_ptr != nullptr && (
                config_working_copy.m_pause_state == PAUSE_STATE::PAUSED ||
                config_working_copy.m_previous_pause_state == PAUSE_STATE::PAUSED))
        {
            if (config_working_copy.is_paused())
                simulator_ptr->on_simulation_paused();
            else
                simulator_ptr->on_simulation_resumed();
        }
        config_working_copy.m_previous_pause_state = config_working_copy.m_pause_state;
        config_ptr->m_previous_pause_state = config_working_copy.m_pause_state;
    }
}

simulation_time_config::auto_updater::~auto_updater()
{
    if (config_working_copy.m_pause_state == PAUSE_STATE::UNPAUSED_FOR_SINGLE_STEP_ONLY)
    {
        if (simulator_ptr != nullptr)
            simulator_ptr->on_simulation_paused();
        config_working_copy.m_previous_pause_state = PAUSE_STATE::PAUSED;
        config_working_copy.m_pause_state = PAUSE_STATE::PAUSED;
        config_ptr->m_previous_pause_state = PAUSE_STATE::PAUSED;
        config_ptr->m_pause_state = PAUSE_STATE::PAUSED;
    }
}


}
