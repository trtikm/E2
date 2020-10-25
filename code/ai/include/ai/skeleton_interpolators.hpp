#ifndef AI_SKELETON_INTERPOLATORS_HPP_INCLUDED
#   define AI_SKELETON_INTERPOLATORS_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/scene_binding.hpp>
#   include <ai/sight_controller.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

namespace ai {


struct  skeleton_interpolator_animation
{
    skeleton_interpolator_animation();
    void  interpolate(float_32_bit const  interpolation_param);
    void  set_target(
            skeletal_motion_templates::motion_template_cursor const&  cursor,
            skeletal_motion_templates const  motion_templates
            );
    void  move_to_target();
    std::vector<angeo::coordinate_system>& get_current_frames_ref() { return m_current_frames; }
    void  commit(skeletal_motion_templates const  motion_templates, scene_binding const&  binding) const;

private:
    std::vector<angeo::coordinate_system>  m_src_frames;
    std::vector<angeo::coordinate_system>  m_current_frames;
    std::vector<angeo::coordinate_system>  m_dst_frames;
    vector3  m_src_offset;
    vector3  m_current_offset;
    vector3  m_dst_offset;
};


struct  skeleton_interpolator_look_at
{
    skeleton_interpolator_look_at();
    void  interpolate(
            float_32_bit const  time_step_in_seconds,
            vector3 const&  look_at_target_in_agent_space,
            std::vector<angeo::coordinate_system>&  frames_to_update,
            skeletal_motion_templates const  motion_templates,
            scene_binding const&  binding
            );
private:
    void  update_look_at_target_in_local_space(
            vector3 const&  look_at_target_from_cortex,
            angeo::coordinate_system_explicit const&  agent_frame,
            ai::sight_controller::camera_perspective_ptr const  camera
            );
    angeo::joint_rotation_states_of_bones  m_joint_rotations;
    vector3  m_look_at_target_in_local_space;
    bool  m_target_pose_reached;
};


struct  skeleton_interpolator_aim_at
{
    skeleton_interpolator_aim_at();
    void  interpolate(
            float_32_bit const  time_step_in_seconds,
            vector3 const&  aim_at_target_in_agent_space,
            std::vector<angeo::coordinate_system>&  frames_to_update,
            skeletal_motion_templates const  motion_templates,
            scene_binding const&  binding
            );
};


}

#endif
