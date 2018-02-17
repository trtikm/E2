#include <qtgl/keyframe.hpp>

namespace qtgl {


void  keyframes::_update_load_state()
{
    if (m_load_state != detail::ASYNC_LOAD_STATE::IN_PROGRESS)
        return;

    for (std::size_t  i = 0ULL; i != num_keyframes(); ++i)
    {
        if (keyframe_at(i).load_failed())
        {
            m_load_state = detail::ASYNC_LOAD_STATE::FINISHED_WITH_ERROR;
            return;
        }
        if (!keyframe_at(i).loaded_successfully())
            return;
    }

    ASSUMPTION(
        [this]() -> bool {
            for (std::size_t i = 0ULL; i != num_keyframes(); ++i)
                if (keyframe_at(i).get_coord_systems().size() != num_coord_systems_per_keyframe())
                    return false;
            return true;
            }()
        );

    std::sort(
        m_keyframes.begin(),
        m_keyframes.end(),
        [](keyframe const& left, keyframe const& right) -> bool {
            return left.get_time_point() < right.get_time_point();
            }
        );
    m_load_state = detail::ASYNC_LOAD_STATE::FINISHED_SUCCESSFULLY;
}


std::pair<std::size_t, std::size_t>  find_indices_of_keyframes_to_interpolate_for_time(
        keyframes const&  keyframes,
        float_32_bit const  time_point
        )
{
    ASSUMPTION(keyframes.start_time_point() <= time_point && time_point <= keyframes.end_time_point());
    std::size_t  keyframe_index = 0ULL;
    while (keyframe_index + 1ULL < keyframes.num_keyframes() &&
           time_point >= keyframes.time_point_at(keyframe_index + 1ULL))
        ++keyframe_index;
    std::size_t const  keyframe_succ_index = keyframe_index + (keyframes.num_keyframes() < 2ULL ? 0ULL : 1ULL);
    INVARIANT(keyframe_succ_index < keyframes.num_keyframes());
    INVARIANT(time_point >= keyframes.time_point_at(keyframe_index));
    INVARIANT(keyframe_index == keyframe_succ_index || time_point < keyframes.time_point_at(keyframe_succ_index));
    return{ keyframe_index , keyframe_succ_index };
}


float_32_bit  compute_interpolation_parameter(
        float_32_bit const  time_point,
        float_32_bit const  keyframe_start_time_point,
        float_32_bit const  keyframe_end_time_point
        )
{
    ASSUMPTION(keyframe_start_time_point <= time_point && time_point <= keyframe_end_time_point);
    float_32_bit  interpolation_param;
    {
        float_32_bit const  dt = keyframe_end_time_point - keyframe_start_time_point;
        if (dt < 0.0001f)
            interpolation_param = 0.0f;
        else
            interpolation_param = (time_point - keyframe_start_time_point) / dt;
        interpolation_param = std::max(0.0f, std::min(interpolation_param, 1.0f));
    }
    return interpolation_param;
}


void  compute_coord_system_of_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        std::pair<std::size_t, std::size_t> const  indices_of_keyframe_to_interpolate,
        std::size_t const  index_of_coord_system_in_keyframes,
        float_32_bit const  interpolation_param, // in range <0,1>
        angeo::coordinate_system&  output
        )
{
    ASSUMPTION(indices_of_keyframe_to_interpolate.first < keyframes.num_keyframes());
    ASSUMPTION(indices_of_keyframe_to_interpolate.second < keyframes.num_keyframes());
    ASSUMPTION(index_of_coord_system_in_keyframes < keyframes.num_coord_systems_per_keyframe());
    ASSUMPTION(0.0f <= interpolation_param && interpolation_param <= 1.0f);
    angeo::interpolate_spherical(
        keyframes.coord_system_at(indices_of_keyframe_to_interpolate.first, index_of_coord_system_in_keyframes),
        keyframes.coord_system_at(indices_of_keyframe_to_interpolate.second, index_of_coord_system_in_keyframes),
        interpolation_param,
        output
        );
}


