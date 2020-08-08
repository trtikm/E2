#ifndef AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_HPP_INCLUDED
#   define AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/scene_binding.hpp>
#   include <com/object_guid.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>

namespace ai { namespace detail {


struct  action_controller_interpolator_base
{
    action_controller_interpolator_base(
            skeletal_motion_templates const  motion_templates,
            scene_binding_ptr const  binding
            );
    virtual ~action_controller_interpolator_base() {}

    skeletal_motion_templates  get_motion_templates() const { return m_motion_templates; }
    scene_binding_ptr  get_binding() const { return m_binding; }
    com::simulation_context const&  ctx() const { return *m_binding->context; }
    com::object_guid  get_frame_guid_of_skeleton() const { return m_binding->frame_guid_of_skeleton; }
    std::vector<com::object_guid> const&  get_frame_guids_of_bones() const { return m_binding->frame_guids_of_bones; }

private:
    skeletal_motion_templates  m_motion_templates;
    scene_binding_ptr  m_binding;
};


struct  action_controller_interpolator_time : public action_controller_interpolator_base
{
    action_controller_interpolator_time(
            skeletal_motion_templates const  motion_templates,
            scene_binding_ptr const  binding
            );

    bool  done() const { return get_remaining_time() <= 0.0f; }

    void  add_time(float_32_bit const  time_step_in_seconds);
    void  reset_time(float_32_bit const  total_interpolation_time_in_seconds);

    float_32_bit  get_interpolation_parameter() const; // The iterpolation parameter is always in <0,1>.

    float_32_bit  get_remaining_time() const
    {
        return std::max(0.0f, m_total_interpolation_time_in_seconds - m_consumed_time_in_seconds);
    }

private:
    float_32_bit  m_total_interpolation_time_in_seconds;
    float_32_bit  m_consumed_time_in_seconds;
};


}}

#endif
