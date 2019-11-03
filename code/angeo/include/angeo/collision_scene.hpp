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

    /// In the model space the end points of 'i'-th triagle (i in range 0..num_triangles-1) are as follows:
    ///      end_point_1_in_model_space = getter_of_end_points_in_model_space(i, 0U)
    ///      end_point_2_in_model_space = getter_of_end_points_in_model_space(i, 1U)
    ///      end_point_3_in_model_space = getter_of_end_points_in_model_space(i, 2U)
    /// The front face of each triangle is the one defined by the counter-clock-wise orientation of vertices.
    /// The 'edges_ignore_mask' of each triangle is set to 0U. If you want to change them, then call the function
    /// 'set_trinagle_edges_ignore_mask' for 'collision_object_id's obtained from this function.
    void  insert_triangle_mesh(
            natural_32_bit const  num_triangles,
            std::function<vector3(natural_32_bit, natural_8_bit)> const&  getter_of_end_points_in_model_space,
            matrix44 const&  from_base_matrix,
            COLLISION_MATERIAL_TYPE const  material,
            bool const  is_dynamic, // Although not mandatory, it is recomended to pass 'false' here (for performance reasons).
            std::vector<collision_object_id>&  output_coids_of_individual_triangles
            );

    void  erase_object(collision_object_id const  coid);

    void  clear();

    void  on_position_changed(collision_object_id const  coid, matrix44 const&  from_base_matrix);

    void  disable_colliding(collision_object_id const  coid_1, collision_object_id const  coid_2);
    void  enable_colliding(collision_object_id const  coid_1, collision_object_id const  coid_2);

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
            ) const;

    void  find_objects_in_proximity_to_line(
            vector3 const&  line_begin,
            vector3 const&  line_end,
            bool const  search_static,
            bool const  search_dynamic,
            collision_object_acceptor const&  acceptor
            ) const;

    bool  ray_cast_precise_collision_object_acceptor(
            collision_object_id const  coid,
            vector3 const&  ray_origin,
            vector3 const&  ray_end,
            vector3 const&  ray_unit_direction_vector,
            float_32_bit const  ray_length,
            std::function<bool(collision_object_id, float_32_bit)> const&  acceptor,
            std::unordered_set<collision_object_id> const* const  ignored_coids_ptr // pass nullptr, if there is nothing to ignore.
            ) const;

    bool  ray_cast(
            vector3 const&  ray_origin,
            vector3 const&  ray_unit_direction_vector,
            float_32_bit const  ray_length,
            bool const  search_static,
            bool const  search_dynamic,
            collision_object_id*  nearest_coid,
            float_32_bit*  ray_parameter_to_nearest_coid,
            std::unordered_set<collision_object_id> const* const  ignored_coids_ptr // pass nullptr, if there is nothing to ignore.
            ) const;

    vector3  get_object_aabb_min_corner(collision_object_id const  coid) const;
    vector3  get_object_aabb_max_corner(collision_object_id const  coid) const;

    bool  is_dynamic(collision_object_id const  coid) const { return m_dynamic_object_ids.count(coid) != 0UL; }

    vector3 const&  get_capsule_end_point_1_in_world_space(collision_object_id const  coid) const;
    vector3 const&  get_capsule_end_point_2_in_world_space(collision_object_id const  coid) const;
    float_32_bit  get_capsule_half_distance_between_end_points(collision_object_id const  coid) const;
    float_32_bit  get_capsule_thickness_from_central_line(collision_object_id const  coid) const;

    vector3 const&  get_sphere_center_in_world_space(collision_object_id const  coid) const;
    float_32_bit  get_sphere_radius(collision_object_id const  coid) const;

    std::function<vector3(natural_32_bit, natural_8_bit)> const&  get_triangle_points_getter(collision_object_id const  coid) const;
    natural_32_bit  get_triangle_index(collision_object_id const  coid) const;
    vector3 const&  get_triangle_end_point_in_world_space(collision_object_id const  coid, natural_8_bit const  end_point_index) const;
    vector3 const&  get_triangle_unit_normal_in_world_space(collision_object_id const  coid) const;
    natural_8_bit  get_trinagle_edges_ignore_mask(collision_object_id const  coid) const;
    void  set_trinagle_edges_ignore_mask(collision_object_id const  coid, natural_8_bit const  mask);
    collision_object_id   get_trinagle_neighbour_over_edge(collision_object_id const  coid, natural_32_bit const  edge_index) const;
    void  set_trinagle_neighbour_over_edge(
            collision_object_id const  coid,
            natural_32_bit const  edge_index,
            collision_object_id const  neighbour_triangle_coid
            );

    COLLISION_MATERIAL_TYPE  get_material(collision_object_id const  coid) const;

    struct  statistics
    {
        statistics(
                proximity_map<collision_object_id> const& static_proximity_map,
                proximity_map<collision_object_id> const& dynamic_proximity_map
                )
            : num_capsules(0U)
            , num_lines(0U)
            , num_points(0U)
            , num_spheres(0U)
            , num_triangles(0U)
            , num_compute_contacts_calls_in_last_frame(0U)
            , max_num_compute_contacts_calls_till_last_frame(0U)
            , num_contacts_in_last_frame(0U)
            , max_num_contacts_till_last_frame(0U)
            , static_objects_proximity(&static_proximity_map.get_statistics())
            , dynamic_objects_proximity(&dynamic_proximity_map.get_statistics())
        {}

        void  clear()
        {
            num_capsules = 0U;
            num_lines = 0U;
            num_points = 0U;
            num_spheres = 0U;
            num_triangles = 0U;
            num_compute_contacts_calls_in_last_frame = 0U;
            max_num_compute_contacts_calls_till_last_frame = 0U;
            num_contacts_in_last_frame = 0U;
            max_num_contacts_till_last_frame = 0U;

            const_cast<proximity_map<collision_object_id>::statistics*>(static_objects_proximity)->clear();
            const_cast<proximity_map<collision_object_id>::statistics*>(dynamic_objects_proximity)->clear();
        }

        // Call this method in the beginning of each simulation step.
        void  on_next_frame() const
        {
            auto const  mutable_self = const_cast<statistics*>(this);

            if (max_num_compute_contacts_calls_till_last_frame < num_compute_contacts_calls_in_last_frame)
                mutable_self->max_num_compute_contacts_calls_till_last_frame = num_compute_contacts_calls_in_last_frame;
            mutable_self->num_compute_contacts_calls_in_last_frame = 0U;

            if (max_num_contacts_till_last_frame < num_contacts_in_last_frame)
                mutable_self->max_num_contacts_till_last_frame = num_contacts_in_last_frame;
            mutable_self->num_contacts_in_last_frame = 0U;

            static_objects_proximity->on_next_frame();
            dynamic_objects_proximity->on_next_frame();
        }

        natural_32_bit  num_capsules;
        natural_32_bit  num_lines;
        natural_32_bit  num_points;
        natural_32_bit  num_spheres;
        natural_32_bit  num_triangles;
        natural_32_bit  num_compute_contacts_calls_in_last_frame;
        natural_32_bit  max_num_compute_contacts_calls_till_last_frame;  // I.e. the last frame is not included; see 'num_computed_contacts_in_last_frame' for the last frame.
        natural_32_bit  num_contacts_in_last_frame;
        natural_32_bit  max_num_contacts_till_last_frame;   // I.e. the last frame is not included; see 'num_contacts_in_last_frame' for the last frame.

        proximity_map<collision_object_id>::statistics const*  static_objects_proximity;
        proximity_map<collision_object_id>::statistics const*  dynamic_objects_proximity;
    };
    statistics const&  get_statistics() const { return m_statistics; }

