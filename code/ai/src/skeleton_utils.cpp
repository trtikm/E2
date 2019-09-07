#include <ai/skeleton_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


void  transform_keyframes_to_reference_frame(
        std::vector<angeo::coordinate_system> const&  frames,
        angeo::coordinate_system const&  reference_frame,
        std::vector<angeo::coordinate_system> const&  pose_frames,
        std::vector<integer_32_bit> const&  parents,
        std::vector<angeo::coordinate_system>&  output_frames
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
        auto const& frame = frames.at(bone);
        output_frames.push_back({ frame.origin() + pose_frames.at(bone).origin(), frame.orientation() });
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


}
