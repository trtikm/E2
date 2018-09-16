#ifndef E2_SCENE_SCENE_UTILS_HPP_INCLUDED
#   define E2_SCENE_SCENE_UTILS_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <angeo/tensor_math.hpp>
#   include <string>
#   include <map>
#   include <unordered_set>

namespace scn {


quaternion  transform_orientation(matrix44 const  transformation, quaternion const&  orientation);

void  transform_origin_and_orientation(
        matrix44 const  transformation,
        vector3&  origin,
        quaternion&  orientation
        );

inline void  transform_origin_and_orientation_from_world_to_scene_node(
        scene_node_ptr const  node,
        vector3&  origin,
        quaternion&  orientation
        )
{
    transform_origin_and_orientation(inverse44(node->get_world_matrix()), origin, orientation);
}


inline void  transform_origin_and_orientation_from_scene_node_to_world(
        scene_node_ptr const  node,
        vector3&  origin,
        quaternion&  orientation
        )
{
    transform_origin_and_orientation(node->get_world_matrix(), origin, orientation);
}


inline vector3  transform_point_from_scene_node_to_world(
        scene_node const&  node,
        vector3 const&  point
        )
{
    return transform_point(point, node.get_world_matrix());
}


inline vector3  transform_vector_from_scene_node_to_world(
        scene_node const&  node,
        vector3 const&  vector
        )
{
    return transform_vector(vector, node.get_world_matrix());
}


inline vector3  transform_point_from_world_to_scene_node(
        scene_node const&  node,
        vector3 const&  point
        )
{
    return transform_point(point, inverse44(node.get_world_matrix()));
}


inline vector3  transform_vector_from_world_to_scene_node(
        scene_node const&  node,
        vector3 const&  vector
        )
{
    return transform_vector(vector, inverse44(node.get_world_matrix()));
}


inline quaternion  transform_orientation_from_scene_node_to_world(
        scene_node const&  node,
        quaternion const&  orientation
        )
{
    return transform_orientation(node.get_world_matrix(), orientation);
}


inline quaternion  transform_orientation_from_world_to_scene_node(
        scene_node const&  node,
        quaternion const&  orientation
        )
{
    return transform_orientation(inverse44(node.get_world_matrix()), orientation);
}


scalar  compute_bounding_sphere_of_batch_of_scene_node(
        scene_node const&  node,
        std::string const&  batch_name,
        vector3&  centre
        );

scalar  compute_bounding_sphere_radius_of_scene_node(scene_node const&  node);


bool  compute_collision_of_scene_node_and_line(
        scene_node const&  node,
        vector3 const&  line_begin,
        vector3 const&  line_end,
        scalar* const  parameter_on_line_to_collision_point
        );

void  find_scene_nodes_on_line(
        scene const&  scene,
        vector3 const&  line_begin,
        vector3 const&  line_end,
        std::map<scalar, std::string>&  result
        );


void  get_bbox_of_selected_scene_nodes(
        std::unordered_set<scene_node_ptr> const&  nodes,
        vector3&  lo,
        vector3&  hi
        );

void  get_bbox_of_selected_scene_nodes(
        scene const&  scene,
        std::unordered_set<std::string> const&  node_names,
        vector3&  lo,
        vector3&  hi
        );


}

#endif
