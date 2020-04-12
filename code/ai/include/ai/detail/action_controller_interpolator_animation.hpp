#ifndef AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_ANIMATION_HPP_INCLUDED
#   define AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_ANIMATION_HPP_INCLUDED

#   include <ai/detail/action_controller_interpolator.hpp>
#   include <ai/blackboard_agent.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

namespace ai { namespace detail {


struct  action_controller_interpolator_animation  final : public action_controller_interpolator
{
    explicit action_controller_interpolator_animation(blackboard_agent_weak_ptr const  blackboard_);

    void  next_round(float_32_bit const  time_step_in_seconds);
    void  set_target(skeletal_motion_templates::motion_template_cursor const&  cursor, float_32_bit const  interpolation_time_in_seconds);
    std::vector<angeo::coordinate_system>& get_current_frames_ref() { return m_current_frames; }

    void  commit(vector3 const&  origin_offset = vector3_zero()) const;

private:
    std::vector<angeo::coordinate_system>  m_src_frames;
    std::vector<angeo::coordinate_system>  m_current_frames;
    std::vector<angeo::coordinate_system>  m_dst_frames;
};


}}

#endif
