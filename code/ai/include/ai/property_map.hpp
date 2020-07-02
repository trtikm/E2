#ifndef AI_PROPERTY_MAP_HPP_INCLUDED
#   define AI_PROPERTY_MAP_HPP_INCLUDED

#   include <ai/scene_basic_types_binding.hpp>
#   include <angeo/tensor_math.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <utility/invariants.hpp>
#   include <memory>
#   include <unordered_map>
#   include <string>

namespace ai {


struct property_map
{
    using  property_name = std::string;

    enum struct  PROPERTY_TYPE
    {
        BOOL = 0,
        INT = 1,
        FLOAT = 2,
        STRING = 3,
    };

    struct  property_type_and_value
    {
        virtual ~property_type_and_value() {}
        virtual PROPERTY_TYPE  get_type() const { UNREACHABLE(); return PROPERTY_TYPE::BOOL; }
    };
    using  property_value_ptr = std::shared_ptr<property_type_and_value>;

    struct  bool_value : public property_type_and_value
    {
        bool_value(bool const  value_) : m_value(value_) {}
        ~bool_value() {}
        PROPERTY_TYPE  get_type() const override final { return PROPERTY_TYPE::BOOL; }
        bool  value() const { return m_value; }
        bool&  value_ref() { return m_value; }
    private:
        bool  m_value;
    };

    struct  int_value : public property_type_and_value
    {
        int_value(integer_32_bit const  value_) : m_value(value_) {}
        ~int_value() {}
        PROPERTY_TYPE  get_type() const override final { return PROPERTY_TYPE::INT; }
        integer_32_bit  value() const { return m_value; }
        integer_32_bit&  value_ref() { return m_value; }
    private:
        integer_32_bit  m_value;
    };

    struct  float_value : public property_type_and_value
    {
        float_value(float_32_bit const  value_) : m_value(value_) {}
        ~float_value() {}
        PROPERTY_TYPE  get_type() const override final { return PROPERTY_TYPE::FLOAT; }
        float_32_bit  value() const { return m_value; }
        float_32_bit&  value_ref() { return m_value; }
    private:
        float_32_bit  m_value;
    };

    struct  string_value : public property_type_and_value
    {
        string_value(std::string const& value_) : m_value(value_) {}
        ~string_value() {}
        PROPERTY_TYPE  get_type() const override final { return PROPERTY_TYPE::STRING; }
        std::string const&  value() const { return m_value; }
        std::string&  value_ref() { return m_value; }
    private:
        std::string  m_value;
    };

    using  map_type = std::unordered_map<property_name, property_value_ptr>;

    struct  default_config_record
    {
        property_value_ptr  value;
        bool  is_mandatory;
        std::string  description;
        natural_32_bit  edit_order;
    };
    using  default_config_records_map = std::unordered_map<property_map::property_name, default_config_record>;


    property_map()
        : m_map()
    {}

    property_map(map_type const&  props)
        : m_map(props)
    {}

    explicit property_map(default_config_records_map const&  cfg)
        : m_map()
    { reset(cfg); }

    bool  empty() const { return get_map().empty(); }
    std::size_t  size() const { return get_map().size(); }

    void  clear() { get_map().clear(); }
    void  reset(default_config_records_map const&  cfg);

    property_type_and_value const&  at(property_name const&  name) const { return *get_map().at(name); }

    map_type::const_iterator  find(property_name const&  name) const { return get_map().find(name); }
    map_type::const_iterator  begin() const { return get_map().cbegin(); }
    map_type::const_iterator  end() const { return get_map().cend(); }

    bool  has(property_name const&  name) const { return find(name) != end(); }
    bool  has_vector3(property_name const& name) const { return has(name + "_x") && has(name + "_y") && has(name + "_z"); }

    template<typename T> static inline T const&  as(property_type_and_value const&  value) { return *dynamic_cast<T const*>(&value); }
    template<typename T> static inline T&  as(property_type_and_value&  value) { return *dynamic_cast<T*>(&value); }

