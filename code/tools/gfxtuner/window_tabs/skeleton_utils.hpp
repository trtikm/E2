#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SKELETON_UTILS_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SKELETON_UTILS_HPP_INCLUDED

#   include <scene/scene_record_id.hpp>
#   include <scene/scene_node_record_id.hpp>
#   include <scene/records/skeleton_props.hpp>

namespace window_tabs { namespace tab_scene {


struct  widgets;


void  insert_skeleton_joint_nodes(
        scn::scene_record_id const&  rid,
        scn::skeleton_props_const_ptr const  skeleton_props,
        widgets* const  w
        );


void  reset_skeleton_joint_nodes(
        scn::scene_record_id const&  rid,
        scn::skeleton_props_const_ptr const  skeleton_props,
        widgets* const  w
        );


}}

#endif
