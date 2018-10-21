#ifndef E2_SCENE_SCENE_HISTORY_NODES_DEFAULT_HPP_INCLUDED
#   define E2_SCENE_SCENE_HISTORY_NODES_DEFAULT_HPP_INCLUDED

#   include <scene/scene_history_node.hpp>
#   include <scene/scene_record_id.hpp>
#   include <angeo/tensor_math.hpp>

namespace scn {


struct  scene_history_coord_system_insert final :
    public scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_insert>
{
    using super_type = scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_insert>;

    scene_history_coord_system_insert(
            scene_node_name const&  name,
            vector3 const&  origin,
            quaternion const&  orientation,
            scene_node_name const&  parent,  // i.e. pass empty string if there is no parent
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'erase'
            )
        : super_type(as_inverse_operation)
        , m_name(name)
        , m_origin(origin)
        , m_orientation(orientation)
        , m_parent(parent)
    {
        ASSUMPTION(!name.empty());
    }

    scene_node_name const&  get_name() const { return m_name; }
    vector3 const&  get_origin() const { return m_origin; }
    quaternion const&  get_orientation() const { return m_orientation; }
    scene_node_name const&  get_parent_name() const { return m_parent; }

private:
    scene_node_name  m_name;
    vector3  m_origin;
    quaternion  m_orientation;
    scene_node_name  m_parent;
};


struct  scene_history_coord_system_relocate final :
    public scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_relocate>
{
    using super_type = scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_relocate>;

    scene_history_coord_system_relocate(
            scene_node_name const&  name,
            vector3 const&  old_origin,
            quaternion const&  old_orientation,
            vector3 const&  new_origin,
            quaternion const&  new_orientation
            )
        : super_type(false)
        , m_name(name)
        , m_old_origin(old_origin)
        , m_old_orientation(old_orientation)
        , m_new_origin(new_origin)
        , m_new_orientation(new_orientation)
    {
        ASSUMPTION(!name.empty());
    }

    scene_node_name const&  get_name() const { return m_name; }
    vector3 const&  get_old_origin() const { return m_old_origin; }
    quaternion const&  get_old_orientation() const { return m_old_orientation; }
    vector3 const&  get_new_origin() const { return m_new_origin; }
    quaternion const&  get_new_orientation() const { return m_new_orientation; }

private:
    scene_node_name  m_name;
    vector3  m_old_origin;
    quaternion  m_old_orientation;
    vector3  m_new_origin;
    quaternion  m_new_orientation;
};


struct  scene_history_coord_system_insert_to_selection final :
    public scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_insert_to_selection>
{
    using super_type = scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_insert_to_selection>;

    scene_history_coord_system_insert_to_selection(
            scene_node_name const&  name,
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'erase_from_selection'
            )
        : super_type(as_inverse_operation)
        , m_name(name)
    {
        ASSUMPTION(!name.empty());
    }

    scene_node_name const&  get_name() const { return m_name; }

    bool  is_mutator() const override { return false; }

private:
    scene_node_name  m_name;
};


template<typename T>
struct  scene_history_record_insert :
    public scene_history_node_with_undo_and_redo_processors<T>
{
    using child_type = T;
    using super_type = scene_history_node_with_undo_and_redo_processors<child_type>;

    scene_history_record_insert(
            scene_record_id const&  id,
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'erase'
            )
        : super_type(as_inverse_operation)
        , m_id(id)
    {
        ASSUMPTION(!m_id.get_node_name().empty() && !m_id.get_record_name().empty());
    }

    scene_record_id const&  get_id() const { return m_id; }

private:
    scene_record_id  m_id;
};


struct  scene_history_record_insert_to_selection :
    public scene_history_node_with_undo_and_redo_processors<scene_history_record_insert_to_selection>
{
    using super_type = scene_history_node_with_undo_and_redo_processors<scene_history_record_insert_to_selection>;

    scene_history_record_insert_to_selection(
            scene_record_id const&  id,
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'erase_from_selection'
            )
        : super_type(as_inverse_operation)
        , m_id(id)
    {
        ASSUMPTION(!m_id.get_node_name().empty() && !m_id.get_record_name().empty());
    }

    scene_record_id const&  get_id() const { return m_id; }

    bool  is_mutator() const override { return false; }

private:
    scene_record_id  m_id;
};


}

#endif
