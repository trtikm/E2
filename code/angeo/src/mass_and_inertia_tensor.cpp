#include <angeo/mass_and_inertia_tensor.hpp>
#include <angeo/collision_shape_id.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>

namespace angeo {


float_32_bit  get_material_density(COLLISION_MATERIAL_TYPE const  material)
{
    switch (material)
    {
    case COLLISION_MATERIAL_TYPE::ASPHALT: return 1041.0f;
    case COLLISION_MATERIAL_TYPE::CONCRETE: return 2400.0f;
    case COLLISION_MATERIAL_TYPE::DIRT: return 1041.0f;
    case COLLISION_MATERIAL_TYPE::GLASS: return 1922.0f;
    case COLLISION_MATERIAL_TYPE::GRASS: return 1.0f;
    case COLLISION_MATERIAL_TYPE::GUM:return 769.0f;
    case COLLISION_MATERIAL_TYPE::ICE: return 916.7f;
    case COLLISION_MATERIAL_TYPE::LEATHER: 600.0f; // TODO: check that value.
    case COLLISION_MATERIAL_TYPE::MUD: return 1.0f;
    case COLLISION_MATERIAL_TYPE::PLASTIC: return 1175.0f;
    case COLLISION_MATERIAL_TYPE::RUBBER: return 400.0f;
    case COLLISION_MATERIAL_TYPE::STEEL: return 2403.0f;
    case COLLISION_MATERIAL_TYPE::WOOD: return 700.0f;
    default:
        UNREACHABLE();
    }
}


void  mass_and_inertia_tensor_builder::insert_capsule(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        matrix44 const&  from_base_matrix,
        COLLISION_MATERIAL_TYPE const  material,
        float_32_bit const  density_multiplier
        )
{
    ASSUMPTION(density_multiplier >= 0.001f);
    m_capsules.push_back({
            half_distance_between_end_points,
            thickness_from_central_line,
            from_base_matrix,
            compute_mass(density_multiplier * get_material_density(material),
                         compute_volume_of_capsule(half_distance_between_end_points, thickness_from_central_line))
            });
}


void  mass_and_inertia_tensor_builder::insert_sphere(
        vector3 const&  center_in_world_space,
        float_32_bit const  radius,
        COLLISION_MATERIAL_TYPE const  material,
        float_32_bit const  density_multiplier
        )
{
    ASSUMPTION(radius >= 0.001f);
    ASSUMPTION(density_multiplier >= 0.001f);
    m_spheres.push_back({
            center_in_world_space,
            radius,
            compute_mass(density_multiplier * get_material_density(material), compute_volume_of_sphere(radius))
            });
}


float_32_bit  mass_and_inertia_tensor_builder::compute_total_mass_and_center_of_mass(vector3&  center_of_mass)
{
    center_of_mass = vector3_zero();
    float_32_bit  total_mass = 0.0f;

    for (capsule_info const&  info : m_capsules)
    {
        center_of_mass += info.mass * translation_vector(info.from_base_matrix);
        total_mass += info.mass;
    }

    for (sphere_info const&  info : m_spheres)
    {
        center_of_mass += info.mass * info.center;
        total_mass += info.mass;
    }

    return  total_mass;
}


static  void  update_inertia_tensor_for_particle(vector3 const&  position, float_32_bit const  mass, matrix33&  inertia_tensor)
{
    inertia_tensor(0 ,0) += mass * (position(1) * position(1) + position(2) *position(2));
    inertia_tensor(1, 1) += mass * (position(0) * position(0) + position(2) *position(2));
    inertia_tensor(2, 2) += mass * (position(0) * position(0) + position(1) *position(1));

    inertia_tensor(1, 0) -= mass * position(0) * position(1);
    inertia_tensor(0, 1) -= mass * position(0) * position(1);

    inertia_tensor(2, 0) -= mass * position(0) * position(2);
    inertia_tensor(0, 2) -= mass * position(0) * position(2);

    inertia_tensor(2, 1) -= mass * position(1) * position(2);
    inertia_tensor(1, 2) -= mass * position(1) * position(2);
}


static  void  foreach_particle_of_capsule(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        std::function<void(vector3 const&)>  acceptor
        )
{
    NOT_IMPLEMENTED_YET();
}


static  void  foreach_particle_of_sphere(float_32_bit const  radius, std::function<void(vector3 const&)>  acceptor)
{
    NOT_IMPLEMENTED_YET();
}


void  mass_and_inertia_tensor_builder::run(
        float_32_bit&  inverted_mass,
        matrix33&  inverted_inertia_tensor,
        vector3&  center_of_mass
        )
{
    float_32_bit  total_mass = compute_total_mass_and_center_of_mass(center_of_mass);
    INVARIANT(total_mass > 0.0001f);

    inverted_mass = 1.0f / total_mass;

    matrix33  inertia_tensor = matrix33_zero();

    for (capsule_info const&  info : m_capsules)
        foreach_particle_of_capsule(
                info.half_distance_between_end_points,
                info.thickness_from_central_line,
                [&inertia_tensor, &info](vector3 const&  position) {
                        update_inertia_tensor_for_particle(position, info.mass, inertia_tensor);
                    }
                );

    for (sphere_info const&  info : m_spheres)
        foreach_particle_of_sphere(
                info.radius,
                [&inertia_tensor, &info](vector3 const&  position) {
                        update_inertia_tensor_for_particle(position, info.mass, inertia_tensor);
                    }
                );

    inverted_inertia_tensor = inverse33(inertia_tensor);
}


}
