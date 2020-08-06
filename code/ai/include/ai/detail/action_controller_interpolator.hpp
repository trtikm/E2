#ifndef AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_HPP_INCLUDED
#   define AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/scene_binding.hpp>
#   include <com/object_guid.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>

namespace ai { namespace detail {


struct  action_controller_interpolator
{
    action_controller_interpolator(
            skeletal_motion_templates const  motion_templates,
            scene_binding_ptr const  binding
            );
    virtual ~action_controller_interpolator() {}

    bool  done() const { return get_remaining_time() <= 0.0f; }

    void  add_time(float_32_bit const  time_step_in_seconds);
    void  reset_time(float_32_bit const  total_interpolation_time_in_seconds);

    float_32_bit  get_interpolation_parameter() const; // The iterpolation parameter is always in <0,1>.

    float_32_bit  get_remaining_time() const
    {
        return std::max(0.0f, m_total_interpolation_time_in_seconds - m_consumed_time_in_seconds);
    }

    skeletal_motion_templates  get_motion_templates() const { return m_motion_templates; }
    scene_binding_ptr  get_binding() const { return m_binding; }
    com::simulation_context const&  ctx() const { return *m_binding->context; }
    com::object_guid  get_frame_guid_of_skeleton() const { return m_binding->frame_guid_of_skeleton; }
    std::vector<com::object_guid> const&  get_frame_guids_of_bones() const { return m_binding->frame_guids_of_bones; }

private:
    skeletal_motion_templates  m_motion_templates;
    scene_binding_ptr  m_binding;
    float_32_bit  m_total_interpolation_time_in_seconds;
    float_32_bit  m_consumed_time_in_seconds;
};


struct  action_controller_interpolator_shared
{
    explicit action_controller_interpolator_shared(action_controller_interpolator const* const  interpolator_)
        : m_interpolator(interpolator_)
    {}
    virtual ~action_controller_interpolator_shared() {}

    bool  done() const { return m_interpolator->done(); }

    float_32_bit  get_interpolation_parameter() const { return m_interpolator->get_interpolation_parameter();  }

    float_32_bit  get_remaining_time() const { return m_interpolator->get_remaining_time(); }

    skeletal_motion_templates  get_motion_templates() const { return m_interpolator->get_motion_templates(); }
    scene_binding_ptr  get_binding() const { return m_interpolator->get_binding(); }
    com::simulation_context const&  ctx() const { return m_interpolator->ctx(); }
    com::object_guid  get_frame_guid_of_skeleton() const { return m_interpolator->get_frame_guid_of_skeleton(); }
    std::vector<com::object_guid> const&  get_frame_guids_of_bones() const { return m_interpolator->get_frame_guids_of_bones(); }

private:

    action_controller_interpolator const*  m_interpolator;
};


}}

#endif
