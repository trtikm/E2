#ifndef E2_SCENE_SCENE_NODE_ID_HPP_INCLUDED
#   define E2_SCENE_SCENE_NODE_ID_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/hash_combine.hpp>
#   include <utility/assumptions.hpp>
#   include <vector>
#   include <string>

namespace scn {


struct  scene_node_id
{
    using  path_element_type = std::string;
    using  path_type = std::vector<path_element_type>;

    scene_node_id()
        : m_path()
    {}

    explicit scene_node_id(path_type const&  path)
        : m_path(path)
    {}

    explicit scene_node_id(path_type::const_iterator const  begin, path_type::const_iterator const  end)
        : m_path(begin, end)
    {}

    explicit scene_node_id(path_element_type const&  element)
        : scene_node_id(path_type{ element })
    {}

    bool  valid() const { return !m_path.empty(); }
    natural_32_bit  depth() const { return (natural_32_bit)m_path.size(); }
    natural_32_bit  is_root() const { return depth() == 1U; }
    path_type const&  path() const { return m_path; }
    path_element_type const&  path_element(natural_32_bit const  index) const { return m_path.at(index); }
    path_element_type const&  path_last_element() const { return m_path.back(); }
    path_element_type const&  path_front_element() const { return m_path.front(); }
    scene_node_id  get_direct_parent_id() const;

    bool  is_prefix_of(scene_node_id const&  other) const;

    scene_node_id  copy(natural_32_bit  start_index = 0U) const;

private:
    path_type  m_path;
};


scene_node_id  operator/(scene_node_id const&  parents, scene_node_id const&  children);


inline scene_node_id  operator/(scene_node_id const&  id, std::string const&  element)
{
    return id / scene_node_id(element);
}


inline scene_node_id  operator/(scene_node_id const&  parents, char const* const  child)
{
    return parents / std::string(child);
}


bool operator==(scene_node_id const&  left, scene_node_id const&  right);


inline bool operator!=(scene_node_id const&  left, scene_node_id const&  right)
{
    return !(left == right);
}


bool operator<(scene_node_id const&  left, scene_node_id const&  right);


inline bool operator>(scene_node_id const&  left, scene_node_id const&  right)
{
    return right < left;
}


inline bool operator<=(scene_node_id const&  left, scene_node_id const&  right)
{
    return left == right || left < right;
}


inline bool operator>=(scene_node_id const&  left, scene_node_id const&  right)
{
    return left == right || right < left;
}


}


natural_32_bit  common_prefix_size(scn::scene_node_id const&  left, scn::scene_node_id const&  right);

std::string  join(scn::scene_node_id::path_type const&  path, char const  sep = '/');
scn::scene_node_id::path_type&  split(
        scn::scene_node_id::path_type&  output,
        std::string const& path,
        bool const  ignore_empty_path_elements = true,
        char const  sep = '/'
        );
scn::scene_node_id  make_absolute_node_id(scn::scene_node_id::path_type const&  rel, scn::scene_node_id::path_type const&  base);

inline std::string  as_string(scn::scene_node_id const&  id)
{  return join(id.path()); }

inline scn::scene_node_id  as_scene_node_id(std::string const&  path, scn::scene_node_id const&  base = scn::scene_node_id())
{ scn::scene_node_id::path_type p; return make_absolute_node_id(split(p, path), base.path()); }


namespace std {


    template<>
    struct hash<scn::scene_node_id>
    {
        size_t  operator()(scn::scene_node_id const&  id) const;
    };


}

#endif
