#ifndef E2_TOOL_GFXTUNER_SCENE_HISTORY_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_SCENE_HISTORY_HPP_INCLUDED

#   include <gfxtuner/scene.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <memory>
#   include <string>
#   include <vector>
#   include <functional>


struct  scene_history_node
{
    virtual  ~scene_history_node() {}

    virtual void  undo() const = 0;
    virtual void  redo() const = 0;
};

using  scene_history_node_ptr = std::shared_ptr<scene_history_node const>;


template<typename T>
struct  scene_history_node_with_undo_and_redo_processors : scene_history_node
{
    using  history_record_type = T;
    static void  set_undo_processor(std::function<void(history_record_type const&)> const&  processor)
    { s_undo_processor = processor; }
    static void  set_redo_processor(std::function<void(history_record_type const&)> const&  processor)
    { s_redo_processor = processor; }

    void  undo() const override { s_undo_processor(*dynamic_cast<history_record_type const*>(this)); }
    void  redo() const override { s_redo_processor(*dynamic_cast<history_record_type const*>(this)); }

private:
    static std::function<void(history_record_type const&)>  s_undo_processor;
    static std::function<void(history_record_type const&)>  s_redo_processor;
};

template<typename T>
std::function<void(T const&)>  scene_history_node_with_undo_and_redo_processors<T>::s_undo_processor;

template<typename T>
std::function<void(T const&)>  scene_history_node_with_undo_and_redo_processors<T>::s_redo_processor;


struct scene_history_coord_system_insert :
    public scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_insert>
{
    scene_history_coord_system_insert(
            std::string const&  name,
            vector3 const&  origin,
            quaternion const&  orientation,
            std::string const&  parent  // i.e. pass empty string if there is no parent
            )
        : m_name(name)
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


struct scene_history_coord_system_erase :
    public scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_erase>
{
    scene_history_coord_system_erase(
            std::string const&  name,
            vector3 const&  origin,
            quaternion const&  orientation,
            std::string const&  parent  // i.e. pass empty string if there is no parent
            )
        : m_name(name)
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


struct scene_history_coord_system_relocate :
    public scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_relocate>
{
    scene_history_coord_system_relocate(
            std::string const&  name,
            vector3 const&  origin,
            quaternion const&  orientation
            )
        : m_name(name)
        , m_origin(origin)
        , m_orientation(orientation)
    {
        ASSUMPTION(!name.empty());
    }

    std::string const&  get_name() const { return m_name; }
    vector3 const&  get_origin() const { return m_origin; }
    quaternion const&  get_orientation() const { return m_orientation; }

private:
    std::string  m_name;
    vector3  m_origin;
    quaternion  m_orientation;
};


struct scene_history_coord_system_insert_to_selection :
    public scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_insert_to_selection>
{
    scene_history_coord_system_insert_to_selection(std::string const&  name)
        : m_name(name)
    {
        ASSUMPTION(!name.empty());
    }

    std::string const&  get_name() const { return m_name; }

private:
    std::string  m_name;
};


struct scene_history_coord_system_erase_from_selection :
    public scene_history_node_with_undo_and_redo_processors<scene_history_coord_system_erase_from_selection>
{
    scene_history_coord_system_erase_from_selection(std::string const&  name)
        : m_name(name)
    {
        ASSUMPTION(!name.empty());
    }

    std::string const&  get_name() const { return m_name; }

private:
    std::string  m_name;
};


struct scene_history_batch_insert :
    public scene_history_node_with_undo_and_redo_processors<scene_history_batch_insert>
{
    scene_history_batch_insert(std::pair<std::string, std::string> const&  name)
        : m_name(name)
    {
        ASSUMPTION(!name.first.empty() && !name.second.empty());
    }

    std::pair<std::string, std::string> const&  get_name() const { return m_name; }
    std::string const&  get_coord_system_name() const { return m_name.first; }
    std::string const&  get_batch_name() const { return m_name.second; }

private:
    std::pair<std::string, std::string>  m_name;
};


struct scene_history_batch_erase :
    public scene_history_node_with_undo_and_redo_processors<scene_history_batch_erase>
{
    scene_history_batch_erase(std::pair<std::string, std::string> const&  name)
        : m_name(name)
    {
        ASSUMPTION(!name.first.empty() && !name.second.empty());
    }

    std::pair<std::string, std::string> const&  get_name() const { return m_name; }
    std::string const&  get_coord_system_name() const { return m_name.first; }
    std::string const&  get_batch_name() const { return m_name.second; }

private:
    std::pair<std::string, std::string>  m_name;
};


struct scene_history_batch_insert_to_selection :
    public scene_history_node_with_undo_and_redo_processors<scene_history_batch_insert_to_selection>
{
    scene_history_batch_insert_to_selection(std::pair<std::string, std::string> const&  name)
        : m_name(name)
    {
        ASSUMPTION(!name.first.empty() && !name.second.empty());
    }

    std::pair<std::string, std::string> const&  get_name() const { return m_name; }
    std::string const&  get_coord_system_name() const { return m_name.first; }
    std::string const&  get_batch_name() const { return m_name.second; }

private:
    std::pair<std::string, std::string>  m_name;
};


struct scene_history_batch_erase_from_selection :
    public scene_history_node_with_undo_and_redo_processors<scene_history_batch_erase_from_selection>
{
    scene_history_batch_erase_from_selection(std::pair<std::string, std::string> const&  name)
        : m_name(name)
    {
        ASSUMPTION(!name.first.empty() && !name.second.empty());
    }

    std::pair<std::string, std::string> const&  get_name() const { return m_name; }
    std::string const&  get_coord_system_name() const { return m_name.first; }
    std::string const&  get_batch_name() const { return m_name.second; }

private:
    std::pair<std::string, std::string>  m_name;
};


struct  scene_history
{
    static scene_history&  get_instance();

    void  insert(scene_history_node_ptr const  node_ptr);

    template<typename scene_history_node_type, typename... arg_types>
    void  insert(arg_types... args_for_constructor_of_history_node)
    {
        insert(scene_history_node_ptr(new scene_history_node_type(args_for_constructor_of_history_node...)));
    }

    void  commit();

    void  undo();
    void  redo();

private:
    scene_history();

    std::vector<scene_history_node_ptr>  m_history;
    std::size_t  m_active_commit;
};

inline scene_history&  get_scene_history() { return scene_history::get_instance(); }


#endif
