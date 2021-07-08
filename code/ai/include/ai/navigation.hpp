#ifndef AI_NAVIGATION_HPP_INCLUDED
#   define AI_NAVIGATION_HPP_INCLUDED

#   include <ai/scene_binding.hpp>
#   include <ai/scene_binding.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <utility/dynamic_array.hpp>
#   include <unordered_map>
#   include <unordered_set>
#   include <string>
#   include <array>
#   include <limits>
#   include <functional>
#   include <memory>

namespace ai {


enum struct  NAVOBJ_KIND : natural_8_bit
{
    NAVCOMPONENT        = 0U,
    NAVLINK             = 1U,

    WAYPOINT2D          = 2U,
    WAYPOINT3D          = 3U,
    WAYLINK             = 4U,

    NONE                = 5U,
};


constexpr natural_8_bit  get_num_navobj_kinds()
{
    return (natural_8_bit)NAVOBJ_KIND::NONE + (natural_8_bit)1;
}


inline natural_8_bit  as_number(NAVOBJ_KIND const  kind) { return (natural_8_bit)kind; }
NAVOBJ_KIND  as_navobj_kind(natural_8_bit  const  kind);
std::string  to_string(NAVOBJ_KIND const  kind);
NAVOBJ_KIND  read_navobj_kind_from_string(std::string const&  name);

struct  navobj_guid
{
    navobj_guid() : m_guid(0U) {}
    navobj_guid(NAVOBJ_KIND const  kind_, natural_32_bit const  index_)
        : m_guid((((natural_32_bit)as_number(kind_)) << 24U) | (index_ & 0xffffffU))
    {}
    bool  valid() const { return kind() != NAVOBJ_KIND::NONE; }
    NAVOBJ_KIND kind() const { return as_navobj_kind((natural_8_bit)(m_guid >> 24U)); }
    natural_32_bit index() const { return m_guid & 0xffffffU; }
    natural_32_bit code() const { return m_guid; }
    bool  is_navcomponent() const { return kind() == NAVOBJ_KIND::NAVCOMPONENT; }
    bool  is_navlink() const { return kind() == NAVOBJ_KIND::NAVLINK; }
    bool  is_waypoint() const { NAVOBJ_KIND const  k = kind(); return k == NAVOBJ_KIND::WAYPOINT2D || k == NAVOBJ_KIND::WAYPOINT3D; }
    bool  is_waypoint2d() const { return kind() == NAVOBJ_KIND::WAYPOINT2D; }
    bool  is_waypoint3d() const { return kind() == NAVOBJ_KIND::WAYPOINT3D; }
    bool  is_waylink() const { return kind() == NAVOBJ_KIND::WAYLINK; }
    static  navobj_guid  s_invalid_guid;
private:
    natural_32_bit  m_guid;
};


inline navobj_guid  invalid_navobj_guid() { return navobj_guid::s_invalid_guid; }


inline bool operator==(navobj_guid const  left, navobj_guid const  right) noexcept
{
    return left.code() == right.code();
}


inline bool operator!=(navobj_guid const  left, navobj_guid const  right) noexcept
{
    return left.code() != right.code();
}


inline bool operator<(navobj_guid const  left, navobj_guid const  right) noexcept
{
    return left.code() < right.code();
}


inline bool operator>(navobj_guid const  left, navobj_guid const  right) noexcept
{
    return right.code() > left.code();
}


inline bool operator<=(navobj_guid const  left, navobj_guid const  right) noexcept
{
    return left.code() <= right.code();
}


inline bool operator>=(navobj_guid const  left, navobj_guid const  right) noexcept
{
    return left.code() >= right.code();
}


}

namespace std {
template<> struct hash<ai::navobj_guid> { size_t operator()(ai::navobj_guid const&  id) const { return (size_t)id.code(); } };
}

namespace ai {


struct  waypoint;
struct  waylink;
struct  navcomponent;
struct  navlink;
struct  navsystem;
struct  naveditor;


struct  waypoint
{
    std::vector<navobj_guid>  links;
    vector3  position;
};


struct  waylink
{
    navobj_guid get_waypoint_guid(natural_32_bit const  idx) const { return waypoints.at(idx); }

    std::array<navobj_guid, 2>  waypoints;
    float_32_bit  length;
};


struct  navlink
{
    navobj_guid get_component_guid(natural_32_bit const  idx) const { return components.at(idx); }

    waylink  link;
    std::array<navobj_guid, 2>  components;   // Elements of the array correspond to elements of m_waylink.m_waypoints.
};


struct  navcomponent
{
    navcomponent();
    navcomponent(angeo::coordinate_system const&  frame, com::object_guid const  collider_guid);

