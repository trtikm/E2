#include <gfxtuner/scene_utils.hpp>
#include <gfxtuner/scene_selection.hpp>
#include <angeo/collide.hpp>
#include <utility/timeprof.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>


void  transform_origin_and_orientation(
        matrix44 const  transformation,
        vector3&  origin,
        quaternion&  orientation
        )
{
    origin = contract43(transformation * expand34(origin));
    vector3  x, y, z;
    rotation_matrix_to_basis(quaternion_to_rotation_matrix(orientation), x, y, z);
    matrix33 rotation;
    basis_to_rotation_matrix(
        contract43(transformation * expand34(x, 0.0f)),
        contract43(transformation * expand34(y, 0.0f)),
        contract43(transformation * expand34(z, 0.0f)),
        rotation
        );
    orientation = rotation_matrix_to_quaternion(rotation);
}


bool  compute_collision_of_scene_node_and_line(
        scene_node const&  node,
        vector3 const&  line_begin,
        vector3 const&  line_end,
        scalar* const  parameter_on_line_to_collision_point
        )
{
    matrix44 const  to_node_space = inverse(node.get_world_matrix());
    vector3 const  A = contract43(to_node_space * expand34(line_begin));
    vector3 const  B = contract43(to_node_space * expand34(line_end));

    auto const  collide_with_bounding_sphere = [&A, &B](
                    scalar const  radius,
                    scalar* const  out_param = nullptr,
                    scalar* const  out_distance_from_origin = nullptr
                    ) -> bool {
        vector3  closest_point;
        scalar const  param = angeo::closest_point_on_line_to_point(A, B, vector3_zero(), &closest_point);
        scalar const  distance = length(closest_point);
        if (param < 0.0f || param > 1.0f || distance > radius + 0.001f)
            return false;
        else
        {
            if (out_param != nullptr)
                *out_param = param;
            if (out_distance_from_origin != nullptr)
                *out_distance_from_origin = distance;
            return true;
        }
    };

    if (node.get_batches().empty())
        return collide_with_bounding_sphere(
                        get_selection_radius_of_bounding_sphere_of_scene_node(),
                        parameter_on_line_to_collision_point
                        );

    auto const  collide_with_bounding_box = [&A, &B](
                    vector3 const&  bbox_lo_corner,
                    vector3 const&  bbox_hi_corner,
                    scalar* const  out_param = nullptr,
                    scalar* const  out_distance_from_origin = nullptr
                    ) -> bool {
        vector3  point;
        scalar  param;
        if (!angeo::clip_line_into_bbox(A, B, bbox_lo_corner, bbox_hi_corner, &point, nullptr, &param, nullptr))
            return false;
        if (out_param != nullptr)
            *out_param = param;
        if (out_distance_from_origin != nullptr)
            *out_distance_from_origin = length(point);
        return true;
    };

    scalar  min_param = 2.0f;
    for (auto const& name_and_batch : node.get_batches())
        if (auto const  binding_ptr = name_and_batch.second->buffers_binding())
            if (auto const  buffer_props_ptr = binding_ptr->find_vertex_buffer_properties())
            {
                qtgl::spatial_boundary const&  boundary = buffer_props_ptr->boundary();

                scalar  sphere_param;
                scalar  sphere_distance_to_origin;
                bool const  sphere_collision = collide_with_bounding_sphere(boundary.radius(), &sphere_param, &sphere_distance_to_origin);

                scalar  bbox_param;
                scalar  bbox_distance_to_origin;
                bool const  bbox_collision = collide_with_bounding_box(boundary.lo_corner(), boundary.hi_corner(), &bbox_param, &bbox_distance_to_origin);

                if (sphere_collision && bbox_collision)
                {
                    scalar const  param = sphere_distance_to_origin < bbox_distance_to_origin ? sphere_param : bbox_param;
                    if (param < min_param)
                        min_param = param;
                }
            }

    if (min_param > 1.0f)
        return false;
    if (parameter_on_line_to_collision_point != nullptr)
        *parameter_on_line_to_collision_point = min_param;
    return true;
}

void  find_scene_nodes_on_line(
        scene const&  scene,
        vector3 const&  line_begin,
        vector3 const&  line_end,
        std::map<scalar, std::string>&  result
        )
{
    for (auto const& name_and_node : scene.get_all_scene_nodes())
    {
        scalar param;
        if (compute_collision_of_scene_node_and_line(*name_and_node.second, line_begin, line_end, &param))
            result.insert({param, name_and_node.first});
    }
}
