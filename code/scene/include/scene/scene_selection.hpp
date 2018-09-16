#ifndef E2_SCENE_SCENE_SELECTION_HPP_INCLUDED
#   define E2_SCENE_SCENE_SELECTION_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <unordered_set>
#   include <string>

namespace scn {


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
    void  insert_batches_of_node(std::string const&  name);

    void  erase_node(std::string const&  name);
    void  erase_batch(std::pair<std::string, std::string> const&  name);
    void  erase_batches_of_node(std::string const&  name);

    void  clear() { clear_nodes();  clear_batches(); }
    void  clear_nodes() { m_nodes.clear(); }
    void  clear_batches() { m_batches.clear(); }
    void  clear_batches_of_node(std::string const&  node_name);

private:
    std::unordered_set<std::string>  m_nodes;
    std::unordered_set<std::pair<std::string, std::string> >  m_batches;
    scene_ptr  m_scene;
};


bool  get_bbox_of_selected_scene_nodes(scene_selection const&  selection, scene_ptr const  scene, vector3&  lo, vector3&  hi);
void  get_nodes_of_selected_batches(scene_selection const&  selection, std::unordered_set<std::string>&  nodes);

inline scalar  get_selection_radius_of_bounding_sphere_of_scene_node() { return 0.25f; }


}

#endif
