#include <angeo/collision_scene.hpp>
#include <angeo/collide.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <set>

namespace angeo { namespace detail {


using  collision_shape_type_pair = std::pair<COLLISION_SHAPE_TYPE, COLLISION_SHAPE_TYPE>;

inline collision_shape_type_pair  make_collision_shape_type_pair(
        COLLISION_SHAPE_TYPE const  first,
        COLLISION_SHAPE_TYPE const  second
        ) noexcept
{
    return as_number(first) < as_number(second) ? collision_shape_type_pair{ first, second } :
                                                  collision_shape_type_pair{ second, first };
}

bool  makes_sense_to_check_bboxes_intersection(
            COLLISION_SHAPE_TYPE const  cst_1,
            COLLISION_SHAPE_TYPE const  cst_2
            )
{
    static std::unordered_set<std::pair<COLLISION_SHAPE_TYPE, COLLISION_SHAPE_TYPE> > const  useless_bbox_checks {
        make_collision_shape_type_pair(COLLISION_SHAPE_TYPE::POINT, COLLISION_SHAPE_TYPE::CAPSULE),
        make_collision_shape_type_pair(COLLISION_SHAPE_TYPE::POINT, COLLISION_SHAPE_TYPE::LINE),
        make_collision_shape_type_pair(COLLISION_SHAPE_TYPE::POINT, COLLISION_SHAPE_TYPE::POINT),
        make_collision_shape_type_pair(COLLISION_SHAPE_TYPE::POINT, COLLISION_SHAPE_TYPE::SPHERE),
        make_collision_shape_type_pair(COLLISION_SHAPE_TYPE::POINT, COLLISION_SHAPE_TYPE::TRIANGLE),

        make_collision_shape_type_pair(COLLISION_SHAPE_TYPE::SPHERE, COLLISION_SHAPE_TYPE::SPHERE),
    };
    return useless_bbox_checks.count(make_collision_shape_type_pair(cst_1, cst_2)) == 0UL;
}


inline collision_shape_feature_id  build_capsule_collision_shape_feature_id(float_32_bit const  param)
{
    return (param < 0.001f) ? make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, 0U) :
           (param > 0.999f) ? make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, 1U) :
                              make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::EDGE, 0U);
};


}}

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
    , m_triangles_neighbours_over_edges()
    , m_triangles_end_point_getters()
    , m_triangles_indices_of_invalidated_end_point_getters()

    , m_statistics(m_proximity_static_objects, m_proximity_dynamic_objects)
{}


collision_object_id  collision_scene::insert_capsule(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        matrix44 const&  from_base_matrix,
        COLLISION_MATERIAL_TYPE const  material,
        bool const  is_dynamic
        )
{
    TMPROF_BLOCK();

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

    ++m_statistics.num_capsules;

    return coid;
}


collision_object_id  collision_scene::insert_line(
        float_32_bit const  half_distance_between_end_points,
        matrix44 const&  from_base_matrix,
        COLLISION_MATERIAL_TYPE const  material,
        bool const  is_dynamic
        )
{
    TMPROF_BLOCK();

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

    ++m_statistics.num_lines;

    return coid;
}


collision_object_id  collision_scene::insert_point(
        matrix44 const&  from_base_matrix,
        COLLISION_MATERIAL_TYPE const  material,
        bool const  is_dynamic  
        )
{
    TMPROF_BLOCK();

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

    ++m_statistics.num_points;

    return coid;
}


collision_object_id  collision_scene::insert_sphere(
        float_32_bit const  radius,
        matrix44 const&  from_base_matrix,
        COLLISION_MATERIAL_TYPE const  material,
        bool const  is_dynamic
        )
{
    TMPROF_BLOCK();

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

    ++m_statistics.num_spheres;

    return coid;
}


void  collision_scene::insert_triangle_mesh(
        natural_32_bit const  num_triangles,
        std::function<vector3(natural_32_bit, natural_8_bit)> const&  getter_of_end_points_in_model_space,
        matrix44 const&  from_base_matrix,
        COLLISION_MATERIAL_TYPE const  material,
        bool const  is_dynamic, // Although not mandatory, it is recomended to pass 'false' here (for performance reasons).
        std::vector<collision_object_id>&  output_coids_of_individual_triangles
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(num_triangles > 0U);

    natural_32_bit  getter_index;
    if (m_triangles_indices_of_invalidated_end_point_getters.empty())
    {
        getter_index = (natural_32_bit)m_triangles_end_point_getters.size();
        m_triangles_end_point_getters.push_back({ getter_of_end_points_in_model_space, num_triangles });
    }
    else
    {
        getter_index = m_triangles_indices_of_invalidated_end_point_getters.back();
        m_triangles_indices_of_invalidated_end_point_getters.pop_back();
        m_triangles_end_point_getters.at(getter_index) = { getter_of_end_points_in_model_space, num_triangles };
    }

    auto const&  getter = m_triangles_end_point_getters.at(getter_index).first;

    for (natural_32_bit  i = 0U; i != num_triangles; ++i)
    {
        collision_object_id  coid;
        {
            vector3 const  end_point_1_in_world_space = transform_point(getter(i, 0U), from_base_matrix);
            vector3 const  end_point_2_in_world_space = transform_point(getter(i, 1U), from_base_matrix);
            vector3 const  end_point_3_in_world_space = transform_point(getter(i, 2U), from_base_matrix);
            triangle_geometry const  geometry {
                    end_point_1_in_world_space,
                    end_point_2_in_world_space,
                    end_point_3_in_world_space,
                    normalised(cross_product(end_point_2_in_world_space - end_point_1_in_world_space,
                                             end_point_3_in_world_space - end_point_1_in_world_space)),
                    i,
                    getter_index,
                    0U
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
                m_triangles_neighbours_over_edges.push_back({coid, coid, coid});
            }
            else
            {
                coid = make_collision_object_id(COLLISION_SHAPE_TYPE::TRIANGLE, invalid_ids.back());

                m_triangles_geometry.at(invalid_ids.back()) = geometry;
                m_triangles_bbox.at(invalid_ids.back()) = bbox;
                m_triangles_material.at(invalid_ids.back()) = material;
                m_triangles_neighbours_over_edges.at(invalid_ids.back()) = { coid, coid, coid };

                invalid_ids.pop_back();
            }
        }
        insert_object(coid, is_dynamic);
        output_coids_of_individual_triangles.push_back(coid);

        ++m_statistics.num_triangles;
    }
}


