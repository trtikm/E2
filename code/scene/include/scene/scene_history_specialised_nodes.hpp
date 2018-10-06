#ifndef E2_SCENE_SCENE_HISTORY_SPECIALISED_NODES_HPP_INCLUDED
#   define E2_SCENE_SCENE_HISTORY_SPECIALISED_NODES_HPP_INCLUDED

#   include <scene/scene_history_default_nodes.hpp>
#   include <scene/scene_record_id.hpp>
#   include <boost/filesystem/path.hpp>

namespace scn {


struct  scene_history_batch_insert final : public scene_history_record_insert<scene_history_batch_insert>
{
    using super_type = scene_history_record_insert<scene_history_batch_insert>;

    scene_history_batch_insert(
            scene_record_id const&  id,
            boost::filesystem::path const&  batch_pathname,
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'erase'
            )
        : super_type(id, as_inverse_operation)
        , m_batch_pathname(batch_pathname)
    {}

    boost::filesystem::path const&  get_batch_pathname() const { return m_batch_pathname; }

private:
    boost::filesystem::path  m_batch_pathname;
};


}

#endif
