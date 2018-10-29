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
    case COLLISION_MATERIAL_TYPE::ASPHALT:
    case COLLISION_MATERIAL_TYPE::CONCRETE:
    case COLLISION_MATERIAL_TYPE::DIRT:
    case COLLISION_MATERIAL_TYPE::GLASS:
    case COLLISION_MATERIAL_TYPE::GRASS:
    case COLLISION_MATERIAL_TYPE::GUM:
    case COLLISION_MATERIAL_TYPE::ICE:
    case COLLISION_MATERIAL_TYPE::LEATHER:
    case COLLISION_MATERIAL_TYPE::MUD:
    case COLLISION_MATERIAL_TYPE::PLASTIC:
    case COLLISION_MATERIAL_TYPE::RUBBER:
    case COLLISION_MATERIAL_TYPE::STEEL:
    case COLLISION_MATERIAL_TYPE::WOOD:
        NOT_IMPLEMENTED_YET();
    default:
        UNREACHABLE();
    }
}


void  mass_and_inertia_tensor_builder::insert_capsule(
        vector3 const&  end_point_1_in_world_space,
        vector3 const&  end_point_2_in_world_space,
        COLLISION_MATERIAL_TYPE const  material
        )
{
    NOT_IMPLEMENTED_YET();
}


void  mass_and_inertia_tensor_builder::insert_sphere(
        vector3 const&  center_in_world_space,
        float_32_bit const  radius,
        COLLISION_MATERIAL_TYPE const  material
        )
{
    ASSUMPTION(radius >= 0.001f);
    m_spheres.push_back({ center_in_world_space, radius, material });
}


void  mass_and_inertia_tensor_builder::run(float_32_bit&  inverted_mass, matrix33&  inverted_inertia_tensor)
{
    if (m_spheres.size() == 1UL && length_squared(m_spheres.front().center) < 0.0001f)
    {
        float_32_bit const  rad2 = m_spheres.front().radius * m_spheres.front().radius;
        float_32_bit const  volume = (3.0f/4.0f) * PI() * rad2 * m_spheres.front().radius;
        float_32_bit const  mass = compute_mass(get_material_density(m_spheres.front().material), volume);

        inverted_mass = 1.0f / mass;

        float_32_bit const  c = inverted_mass * 5.0f / (2.0f * rad2);
        inverted_inertia_tensor(0, 0) = c;      inverted_inertia_tensor(0, 1) = 0;      inverted_inertia_tensor(0, 2) = 0;
        inverted_inertia_tensor(1, 0) = 0;      inverted_inertia_tensor(1, 1) = c;      inverted_inertia_tensor(1, 2) = 0;
        inverted_inertia_tensor(2, 0) = 0;      inverted_inertia_tensor(2, 1) = 0;      inverted_inertia_tensor(2, 2) = c;
    }
    else
    {
        NOT_IMPLEMENTED_YET();
    }
}


}