void  collision_scene::erase_object(collision_object_id const  coid)
{
    TMPROF_BLOCK();

    release_data_of_erased_object(coid);

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

    switch (get_shape_type(coid))
    {
    case COLLISION_SHAPE_TYPE::CAPSULE: --m_statistics.num_capsules; break;
    case COLLISION_SHAPE_TYPE::LINE: --m_statistics.num_lines; break;
    case COLLISION_SHAPE_TYPE::POINT: --m_statistics.num_points; break;
    case COLLISION_SHAPE_TYPE::SPHERE: --m_statistics.num_spheres; break;
    case COLLISION_SHAPE_TYPE::TRIANGLE: --m_statistics.num_triangles; break;
    }
}


void  collision_scene::clear()
{
    TMPROF_BLOCK();

    m_proximity_static_objects.clear();
    m_proximity_dynamic_objects.clear();

    m_dynamic_object_ids.clear();
    m_does_proximity_static_need_rebalancing = false;
    m_does_proximity_dynamic_need_rebalancing = false;

    m_disabled_colliding.clear();

    for (auto&  vec : m_invalid_object_ids)
        vec.clear();

    m_capsules_geometry.clear();
    m_capsules_bbox.clear();
    m_capsules_material.clear();

    m_lines_geometry.clear();
    m_lines_bbox.clear();
    m_lines_material.clear();

    m_points_geometry.clear();
    m_points_material.clear();

    m_spheres_geometry.clear();
    m_spheres_material.clear();

    m_triangles_geometry.clear();
    m_triangles_bbox.clear();
    m_triangles_material.clear();
    m_triangles_neighbours_over_edges.clear();
    m_triangles_end_point_getters.clear();
    m_triangles_indices_of_invalidated_end_point_getters.clear();

    m_statistics.clear();
}


