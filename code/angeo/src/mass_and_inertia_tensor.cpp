#include <angeo/mass_and_inertia_tensor.hpp>

namespace angeo {


void  mass_and_inertia_tensor_builder::insert_capsule(
        vector3 const&  end_point_1_in_world_space,
        vector3 const&  end_point_2_in_world_space,
        COLLISION_MATERIAL_TYPE const  material
        )
{
    // TODO!
}


void  mass_and_inertia_tensor_builder::insert_sphere(
        vector3 const&  center_in_world_space,
        float_32_bit const  radius,
        COLLISION_MATERIAL_TYPE const  material
        )
{
    // TODO!
}


void  mass_and_inertia_tensor_builder::run(float_32_bit&  inverted_mass, matrix33&  inverted_inertia_tensor)
{
    // TODO!
//    inverted_mass = 0.0f;
//    inverted_inertia_tensor = matrix33_zero();
}


}
