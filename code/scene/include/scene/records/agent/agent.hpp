#ifndef E2_SCENE_RECORDS_AGENT_AGENT_HPP_INCLUDED
#   define E2_SCENE_RECORDS_AGENT_AGENT_HPP_INCLUDED

#   include <scene/scene_node_id.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/agent_id.hpp>
#   include <ai/agent_kind.hpp>
#   include <boost/filesystem/path.hpp>
#   include <vector>
#   include <memory>

namespace scn {


struct  skeleton_props  final
{
    boost::filesystem::path  skeleton_directory;
    ai::skeletal_motion_templates  skeletal_motion_templates;
};


using  skeleton_props_ptr = std::shared_ptr<skeleton_props>;
using  skeleton_props_const_ptr = std::shared_ptr<skeleton_props const>;


inline skeleton_props_ptr  create_skeleton_props(
        boost::filesystem::path const&  skeleton_dir,
        ai::skeletal_motion_templates const  skeletal_motion_templates
        )
{
    skeleton_props_ptr const  props = std::make_shared<skeleton_props>();
    {
        props->skeleton_directory = skeleton_dir;
        props->skeletal_motion_templates = skeletal_motion_templates;
    }
    return props;
}


struct  agent_props
{
    ai::AGENT_KIND  m_agent_kind;
    skeleton_props_const_ptr  m_skeleton_props;
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