void  collision_scene::on_position_changed(collision_object_id const  coid, matrix44 const&  from_base_matrix)
{
    TMPROF_BLOCK();

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


void  collision_scene::disable_colliding(
            collision_object_id const  coid_1,
            collision_object_id const  coid_2
            )
{
    m_disabled_colliding.insert(make_collision_object_id_pair(coid_1, coid_2));
}


void  collision_scene::enable_colliding(
            collision_object_id const  coid_1,
            collision_object_id const  coid_2
            )
{
    m_disabled_colliding.erase(make_collision_object_id_pair(coid_1, coid_2));
}


void  collision_scene::compute_contacts_of_all_dynamic_objects(contact_acceptor const&  acceptor, bool  with_static)
{
    TMPROF_BLOCK();

    {
        rebalance_dynamic_proximity_map_if_needed();

        std::unordered_set<collision_object_id_pair>  processed_collision_queries;
        natural_32_bit  current_leaf_node_index = 0U;
        std::set<collision_object_id>  cluster;
        auto const  process_cluster_content =
            [this, &processed_collision_queries, &acceptor, &with_static](std::set<collision_object_id> const&  cluster) -> bool {
                for (auto it = cluster.cbegin(); it != cluster.cend(); ++it)
                    for (auto next_it = std::next(it); next_it != cluster.cend(); ++next_it)
                    {
                        collision_object_id_pair const coid_pair =
                                make_collision_object_id_pair(*it, *next_it);
                        if (m_disabled_colliding.count(coid_pair) != 0UL)
                            continue;
                        if (processed_collision_queries.count(coid_pair) == 0UL)
                        {
                            processed_collision_queries.insert(coid_pair);
                            if (compute_contacts(coid_pair, acceptor, false) == false)
                            {
                                with_static = false;
                                return false;
                            }
                        }
                    }
                return true;
            };
        m_proximity_dynamic_objects.enumerate(
            [this, &process_cluster_content, &current_leaf_node_index, &cluster](
                    collision_object_id const  coid,
                    natural_32_bit const leaf_node_index
                    ) -> bool
                {
                    if (leaf_node_index != current_leaf_node_index)
                    {
                        if (process_cluster_content(cluster) == false)
                            return false;
                        cluster.clear();
                        current_leaf_node_index = leaf_node_index;
                    }
                    cluster.insert(coid);
                    return true;
                }
            );
        process_cluster_content(cluster);
    }
    if (with_static)
        for (auto const  coid : m_dynamic_object_ids)
            compute_contacts_of_single_dynamic_object(coid, acceptor, true, false);
}


void  collision_scene::compute_contacts_of_single_dynamic_object(
        collision_object_id const  coid,
        contact_acceptor const&  acceptor,
        bool const with_static,
        bool const with_dynamic
        )
{
    TMPROF_BLOCK();

    vector3 const  min_corner_of_objects_bbox = get_object_aabb_min_corner(coid);
    vector3 const  max_corner_of_objects_bbox = get_object_aabb_max_corner(coid);

    if (with_static)
    {
        rebalance_static_proximity_map_if_needed();
        std::unordered_set<collision_object_id>  visited{ coid };
        m_proximity_static_objects.find_by_bbox(
                min_corner_of_objects_bbox,
                max_corner_of_objects_bbox,
                [this, &acceptor, coid, &visited](collision_object_id const  other_coid) -> bool {
                        if (visited.count(other_coid) != 0UL)
                            return true;
                        collision_object_id_pair const coid_pair = make_collision_object_id_pair(coid, other_coid);
                        if (m_disabled_colliding.count(coid_pair) != 0UL)
                            return true;
                        visited.insert(other_coid);
                        return compute_contacts(coid_pair, acceptor, true);
                    }
                );
    }
    if (with_dynamic)
    {
        rebalance_dynamic_proximity_map_if_needed();
        std::unordered_set<collision_object_id>  visited{ coid };
        m_proximity_dynamic_objects.find_by_bbox(
                min_corner_of_objects_bbox,
                max_corner_of_objects_bbox,
                [this, &acceptor, coid, &visited](collision_object_id const  other_coid) -> bool {
                        if (visited.count(other_coid) != 0UL)
                            return true;
                        collision_object_id_pair const coid_pair = make_collision_object_id_pair(coid, other_coid);
                        if (m_disabled_colliding.count(coid_pair) != 0UL)
                            return true;
                        visited.insert(other_coid);
                        return compute_contacts(coid_pair, acceptor, true);
                    }
                );
    }
}


void  collision_scene::find_objects_in_proximity_to_axis_aligned_bounding_box(
        vector3 const& min_corner,
        vector3 const& max_corner,
        bool const search_static,
        bool const search_dynamic,
        collision_object_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    if (search_static)
    {
        rebalance_static_proximity_map_if_needed();
        std::unordered_set<collision_object_id>  visited;
        m_proximity_static_objects.find_by_bbox(
                min_corner,
                max_corner,
                [&acceptor, &visited](collision_object_id const  coid) -> bool {
                        if (visited.count(coid) != 0UL)
                            return true;
                        visited.insert(coid);
                        return acceptor(coid);
                    }
                );
    }
    if (search_dynamic)
    {
        rebalance_dynamic_proximity_map_if_needed();
        std::unordered_set<collision_object_id>  visited;
        m_proximity_dynamic_objects.find_by_bbox(
                min_corner,
                max_corner,
                [&acceptor, &visited](collision_object_id const  coid) -> bool {
                        if (visited.count(coid) != 0UL)
                            return true;
                        visited.insert(coid);
                        return acceptor(coid);
                    }
                );
    }
}


void  collision_scene::find_objects_in_proximity_to_line(
        vector3 const&  line_begin,
        vector3 const&  line_end,
        bool const search_static,
        bool const search_dynamic,
        collision_object_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    if (search_static)
    {
        rebalance_static_proximity_map_if_needed();
        std::unordered_set<collision_object_id>  visited;
        m_proximity_static_objects.find_by_line(
                line_begin,
                line_end,
                [&acceptor, &visited](collision_object_id const  coid) -> bool {
                        if (visited.count(coid) != 0UL)
                            return true;
                        visited.insert(coid);
                        return acceptor(coid);
                    }
                );
    }
    if (search_dynamic)
    {
        rebalance_dynamic_proximity_map_if_needed();
        std::unordered_set<collision_object_id>  visited;
        m_proximity_dynamic_objects.find_by_line(
                line_begin,
                line_end,
                [&acceptor, &visited](collision_object_id const  coid) -> bool {
                        if (visited.count(coid) != 0UL)
                            return true;
                        visited.insert(coid);
                        return acceptor(coid);
                    }
                );
    }
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
    case COLLISION_SHAPE_TYPE::SPHERE:
        {
            auto const&  geometry = m_spheres_geometry.at(get_instance_index(coid));
            return geometry.center_in_world_space + vector3(geometry.radius, geometry.radius, geometry.radius);
        }
    case COLLISION_SHAPE_TYPE::TRIANGLE:
        return m_triangles_bbox.at(get_instance_index(coid)).max_corner;
    default:
        UNREACHABLE();
    }
}

float_32_bit  collision_scene::get_capsule_half_distance_between_end_points(collision_object_id const  coid) const
{
    ASSUMPTION(get_shape_type((coid)) == COLLISION_SHAPE_TYPE::CAPSULE);
    capsule_geometry const&  geometry = m_capsules_geometry.at(get_instance_index(coid));
    return geometry.half_distance_between_end_points;
}

float_32_bit  collision_scene::get_capsule_thickness_from_central_line(collision_object_id const  coid) const
{
    ASSUMPTION(get_shape_type((coid)) == COLLISION_SHAPE_TYPE::CAPSULE);
    capsule_geometry const&  geometry = m_capsules_geometry.at(get_instance_index(coid));
    return geometry.thickness_from_central_line;
}

float_32_bit  collision_scene::get_sphere_radius(collision_object_id const  coid) const
{
    ASSUMPTION(get_shape_type((coid)) == COLLISION_SHAPE_TYPE::SPHERE);
    sphere_geometry const&  geometry = m_spheres_geometry.at(get_instance_index(coid));
    return geometry.radius;
}

std::function<vector3(natural_32_bit, natural_8_bit)> const&  collision_scene::get_triangle_points_getter(
        collision_object_id const  coid) const
{
    ASSUMPTION(get_shape_type((coid)) == COLLISION_SHAPE_TYPE::TRIANGLE);
    triangle_geometry const&  geometry = m_triangles_geometry.at(get_instance_index(coid));
    return m_triangles_end_point_getters.at(geometry.end_points_getter_index).first;
}