void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        std::pair<std::size_t, std::size_t> const  indices_of_keyframe_to_interpolate,
        float_32_bit const  interpolation_param, // in range <0,1>
        std::vector<angeo::coordinate_system>&  output
        )
{
    output.resize(keyframes.num_coord_systems_per_keyframe());
    for (std::size_t  i = 0UL, n = keyframes.num_coord_systems_per_keyframe(); i != n; ++i)
        compute_coord_system_of_frame_of_keyframe_animation(
                keyframes,
                indices_of_keyframe_to_interpolate,
                i,
                interpolation_param,
                output.at(i)
                );
}


void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        std::pair<std::size_t, std::size_t> const  indices_of_keyframe_to_interpolate,
        float_32_bit const  interpolation_param, // in range <0,1>
        std::vector<matrix44>&  output
        )
{
    output.resize(keyframes.num_coord_systems_per_keyframe());
    for (std::size_t  i = 0UL, n = keyframes.num_coord_systems_per_keyframe(); i != n; ++i)
    {
        matrix44 M;
        {
            angeo::coordinate_system  S;
            compute_coord_system_of_frame_of_keyframe_animation(
                    keyframes,
                    indices_of_keyframe_to_interpolate,
                    i,
                    interpolation_param,
                    S
                    );
            angeo::from_base_matrix(S, M);
        }
        output.at(i) *= M;
    }
}


template<typename T>
static void  _compute_frame_of_keyframe_animation(
    keyframes const&  keyframes,
    float_32_bit const  time_point,
    std::vector<T>&  output
    )
{
    std::pair<std::size_t, std::size_t> const  indices_of_keyframe_to_interpolate =
        find_indices_of_keyframes_to_interpolate_for_time(keyframes, time_point);
    float_32_bit const  interpolation_param = qtgl::compute_interpolation_parameter(
        time_point,
        keyframes.time_point_at(indices_of_keyframe_to_interpolate.first),
        keyframes.time_point_at(indices_of_keyframe_to_interpolate.second)
        );
    compute_frame_of_keyframe_animation(
        keyframes,
        indices_of_keyframe_to_interpolate,
        interpolation_param,
        output
        );
}



void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        float_32_bit const  time_point,
        std::vector<angeo::coordinate_system>&  output
        )
{
    _compute_frame_of_keyframe_animation<angeo::coordinate_system>(keyframes, time_point, output);
}


void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        float_32_bit const  time_point,
        std::vector<matrix44>&  output
        )
{
    _compute_frame_of_keyframe_animation<matrix44>(keyframes, time_point, output);
}


void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        matrix44 const&  target_space, // typically, the target space is a camera space, i.e. view_projection_matrix
        float_32_bit const  time_point,
        std::vector<matrix44>&  output // old content won't be used and it will be owerwritten by the computed data.
        )
{
    std::vector<matrix44>  transform_matrices(keyframes.num_coord_systems_per_keyframe(), target_space);
    compute_frame_of_keyframe_animation(keyframes, time_point, transform_matrices);
    using std::swap;
    swap(transform_matrices, output);
}


float_32_bit  update_animation_time(
        float_32_bit  current_animation_time_point,
        float_32_bit const  time_delta,
        float_32_bit const  keyframes_start_time,
        float_32_bit const  keyframes_end_time
        )
{
    ASSUMPTION(keyframes_start_time <= keyframes_end_time);

    if (keyframes_end_time - keyframes_start_time < 0.0001f)
        return keyframes_start_time;

    current_animation_time_point += time_delta;

    if (current_animation_time_point < keyframes_start_time)
    {
        float_32_bit const  anim_duration = keyframes_start_time - keyframes_end_time;
        float_32_bit const  distance = current_animation_time_point - keyframes_end_time;
        float_32_bit const  param = distance / anim_duration;
        current_animation_time_point = (param - std::floor(param)) * distance;
    }
    else if (current_animation_time_point > keyframes_end_time)
    {
        float_32_bit const  anim_duration = keyframes_end_time - keyframes_start_time;
        float_32_bit const  distance = current_animation_time_point - keyframes_start_time;
        float_32_bit const  param = distance / anim_duration;
        current_animation_time_point = (param - std::floor(param)) * distance;
    }
    return std::max(keyframes_start_time, std::min(current_animation_time_point, keyframes_end_time));
}


}
