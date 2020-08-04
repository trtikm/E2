#ifndef AIOLD_SKELETON_UTILS_HPP_INCLUDED
#   define AIOLD_SKELETON_UTILS_HPP_INCLUDED

#   include <aiold/skeletal_motion_templates.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>
#   include <unordered_map>
#   include <utility>

namespace aiold {


void  transform_keyframes_to_reference_frame(
        std::vector<angeo::coordinate_system> const&  frames,
        std::unordered_map<natural_32_bit, natural_32_bit> const&  bones_to_indices,
        angeo::coordinate_system const&  reference_frame,
        std::vector<angeo::coordinate_system> const&  pose_frames,
        std::vector<integer_32_bit> const&  parents,
        std::vector<angeo::coordinate_system>&  output_frames,
        bool const  initialise_not_animated_frames_from_pose = true
        );


void  interpolate_keyframes_spherical(
        std::vector<angeo::coordinate_system> const&  src_frames,
        std::vector<angeo::coordinate_system> const&  dst_frames,
        float_32_bit const  interpolation_param, // in range <0,1>
        std::vector<angeo::coordinate_system>&  output
        );


std::pair<natural_32_bit, float_32_bit>  get_motion_template_transition_props(
        skeletal_motion_templates::transitions_map const&  transition_props,
        skeletal_motion_templates::motion_template_cursor const  src_template,
        std::string const&  dst_template_name,
        std::pair<natural_32_bit, float_32_bit> const&  default_props
        );


}

#endif