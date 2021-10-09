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
    : m_waypoints()
    , m_waylinks()

    , m_border_waypoints()

    , m_waypoints_proximity(
            [this](navobj_guid const  waypoint_guid) {
                vector3 const  lo = get_waypoint(waypoint_guid).position - vector3{0.01f, 0.01f, 0.01f};
                return angeo::point3_from_coordinate_system(lo, m_frame);
                },
            [this](navobj_guid const  waypoint_guid) {
                vector3 const  hi = get_waypoint(waypoint_guid).position + vector3{0.01f, 0.01f, 0.01f};
                return angeo::point3_from_coordinate_system(hi, m_frame);
                }
            )

    , m_frame(frame)
    , m_collider_guid(collider_guid)
{}


waypoint const&  navcomponent::get_waypoint(navobj_guid const  nav_guid) const
{
    return m_waypoints.at(nav_guid.index());
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
    return *m_components.at(nav_guid.index());
}


navcomponent const&  navsystem::get_component(navlink const&  link, natural_32_bit const  idx) const
{
    return get_component(link.get_component_guid(idx));
}


navcomponent&  navsystem::component_ref(navobj_guid const  nav_guid)
{
    ASSUMPTION(nav_guid.is_navcomponent());
    return *m_components.at(nav_guid.index());
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
            2.0f,               // m_waypoint_separation
            PI()/4.0f           // m_max_incline_angle
            }

    , m_on_navcomponent_updated([](navobj_guid){})

    , m_context(m_navsystem->m_context)
{}


void  naveditor::add_navcomponents_2d(com::object_guid const  collider_guid, std::vector<navobj_guid>* const  new_component_guids)
{
    TMPROF_BLOCK();

    ASSUMPTION(m_context->is_valid_collider_guid(collider_guid) && m_navsystem->m_colliders_to_components.count(collider_guid) == 0UL);

    std::unordered_set<navobj_guid>  updated_components;
    switch (m_context->collider_shape_type(collider_guid))
    {
    case angeo::COLLISION_SHAPE_TYPE::BOX:
        add_navcomponents_2d_from_box(collider_guid, new_component_guids, updated_components);
        break;
    default:
        return; // TODO: At least generation of navdata on triangle mesh should also be supported.
    }

    for (navobj_guid  component_guid : updated_components)
        m_on_navcomponent_updated(component_guid);
}


void  naveditor::del_navcomponents_2d(com::object_guid const  collider_guid)
{
    TMPROF_BLOCK();

    auto const  it = m_navsystem->m_colliders_to_components.find(collider_guid);
    if (it != m_navsystem->m_colliders_to_components.end())
    {
        std::unordered_set<navobj_guid>  updated_components;
        for (navobj_guid  component_guid : it->second)
            del_navlinks(component_guid, &updated_components);
        for (navobj_guid  component_guid : updated_components)
            if (it->second.count(component_guid) == 0UL)
                m_on_navcomponent_updated(component_guid);

        for (navobj_guid  component_guid : it->second)
        {
            m_navsystem->m_components.erase(component_guid.index());
            m_navsystem->m_dynamic_components.erase(component_guid);
        }
        m_navsystem->m_colliders_to_components.erase(it);
    }
}


void  naveditor::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    // TODO: Rewrite the algorithm so that navlinks are not recomputed when connected components
    //       do not move relatively to each other (i.e. they move with the same velocity).

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


navobj_guid  naveditor::create_empty_component(com::object_guid const  collider_guid)
{
    navobj_guid const  new_component_guid(
            NAVOBJ_KIND::NAVCOMPONENT,
            m_navsystem->m_components.insert(
                    std::make_shared<navcomponent>(
                            m_context->frame_coord_system_in_world_space(m_context->frame_of_collider(collider_guid)), collider_guid
                            )
                    )
            );
    m_navsystem->m_colliders_to_components[collider_guid].insert(new_component_guid);
    if (m_context->collision_class_of(collider_guid) != angeo::COLLISION_CLASS::STATIC_OBJECT)
        m_navsystem->m_dynamic_components.insert(new_component_guid);

    return  new_component_guid;
}