private:

    void  release_data_of_erased_object(collision_object_id const  coid);

    void  update_shape_position(collision_object_id const  coid, matrix44 const&  from_base_matrix);

    void  insert_object(collision_object_id const  coid, bool const  is_dynamic)
    {
        if (is_dynamic) insert_dynamic_object(coid); else insert_static_object(coid);
    }
    void  insert_static_object(collision_object_id const  coid);
    void  insert_dynamic_object(collision_object_id const  coid);

    void  rebalance_static_proximity_map_if_needed() const;
    void  rebalance_dynamic_proximity_map_if_needed() const;

    bool  compute_contacts(
            collision_object_id_pair  cop,
            contact_acceptor const&  contact_acceptor_,
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


    mutable proximity_map<collision_object_id>  m_proximity_static_objects;  ///< These do not collide amongst each other.
    mutable proximity_map<collision_object_id>  m_proximity_dynamic_objects; ///< These collide amongst each other, plus with
                                                                             ///< those in 'm_proximity_static_objects' map.
    std::unordered_set<collision_object_id>  m_dynamic_object_ids;
    mutable bool  m_does_proximity_static_need_rebalancing;
    mutable bool  m_does_proximity_dynamic_need_rebalancing;

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

        natural_32_bit  triangle_index;
        natural_32_bit  end_points_getter_index;

        natural_8_bit  edges_ignore_mask;
    };

    std::vector<triangle_geometry>  m_triangles_geometry;
    std::vector<axis_aligned_bounding_box>  m_triangles_bbox;
    std::vector<COLLISION_MATERIAL_TYPE>  m_triangles_material;
    std::vector<std::array<collision_object_id, 3U> >  m_triangles_neighbours_over_edges;
    std::vector<std::pair<std::function<vector3(natural_32_bit, natural_8_bit)>, natural_32_bit> >
            m_triangles_end_point_getters;
    std::vector<natural_32_bit>  m_triangles_indices_of_invalidated_end_point_getters;


    /////////////////////////////////////////////////////////////////////////////////

    statistics  m_statistics;
};


}

#endif
