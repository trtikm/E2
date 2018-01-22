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
    for (auto const& name_batch : m_scene->get_scene_node(name)->get_batches())
        erase_batch({ name, name_batch.first });
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
