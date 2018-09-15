#ifndef ANGEO_CONTACT_ACCEPTOR_HPP_INCLUDED
#   define ANGEO_CONTACT_ACCEPTOR_HPP_INCLUDED

#   include <angeo/contact_id.hpp>
#   include <angeo/tensor_math.hpp>
#   include <functional>

namespace angeo {


/**
* INVARIANT: For each triple '(cid, contact_point, unit_normal)' passed to the 'contact_acceptor' callback
*            function: The contact unit normal vector 'unit_normal' points towards the collision object
*            with id 'get_object_id(get_first_collider_id(cid))'.
* The callback function can early terminate the search for other contacts by returning false.
*/
using  contact_acceptor = std::function<bool(contact_id const& cid,
                                             vector3 const& contact_point,
                                             vector3 const& unit_normal,
                                             float_32_bit  penetration_depth)>;


}

#endif
