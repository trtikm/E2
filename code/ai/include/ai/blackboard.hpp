#ifndef AI_BLACKBOARD_HPP_INCLUDED
#   define AI_BLACKBOARD_HPP_INCLUDED

#   include <ai/environment_models.hpp>
#   include <ai/skeleton_composition.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <memory>

namespace ai {


/// A storage of data shared by all modules of an agent.
/// NOTE: This is NOT an implementation of the 'blackboard' AI approach.
///       Only the idea of a shared storage is used here.
struct blackboard
{
    environment_models_ptr  m_environment_models;
    skeleton_composition_const_ptr  m_skeleton_composition;
    skeletal_motion_templates_const_ptr  m_motion_templates;
};


using  blackboard_ptr = std::shared_ptr<blackboard>;
using  blackboard_const_ptr = std::shared_ptr<blackboard const>;


}

#endif
