#ifndef AI_SKELETAL_MOTION_TEMPLATES_HPP_INCLUDED
#   define AI_SKELETAL_MOTION_TEMPLATES_HPP_INCLUDED

#   include <qtgl/keyframe.hpp>
#   include <unordered_map>
#   include <string>
#   include <memory>

namespace ai {


struct skeletal_motion_templates
{
    using  keyframe = qtgl::keyframe;
    using  keyframes = qtgl::keyframes;
    
    std::unordered_map<std::string, keyframes>  motions_map;
};


using  skeletal_motion_templates_ptr = std::shared_ptr<skeletal_motion_templates>;
using  skeletal_motion_templates_const_ptr = std::shared_ptr<skeletal_motion_templates const>;


}

#endif
