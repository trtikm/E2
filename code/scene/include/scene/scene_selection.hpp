#ifndef E2_SCENE_SCENE_SELECTION_HPP_INCLUDED
#   define E2_SCENE_SCENE_SELECTION_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <scene/scene_record_id.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/axis_aligned_bounding_box.hpp>
#   include <unordered_set>

namespace scn {


struct scene_selection
{
    explicit scene_selection(scene_ptr const  scene)
        : m_scene(scene)
    {}

    std::unordered_set<scene_node_id> const&  get_nodes() const { return m_nodes; }
    std::unordered_set<scene_record_id> const&  get_records() const { return m_records; }

    bool  empty() const { return empty_nodes() && empty_records(); }
    bool  empty_nodes() const { return m_nodes.empty(); }
    bool  empty_records() const { return m_records.empty(); }

    bool  is_node_selected(scene_node_id const&  name) const { return m_nodes.count(name) != 0UL; }
    bool  is_record_selected(scene_record_id const&  id) const { return m_records.count(id) != 0UL; }

    void  insert_node(scene_node_id const&  id);
    void  insert_record(scene_record_id const&  id);
    void  insert_records_of_node(scene_node_id const&  id);

    void  erase_node(scene_node_id const&  id);
    void  erase_record(scene_record_id const&  id);
    void  erase_records_of_node(scene_node_id const&  id);

    void  clear() { clear_nodes();  clear_records(); }
    void  clear_nodes() { m_nodes.clear(); }
    void  clear_records() { m_records.clear(); }

    scene_ptr  get_scene_ptr() const { return m_scene; }

private:
    std::unordered_set<scene_node_id>  m_nodes;
    std::unordered_set<scene_record_id>  m_records;
    scene_ptr  m_scene;
};


vector3  get_center_of_selected_scene_nodes(scene_selection const&  selection);

void  get_nodes_of_selected_records(scene_selection const&  selection, std::unordered_set<scene_node_id>&  nodes);


}

#endif
