#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_BOOL_LOCK_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_BOOL_LOCK_HPP_INCLUDED

namespace window_tabs { namespace tab_scene {


struct  lock_bool
{
    lock_bool(bool* ptr)
        : m_ptr(ptr)
        , m_old_value()
    {
        if (m_ptr != nullptr)
        { 
            m_old_value = *m_ptr;
            *m_ptr = true;
        }
    }
    
    ~lock_bool()
    {
        if (m_ptr != nullptr)
            *m_ptr = m_old_value;
    }
private:
    bool* m_ptr;
    bool  m_old_value;
};


}}

#endif
