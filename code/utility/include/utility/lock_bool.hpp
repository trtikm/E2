#ifndef UTILITY_LOCK_BOOL_HPP_INCLUDED
#   define UTILITY_LOCK_BOOL_HPP_INCLUDED


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


#   define LOCK_BOOL_BLOCK_BEGIN(BOOL_VAR) if (!(BOOL_VAR)) { lock_bool const ____lock_of_lock_bool_variable__(&(BOOL_VAR));
#   define LOCK_BOOL_BLOCK_END() }


#endif
