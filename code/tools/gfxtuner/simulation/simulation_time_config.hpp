#ifndef E2_TOOL_GFXTUNER_SIMULATION_TIME_CONFIG_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_SIMULATION_TIME_CONFIG_HPP_INCLUDED

#   include <gfxtuner/simulation/simulator_notifications.hpp>
#   include <qtgl/real_time_simulator.hpp>


struct  simulation_time_config
{
    enum struct  PAUSE_STATE
    {
        NOT_PAUSED = 0,
        PAUSED = 1,
        UNPAUSED_FOR_SINGLE_STEP_ONLY = 2
    };

    enum struct  FIXED_TIME_STEP_USE
    {
        NEVER = 0,
        ALWAYS = 1,
        FOR_SINGLE_STEP_ONLY = 2
    };

    struct  auto_updater;

    simulation_time_config();

    bool  is_paused() const { return m_pause_state == PAUSE_STATE::PAUSED; }
    void  set_paused(bool const  state) { m_pause_state = state ? PAUSE_STATE::PAUSED : PAUSE_STATE::NOT_PAUSED; }
    void  toggle_pause() { m_pause_state = m_pause_state == PAUSE_STATE::PAUSED ? PAUSE_STATE::NOT_PAUSED : PAUSE_STATE::PAUSED; }

    void  perform_single_step() { m_pause_state = PAUSE_STATE::UNPAUSED_FOR_SINGLE_STEP_ONLY; }

    void  set_fixed_time_step_in_seconds(float_32_bit const  dt) { m_fixed_time_step_in_seconds = dt; }
    void  set_fixed_step_usage(FIXED_TIME_STEP_USE const  usage) { m_fixed_time_step_use = usage; }

    void  set_longest_time_step_in_seconds(float_32_bit const  dt) { m_longest_time_step_in_seconds = dt; }

    float_32_bit  get_clipped_simulation_time_step_in_seconds(float_32_bit const  duration_of_last_time_step_in_seconds) const;
    float_32_bit  get_raw_simulation_time_step_in_seconds(float_32_bit const  duration_of_last_time_step_in_seconds) const;

    PAUSE_STATE  m_pause_state;
    FIXED_TIME_STEP_USE  m_fixed_time_step_use;
    float_32_bit  m_fixed_time_step_in_seconds;
    float_32_bit  m_longest_time_step_in_seconds;
private:
    PAUSE_STATE  m_previous_pause_state;
};


struct  simulation_time_config::auto_updater
{
    auto_updater(simulation_time_config& cfg, qtgl::real_time_simulator* const  simulator_or_null);
    ~auto_updater();

    simulation_time_config const&  operator()() const { return config_working_copy; }

private:
    simulation_time_config* config_ptr;
    simulation_time_config  config_working_copy;
    qtgl::real_time_simulator*  simulator_ptr;
};


#endif