void  naveditor::add_navcomponents_2d_from_box(
        com::object_guid const  collider_guid,
        std::vector<navobj_guid>* const  new_component_guids,
        std::unordered_set<navobj_guid>&  updated_components
        )
{
    bool const  is_static = m_context->collision_class_of(collider_guid) == angeo::COLLISION_CLASS::STATIC_OBJECT;
    vector3 const&  box_half_sizes = m_context->collider_box_half_sizes_along_axes(collider_guid);
    angeo::coordinate_system_explicit const&  frame = m_context->frame_explicit_coord_system_in_world_space(m_context->frame_of_collider(collider_guid));

    struct  normal_axis_angle
    {
        vector3  normal;
        vector3  axis;
        float_32_bit  angle;
        vector3  half_sizes;
    } const  box_sides[6] = {
        {  vector3_unit_x(),  vector3_unit_y(), PI() / 2.0f, { box_half_sizes(2), box_half_sizes(1), box_half_sizes(0) } },
        { -vector3_unit_x(), -vector3_unit_y(), PI() / 2.0f, { box_half_sizes(2), box_half_sizes(1), box_half_sizes(0) } },
        {  vector3_unit_y(), -vector3_unit_x(), PI() / 2.0f, { box_half_sizes(0), box_half_sizes(2), box_half_sizes(1) } },
        { -vector3_unit_y(),  vector3_unit_x(), PI() / 2.0f, { box_half_sizes(0), box_half_sizes(2), box_half_sizes(1) } },
        {  vector3_unit_z(),  vector3_unit_x(), 0.0f       , { box_half_sizes(0), box_half_sizes(1), box_half_sizes(2) } },
        { -vector3_unit_z(),  vector3_unit_x(), PI()       , { box_half_sizes(0), box_half_sizes(1), box_half_sizes(2) } },
    };

    ASSUMPTION(m_config2d.m_max_incline_angle >= 0.0f && m_config2d.m_max_incline_angle <= PI() / 2.0f);
    float_32_bit const  cos_of_incline_angle = std::cosf(PI() - m_config2d.m_max_incline_angle);

    for (normal_axis_angle const&   side : box_sides)
    {
        if (is_static)
        {
            vector3 const   force_field_dir = -vector3_unit_z(); // TODO: We should read this vector from somewhere!
            if (dot_product(angeo::vector3_from_coordinate_system(side.normal, frame), force_field_dir) > cos_of_incline_angle)
                continue;
        }

        navobj_guid const  component_guid = create_empty_component(collider_guid);
        navcomponent&  component = m_navsystem->component_ref(component_guid);

        add_waypoints2d_and_waylinks_onto_xy_rectangle(component, side.half_sizes);

        matrix33 const  R = angle_axis_to_rotation_matrix(side.angle, side.axis);
        for (waypoint&  wp : component.m_waypoints)
            wp.position = R * wp.position;

        for (natural_32_bit  idx : component.m_waypoints.valid_indices())
            component.m_waypoints_proximity.insert(navobj_guid(NAVOBJ_KIND::WAYPOINT2D, idx));
        component.m_waypoints_proximity.rebalance();

        if (new_component_guids != nullptr)
            new_component_guids->push_back(component_guid);
    }
}


