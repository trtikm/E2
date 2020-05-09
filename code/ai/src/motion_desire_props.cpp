#include <ai/motion_desire_props.hpp>

namespace ai {


motion_desire_props::motion_desire_props()
    : speed(vector4_zero())
    , look_at_target(5.0f * vector3_unit(DESIRE_COORD::FORWARD))
{}


}