natural_32_bit  collision_scene::get_triangle_index(collision_object_id const  coid) const
{
    ASSUMPTION(get_shape_type((coid)) == COLLISION_SHAPE_TYPE::TRIANGLE);
    return m_triangles_geometry.at(get_instance_index(coid)).triangle_index;
}

vector3 const&  collision_scene::get_triangle_end_point_in_world_space(collision_object_id const  coid, natural_8_bit const  end_point_index) const
{
    ASSUMPTION(get_shape_type((coid)) == COLLISION_SHAPE_TYPE::TRIANGLE);
    triangle_geometry const&  geometry = m_triangles_geometry.at(get_instance_index(coid));
    return end_point_index == 0U ? geometry.end_point_1_in_world_space :
           end_point_index == 1U ? geometry.end_point_2_in_world_space :
                                   geometry.end_point_3_in_world_space ;
}

vector3 const&  collision_scene::get_triangle_unit_normal_in_world_space(collision_object_id const  coid) const
{
    ASSUMPTION(get_shape_type((coid)) == COLLISION_SHAPE_TYPE::TRIANGLE);
    return m_triangles_geometry.at(get_instance_index(coid)).unit_normal_in_world_space;
}

natural_8_bit  collision_scene::get_trinagle_edges_ignore_mask(collision_object_id const  coid) const
{
    ASSUMPTION(get_shape_type((coid)) == COLLISION_SHAPE_TYPE::TRIANGLE);
    return m_triangles_geometry.at(get_instance_index(coid)).edges_ignore_mask;
}

void  collision_scene::set_trinagle_edges_ignore_mask(collision_object_id const  coid, natural_8_bit const  mask)
{
    ASSUMPTION(get_shape_type((coid)) == COLLISION_SHAPE_TYPE::TRIANGLE);
    m_triangles_geometry.at(get_instance_index(coid)).edges_ignore_mask = mask;
}

collision_object_id   collision_scene::get_trinagle_neighbour_over_edge(
        collision_object_id const  coid,
        natural_32_bit const  edge_index
        ) const
{
    ASSUMPTION(get_shape_type((coid)) == COLLISION_SHAPE_TYPE::TRIANGLE && edge_index < 3U);
    return m_triangles_neighbours_over_edges.at(get_instance_index(coid)).at(edge_index);
}

void  collision_scene::set_trinagle_neighbour_over_edge(
        collision_object_id const  coid,
        natural_32_bit const  edge_index,
        collision_object_id const  neighbour_triangle_coid
        )
{
    ASSUMPTION(get_shape_type((coid)) == COLLISION_SHAPE_TYPE::TRIANGLE && edge_index < 3U);
    m_triangles_neighbours_over_edges.at(get_instance_index(coid)).at(edge_index) = neighbour_triangle_coid;
}

COLLISION_MATERIAL_TYPE  collision_scene::get_material(collision_object_id const  coid) const
{
    switch (get_shape_type(coid))
    {
    case COLLISION_SHAPE_TYPE::CAPSULE:
        return m_capsules_material.at(get_instance_index(coid));
    case COLLISION_SHAPE_TYPE::LINE:
        return m_lines_material.at(get_instance_index(coid));
    case COLLISION_SHAPE_TYPE::POINT:
        return m_points_material.at(get_instance_index(coid));
    case COLLISION_SHAPE_TYPE::SPHERE:
        return m_spheres_material.at(get_instance_index(coid));
    case COLLISION_SHAPE_TYPE::TRIANGLE:
        return m_triangles_material.at(get_instance_index(coid));
    default:
        UNREACHABLE();
    }
}


void  collision_scene::release_data_of_erased_object(collision_object_id const  coid)
{
    switch (get_shape_type(coid))
    {
    case COLLISION_SHAPE_TYPE::CAPSULE:
    case COLLISION_SHAPE_TYPE::LINE:
    case COLLISION_SHAPE_TYPE::POINT:
    case COLLISION_SHAPE_TYPE::SPHERE:
        break; // Nothing to release for these coids.
    case COLLISION_SHAPE_TYPE::TRIANGLE:
        {
            natural_32_bit const  instance_idx = get_instance_index(coid);

            auto&  geometry = m_triangles_geometry.at(instance_idx);
            auto&  getter_info = m_triangles_end_point_getters.at(geometry.end_points_getter_index);
            ASSUMPTION(getter_info.second > 0U);
            --getter_info.second;
            if (getter_info.second == 0U)
                m_triangles_indices_of_invalidated_end_point_getters.push_back(geometry.end_points_getter_index);

            std::array<collision_object_id, 3U> const&  neighbours = m_triangles_neighbours_over_edges.at(instance_idx);
            for (natural_32_bit  i = 0U; i!= 3U; ++i)
            {
                collision_object_id const  neighbour_i_coid = neighbours.at(i);
                if (neighbour_i_coid != coid)
                {
                    std::array<collision_object_id, 3U>&  neighbour_i_neighbours =
                            m_triangles_neighbours_over_edges.at(get_instance_index(neighbour_i_coid))
                            ;
                    for (natural_32_bit  j = 0U; j != 3U; ++j)
                        if (neighbour_i_neighbours.at(j) == coid)
                            neighbour_i_neighbours.at(j) = neighbour_i_coid;
                }
            }
        }
        break;
    default:
        UNREACHABLE();
    }
}


