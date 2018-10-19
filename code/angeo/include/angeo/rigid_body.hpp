#ifndef ANGEO_RIGID_BODY_HPP_INCLUDED
#   define ANGEO_RIGID_BODY_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>

namespace angeo {


struct  linear_and_angular_vector
{
    vector3  m_linear;
    vector3  m_angular;
};


// An id of a rigid body is ALWAYS interpreted as an INDEX to a vector/array of 'rigid_body' structures (defined below).
using  rigid_body_id = natural_32_bit;


struct  rigid_body
{
    // All fields below are in the world space!

    vector3  m_position_of_mass_centre;
    quaternion  m_orientation;

    linear_and_angular_vector  m_velocity; // Must be updated after each time step.
    linear_and_angular_vector  m_acceleration_from_constraints; // Must be cleared in the end of each time step.
    linear_and_angular_vector  m_acceleration_from_external_forces; // The linear and algular components depend on fields 'm_inverted_mass' and 'm_inverted_inertia_tensor', respectively.

    float_32_bit  m_inverted_mass;  // The value 0.0f means the mass is infinite. 
    matrix33  m_inverted_inertia_tensor; // Zero matrix means an infinite inertia. Must be updated after each time step.
};


}

#endif
