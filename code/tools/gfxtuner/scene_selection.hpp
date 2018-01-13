#ifndef E2_TOOL_GFXTUNER_SCENE_SELECTION_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_SCENE_SELECTION_HPP_INCLUDED

#   include <gfxtuner/scene.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <unordered_set>
#   include <string>


struct scene_selection
{
    explicit scene_selection(scene_ptr const  scene)
        : m_scene(scene)
    {}

    std::unordered_set<std::string> const&  get_nodes() const { return m_nodes; }
    std::unordered_set<std::pair<std::string, std::string> > const&  get_batches() const { return m_batches; }

    bool  empty() const { return empty_nodes() && empty_batches(); }
    bool  empty_nodes() const { return m_nodes.empty(); }
    bool  empty_batches() const { return m_batches.empty(); }

    bool  is_node_selected(std::string const&  name) const { return m_nodes.count(name) != 0UL; }
    bool  is_batch_selected(std::pair<std::string, std::string> const&  name) const { return m_batches.count(name) != 0UL; }

    void  insert_node(std::string const&  name);
    void  insert_batch(std::pair<std::string, std::string> const&  name);

    void  erase_node(std::string const&  name);
    void  erase_batch(std::pair<std::string, std::string> const&  name);
    void  erase_batches_of_node(std::string const&  name);

    void  clear() { clear_nodes();  clear_batches(); }
    void  clear_nodes() { m_nodes.clear(); }
    void  clear_batches() { m_batches.clear(); }

private:
    std::unordered_set<std::string>  m_nodes;
    std::unordered_set<std::pair<std::string, std::string> >  m_batches;
    scene_ptr  m_scene;
};

#endif