    static inline property_value_ptr  make_bool(bool const  value) { return std::make_shared<bool_value>(value); }
    static inline property_value_ptr  make_int(integer_32_bit const  value) { return std::make_shared<int_value>(value); }
    static inline property_value_ptr  make_float(float_32_bit const  value) { return std::make_shared<float_value>(value); }
    static inline property_value_ptr  make_string(std::string const&  value) { return std::make_shared<string_value>(value); }

    property_map  clone() const;

    PROPERTY_TYPE  get_type(property_name const&  name) const { return at(name).get_type(); }

    bool  get_bool(property_name const&  name) const { return as<bool_value>(at(name)).value(); }
    integer_32_bit  get_int(property_name const&  name) const { return as<int_value>(at(name)).value(); }
    float_32_bit  get_float(property_name const&  name) const { return as<float_value>(at(name)).value(); }
    std::string const&  get_string(property_name const&  name) const { return as<string_value>(at(name)).value(); }

    vector3  get_vector3() const { return vector3(get_float("x"), get_float("y"), get_float("z")); }
    vector3  get_vector3(property_name const&  name) const
    { return vector3(get_float(name + "_x"), get_float(name + "_y"), get_float(name + "_z")); }

    matrix33  get_matrix33() const { return row_vectors_to_matrix(get_vector3("0"), get_vector3("1"), get_vector3("2")); }
    matrix33  get_matrix33(property_name const&  name) const
    { return row_vectors_to_matrix(get_vector3(name + "_0"), get_vector3(name + "_1"), get_vector3(name + "_2")); }

    scene_node_id  get_scene_node_id(property_name const&  name, scene_node_id const&  base_id = scene_node_id()) const
    { return as_scene_node_id(get_string(name), base_id); }
    scene_record_id  get_scene_record_id(property_name const& name, scene_node_id const&  base_id = scene_node_id()) const
    { return as_scene_record_id(get_string(name), base_id); }

    bool&  get_bool_ref(property_name const&  name) { return as<bool_value>(at(name)).value_ref(); }
    integer_32_bit&  get_int_ref(property_name const&  name) { return as<int_value>(at(name)).value_ref(); }
    float_32_bit&  get_float_ref(property_name const&  name) { return as<float_value>(at(name)).value_ref(); }
    std::string&  get_string_ref(property_name const&  name) { return as<string_value>(at(name)).value_ref(); }

    void  set_bool(property_name const&  name, bool const  value) { get_map()[name] = make_bool(value); }
    void  set_int(property_name const&  name, integer_32_bit const  value) { get_map()[name] = make_int(value); }
    void  set_float(property_name const&  name, float_32_bit const  value) { get_map()[name] = make_float(value); }
    void  set_string(property_name const&  name, std::string const&  value) { get_map()[name] = make_string(value); }

    void  set(property_name const&  name, property_type_and_value const&  value);
    void  set(map_type::value_type const&  name_and_value) { set(name_and_value.first, *name_and_value.second); }
    void  set(property_name const& name, property_value_ptr const  value) { set(name, *value); }
    void  set_shared(property_name const& name, property_value_ptr const  value) { get_map()[name] = value; }

private:
    map_type const&  get_map() const { return m_map; }
    map_type&  get_map() { return m_map; }

    property_type_and_value&  at(property_name const& name) { return *get_map().at(name); }

    map_type  m_map;
};


}


std::string  as_string(ai::property_map::property_type_and_value const&  prop);
ai::property_map::property_value_ptr  as_property_map_value(
        ai::property_map::PROPERTY_TYPE const  type,
        std::string const&  value_text
        );

boost::property_tree::ptree  as_ptree(ai::property_map const&  map);
ai::property_map  as_property_map(boost::property_tree::ptree const&  tree);


#endif
