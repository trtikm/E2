#ifndef UTILITY_ARRAY_OF_DERIVED_HPP_INCLUDED
#   define UTILITY_ARRAY_OF_DERIVED_HPP_INCLUDED

#include <utility/type_envelope.hpp>
#include <utility/timeprof.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <type_traits>
#include <memory>
#include <algorithm>


template<typename B>
struct array_of_derived
{
    using  base_type = B;

    struct iterator
    {
        // TODO!
        natural_8_bit*  m_pointer;
        natural_64_bit  m_shift;
    };

    struct const_iterator
    {
        // TODO!
        natural_8_bit const*  m_pointer;
        natural_64_bit  m_shift;
    };

    template<typename derived_type, typename... arg_types>
    explicit array_of_derived(natural_64_bit const  num_elements, type_envelope<derived_type>, arg_types... args);

    ~array_of_derived();

    array_of_derived(array_of_derived<base_type>&&  other);

    void  swap(array_of_derived<base_type>&  other);

    natural_64_bit  size() const noexcept { return m_num_elements; }
    natural_64_bit  element_size() const noexcept { return m_num_bytes_per_element; }

    base_type const&  at(natural_64_bit const  element_index) const { return *get_element_ptr(element_index); }
    base_type&  at(natural_64_bit const  element_index) { return *get_element_ptr(element_index); }

private:

    array_of_derived() = delete;
    array_of_derived(array_of_derived const&) = delete;
    array_of_derived&  operator=(array_of_derived const&) = delete;

    base_type*  get_element_ptr(natural_64_bit const  element_index) noexcept
    {
        return reinterpret_cast<base_type*>(get_raw_element_ptr(element_index));
    }

    natural_8_bit*  get_raw_element_ptr(natural_64_bit const  element_index)
    {
        ASSUMPTION(element_index < m_num_elements);
        return m_data + (element_index * m_num_bytes_per_element);
    }

    natural_64_bit  m_num_bytes_per_element;
    natural_64_bit  m_num_elements;
    natural_8_bit*  m_data;
};


template<typename B>
template<typename derived_type, typename... arg_types>
array_of_derived<B>::array_of_derived(natural_64_bit const  num_elements, type_envelope<derived_type>, arg_types... args)
    : m_num_bytes_per_element(sizeof(derived_type))
    , m_num_elements(num_elements)
    , m_data(m_num_bytes_per_element * m_num_elements == 0UL ? nullptr : new natural_8_bit[m_num_bytes_per_element * m_num_elements])
{
    static_assert(std::is_base_of<base_type, derived_type>::value, "");
    static_assert(std::has_virtual_destructor<base_type>::value, "");
    ASSUMPTION(m_num_bytes_per_element > 0UL);
    for (natural_64_bit i = 0UL; i < m_num_elements; ++i)
        new((void*)get_raw_element_ptr(i)) derived_type(args...);
}


template<typename B>
array_of_derived<B>::~array_of_derived()
{
    for (natural_64_bit i = 0UL; i < m_num_elements; ++i)
        at(i).~base_type();
    delete[] m_data;
}


template<typename B>
array_of_derived<B>::array_of_derived(array_of_derived<B>&&  other)
    : m_num_bytes_per_element(other.m_num_bytes_per_element)
    , m_num_elements(other.m_num_elements)
    , m_data(other.m_data)
{
    other.m_num_bytes_per_element = 1UL;
    other.m_num_elements = 0UL;
    other.m_data = nullptr;
}


template<typename B>
void  array_of_derived<B>::swap(array_of_derived<B>&  other)
{
    std::swap(m_num_bytes_per_element,other.m_num_bytes_per_element);
    std::swap(m_num_elements, other.m_num_elements);
    std::swap(m_data, other.m_data);
}


template<typename base_type, typename derived_type, typename... arg_types_for_constructor_of_derived>
std::unique_ptr< array_of_derived<base_type> >  make_array_of_derived(
    natural_64_bit const  num_elements,
    arg_types_for_constructor_of_derived... args
    )
{
    return std::unique_ptr< array_of_derived<base_type> >( new array_of_derived<base_type>(
        num_elements,
        type_envelope<derived_type>(),
        args...
        ));
}


#endif
