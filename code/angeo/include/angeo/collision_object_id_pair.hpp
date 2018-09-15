#ifndef ANGEO_COLLISION_OBJECT_ID_PAIR_HPP_INCLUDED
#   define ANGEO_COLLISION_OBJECT_ID_PAIR_HPP_INCLUDED

#   include <angeo/collision_object_id.hpp>
#   include <tuple>

namespace angeo {


/**
 * INVARIANT: For any instance 'p' of 'collision_object_id_pair': p.first < p.second
 */
using  collision_object_id_pair = std::pair<collision_object_id, collision_object_id>;


inline collision_object_id_pair  make_collision_object_id_pair(
        collision_object_id const  coid1,
        collision_object_id const  coid2
        ) noexcept
{
    return coid1 < coid2 ? collision_object_id_pair(coid1, coid2) : collision_object_id_pair(coid2, coid1) ;
}


}

#endif