void  collision_scene::update_shape_position(collision_object_id const  coid, matrix44 const&  from_base_matrix)
{
    TMPROF_BLOCK();

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
            auto const&  getter = m_triangles_end_point_getters.at(geometry.end_points_getter_index).first;
            geometry.end_point_1_in_world_space = transform_point(getter(geometry.triangle_index, 0U), from_base_matrix);
            geometry.end_point_2_in_world_space = transform_point(getter(geometry.triangle_index, 1U), from_base_matrix);
            geometry.end_point_3_in_world_space = transform_point(getter(geometry.triangle_index, 2U), from_base_matrix);
            geometry.unit_normal_in_world_space = 
                normalised(cross_product(geometry.end_point_2_in_world_space - geometry.end_point_1_in_world_space,
                                         geometry.end_point_3_in_world_space - geometry.end_point_1_in_world_space))
                ;

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

void  collision_scene::rebalance_static_proximity_map_if_needed()
{
    if (m_does_proximity_static_need_rebalancing)
    {
        m_proximity_static_objects.rebalance();
        m_does_proximity_static_need_rebalancing = false;
    }
}

void  collision_scene::rebalance_dynamic_proximity_map_if_needed()
{
    if (m_does_proximity_dynamic_need_rebalancing)
    {
        m_proximity_dynamic_objects.rebalance();
        m_does_proximity_dynamic_need_rebalancing = false;
    }
}


bool  collision_scene::compute_contacts(
        collision_object_id_pair  cop,
        contact_acceptor const&  contact_acceptor_,
        bool const  bboxes_of_objects_surely_intersect
        )
{
    TMPROF_BLOCK();

    ++m_statistics.num_compute_contacts_calls_in_last_frame;

    COLLISION_SHAPE_TYPE const  shape_type_1 = get_shape_type(cop.first);
    COLLISION_SHAPE_TYPE const  shape_type_2 = get_shape_type(cop.second);

    if (bboxes_of_objects_surely_intersect == false
            && detail::makes_sense_to_check_bboxes_intersection(shape_type_1,shape_type_2))
    {
        if (false == collision_bbox_bbox(
                            get_object_aabb_min_corner(cop.first),
                            get_object_aabb_max_corner(cop.first),
                            get_object_aabb_min_corner(cop.second),
                            get_object_aabb_max_corner(cop.second)
                            ))
            return true; // I.e. do not stop a high-level contact search algorithm.
    }

    auto const  acceptor =
            [this, &contact_acceptor_](contact_id const& cid,
                        vector3 const& contact_point,
                        vector3 const& unit_normal,
                        float_32_bit const  penetration_depth)
                        -> bool {
                ++m_statistics.num_contacts_in_last_frame;
                return contact_acceptor_(cid, contact_point, unit_normal, penetration_depth);
            };

    auto const  swap_acceptor =
            [this, &contact_acceptor_](contact_id const& cid,
                        vector3 const& contact_point,
                        vector3 const& unit_normal,
                        float_32_bit const  penetration_depth)
                        -> bool {
                ++m_statistics.num_contacts_in_last_frame;
                return contact_acceptor_(contact_id(cid.second, cid.first), contact_point, -unit_normal, penetration_depth);
            };

    switch (shape_type_1)
    {
    case COLLISION_SHAPE_TYPE::CAPSULE:
        switch (shape_type_2)
        {
        case COLLISION_SHAPE_TYPE::CAPSULE:
            return compute_contacts__capsule_vs_capsule(cop.first, cop.second, acceptor);
        case COLLISION_SHAPE_TYPE::LINE:
            return compute_contacts__capsule_vs_line(cop.first, cop.second, acceptor);
        case COLLISION_SHAPE_TYPE::POINT:
            return compute_contacts__capsule_vs_point(cop.first, cop.second, acceptor);
        case COLLISION_SHAPE_TYPE::SPHERE:
            return compute_contacts__capsule_vs_sphere(cop.first, cop.second, acceptor);
        case COLLISION_SHAPE_TYPE::TRIANGLE:
            return compute_contacts__capsule_vs_triangle(cop.first, cop.second, acceptor);
        default: UNREACHABLE();
        }
        break;
    case COLLISION_SHAPE_TYPE::LINE:
        switch (shape_type_2)
        {
        case COLLISION_SHAPE_TYPE::CAPSULE:
            return compute_contacts__capsule_vs_line(cop.second, cop.first, swap_acceptor);
        case COLLISION_SHAPE_TYPE::LINE:
            return compute_contacts__line_vs_line(cop.first, cop.second, acceptor);
        case COLLISION_SHAPE_TYPE::POINT:
            return compute_contacts__line_vs_point(cop.first, cop.second, acceptor);
        case COLLISION_SHAPE_TYPE::SPHERE:
            return compute_contacts__line_vs_sphere(cop.first, cop.second, acceptor);
        case COLLISION_SHAPE_TYPE::TRIANGLE:
            return compute_contacts__line_vs_triangle(cop.first, cop.second, acceptor);
        default: UNREACHABLE();
        }
        break;
    case COLLISION_SHAPE_TYPE::POINT:
        switch (shape_type_2)
        {
        case COLLISION_SHAPE_TYPE::CAPSULE:
            return compute_contacts__capsule_vs_point(cop.second, cop.first, swap_acceptor);
        case COLLISION_SHAPE_TYPE::LINE:
            return compute_contacts__line_vs_point(cop.second, cop.first, swap_acceptor);
        case COLLISION_SHAPE_TYPE::POINT:
            return compute_contacts__point_vs_point(cop.first, cop.second, acceptor);
        case COLLISION_SHAPE_TYPE::SPHERE:
            return compute_contacts__point_vs_sphere(cop.first, cop.second, acceptor);
        case COLLISION_SHAPE_TYPE::TRIANGLE:
            return compute_contacts__point_vs_triangle(cop.first, cop.second, acceptor);
        default: UNREACHABLE();
        }
        break;
    case COLLISION_SHAPE_TYPE::SPHERE:
        switch (shape_type_2)
        {
        case COLLISION_SHAPE_TYPE::CAPSULE:
            return compute_contacts__capsule_vs_sphere(cop.second, cop.first, swap_acceptor);
        case COLLISION_SHAPE_TYPE::LINE:
            return compute_contacts__line_vs_sphere(cop.second, cop.first, swap_acceptor);
        case COLLISION_SHAPE_TYPE::POINT:
            return compute_contacts__point_vs_sphere(cop.second, cop.first, swap_acceptor);
        case COLLISION_SHAPE_TYPE::SPHERE:
            return compute_contacts__sphere_vs_sphere(cop.first, cop.second, acceptor);
        case COLLISION_SHAPE_TYPE::TRIANGLE:
            return compute_contacts__sphere_vs_triangle(cop.first, cop.second, acceptor);
        default: UNREACHABLE();
        }
        break;
    case COLLISION_SHAPE_TYPE::TRIANGLE:
        switch (shape_type_2)
        {
        case COLLISION_SHAPE_TYPE::CAPSULE:
            return compute_contacts__capsule_vs_triangle(cop.second, cop.first, swap_acceptor);
        case COLLISION_SHAPE_TYPE::LINE:
            return compute_contacts__line_vs_triangle(cop.second, cop.first, swap_acceptor);
        case COLLISION_SHAPE_TYPE::POINT:
            return compute_contacts__point_vs_triangle(cop.second, cop.first, swap_acceptor);
        case COLLISION_SHAPE_TYPE::SPHERE:
            return compute_contacts__sphere_vs_triangle(cop.second, cop.first, swap_acceptor);
        case COLLISION_SHAPE_TYPE::TRIANGLE:
            return compute_contacts__triangle_vs_triangle(cop.first, cop.second, acceptor);
        default: UNREACHABLE();
        }
        break;
    default: UNREACHABLE();
    }

    return true; // I.e. do not stop a high-level contact search algorithm.
}


bool  collision_scene::compute_contacts__capsule_vs_capsule(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    capsule_geometry const&  geometry_1 = m_capsules_geometry.at(get_instance_index(coid_1));
    capsule_geometry const&  geometry_2 = m_capsules_geometry.at(get_instance_index(coid_2));

    vector3  capsule_1_point_1, capsule_1_point_2;
    float_32_bit  capsule_1_param_1, capsule_1_param_2;
    vector3  capsule_2_point_1, capsule_2_point_2;
    float_32_bit  capsule_2_param_1, capsule_2_param_2;
    natural_32_bit const  num_collision_point_pairs = closest_points_of_two_lines(
        geometry_1.end_point_1_in_world_space,
        geometry_1.end_point_2_in_world_space,
        geometry_2.end_point_1_in_world_space,
        geometry_2.end_point_2_in_world_space,
        &capsule_1_point_1, &capsule_1_param_1,
        &capsule_2_point_1, &capsule_2_param_1,
        &capsule_1_point_2, &capsule_1_param_2,
        &capsule_2_point_2, &capsule_2_param_2
        );

    float_32_bit const  separation_distance = geometry_1.thickness_from_central_line + geometry_2.thickness_from_central_line;
    float_32_bit constexpr  epsilon_distance = 0.0001f;
    float_32_bit const  min_thickness = std::min(geometry_1.thickness_from_central_line, geometry_2.thickness_from_central_line);
    float_32_bit const  max_thickness = std::max(geometry_1.thickness_from_central_line, geometry_2.thickness_from_central_line);

    {
        vector3 const  u = capsule_1_point_1 - capsule_2_point_1;
        float_32_bit const  u_len = length(u);
        if (u_len < separation_distance)
        {
            float_32_bit const  penetration_depth = std::min(separation_distance - u_len, max_thickness);

            vector3  unit_normal;
            float_32_bit  contact_point_param;
            if (u_len > epsilon_distance)
            {
                unit_normal = u / u_len;
                contact_point_param =
                        geometry_2.thickness_from_central_line
                            - penetration_depth
                            + std::min(0.5f * penetration_depth, min_thickness)
                        ;
            }
            else
            {
                unit_normal = vector3_unit_z();
                contact_point_param = 0.0f;
            }

            if (acceptor(
                    {
                        { coid_1, detail::build_capsule_collision_shape_feature_id(capsule_1_param_1) },
                        { coid_2, detail::build_capsule_collision_shape_feature_id(capsule_2_param_1) }
                    },
                    capsule_2_point_1 + contact_point_param * unit_normal,
                    unit_normal,
                    penetration_depth
                    ) == false)
                return false;
        }
    }
    if (num_collision_point_pairs == 2U)
    {
        vector3 const  u = capsule_1_point_2 - capsule_2_point_2;
        float_32_bit const  u_len = length(u);
        if (u_len < separation_distance)
        {
            float_32_bit const  penetration_depth = std::min(separation_distance - u_len, max_thickness);

            vector3  unit_normal;
            float_32_bit  contact_point_param;
            if (u_len > epsilon_distance)
            {
                unit_normal = u / u_len;
                contact_point_param =
                        geometry_2.thickness_from_central_line
                            - penetration_depth
                            + std::min(0.5f * penetration_depth, min_thickness)
                        ;
            }
            else
            {
                unit_normal = vector3_unit_z();
                contact_point_param = 0.0f;
            }

            if (acceptor(
                    {
                        { coid_1, detail::build_capsule_collision_shape_feature_id(capsule_1_param_2) },
                        { coid_2, detail::build_capsule_collision_shape_feature_id(capsule_2_param_2) }
                    },
                    capsule_2_point_2 + contact_point_param * unit_normal,
                    unit_normal,
                    penetration_depth
                    ) == false)
                return false;
        }
    }
    return true;
}

bool  collision_scene::compute_contacts__capsule_vs_line(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    capsule_geometry const&  geometry_1 = m_capsules_geometry.at(get_instance_index(coid_1));
    line_geometry const&  geometry_2 = m_lines_geometry.at(get_instance_index(coid_2));

    NOT_IMPLEMENTED_YET();
}

bool  collision_scene::compute_contacts__capsule_vs_point(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    capsule_geometry const&  geometry_1 = m_capsules_geometry.at(get_instance_index(coid_1));
    vector3 const&  point_2 = m_points_geometry.at(get_instance_index(coid_2));

    NOT_IMPLEMENTED_YET();
}

bool  collision_scene::compute_contacts__capsule_vs_sphere(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    capsule_geometry const&  geometry_1 = m_capsules_geometry.at(get_instance_index(coid_1));
    sphere_geometry const&  geometry_2 = m_spheres_geometry.at(get_instance_index(coid_2));

    vector3  capsule_point;
    float_32_bit const  capsule_point_param = closest_point_on_line_to_point(
            geometry_1.end_point_1_in_world_space,
            geometry_1.end_point_2_in_world_space,
            geometry_2.center_in_world_space,
            &capsule_point
            );

    vector3 const  u = capsule_point - geometry_2.center_in_world_space;
    float_32_bit const  u_len = length(u);
    float_32_bit const  separation_distance = geometry_1.thickness_from_central_line + geometry_2.radius;
    if (u_len < separation_distance)
    {
        float_32_bit const  epsilon_distance = 0.0001f;

        float_32_bit const  min_thickness = std::min(geometry_1.thickness_from_central_line, geometry_2.radius);
        float_32_bit const  max_thickness = std::max(geometry_1.thickness_from_central_line, geometry_2.radius);

        float_32_bit const  penetration_depth = std::min(separation_distance - u_len, max_thickness);

        vector3  unit_normal;
        float_32_bit  contact_point_param;
        if (u_len > epsilon_distance)
        {
            unit_normal = u / u_len;
            contact_point_param = geometry_2.radius - penetration_depth + std::min(0.5f * penetration_depth, min_thickness);
        }
        else
        {
            unit_normal = vector3_unit_z();
            contact_point_param = 0.0f;
        }

        return acceptor(
                    {
                        { coid_1, detail::build_capsule_collision_shape_feature_id(capsule_point_param) },
                        { coid_2, make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, 0U) }
                    },
                    geometry_2.center_in_world_space + contact_point_param * unit_normal,
                    unit_normal,
                    penetration_depth
                    );
    }
    return true;
}

