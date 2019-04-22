#ifndef AI_SKELETAL_MOTION_TEMPLATES_HPP_INCLUDED
#   define AI_SKELETAL_MOTION_TEMPLATES_HPP_INCLUDED

#   include <angeo/coordinate_system.hpp>
#   include <angeo/tensor_math.hpp>
#   include <qtgl/keyframe.hpp>
#   include <unordered_map>
#   include <string>
#   include <vector>
#   include <memory>

namespace ai {


struct skeletal_motion_templates
{
    using  keyframe = qtgl::keyframe;
    using  keyframes = qtgl::keyframes;

    struct  motion_template_cursor
    {
        std::string  motion_name;
        natural_32_bit  keyframe_index;
    };

    struct  template_motion_info
    {
        motion_template_cursor  src_pose;
        motion_template_cursor  dst_pose;
        float_32_bit  total_interpolation_time_in_seconds;
        float_32_bit  consumed_time_in_seconds;
    };

    skeletal_motion_templates();
    bool  is_ready() const;
    bool  wait_till_loaded_is_finished() const;

    std::unordered_map<std::string, keyframes>  motions_map;

private:
    mutable bool  is_loaded;
};


using  skeletal_motion_templates_ptr = std::shared_ptr<skeletal_motion_templates>;
using  skeletal_motion_templates_const_ptr = std::shared_ptr<skeletal_motion_templates const>;


}

#endif
