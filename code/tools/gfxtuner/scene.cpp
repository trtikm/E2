#include <gfxtuner/scene.hpp>
#include <utility/timeprof.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>


scene_node_ptr  scene_node::create(std::string const&  name)
{
    return scene_node_ptr(new scene_node(name));
}

scene_node_ptr  scene_node::create(
    std::string const&  name,
    vector3 const&  origin,
    quaternion const&  orientation
    )
{
    return scene_node_ptr(new scene_node(name, origin, orientation));
}

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
    , m_is_world_matrix_valid(false)
{
    ASSUMPTION(!m_name.empty());
}

void  scene_node::relocate_coordinate_system(std::function<void(angeo::coordinate_system&)> const&  relocator)
{
    TMPROF_BLOCK();

    relocator(*m_coord_system);
    invalidate_world_matrix();
}

void  scene_node::insert_batches(std::unordered_map<std::string, qtgl::batch_ptr> const&  batches)
{
    TMPROF_BLOCK();

    for (auto name_batch : batches)
        m_batches.insert(name_batch);
}

void  scene_node::erase_batches(std::unordered_set<std::string> const&  names_of_batches)
{
    TMPROF_BLOCK();

    for (auto const& name : names_of_batches)
    {
        auto const  it = m_batches.find(name);
        ASSUMPTION(it != m_batches.end());
        m_batches.erase(it);
    }
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
        for (auto  name_child : get_children())
            name_child.second->invalidate_world_matrix();
    }
}

bool is_parent_and_child(scene_node_ptr const  parent, scene_node_ptr const  child)
{
    TMPROF_BLOCK();

    for (auto node = child; node->has_parent(); node = node->get_parent())
        if (is_direct_parent_and_child(parent, node))
            return true;
    return false;
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
        parent->m_children.insert({child->get_name(), child});
        child->m_parent = parent;
        child->invalidate_world_matrix();
    }
}

void  scene_node::erase_children_from_parent(std::vector<scene_node_ptr> const&  children, scene_node_ptr const  parent)
{
    TMPROF_BLOCK();

    ASSUMPTION(parent != nullptr);

    for (auto const child : children)
    {
        ASSUMPTION(child->get_parent() == parent);
        auto const  it = parent->m_children.find(child->get_name());
        ASSUMPTION(it != parent->m_children.end());
        parent->m_children.erase(it);
        child->m_parent.reset();
        child->invalidate_world_matrix();
    }
}
