#ifndef AI_PROPERTY_MAP_HPP_INCLUDED
#   define AI_PROPERTY_MAP_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <unordered_map>
#   include <string>

namespace ai {


struct property_map
{
    using  property_name = std::string;

    enum struct  PROPERTY_TYPE
    {
        INT = 0,
        FLOAT = 1,
        STRING = 2,
    };

    struct  property_type_and_value
    {
        property_type_and_value(integer_32_bit const  value);
        property_type_and_value(float_32_bit const  value);
        property_type_and_value(std::string const& value);

        PROPERTY_TYPE  get_type() const { return type; }

        integer_32_bit  get_int() const;
        float_32_bit  get_float() const;
        std::string const& get_string() const;

        void  set_int(integer_32_bit const  value);
        void  set_float(float_32_bit const  value);
        void  set_string(std::string const&  value);

    private:
        PROPERTY_TYPE type;
        integer_32_bit  value_int;
        float_32_bit  value_float;
        std::string  value_string;
    };

    using  map_type = std::unordered_map<property_name, property_type_and_value>;

    property_map()
        : m_map()
    {}

    property_map(map_type const&  props)
        : m_map(props)
    {}

    bool  empty() const { return get_map().empty(); }
    std::size_t  size() const { return get_map().size(); }

    property_type_and_value const&  at(property_name const&  name) const { return get_map().at(name); }
    property_type_and_value&  at(property_name const&  name) { return get_map().at(name); }

    map_type::const_iterator  find(property_name const& name) const { return get_map().find(name); }
    map_type::const_iterator  begin() const { return get_map().cbegin(); }
    map_type::const_iterator  end() const { return get_map().cend(); }

    map_type const&  get_map() const { return m_map; }
    map_type&  get_map() { return m_map; }

private:
    map_type  m_map;
};


boost::property_tree::ptree  as_ptree(property_map const&  map);
property_map  as_property_map(boost::property_tree::ptree const&  tree);


}

#endif
