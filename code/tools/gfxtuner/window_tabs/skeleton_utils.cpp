#include <gfxtuner/window_tabs/skeleton_utils.hpp>
#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/program_window.hpp>
#include <scene/scene.hpp>
#include <aiold/skeleton_utils.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>

namespace window_tabs { namespace tab_scene {


void  insert_skeleton_joint_nodes(
        scn::scene_record_id const&  rid,
        scn::skeleton_props_const_ptr const  skeleton_props,
        widgets* const  w
        )
{
    std::vector<std::pair<scn::scene_node_id, tree_widget_item*> >  inserted_nodes{
            { rid.get_node_id(), w->scene_tree()->find(rid.get_node_id()) }
            };
    for (natural_32_bit i = 0U; i != skeleton_props->skeletal_motion_templates.pose_frames().size(); ++i)
    {
        scn::scene_node_id const  bone_node_id =
                inserted_nodes.at(skeleton_props->skeletal_motion_templates.parents().at(i) + 1).first
                / skeleton_props->skeletal_motion_templates.names().at(i);
        if (w->wnd()->glwindow().call_now(&simulator::get_scene_node, std::cref(bone_node_id)))
        {
            tree_widget_item*  const  widget = w->scene_tree()->find(bone_node_id);
            ASSUMPTION(widget != nullptr);
            inserted_nodes.push_back({ bone_node_id, widget });
        }
        else
        {
            w->wnd()->glwindow().call_now(
                    &simulator::insert_scene_node_at,
                    bone_node_id,
                    skeleton_props->skeletal_motion_templates.pose_frames().at(i).origin(),
                    skeleton_props->skeletal_motion_templates.pose_frames().at(i).orientation()
                    );
            auto const  bone_tree_item =
                    insert_coord_system_to_tree_widget(
                            w->scene_tree(),
                            bone_node_id,
                            skeleton_props->skeletal_motion_templates.pose_frames().at(i).origin(),
                            skeleton_props->skeletal_motion_templates.pose_frames().at(i).orientation(),
                            w->get_node_icon(),
                            inserted_nodes.at(skeleton_props->skeletal_motion_templates.parents().at(i) + 1).second
                            );
            w->get_scene_history()->insert<scn::scene_history_coord_system_insert>(
                    bone_node_id,
                    skeleton_props->skeletal_motion_templates.pose_frames().at(i).origin(),
                    skeleton_props->skeletal_motion_templates.pose_frames().at(i).orientation(),
                    false
                    );
            inserted_nodes.push_back({ bone_node_id, bone_tree_item });
        }
    }
}


void  reset_skeleton_joint_nodes(
        scn::scene_record_id const&  rid,
        scn::skeleton_props_const_ptr const  skeleton_props,
        widgets* const  w
        )
{
    std::vector<scn::scene_node_id>  processed_nodes{ rid.get_node_id() };
    for (natural_32_bit i = 0U; i != skeleton_props->skeletal_motion_templates.pose_frames().size(); ++i)
    {
        angeo::coordinate_system const&  bone_pose_coord_system = skeleton_props->skeletal_motion_templates.pose_frames().at(i);
        scn::scene_node_id const  bone_node_id =
            processed_nodes.at(skeleton_props->skeletal_motion_templates.parents().at(i) + 1)
            / skeleton_props->skeletal_motion_templates.names().at(i);
        scn::scene_node_ptr const  bone_node_ptr = w->wnd()->glwindow().call_now(&simulator::get_scene_node, std::cref(bone_node_id));
        w->get_scene_history()->insert<scn::scene_history_coord_system_relocate>(
                bone_node_ptr->get_id(),
                bone_node_ptr->get_coord_system()->origin(),
                bone_node_ptr->get_coord_system()->orientation(),
                bone_pose_coord_system.origin(),
                bone_pose_coord_system.orientation()
                );
        bone_node_ptr->relocate(bone_pose_coord_system.origin(), bone_pose_coord_system.orientation());
        processed_nodes.push_back(bone_node_id);
    }
}


}}
