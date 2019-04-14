#ifndef E2_SCENE_RECORDS_AGENT_UTILS_HPP_INCLUDED
#   define E2_SCENE_RECORDS_AGENT_UTILS_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <scene/scene_utils.hpp>
#   include <scene/records/agent/agent.hpp>

namespace scn {


inline scene_node::folder_name  get_agent_folder_name() { return "agent"; }
inline scene_node::record_name  get_agent_record_name() { return "instance"; }

inline scene_record_id  make_agent_record_id(scene_node_id const&  id)
{
    return { id, get_agent_folder_name(), get_agent_record_name() };
}

inline scene_node_record_id  make_agent_node_record_id()
{
    return { get_agent_folder_name(), get_agent_record_name() };
}

inline agent const*  get_agent(scene_node const&  node)
{
    scene_node::folder_content::records_map const&  folder = get_folder_records_map(node, get_agent_folder_name());
    return folder.empty() ? nullptr : &record_cast<agent>(folder.cbegin()->second);
}

inline agent*  get_agent(scene_node&  node)
{
    scene_node::folder_content::records_map const&  folder = get_folder_records_map(node, get_agent_folder_name());
    return folder.empty() ? nullptr : const_cast<agent*>(&record_cast<agent>(folder.cbegin()->second));
}

inline bool  has_agent(scene_node const&  node)
{
    return get_agent(node) != nullptr;
}

inline void  insert_agent(scene_node&  n, ai::agent_id const  agent_id, skeleton_props_const_ptr const  props)
{
    insert_record<agent>(n, make_agent_node_record_id(), agent(agent_id, props));
}

inline void  insert_agent(scene&  s, scene_node_id const&  node_id, ai::agent_id const  agent_id, skeleton_props_const_ptr const  props)
{
    insert_record<agent>(s, make_agent_record_id(node_id), agent(agent_id, props));
}

inline void  erase_agent(scene_node&  n)
{
    erase_record(n, make_agent_node_record_id());
}


}

#endif
