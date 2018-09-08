#include <angeo/collision_scene.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace angeo {


axis_aligned_bounding_box  compute_aabb_of_capsule(
        vector3 const&  point_1,
        vector3 const&  point_2,
        float_32_bit const  radius
        )
{
    return  {
                vector3(std::min(point_1(0), point_2(0)) - radius,
                        std::min(point_1(1), point_2(1)) - radius,
                        std::min(point_1(2), point_2(2)) - radius
                        ),
                vector3(std::max(point_1(0), point_2(0)) + radius,
                        std::max(point_1(1), point_2(1)) + radius,
                        std::max(point_1(2), point_2(2)) + radius
                        )
            };
}


axis_aligned_bounding_box  compute_aabb_of_line(
        vector3 const&  point_1,
        vector3 const&  point_2
        )
{
    return  {
                vector3(std::min(point_1(0), point_2(0)),
                        std::min(point_1(1), point_2(1)),
                        std::min(point_1(2), point_2(2))
                        ),
                vector3(std::max(point_1(0), point_2(0)),
                        std::max(point_1(1), point_2(1)),
                        std::max(point_1(2), point_2(2))
                        )
            };
}


axis_aligned_bounding_box  compute_aabb_of_triangle(
        vector3 const&  point_1,
        vector3 const&  point_2,
        vector3 const&  point_3
        )
{
    return  {
                vector3(std::min(std::min(point_1(0), point_2(0)), point_3(0)),
                        std::min(std::min(point_1(1), point_2(1)), point_3(1)),
                        std::min(std::min(point_1(2), point_2(2)), point_3(2))
                        ),
                vector3(std::max(std::max(point_1(0), point_2(0)), point_3(0)),
                        std::max(std::max(point_1(1), point_2(1)), point_3(1)),
                        std::max(std::max(point_1(2), point_2(2)), point_3(2))
                        )
            };
}


}


namespace angeo {


collision_scene::collision_scene()
    : m_proximity_static_objects(
            std::bind(&collision_scene::get_object_aabb_min_corner, this, std::placeholders::_1),
            std::bind(&collision_scene::get_object_aabb_max_corner, this, std::placeholders::_1)
            )
    , m_proximity_dynamic_objects(
            std::bind(&collision_scene::get_object_aabb_min_corner, this, std::placeholders::_1),
            std::bind(&collision_scene::get_object_aabb_max_corner, this, std::placeholders::_1)
            )
    , m_dynamic_object_ids()
    , m_does_proximity_static_need_rebalancing(false)
    , m_does_proximity_dynamic_need_rebalancing(false)

    , m_disabled_colliding()

    , m_invalid_object_ids()

    , m_capsules_geometry()
    , m_capsules_bbox()
    , m_capsules_material()

    , m_lines_geometry()
    , m_lines_bbox()
    , m_lines_material()

    , m_points_geometry()
    , m_points_material()

    , m_spheres_geometry()
    , m_spheres_material()

