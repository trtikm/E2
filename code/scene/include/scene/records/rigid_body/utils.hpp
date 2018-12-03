#ifndef E2_SCENE_RECORDS_RIGID_BODY_UTILS_HPP_INCLUDED
#   define E2_SCENE_RECORDS_RIGID_BODY_UTILS_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <scene/scene_utils.hpp>
#   include <scene/records/rigid_body/rigid_body.hpp>

namespace scn {


inline scene_node::folder_name  get_rigid_body_folder_name() { return "rigid_body"; }
inline scene_node::record_name  get_rigid_body_record_name() { return "instance"; }

inline scene_record_id  make_rigid_body_record_id(scene_node_id const&  id)
{
    return { id, get_rigid_body_folder_name(), get_rigid_body_record_name() };
}

inline scene_node_record_id  make_rigid_body_node_record_id()
{
    return { get_rigid_body_folder_name(), get_rigid_body_record_name() };
}

inline rigid_body const*  get_rigid_body(scene_node const&  node)
{
    scene_node::folder_content::records_map const&  folder = get_folder_records_map(node, get_rigid_body_folder_name());
    return folder.empty() ? nullptr : &record_cast<rigid_body>(folder.cbegin()->second);
}

inline rigid_body*  get_rigid_body(scene_node&  node)
{
    scene_node::folder_content::records_map const&  folder = get_folder_records_map(node, get_rigid_body_folder_name());
    return folder.empty() ? nullptr : const_cast<rigid_body*>(&record_cast<rigid_body>(folder.cbegin()->second));
}

inline bool  has_rigid_body(scene_node const&  node)
{
    return get_rigid_body(node) != nullptr;
}

inline void  insert_rigid_body(scene_node&  n, angeo::rigid_body_id const  rb_id)
{
    insert_record<rigid_body>(n, make_rigid_body_node_record_id(), rigid_body(rb_id));
}

inline void  insert_rigid_body(scene&  s, scene_node_id const&  node_id, angeo::rigid_body_id const  rb_id)
{
    insert_record<rigid_body>(s, make_rigid_body_record_id(node_id), rigid_body(rb_id));
}

inline void  erase_rigid_body(scene_node&  n)
{
    erase_record(n, make_rigid_body_node_record_id());
}


}

#endif
