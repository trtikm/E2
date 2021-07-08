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

    struct  const_iterator
    {
        using value_type      = element_type;
        using difference_type = element_index;
        using pointer         = value_type const*;
        using reference       = value_type const&;

        using container_type        = indices_set;
        using indices_iterator_type = typename container_type::const_iterator;

        const_iterator() noexcept : indices_it(), data(nullptr) {}
        const_iterator(indices_iterator_type const it, data_vector const* const  data_) noexcept : indices_it(it), data(data_) {}

        reference  operator*() const { return data->at(index()); }

        const_iterator&  operator++() { ++indices_it; return *this; }
        const_iterator  operator++(int) { const_iterator  tmp = *this; ++indices_it; return tmp; }

        bool  operator==(const_iterator const&  other) const { return indices_it == other.indices_it && data == other.data; }
        bool  operator!=(const_iterator const&  other) const { return !(*this == other); }

        element_index  index() const { return *indices_it; }

    private:

        indices_iterator_type  indices_it;
        data_vector const*  data;
    };

    element_index  insert(element_type const&  value = element_type());
    void  erase(element_index const  idx) { at(idx) = element_type(); m_valid_indices.erase(idx); m_free_indices.insert(idx); }
    void  clear() { m_data.clear(); m_valid_indices.clear(); m_free_indices.clear(); }

    bool  empty() const { return m_valid_indices.empty(); }

    bool  valid(element_index const  idx) const { return m_valid_indices.count(idx) != 0UL; }

    element_type const&  at(element_index const  idx) const { return m_data.at(idx); }
    element_type&  at(element_index const  idx) { return m_data.at(idx); }

    const_iterator  begin() const { return const_iterator(m_valid_indices.begin(), &m_data); }
    const_iterator  end() const { return const_iterator(m_valid_indices.end(), &m_data); }

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