    dynamic_array<waypoint> const&  get_waypoints_2d() const { return m_waypoints_2d; }
    dynamic_array<waypoint> const&  get_waypoints_3d() const { return m_waypoints_3d; }
    waypoint const&  get_waypoint(navobj_guid const  nav_guid) const;
    waypoint const&  get_waypoint(waylink const&  link, natural_32_bit const  idx) const;
    waypoint&  waypoint_ref(waylink const&  link, natural_32_bit const  idx);
    waypoint&  waypoint_ref(navobj_guid const  nav_guid);

    dynamic_array<waylink> const&  get_waylinks() const { return m_waylinks; }
    waylink const&  get_waylink(navobj_guid const  nav_guid) const;
    waylink&  waylink_ref(navobj_guid const  nav_guid);


    angeo::coordinate_system const&  get_frame() const { return m_frame; }
    com::object_guid  get_collider_guid() const { return m_collider_guid; }

    bool  valid(navobj_guid const  guid) const;

private:
    friend struct  ai::naveditor;

    dynamic_array<waypoint>  m_waypoints_2d;
    dynamic_array<waypoint>  m_waypoints_3d;
    dynamic_array<waylink>  m_waylinks;

    angeo::coordinate_system  m_frame;
    com::object_guid  m_collider_guid;
};


struct  navsystem
{
    navsystem(simulation_context_const_ptr const  context_);

    dynamic_array<navcomponent> const&  get_components() const { return m_components; }
    dynamic_array<navlink> const&  get_navlinks() const { return m_navlinks; }

    navcomponent const&  get_component(navobj_guid const  nav_guid) const;
    navcomponent const&  get_component(navlink const&  link, natural_32_bit const  idx) const;
    navcomponent&  component_ref(navobj_guid const  nav_guid);
    navcomponent&  component_ref(navlink const&  link, natural_32_bit const  idx);

    navlink const&  get_navlink(navobj_guid const  nav_guid) const;
    navlink&  navlink_ref(navobj_guid const  nav_guid);

    waypoint const&  get_waypoint(navlink const&  link, natural_32_bit const  idx) const;
    waypoint&  waypoint_ref(navlink const&  link, natural_32_bit const  idx);

    std::unordered_set<navobj_guid> const&  get_dynamic_components() const { return m_dynamic_components; }
    bool  is_dynamic_component(navobj_guid const  nav_guid) const { return m_dynamic_components.count(nav_guid) != 0UL; }

    simulation_context_const_ptr  get_scene_context() const { return m_context; }

    void  clear();

private:
    friend struct  ai::naveditor;

    dynamic_array<navcomponent>  m_components;
    dynamic_array<navlink>  m_navlinks;

    std::unordered_map<com::object_guid, navobj_guid>  m_colliders_to_components;
    std::unordered_set<navobj_guid>  m_dynamic_components;

    simulation_context_const_ptr  m_context;
};


using  navsystem_ptr = std::shared_ptr<navsystem>;
using  navsystem_const_ptr = std::shared_ptr<navsystem const>;


}

namespace ai {


struct  naveditor
{
    struct  config2d
    {
        float_32_bit  m_agent_roller_radius;    // An offset of a waypoint in z coordinate above the collider.
                                                // Also defines the distance from vertical walls.
                                                // must be >= 0.
        float_32_bit  m_waypoint_separation;    // An ideal distance between adjacent waypoints. Must be > 0.
    };

    using  callback_navcomponent_updated = std::function<void(navobj_guid)>;

    naveditor(navsystem_ptr const  navsystem_);

    navobj_guid  add_navcomponent_2d(com::object_guid const  collider_guid);
    void  del_navcomponent_2d(com::object_guid const  collider_guid);

    void  next_round(float_32_bit const  time_step_in_seconds);

    void  set_callback_navcomponent_updated(callback_navcomponent_updated const  fn) { m_on_navcomponent_updated = fn; }

    void  clear();

private:
    void  add_waypoints2d_and_waylinks(navobj_guid const  component_guid);
    void  add_waypoints2d_and_waylinks_from_box(navobj_guid const  component_guid);

    void  add_navlinks(navobj_guid const  component_guid, std::unordered_set<navobj_guid>* const  updated_components = nullptr);
    void  del_navlinks(navobj_guid const  component_guid, std::unordered_set<navobj_guid>* const  updated_components = nullptr);

    navsystem_ptr  m_navsystem;
    config2d  m_config2d;

    callback_navcomponent_updated  m_on_navcomponent_updated;

    simulation_context_const_ptr  m_context;
};


using  naveditor_ptr = std::shared_ptr<naveditor>;


}

#endif
