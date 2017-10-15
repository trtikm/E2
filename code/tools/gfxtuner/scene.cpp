#include <gfxtuner/scene.hpp>
#include <utility/timeprof.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>


scene_node::scene_node(
        std::string const&  name,
        vector3 const&  origin,
        quaternion const&  orientation
        )
    : m_name(name)
    , m_parent()
    , m_coord_system(new angeo::coordinate_system(origin, orientation))
    , m_batches()
    , m_children()
{
    ASSUMPTION(!m_name.empty());
}

void  scene_node::rename(std::string const&  new_name)
{
    ASSUMPTION(!new_name.empty());
    m_name = new_name;
}

void  scene_node::insert(std::vector<qtgl::batch_ptr> const&  batches)
{
    TMPROF_BLOCK();
    for (auto batch : batches)
        m_batches.push_back(batch);
}

matrix44 const&  scene_node::get_world_matrix() const
{
    TMPROF_BLOCK();

    if (!m_is_world_matrix_valid)
    {
        angeo::from_base_matrix(*get_coord_system(), m_world_matrix);
        if (has_parent())
            m_world_matrix = get_parent()->get_world_matrix() * m_world_matrix;
        m_is_world_matrix_valid = true;
    }
    return m_world_matrix;
}

void  scene_node::invalidate_world_matrix()
{
    TMPROF_BLOCK();

    if (m_is_world_matrix_valid)
    {
        m_is_world_matrix_valid = false;
        for (auto  child : get_children())
            child->invalidate_world_matrix();
    }
}

void  scene_node::insert_children_to_parent(
    std::vector<scene_node_ptr> const&  children,
    scene_node_ptr const  parent
    )
{
    TMPROF_BLOCK();

    for (auto child : children)
    {
        ASSUMPTION(!is_parent_and_child(child, parent));
        parent->m_children.push_back(child);
        child->m_parent = parent;
        child->invalidate_world_matrix();
    }
}

bool is_parent_and_child(scene_node_ptr const  parent, scene_node_ptr const  child)
{
    TMPROF_BLOCK();

    if (is_direct_parent_and_child(parent, child))
        return true;
    for (auto child_of_child : child->get_children())
        if (is_parent_and_child(parent, child_of_child))
            return true;
    return false;
}
