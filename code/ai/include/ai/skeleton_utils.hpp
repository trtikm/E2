#ifndef AI_SKELETON_UTILS_HPP_INCLUDED
#   define AI_SKELETON_UTILS_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

namespace ai {


void  transform_keyframes_to_reference_frame(
        std::vector<angeo::coordinate_system> const&  frames,
        angeo::coordinate_system const&  reference_frame,
        std::vector<angeo::coordinate_system> const&  pose_frames,
        std::vector<integer_32_bit> const&  parents,
        std::vector<angeo::coordinate_system>&  output_frames
        );


void  interpolate_keyframes_spherical(
        std::vector<angeo::coordinate_system> const&  src_frames,
        std::vector<angeo::coordinate_system> const&  dst_frames,
        float_32_bit const  interpolation_param, // in range <0,1>
        std::vector<angeo::coordinate_system>&  output
        );


}

#endif
