#include <ai/navigation.hpp>
#include <com/simulation_context.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <unordered_map>
#include <unordered_set>

namespace ai {


NAVOBJ_KIND  as_navobj_kind(natural_8_bit  const  kind)
{
    ASSUMPTION(kind < get_num_navobj_kinds());
    return (NAVOBJ_KIND)kind;
}


std::string  to_string(NAVOBJ_KIND const  kind)
{
    switch (kind)
    {
    case NAVOBJ_KIND::NAVCOMPONENT: return "NAVCOMPONENT";
    case NAVOBJ_KIND::WAYPOINT2D: return "WAYPOINT2D";
    case NAVOBJ_KIND::WAYPOINT3D: return "WAYPOINT3D";
    case NAVOBJ_KIND::WAYLINK: return "WAYLINK";
    case NAVOBJ_KIND::NONE: return "NONE";
    default:
        UNREACHABLE();
    }
}


NAVOBJ_KIND  read_navobj_kind_from_string(std::string const&  name)
{
    static std::unordered_map<std::string, NAVOBJ_KIND> const  map{
        {"NAVCOMPONENT", NAVOBJ_KIND::NAVCOMPONENT},
        {"WAYPOINT2D", NAVOBJ_KIND::WAYPOINT2D},
        {"WAYPOINT3D", NAVOBJ_KIND::WAYPOINT3D},
        {"WAYLINK", NAVOBJ_KIND::WAYLINK},
        {"NONE", NAVOBJ_KIND::NONE}
    };
    auto const  it = map.find(name);
    ASSUMPTION(it != map.cend());
    return it->second;
}


navobj_guid  navobj_guid::s_invalid_guid(NAVOBJ_KIND::NONE, 0U);


}

namespace ai {


navcomponent::navcomponent()
    : navcomponent({}, com::invalid_object_guid())
{}


navcomponent::navcomponent(angeo::coordinate_system const&  frame, com::object_guid const  collider_guid)
    : m_waypoints_2d()
    , m_waypoints_3d()
    , m_waylinks()

    , m_frame(frame)
    , m_collider_guid(collider_guid)
{}


waypoint const&  navcomponent::get_waypoint(navobj_guid const  nav_guid) const
{
    switch (nav_guid.kind())
    {
    case NAVOBJ_KIND::WAYPOINT2D: return m_waypoints_2d.at(nav_guid.index()); break;
    case NAVOBJ_KIND::WAYPOINT3D: return m_waypoints_3d.at(nav_guid.index()); break;
    default: UNREACHABLE(); break;
    }
}


waylink const&  navcomponent::get_waylink(navobj_guid const  nav_guid) const
{
    ASSUMPTION(nav_guid.is_waylink());
    return m_waylinks.at(nav_guid.index());
}


waypoint const&  navcomponent::get_waypoint(waylink const&  link, natural_32_bit const  idx) const
{
    return get_waypoint(link.get_waypoint_guid(idx));
}


waypoint&  navcomponent::waypoint_ref(waylink const&  link, natural_32_bit const  idx)
{
    return waypoint_ref(link.get_waypoint_guid(idx));
}


waypoint&  navcomponent::waypoint_ref(navobj_guid const  nav_guid)
{
    return const_cast<waypoint&>(get_waypoint(nav_guid));
}


waylink&  navcomponent::waylink_ref(navobj_guid const  nav_guid)
{
    ASSUMPTION(nav_guid.is_waylink());
    return m_waylinks.at(nav_guid.index());
}


navsystem::navsystem(simulation_context_const_ptr const  context_)
    : m_components()
    , m_navlinks()

    , m_colliders_to_components()
    , m_dynamic_components()

    , m_context(context_)
{
    ASSUMPTION(m_context != nullptr);
}


navcomponent const&  navsystem::get_component(navobj_guid const  nav_guid) const
{
    ASSUMPTION(nav_guid.is_navcomponent());
    return m_components.at(nav_guid.index());
}


navcomponent const&  navsystem::get_component(navlink const&  link, natural_32_bit const  idx) const
{
    return get_component(link.get_component_guid(idx));
}


navcomponent&  navsystem::component_ref(navobj_guid const  nav_guid)
{
    ASSUMPTION(nav_guid.is_navcomponent());
    return m_components.at(nav_guid.index());
}


navcomponent&  navsystem::component_ref(navlink const&  link, natural_32_bit const  idx)
{
    return component_ref(link.get_component_guid(idx));
}


navlink const&  navsystem::get_navlink(navobj_guid const  nav_guid) const
{
    ASSUMPTION(nav_guid.is_navlink());
    return m_navlinks.at(nav_guid.index());
}


navlink&  navsystem::navlink_ref(navobj_guid const  nav_guid)
{
    ASSUMPTION(nav_guid.is_navlink());
    return m_navlinks.at(nav_guid.index());
}


waypoint const& navsystem::get_waypoint(navlink const& link, natural_32_bit const  idx) const
{
    ai::navcomponent const&  component = get_component(link.get_component_guid(idx));
    return component.get_waypoint(link.link, idx);
}


waypoint& navsystem::waypoint_ref(navlink const& link, natural_32_bit const  idx)
{
    ai::navcomponent&  component = component_ref(link.get_component_guid(idx));
    return component.waypoint_ref(link.link, idx);
}


void  navsystem::clear()
{
    m_components.clear();
    m_navlinks.clear();
    m_colliders_to_components.clear();
    m_dynamic_components.clear();
}


}

