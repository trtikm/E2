#include <ai/action_controller_human.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai {


action_controller_human::action_controller_human(blackboard_ptr const  blackboard_)
    : action_controller(blackboard_)
{
    TMPROF_BLOCK();
}


action_controller_human::~action_controller_human()
{
    TMPROF_BLOCK();
}


void  action_controller_human::next_round_internal(float_32_bit  time_step_in_seconds)
{
    TMPROF_BLOCK();

    m_motion_desire_props.linear_velocity_unit_direction_in_world_space =
            normalised(
                    quaternion_to_rotation_matrix(
                            angle_axis_to_quaternion(
                                    get_blackboard()->m_cortex_cmd_turn_intensity
                                        * get_blackboard()->m_max_turn_speed_in_radians_per_second
                                        * time_step_in_seconds,
                                    vector3_unit_z()
                                    )
                            )
                    * vector3(m_motion_desire_props.linear_velocity_unit_direction_in_world_space(0),
                              m_motion_desire_props.linear_velocity_unit_direction_in_world_space(1),
                              0.0f)
                    );
    float_32_bit const  elevation_angle = get_blackboard()->m_cortex_cmd_elevation_intensity * 0.5f * PI();
    m_motion_desire_props.linear_velocity_unit_direction_in_world_space *= std::cosf(elevation_angle);
    m_motion_desire_props.linear_velocity_unit_direction_in_world_space(2) = std::sinf(elevation_angle);

    m_motion_desire_props.linear_speed =
        get_blackboard()->m_cortex_cmd_move_intensity * get_blackboard()->m_max_forward_speed_in_meters_per_second;

    // Currently we do not support motions where m_desired_forward_unit_vector_in_world_space and
    // m_desired_linear_velocity_unit_direction_in_world_space could be different. Also there is
    // distinguishing output from the cortex.
    m_motion_desire_props.forward_unit_vector_in_world_space = m_motion_desire_props.linear_velocity_unit_direction_in_world_space;
}


}
