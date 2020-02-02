#ifndef E2_SCENE_RECORDS_AGENT_AGENT_HPP_INCLUDED
#   define E2_SCENE_RECORDS_AGENT_AGENT_HPP_INCLUDED

#   include <scene/records/skeleton_props.hpp>
#   include <ai/agent_id.hpp>
#   include <ai/agent_kind.hpp>
#   include <ai/sensor_action.hpp>
#   include <vector>

namespace scn {


struct  agent_props
{
    ai::AGENT_KIND  m_agent_kind;
    skeleton_props_const_ptr  m_skeleton_props;
    ai::from_sensor_node_to_sensor_action_map  m_sensor_action_map;
};


struct  agent  final
{
    agent(ai::agent_id const  id, agent_props const&  props)
        : m_id(id)
        , m_props(props)
    {}

    ai::agent_id  id() const { return m_id; }
    agent_props const&  get_props() const { return m_props; }

private:
    ai::agent_id  m_id;
    agent_props  m_props;
};


}

#endif
