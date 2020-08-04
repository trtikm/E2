#ifndef E2_SCENE_RECORDS_AGENT_AGENT_HPP_INCLUDED
#   define E2_SCENE_RECORDS_AGENT_AGENT_HPP_INCLUDED

#   include <scene/records/skeleton_props.hpp>
#   include <aiold/agent_id.hpp>
#   include <aiold/agent_kind.hpp>
#   include <aiold/sensor_action.hpp>
#   include <vector>

namespace scn {


struct  agent_props
{
    aiold::AGENT_KIND  m_agent_kind;
    skeleton_props_const_ptr  m_skeleton_props;
    aiold::from_sensor_record_to_sensor_action_map  m_sensor_action_map;
};


struct  agent  final
{
    agent(aiold::agent_id const  id, agent_props const&  props)
        : m_id(id)
        , m_props(props)
    {}

    aiold::agent_id  id() const { return m_id; }
    agent_props const&  get_props() const { return m_props; }

private:
    aiold::agent_id  m_id;
    agent_props  m_props;
};


}

#endif
