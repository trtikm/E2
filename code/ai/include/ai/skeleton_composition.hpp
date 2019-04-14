#ifndef AI_SKELETON_COMNPOSITION_HPP_INCLUDED
#   define AI_SKELETON_COMNPOSITION_HPP_INCLUDED

#   include <angeo/coordinate_system.hpp>
#   include <angeo/collision_object_id.hpp>
#   include <angeo/rigid_body.hpp>
#   include <angeo/skeleton_kinematics.hpp>
#   include <angeo/tensor_math.hpp>
#   include <string>
#   include <memory>

namespace ai {


struct  skeleton_composition
{
    std::vector<angeo::coordinate_system>  pose_frames; // For each bone a coord. system in the local space of the parent bone (if any).
    std::vector<std::string>  names;
    std::vector<integer_32_bit>  parents;   // INVARIANT(parents.at(0)==-1); The first bone must be the 'pivot/root' bone of the skeleton, like 'hips'.
    std::vector<std::vector<integer_32_bit> >  children;

    std::vector<std::vector<angeo::joint_rotation_props> > rotation_props;

    std::vector<angeo::rigid_body_id>  rigid_bodies;    // Note: for some bones the corresponding IDs may be equal to 'invalid_rigid_body_id()'.
    std::vector<std::vector<angeo::collision_object_id> >  colliders;
    std::vector<angeo::coordinate_system>  rigid_body_frames;   // For each bone a frame of the associated rigid body (with the origin
                                                                // in the centre of mass) in frame of the corresponding bone. Ignored
                                                                // are coord. systems where the corrensponding rigid_body_id (in the
                                                                // vector 'rigid_bodies') is equal to 'invalid_rigid_body_id()'.
};


using  skeleton_composition_ptr = std::shared_ptr<skeleton_composition>;
using  skeleton_composition_const_ptr = std::shared_ptr<skeleton_composition const>;


}

#endif
