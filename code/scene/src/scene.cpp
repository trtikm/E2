#include <scene/scene.hpp>
#include <utility/timeprof.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <algorithm>

namespace scn { namespace detail {


static bool is_direct_parent_and_child(scene_node_ptr const  parent, scene_node_ptr const  child)
{
    return child->get_parent() == parent;
}


static bool is_parent_and_child(scene_node_ptr const  parent, scene_node_ptr const  child)
{
    TMPROF_BLOCK();

    for (auto node = child; node->has_parent(); node = node->get_parent())
        if (detail::is_direct_parent_and_child(parent, node))
            return true;
    return false;
}


}}

namespace scn {


scene_node_ptr  scene_node::create(
    scene_node::node_name const&  name,
    vector3 const&  origin,
    quaternion const&  orientation
    )
{
    return scene_node_ptr(new scene_node(name, origin, orientation));
}

scene_node::scene_node(
        scene_node::node_name const&  name,
        vector3 const&  origin,
        quaternion const&  orientation
        )
    : m_name(name)
    , m_parent()
    , m_coord_system(new angeo::coordinate_system(origin, orientation))
    , m_folders()
    , m_children()
    , m_is_world_matrix_valid(false)
{
    ASSUMPTION(!m_name.empty());
}


scene_node_id  scene_node::get_id() const
{
    scene_node_id::path_type  path;
    for (scene_node const*  ptr = this; ptr != nullptr; ptr = ptr->get_parent().get())
        path.push_back(ptr->get_name());
    std::reverse(path.begin(), path.end());
    return scene_node_id(path);
}


bool  scene_node::foreach_child(std::function<bool(scene_node_ptr)> const&  action, bool const  recursively) const
{
    for (auto name_child : get_children())
    {
        if (recursively)
            if (name_child.second->foreach_child(action, recursively) == false)
                return false;
        if (action(name_child.second) == false)
            return false;
    }
    return true;
}


bool  scene_node::foreach_child(
        std::function<bool(scene_node_ptr)> const&  action,
        std::function<bool(scene_node_ptr)> const&  iterate_children_of
        ) const
{
    for (auto name_child : get_children())
    {
        if (iterate_children_of(name_child.second))
            if (name_child.second->foreach_child(action, iterate_children_of) == false)
                return false;
        if (action(name_child.second) == false)
            return false;
    }
    return true;
}


scene_node_ptr  scene_node::find_child(scene_node_id const&  id, scene_node_ptr const  ret_value_on_failure) const
{
    auto const  it = find_child(id.path().front());
    if (it == get_children().cend())
        return ret_value_on_failure;
    scene_node_ptr  node = it->second;
    for (natural_32_bit i = 1U, n = id.depth(); i != n; ++i)
    {
        auto const  child_it = node->find_child(id.path_element(i));
        if (child_it == node->get_children().cend())
            return ret_value_on_failure;
        node = child_it->second;
    }
    return node;
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

void  scene_node::insert_children_to_parent(
    std::vector<scene_node_ptr> const&  children,
    scene_node_ptr const  parent
    )
{
    TMPROF_BLOCK();

    for (auto child : children)
    {
        ASSUMPTION(!detail::is_parent_and_child(child, parent));
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


bool  scene::foreach_node(std::function<bool(scene_node_ptr)> const&  action, bool const  root_nodes_only) const
{
    for (auto const& name_and_node : get_root_nodes())
    {
        if (!root_nodes_only)
            if (name_and_node.second->foreach_child(action, true) == false)
                return false;
        if (action(name_and_node.second) == false)
            return false;
    }
    return true;
}

scene_node_ptr scene::get_scene_node(scene_node_id const&  id) const
{
    if (id.path().empty())
        return nullptr;
    auto const  it = m_scene.find(id.path().front());
    if (it == m_scene.cend())
        return nullptr;
    scene_node_ptr  node = it->second;
    for (natural_32_bit  i = 1U, n = id.depth(); i != n; ++i)
    {
        auto const  child_it = node->find_child(id.path_element(i));
        if (child_it == node->get_children().cend())
            return nullptr;
        node = child_it->second;
    }
    return node;
}

scene_node_ptr  scene::insert_scene_node(
    scene_node_id const&  id,
    vector3 const&  origin,
    quaternion const&  orientation
    )
{
    TMPROF_BLOCK();

    ASSUMPTION(get_scene_node(id) == nullptr);

    scene_node_ptr const  node = scene_node::create(id.path().back(), origin, orientation);
    if (id.depth() == 1U)
        m_scene.insert({ id.path().back(), node });
    else
        insert_children_to_parent({ node }, get_scene_node(id.get_direct_parent_id()));

    return node;
}

void  scene::erase_scene_node(scene_node_id const&  id)
{
    TMPROF_BLOCK();

    auto const  node = get_scene_node(id);
    ASSUMPTION(node != nullptr);
    ASSUMPTION(node->get_children().empty());

    if (node->has_parent())
        erase_children_from_parent({ node }, node->get_parent());
    else
    {
        auto const  it = m_scene.find(node->get_name());
        ASSUMPTION(it != m_scene.end());
        m_scene.erase(it);
    }
}

bool scene::is_direct_parent_and_child(scene_node_ptr const  parent, scene_node_ptr const  child)
{
    ASSUMPTION(get_scene_node(parent->get_id()) == parent);
    ASSUMPTION(get_scene_node(child->get_id()) == child);
    return detail::is_direct_parent_and_child(parent, child);
}

bool scene::is_parent_and_child(scene_node_ptr const  parent, scene_node_ptr const  child)
{
    ASSUMPTION(get_scene_node(parent->get_id()) == parent);
    ASSUMPTION(get_scene_node(child->get_id()) == child);
    return detail::is_parent_and_child(parent, child);
}

void  scene::clear()
{
    m_scene.clear();
}


}
