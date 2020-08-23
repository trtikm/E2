#ifndef E2_EXPERIMENTS_NETLAB_CORTEX_FUNCTIONS_EXP_INHIBITORY_WEIGHT_HPP_INCLUDED
#   define E2_EXPERIMENTS_NETLAB_CORTEX_FUNCTIONS_EXP_INHIBITORY_WEIGHT_HPP_INCLUDED

#   include <netlab_cortex_functions/simulator_base.hpp>
#   include <netlab/cortex.hpp>
#   include <com/simulator.hpp>


struct  exp_inhibitory_weight : public simulator_base
{
    void  network_setup() override;
    void  network_update() override;
    void  scene_setup() override;
    void  scene_update() override;
    void  help() override;

private:
    netlab::cortex::layer_index  src_layer_idx;
    netlab::cortex::layer_index  dst_layer_idx;

    com::object_guid  weight_frame_guid;
    com::object_guid  membrane_potential_frame_guid;
};


#endif
