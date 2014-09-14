#ifndef UTILITY_INSTANCE_WRAPPER_HPP_INCLUDED
#   define UTILITY_INSTANCE_WRAPPER_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/invariants.hpp>
#   include <new>


template<typename instance_type>
struct instance_wrapper
{
    instance_wrapper()
        : m_constructed(false)
    {}

    ~instance_wrapper()
    {
        destruct_instance();
    }

    template<typename argument_type>
    void construct_instance(argument_type const& argument)
    {
        destruct_instance();
        new(&m_memory[0]) instance_type(argument);
        m_constructed = true;
    }

    instance_type* operator->()
    {
        INVARIANT(m_constructed);
        return reinterpret_cast<instance_type*>(&m_memory[0]);
    }

    instance_type const* operator->() const
    {
        INVARIANT(m_constructed);
        return reinterpret_cast<instance_type const*>(&m_memory[0]);
    }

    instance_type& reference_to_instance()
    {
        return *this->operator->();
    }

    instance_type const& reference_to_instance() const
    {
        return *this->operator->();
    }

    operator instance_type() const
    {
        return reference_to_instance();
    }

private:

    void destruct_instance()
    {
        if (m_constructed)
        {
            reference_to_instance().~instance_type();
            m_constructed = false;
        }
    }

    natural_8_bit m_memory[sizeof(instance_type)];
    bool m_constructed;
};


#endif
