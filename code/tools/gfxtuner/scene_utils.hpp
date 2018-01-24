#ifndef E2_TOOL_GFXTUNER_SCENE_UTILS_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_SCENE_UTILS_HPP_INCLUDED

#   include <gfxtuner/scene.hpp>
#   include <angeo/tensor_math.hpp>
#   include <string>
#   include <map>


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
    transform_origin_and_orientation(inverse(node->get_world_matrix()), origin, orientation);
}


inline void  transform_origin_and_orientation_from_scene_node_to_world(
        scene_node_ptr const  node,
        vector3&  origin,
        quaternion&  orientation
        )
{
    transform_origin_and_orientation(node->get_world_matrix(), origin, orientation);
}


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


#endif
