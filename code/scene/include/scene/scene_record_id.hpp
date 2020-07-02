#ifndef E2_SCENE_SCENE_RECORD_ID_HPP_INCLUDED
#   define E2_SCENE_SCENE_RECORD_ID_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <scene/scene_node_id.hpp>
#   include <utility/hash_combine.hpp>
#   include <utility/assumptions.hpp>
#   include <string>

namespace scn {


struct  scene_record_id
{
    scene_record_id()
        : m_node_id()
        , m_folder_name()
        , m_record_name()
    {}

    scene_record_id(
            scene_node_id const&  node_id,
            scene_node::folder_name const&  folder_name,
            scene_node::record_name const&  record_name
            )
        : m_node_id(node_id)
        , m_folder_name(folder_name)
        , m_record_name(record_name)
    {}

    scene_record_id(scene_node_id const&  node_id, scene_node::folder_name const&  folder_name)
        : m_node_id(node_id)
        , m_folder_name(folder_name)
        , m_record_name()
    {}

    explicit scene_record_id(scene_node_id const&  node_id)
        : m_node_id(node_id)
        , m_folder_name()
        , m_record_name()
    {}

    // ASSUMPTION(path.size() >= 2U);
    explicit scene_record_id(scene_node_id::path_type const&  path)
        : m_node_id(path.cbegin(), std::next(path.cbegin(), path.size() - 2UL))
        , m_folder_name(path.at(path.size() - 2UL))
        , m_record_name(path.at(path.size() - 1UL))
    {}

    bool  valid() const { return get_node_id().valid(); }
    scene_node_id const&  get_node_id() const { return m_node_id; }
    scene_node::folder_name const&  get_folder_name() const { return m_folder_name; }
    scene_node::record_name const&  get_record_name() const { return m_record_name; }

    bool  is_node_reference() const { return get_folder_name().empty() && get_record_name().empty(); }
    bool  is_folder_reference() const { return get_record_name().empty(); }

private:
    scene_node_id  m_node_id;
    scene_node::folder_name  m_folder_name;
    scene_node::record_name  m_record_name;
};


inline bool operator==(scene_record_id const&  left, scene_record_id const&  right)
{
    return left.get_node_id() == right.get_node_id() &&
           left.get_folder_name() == right.get_folder_name() &&
           left.get_record_name() == right.get_record_name()
           ;
}


inline bool operator!=(scene_record_id const&  left, scene_record_id const&  right)
{
    return !(left == right);
}


inline bool operator<(scene_record_id const&  left, scene_record_id const&  right)
{
    if (left.get_node_id() < right.get_node_id())
        return true;
    if (left.get_node_id() != right.get_node_id())
        return false;

    if (left.get_folder_name() < right.get_folder_name())
        return true;
    if (left.get_folder_name() != right.get_folder_name())
        return false;

    return left.get_record_name() < right.get_record_name();
}


inline bool operator>(scene_record_id const&  left, scene_record_id const&  right)
{
    return right < left;
}


inline bool operator<=(scene_record_id const&  left, scene_record_id const&  right)
{
    return left == right || left < right;
}


inline bool operator>=(scene_record_id const&  left, scene_record_id const&  right)
{
    return left == right || right < left;
}


}


inline std::string  as_string(scn::scene_record_id const&  id)
{ return msgstream() << as_string(id.get_node_id()) << (id.get_node_id().depth() == 0U ? "" : "/") << id.get_folder_name() << '/' << id.get_record_name(); }

inline scn::scene_record_id  as_scene_record_id(std::string const&  path, scn::scene_node_id const&  base = scn::scene_node_id())
{
    scn::scene_node_id::path_type  p = as_scene_node_id(path, base).path();
    ASSUMPTION(p.size() > 2UL);
    std::string const  rec_name = p.back();
    p.pop_back();
    std::string const  fol_name = p.back();
    p.pop_back();
    return scn::scene_record_id(scn::scene_node_id(p), fol_name, rec_name);
}


namespace std {


    template<>
    struct hash<scn::scene_record_id>
    {
        size_t operator()(scn::scene_record_id const&  id) const
        {
            size_t  seed = hash<scn::scene_node_id>()(id.get_node_id());
            ::hash_combine(seed, id.get_folder_name());
            ::hash_combine(seed, id.get_record_name());
            return seed;
        }
    };


}

#endif
