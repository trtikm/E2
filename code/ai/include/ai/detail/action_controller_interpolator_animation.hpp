#ifndef AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_ANIMATION_HPP_INCLUDED
#   define AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_ANIMATION_HPP_INCLUDED

#   include <ai/detail/action_controller_interpolator.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

namespace ai { namespace detail {


struct  action_controller_interpolator_animation  final : public action_controller_interpolator_base
{
    action_controller_interpolator_animation(
            skeletal_motion_templates const  motion_templates,
            scene_binding_ptr const  binding,
            skeletal_motion_templates::motion_template_cursor const&  initial_template_cursor,
            float_32_bit const  reference_offset
            );

    void  interpolate(float_32_bit const  interpolation_param);
    void  set_target(skeletal_motion_templates::motion_template_cursor const&  cursor);

    std::vector<angeo::coordinate_system>& get_current_frames_ref() { return m_current_frames; }

    void  commit() const;

private:
    std::vector<angeo::coordinate_system>  m_src_frames;
    std::vector<angeo::coordinate_system>  m_current_frames;
    std::vector<angeo::coordinate_system>  m_dst_frames;

    float_32_bit  m_reference_offset;

    float_32_bit  m_src_offset;
    float_32_bit  m_current_offset;
    float_32_bit  m_dst_offset;
};


}}

#endif
