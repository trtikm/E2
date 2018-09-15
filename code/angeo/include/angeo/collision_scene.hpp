#ifndef ANGEO_COLLISION_SCENE_HPP_INCLUDED
#   define ANGEO_COLLISION_SCENE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/axis_aligned_bounding_box.hpp>
#   include <angeo/collision_material.hpp>
#   include <angeo/collision_object_acceptor.hpp>
#   include <angeo/collision_object_and_shape_feature_id.hpp>
#   include <angeo/collision_object_id.hpp>
#   include <angeo/collision_object_id_pair.hpp>
#   include <angeo/collision_shape_feature_id.hpp>
#   include <angeo/collision_shape_id.hpp>
#   include <angeo/contact_acceptor.hpp>
#   include <angeo/contact_id.hpp>
#   include <angeo/proximity_map.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <unordered_set>
#   include <array>
#   include <vector>
#   include <functional>
#   include <tuple>

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
 *
 * NOTE: In order to achieve the best performance we strongly recommend to prevent interleaving of calls
 *       to methods from different 'purpose groups' as much as possible. The methods of 'collison_scene'
 *       class are split into these two purpose groups:
 *          1. [Update group]: All 'insert_*' methods, 'erase_object', and 'on_position_changed'.
 *          2. [Query group]: Both 'compute_contacts_*' methods, both 'find_objects_*' methods, and
 *                            both 'enable/disable_colliding_of_dynamic_objects'.
 *       This is because each method in the 'update group' marks an internal 'proximity_map' as
 *       'need-for-rebalancing' and each method in the 'query group' must be performed on rebalanced
 *       proximity map, i.e. the method must call 'rebalance' method on the proximity map before
 *       starting the query, if the mark 'need-for-rebalancing' is set. The 'rebalance' method can
 *       be costly, especially for large scenes and/or when it is called too often.
 */
struct  collision_scene
{
    collision_scene();

    /// In the models space the end points have coordinates:
    ///      end_point_1_in_model_space = vector3(0,0,+half_distance_between_end_points)
    ///      end_point_2_in_model_space = vector3(0,0,-half_distance_between_end_points)
    collision_object_id  insert_capsule(
            float_32_bit const  half_distance_between_end_points,
            float_32_bit const  thickness_from_central_line,
            matrix44 const&  from_base_matrix,
            COLLISION_MATERIAL_TYPE const  material,
            bool const  is_dynamic
            );

    /// In the models space the end points have coordinates:
    ///      end_point_1_in_model_space = vector3(0,0,+half_distance_between_end_points)
    ///      end_point_2_in_model_space = vector3(0,0,-half_distance_between_end_points)
    collision_object_id  insert_line(
            float_32_bit const  half_distance_between_end_points,
            matrix44 const&  from_base_matrix,
            COLLISION_MATERIAL_TYPE const  material,
            bool const  is_dynamic
            );

    /// In the models space the points have coordinates:
    ///      point_in_model_space = vector3_zero()
    collision_object_id  insert_point(
            matrix44 const&  from_base_matrix,
            COLLISION_MATERIAL_TYPE const  material,
            bool const  is_dynamic
            );

    /// In the models space the points have coordinates:
    ///      point_in_model_space = vector3_zero()
    collision_object_id  insert_sphere(
            float_32_bit const  radius,
            matrix44 const&  from_base_matrix,
            COLLISION_MATERIAL_TYPE const  material,
            bool const  is_dynamic
            );

    /// In the model space the end points are assumed as follows:
    ///      end_point_1_in_model_space = vector3_zero()
    ///      end_point_2_in_model_space = vector3(end_point_2_x_coord_in_model_space,0,0)
    ///      end_point_3_in_model_space = expand23(end_point_3_in_model_space,0)
    collision_object_id  insert_triangle(
            float_32_bit const  end_point_2_x_coord_in_model_space,
            vector2 const&  end_point_3_in_model_space,
            matrix44 const&  from_base_matrix,
            COLLISION_MATERIAL_TYPE const  material,
            bool const  is_dynamic
            );

    void  erase_object(collision_object_id const  coid);

    void  on_position_changed(collision_object_id const  coid, matrix44 const&  from_base_matrix);

    void  disable_colliding_of_dynamic_objects(collision_object_id const  coid_1, collision_object_id const  coid_2);
    void  enable_colliding_of_dynamic_objects(collision_object_id const  coid_1, collision_object_id const  coid_2);

    void  compute_contacts_of_all_dynamic_objects(contact_acceptor const&  acceptor, bool  with_static = true);
    void  compute_contacts_of_single_dynamic_object(
            collision_object_id const  coid,
            contact_acceptor const&  acceptor,
            bool const with_static = true,
            bool const with_dynamic = true
            );

    void  find_objects_in_proximity_to_axis_aligned_bounding_box(
            vector3 const& min_corner,
            vector3 const& max_corner,
            bool const search_static,
            bool const search_dynamic,
            collision_object_acceptor const&  acceptor
            );

    void  find_objects_in_proximity_to_line(
            vector3 const&  line_begin,
            vector3 const&  line_end,
            bool const search_static,
            bool const search_dynamic,
            collision_object_acceptor const&  acceptor
            );

private:

    vector3  get_object_aabb_min_corner(collision_object_id const  coid) const;
    vector3  get_object_aabb_max_corner(collision_object_id const  coid) const;

    void  update_shape_position(collision_object_id const  coid, matrix44 const&  from_base_matrix);

