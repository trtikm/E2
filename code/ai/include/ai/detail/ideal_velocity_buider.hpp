#ifndef AI_DETAIL_IDEAL_VELOCITY_BUILDER_HPP_INCLUDED
#   define AI_DETAIL_IDEAL_VELOCITY_BUILDER_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>

namespace ai { namespace detail {


struct  ideal_velocity_buider
{
    ideal_velocity_buider(
            skeletal_motion_templates::motion_template_cursor const&  start_cursor,
            skeletal_motion_templates const  motion_templates
            );

    void  reset(skeletal_motion_templates::motion_template_cursor const&  start_cursor);

    void  extend(
            skeletal_motion_templates::motion_template_cursor const& cursor,
            float_32_bit const  transition_time
            );

    void  close(
            angeo::coordinate_system const&  motion_object_frame,
            vector3&  ideal_linear_velocity_in_world_space,
            vector3&  ideal_angular_velocity_in_world_space
            );

private:
    void  close();

    std::vector<skeletal_motion_templates::motion_template_cursor>  m_path;
    float_32_bit  m_time;
    skeletal_motion_templates  m_motion_templates;
};


}}

#endif
