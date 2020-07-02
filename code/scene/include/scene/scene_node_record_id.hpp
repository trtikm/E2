#ifndef E2_SCENE_SCENE_NODE_RECORD_ID_HPP_INCLUDED
#   define E2_SCENE_SCENE_NODE_RECORD_ID_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <utility/hash_combine.hpp>
#   include <utility/msgstream.hpp>
#   include <string>

namespace scn {


struct  scene_node_record_id
{
    scene_node_record_id(
            scene_node::folder_name const&  folder_name,
            scene_node::record_name const&  record_name
            )
        : m_folder_name(folder_name)
        , m_record_name(record_name)
    {
        ASSUMPTION(!m_folder_name.empty() && !m_record_name.empty());
    }

    scene_node::folder_name const&  get_folder_name() const { return m_folder_name; }
    scene_node::record_name const&  get_record_name() const { return m_record_name; }

private:
    scene_node::folder_name  m_folder_name;
    scene_node::record_name  m_record_name;
};


inline bool operator==(scene_node_record_id const&  left, scene_node_record_id const&  right)
{
    return left.get_folder_name() == right.get_folder_name() && left.get_record_name() == right.get_record_name();
}


inline bool operator!=(scene_node_record_id const&  left, scene_node_record_id const&  right)
{
    return !(left == right);
}


inline bool operator<(scene_node_record_id const&  left, scene_node_record_id const&  right)
{
    if (left.get_folder_name() < right.get_folder_name())
        return true;
    if (left.get_folder_name() != right.get_folder_name())
        return false;

    return left.get_record_name() < right.get_record_name();
}


inline bool operator>(scene_node_record_id const&  left, scene_node_record_id const&  right)
{
    return right < left;
}


inline bool operator<=(scene_node_record_id const&  left, scene_node_record_id const&  right)
{
    return left == right || left < right;
}


inline bool operator>=(scene_node_record_id const&  left, scene_node_record_id const&  right)
{
    return left == right || right < left;
}


}


inline std::string  as_string(scn::scene_node_record_id const& id)
{ return msgstream() << id.get_folder_name() << '/' << id.get_record_name(); }

inline scn::scene_node_record_id  as_scene_node_record_id(std::string const&  path, scn::scene_node_id const&  base = scn::scene_node_id())
{ scn::scene_node_id::path_type p; split(p, path); ASSUMPTION(p.size() == 2UL); return {p.front(), p.back()}; }


namespace std {


    template<>
    struct hash<scn::scene_node_record_id>
    {
        size_t operator()(scn::scene_node_record_id const&  id) const
        {
            size_t  seed = 0;
            ::hash_combine(seed, id.get_folder_name());
            ::hash_combine(seed, id.get_record_name());
            return seed;
        }
    };


}

#endif
