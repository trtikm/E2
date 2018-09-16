#include <scene/scene_selection.hpp>
#include <scene/scene_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace scn {


void  scene_selection::insert_node(std::string const&  name)
{
    ASSUMPTION(m_scene->has_scene_node(name));
    m_nodes.insert(name);
}

void  scene_selection::insert_batch(std::pair<std::string, std::string> const&  name)
{
    ASSUMPTION(m_scene->has_scene_node(name.first));
    ASSUMPTION(m_scene->get_scene_node(name.first)->has_batch(name.second));
    m_batches.insert(name);
}

void  scene_selection::insert_batches_of_node(std::string const&  name)
{
    scene_node_ptr const  node_ptr = m_scene->get_scene_node(name);
    ASSUMPTION(node_ptr != nullptr);
    for (auto const& node_batch : node_ptr->get_batches())
        m_batches.insert({ name, node_batch.first });
}


void  scene_selection::erase_node(std::string const&  name)
{
    ASSUMPTION(m_scene->has_scene_node(name));
    m_nodes.erase(name);
}

void  scene_selection::erase_batch(std::pair<std::string, std::string> const&  name)
{
    ASSUMPTION(m_scene->has_scene_node(name.first));
    ASSUMPTION(m_scene->get_scene_node(name.first)->has_batch(name.second));
    m_batches.erase(name);
}

void  scene_selection::erase_batches_of_node(std::string const&  name)
{
    ASSUMPTION(m_scene->has_scene_node(name));
    std::vector<std::pair<std::string, std::string> >  batches_to_erase;
    for (auto const& node_batch : m_batches)
        if (node_batch.first == name)
            batches_to_erase.push_back(node_batch);
    for (auto const& batch : batches_to_erase)
        m_batches.erase(batch);
}


bool  get_bbox_of_selected_scene_nodes(scene_selection const&  selection, scene_ptr const  scene, vector3&  lo, vector3&  hi)
{
    TMPROF_BLOCK();

    if (selection.empty())
        return false;

    std::unordered_set<std::string>  nodes(selection.get_nodes());
    get_nodes_of_selected_batches(selection, nodes);

    if (nodes.empty())
        return false;

    get_bbox_of_selected_scene_nodes(*scene, nodes, lo, hi);

    return true;
}


void  get_nodes_of_selected_batches(scene_selection const&  selection, std::unordered_set<std::string>&  nodes)
{
    for (auto const&  node_batch : selection.get_batches())
        nodes.insert(node_batch.first);
}


}
