#include <ai/agent_system_variables.hpp>
#include <ai/agent_system_state.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


agent_system_variables  load_agent_system_variables()
{
    return agent_system_variables{
            { "sys_zero", 0.0f },
            { "sys_one", 1.0f },
            { "sys_animation_speed", 1.0f },
            { "sys_angle_look_forward", 0.0f },
            { "sys_angle_look_attractor_xp", 0.0f },
            { "sys_angle_look_attractor_xn", 0.0f },
            { "sys_angle_look_attractor_yp", 0.0f },
            { "sys_angle_look_attractor_yn", 0.0f },
            };
}


void  update_system_variables(agent_system_variables&  variables, agent_system_state const&  state)
{
    TMPROF_BLOCK();

    variables["sys_angle_look_forward"] = // in range <0.0f, PI()/2.0f>
            angle(state.motion_object_frame.basis_vector_y(), state.camera_frame.basis_vector_z());

    variables["sys_angle_look_attractor_xp"] = // in range <0.0f, PI()>
            angle(vector3{5.0f, 0.0f, 0.0f}, -state.camera_frame.basis_vector_z());
    variables["sys_angle_look_attractor_xn"] = // in range <0.0f, PI()>
            angle(vector3{-5.0f, 0.0f, 0.0f}, -state.camera_frame.basis_vector_z());
    variables["sys_angle_look_attractor_yp"] = // in range <0.0f, PI()>
            angle(vector3{0.0f, 5.0f, 0.0f}, -state.camera_frame.basis_vector_z());
    variables["sys_angle_look_attractor_yn"] = // in range <0.0f, PI()>
            angle(vector3{0.0f, -5.0f, 0.0f}, -state.camera_frame.basis_vector_z());
}


}
