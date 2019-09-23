#ifndef AI_DETAIL_COLLIDER_UTILS_HPP_INCLUDED
#   define AI_DETAIL_COLLIDER_UTILS_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <angeo/tensor_math.hpp>

namespace ai { namespace detail {


float_32_bit  distance_from_center_of_collider_to_surface_in_direction(
        skeletal_motion_templates::collider_ptr const  collider_ptr,
        vector3 const&  unit_direction
        );


vector3  compute_offset_for_center_of_second_collider_to_get_surfaces_alignment_in_direction(
        skeletal_motion_templates::collider_ptr const  first_collider_ptr,
        skeletal_motion_templates::collider_ptr const  second_collider_ptr,
        vector3 const& unit_direction
        );


}}

#endif