bool  collision_scene::compute_contacts__capsule_vs_triangle(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    capsule_geometry const&  geometry_1 = m_capsules_geometry.at(get_instance_index(coid_1));
    triangle_geometry const&  geometry_2 = m_triangles_geometry.at(get_instance_index(coid_2));

    vector3 triangle_closest_point_1;
    collision_shape_feature_id  triangle_shape_feature_id_1;
    vector3  capsule_closest_point_1;
    collision_shape_feature_id  capsule_shape_feature_id_1;

    vector3 triangle_closest_point_2;
    collision_shape_feature_id  triangle_shape_feature_id_2;
    vector3  capsule_closest_point_2;
    collision_shape_feature_id  capsule_shape_feature_id_2;

    natural_32_bit const  num_collision_point_pairs = closest_points_of_triangle_and_line(
            geometry_2.end_point_1_in_world_space,
            geometry_2.end_point_2_in_world_space,
            geometry_2.end_point_3_in_world_space,

            geometry_2.unit_normal_in_world_space,

            geometry_1.end_point_1_in_world_space,
            geometry_1.end_point_2_in_world_space,

            &triangle_closest_point_1,
            &triangle_shape_feature_id_1,
            &capsule_closest_point_1,
            &capsule_shape_feature_id_1,

            &triangle_closest_point_2,
            &triangle_shape_feature_id_2,
            &capsule_closest_point_2,
            &capsule_shape_feature_id_2
            );
    if (num_collision_point_pairs == 0U)
        return true;
    {
        vector3 const  u = capsule_closest_point_1 - triangle_closest_point_1;
        float_32_bit const  u_len = length(u);
        if (u_len < geometry_1.thickness_from_central_line)
            if (acceptor(
                    { { coid_1, capsule_shape_feature_id_1 }, { coid_2, triangle_shape_feature_id_1 }, },
                    triangle_closest_point_1,
                    u_len > 0.0001f ? vector3(u / u_len) : vector3_unit_z(),
                    geometry_1.thickness_from_central_line - u_len
                    ) == false)
                return false;
    }
    if (num_collision_point_pairs == 2U)
    {
        vector3 const  u = capsule_closest_point_2 - triangle_closest_point_2;
        float_32_bit const  u_len = length(u);
        if (u_len < geometry_1.thickness_from_central_line)
            if (acceptor(
                    { { coid_1, capsule_shape_feature_id_2 }, { coid_2, triangle_shape_feature_id_2 }, },
                    triangle_closest_point_2,
                    u_len > 0.0001f ? vector3(u / u_len) : vector3_unit_z(),
                    geometry_1.thickness_from_central_line - u_len
                    ) == false)
                return false;
    }
    return true;
}


