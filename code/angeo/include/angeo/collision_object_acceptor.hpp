#ifndef ANGEO_COLLISION_OBJECT_ACCEPTOR_HPP_INCLUDED
#   define ANGEO_COLLISION_OBJECT_ACCEPTOR_HPP_INCLUDED

#   include <angeo/collision_object_id.hpp>
#   include <functional>

namespace angeo {


/**
* The callback function can early terminate the search for other objects by returning false.
*/
using  collision_object_acceptor = std::function<bool(collision_object_id)>;


}

#endif
