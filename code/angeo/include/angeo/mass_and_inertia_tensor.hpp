#ifndef ANGEO_MASS_AND_INERTIA_TENSOR_HPP_INCLUDED
#   define ANGEO_MASS_AND_INERTIA_TENSOR_HPP_INCLUDED

#   include <angeo/collision_material.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <tuple>

namespace angeo {


float_32_bit  get_material_density(COLLISION_MATERIAL_TYPE const  material);
inline float_32_bit  compute_mass(float_32_bit const  density, float_32_bit const  volume) { return density * volume; }

inline float_32_bit  compute_volume_of_box(vector3 const&  half_sizes_along_axes)
{
    return 8.0f * half_sizes_along_axes(0) * half_sizes_along_axes(1) * half_sizes_along_axes(2);
}

inline float_32_bit  compute_volume_of_sphere(float_32_bit const  radius) { return 4.0f * (PI() * radius * radius * radius) / 3.0f; }

inline float_32_bit  compute_volume_of_capsule(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line
        )
{
    return compute_volume_of_sphere(thickness_from_central_line) +
           PI() * thickness_from_central_line * thickness_from_central_line * (2.0f * half_distance_between_end_points);
}


struct  mass_and_inertia_tensor_builder
{
    void  insert_box(
            vector3 const&  half_sizes_along_axes,
            matrix44 const&  from_base_matrix,
            COLLISION_MATERIAL_TYPE const  material,
            float_32_bit const  density_multiplier = 1.0f
            );

    void  insert_capsule(
            float_32_bit const  half_distance_between_end_points,
            float_32_bit const  thickness_from_central_line,
            matrix44 const&  from_base_matrix,
            COLLISION_MATERIAL_TYPE const  material,
            float_32_bit const  density_multiplier = 1.0f
            );

    void  insert_sphere(
            vector3 const&  center_in_world_space,
            float_32_bit const  radius,
            COLLISION_MATERIAL_TYPE const  material,
            float_32_bit const  density_multiplier = 1.0f
            );

    /// All three parameters are outputs. The inertia tensor is computed in coodrinate system
    /// with center at the returned 'center_of_mass' and axis vectors 'vector3_unit_x/y/z'.
    void  run(float_32_bit&  inverted_mass, matrix33&  inverted_inertia_tensor, vector3&  center_of_mass);

private:

    void  compute_inverted_total_mass_and_center_of_mass(float_32_bit&  inverted_mass, vector3&  center_of_mass);

    struct  box_info {
        vector3  half_sizes_along_axes;
        matrix44  from_base_matrix;
        float_32_bit  mass;
    };
    std::vector<box_info>  m_boxes;

    struct  capsule_info {
        float_32_bit  half_distance_between_end_points;
        float_32_bit  thickness_from_central_line;
        matrix44  from_base_matrix;
        float_32_bit  mass;
    };
    std::vector<capsule_info>  m_capsules;

    struct  sphere_info {
        vector3  center;
        float_32_bit  radius;
        float_32_bit  mass;
    };
    std::vector<sphere_info>  m_spheres;
};


}

#endif
