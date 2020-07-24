#include <com/frame_of_reference.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <algorithm>

namespace com {


frames_provider::frame_of_reference::frame_of_reference()
    : parent(invalid_frame_id())
    , children()
    , frame()
    , frame_explicit()
    , frame_in_world_space()
    , frame_explicit_in_world_space()
    , world_matrix(matrix44_identity())
    , is_frame_explicit_valid(true)
    , is_frame_in_world_space_valid(true)
    , is_frame_in_world_space_explicit_valid(true)
    , is_world_matrix_valid(true)
{}


frame_id  frames_provider::insert()
{
    return m_frames.insert();
}


void  frames_provider::erase(frame_id const  id)
{
    ASSUMPTION(valid(id));
    frame_of_reference&  frame = m_frames.at(id);
    for (frame_id  child_id : frame.children)
        set_parent(child_id, frame.parent);
    set_parent(id, invalid_frame_id());
    m_frames.erase(id);
}


void  frames_provider::clear()
{
    m_frames.clear();
}


bool  frames_provider::valid(frame_id const  id) const
{
    return m_frames.valid(id);
}


void  frames_provider::set_parent(frame_id const  id, frame_id const  parent_id)
{
    ASSUMPTION(
        valid(id) &&
        [this](frame_id const  id, frame_id  parent_id) -> bool {
            for ( ; parent_id != invalid_frame_id(); parent_id = parent(parent_id))
                if (parent_id == id)
                    return false;
            return true;
        }(id, parent_id)
        );

    frame_of_reference&  frame = m_frames.at(id);
    if (frame.parent == parent_id)
        return;
    if (frame.parent != invalid_frame_id())
    {
        auto&  chs = m_frames.at(frame.parent).children;
        chs.erase(std::find(chs.begin(), chs.end(), id));
    }
    frame.parent = parent_id;
    if (parent_id != invalid_frame_id())
        m_frames.at(parent_id).children.push_back(id);
    invalidate(id);
}


frame_id  frames_provider::parent(frame_id const  id) const
{
    ASSUMPTION(valid(id));
    return m_frames.at(id).parent;
}


std::vector<frame_id> const&  frames_provider::children(frame_id const  id) const
{
    ASSUMPTION(valid(id));
    return m_frames.at(id).children;
}


angeo::coordinate_system const&  frames_provider::frame(frame_id const  id) const
{
    ASSUMPTION(valid(id));
    return m_frames.at(id).frame;
}


angeo::coordinate_system_explicit const&  frames_provider::frame_explicit(frame_id const  id) const
{
    ASSUMPTION(valid(id));
    frame_of_reference const&  frame = m_frames.at(id);
    if (!frame.is_frame_explicit_valid)
    {
        frame.frame_explicit = frame.frame;
        frame.is_frame_explicit_valid = true;
    }
    return frame.frame_explicit;
}


angeo::coordinate_system const&  frames_provider::frame_in_world_space(frame_id const  id) const
{
    ASSUMPTION(valid(id));
    frame_of_reference const&  frame = m_frames.at(id);
    if (!frame.is_frame_in_world_space_valid)
    {
        world_matrix(id);
        vector3  u;
        matrix33  R;
        decompose_matrix44(frame.world_matrix, u, R);
        frame.frame_in_world_space = angeo::coordinate_system{u, R};
        frame.is_frame_in_world_space_valid = true;
    }
    return frame.frame_in_world_space;
}


angeo::coordinate_system_explicit const&  frames_provider::frame_explicit_in_world_space(frame_id const  id) const
{
    ASSUMPTION(valid(id));
    frame_of_reference const&  frame = m_frames.at(id);
    if (!frame.is_frame_in_world_space_explicit_valid)
    {
        frame_in_world_space(id);
        frame.frame_explicit_in_world_space = frame.frame_in_world_space;
        frame.is_frame_in_world_space_explicit_valid = true;
    }
    return frame.frame_explicit_in_world_space;
}


matrix44 const&  frames_provider::world_matrix(frame_id const  id) const
{
    ASSUMPTION(valid(id));
    frame_of_reference const&  frame = m_frames.at(id);
    if (!frame.is_world_matrix_valid)
    {
        angeo::from_base_matrix(frame.frame, frame.world_matrix);
        if (frame.parent != invalid_frame_id())
            frame.world_matrix = world_matrix(frame.parent) * frame.world_matrix;
        frame.is_world_matrix_valid = true;
    }
    return frame.world_matrix;
}


void  frames_provider::translate(frame_id const  id, vector3 const&  shift)
{
    ASSUMPTION(valid(id));
    angeo::translate(m_frames.at(id).frame, shift);
    invalidate(id);
}


void  frames_provider::rotate(frame_id const  id, quaternion const&  rotation)
{
    ASSUMPTION(valid(id));
    angeo::rotate(m_frames.at(id).frame, rotation);
    invalidate(id);
}


void  frames_provider::set_origin(frame_id const  id, vector3 const&  new_origin)
{
    ASSUMPTION(valid(id));
    m_frames.at(id).frame.set_origin(new_origin);
    invalidate(id);
}


void  frames_provider::set_orientation(frame_id const  id, quaternion const&  new_orientation)
{
    ASSUMPTION(valid(id));
    m_frames.at(id).frame.set_orientation(new_orientation);
    invalidate(id);
}


void  frames_provider::relocate(frame_id const  id, angeo::coordinate_system const& new_coord_system)
{
    relocate(id, new_coord_system.origin(), new_coord_system.orientation());
}


void  frames_provider::relocate(frame_id const  id, vector3 const&  new_origin, quaternion const&  new_orientation)
{
    ASSUMPTION(valid(id));
    angeo::coordinate_system&  frame = m_frames.at(id).frame;
    frame.set_origin(new_origin);
    frame.set_orientation(new_orientation);
    invalidate(id);
}


void  frames_provider::relocate_relative_to_parent(frame_id const  id, vector3 const&  new_origin, quaternion const&  new_orientation)
{
    frame_id const  parent_id = parent(id);
    if (parent_id == invalid_frame_id())
        relocate(id, new_origin, new_orientation);
    else
    {
        vector3  relocated_origin;
        quaternion  relocated_orientation;
        {
            matrix44  from_relocated_to_parent_matrix;
            matrix33  R;
            {
                matrix44  from_relocated_frame_matrix;
                compose_from_base_matrix(new_origin, quaternion_to_rotation_matrix(new_orientation), from_relocated_frame_matrix);
                matrix44  to_parent_matrix;
                {
                    vector3  u;
                    decompose_matrix44(world_matrix(parent_id), u, R);
                    compose_to_base_matrix(u, R, to_parent_matrix);
                }
                from_relocated_to_parent_matrix = to_parent_matrix * from_relocated_frame_matrix;
            }

            decompose_matrix44(from_relocated_to_parent_matrix, relocated_origin, R);
            relocated_orientation = rotation_matrix_to_quaternion(R);
        }

        relocate(id, relocated_origin, relocated_orientation);
    }
}


void  frames_provider::invalidate(frame_id const  id) const
{
    frame_of_reference const&  frame = m_frames.at(id);
    frame.is_frame_explicit_valid = false;
    frame.is_frame_in_world_space_valid = false;
    frame.is_frame_in_world_space_explicit_valid = false;
    frame.is_world_matrix_valid = false;
    for (frame_id  child_id : frame.children)
        invalidate(child_id);
}


}