bool  collision_scene::compute_contacts__line_vs_line(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    line_geometry const&  geometry_1 = m_lines_geometry.at(get_instance_index(coid_1));
    line_geometry const&  geometry_2 = m_lines_geometry.at(get_instance_index(coid_2));

    NOT_IMPLEMENTED_YET();
}

bool  collision_scene::compute_contacts__line_vs_point(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    line_geometry const&  geometry_1 = m_lines_geometry.at(get_instance_index(coid_1));
    vector3 const&  point_2 = m_points_geometry.at(get_instance_index(coid_2));

    NOT_IMPLEMENTED_YET();
}

bool  collision_scene::compute_contacts__line_vs_sphere(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    line_geometry const&  geometry_1 = m_lines_geometry.at(get_instance_index(coid_1));
    sphere_geometry const&  geometry_2 = m_spheres_geometry.at(get_instance_index(coid_2));

    NOT_IMPLEMENTED_YET();
}

bool  collision_scene::compute_contacts__line_vs_triangle(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    line_geometry const&  geometry_1 = m_lines_geometry.at(get_instance_index(coid_1));
    triangle_geometry const&  geometry_2 = m_triangles_geometry.at(get_instance_index(coid_2));

    NOT_IMPLEMENTED_YET();
}


bool  collision_scene::compute_contacts__point_vs_point(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    vector3 const&  point_1 = m_points_geometry.at(get_instance_index(coid_1));
    vector3 const&  point_2 = m_points_geometry.at(get_instance_index(coid_2));

    NOT_IMPLEMENTED_YET();
}

