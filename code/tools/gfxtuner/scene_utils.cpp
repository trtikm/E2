#include <gfxtuner/scene_utils.hpp>
#include <gfxtuner/scene_selection.hpp>
#include <angeo/collide.hpp>
#include <utility/timeprof.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>


vector3  transform_point(matrix44 const  transformation, vector3 const&  point)
{
    return contract43(transformation * expand34(point));
}

vector3  transform_vector(matrix44 const  transformation, vector3 const&  vector)
{
    return contract43(transformation * expand34(vector, 0.0f));
}

quaternion  transform_orientation(matrix44 const  transformation, quaternion const&  orientation) 
{
    vector3  x, y, z;
    rotation_matrix_to_basis(quaternion_to_rotation_matrix(orientation), x, y, z);
    matrix33 rotation;
    basis_to_rotation_matrix(
        contract43(transformation * expand34(x, 0.0f)),
        contract43(transformation * expand34(y, 0.0f)),
        contract43(transformation * expand34(z, 0.0f)),
        rotation
        );
    return rotation_matrix_to_quaternion(rotation);
}

void  transform_origin_and_orientation(
        matrix44 const  transformation,
        vector3&  origin,
        quaternion&  orientation
        )
{
    origin = transform_point(transformation, origin);
    orientation = transform_orientation(transformation, orientation);
}


scalar  compute_bounding_sphere_of_batch_of_scene_node(
        scene_node const&  node,
        std::string const&  batch_name,
        vector3&  centre
        )
{
    centre = vector3_zero();
    auto const  batch = node.get_batch(batch_name);
    if (batch.empty())
        return 0.0f;
    auto const  binding = batch.get_buffers_binding();
    if (!binding.loaded_successfully())
        return 0.0f;
    auto const&  boundary = binding.get_boundary();
    centre = 0.5f * (boundary.lo_corner() + boundary.hi_corner());
    return std::max(length(boundary.lo_corner() - centre), length(boundary.hi_corner() - centre));
}


scalar  compute_bounding_sphere_radius_of_scene_node(scene_node const&  node)
{
    scalar  max_radius = get_selection_radius_of_bounding_sphere_of_scene_node();
    for (auto const& name_and_batch : node.get_batches())
    {
        auto const  binding = name_and_batch.second.get_buffers_binding();
        if (binding.loaded_successfully())
        {
            qtgl::spatial_boundary const&  boundary = binding.get_boundary();
            if (boundary.radius() > max_radius)
                max_radius = boundary.radius();
        }
    }
    return max_radius;
}


bool  compute_collision_of_scene_node_and_line(
        scene_node const&  node,
        vector3 const&  line_begin,
        vector3 const&  line_end,
        scalar* const  parameter_on_line_to_collision_point
        )
{
    matrix44 const  to_node_space = inverse44(node.get_world_matrix());
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
        if (name_and_batch.second.get_buffers_binding().loaded_successfully())
        {
            qtgl::spatial_boundary const&  boundary = name_and_batch.second.get_buffers_binding().get_boundary();

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


void  get_bbox_of_selected_scene_nodes(
        std::unordered_set<scene_node_ptr> const&  nodes,
        vector3&  lo,
        vector3&  hi
        )
{
    lo = vector3{ 1e20f,  1e20f,  1e20f };
    hi = vector3{ -1e20f, -1e20f, -1e20f };
    for (auto const& node : nodes)
    {
        vector3 const  node_wold_pos = transform_point(vector3_zero(), node->get_world_matrix());
        for (int i = 0; i != 3; ++i)
        {
            if (lo(i) > node_wold_pos(i))
                lo(i) = node_wold_pos(i);
            if (hi(i) < node_wold_pos(i))
                hi(i) = node_wold_pos(i);
        }
    }
}

void  get_bbox_of_selected_scene_nodes(
        scene const&  scene,
        std::unordered_set<std::string> const&  node_names,
        vector3&  lo,
        vector3&  hi
        )
{
    std::unordered_set<scene_node_ptr>  nodes;
    for (auto const& node_name : node_names)
        nodes.insert(scene.get_scene_node(node_name));
    get_bbox_of_selected_scene_nodes(nodes, lo, hi);
}
