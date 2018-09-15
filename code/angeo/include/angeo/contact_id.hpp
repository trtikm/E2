#ifndef ANGEO_CONTACT_ID_HPP_INCLUDED
#   define ANGEO_CONTACT_ID_HPP_INCLUDED

#   include <angeo/collision_object_and_shape_feature_id.hpp>
#   include <tuple>

namespace angeo {


/**
 * INVARIANT: For each instance 'X' of the type 'contact_id':
 *            'get_object_id(get_first_collider_id(X)) < get_object_id(get_second_collider_id(X))'.
 */
using  contact_id = std::pair<collision_object_and_shape_feature_id, collision_object_and_shape_feature_id>;


inline contact_id  make_contact_id(
        collision_object_and_shape_feature_id const  first,
        collision_object_and_shape_feature_id const  second
        ) noexcept
{
    return get_object_id(first) < get_object_id(second) ? contact_id(first, second) : contact_id(second, first);
}

inline collision_object_and_shape_feature_id const&  get_first_collider_id(contact_id const&  cid)
{
    return cid.first;
}

inline collision_object_and_shape_feature_id const&  get_second_collider_id(contact_id const&  cid)
{
    return cid.second;
}


}

#endif
