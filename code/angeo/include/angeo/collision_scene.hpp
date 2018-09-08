#ifndef E2_TOOL_GFXTUNER_COLLISION_SYSTEM_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_COLLISION_SYSTEM_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/proximity_map.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <unordered_set>
#   include <array>
#   include <vector>
#   include <functional>
#   include <tuple>

namespace angeo {


std::pair<vector3, vector3>  compute_aabb_of_capsule(
        vector3 const&  point_1,
        vector3 const&  point_2,
        float_32_bit const  radius
        );


std::pair<vector3, vector3>  compute_aabb_of_line(
        vector3 const&  point_1,
        vector3 const&  point_2
        );


std::pair<vector3, vector3>  compute_aabb_of_triangle(
        vector3 const&  point_1,
        vector3 const&  point_2,
        vector3 const&  point_3
        );


}

namespace angeo {


enum struct  COLLISION_SHAPE_TYPE : natural_8_bit
{
    //BOX                     = 0,
    CAPSULE                 = 1,
    //CONE                    = 2,
    LINE                    = 3,
    POINT                   = 4,
    //POLYHEDRON              = 5,
    SPHERE                  = 6,
    //TORUS                   = 7,
    TRIANGLE                = 8,
};

inline natural_8_bit  as_number(COLLISION_SHAPE_TYPE const  cst)
{
    return static_cast<natural_8_bit>(cst);
}

inline constexpr natural_8_bit  get_max_collision_shape_type_id()
{ 
    return static_cast<natural_8_bit>(COLLISION_SHAPE_TYPE::TRIANGLE);
}


struct  collision_object_id
{
    natural_32_bit   m_shape_type : 4,
                     m_instance_index : 32 - 4;
};


static_assert(sizeof(collision_object_id) == sizeof(natural_32_bit), "The id must exactly fit to 32 bits.");


inline collision_object_id  make_collision_object_id(
        COLLISION_SHAPE_TYPE const  shape_type,
        natural_32_bit  instance_index
        ) noexcept
{
    collision_object_id  coid;
    coid.m_shape_type = static_cast<natural_8_bit>(shape_type);
    coid.m_instance_index = instance_index;
    return coid;
}


inline COLLISION_SHAPE_TYPE  get_shape_type(collision_object_id const  coid) noexcept
{
    return static_cast<COLLISION_SHAPE_TYPE>(coid.m_shape_type);
}


inline natural_32_bit  get_instance_index(collision_object_id const  coid) noexcept
{
    return coid.m_instance_index;
}


inline natural_32_bit  as_number(collision_object_id const  coid) noexcept
{
    return *reinterpret_cast<natural_32_bit const*>(&coid);
}


inline bool operator==(collision_object_id const  left, collision_object_id const  right) noexcept
{
    return as_number(left) == as_number(right);
}


inline bool operator!=(collision_object_id const  left, collision_object_id const  right) noexcept
{
    return as_number(left) != as_number(right);
}


inline bool operator<(collision_object_id const  left, collision_object_id const  right) noexcept
{
    return as_number(left) < as_number(right);
}


inline bool operator>(collision_object_id const  left, collision_object_id const  right) noexcept
{
    return as_number(left) > as_number(right);
}


inline bool operator<=(collision_object_id const  left, collision_object_id const  right) noexcept
{
    return as_number(left) <= as_number(right);
}


inline bool operator>=(collision_object_id const  left, collision_object_id const  right) noexcept
{
    return as_number(left) >= as_number(right);
}


}

namespace std {


template<> struct hash<angeo::collision_object_id>
{
    inline size_t operator()(angeo::collision_object_id const&  coid) const
    {
        return std::hash<natural_32_bit>()(as_number(coid));
    }
};


}

namespace angeo { namespace detail {


/**
 * INVARIANT: For any instance 'p' of 'collision_objects_pair': p.first < p.second
 */
using  collision_objects_pair = std::pair<collision_object_id, collision_object_id>;

inline collision_objects_pair  make_collision_objects_pair(
        collision_object_id const  coid1,
        collision_object_id const  coid2
        ) noexcept
{
    return coid1 < coid2 ? collision_objects_pair(coid1, coid2) : collision_objects_pair(coid2, coid1) ;
}


}}

