#ifndef E2_TOOL_GFXTUNER_COLLISION_SYSTEM_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_COLLISION_SYSTEM_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <angeo/proximity_map.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <unordered_set>
#   include <functional>
//#   include <gfxtuner/scene.hpp>
#   include <tuple>


enum struct  COLLISION_SHAPE_TYPE : natural_8_bit
{
    LINE,
    POIN,
    SPHERE,
    TRIANGLE,
    //CAPSULE             = 4,
    //BOX                 = 5,
    //TORUS               = 6,
    //CONE                = 7,
    //POLYHEDRON          = 8,
};


enum struct  COLLISION_MATERIAL_TYPE : natural_8_bit
{
    CONCRETE,
    GLASS,
    GUM,
    PLASTIC,
    STEEL,
    WOOD,
};


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
    return *(natural_32_bit const*)&coid;
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


namespace std
{
    template<> struct hash<collision_object_id>
    {
        inline size_t operator()(collision_object_id const&  coid) const
        {
            return as_number(coid);
        }
    };
}


/// For any instance 'p' of 'collision_objects_pair' there is always true that:
///     p.first < p.second
using  collision_objects_pair = std::pair<collision_object_id, collision_object_id>;


inline collision_objects_pair  make_collision_objects_pair(
        collision_object_id const  coid1,
        collision_object_id const  coid2
        )
{
    return as_number(coid1) < as_number(coid2) ? collision_objects_pair(coid1, coid2) :
                                                 collision_objects_pair(coid2, coid1) ;
}


enum struct  COLLISION_FEATURE_TYPE : natural_8_bit
{
    EDGE,
    FACE,
    VERTEX
};


struct  collision_feature_id
{
    natural_32_bit   m_feature_type : 2,
                     m_feature_index : 32 - 2;
};


static_assert(sizeof(collision_feature_id) == sizeof(natural_32_bit), "The id must exactly fit to 32 bits.");


struct  collision_point
{
    /// INVARIANT: as_number(m_first_object) < as_number(m_second_object).
    collision_object_id  m_first_object;
    collision_object_id  m_second_object;   
    COLLISION_MATERIAL_TYPE  m_first_object_material;
    COLLISION_MATERIAL_TYPE  m_second_object_material;
    collision_feature_id  m_first_object_feature_id;
    collision_feature_id  m_second_object_feature_id;
    vector3  m_position;
    vector3  m_normal; ///< Points into the 'm_first_object'
};


using  collision_point_acceptor = std::function<bool(collision_point const&)>;
using  collision_object_acceptor = std::function<bool(collision_object_id)>;


/**
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
/// Pass 'false' only if you will NOT call 'on_position_changed' often
/// for this object during the simulation (like never or very rarely;
/// definitely not in every frame). 
struct  collision_system
{
    collision_object_id  insert_point(
            vector3 const&  position,
            COLLISION_MATERIAL_TYPE const  material,
            bool const  is_dynamic  
            );

    collision_object_id  insert_line(
            vector3 const&  point_1,
            vector3 const&  point_2,
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

    collision_object_id  insert_sphere(
            vector3 const&  centre,
            float_32_bit const  radius,
            COLLISION_MATERIAL_TYPE const  material,
            bool const  is_dynamic
            );

    void  erase_object(collision_object_id const  coid);

    void  on_position_changed(collision_object_id const  coid, matrix44 const&  from_base_matrix);

    void  disable_colliding_of_dynamic_objects(collision_object_id const  coid_1, collision_object_id const  coid_2);
    void  enable_colliding_of_dynamic_objects(collision_object_id const  coid_1, collision_object_id const  coid_2);

    void  compute_collisions_of_all_dynamic_objects(collision_point_acceptor&  acceptor);
    void  compute_collisions_of_single_dynamic_object(
            collision_object_id const  coid,
            collision_point_acceptor&  acceptor
            );

    void  find_objects_in_axis_aligned_bounding_box(
            vector3 const& min_corner,
            vector3 const& max_corner,
            collision_object_acceptor&  acceptor
            );

    void  find_objects_on_line(
            vector3 const&  line_begin,
            vector3 const&  line_end,
            collision_object_acceptor&  acceptor
            );

private:

    angeo::proximity_map<collision_object_id>  m_proximity_static_objects;  ///< These do not collide amongst each other.
    angeo::proximity_map<collision_object_id>  m_proximity_dynamic_objects; ///< These collide amongst each other, plus with
                                                                            ///< those in 'm_proximity_static_objects' map.
    std::unordered_set<collision_object_id>  m_dynamic_object_ids;
    bool  m_does_proximity_static_need_rebalancing;
    bool  m_does_proximity_dynamic_need_rebalancing;

    std::unordered_set<collision_objects_pair>  m_disabled_colliding;

    std::vector<vector3>  m_points_geometry;
    std::vector<COLLISION_MATERIAL_TYPE>  m_points_material;

    std::vector<std::pair<vector3, vector3> >  m_lines_geometry;
    std::vector<std::pair<vector3, vector3> >  m_lines_bbox;
    std::vector<COLLISION_MATERIAL_TYPE>  m_lines_material;

    std::vector<std::tuple<vector3, vector3, vector3> >  m_triangles_geometry;
    std::vector<std::pair<vector3, vector3> >  m_triangles_bbox;
    std::vector<COLLISION_MATERIAL_TYPE>  m_triangles_material;

    std::vector<std::pair<vector3, float_32_bit> >  m_spheres_geometry;
    std::vector<COLLISION_MATERIAL_TYPE>  m_spheres_material;
};


#endif
