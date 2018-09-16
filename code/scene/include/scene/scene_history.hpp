#ifndef E2_SCENE_SCENE_HISTORY_HPP_INCLUDED
#   define E2_SCENE_SCENE_HISTORY_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <boost/filesystem/path.hpp>
#   include <memory>
#   include <string>
#   include <vector>
#   include <functional>

namespace scn {


struct  scene_history_node
{
    virtual  ~scene_history_node() {}

    virtual void  undo() const = 0;
    virtual void  redo() const = 0;

    virtual bool  is_mutator() const { return true; }
};

using  scene_history_node_ptr = std::shared_ptr<scene_history_node const>;


template<typename T>
struct  scene_history_node_with_undo_and_redo_processors : scene_history_node
{
    scene_history_node_with_undo_and_redo_processors(bool const  is_inverse_operation)
        : m_is_inverse_operation(is_inverse_operation)
    {}

    using  history_record_type = T;
    using  processor_type = std::function<void(history_record_type const&)>;

    static void  set_undo_processor(processor_type const&  processor) { s_undo_processor = processor; }
    static void  set_redo_processor(processor_type const&  processor) { s_redo_processor = processor; }

    processor_type const&  get_undo_processor() const { return represents_inverse_operation() ? s_redo_processor : s_undo_processor; }
    processor_type const&  get_redo_processor() const { return represents_inverse_operation() ? s_undo_processor : s_redo_processor; }

    bool  represents_inverse_operation() const { return m_is_inverse_operation; }

    void  undo() const override { get_undo_processor()(*dynamic_cast<history_record_type const*>(this)); }
    void  redo() const override { get_redo_processor()(*dynamic_cast<history_record_type const*>(this)); }

private:
    static processor_type  s_undo_processor;
    static processor_type  s_redo_processor;

    bool  m_is_inverse_operation;
};

template<typename T>
std::function<void(T const&)>  scene_history_node_with_undo_and_redo_processors<T>::s_undo_processor;

template<typename T>
std::function<void(T const&)>  scene_history_node_with_undo_and_redo_processors<T>::s_redo_processor;


struct  scene_history_coord_system_insert final :
    public scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_insert>
{
    using super_type = scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_insert>;

    scene_history_coord_system_insert(
            std::string const&  name,
            vector3 const&  origin,
            quaternion const&  orientation,
            std::string const&  parent,  // i.e. pass empty string if there is no parent
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

    std::string const&  get_name() const { return m_name; }
    vector3 const&  get_origin() const { return m_origin; }
    quaternion const&  get_orientation() const { return m_orientation; }
    std::string const&  get_parent_name() const { return m_parent; }

private:
    std::string  m_name;
    vector3  m_origin;
    quaternion  m_orientation;
    std::string  m_parent;
};


struct  scene_history_coord_system_relocate final :
    public scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_relocate>
{
    using super_type = scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_relocate>;

    scene_history_coord_system_relocate(
            std::string const&  name,
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

    std::string const&  get_name() const { return m_name; }
    vector3 const&  get_old_origin() const { return m_old_origin; }
    quaternion const&  get_old_orientation() const { return m_old_orientation; }
    vector3 const&  get_new_origin() const { return m_new_origin; }
    quaternion const&  get_new_orientation() const { return m_new_orientation; }

private:
    std::string  m_name;
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
            std::string const&  name,
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'erase_from_selection'
            )
        : super_type(as_inverse_operation)
        , m_name(name)
    {
        ASSUMPTION(!name.empty());
    }

    std::string const&  get_name() const { return m_name; }

    bool  is_mutator() const override { return false; }

private:
    std::string  m_name;
};


struct  scene_history_batch_insert final :
    public scene_history_node_with_undo_and_redo_processors<scene_history_batch_insert>
{
    using super_type = scene_history_node_with_undo_and_redo_processors<scene_history_batch_insert>;

    scene_history_batch_insert(
            std::pair<std::string, std::string> const&  name,
            boost::filesystem::path const&  batch_pathname,
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'erase'
            )
        : super_type(as_inverse_operation)
        , m_name(name)
        , m_batch_pathname(batch_pathname)
    {
        ASSUMPTION(!name.first.empty() && !name.second.empty());
    }

    std::pair<std::string, std::string> const&  get_name() const { return m_name; }
    std::string const&  get_coord_system_name() const { return m_name.first; }
    std::string const&  get_batch_name() const { return m_name.second; }
    boost::filesystem::path const&  get_batch_pathname() const { return m_batch_pathname; }

private:
    std::pair<std::string, std::string>  m_name;
    boost::filesystem::path  m_batch_pathname;
};


struct  scene_history_batch_insert_to_selection final :
    public scene_history_node_with_undo_and_redo_processors<scene_history_batch_insert_to_selection>
{
    using super_type = scene_history_node_with_undo_and_redo_processors<scene_history_batch_insert_to_selection>;

    scene_history_batch_insert_to_selection(
            std::pair<std::string, std::string> const&  name,
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'erase_from_selection'
            )
        : super_type(as_inverse_operation)
        , m_name(name)
    {
        ASSUMPTION(!name.first.empty() && !name.second.empty());
    }

    std::pair<std::string, std::string> const&  get_name() const { return m_name; }
    std::string const&  get_coord_system_name() const { return m_name.first; }
    std::string const&  get_batch_name() const { return m_name.second; }

    bool  is_mutator() const override { return false; }

private:
    std::pair<std::string, std::string>  m_name;
};


struct  scene_history final
{
    scene_history();

    void  insert(scene_history_node_ptr const  node_ptr);

    template<typename scene_history_node_type, typename... arg_types>
    void  insert(arg_types... args_for_constructor_of_history_node)
    {
        insert(scene_history_node_ptr(new scene_history_node_type(args_for_constructor_of_history_node...)));
    }

    void  commit();
    natural_64_bit  get_active_commit_id() const;
    bool  was_applied_mutator_since_commit(natural_64_bit const  commit_id) const;

    void  undo();
    void  redo();

    void  clear();
    bool  empty() const { return m_active_commit == 0UL; }

private:

    /// Returns the index of the commit in 'm_history' vector.
    std::size_t  find_commit(natural_64_bit const  commit_id) const;

    std::vector<scene_history_node_ptr>  m_history;
    std::size_t  m_active_commit;
};

using scene_history_ptr = std::shared_ptr<scene_history>;
using scene_history_const_ptr = std::shared_ptr<scene_history const>;


inline constexpr natural_64_bit  get_invalid_scene_history_commit_id() noexcept { return 0ULL; }


}

#endif
