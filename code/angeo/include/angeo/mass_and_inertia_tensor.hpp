#ifndef ANGEO_MASS_AND_INERTIA_TENSOR_HPP_INCLUDED
#   define ANGEO_MASS_AND_INERTIA_TENSOR_HPP_INCLUDED

#   include <angeo/collision_material.hpp>
#   include <angeo/tensor_math.hpp>

namespace angeo {


struct  mass_and_inertia_tensor_builder
{
    void  insert_capsule(
            vector3 const&  end_point_1_in_world_space,
            vector3 const&  end_point_2_in_world_space,
            COLLISION_MATERIAL_TYPE const  material
            );

    void  insert_sphere(
            vector3 const&  center_in_world_space,
            float_32_bit const  radius,
            COLLISION_MATERIAL_TYPE const  material
            );

    void  run(float_32_bit&  inverted_mass, matrix33&  inverted_inertia_tensor);

private:
};


}

#endif
