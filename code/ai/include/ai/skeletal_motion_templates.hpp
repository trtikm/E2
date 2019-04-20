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

    struct  template_motion_info
    {
        std::vector<angeo::coordinate_system>  current_frames_in_world_space;
        std::vector<angeo::coordinate_system>  dst_frames_in_world_space;
        float_32_bit  time_to_reach_dst_frames_in_seconds;
    };

    struct  template_cursor
    {
        angeo::coordinate_system  frame_in_world_space;
        std::string  name;
        natural_32_bit  keyframe_index;
        bool  repeat;
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

namespace ai {


void  update_motion_using_templates(
        float_32_bit  time_step_in_seconds,
        skeletal_motion_templates const&  smt,
        skeletal_motion_templates::template_motion_info&  motion_info,
        skeletal_motion_templates::template_cursor&  cursor
        );


}

#endif
