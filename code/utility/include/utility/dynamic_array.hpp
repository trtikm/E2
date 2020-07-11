#ifndef UTILITY_DYNAMIC_ARRAY_HPP_INCLUDED
#   define UTILITY_DYNAMIC_ARRAY_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <unordered_set>


template<typename T, typename I = natural_32_bit>
struct  dynamic_array
{
    using  element_type = T;
    using  element_index = I;
    using  data_vector = std::vector<element_type>;
    using  indices_set = std::unordered_set<element_index>;

    element_index  insert(element_type const&  value = element_type());
    void  erase(element_index const  idx) { at(idx) = element_type(); m_free_indices.erase(idx); m_free_indices.insert(idx); }
    void  clear() { m_data.clear(); m_valid_indices.clear(); m_free_indices.clear(); }

    bool  empty() const { return m_valid_indices.empty(); }

    bool  valid(element_index const  idx) const { return m_valid_indices.count(idx) != 0UL; }

    element_type const&  at(element_index const  idx) const { return m_data.at(idx); }
    element_type&  at(element_index const  idx) { return m_data.at(idx); }

    data_vector const&  data() const { return m_data; }
    indices_set const&  valid_indices() const { return m_valid_indices; }
    indices_set const&  free_indices() const { return m_free_indices; }

private:
    std::vector<element_type>  m_data;
    indices_set  m_valid_indices;
    indices_set  m_free_indices;
};


template<typename T, typename I>
typename dynamic_array<T,I>::element_index  dynamic_array<T,I>::insert(element_type const&  value)
{
    element_index  idx;
    if (m_free_indices.empty())
    {
        idx = (element_index)m_data.size();
        m_valid_indices.insert(idx);
        m_data.push_back(value);
    }
    else
    {
        auto const  it = m_free_indices.begin();
        idx = *it;
        m_free_indices.erase(it);
        m_valid_indices.insert(idx);
        m_data.at(idx) = value;
    }
    return idx;
}


#endif
