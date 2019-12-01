#ifndef AI_ACTION_REGULATOR_HPP_INCLUDED
#   define AI_ACTION_REGULATOR_HPP_INCLUDED

#   include <ai/motion_desire_props.hpp>
#   include <angeo/tensor_math.hpp>

namespace ai {


struct  action_controller;


struct  action_regulator
{
    explicit  action_regulator(action_controller const* const  controller_ptr);

    void  initialise();
    void  next_round(motion_desire_props const&  motion_desire_props_of_the_cortex);

    motion_desire_props const&  get_motion_desire_props() const { return m_motion_desire_props; }

private:
    action_controller const*  m_controller;
    motion_desire_props  m_motion_desire_props;
};


}

#endif
