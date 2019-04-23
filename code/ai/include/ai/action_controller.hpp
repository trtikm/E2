#ifndef AI_ACTION_CONTROLLER_HPP_INCLUDED
#   define AI_ACTION_CONTROLLER_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <string>

namespace ai {


struct  action_controller
{
    action_controller(blackboard_ptr const  blackboard_, angeo::coordinate_system const&  start_reference_frame_in_world_space)
        : m_blackboard(blackboard_)
        , m_reference_frame_in_world_space(start_reference_frame_in_world_space)
    {}

    virtual ~action_controller() {}

    virtual void  next_round(float_32_bit  time_step_in_seconds) {}

    blackboard_ptr  get_blackboard() const { return m_blackboard; }

    angeo::coordinate_system const&  get_reference_frame_in_world_space() const { return m_reference_frame_in_world_space; }
    angeo::coordinate_system&  reference_frame_in_world_space_ref() { return m_reference_frame_in_world_space; }

private:
    blackboard_ptr  m_blackboard;
    angeo::coordinate_system  m_reference_frame_in_world_space;
};


}

#endif