    void  insert_object(collision_object_id const  coid, bool const  is_dynamic)
    {
        if (is_dynamic) insert_dynamic_object(coid); else insert_static_object(coid);
    }
    void  insert_static_object(collision_object_id const  coid);
    void  insert_dynamic_object(collision_object_id const  coid);

    void  rebalance_static_proximity_map_if_needed();
    void  rebalance_dynamic_proximity_map_if_needed();

    bool  compute_contacts(
            collision_object_id_pair  cop,
            contact_acceptor const&  acceptor,
            bool const  bboxes_of_objects_surely_intersect = false
            );

    bool  compute_contacts__capsule_vs_capsule(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor );
    bool  compute_contacts__capsule_vs_line(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor);
    bool  compute_contacts__capsule_vs_point(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor);
    bool  compute_contacts__capsule_vs_sphere(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor);
    bool  compute_contacts__capsule_vs_triangle(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor);

    bool  compute_contacts__line_vs_line(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor);
    bool  compute_contacts__line_vs_point(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor);
    bool  compute_contacts__line_vs_sphere(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor);
    bool  compute_contacts__line_vs_triangle(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor);

    bool  compute_contacts__point_vs_point(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor);
    bool  compute_contacts__point_vs_sphere(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor);
    bool  compute_contacts__point_vs_triangle(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor);

    bool  compute_contacts__sphere_vs_sphere(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor);
    bool  compute_contacts__sphere_vs_triangle(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor);

    bool  compute_contacts__triangle_vs_triangle(collision_object_id const  coid1, collision_object_id const  coid2, contact_acceptor const&  acceptor);

    /////////////////////////////////////////////////////////////////////////////////
    // DATA
    /////////////////////////////////////////////////////////////////////////////////


    angeo::proximity_map<collision_object_id>  m_proximity_static_objects;  ///< These do not collide amongst each other.
    angeo::proximity_map<collision_object_id>  m_proximity_dynamic_objects; ///< These collide amongst each other, plus with
                                                                            ///< those in 'm_proximity_static_objects' map.
    std::unordered_set<collision_object_id>  m_dynamic_object_ids;
    bool  m_does_proximity_static_need_rebalancing;
    bool  m_does_proximity_dynamic_need_rebalancing;

    std::unordered_set<collision_object_id_pair>  m_disabled_colliding;

    std::array<std::vector<natural_32_bit>, get_max_collision_shape_type_id() + 1U>  m_invalid_object_ids;

    /////////////////////////////////////////////////////////////////////////////////
    // CAPSULES

    struct capsule_geometry
    {
        vector3  end_point_1_in_world_space;
        vector3  end_point_2_in_world_space;

        // In the models space the end points have coordinates:
        //      end_point_1_in_model_space = vector3(0,0,+half_distance_between_end_points)
        //      end_point_2_in_model_space = vector3(0,0,-half_distance_between_end_points)

        float_32_bit  half_distance_between_end_points;

        float_32_bit  thickness_from_central_line;
    };

    std::vector<capsule_geometry>  m_capsules_geometry;
    std::vector<axis_aligned_bounding_box>  m_capsules_bbox;
    std::vector<COLLISION_MATERIAL_TYPE>  m_capsules_material;

    /////////////////////////////////////////////////////////////////////////////////
    // LINES

    struct line_geometry
    {
        vector3  end_point_1_in_world_space;
        vector3  end_point_2_in_world_space;

        // In the models space the end points have coordinates:
        //      end_point_1_in_model_space = vector3(0,0,+half_distance_between_end_points)
        //      end_point_2_in_model_space = vector3(0,0,-half_distance_between_end_points)

        float_32_bit  half_distance_between_end_points;
    };

    std::vector<line_geometry>  m_lines_geometry;
    std::vector<axis_aligned_bounding_box>  m_lines_bbox;
    std::vector<COLLISION_MATERIAL_TYPE>  m_lines_material;

    /////////////////////////////////////////////////////////////////////////////////
    // POINTS

    std::vector<vector3>  m_points_geometry;
    std::vector<COLLISION_MATERIAL_TYPE>  m_points_material;

    /////////////////////////////////////////////////////////////////////////////////
    // SPHERES

    struct sphere_geometry
    {
        vector3  center_in_world_space;
        float_32_bit  radius;
    };

    std::vector<sphere_geometry>  m_spheres_geometry;
    std::vector<COLLISION_MATERIAL_TYPE>  m_spheres_material;

    /////////////////////////////////////////////////////////////////////////////////
    // TRIANGLES

    struct triangle_geometry
    {
        vector3  end_point_1_in_world_space;
        vector3  end_point_2_in_world_space;
        vector3  end_point_3_in_world_space;

        vector3  unit_normal_in_world_space;

        // In the model space the end points are assumed as follows:
        //      end_point_1_in_model_space = vector3_zero()
        //      end_point_2_in_model_space = vector3(end_point_2_x_coord_in_model_space,0,0)
        //      end_point_3_in_model_space = expand23(end_point_3_in_model_space,0)

        float_32_bit  end_point_2_x_coord_in_model_space;
        vector2  end_point_3_in_model_space;
    };

    std::vector<triangle_geometry>  m_triangles_geometry;
    std::vector<axis_aligned_bounding_box>  m_triangles_bbox;
    std::vector<COLLISION_MATERIAL_TYPE>  m_triangles_material;
};


}

#endif
