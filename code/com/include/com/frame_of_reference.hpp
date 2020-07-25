#ifndef COM_FRAME_OF_REFERENCE_HPP_INCLUDED
#   define COM_FRAME_OF_REFERENCE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <utility/dynamic_array.hpp>
#   include <vector>
#   include <unordered_set>
#   include <limits>

namespace com {


using  frame_id = natural_32_bit;
inline constexpr frame_id  invalid_frame_id() { return std::numeric_limits<natural_32_bit>::max(); }


struct  frames_provider
{
    frame_id  insert();
    void  erase(frame_id const  id);
    void  clear();

    bool  valid(frame_id const  id) const;

    void  set_parent(frame_id const  id, frame_id const  parent_id);
    frame_id  parent(frame_id const  id) const;
    std::vector<frame_id> const&  children(frame_id const  id) const;

    angeo::coordinate_system const&  frame(frame_id const  id) const;
    angeo::coordinate_system_explicit const&  frame_explicit(frame_id const  id) const;
    angeo::coordinate_system const&  frame_in_world_space(frame_id const  id) const;
    angeo::coordinate_system_explicit const&  frame_explicit_in_world_space(frame_id const  id) const;
    matrix44 const&  world_matrix(frame_id const  id) const;

    void  translate(frame_id const  id, vector3 const&  shift);
    void  rotate(frame_id const  id, quaternion const&  rotation);
    void  set_origin(frame_id const  id, vector3 const&  new_origin);
    void  set_orientation(frame_id const  id, quaternion const&  new_orientation);
    void  relocate(frame_id const  id, angeo::coordinate_system const& new_coord_system);
    void  relocate(frame_id const  id, vector3 const&  new_origin, quaternion const&  new_orientation);
    void  relocate_relative_to_parent(frame_id const  id, vector3 const&  new_origin, quaternion const&  new_orientation);
    void  relocate_relative_to_parent(frame_id const  id, frame_id const  relocation_id);

private:

    struct  frame_of_reference
    {
        frame_of_reference();

        frame_id  parent;
        std::vector<frame_id>  children;

        angeo::coordinate_system  frame;

        mutable angeo::coordinate_system_explicit  frame_explicit;
        mutable angeo::coordinate_system  frame_in_world_space;
        mutable angeo::coordinate_system_explicit  frame_explicit_in_world_space;
        mutable matrix44  world_matrix;

        mutable bool  is_frame_explicit_valid;
        mutable bool  is_frame_in_world_space_valid;
        mutable bool  is_frame_in_world_space_explicit_valid;
        mutable bool  is_world_matrix_valid;
    };

    void  invalidate(frame_id const  id) const;

    dynamic_array<frame_of_reference, frame_id>  m_frames;
};


}

#endif