bool  collision_scene::compute_contacts__point_vs_sphere(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    vector3 const&  point_1 = m_points_geometry.at(get_instance_index(coid_1));
    sphere_geometry const&  geometry_2 = m_spheres_geometry.at(get_instance_index(coid_2));

    NOT_IMPLEMENTED_YET();
}

bool  collision_scene::compute_contacts__point_vs_triangle(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    vector3 const&  point_1 = m_points_geometry.at(get_instance_index(coid_1));
    triangle_geometry const&  geometry_2 = m_triangles_geometry.at(get_instance_index(coid_2));

    NOT_IMPLEMENTED_YET();
}


bool  collision_scene::compute_contacts__sphere_vs_sphere(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    sphere_geometry const&  geometry_1 = m_spheres_geometry.at(get_instance_index(coid_1));
    sphere_geometry const&  geometry_2 = m_spheres_geometry.at(get_instance_index(coid_2));

    vector3 const  u = geometry_1.center_in_world_space - geometry_2.center_in_world_space;
    float_32_bit const  u_len = length(u);
    float_32_bit const  separation_distance = geometry_1.radius + geometry_2.radius;
    if (u_len < separation_distance)
    {
        contact_id const  cid{
            { coid_1, make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, 0U) },
            { coid_2, make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, 0U) },
        };

        float_32_bit const  max_thickness = std::max(geometry_1.radius, geometry_2.radius);
        float_32_bit const  penetration_depth = std::min(separation_distance - u_len, max_thickness);

        vector3  contact_point;
        vector3  unit_normal;
        {
            float_32_bit const  epsilon_distance = 0.0001f;
            if (u_len > epsilon_distance)
            {
                unit_normal = u / u_len;
                float_32_bit const  min_thickness = std::min(geometry_1.radius, geometry_2.radius);
                float_32_bit const  contact_point_param =
                        geometry_2.radius - penetration_depth + std::min(0.5f * penetration_depth, min_thickness)
                        ;
                contact_point = geometry_2.center_in_world_space + contact_point_param * unit_normal;
            }
            else
            {
                unit_normal = vector3_unit_z();
                contact_point = geometry_2.center_in_world_space;
            }
        }

        return acceptor(cid, contact_point, unit_normal, penetration_depth);
    }
    return true;
}

bool  collision_scene::compute_contacts__sphere_vs_triangle(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    sphere_geometry const&  geometry_1 = m_spheres_geometry.at(get_instance_index(coid_1));
    triangle_geometry const&  geometry_2 = m_triangles_geometry.at(get_instance_index(coid_2));

    vector3  triangle_closest_point;
    collision_shape_feature_id  triangle_shape_feature_id;
    if (false == closest_point_of_triangle_to_point(
            geometry_2.end_point_1_in_world_space,
            geometry_2.end_point_2_in_world_space,
            geometry_2.end_point_3_in_world_space,
            geometry_2.unit_normal_in_world_space,
            true,
            geometry_1.center_in_world_space,
            &triangle_closest_point,
            &triangle_shape_feature_id,
            nullptr
            ))
        return true;

    vector3 const  u = geometry_1.center_in_world_space - triangle_closest_point;
    float_32_bit const  u_len = length(u);
    if (u_len < geometry_1.radius)
        return acceptor(
                    {
                        { coid_1, make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, 0U) },
                        { coid_2, triangle_shape_feature_id },
                    },
                    triangle_closest_point,
                    u_len > 0.0001f ? vector3(u / u_len) : vector3_unit_z(),
                    geometry_1.radius - u_len
                    );
    return true;
}


bool  collision_scene::compute_contacts__triangle_vs_triangle(
        collision_object_id const  coid_1,
        collision_object_id const  coid_2,
        contact_acceptor const&  acceptor
        )
{
    TMPROF_BLOCK();

    triangle_geometry const&  geometry_1 = m_triangles_geometry.at(get_instance_index(coid_1));
    triangle_geometry const&  geometry_2 = m_triangles_geometry.at(get_instance_index(coid_2));

    NOT_IMPLEMENTED_YET();
}



}
