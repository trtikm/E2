#include <gfxtuner/scene_selection.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>


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
