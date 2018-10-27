#ifndef ANGEO_MASS_AND_INERTIA_TENSOR_HPP_INCLUDED
#   define ANGEO_MASS_AND_INERTIA_TENSOR_HPP_INCLUDED

#   include <angeo/collision_material.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <tuple>

namespace angeo {


float_32_bit  get_material_density(COLLISION_MATERIAL_TYPE const  material);

inline float_32_bit  compute_mass(float_32_bit const  density, float_32_bit const  volume) { return density * volume; }


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

    struct  sphere_info {
        vector3  center;
        float_32_bit radius;
        COLLISION_MATERIAL_TYPE  material;
    };
    std::vector<sphere_info>  m_spheres;
};


}

#endif
