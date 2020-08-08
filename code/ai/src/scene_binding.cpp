#include <ai/scene_binding.hpp>
#include <ai/skeletal_motion_templates.hpp>
#include <com/simulation_context.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


scene_binding_ptr  scene_binding::create(
        com::simulation_context const*  context_,
        com::object_guid  folder_guid_of_agent_,
        skeletal_motion_templates const&  motion_templates,
        vector3 const&  skeleton_frame_origin,
        quaternion const&  skeleton_frame_orientation
        )
{
    return std::shared_ptr<scene_binding>(new scene_binding(
            context_,
            folder_guid_of_agent_,
            motion_templates,
            skeleton_frame_origin,
            skeleton_frame_orientation
            ));
}


scene_binding::scene_binding(
        com::simulation_context const*  context_,
        com::object_guid  folder_guid_of_agent_,
        skeletal_motion_templates const&  motion_templates,
        vector3 const&  skeleton_frame_origin,
        quaternion const&  skeleton_frame_orientation
        )
    : context(context_)
    , folder_guid_of_agent(folder_guid_of_agent_)
    , frame_guid_of_skeleton(com::invalid_object_guid())
    , frame_guids_of_bones()
{
    ASSUMPTION(
            context != nullptr &&
            context->is_valid_folder_guid(folder_guid_of_agent) &&
            context->folder_content(folder_guid_of_agent).content.count("FRAME") == 0UL &&
            context->folder_content(folder_guid_of_agent).child_folders.count("skeleton") == 0UL &&
            context->folder_content(folder_guid_of_agent).child_folders.count("roller") == 0UL &&
            motion_templates.loaded_successfully()
            );

    com::object_guid const  skeleton_folder_guid = context->insert_folder(folder_guid_of_agent, "skeleton", false);
    frame_guid_of_skeleton = context->insert_frame(
            skeleton_folder_guid,
            com::invalid_object_guid(),
            skeleton_frame_origin,
            skeleton_frame_orientation
            );

    frame_guids_of_bones.resize(motion_templates.pose_frames().size(), com::invalid_object_guid());
    for (natural_32_bit bone = 0U; bone != frame_guids_of_bones.size(); ++bone)
    {
        com::object_guid  parent_folder_guid, parent_frame_guid;
        {
            integer_32_bit const  parent_bone = motion_templates.parents().at(bone);
            if (parent_bone < 0)
            {
                parent_folder_guid = skeleton_folder_guid;
                parent_frame_guid = frame_guid_of_skeleton;
            }
            else
            {
                parent_folder_guid = context->folder_of_frame(frame_guids_of_bones.at(parent_bone));
                parent_frame_guid = frame_guids_of_bones.at(parent_bone);
            }
        }

        angeo::coordinate_system const&  pose_frame = motion_templates.pose_frames().at(bone);

        com::object_guid const  bone_folder_guid =
                context->insert_folder(parent_folder_guid, motion_templates.names().at(bone), false);
        frame_guids_of_bones.at(bone) = context->insert_frame(
                bone_folder_guid,
                parent_frame_guid,
                pose_frame.origin(),
                pose_frame.orientation()
                );
    }
}


scene_binding::~scene_binding()
{
    if (context->is_valid_frame_guid(frame_guid_of_skeleton))
        context->request_erase_non_root_folder(context->folder_of_frame(frame_guid_of_skeleton));
}


}
