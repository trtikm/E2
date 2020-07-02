#include <scene/scene_node_id.hpp>
#include <sstream>

namespace scn {


scene_node_id  scene_node_id::get_direct_parent_id() const
{
    path_type  path(m_path);
    path.pop_back();
    return scene_node_id(path);
}


bool  scene_node_id::is_prefix_of(scene_node_id const&  other) const
{
    if (other.depth() < depth())
        return false;
    for (natural_32_bit  i = 0U; i != depth(); ++i)
        if (path_element(i) != other.path_element(i))
            return false;
    return true;
}


scene_node_id  scene_node_id::copy(natural_32_bit  start_index) const
{
    path_type  path;
    for ( ; start_index < depth(); ++start_index)
        path.push_back(path_element(start_index));
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


natural_32_bit  common_prefix_size(scn::scene_node_id const&  left, scn::scene_node_id const&  right)
{
    natural_32_bit  i = 0U;
    while (i < left.depth() && i < right.depth() && left.path_element(i) == right.path_element(i))
        ++i;
    return i;
}


std::string  join(scn::scene_node_id::path_type const& path, char const  sep)
{
    std::stringstream  sstr;
    std::string  xsep;
    for (auto const&  elem : path)
    {
        sstr << xsep << elem;
        xsep = sep;
    }
    return sstr.str();
}


scn::scene_node_id::path_type&  split(
        scn::scene_node_id::path_type&  output,
        std::string const&  path,
        bool const  ignore_empty_path_elements,
        char const  sep
        )
{
    std::string  buffer;
    for (auto c : path)
        if (c == sep)
        {
            if (!ignore_empty_path_elements || !buffer.empty())
                output.push_back(buffer);
            buffer.clear();
        }
        else
            buffer.push_back(c);
    if (!buffer.empty())
        output.push_back(buffer);
    return output;
}

scn::scene_node_id  make_absolute_node_id(scn::scene_node_id::path_type const&  rel, scn::scene_node_id::path_type const&  base)
{
    if (rel.empty() || base.empty() || (rel.front() != "." && rel.front() != ".."))
        return scn::scene_node_id(rel);
    scn::scene_node_id::path_type  p = base;
    for (auto const&  x : rel)
        if (x == ".")
            continue;
        else if (x == "..")
        {
            ASSUMPTION(!p.empty());
            p.pop_back();
        }
        else
            p.push_back(x);
    return scn::scene_node_id(p);
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

