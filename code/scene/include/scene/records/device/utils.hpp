#ifndef E2_SCENE_RECORDS_DEVICE_UTILS_HPP_INCLUDED
#   define E2_SCENE_RECORDS_DEVICE_UTILS_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <scene/scene_utils.hpp>
#   include <scene/records/device/device.hpp>

namespace scn {


inline scene_node::folder_name  get_device_folder_name() { return "device"; }
inline scene_node::record_name  get_device_record_name() { return "instance"; }

inline scene_record_id  make_device_record_id(scene_node_id const&  id)
{
    return { id, get_device_folder_name(), get_device_record_name() };
}

inline scene_node_record_id  make_device_node_record_id()
{
    return { get_device_folder_name(), get_device_record_name() };
}

inline device const*  get_device(scene_node const&  node)
{
    scene_node::folder_content::records_map const&  folder = get_folder_records_map(node, get_device_folder_name());
    return folder.empty() ? nullptr : &record_cast<device>(folder.cbegin()->second);
}

inline device*  get_device(scene_node&  node)
{
    scene_node::folder_content::records_map const&  folder = get_folder_records_map(node, get_device_folder_name());
    return folder.empty() ? nullptr : const_cast<device*>(&record_cast<device>(folder.cbegin()->second));
}

inline bool  has_device(scene_node const&  node)
{
    return get_device(node) != nullptr;
}

inline void  insert_device(scene_node&  n, aiold::device_id const  device_id, device_props const&  props)
{
    insert_record<device>(n, make_device_node_record_id(), device(device_id, props));
}

inline void  insert_device(scene&  s, scene_node_id const&  node_id, aiold::device_id const  device_id, device_props const&  props)
{
    insert_record<device>(s, make_device_record_id(node_id), device(device_id, props));
}

inline void  erase_device(scene_node&  n)
{
    erase_record(n, make_device_node_record_id());
}


}

#endif
