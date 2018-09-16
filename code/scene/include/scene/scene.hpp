#ifndef E2_SCENE_SCENE_HPP_INCLUDED
#   define E2_SCENE_SCENE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <qtgl/batch.hpp>
#   include <utility/assumptions.hpp>
#   include <unordered_map>
#   include <unordered_set>
#   include <vector>
#   include <string>
#   include <functional>
#   include <memory>

namespace scn {


struct scene;


struct scene_node
{
    using scene_node_ptr = std::shared_ptr<scene_node>;

    static scene_node_ptr  create(
        std::string const&  name,
        vector3 const&  origin = vector3_zero(),
        quaternion const&  orientation = quaternion_identity()
        );

    scene_node(
        std::string const&  name,
        vector3 const&  origin,
        quaternion const&  orientation
        );

    std::string const&  get_name() const { return m_name; }

    std::unordered_map<std::string, scene_node_ptr> const&  get_children() const { return m_children; }
    bool  has_child(std::string const&  name) const { return get_children().count(name) != 0U; }
    bool  has_parent() const { return !m_parent.expired(); }
    scene_node_ptr  get_parent() const { return m_parent.lock(); }

    std::unordered_map<std::string, qtgl::batch> const&  get_batches() const { return m_batches; }
    bool  has_batch(std::string const&  name) const { return get_batches().count(name) != 0U; }
    qtgl::batch  get_batch(std::string const&  name) const;

    void  insert_batches(std::unordered_map<std::string, qtgl::batch> const&  batches);
    void  erase_batches(std::unordered_set<std::string> const&  names_of_batches);

    angeo::coordinate_system_const_ptr  get_coord_system() const { return m_coord_system; }
    void  translate(vector3 const&  shift) { angeo::translate(*m_coord_system, shift); invalidate_world_matrix(); }
    void  rotate(quaternion const&  rotation) { angeo::rotate(*m_coord_system, rotation); invalidate_world_matrix(); }
    void  set_origin(vector3 const&  new_origin) { m_coord_system->set_origin(new_origin); invalidate_world_matrix(); }
    void  set_orientation(quaternion const&  new_orientation) { m_coord_system->set_orientation(new_orientation); invalidate_world_matrix(); }
    void  relocate(vector3 const&  new_origin, quaternion const&  new_orientation)
    { m_coord_system->set_origin(new_origin); m_coord_system->set_orientation(new_orientation); invalidate_world_matrix(); }

    matrix44 const&  get_world_matrix() const;

    void  invalidate_world_matrix();

private:

    static void  insert_children_to_parent(std::vector<scene_node_ptr> const&  children, scene_node_ptr const  parent);
    static void  erase_children_from_parent(std::vector<scene_node_ptr> const&  children, scene_node_ptr const  parent);

    std::string  m_name;
    angeo::coordinate_system_ptr  m_coord_system;
    std::unordered_map<std::string, qtgl::batch>  m_batches;
    std::unordered_map<std::string, scene_node_ptr>  m_children;
    std::weak_ptr<scene_node>  m_parent;
    mutable matrix44  m_world_matrix;
    mutable bool  m_is_world_matrix_valid;

    friend struct scene;
};

using scene_node_ptr = scene_node::scene_node_ptr;
using scene_node_const_ptr = std::shared_ptr<scene_node const>;


struct scene
{
    std::unordered_map<std::string, scene_node_ptr> const&  get_root_nodes() const { return m_scene; }
    std::unordered_map<std::string, scene_node_ptr> const&  get_all_scene_nodes() const { return m_names_to_nodes; }

    bool  has_scene_node(std::string const&  name) const { return m_names_to_nodes.count(name) != 0U; }
    scene_node_ptr  get_scene_node(std::string const&  name) const;

    scene_node_ptr  insert_scene_node(
        std::string const&  name,
        vector3 const&  origin = vector3_zero(),
        quaternion const&  orientation = quaternion_identity(),
        scene_node_ptr const  parent = nullptr
        );

    void  erase_scene_node(std::string const&  name);

    void  insert_children_to_parent(std::vector<scene_node_ptr> const&  children, scene_node_ptr const  parent)
    {
        ASSUMPTION(get_scene_node(parent->get_name()) == parent);
        scene_node::insert_children_to_parent(children, parent);
    }

    void  erase_children_from_parent(std::vector<scene_node_ptr> const&  children, scene_node_ptr const  parent)
    {
        ASSUMPTION(get_scene_node(parent->get_name()) == parent);
        scene_node::erase_children_from_parent(children, parent);
    }

    bool is_direct_parent_and_child(scene_node_ptr const  parent, scene_node_ptr const  child);
    bool is_parent_and_child(scene_node_ptr const  parent, scene_node_ptr const  child);

    void  clear();

private:
    std::unordered_map<std::string, scene_node_ptr>  m_scene;
    std::unordered_map<std::string, scene_node_ptr>  m_names_to_nodes;
};

using scene_ptr = std::shared_ptr<scene>;
using scene_const_ptr = std::shared_ptr<scene const>;


}

#endif
