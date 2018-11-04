#ifndef E2_SCENE_RECORDS_COLLIDER_UTILS_HPP_INCLUDED
#   define E2_SCENE_RECORDS_COLLIDER_UTILS_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <scene/scene_utils.hpp>
#   include <scene/records/collider/collider.hpp>
#   include <angeo/collision_object_id.hpp>

namespace scn {


inline scene_node::folder_name  get_collider_folder_name() { return "collider"; }

inline scene_record_id  make_collider_record_id(scene_node_name const&  node_name, std::string const&  shape_type)
{
    return { node_name, get_collider_folder_name(), shape_type };
}

inline scene_node_record_id  make_collider_node_record_id(std::string const&  shape_type)
{
    return { get_collider_folder_name(), shape_type };
}

inline collider const*  get_collider(scene_node const&  node)
{
    scene_node::folder_content::records_map const&  folder = get_folder_records_map(node, get_collider_folder_name());
    return folder.empty() ? nullptr : &record_cast<collider>(folder.cbegin()->second);
}

inline collider*  get_collider(scene_node&  node)
{
    scene_node::folder_content::records_map const&  folder = get_folder_records_map(node, get_collider_folder_name());
    return folder.empty() ? nullptr : const_cast<collider*>(&record_cast<collider>(folder.cbegin()->second));
}

inline bool  has_collider(scene_node const&  node)
{
    return get_collider(node) != nullptr;
}

inline scene_node::record_name  get_collider_record_name(scene_node const&  node)
{
    return get_folder_records_map(node, get_collider_folder_name()).cbegin()->first;
}

inline void  insert_collider(
        scene_node&  n,
        std::string const&  shape_type,
        angeo::collision_object_id const  collider_id,
        float_32_bit const  density_multiplier = 1.0f
        )
{
    insert_record<collider>(n, make_collider_node_record_id(shape_type), collider(collider_id, density_multiplier));
}

inline void  insert_collider(
        scene&  s,
        scene_node_name const&  node_name,
        std::string const&  shape_type,
        angeo::collision_object_id const  collider_id,
        float_32_bit const  density_multiplier = 1.0f
        )
{
    insert_record<collider>(s, make_collider_record_id(node_name, shape_type), collider(collider_id, density_multiplier));
}

inline void  erase_collider(scene_node&  n)
{
    erase_folder(n, get_collider_folder_name());
}


}

#endif
