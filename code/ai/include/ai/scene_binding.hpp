#ifndef AI_SCENE_BINDING_HPP_INCLUDED
#   define AI_SCENE_BINDING_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <com/object_guid.hpp>
#   include <vector>
#   include <memory>

namespace com { struct  simulation_context; }

namespace ai {


struct  skeletal_motion_templates;


struct  scene_binding
{
    scene_binding(
            com::simulation_context const*  context_,
            com::object_guid  folder_guid_of_agent_,
            skeletal_motion_templates const&  motion_templates,
            vector3 const&  skeleton_frame_origin = vector3_zero(),
            quaternion const&  skeleton_frame_orientation = quaternion_identity()
            );
    com::simulation_context const*  context;
    com::object_guid  folder_guid_of_agent;
    com::object_guid  frame_guid_of_skeleton;
    std::vector<com::object_guid>  frame_guids_of_bones;
};


using  scene_binding_ptr = std::shared_ptr<scene_binding const>;


}

#endif
