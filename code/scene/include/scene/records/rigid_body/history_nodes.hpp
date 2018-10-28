#ifndef E2_SCENE_RECORDS_RIGID_BODY_HISTORY_NODES_HPP_INCLUDED
#   define E2_SCENE_RECORDS_RIGID_BODY_HISTORY_NODES_HPP_INCLUDED

#   include <scene/scene_history_nodes_default.hpp>
#   include <scene/scene_record_id.hpp>
#   include <boost/filesystem/path.hpp>

namespace scn {


struct  scene_history_rigid_body_insert final : public scene_history_record_insert<scene_history_rigid_body_insert>
{
    using super_type = scene_history_record_insert<scene_history_rigid_body_insert>;

    scene_history_rigid_body_insert(
            scene_record_id const&  id,
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'erase'
            )
        : super_type(id, as_inverse_operation)
    {}

private:
};


}

#endif