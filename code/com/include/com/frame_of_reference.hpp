#ifndef COM_FRAME_OF_REFERENCE_HPP_INCLUDED
#   define COM_FRAME_OF_REFERENCE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <unordered_map>
#   include <vector>
#   include <memory>

namespace com {


struct  frame_of_reference;
using  frame_ptr = std::shared_ptr<frame_of_reference>;
using  frame_id = frame_ptr;


struct  frame_of_reference
{
    frame_ptr  parent;
    angeo::coordinate_system  frame;
    mutable angeo::coordinate_system_explicit  frame_explicit;
    mutable matrix44  world_matrix;
    mutable bool  is_frame_explicit_valid;
    mutable bool  is_world_matrix_valid;
    bool  valid;
};


}

#endif
