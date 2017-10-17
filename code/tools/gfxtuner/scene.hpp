#ifndef E2_TOOL_GFXTUNER_SCENE_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_SCENE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <qtgl/batch.hpp>
#   include <utility/assumptions.hpp>
#   include <unordered_map>
#   include <vector>
#   include <string>
#   include <memory>


struct scene_node
{
    using scene_node_ptr = std::shared_ptr<scene_node>;

    scene_node(std::string const&  name)
        : scene_node(name, vector3_zero(), quaternion_identity())
    {}

    scene_node(
        std::string const&  name,
        vector3 const&  origin,
        quaternion const&  orientation
        );

    std::string const&  get_name() const { return m_name; }
    angeo::coordinate_system_const_ptr  get_coord_system() const { return m_coord_system; }

    std::unordered_map<std::string, qtgl::batch_ptr> const&  get_batches() const { return m_batches; }

    std::unordered_map<std::string, scene_node_ptr> const&  get_children() const { return m_children; }
    bool  has_parent() const { return !m_parent.expired(); }
    scene_node_ptr  get_parent() const { return m_parent.lock(); }

    void  rename(std::string const&  new_name);

    void  insert(std::unordered_map<std::string, qtgl::batch_ptr> const&  batches);

    matrix44 const&  get_world_matrix() const;

    void  invalidate_world_matrix();

    static void  insert_children_to_parent(std::vector<scene_node_ptr> const&  children, scene_node_ptr const  parent);

private:

    std::string  m_name;
    angeo::coordinate_system_ptr  m_coord_system;
    std::unordered_map<std::string, qtgl::batch_ptr>  m_batches;
    std::unordered_map<std::string, scene_node_ptr>  m_children;
    std::weak_ptr<scene_node>  m_parent;
    mutable matrix44  m_world_matrix;
    mutable bool  m_is_world_matrix_valid;
};

using scene_node_ptr = scene_node::scene_node_ptr;


inline bool is_direct_parent_and_child(scene_node_ptr const  parent, scene_node_ptr const  child)
{ return child->get_parent() == parent; }

bool is_parent_and_child(scene_node_ptr const  parent, scene_node_ptr const  child);

inline void  insert_children_to_parent(std::vector<scene_node_ptr> const&  children, scene_node_ptr const  parent)
{ scene_node::insert_children_to_parent(children, parent); }


#endif
