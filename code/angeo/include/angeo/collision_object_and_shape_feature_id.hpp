#ifndef ANGEO_COLLISION_OBJECT_AND_SHAPE_FEATURE_ID_HPP_INCLUDED
#   define ANGEO_COLLISION_OBJECT_AND_SHAPE_FEATURE_ID_HPP_INCLUDED

#   include <angeo/collision_object_id.hpp>
#   include <angeo/collision_shape_feature_id.hpp>
#   include <tuple>

namespace angeo {


using  collision_object_and_shape_feature_id = std::pair<collision_object_id, collision_shape_feature_id>;


inline collision_object_id  get_object_id(collision_object_and_shape_feature_id const&  cosfid)
{
    return cosfid.first;
}

inline collision_shape_feature_id  get_shape_feature_id(collision_object_and_shape_feature_id const&  cosfid)
{
    return cosfid.second;
}


}

#endif
