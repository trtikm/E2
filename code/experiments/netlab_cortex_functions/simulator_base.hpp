#ifndef E2_EXPERIMENTS_NETLAB_CORTEX_FUNCTIONS_SIMULATOR_BASE_HPP_INCLUDED
#   define E2_EXPERIMENTS_NETLAB_CORTEX_FUNCTIONS_SIMULATOR_BASE_HPP_INCLUDED

#   include <com/simulator.hpp>
#   include <netlab/cortex.hpp>
#   include <string>

struct  simulator_base : public com::simulator
{
    simulator_base();

    void  initialise() override final;
    void  custom_module_round() override final;
    void  on_begin_round() override final;
    void  on_begin_render() override final;

    bool  is_shift_down() const;
    bool  is_ctrl_down() const;
    bool  is_alt_down() const;
    bool  is_key_pressed(osi::keyboard_key_name const&  key) const;
    bool  is_key_just_pressed(osi::keyboard_key_name const&  key) const;

    std::string  get_experiment_dir() const;

    virtual bool  move_to_next_phase() { return false; }
    virtual void  network_setup() = 0;
    virtual void  network_update() = 0;
    virtual void  scene_setup() {}
    virtual void  scene_update() {}
    virtual void  custom_render() {}
    virtual void  on_restart() {}
    virtual void  help() {}

protected:

    netlab::cortex  cortex;
};


#endif
