#include <angeo/mass_and_inertia_tensor.hpp>
#include <angeo/axis_aligned_bounding_box.hpp>
#include <angeo/collide.hpp>
#include <angeo/collision_shape_id.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>

namespace angeo { namespace detail {


static void  update_inertia_tensor_for_particle(vector3 const&  position, float_32_bit const  mass, matrix33&  inertia_tensor)
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


static  void  foreach_particle_in_bbox(
        axis_aligned_bounding_box const&  bbox,
        float_32_bit const  total_mass,
        natural_32_bit const  num_of_particles,
        std::function<bool(vector3 const&)>  is_inside_collider,
        std::function<void(vector3 const&, float_32_bit)>  acceptor
        )
{
    vector3 const  delta = bbox.max_corner - bbox.min_corner;

    natural_32_bit  num_cells_x, num_cells_y, num_cells_z;
    {
        float_32_bit const  max_delta = std::max(std::max(delta(0), delta(1)), delta(2));
        ASSUMPTION(max_delta > 0.0001f);
        vector3 const  cells_count_ratios = delta / max_delta;
        float_32_bit const  num_cells_along_longest_edge =
                std::pow((float)num_of_particles / (cells_count_ratios(0) * cells_count_ratios(1) * cells_count_ratios(2)),
                         1.0f / 3.0f);
        num_cells_x = std::max(1U, (natural_32_bit)std::ceil(num_cells_along_longest_edge * cells_count_ratios(0)));
        num_cells_y = std::max(1U, (natural_32_bit)std::ceil(num_cells_along_longest_edge * cells_count_ratios(1)));
        num_cells_z = std::max(1U, (natural_32_bit)std::ceil(num_cells_along_longest_edge * cells_count_ratios(2)));
    }

    vector3 const  shift{ 
            delta(0) / (float_32_bit)num_cells_x,
            delta(1) / (float_32_bit)num_cells_y,
            delta(2) / (float_32_bit)num_cells_z,
            };

    float_32_bit const  mass_of_particle = total_mass / float_32_bit(num_cells_x * num_cells_y * num_cells_z);

    for (natural_32_bit  i = 0U; i <= num_cells_x; ++i)
    {
        vector3 const  shift_x{ (float_32_bit)i * shift(0), 0.0f, 0.0f };
        for (natural_32_bit j = 0U; j <= num_cells_y; ++j)
        {
            vector3 const  shift_y{ 0.0f, (float_32_bit)j * shift(1), 0.0f };
            for (natural_32_bit k = 0U; k <= num_cells_z; ++k)
            {
                vector3 const  shift_z{ 0.0f, 0.0f, (float_32_bit)k * shift(2) };
                vector3 const  position = bbox.min_corner + shift_x + shift_y + shift_z;
                if (is_inside_collider(position))
                    acceptor(position, mass_of_particle);
            }
        }
    }
}


}}

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

    natural_32_bit const  num_particles_per_collider = 1000U;
    for (capsule_info const&  info : m_capsules)
        detail::foreach_particle_in_bbox(
                compute_aabb_of_capsule(info.half_distance_between_end_points, info.thickness_from_central_line),
                info.mass,
                1000U,
                [&info](vector3 const&  position) -> bool {
                        return is_point_inside_capsule(
                                    position,
                                    info.half_distance_between_end_points,
                                    info.thickness_from_central_line
                                    );
                    },
                [&inertia_tensor, &info, &center_of_mass](vector3 const&  position, float_32_bit const  mass) -> void {
                        detail::update_inertia_tensor_for_particle(
                                transform_point(position, info.from_base_matrix) - center_of_mass,
                                mass,
                                inertia_tensor
                                );
                    }
                );

    for (sphere_info const&  info : m_spheres)
        detail::foreach_particle_in_bbox(
                compute_aabb_of_sphere(info.radius),
                info.mass,
                1000U,
                [&info](vector3 const&  position) -> bool {
                        return is_point_inside_sphere(position, info.radius);
                    },
                [&inertia_tensor, &info, &center_of_mass](vector3 const&  position, float_32_bit const  mass) -> void {
                        detail::update_inertia_tensor_for_particle(
                                info.center + position - center_of_mass,
                                mass,
                                inertia_tensor
                                );
                    }
                );

    inverted_inertia_tensor = inverse33(inertia_tensor);
}


}
