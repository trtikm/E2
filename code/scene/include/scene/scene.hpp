#ifndef E2_SCENE_SCENE_HPP_INCLUDED
#   define E2_SCENE_SCENE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <angeo/axis_aligned_bounding_box.hpp>
#   include <qtgl/batch.hpp>
#   include <utility/assumptions.hpp>
#   include <boost/any.hpp>
#   include <unordered_map>
#   include <unordered_set>
#   include <vector>
#   include <string>
#   include <functional>
#   include <memory>

namespace scn {


struct scene;


/**
 * The string represents a unique identifier of the node in the scene.
 */
using  scene_node_name = std::string;


/**
 * A node in a hierarchy of reference frames (i.e. coordinate systems) in 3D space.
 * A node has at most 1 parent node and any number of child nodes.
 * Each node can further hold data, called 'records', which must live in that frame.
 * A record can be of any type. Records can be logically arranged in so called 'folders'
 * inside a node. Each record has a unique name inside its folder. Each folder has
 * a unique name inside the node. A folder cannot contain other folders, only records.
 * Technically, since a record can be of any type, it can thus be a folder. Nevertheless,
 * scene_node's interface will ignore that (i.e. the interface assumes there are only
 * normal records in a folder). The empty string "" is a valid folder name. You can
 * think of records inside this folder as being directly in the node (although they are
 * in fact in that folder). The folder "" is traited is 'default' folder (i.e. for records
 * you do not have any folder).
 */
struct scene_node final
{
    using  scene_node_ptr = std::shared_ptr<scene_node>;

    using  record_name = std::string;
    using  record_holder = boost::any;
    using  record_bbox_getter = std::function<void(angeo::axis_aligned_bounding_box&)>;

    using  folder_name = std::string;

    struct  folder_content final
    {
        using  records_map = std::unordered_map<record_name, record_holder>;
        using  bbox_getters_map = std::unordered_map<record_name, record_bbox_getter>;

        records_map::const_iterator  find_record_holder(record_name const&  name) const { return m_records.find(name); }
        records_map::iterator  find_record_holder(record_name const&  name) { return m_records.find(name); }
        bbox_getters_map::const_iterator  find_bbox_getter(record_name const&  name) const { return m_bbox_getters.find(name); }
        bbox_getters_map::iterator  find_bbox_getter(record_name const&  name) { return m_bbox_getters.find(name); }

        template<typename TRecordValueType>
        void  insert_record(record_name const&  name, TRecordValueType const&  value)
        {
            ASSUMPTION(m_records.count(name) == 0UL);
            m_records.insert({name, record_holder(value)});
        }

        template<typename TRecordValueType>
        void  insert_record(record_name const&  name,
                            TRecordValueType const&  value,
                            record_bbox_getter const&  bbox_getter)
        {
            ASSUMPTION(m_records.count(name) == 0UL);
            m_records.insert({name, record_holder(value)});
            m_bbox_getters.insert({name, bbox_getter});
        }

        template<typename TRecordValueType>
        void  insert_record(record_name const&  name,
                            TRecordValueType const&  value,
                            angeo::axis_aligned_bounding_box const&  bbox)
        {
            ASSUMPTION(m_records.count(name) == 0UL);
            m_records.insert({name, record_holder(value)});
            m_bbox_getters.insert({name, [bbox](angeo::axis_aligned_bounding_box& out_bbox) -> void { out_bbox = bbox; }});
        }

        void  erase_record(record_name const&  name)
        {
            ASSUMPTION(m_records.count(name) != 0UL);
            m_records.erase(name);
            m_bbox_getters.erase(name);
        }

        void  insert_bbox_getter_for_record(record_name const&  name, record_bbox_getter const&  bbox_getter)
        {
            ASSUMPTION(m_records.count(name) != 0UL);
            m_bbox_getters[name] = bbox_getter;
        }

        void  insert_bbox_for_record(record_name const&  name, angeo::axis_aligned_bounding_box const&  bbox)
        {
            ASSUMPTION(m_records.count(name) != 0UL);
            m_bbox_getters[name] = [bbox](angeo::axis_aligned_bounding_box& out_bbox) -> void { out_bbox = bbox; };
        }

        void  erase_bbox_getter_of_record(record_name const&  name)
        {
            m_bbox_getters.erase(name);
        }

        bool  get_bbox_in_local_coord_system(record_name const&  name, angeo::axis_aligned_bounding_box&  output_bbox) const
        {
            auto const it = find_bbox_getter(name);
            if (it == get_bbox_getters().cend())
                return false;
            it->second(output_bbox);
            return true;
        }

        records_map const&  get_records() const { return m_records; }
        bbox_getters_map const&  get_bbox_getters() const { return m_bbox_getters; }

