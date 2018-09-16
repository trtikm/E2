#include <scene/scene.hpp>
#include <utility/timeprof.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>

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

qtgl::batch  scene_node::get_batch(std::string const&  name) const
{
    auto const  it = get_batches().find(name);
    return it == get_batches().cend() ? qtgl::batch() : it->second;
}

void  scene_node::insert_batches(std::unordered_map<std::string, qtgl::batch> const&  batches)
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


scene_node_ptr scene::get_scene_node(std::string const&  name) const
{
    auto const  it = m_names_to_nodes.find(name);
    return it == m_names_to_nodes.cend() ? nullptr : it->second;
}

scene_node_ptr  scene::insert_scene_node(
    std::string const&  name,
    vector3 const&  origin,
    quaternion const&  orientation,
    scene_node_ptr const  parent
    )
{
    TMPROF_BLOCK();

    ASSUMPTION(get_scene_node(name) == nullptr);

    scene_node_ptr const  node = scene_node::create(name, origin, orientation);
    if (parent == nullptr)
        m_scene.insert({ name, node });
    else
        insert_children_to_parent({ node }, parent);
    m_names_to_nodes.insert({ name, node });

    return node;
}

void  scene::erase_scene_node(std::string const&  name)
{
    TMPROF_BLOCK();

    auto const  node = get_scene_node(name);
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
    m_names_to_nodes.erase(node->get_name());
}

//void  scene::erase_batch_from_scene_node(std::string const&  batch_name, std::string const&  scene_node_name)
//{
//    auto const  node = get_scene_node(scene_node_name);
//    node->erase_batches({ batch_name });
//    m_names_to_selected_batches.erase({ node->get_name(), batch_name });
//}

bool scene::is_direct_parent_and_child(scene_node_ptr const  parent, scene_node_ptr const  child)
{
    ASSUMPTION(get_scene_node(parent->get_name()) == parent);
    ASSUMPTION(get_scene_node(child->get_name()) == child);
    return detail::is_direct_parent_and_child(parent, child);
}

bool scene::is_parent_and_child(scene_node_ptr const  parent, scene_node_ptr const  child)
{
    ASSUMPTION(get_scene_node(parent->get_name()) == parent);
    ASSUMPTION(get_scene_node(child->get_name()) == child);
    return detail::is_parent_and_child(parent, child);
}

void  scene::clear()
{
    m_scene.clear();
    m_names_to_nodes.clear();
}


}
