#ifndef E2_SCENE_RECORDS_AGENT_AGENT_HPP_INCLUDED
#   define E2_SCENE_RECORDS_AGENT_AGENT_HPP_INCLUDED

#   include <scene/scene_node_id.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/skeleton_composition.hpp>
#   include <ai/agent_id.hpp>
#   include <boost/filesystem/path.hpp>
#   include <vector>
#   include <memory>

namespace scn {


struct  skeleton_props  final
{
    boost::filesystem::path  skeleton_directory;
    ai::skeleton_composition_const_ptr  skeleton_composition;
    ai::skeletal_motion_templates_const_ptr  skeletal_motion_templates;
};


using  skeleton_props_ptr = std::shared_ptr<skeleton_props>;
using  skeleton_props_const_ptr = std::shared_ptr<skeleton_props const>;


inline skeleton_props_ptr  create_skeleton_props(
        boost::filesystem::path const&  skeleton_dir,
        ai::skeleton_composition_ptr const  skeleton_composition,
        ai::skeletal_motion_templates_ptr const  skeletal_motion_templates
        )
{
    skeleton_props_ptr const  props = std::make_shared<skeleton_props>();
    {
        props->skeleton_directory = skeleton_dir;
        props->skeleton_composition = skeleton_composition;
        props->skeletal_motion_templates = skeletal_motion_templates;
    }
    return props;
}


struct  agent  final
{
    agent(ai::agent_id const  id, skeleton_props_const_ptr const  props)
        : m_id(id)
        , m_skeleton_props(props)
    {}

    ai::agent_id  id() const { return m_id; }
    skeleton_props_const_ptr  get_skeleton_props() const { return m_skeleton_props; }

private:
    ai::agent_id  m_id;
    skeleton_props_const_ptr  m_skeleton_props;
};


}

#endif
