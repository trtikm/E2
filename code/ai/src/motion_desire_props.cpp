#include <ai/motion_desire_props.hpp>

namespace ai {


motion_desire_props::motion_desire_props()
    : speed(vector4_zero())
    , guesture_subject(vector2_zero())
    , guesture_sign(vector2_zero())
    , guesture_intensity(0.0f)
    , look_at_target(5.0f * vector3_unit(DESIRE_COORD::FORWARD))
    , aim_at_target(0.3f * vector3_unit(DESIRE_COORD::FORWARD) + 0.3f * vector3_unit(DESIRE_COORD::UP))
{}


}
