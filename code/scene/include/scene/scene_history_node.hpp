#ifndef E2_SCENE_SCENE_HISTORY_NODE_HPP_INCLUDED
#   define E2_SCENE_SCENE_HISTORY_NODE_HPP_INCLUDED

#   include <utility/invariants.hpp>
#   include <memory>
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


}

#endif
