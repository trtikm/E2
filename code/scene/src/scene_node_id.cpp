#include <scene/scene_node_id.hpp>

namespace scn {


scene_node_id  scene_node_id::get_direct_parent_id() const
{
    path_type  path(m_path);
    path.pop_back();
    return scene_node_id(path);
}

scene_node_id  operator/(scene_node_id const&  parents, scene_node_id const&  children)
{
    scene_node_id::path_type  path(parents.path());
    for (auto const& elem : children.path())
        path.push_back(elem);
    return scene_node_id(path);
}


bool operator==(scene_node_id const&  left, scene_node_id const&  right)
{
    if (left.depth() != right.depth())
        return false;
    for (natural_32_bit i = 0U, n = left.depth(); i != n; ++i)
        if (left.path_element(i) != right.path_element(i))
            return false;
    return true;
}


bool operator<(scene_node_id const&  left, scene_node_id const&  right)
{
    for (natural_32_bit i = 0U, n = std::min(left.depth(), right.depth()); i != n; ++i)
    {
        if (left.path_element(i) < right.path_element(i))
            return true;
        if (left.path_element(i) != right.path_element(i))
            return false;
    }
    return left.depth() < right.depth();
}


}

namespace std {


    size_t  hash<scn::scene_node_id>::operator()(scn::scene_node_id const&  id) const
    {
        size_t  seed = 0;
        for (auto const& elem : id.path())
            ::hash_combine(seed, elem);
        return seed;
    }


}