void  naveditor::add_waypoints2d_and_waylinks_onto_xy_rectangle(navcomponent&  component, vector3 const&  half_sizes)
{
    TMPROF_BLOCK();

    std::vector<std::vector<navobj_guid> >  waypoints_grid;
    {
        vector2 const  x_range(-half_sizes(0) + m_config2d.m_agent_roller_radius, half_sizes(0) - m_config2d.m_agent_roller_radius);
        vector2 const  y_range(-half_sizes(1) + m_config2d.m_agent_roller_radius, half_sizes(1) - m_config2d.m_agent_roller_radius);
        bool  x_shift = false;
        for (float_32_bit  y = y_range(0);
             y <= y_range(1) + (x_shift ? 1.0f : 2.0f) * m_config2d.m_waypoint_separation - 0.5f * m_config2d.m_agent_roller_radius;
             y += m_config2d.m_waypoint_separation, x_shift = !x_shift)
        {
            waypoints_grid.push_back({});
            for (float_32_bit  x = x_range(0)+ x_shift * m_config2d.m_waypoint_separation;
                 x <= x_range(1) + (x_shift ? 1.0f : 2.0f) * m_config2d.m_waypoint_separation - 0.5f * m_config2d.m_agent_roller_radius;
                 x += 2.0f * m_config2d.m_waypoint_separation)
            {
                //vector3  position{ x, y, half_sizes(2) + m_config2d.m_agent_roller_radius };
                vector3  position{ std::min(x, x_range(1)), std::min(y, y_range(1)), half_sizes(2) + m_config2d.m_agent_roller_radius };
                if (x_shift)
                {
                    if (x_shift && x > x_range(1))
                        position(0) = 0.5f * (std::max(x - m_config2d.m_waypoint_separation, x_range(0)) + std::min(x, x_range(1)));
                    if (y > y_range(1))
                        position(1) = 0.5f * (std::max(y - m_config2d.m_waypoint_separation, y_range(0)) + std::min(y, y_range(1)));
                }

                navobj_guid const  wp(
                        NAVOBJ_KIND::WAYPOINT2D,
                        component.m_waypoints.insert({
                                {}, // waylinks
                                position
                                })
                        );
                waypoints_grid.back().push_back(wp);
            }
        }
        if (waypoints_grid.empty())
            return;

        for (auto const&  wp : waypoints_grid.front())
            component.m_border_waypoints.insert(wp);
        for (auto const&  wp : waypoints_grid.back())
            component.m_border_waypoints.insert(wp);
        for (auto const&  wp_row : waypoints_grid)
            if (!wp_row.empty())
            {
                component.m_border_waypoints.insert(wp_row.front());
                component.m_border_waypoints.insert(wp_row.back());
            }
    }

    auto const&  insert_waylink =[this, &component, &waypoints_grid]
        (natural_32_bit const  i0, natural_32_bit const  j0, natural_32_bit const  i1, natural_32_bit const  j1) {
            INVARIANT(i0 < (natural_32_bit)waypoints_grid.size() && j0 < (natural_32_bit)waypoints_grid.at(i0).size());
            if (i1 < (natural_32_bit)waypoints_grid.size() && j1 < (natural_32_bit)waypoints_grid.at(i1).size())
                add_waylink(component, waypoints_grid.at(i0).at(j0), waypoints_grid.at(i1).at(j1));
        };

    for (natural_32_bit  i = 0U, m = (natural_32_bit)waypoints_grid.size(); i < m; ++i)
        for (natural_32_bit  j = 0U, n = (natural_32_bit)waypoints_grid.at(i).size(); j < n; ++j)
            if ((i & 1U) == 0U)
            {
                insert_waylink(i, j, i, j + 1U);
                insert_waylink(i, j, i + 1U, j);
                insert_waylink(i, j, i + 2U, j);
            }
            else
            {
                insert_waylink(i, j, i - 1U, j + 1U);
                insert_waylink(i, j, i + 1U, j);
                insert_waylink(i, j, i + 1U, j + 1U);
            }
}


navobj_guid  naveditor::add_waylink(navcomponent&  component, navobj_guid const  wp1_guid, navobj_guid const  wp2_guid)
{
    INVARIANT(wp1_guid.kind() == wp2_guid.kind() && wp1_guid.index() != wp2_guid.index());
    waypoint&  wp1 = component.waypoint_ref(wp1_guid);
    waypoint&  wp2 = component.waypoint_ref(wp2_guid);
    navobj_guid const   waylink_guid(
                NAVOBJ_KIND::WAYLINK,
                component.m_waylinks.insert({
                        { wp1_guid, wp2_guid }, // waypoints
                        length(wp1.position - wp2.position) // length
                        })
                );
    wp1.links.push_back(waylink_guid);
    wp2.links.push_back(waylink_guid);
    return waylink_guid;
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
