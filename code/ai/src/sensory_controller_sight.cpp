#include <ai/sensory_controller_sight.hpp>
#include <ai/action_controller.hpp>
#include <angeo/coordinate_system.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


sensory_controller_sight::camera_config::camera_config(
        float_32_bit const  fov_angle_,
        float_32_bit const  near_plane_,
        float_32_bit const  far_plane_,
        float_32_bit const  origin_z_shift_
        )
    : fov_angle(fov_angle_)
    , near_plane(near_plane_)
    , far_plane(far_plane_)
    , origin_z_shift(origin_z_shift_)
{}


sensory_controller_sight::sensory_controller_sight(
        blackboard_agent_weak_ptr const  blackboard_,
        camera_config const&  camera_config_)
    : m_blackboard(blackboard_)
    , m_camera_config(camera_config_)
    , m_camera()
{}


void  sensory_controller_sight::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    angeo::coordinate_system  camera_coord_system;
    {
        TMPROF_BLOCK();

        std::vector<std::pair<vector3, vector3> >  look_at_props;
        std::vector<vector3>  eye_right_directions;
        for (skeletal_motion_templates::look_at_info_ptr  look_at_ptr :
             get_blackboard()->m_action_controller->get_free_bones_for_look_at())
        {
            angeo::coordinate_system  frame;
            get_blackboard()->m_scene->get_frame_of_scene_node(
                    get_blackboard()->m_bone_nids.at(look_at_ptr->end_effector_bone),
                    false, frame
                    );
            look_at_props.push_back({ frame.origin(), angeo::axis_y(frame) });
            eye_right_directions.push_back(angeo::axis_x(frame));
        }

        if (look_at_props.empty())
        {
            m_camera = nullptr;
            return;
        }

        vector3  camera_origin;
        {
            camera_origin = vector3_zero();
            for (auto const&  pos_and_dir : look_at_props)
                camera_origin += pos_and_dir.first;
            camera_origin = (1.0f / (float_32_bit)look_at_props.size()) * camera_origin;
            //// Let us predict/estimate the position of the origin at the next round.
            //camera_origin += (1.0f * time_step_in_seconds) * get_blackboard()->m_action_controller->get_motion_object_motion().velocity.m_linear;
        }
        vector3 const  look_at_target =
                look_at_props.size() == 1UL ?
                        camera_origin + look_at_props.front().second :
                        angeo::compute_common_look_at_target_for_multiple_eyes(look_at_props);
        vector3  camera_z_axis;
        {
            camera_z_axis = camera_origin - look_at_target;

            float_32_bit const  d = length(camera_z_axis);
            if (std::fabs(d) < 0.001f)
            {
                m_camera = nullptr;
                return;
            }

            camera_z_axis = (1.0f / d) * camera_z_axis;
        }
        vector3  camera_x_axis, camera_y_axis;
        {
            camera_x_axis = vector3_zero();
            for (auto const& dir : eye_right_directions)
                camera_x_axis += dir;
            camera_x_axis = (1.0f / (float_32_bit)eye_right_directions.size()) * camera_x_axis;
            camera_y_axis = cross_product(camera_z_axis, camera_x_axis);

            float_32_bit const  d = length(camera_y_axis);
            if (std::fabs(d) < 0.001f)
            {
                m_camera = nullptr;
                return;
            }

            camera_y_axis = (1.0f / d) * camera_y_axis;
            camera_x_axis = cross_product(camera_y_axis, camera_z_axis);
        }
        matrix33 R;
        basis_to_rotation_matrix(camera_x_axis, camera_y_axis, camera_z_axis, R);

        camera_coord_system = { camera_origin + m_camera_config.origin_z_shift * camera_z_axis, rotation_matrix_to_quaternion(R) };
    }

    if (m_camera == nullptr)
    {
        float_32_bit const  tan_half_FOV_angle = std::tanf(m_camera_config.fov_angle * 0.5f);
        float_32_bit const  camera_window_half_size = m_camera_config.near_plane * tan_half_FOV_angle;

        m_camera = qtgl::camera_perspective::create(
                        angeo::coordinate_system::create(vector3_zero(), quaternion_identity()),
                        m_camera_config.near_plane,
                        m_camera_config.far_plane,
                        -camera_window_half_size,
                        camera_window_half_size,
                        -camera_window_half_size,
                        camera_window_half_size
                        );
    }

    m_camera->set_coordinate_system(camera_coord_system);
}


}
