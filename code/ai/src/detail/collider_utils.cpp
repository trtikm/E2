#include <ai/detail/collider_utils.hpp>
#include <angeo/collide.hpp>
#include <utility/development.hpp>
#include <memory>

namespace ai { namespace detail {


float_32_bit  distance_from_center_of_collider_to_surface_in_direction(
        skeletal_motion_templates::collider_ptr const  collider_ptr,
        vector3 const&  unit_direction
        )
{
    if (auto const  ptr = std::dynamic_pointer_cast<skeletal_motion_templates::collider_capsule const>(collider_ptr))
        return angeo::distance_from_center_of_capsule_to_surface_in_direction(ptr->length, ptr->radius, unit_direction);
    else
    {
        NOT_IMPLEMENTED_YET();
    }
}


vector3  compute_offset_for_center_of_second_collider_to_get_surfaces_alignment_in_direction(
        skeletal_motion_templates::collider_ptr const  first_collider_ptr,
        skeletal_motion_templates::collider_ptr const  second_collider_ptr,
        vector3 const& unit_direction
        )
{
    float_32_bit const  distance_delta =
            distance_from_center_of_collider_to_surface_in_direction(first_collider_ptr, unit_direction) -
            distance_from_center_of_collider_to_surface_in_direction(second_collider_ptr, unit_direction)
            ;
    return  distance_delta * unit_direction;
}


}}
