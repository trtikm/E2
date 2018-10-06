#include <scene/scene_selection.hpp>
#include <scene/scene_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace scn {


void  scene_selection::insert_node(scene_node_name const&  name)
{
    ASSUMPTION(has_node(*m_scene, name));
    m_nodes.insert(name);
}

void  scene_selection::insert_record(scene_record_id const&  id)
{
    ASSUMPTION(has_record(*m_scene, id));
    m_records.insert(id);
}

void  scene_selection::insert_records_of_node(scene_node_name const&  name)
{
    TMPROF_BLOCK();

    scene_node_ptr const  node_ptr = m_scene->get_scene_node(name);
    ASSUMPTION(node_ptr != nullptr);
    for (auto const& name_records : node_ptr->get_folders())
        for (auto const& name_record : name_records.second.get_records())
            m_records.insert({ name, name_records.first, name_record.first });
}


void  scene_selection::erase_node(scene_node_name const&  name)
{
    ASSUMPTION(has_node(*m_scene, name));
    m_nodes.erase(name);
}

void  scene_selection::erase_record(scene_record_id const&  id)
{
    ASSUMPTION(has_record(*m_scene, id));
    m_records.erase(id);
}

void  scene_selection::erase_records_of_node(scene_node_name const&  name)
{
    TMPROF_BLOCK();

    ASSUMPTION(has_node(*m_scene, name));
    std::vector<scene_record_id>  records_to_erase;
    for (auto const& id : m_records)
        if (id.get_node_name() == name)
            records_to_erase.push_back(id);
    for (auto const& id : records_to_erase)
        m_records.erase(id);
}


vector3  get_center_of_selected_scene_nodes(scene_selection const&  selection)
{
    TMPROF_BLOCK();

    ASSUMPTION(!selection.empty());

    std::unordered_set<scene_node_name>  nodes(selection.get_nodes());
    get_nodes_of_selected_records(selection, nodes);

    vector3 center = vector3_zero();
    for (auto const& node_name : nodes)
        center += transform_point(vector3_zero(), selection.get_scene_ptr()->get_scene_node(node_name)->get_world_matrix());

    return center / scalar(nodes.size());
}


void  get_nodes_of_selected_records(scene_selection const&  selection, std::unordered_set<scene_node_name>&  nodes)
{
    for (auto const&  node_records : selection.get_records())
        nodes.insert(node_records.get_node_name());
}


}
