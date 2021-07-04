#ifndef AI_NAVIGATION_HPP_INCLUDED
#   define AI_NAVIGATION_HPP_INCLUDED

#   include <ai/scene_binding.hpp>
#   include <ai/scene_binding.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/dynamic_array.hpp>
#   include <functional>
#   include <string>
#   include <array>
#   include <limits>
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
private:
    natural_32_bit  m_guid;
};


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
    navobj_guid  m_self;
    std::vector<navobj_guid>  m_links; // if m_self.is_waypoint2d(), then links are stored in CCW order.
};


struct  waylink
{
    std::array<navobj_guid, 2>  m_waypoints;
    float_32_bit  m_length;
};


struct  navlink
{
    waylink  m_waylink;
    std::array<navobj_guid, 2>  m_components;   // Elements of the array correspond to elements of m_waylink.m_waypoints.
};


struct  navcomponent
{
    bool  valid(navobj_guid const  guid) const;

private:
    friend struct  ai::naveditor;

    dynamic_array<waypoint>  m_waypoints_2d;
    dynamic_array<waypoint>  m_waypoints_3d;
    dynamic_array<waylink>  m_waylinks;

    com::object_guid  m_frame_guid;
};


struct  navsystem
{
    navsystem(simulation_context_const_ptr const  context_);

private:
    friend struct  ai::naveditor;

    dynamic_array<navcomponent>  m_components;
    dynamic_array<navlink>  m_navlinks;

    simulation_context_const_ptr  m_context;
};


using  navsystem_ptr = std::shared_ptr<navsystem>;
using  navsystem_const_ptr = std::shared_ptr<navsystem const>;


}

namespace ai {


struct  naveditor
{
    naveditor(navsystem_ptr const  navsystem_);

private:
    navsystem_ptr  m_navsystem;

    simulation_context_const_ptr  m_context;
};


using  naveditor_ptr = std::shared_ptr<naveditor>;


}

#endif