    private:
        records_map  m_records;
        bbox_getters_map  m_bbox_getters;
    };

    using  folders = std::unordered_map<folder_name, folder_content>;

    using  children_map = std::unordered_map<scene_node_name, scene_node_ptr>;

    static scene_node_ptr  create(
        scene_node_name const&  name,
        vector3 const&  origin = vector3_zero(),
        quaternion const&  orientation = quaternion_identity()
        );

    scene_node(
        scene_node_name const&  name,
        vector3 const&  origin,
        quaternion const&  orientation
        );

    scene_node_name const&  get_name() const { return m_name; }

    void  foreach_child(std::function<void(scene_node_ptr)> const&  action);

    children_map const&  get_children() const { return m_children; }
    children_map::const_iterator  find_child(scene_node_name const&  name) const { return get_children().find(name); }
    bool  has_parent() const { return !m_parent.expired(); }
    scene_node_ptr  get_parent() const { return m_parent.lock(); }

    folders const&  get_folders() const { return m_folders; }
    folders::const_iterator  find_folder(folder_name const&  name = "") const { return m_folders.find(name); }
    folders::iterator  find_folder(folder_name const&  name = "") { return m_folders.find(name); }

    folders::iterator  insert_folder(folder_name const&  name) { return m_folders.insert({name, {}}).first; }
    void  erase_folder(folder_name const&  name) { m_folders.erase(name); }

    angeo::coordinate_system_const_ptr  get_coord_system() const { return m_coord_system; }
    void  translate(vector3 const&  shift) { angeo::translate(*m_coord_system, shift); invalidate_world_matrix(); }
    void  rotate(quaternion const&  rotation) { angeo::rotate(*m_coord_system, rotation); invalidate_world_matrix(); }
    void  set_origin(vector3 const&  new_origin) { m_coord_system->set_origin(new_origin); invalidate_world_matrix(); }
    void  set_orientation(quaternion const&  new_orientation) { m_coord_system->set_orientation(new_orientation); invalidate_world_matrix(); }
    void  relocate(vector3 const&  new_origin, quaternion const&  new_orientation)
    {
        m_coord_system->set_origin(new_origin); m_coord_system->set_orientation(new_orientation); invalidate_world_matrix();
    }

    matrix44 const&  get_world_matrix() const;

    void  invalidate_world_matrix();

private:

    static void  insert_children_to_parent(std::vector<scene_node_ptr> const&  children, scene_node_ptr const  parent);
    static void  erase_children_from_parent(std::vector<scene_node_ptr> const&  children, scene_node_ptr const  parent);

    scene_node_name  m_name;
    angeo::coordinate_system_ptr  m_coord_system;
    folders  m_folders;
    children_map  m_children;
    std::weak_ptr<scene_node>  m_parent;
    mutable matrix44  m_world_matrix;
    mutable bool  m_is_world_matrix_valid;

    friend struct scene;
};

using scene_node_ptr = scene_node::scene_node_ptr;
using scene_node_const_ptr = std::shared_ptr<scene_node const>;


struct scene final
{
    std::unordered_map<scene_node_name, scene_node_ptr> const&  get_root_nodes() const { return m_scene; }
    std::unordered_map<scene_node_name, scene_node_ptr> const&  get_all_scene_nodes() const { return m_names_to_nodes; }

    /// Returns nullptr if there is no such node.
    scene_node_ptr  get_scene_node(scene_node_name const&  name) const;

    scene_node_ptr  insert_scene_node(
        scene_node_name const&  name,
        vector3 const&  origin = vector3_zero(),
        quaternion const&  orientation = quaternion_identity(),
        scene_node_ptr const  parent = nullptr
        );

    void  erase_scene_node(scene_node_name const&  name);

    void  insert_children_to_parent(std::vector<scene_node_ptr> const&  children, scene_node_ptr const  parent)
    {
        ASSUMPTION(get_scene_node(parent->get_name()) == parent);
        scene_node::insert_children_to_parent(children, parent);
    }

    void  erase_children_from_parent(std::vector<scene_node_ptr> const&  children, scene_node_ptr const  parent)
    {
        ASSUMPTION(get_scene_node(parent->get_name()) == parent);
        scene_node::erase_children_from_parent(children, parent);
    }

    bool is_direct_parent_and_child(scene_node_ptr const  parent, scene_node_ptr const  child);
    bool is_parent_and_child(scene_node_ptr const  parent, scene_node_ptr const  child);

    void  clear();

private:
    std::unordered_map<scene_node_name, scene_node_ptr>  m_scene;
    std::unordered_map<scene_node_name, scene_node_ptr>  m_names_to_nodes;
};

using scene_ptr = std::shared_ptr<scene>;
using scene_const_ptr = std::shared_ptr<scene const>;


}

#endif