    , m_triangles_geometry()
    , m_triangles_bbox()
    , m_triangles_material()
{}


collision_object_id  collision_scene::insert_capsule(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        matrix44 const&  from_base_matrix,
        COLLISION_MATERIAL_TYPE const  material,
        bool const  is_dynamic
        )
{
    collision_object_id  coid;
    {
        vector3 const  end_point_1_in_model_space(0.0f, 0.0f, +half_distance_between_end_points);
        vector3 const  end_point_2_in_model_space(0.0f, 0.0f, -half_distance_between_end_points);

        capsule_geometry const  geometry {
                transform_point(end_point_1_in_model_space, from_base_matrix),
                transform_point(end_point_2_in_model_space, from_base_matrix),
                half_distance_between_end_points,
                thickness_from_central_line
                };
        axis_aligned_bounding_box const  bbox =
                compute_aabb_of_capsule(
                        geometry.end_point_1_in_world_space,
                        geometry.end_point_2_in_world_space,
                        geometry.thickness_from_central_line
                        );

        auto&  invalid_ids = m_invalid_object_ids.at(as_number(COLLISION_SHAPE_TYPE::CAPSULE));
        if (invalid_ids.empty())
        {
            coid = make_collision_object_id(COLLISION_SHAPE_TYPE::CAPSULE, (natural_32_bit)m_capsules_geometry.size());

            m_capsules_geometry.push_back(geometry);
            m_capsules_bbox.push_back(bbox);
            m_capsules_material.push_back(material);
        }
        else
        {
            coid = make_collision_object_id(COLLISION_SHAPE_TYPE::CAPSULE, invalid_ids.back());

            m_capsules_geometry.at(invalid_ids.back()) = geometry;
            m_capsules_bbox.at(invalid_ids.back()) = bbox;
            m_capsules_material.at(invalid_ids.back()) = material;

            invalid_ids.pop_back();
        }
    }
    insert_object(coid, is_dynamic);
    return coid;
}


collision_object_id  collision_scene::insert_line(
        float_32_bit const  half_distance_between_end_points,
        matrix44 const&  from_base_matrix,
        COLLISION_MATERIAL_TYPE const  material,
        bool const  is_dynamic
        )
{
    collision_object_id  coid;
    {
        vector3 const  end_point_1_in_model_space(0.0f, 0.0f, +half_distance_between_end_points);
        vector3 const  end_point_2_in_model_space(0.0f, 0.0f, -half_distance_between_end_points);

        line_geometry const  geometry {
                transform_point(end_point_1_in_model_space, from_base_matrix),
                transform_point(end_point_2_in_model_space, from_base_matrix),
                half_distance_between_end_points
                };
        axis_aligned_bounding_box const  bbox =
                compute_aabb_of_line(geometry.end_point_1_in_world_space, geometry.end_point_2_in_world_space);

        auto&  invalid_ids = m_invalid_object_ids.at(as_number(COLLISION_SHAPE_TYPE::LINE));
        if (invalid_ids.empty())
        {
            coid = make_collision_object_id(COLLISION_SHAPE_TYPE::LINE, (natural_32_bit)m_lines_geometry.size());

            m_lines_geometry.push_back(geometry);
            m_lines_bbox.push_back(bbox);
            m_lines_material.push_back(material);

        }
        else
        {
            coid = make_collision_object_id(COLLISION_SHAPE_TYPE::LINE, invalid_ids.back());

            m_lines_geometry.at(invalid_ids.back()) = geometry;
            m_lines_bbox.at(invalid_ids.back()) = bbox;
            m_lines_material.at(invalid_ids.back()) = material;

            invalid_ids.pop_back();
        }
    }
    insert_object(coid, is_dynamic);
    return coid;
}


collision_object_id  collision_scene::insert_point(
        matrix44 const&  from_base_matrix,
        COLLISION_MATERIAL_TYPE const  material,
        bool const  is_dynamic  
        )
{
    collision_object_id  coid;
    {
        vector3 const  position_in_world_space = transform_point(vector3_zero(), from_base_matrix);

        auto&  invalid_ids = m_invalid_object_ids.at(as_number(COLLISION_SHAPE_TYPE::POINT));
        if (invalid_ids.empty())
        {
            coid = make_collision_object_id(COLLISION_SHAPE_TYPE::POINT, (natural_32_bit)m_points_geometry.size());

            m_points_geometry.push_back(position_in_world_space);
            m_points_material.push_back(material);

        }
        else
        {
            coid = make_collision_object_id(COLLISION_SHAPE_TYPE::POINT, invalid_ids.back());

            m_points_geometry.at(invalid_ids.back()) = position_in_world_space;
            m_points_material.at(invalid_ids.back()) = material;

            invalid_ids.pop_back();
        }
    }
    insert_object(coid, is_dynamic);
    return coid;
}


collision_object_id  collision_scene::insert_sphere(
        float_32_bit const  radius,
        matrix44 const&  from_base_matrix,
        COLLISION_MATERIAL_TYPE const  material,
        bool const  is_dynamic
        )
{
    collision_object_id  coid;
    {
        sphere_geometry const  geometry { transform_point(vector3_zero(), from_base_matrix), radius };

        auto&  invalid_ids = m_invalid_object_ids.at(as_number(COLLISION_SHAPE_TYPE::SPHERE));
        if (invalid_ids.empty())
        {
            coid = make_collision_object_id(COLLISION_SHAPE_TYPE::SPHERE, (natural_32_bit)m_spheres_geometry.size());

            m_spheres_geometry.push_back(geometry);
            m_spheres_material.push_back(material);

        }
        else
        {
            coid = make_collision_object_id(COLLISION_SHAPE_TYPE::SPHERE, invalid_ids.back());

            m_spheres_geometry.at(invalid_ids.back()) = geometry;
            m_spheres_material.at(invalid_ids.back()) = material;

            invalid_ids.pop_back();
        }
    }
    insert_object(coid, is_dynamic);
    return coid;
}


collision_object_id  collision_scene::insert_triangle(
        float_32_bit const  end_point_2_x_coord_in_model_space,
        vector2 const&  end_point_3_in_model_space,
        matrix44 const&  from_base_matrix,
        COLLISION_MATERIAL_TYPE const  material,
        bool const  is_dynamic
        )
{
    collision_object_id  coid;
    {
        vector3 const  end_point_1_in_model_space_ = vector3_zero();
        vector3 const  end_point_2_in_model_space_ = vector3(end_point_2_x_coord_in_model_space, 0.0f, 0.0f);
        vector3 const  end_point_3_in_model_space_ = expand23(end_point_3_in_model_space, 0.0f);

        triangle_geometry const  geometry {
                transform_point(end_point_1_in_model_space_, from_base_matrix),
                transform_point(end_point_2_in_model_space_, from_base_matrix),
                transform_point(end_point_3_in_model_space_, from_base_matrix),
                end_point_2_x_coord_in_model_space,
                end_point_3_in_model_space
                };
        axis_aligned_bounding_box const  bbox =
                compute_aabb_of_triangle(
                        geometry.end_point_1_in_world_space,
                        geometry.end_point_2_in_world_space,
                        geometry.end_point_3_in_world_space
                        );

        auto&  invalid_ids = m_invalid_object_ids.at(as_number(COLLISION_SHAPE_TYPE::TRIANGLE));
        if (invalid_ids.empty())
        {
            coid = make_collision_object_id(COLLISION_SHAPE_TYPE::TRIANGLE, (natural_32_bit)m_triangles_geometry.size());

            m_triangles_geometry.push_back(geometry);
            m_triangles_bbox.push_back(bbox);
            m_triangles_material.push_back(material);

        }
        else
        {
            coid = make_collision_object_id(COLLISION_SHAPE_TYPE::TRIANGLE, invalid_ids.back());

            m_triangles_geometry.at(invalid_ids.back()) = geometry;
            m_triangles_bbox.at(invalid_ids.back()) = bbox;
            m_triangles_material.at(invalid_ids.back()) = material;

            invalid_ids.pop_back();
        }
    }
    insert_object(coid, is_dynamic);
    return coid;
}


void  collision_scene::erase_object(collision_object_id const  coid)
{
    auto const  it = m_dynamic_object_ids.find(coid);
    if (it == m_dynamic_object_ids.end())
    {
        m_proximity_static_objects.erase(coid);
        m_does_proximity_static_need_rebalancing = true;
    }
    else
    {
        m_dynamic_object_ids.erase(it);
        m_proximity_dynamic_objects.erase(coid);
        m_does_proximity_dynamic_need_rebalancing = true;
    }

    m_invalid_object_ids.at(as_number(get_shape_type(coid))).push_back(get_instance_index(coid));
}


void  collision_scene::on_position_changed(collision_object_id const  coid, matrix44 const&  from_base_matrix)
{
    if (m_dynamic_object_ids.count(coid) == 0UL)
    {
        m_proximity_static_objects.erase(coid);

        update_shape_position(coid, from_base_matrix);

        m_proximity_static_objects.insert(coid);
        m_does_proximity_static_need_rebalancing = true;
    }
    else
    {
        m_proximity_dynamic_objects.erase(coid);

        update_shape_position(coid, from_base_matrix);

        m_proximity_dynamic_objects.insert(coid);
        m_does_proximity_dynamic_need_rebalancing = true;
    }
}


void  collision_scene::disable_colliding_of_dynamic_objects(
            collision_object_id const  coid_1,
            collision_object_id const  coid_2
            )
{
    m_disabled_colliding.insert(detail::make_collision_objects_pair(coid_1, coid_2));
}


void  collision_scene::enable_colliding_of_dynamic_objects(
            collision_object_id const  coid_1,
            collision_object_id const  coid_2
            )
{
    m_disabled_colliding.erase(detail::make_collision_objects_pair(coid_1, coid_2));
}


void  collision_scene::compute_contacts_of_all_dynamic_objects(contact_acceptor&  acceptor)
{
    // TODO!
}


void  collision_scene::compute_contacts_of_single_dynamic_object(
        collision_object_id const  coid,
        contact_acceptor&  acceptor
        )
{
    // TODO!
}


void  collision_scene::find_objects_in_proximity_to_axis_aligned_bounding_box(
        vector3 const& min_corner,
        vector3 const& max_corner,
        collision_object_acceptor&  acceptor
        )
{
    // TODO!
}


void  collision_scene::find_objects_in_proximity_to_line(
        vector3 const&  line_begin,
        vector3 const&  line_end,
        collision_object_acceptor&  acceptor
        )
{
    // TODO!
}


vector3  collision_scene::get_object_aabb_min_corner(collision_object_id const  coid) const
{
    switch (get_shape_type(coid))
    {
    case COLLISION_SHAPE_TYPE::CAPSULE:
        return m_capsules_bbox.at(get_instance_index(coid)).min_corner;
    case COLLISION_SHAPE_TYPE::LINE:
        return m_lines_bbox.at(get_instance_index(coid)).min_corner;
    case COLLISION_SHAPE_TYPE::POINT:
        return m_points_geometry.at(get_instance_index(coid));
        break;
    case COLLISION_SHAPE_TYPE::SPHERE:
        {
            auto const&  geometry = m_spheres_geometry.at(get_instance_index(coid));
            return geometry.center_in_world_space - vector3(geometry.radius, geometry.radius, geometry.radius);
        }
        break;
    case COLLISION_SHAPE_TYPE::TRIANGLE:
        return m_triangles_bbox.at(get_instance_index(coid)).min_corner;
        break;
    default:
        UNREACHABLE();
    }
}


vector3  collision_scene::get_object_aabb_max_corner(collision_object_id const  coid) const
{
    switch (get_shape_type(coid))
    {
    case COLLISION_SHAPE_TYPE::CAPSULE:
        return m_capsules_bbox.at(get_instance_index(coid)).max_corner;
    case COLLISION_SHAPE_TYPE::LINE:
        return m_lines_bbox.at(get_instance_index(coid)).max_corner;
    case COLLISION_SHAPE_TYPE::POINT:
        return m_points_geometry.at(get_instance_index(coid));
        break;
    case COLLISION_SHAPE_TYPE::SPHERE:
        {
            auto const&  geometry = m_spheres_geometry.at(get_instance_index(coid));
            return geometry.center_in_world_space + vector3(geometry.radius, geometry.radius, geometry.radius);
    }
        break;
    case COLLISION_SHAPE_TYPE::TRIANGLE:
        return m_triangles_bbox.at(get_instance_index(coid)).max_corner;
        break;
    default:
        UNREACHABLE();
    }
}


void  collision_scene::update_shape_position(collision_object_id const  coid, matrix44 const&  from_base_matrix)
{
    switch (get_shape_type(coid))
    {
    case COLLISION_SHAPE_TYPE::CAPSULE:
        {
            auto&  geometry = m_capsules_geometry.at(get_instance_index(coid));
            vector3 const  end_point_1_in_model_space(0.0f, 0.0f, +geometry.half_distance_between_end_points);
            vector3 const  end_point_2_in_model_space(0.0f, 0.0f, -geometry.half_distance_between_end_points);
            geometry.end_point_1_in_world_space = transform_point(end_point_1_in_model_space, from_base_matrix);
            geometry.end_point_2_in_world_space = transform_point(end_point_2_in_model_space, from_base_matrix);

            m_capsules_bbox.at(get_instance_index(coid)) =
                    compute_aabb_of_capsule(
                            geometry.end_point_1_in_world_space,
                            geometry.end_point_2_in_world_space,
                            geometry.thickness_from_central_line
                            );
        }
        break;
    case COLLISION_SHAPE_TYPE::LINE:
        {
            auto&  geometry = m_lines_geometry.at(get_instance_index(coid));
            vector3 const  end_point_1_in_model_space(0.0f, 0.0f, +geometry.half_distance_between_end_points);
            vector3 const  end_point_2_in_model_space(0.0f, 0.0f, -geometry.half_distance_between_end_points);
            geometry.end_point_1_in_world_space = transform_point(end_point_1_in_model_space, from_base_matrix);
            geometry.end_point_2_in_world_space = transform_point(end_point_2_in_model_space, from_base_matrix);

            m_lines_bbox.at(get_instance_index(coid)) =
                    compute_aabb_of_line(geometry.end_point_1_in_world_space, geometry.end_point_2_in_world_space);
        }
        break;
    case COLLISION_SHAPE_TYPE::POINT:
        m_points_geometry.at(get_instance_index(coid)) = transform_point(vector3_zero(), from_base_matrix);
        break;
    case COLLISION_SHAPE_TYPE::SPHERE:
        m_spheres_geometry.at(get_instance_index(coid)).center_in_world_space = transform_point(vector3_zero(), from_base_matrix);
        break;
    case COLLISION_SHAPE_TYPE::TRIANGLE:
        {
            auto&  geometry = m_triangles_geometry.at(get_instance_index(coid));
            vector3 const  end_point_1_in_model_space = vector3_zero();
            vector3 const  end_point_2_in_model_space = vector3(geometry.end_point_2_x_coord_in_model_space, 0.0f, 0.0f);
            vector3 const  end_point_3_in_model_space = expand23(geometry.end_point_3_in_model_space, 0.0f);
            geometry.end_point_1_in_world_space = transform_point(end_point_1_in_model_space, from_base_matrix);
            geometry.end_point_2_in_world_space = transform_point(end_point_2_in_model_space, from_base_matrix);
            geometry.end_point_2_in_world_space = transform_point(end_point_3_in_model_space, from_base_matrix);

            m_triangles_bbox.at(get_instance_index(coid)) =
                    compute_aabb_of_triangle(
                            geometry.end_point_1_in_world_space,
                            geometry.end_point_2_in_world_space,
                            geometry.end_point_3_in_world_space
                            );
        }
        break;
    default:
        UNREACHABLE();
    }
    // TODO!
}


void  collision_scene::insert_static_object(collision_object_id const  coid)
{
    m_proximity_static_objects.insert(coid);
    m_does_proximity_static_need_rebalancing = true;
}

void  collision_scene::insert_dynamic_object(collision_object_id const  coid)
{
    m_dynamic_object_ids.insert(coid);
    m_proximity_dynamic_objects.insert(coid);
    m_does_proximity_dynamic_need_rebalancing = true;
}


}
