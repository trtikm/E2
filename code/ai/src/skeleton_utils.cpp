#include <ai/skeleton_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


void  transform_keyframes_to_reference_frame(
        std::vector<angeo::coordinate_system> const&  frames,
        std::unordered_map<natural_32_bit, natural_32_bit> const&  bones_to_indices,
        angeo::coordinate_system const&  reference_frame,
        std::vector<angeo::coordinate_system> const&  pose_frames,
        std::vector<integer_32_bit> const&  parents,
        std::vector<angeo::coordinate_system>&  output_frames,
        bool const  initialise_not_animated_frames_from_pose
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(frames.size() == pose_frames.size());

    output_frames.clear();
    output_frames.reserve(pose_frames.size());
    matrix44  Ainv;
    angeo::to_base_matrix(reference_frame, Ainv);
    for (natural_32_bit bone = 0; bone != pose_frames.size(); ++bone)
    {
        auto const  it = bones_to_indices.find(bone);
        if (it == bones_to_indices.end())
        {
            if (initialise_not_animated_frames_from_pose)
                output_frames.push_back(pose_frames.at(bone));
        }
        else
        {
            auto const& frame = frames.at(it->second);
            output_frames.push_back({ frame.origin() + pose_frames.at(bone).origin(), frame.orientation() });
        }
        if (parents.at(bone) < 0)
        {
            vector3  u;
            matrix33  R;
            {
                matrix44  N;
                angeo::from_base_matrix(output_frames.back(), N);
                decompose_matrix44(Ainv * N, u, R);
            }
            output_frames.back() = { u, normalised(rotation_matrix_to_quaternion(R)) };
        }
    }
}


void  interpolate_keyframes_spherical(
        std::vector<angeo::coordinate_system> const&  src_frames,
        std::vector<angeo::coordinate_system> const&  dst_frames,
        float_32_bit const  interpolation_param, // in range <0,1>
        std::vector<angeo::coordinate_system>&  output
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(src_frames.size() == dst_frames.size());
    output.resize(src_frames.size());
    for (std::size_t i = 0UL; i != src_frames.size(); ++i)
        angeo::interpolate_spherical(src_frames.at(i), dst_frames.at(i), interpolation_param, output.at(i));
}


std::pair<natural_32_bit, float_32_bit>  get_motion_template_transition_props(
        skeletal_motion_templates::transitions_map const&  transition_props,
        skeletal_motion_templates::motion_template_cursor const  src_template,
        std::string const&  dst_template_name,
        std::pair<natural_32_bit, float_32_bit> const&  default_props
        )
{
    std::pair<std::string, std::string>  key = { src_template.motion_name, dst_template_name };
    std::array<natural_32_bit, 2U>  indices;
    auto  it = transition_props.find(key);
    if (it == transition_props.cend())
    {
        key = { dst_template_name, src_template.motion_name };
        it = transition_props.find(key);
        if (it == transition_props.cend() || !it->second.is_bidirectional || it->second.transitions.empty() )
            return default_props;
        indices = { 1U, 0U };
    }
    else if (it->second.transitions.empty())
        return default_props;
    else
        indices = { 0U, 1U };

    auto const  get_distance = [](natural_32_bit const  a, natural_32_bit const  b) -> natural_32_bit {
        return (natural_32_bit)std::abs(((integer_32_bit)a) - ((integer_32_bit)b));
    };

    std::vector<skeletal_motion_templates::transition_info> const&  transitions = it->second.transitions;

    natural_32_bit  smallest_distance =
            get_distance(src_template.keyframe_index, transitions.front().keyframe_indices.at(indices.front()));
    natural_32_bit  best_index = 0U;
    for (natural_32_bit  i = 1U; i < transitions.size(); ++i)
    {
        natural_32_bit const  distance =
                get_distance(src_template.keyframe_index, transitions.at(i).keyframe_indices.at(indices.front()));
        if (distance < smallest_distance)
        {
            smallest_distance = distance;
            best_index = i;
        }
    }

    return { transitions.at(best_index).keyframe_indices.at(indices.back()), transitions.at(best_index).seconds_to_interpolate };
}


}
