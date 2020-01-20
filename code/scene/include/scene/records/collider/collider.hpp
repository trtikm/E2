#ifndef E2_SCENE_RECORDS_COLLIDER_COLLIDER_HPP_INCLUDED
#   define E2_SCENE_RECORDS_COLLIDER_COLLIDER_HPP_INCLUDED

#   include <angeo/collision_object_id.hpp>
#   include <angeo/collision_material.hpp>
#   include <angeo/collision_class.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/filesystem/path.hpp>
#   include <vector>
#   include <string>

namespace scn {


struct  collider final
{
    explicit collider(angeo::collision_object_id const  id, float_32_bit const  density_multiplier = 1.0f)
        : m_ids{id}
        , m_density_multiplier(density_multiplier)
    {}

    explicit collider(std::vector<angeo::collision_object_id> const&  ids, float_32_bit const  density_multiplier = 1.0f)
        : m_ids(ids)
        , m_density_multiplier(density_multiplier)
    {}

    angeo::collision_object_id  id() const { return m_ids.front(); }
    void  set_id(angeo::collision_object_id const  new_id) { m_ids.front() = new_id; }

    std::vector<angeo::collision_object_id> const&  ids() const { return m_ids; }
    std::vector<angeo::collision_object_id>&  ids() { return m_ids; }

    void  set_density_multiplier(float_32_bit const  density_multiplier) { m_density_multiplier = density_multiplier; }
    float_32_bit  get_density_multiplier() const { return m_density_multiplier; }

private:
    std::vector<angeo::collision_object_id>  m_ids;
    float_32_bit  m_density_multiplier;
};


struct  collider_props final
{
    std::string  m_shape_type;

    bool  m_as_dynamic;
    angeo::COLLISION_MATERIAL_TYPE  m_material;
    angeo::COLLISION_CLASS  m_collision_class;
    float_32_bit  m_density_multiplier;

    // DATA OF CAPSULE

    float_32_bit  m_capsule_half_distance_between_end_points;
    float_32_bit  m_capsule_thickness_from_central_line;

    // DATA OF SPHERE 

    float_32_bit  m_sphere_radius;

    // DATA OF TRIANGLE MESH

    boost::filesystem::path  m_triangle_mesh_buffers_directory;
};


}

#endif