namespace ai {


naveditor::naveditor(navsystem_ptr const  navsystem_)
    : m_navsystem(navsystem_)

    , m_config2d{
            0.25f,              // m_agent_roller_radius
            2.0f                // m_waypoint_separation
            }

    , m_on_navcomponent_updated([](navobj_guid){})

    , m_context(m_navsystem->m_context)
{}


navobj_guid  naveditor::add_navcomponent_2d(com::object_guid const  collider_guid)
{
    TMPROF_BLOCK();

    ASSUMPTION(m_context->is_valid_collider_guid(collider_guid) && m_navsystem->m_colliders_to_components.count(collider_guid) == 0UL);

    navobj_guid const  new_component_guid(
            NAVOBJ_KIND::NAVCOMPONENT,
            m_navsystem->m_components.insert(
                    navcomponent(m_context->frame_coord_system_in_world_space(m_context->frame_of_collider(collider_guid)), collider_guid)
                    )
            );
    m_navsystem->m_colliders_to_components.insert({ collider_guid, new_component_guid });
    if (m_context->collision_class_of(collider_guid) != angeo::COLLISION_CLASS::STATIC_OBJECT)
        m_navsystem->m_dynamic_components.insert(new_component_guid);
    add_waypoints2d_and_waylinks(new_component_guid);

    std::unordered_set<navobj_guid>  updated_components;
    add_navlinks(new_component_guid, &updated_components);
    for (navobj_guid  component_guid : updated_components)
        if (component_guid != new_component_guid)
            m_on_navcomponent_updated(component_guid);

    return new_component_guid;
}


void  naveditor::del_navcomponent_2d(com::object_guid const  collider_guid)
{
    TMPROF_BLOCK();

    auto const  it = m_navsystem->m_colliders_to_components.find(collider_guid);
    if (it != m_navsystem->m_colliders_to_components.end())
    {
        std::unordered_set<navobj_guid>  updated_components;
        del_navlinks(it->second, &updated_components);
        for (navobj_guid  component_guid : updated_components)
            if (component_guid != it->second)
                m_on_navcomponent_updated(component_guid);

        m_navsystem->m_components.erase(it->second.index());
        m_navsystem->m_dynamic_components.erase(it->second);
    }
}


void  naveditor::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    std::unordered_set<navobj_guid>  updated_components;
    std::unordered_set<com::object_guid> const&  relocated = m_context->relocated_frame_guids();
    for (navobj_guid  component_guid : m_navsystem->get_dynamic_components())
    {
        navcomponent&  component = m_navsystem->component_ref(component_guid);
        com::object_guid const  frame_guid = m_context->frame_of_collider(component.get_collider_guid());
        if (relocated.count(frame_guid) != 0UL)
        {
            del_navlinks(component_guid, &updated_components);
            component.m_frame = m_context->frame_coord_system_in_world_space(frame_guid);
            add_navlinks(component_guid, &updated_components);
        }
    }
    for (navobj_guid  component_guid : updated_components)
        m_on_navcomponent_updated(component_guid);
}


void  naveditor::add_waypoints2d_and_waylinks(navobj_guid const  component_guid)
{
    com::object_guid const  collider_guid = m_navsystem->get_component(component_guid).get_collider_guid();
    switch (m_context->collider_shape_type(collider_guid))
    {
    case angeo::COLLISION_SHAPE_TYPE::BOX:
        add_waypoints2d_and_waylinks_from_box(component_guid);
        break;
    default:
        return; // TODO: At least generation of navdata on triangle mesh should also be supported.
    }
}


void  naveditor::add_waypoints2d_and_waylinks_from_box(navobj_guid const  component_guid)
{
    TMPROF_BLOCK();

    navcomponent&  component = m_navsystem->component_ref(component_guid);
    bool const  is_static = !m_navsystem->is_dynamic_component(component_guid);
    vector3 const&  box_half_sizes = m_context->collider_box_half_sizes_along_axes(component.get_collider_guid());
}


void  naveditor::add_navlinks(navobj_guid const  component_guid, std::unordered_set<navobj_guid>* const  updated_components)
{
    TMPROF_BLOCK();

    // TODO!

    if (updated_components != nullptr)
        updated_components->insert(component_guid);
}


void  naveditor::del_navlinks(navobj_guid const  component_guid, std::unordered_set<navobj_guid>* const  updated_components)
{
    TMPROF_BLOCK();

    // TODO!

    if (updated_components != nullptr)
        updated_components->insert(component_guid);
}


void  naveditor::clear()
{
    // nothing to do, yet.
}


}