namespace angeo {


enum struct  COLLISION_SHAPE_FEATURE_TYPE : natural_8_bit
{
    EDGE        = 0,
    FACE        = 1,
    VERTEX      = 2
};


struct  collision_shape_feature_id
{
    natural_32_bit   m_feature_type : 2,
                     m_feature_index : 32 - 2;
};


static_assert(sizeof(collision_shape_feature_id) == sizeof(natural_32_bit), "The id must exactly fit to 32 bits.");


inline collision_shape_feature_id  make_collision_shape_feature_id(
        COLLISION_SHAPE_FEATURE_TYPE const  feature_type,
        natural_32_bit  feature_index
        ) noexcept
{
    collision_shape_feature_id  cfid;
    cfid.m_feature_type = static_cast<natural_8_bit>(feature_type);
    cfid.m_feature_index = feature_index;
    return cfid;
}


inline COLLISION_SHAPE_FEATURE_TYPE  get_feature_type(collision_shape_feature_id const  cfid) noexcept
{
    return static_cast<COLLISION_SHAPE_FEATURE_TYPE>(cfid.m_feature_type);
}


inline natural_32_bit  get_feature_index(collision_shape_feature_id const  cfid) noexcept
{
    return cfid.m_feature_index;
}


inline natural_32_bit  as_number(collision_shape_feature_id const  cfid) noexcept
{
    return *reinterpret_cast<natural_32_bit const*>(&cfid);
}


inline bool operator==(collision_shape_feature_id const  left, collision_shape_feature_id const  right) noexcept
{
    return as_number(left) == as_number(right);
}


inline bool operator!=(collision_shape_feature_id const  left, collision_shape_feature_id const  right) noexcept
{
    return as_number(left) != as_number(right);
}


inline bool operator<(collision_shape_feature_id const  left, collision_shape_feature_id const  right) noexcept
{
    return as_number(left) < as_number(right);
}


inline bool operator>(collision_shape_feature_id const  left, collision_shape_feature_id const  right) noexcept
{
    return as_number(left) > as_number(right);
}


inline bool operator<=(collision_shape_feature_id const  left, collision_shape_feature_id const  right) noexcept
{
    return as_number(left) <= as_number(right);
}


inline bool operator>=(collision_shape_feature_id const  left, collision_shape_feature_id const  right) noexcept
{
    return as_number(left) >= as_number(right);
}


}

namespace std
{


template<> struct hash<angeo::collision_shape_feature_id>
{
    inline size_t operator()(angeo::collision_shape_feature_id const&  cfid) const
    {
        return std::hash<natural_32_bit>()(as_number(cfid));
    }
};


}

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

namespace angeo {


/**
 * INVARIANT: For each instance 'X' of the type 'contact_id': 'X.first < X.second'.
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

namespace angeo {


/**
* The callback function can early terminate the search for other objects by returning false.
*/
using  collision_object_acceptor = std::function<bool(collision_object_id)>;


/**
* INVARIANT: For each triple '(cid, contact_point, unit_normal)' passed to the 'contact_acceptor' callback
*            function: The contact unit normal vector 'unit_normal' points towards the collision object
*            with id 'get_object_id(get_first_collider_id(cid))'.
* The callback function can early terminate the search for other contacts by returning false.
*/
using  contact_acceptor = std::function<bool(contact_id const& cid, vector3 const& contact_point, vector3 const& unit_normal)>;


}

namespace angeo {


enum struct  COLLISION_MATERIAL_TYPE : natural_8_bit
{
    CONCRETE            = 0,
    GLASS               = 1,
    GUM                 = 2,
    PLASTIC             = 3,
    RUBBER              = 4,
    STEEL               = 5,
    WOOD                = 6,
};


}

namespace angeo {


/**
 * An instance of this type is a special purpose container for so called 'collision objects'.
 * A collision object is always a convex set of points in 3D Euklidian space, e.g. sphere,
 * triangle, box (see COLLISION_SHAPE_TYPE for complete list of supported shapes).
 *
 * Each object inserted to the container is considered either 'static' or 'dynamic', depending on
 * the value passed to the parameter 'is_dynamic' to a particular 'insert_*' function. Although you
 * can then call the method 'on_position_changed' for any object inserted into the contained (to
 * relocate that object's collision shape in the 3D space), it is expected that you will call it
 * frequently ONLY for dynamic objects. Calling that method for static objects may lead to decrease
 * of performance (when there was inserted a large number of static objects into the container).
 *
 * The container provides two kinds of queries on inserted collision objects:
 *      1. A search for objects in close proximity to either line or an axis aligned boundig box (AABB).
 *         An objects is in a proximity of a line/AABB if an only if its AABB has non-empty intersection
 *         with that line/AABB.
 *
 *      2. Computation of contact points (and normal vectors at those points) between individual objects.
 *         Given a pair of objects, computed contacts between them should be sufficient for computation
 *         of forces/impulses preventing deeper penetration of collision shapes of the objects.
 *         NOTE: Contacts are NEVER computed between any two STATIC objects.
 *
 * It is also possible to specify pairs of objects for which the computation of contacts will be
 * disabled. Each such pair must contain at least one dynamic object.
 */
struct  collision_scene
{
    collision_scene();

