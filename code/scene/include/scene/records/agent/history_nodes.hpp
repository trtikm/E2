#ifndef E2_SCENE_RECORDS_AGENT_HISTORY_NODES_HPP_INCLUDED
#   define E2_SCENE_RECORDS_AGENT_HISTORY_NODES_HPP_INCLUDED

#   include <scene/scene_history_nodes_default.hpp>
#   include <scene/scene_record_id.hpp>
#   include <scene/records/agent/agent.hpp>

namespace scn {


struct  scene_history_agent_insert final : public scene_history_record_insert<scene_history_agent_insert>
{
    using super_type = scene_history_record_insert<scene_history_agent_insert>;

    scene_history_agent_insert(
            scene_record_id const&  id,
            scn::skeleton_props_const_ptr const  skeleton_props_ptr,
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'erase'
            )
        : super_type(id, as_inverse_operation)
        , m_skeleton_props(skeleton_props_ptr)
    {}

    scn::skeleton_props_const_ptr  get_skeleton_props() const { return m_skeleton_props; }

private:
    scn::skeleton_props_const_ptr  m_skeleton_props;
};


struct  scene_history_agent_update_props final : public scene_history_record_update<scene_history_agent_update_props>
{
    using super_type = scene_history_record_update<scene_history_agent_update_props>;

    scene_history_agent_update_props(
            scene_record_id const&  id,
            scn::skeleton_props_const_ptr const  old_skeleton_props_ptr,
            scn::skeleton_props_const_ptr const  new_skeleton_props_ptr,
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'return-to-old-props'
            )
        : super_type(id, as_inverse_operation)
        , m_old_skeleton_props(old_skeleton_props_ptr)
        , m_new_skeleton_props(new_skeleton_props_ptr)
    {}

    scn::skeleton_props_const_ptr  get_old_skeleton_props() const { return m_old_skeleton_props; }
    scn::skeleton_props_const_ptr  get_new_skeleton_props() const { return m_new_skeleton_props; }

private:
    scn::skeleton_props_const_ptr  m_old_skeleton_props;
    scn::skeleton_props_const_ptr  m_new_skeleton_props;
};


}

#endif
