#ifndef AIOLD_MOTION_DESIRE_PROPS_HPP_INCLUDED
#   define AIOLD_MOTION_DESIRE_PROPS_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>

namespace aiold {


struct  motion_desire_props
{
    // All fields, including those in nested structures, are in cortex's LOGICAL space
    // which lies inside agent's space. They all are also in the range <-1.0, 1.0>.
    // Action controller defines the actual transformations between the logical and agent's
    // space.

    struct  move_props
    {
        move_props() : forward(0.0f), left(0.0f), up(0.0f), turn_ccw(0.0f) {}

        float_32_bit  forward;
        float_32_bit  left;
        float_32_bit  up;
        float_32_bit  turn_ccw;
    };

    struct  guesture_props
    {
        struct  subject_props
        {
            subject_props() : head(0.0f), tail(0.0f) {}

            float_32_bit  head;
            float_32_bit  tail;
        };

        struct  sign_props
        {
            sign_props() : head(0.0f), tail(0.0f), intensity(0.0f) {}

            float_32_bit  head;
            float_32_bit  tail;
            float_32_bit  intensity;
        };

        subject_props  subject;
        sign_props  sign;
    };

    struct  target_props
    {
        target_props() : longitude(0.0f), altitude(0.0f), magnitude(0.0f) {}

        float_32_bit  longitude;
        float_32_bit  altitude;
        float_32_bit  magnitude;
    };

    move_props  move;
    guesture_props  guesture;
    target_props  look_at;
    target_props  aim_at;
};


}

#endif