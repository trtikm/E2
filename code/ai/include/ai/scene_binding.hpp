#ifndef AI_SCENE_BINDING_HPP_INCLUDED
#   define AI_SCENE_BINDING_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <com/object_guid.hpp>
#   include <vector>
#   include <memory>

namespace com { struct  simulation_context; }

namespace ai {


using  simulation_context_const_ptr = std::shared_ptr<com::simulation_context const>;


struct  skeletal_motion_templates;


struct  scene_binding;
using  scene_binding_ptr = std::shared_ptr<scene_binding const>;


struct  scene_binding
{
    static scene_binding_ptr  create(
            simulation_context_const_ptr  context_,
            com::object_guid  folder_guid_of_agent_,
            skeletal_motion_templates const&  motion_templates
            );
    ~scene_binding();

    simulation_context_const_ptr  context;

    com::object_guid  folder_guid_of_agent;

    com::object_guid  folder_guid_of_motion_object;
    com::object_guid  frame_guid_of_motion_object;

    com::object_guid  folder_guid_of_skeleton;
    com::object_guid  frame_guid_of_skeleton; // always has a parent frame: either motion object or ghost frame.
    std::vector<com::object_guid>  frame_guids_of_bones;

    com::object_guid  folder_guid_of_skeleton_sync;
    com::object_guid  frame_guid_of_skeleton_sync_source;
    com::object_guid  frame_guid_of_skeleton_sync_target;
private:
    scene_binding(
            simulation_context_const_ptr  context_,
            com::object_guid  folder_guid_of_agent_,
            skeletal_motion_templates const&  motion_templates
            );
};


}

#endif
