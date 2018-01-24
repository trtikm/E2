#include <gfxtuner/scene_selection.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>


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

    lo = vector3{ 1e20f,  1e20f,  1e20f };
    hi = vector3{ -1e20f, -1e20f, -1e20f };

    auto const  update_lo_hi = [scene, &lo, &hi](std::string const& node_name) {
        vector3 const  node_wold_pos = transform_point(vector3_zero(), scene->get_scene_node(node_name)->get_world_matrix());
        for (int i = 0; i != 3; ++i)
        {
            if (lo(i) > node_wold_pos(i))
                lo(i) = node_wold_pos(i);
            if (hi(i) < node_wold_pos(i))
                hi(i) = node_wold_pos(i);
        }
    };

    for (auto const& node_name : selection.get_nodes())
        update_lo_hi(node_name);
    for (auto const& node_batch_names : selection.get_batches())
        update_lo_hi(node_batch_names.first);

    return true;
}


void  get_nodes_of_selected_batches(scene_selection const&  selection, std::unordered_set<std::string>&  nodes)
{
    for (auto const&  node_batch : selection.get_batches())
        nodes.insert(node_batch.first);
}