    collision_object_id  insert_capsule(
            vector3 const&  point_1,
            vector3 const&  point_2,
            float_32_bit const  radius,
            COLLISION_MATERIAL_TYPE const  material,
            bool const  is_dynamic
            );

    collision_object_id  insert_line(
            vector3 const&  point_1,
            vector3 const&  point_2,
            COLLISION_MATERIAL_TYPE const  material,
            bool const  is_dynamic
            );

    collision_object_id  insert_point(
            vector3 const&  position,
            COLLISION_MATERIAL_TYPE const  material,
            bool const  is_dynamic  
            );

    collision_object_id  insert_sphere(
            vector3 const&  centre,
            float_32_bit const  radius,
            COLLISION_MATERIAL_TYPE const  material,
            bool const  is_dynamic
            );

    collision_object_id  insert_triangle(
            vector3 const&  point_1,
            vector3 const&  point_2,
            vector3 const&  point_3,
            COLLISION_MATERIAL_TYPE const  material,
            bool const  is_dynamic
            );

    void  erase_object(collision_object_id const  coid);

    void  on_position_changed(collision_object_id const  coid, matrix44 const&  from_base_matrix);

    void  disable_colliding_of_dynamic_objects(collision_object_id const  coid_1, collision_object_id const  coid_2);
    void  enable_colliding_of_dynamic_objects(collision_object_id const  coid_1, collision_object_id const  coid_2);

    void  compute_contacts_of_all_dynamic_objects(contact_acceptor&  acceptor);
    void  compute_contacts_of_single_dynamic_object(
            collision_object_id const  coid,
            contact_acceptor&  acceptor
            );

    void  find_objects_in_proximity_to_axis_aligned_bounding_box(
            vector3 const& min_corner,
            vector3 const& max_corner,
            collision_object_acceptor&  acceptor
            );

    void  find_objects_in_proximity_to_line(
            vector3 const&  line_begin,
            vector3 const&  line_end,
            collision_object_acceptor&  acceptor
            );

private:

    vector3  get_object_aabb_min_corner(collision_object_id const  coid) const;
    vector3  get_object_aabb_max_corner(collision_object_id const  coid) const;

    void  insert_object(collision_object_id const  coid, bool const  is_dynamic)
    {
        if (is_dynamic) insert_dynamic_object(coid); else insert_static_object(coid);
    }
    void  insert_static_object(collision_object_id const  coid);
    void  insert_dynamic_object(collision_object_id const  coid);

    angeo::proximity_map<collision_object_id>  m_proximity_static_objects;  ///< These do not collide amongst each other.
    angeo::proximity_map<collision_object_id>  m_proximity_dynamic_objects; ///< These collide amongst each other, plus with
                                                                            ///< those in 'm_proximity_static_objects' map.
    std::unordered_set<collision_object_id>  m_dynamic_object_ids;
    bool  m_does_proximity_static_need_rebalancing;
    bool  m_does_proximity_dynamic_need_rebalancing;

    std::unordered_set<detail::collision_objects_pair>  m_disabled_colliding;

    std::array<std::vector<natural_32_bit>, get_max_collision_shape_type_id() + 1U>  m_invalid_object_ids;

    std::vector<std::pair<std::pair<vector3, vector3>, float_32_bit> >  m_capsules_geometry;
    std::vector<std::pair<vector3, vector3> >  m_capsules_bbox;
    std::vector<COLLISION_MATERIAL_TYPE>  m_capsules_material;

    std::vector<std::pair<vector3, vector3> >  m_lines_geometry;
    std::vector<std::pair<vector3, vector3> >  m_lines_bbox;
    std::vector<COLLISION_MATERIAL_TYPE>  m_lines_material;

    std::vector<vector3>  m_points_geometry;
    std::vector<COLLISION_MATERIAL_TYPE>  m_points_material;

    std::vector<std::pair<vector3, float_32_bit> >  m_spheres_geometry;
    std::vector<COLLISION_MATERIAL_TYPE>  m_spheres_material;

    std::vector<std::tuple<vector3, vector3, vector3> >  m_triangles_geometry;
    std::vector<std::pair<vector3, vector3> >  m_triangles_bbox;
    std::vector<COLLISION_MATERIAL_TYPE>  m_triangles_material;
};


}

#endif
