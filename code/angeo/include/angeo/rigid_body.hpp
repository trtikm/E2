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


using  rigid_body_velocity = linear_and_angular_vector;


// An id of a rigid body is ALWAYS interpreted as an INDEX to a vector/array of 'rigid_body' structures (defined below).
using  rigid_body_id = natural_32_bit;


struct  rigid_body
{
    // All fields below are in the world space!

    coordinate_system_ptr  m_coord_system; // The centre of mass is assumed to be in the origin of that coordinate system!!
    rigid_body_velocity  m_velocity;
};


using  rigid_body_matter_props_id = natural_32_bit;


struct  rigid_body_matter_props
{
    float_32_bit  m_inverted_mass;  // The value 0.0f means the mass is infinite. 
    matrix33  m_inverted_inertia_tensor; // Always in the local space. Zero matrix means an infinite inertia.
};


}

#endif
