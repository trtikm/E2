#include <ai/detail/ideal_velocity_buider.hpp>
#include <utility/assumptions.hpp>

namespace ai { namespace detail {


ideal_velocity_buider::ideal_velocity_buider(
        skeletal_motion_templates::motion_template_cursor const&  start_cursor,
        skeletal_motion_templates const  motion_templates)
    : m_path({ start_cursor })
    , m_time(0.0f)
    , m_motion_templates(motion_templates)
{}


void  ideal_velocity_buider::reset(skeletal_motion_templates::motion_template_cursor const&  start_cursor)
{
    m_path.clear();
    m_path.push_back(start_cursor);
    m_time = 0.0f;
}


void  ideal_velocity_buider::extend(
        skeletal_motion_templates::motion_template_cursor const&  cursor,
        float_32_bit const  transition_time
        )
{
    if ((m_path.size() & 1UL) == 0UL)
    {
        // Even number of cursors.

        if (cursor.motion_name == m_path.back().motion_name && cursor.keyframe_index > m_path.back().keyframe_index)
        {
            m_path.back() = cursor;
            m_time += transition_time;
        }
        else
            m_path.push_back(cursor);
    }
    else
    {
        // Odd number of cursors.

        if (cursor.motion_name == m_path.back().motion_name && cursor.keyframe_index > m_path.back().keyframe_index)
        {
            m_path.push_back(cursor);
            m_time += transition_time;
        }
        else if (transition_time < 0.001f && m_path.size() == 1UL)
            m_path.back() = cursor;
        else
        {
            if (m_path.back().keyframe_index > 0U)
                --m_path.back().keyframe_index;
            m_path.push_back(m_path.back());
            ++m_path.back().keyframe_index;

            auto const&  keyframes = m_motion_templates.at(m_path.back().motion_name).keyframes;
            m_time += keyframes.time_point_at(m_path.crbegin()->keyframe_index) -
                      keyframes.time_point_at(std::next(m_path.crbegin())->keyframe_index);

            m_path.push_back(cursor);
        }
    }
}


void  ideal_velocity_buider::close()
{
    if ((m_path.size() & 1UL) != 0UL)
    {
        // Odd number of cursors.

        auto const&  keyframes = m_motion_templates.at(m_path.back().motion_name).keyframes;

        if (m_path.back().keyframe_index + 1U >= keyframes.num_keyframes())
            --m_path.back().keyframe_index;
        m_path.push_back(m_path.back());
        ++m_path.back().keyframe_index;

        m_time += keyframes.time_point_at(m_path.crbegin()->keyframe_index) -
                  keyframes.time_point_at(std::next(m_path.crbegin())->keyframe_index);
    }
}


void  ideal_velocity_buider::close(
        angeo::coordinate_system const&  motion_object_frame,
        vector3&  ideal_linear_velocity_in_world_space,
        vector3&  ideal_angular_velocity_in_world_space
        )
{
    ASSUMPTION(!m_path.empty() && m_time > 0.0001f);

    close();

    matrix44  A, M = matrix44_identity();
    for (auto rit = m_path.crbegin(); rit != m_path.crend(); ++rit)
    {
        angeo::from_base_matrix(m_motion_templates.at(rit->motion_name).reference_frames.at(rit->keyframe_index), A);
        M = A * M;
        ++rit;
        angeo::to_base_matrix(m_motion_templates.at(rit->motion_name).reference_frames.at(rit->keyframe_index), A);
        M = A * M;
    }
    vector3  pos;
    matrix33  rot;
    decompose_matrix44(M, pos, rot);

    angeo::from_base_matrix(motion_object_frame, A);

    ideal_linear_velocity_in_world_space = transform_vector(pos, A) / m_time;

    vector3  axis;
    float_32_bit const angle = quaternion_to_angle_axis(rotation_matrix_to_quaternion(rot), axis);
    ideal_angular_velocity_in_world_space = (angle / m_time) * transform_vector(axis, A);
}

}}
