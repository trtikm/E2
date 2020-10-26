#include <ai/action_controller.hpp>
#include <ai/agent.hpp>
#include <ai/skeleton_utils.hpp>
#include <angeo/collide.hpp>
#include <angeo/linear_segment_curve.hpp>
#include <com/simulation_context.hpp>
#include <utility/std_pair_hash.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <queue>
#include <functional>
#include <algorithm>

namespace ai { namespace {


vector3  read_vector3(boost::property_tree::ptree const&  ptree)
{
    return { ptree.get<float_32_bit>("x"), ptree.get<float_32_bit>("y"), ptree.get<float_32_bit>("z") };
}


vector3  read_aabb_half_size(boost::property_tree::ptree const&  ptree)
{
    vector3  u = read_vector3(ptree);
    ASSUMPTION(u(0) > 0.0f && u(1) > 0.0f && u(2) > 0.0f);
    return u;
}


void  insert_collider_to_context(
        com::simulation_context const&  ctx,
        com::object_guid const  under_folder_guid,
        angeo::COLLISION_SHAPE_TYPE const  shape_type,
        vector3 const&  aabb_half_size,
        angeo::COLLISION_MATERIAL_TYPE const  collision_material,
        angeo::COLLISION_CLASS const  collision_class
        )
{
    switch (shape_type)
    {
    case angeo::COLLISION_SHAPE_TYPE::BOX:
        ctx.request_insert_collider_box(
                under_folder_guid,
                com::to_string(com::OBJECT_KIND::COLLIDER),
                aabb_half_size,
                collision_material,
                collision_class
                );
        break;
    case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
        ctx.request_insert_collider_capsule(
                under_folder_guid,
                com::to_string(com::OBJECT_KIND::COLLIDER),
                max_coord(aabb_half_size) - min_coord(aabb_half_size),
                min_coord(aabb_half_size),
                collision_material,
                collision_class
                );
        break;
    case angeo::COLLISION_SHAPE_TYPE::SPHERE:
        ctx.request_insert_collider_sphere(
                under_folder_guid,
                com::to_string(com::OBJECT_KIND::COLLIDER),
                min_coord(aabb_half_size),
                collision_material,
                collision_class
                );
        break;
    default: { UNREACHABLE(); break; }
    }
}


com::object_guid  get_frame_guid_under_agent_folder(
        com::object_guid const  folder_guid_of_agent,
        std::string const&  relative_path_to_folder_containing_the_frame,
        com::simulation_context const&  ctx
        )
{
    com::object_guid const  folder_guid = ctx.from_relative_path(folder_guid_of_agent, relative_path_to_folder_containing_the_frame);
    com::object_guid const  frame_guid = ctx.folder_content(folder_guid).content.at(com::to_string(com::OBJECT_KIND::FRAME));
    return frame_guid;
}


}}


namespace ai {


action_execution_context::action_execution_context(agent* const  myself_)
    : myself(myself_)
    , animate()
    , look_at()
    , aim_at()
    , time_buffer(0.0f)
{}


motion_desire_props const&  action_execution_context::desire() const
{
    return myself->get_cortex().get_motion_desire_props();
}


agent_state_variables&  action_execution_context::state_variables() const
{
    return myself->state_variables_ref();
}


skeletal_motion_templates  action_execution_context::motion_templates() const
{
    return myself->get_motion_templates();
}


scene_binding const&  action_execution_context::binding() const
{
    return *myself->get_binding();
}


com::simulation_context const&  action_execution_context::ctx() const
{
    return *binding().context;
}




bool  agent_action::motion_object_config::operator==(motion_object_config const&  other) const
{
    return shape_type == other.shape_type &&
           are_equal_3d(aabb_half_size, other.aabb_half_size, 0.001f) &&
           collision_material == other.collision_material &&
           are_equal(mass_inverted, other.mass_inverted, 0.001f) &&
           are_equal_33(inertia_tensor_inverted, other.inertia_tensor_inverted, 0.001f) &&
           is_moveable == other.is_moveable
           ;
}


agent_action::agent_action(
        std::string const&  name_,
        boost::property_tree::ptree const&  ptree_,
        boost::property_tree::ptree const&  defaults_,
        action_execution_context_ptr const  context_
        )
    // CONSTANTS
    : NAME(name_)
    , DESIRE() // loaded below
    , EFFECTS() // loaded below
    , MOTION_TEMPLATE_NAME(ptree_.get<std::string>("MOTION_TEMPLATE_NAME"))
    , ONLY_INTERPOLATE_TO_MOTION_TEMPLATE(ptree_.get<bool>("ONLY_INTERPOLATE_TO_MOTION_TEMPLATE"))
    , IS_CYCLIC(ptree_.get<bool>("IS_CYCLIC"))
    , IS_LOOK_AT_ENABLED(ptree_.get<bool>("IS_LOOK_AT_ENABLED"))
    , IS_AIM_AT_ENABLED(ptree_.get<bool>("IS_AIM_AT_ENABLED"))
    , USE_GHOST_OBJECT_FOR_SKELETON_LOCATION(ptree_.get<bool>("USE_GHOST_OBJECT_FOR_SKELETON_LOCATION"))
    , SENSORS() // loaded below
    , TRANSITIONS() // loaded below
    , MOTION_OBJECT_CONFIG() // loaded below
    // MUTABLE DATA
    , m_context(context_)
    , m_start_time(0.0f)
    , m_end_time(0.0f)
    , m_end_ghost_time(0.0f)
    , m_current_time(0.0f)
    , m_ghost_object_start_coord_system()
    , m_animation{0.0f, 0U, 0U}
{
    ASSUMPTION(motion_templates().motions_map().count(MOTION_TEMPLATE_NAME) != 0UL);
    load_desire(
            ptree_.find("DESIRE")->second,
            defaults_.count("DESIRE") == 0UL ?
                    boost::property_tree::ptree() : defaults_.find("DESIRE")->second
            );
    load_effects(
            ptree_.find("EFFECTS")->second,
            defaults_.count("EFFECTS") == 0UL ?
                    boost::property_tree::ptree() : defaults_.find("EFFECTS")->second
            );
    load_motion_object_config(
            ptree_.find("MOTION_OBJECT_CONFIG")->second,
            defaults_.count("MOTION_OBJECT_CONFIG") == 0UL ?
                    boost::property_tree::ptree() : defaults_.find("MOTION_OBJECT_CONFIG")->second
            );
    load_sensors(ptree_.find("SENSORS")->second);
    load_transitions(
            ptree_.find("TRANSITIONS")->second,
            defaults_.count("TRANSITIONS") == 0UL ?
                    boost::property_tree::ptree() : defaults_.find("TRANSITIONS")->second
            );
}


void  agent_action::load_desire(
        boost::property_tree::ptree const&  ptree,
        boost::property_tree::ptree const&  defaults
        )
{
    auto get = [&ptree, &defaults](std::string const&  property_name) -> float_32_bit {
        auto  it = ptree.find(property_name);
        if (it == ptree.not_found())
            it = defaults.find(property_name);
        return it->second.get_value<float_32_bit>();
    };
    load(DESIRE.ideal, ptree.find("ideal")->second, defaults.find("ideal")->second);
    load(DESIRE.weights, ptree.find("weights")->second, defaults.find("weights")->second);
    DESIRE.treashold = get("treashold");
}


void  agent_action::load_effects(
        boost::property_tree::ptree const&  ptree,
        boost::property_tree::ptree const&  defaults
        )
{
    // TODO!
}


void  agent_action::load_motion_object_config(
        boost::property_tree::ptree const&  ptree,
        boost::property_tree::ptree const&  defaults
        )
{
    auto get_ptree = [&ptree, &defaults](std::string const&  property_name) -> boost::property_tree::ptree const* {
        auto  it = ptree.find(property_name);
        if (it == ptree.not_found())
            it = defaults.find(property_name);
        if (it == defaults.not_found())
            return nullptr;
        return &it->second;
    };

    MOTION_OBJECT_CONFIG.shape_type = angeo::as_collision_shape_type(ptree.get<std::string>("shape_type"));

    {
        boost::property_tree::ptree const* const  p = get_ptree("aabb_half_size");
        if (p == nullptr)
            MOTION_OBJECT_CONFIG.aabb_half_size = motion_templates().at(MOTION_TEMPLATE_NAME).bboxes.front();
        else
            MOTION_OBJECT_CONFIG.aabb_half_size = read_vector3(*p);
    }

    MOTION_OBJECT_CONFIG.collision_material =
            angeo::read_collison_material_from_string(get_ptree("collision_material")->get_value<std::string>());

    MOTION_OBJECT_CONFIG.mass_inverted = get_ptree("mass_inverted")->get_value<float_32_bit>();

    {
        boost::property_tree::ptree const* const  p = get_ptree("inertia_tensor_inverted");
        MOTION_OBJECT_CONFIG.inertia_tensor_inverted(0,0) = p->get<float_32_bit>("00");
        MOTION_OBJECT_CONFIG.inertia_tensor_inverted(0,1) = p->get<float_32_bit>("01");
        MOTION_OBJECT_CONFIG.inertia_tensor_inverted(0,2) = p->get<float_32_bit>("02");
        MOTION_OBJECT_CONFIG.inertia_tensor_inverted(1,0) = p->get<float_32_bit>("10");
        MOTION_OBJECT_CONFIG.inertia_tensor_inverted(1,1) = p->get<float_32_bit>("11");
        MOTION_OBJECT_CONFIG.inertia_tensor_inverted(1,2) = p->get<float_32_bit>("12");
        MOTION_OBJECT_CONFIG.inertia_tensor_inverted(2,0) = p->get<float_32_bit>("20");
        MOTION_OBJECT_CONFIG.inertia_tensor_inverted(2,1) = p->get<float_32_bit>("21");
        MOTION_OBJECT_CONFIG.inertia_tensor_inverted(2,2) = p->get<float_32_bit>("22");
    }

    MOTION_OBJECT_CONFIG.is_moveable = get_ptree("is_moveable")->get_value<bool>();
}


void  agent_action::load_sensors(boost::property_tree::ptree const&  ptree)
{
    for (auto const&  name_and_props : ptree)
    {
        sensor_config  sc;
        sc.name = name_and_props.first;
        sc.under_folder = name_and_props.second.get<std::string>("under_folder");
        ASSUMPTION(ctx().is_valid_folder_guid(ctx().from_relative_path(binding().folder_guid_of_agent, sc.under_folder)));
        sc.shape_type = angeo::as_collision_shape_type(name_and_props.second.get<std::string>("shape_type"));
        sc.collision_class = angeo::read_collison_class_from_string(name_and_props.second.get<std::string>("collision_class"));
        sc.aabb_half_size = read_aabb_half_size(name_and_props.second.find("aabb_half_size")->second);
        sc.frame.set_origin(read_vector3(name_and_props.second.find("origin")->second));
        sc.frame.set_orientation(quaternion_identity()); // TODO!
        SENSORS.insert({ name_and_props.first, sc });
    }
}


void  agent_action::load_transitions(
        boost::property_tree::ptree const&  ptree,
        boost::property_tree::ptree const&  defaults
        )
{
    auto get = [&defaults](boost::property_tree::ptree const&  ptree, std::string const&  property_name) -> std::string {
        auto  it = ptree.find(property_name);
        if (it == ptree.not_found())
            it = defaults.find(property_name);
        return it->second.get_value<std::string>();
    };

    for (auto const&  name_and_props : ptree)
    {
        transition_config  tc;
        if (name_and_props.second.count("perception_guard") != 0UL)
        {
            boost::property_tree::ptree const&  guard = name_and_props.second.find("perception_guard")->second;
            tc.perception_guard = std::make_shared<agent_action::transition_config::perception_guard_config>();
            tc.perception_guard->perception_kind =
                    guard.get<std::string>("perception_kind") == "SIGHT" ?
                            transition_config::PERCEPTION_KIND::SIGHT :
                            transition_config::PERCEPTION_KIND::TOUCH ;
            tc.perception_guard->sensor_folder_name = guard.get<std::string>("sensor_folder_name");
            ASSUMPTION(!tc.perception_guard->sensor_folder_name.empty() &&
                       tc.perception_guard->sensor_folder_name.back() != '/');
            tc.perception_guard->sensor_owner_kind =
                    com::read_object_kind_from_string(guard.get<std::string>("sensor_owner_kind"));
            ASSUMPTION(tc.perception_guard->sensor_owner_kind == com::OBJECT_KIND::AGENT ||
                       tc.perception_guard->sensor_owner_kind == com::OBJECT_KIND::SENSOR);
            if (tc.perception_guard->sensor_owner_kind == com::OBJECT_KIND::AGENT)
            tc.perception_guard->sensor_owner_can_be_myself = tc.perception_guard->sensor_owner_kind == com::OBJECT_KIND::AGENT ?
                    guard.get<bool>("sensor_owner_can_be_myself") : false;
            boost::property_tree::ptree const&  location = guard.find("location_constraint")->second;
            tc.perception_guard->location_constraint.frame_folder = location.count("frame_folder") == 0UL ?
                    "motion_object/" : location.get<std::string>("frame_folder");
            tc.perception_guard->location_constraint.shape_type = location.count("shape_type") == 0UL ?
                    angeo::COLLISION_SHAPE_TYPE::BOX : angeo::as_collision_shape_type(location.get<std::string>("shape_type"));
            ASSUMPTION(
                tc.perception_guard->location_constraint.shape_type == angeo::COLLISION_SHAPE_TYPE::BOX ||
                tc.perception_guard->location_constraint.shape_type == angeo::COLLISION_SHAPE_TYPE::CAPSULE ||
                tc.perception_guard->location_constraint.shape_type == angeo::COLLISION_SHAPE_TYPE::SPHERE
                );
            tc.perception_guard->location_constraint.origin = location.count("origin") == 0UL ?
                    vector3_zero() : read_vector3(location.find("origin")->second);
            tc.perception_guard->location_constraint.aabb_half_size = read_aabb_half_size(location.find("aabb_half_size")->second);
            ASSUMPTION(min_coord(tc.perception_guard->location_constraint.aabb_half_size) > 0.0001f);
        }
        else
            tc.perception_guard = nullptr;

        if (name_and_props.second.count("motion_object_position") != 0UL)
        {
            boost::property_tree::ptree const&  position = name_and_props.second.find("motion_object_position")->second;
            tc.motion_object_position = std::make_shared<agent_action::transition_config::motion_object_position_config>();
            tc.motion_object_position->is_self_frame = position.get<std::string>("frame_owner") == "SELF";
            tc.motion_object_position->frame_folder = position.get<std::string>("frame_folder");
            ASSUMPTION(!tc.motion_object_position->frame_folder.empty() &&
                       tc.motion_object_position->frame_folder.back() == '/');
            tc.motion_object_position->origin = read_vector3(position.find("origin")->second);

            tc.aabb_alignment = transition_config::AABB_ALIGNMENT::CENTER;
        }
        else
        {
            tc.motion_object_position = nullptr;
            static std::unordered_map<std::string, transition_config::AABB_ALIGNMENT>  alignments {
                { "CENTER", transition_config::AABB_ALIGNMENT::CENTER },
                { "X_LO", transition_config::AABB_ALIGNMENT::X_LO },
                { "X_HI", transition_config::AABB_ALIGNMENT::X_HI },
                { "Y_LO", transition_config::AABB_ALIGNMENT::Y_LO },
                { "Y_HI", transition_config::AABB_ALIGNMENT::Y_HI },
                { "Z_LO", transition_config::AABB_ALIGNMENT::Z_LO },
                { "Z_HI", transition_config::AABB_ALIGNMENT::Z_HI }
            };
            tc.aabb_alignment = alignments.at(get(name_and_props.second, "aabb_alignment"));
        }
        TRANSITIONS.insert({ name_and_props.first, tc });
    }
}


float_32_bit  agent_action::compute_desire_penalty() const
{
    std::vector<float_32_bit>  props_vec, ideal_vec, weights_vec;
    as_vector(desire(), props_vec);
    as_vector(DESIRE.ideal, ideal_vec);
    as_vector(DESIRE.weights, weights_vec);

    float_32_bit  penalty = 0.0f;
    for (natural_32_bit  i = 0U, n = (natural_32_bit)props_vec.size(); i < n; ++i)
    {
        float_32_bit const  delta = props_vec.at(i) - ideal_vec.at(i);
        penalty += delta * delta * weights_vec.at(i);
    }

    return penalty;
}


bool  agent_action::is_complete() const
{
    return  m_current_time >= m_end_time;
}


bool  agent_action::is_ghost_complete() const
{
    return  m_current_time >= m_end_ghost_time;
}


float_32_bit  agent_action::interpolation_parameter() const
{
    return  m_end_time - m_start_time < 0.0001f ?
                1.0f : (m_current_time - m_start_time) / (m_end_time - m_start_time);
}


float_32_bit  agent_action::interpolation_parameter_ghost() const
{
    return  m_end_ghost_time - m_start_time < 0.0001f ?
                1.0f : (m_current_time - m_start_time) / (m_end_ghost_time - m_start_time);
}

com::object_guid  agent_action::get_frame_guid_of_skeleton_location() const
{
    return USE_GHOST_OBJECT_FOR_SKELETON_LOCATION ? binding().frame_guid_of_ghost_object :
                                                    binding().frame_guid_of_motion_object;
}


void  agent_action::apply_effects(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    // TODO!
}


void  agent_action::update_time(float_32_bit const  time_step_in_seconds)
{
    m_current_time += state_variables().at("animation_speed").value * time_step_in_seconds;
    m_current_time += m_context->time_buffer;
    m_context->time_buffer = 0.0f;

    if (m_current_time >= m_end_time)
    {
        m_context->time_buffer += m_current_time - m_end_time;
        m_current_time = m_end_time;
    }
    else
        m_context->time_buffer = 0.0f;
}


void  agent_action::update_ghost()
{
    angeo::coordinate_system  cs_motion = ctx().frame_coord_system(binding().frame_guid_of_motion_object);
    angeo::coordinate_system  cs_ghost = ctx().frame_coord_system(binding().frame_guid_of_ghost_object);
    float_32_bit  param_ghost = interpolation_parameter_ghost();

    if (is_ghost_complete())
    {
        if (ctx().parent_frame(binding().frame_guid_of_skeleton) != binding().frame_guid_of_motion_object)
            ctx().request_set_parent_frame(binding().frame_guid_of_skeleton, binding().frame_guid_of_motion_object);
        return;
    }

    angeo::coordinate_system  result;
    angeo::interpolate_spherical(
            m_ghost_object_start_coord_system,
            ctx().frame_coord_system(binding().frame_guid_of_motion_object),
            interpolation_parameter_ghost(),
            result
            );
    ctx().request_relocate_frame(binding().frame_guid_of_ghost_object, result);
}


void  agent_action::update_look_at(float_32_bit const  time_step_in_seconds)
{
    float_32_bit const  longitude = 0.5f * PI() * (desire().look_at.longitude - 1.0f);
    float_32_bit const  altitude = 0.5f * PI() * desire().look_at.altitude;
    float_32_bit const  magnitude = (0.5f * (1.0f + desire().look_at.magnitude)) *
                                    myself().get_sight_controller().get_camera()->far_plane();
    float_32_bit const  cos_altitude = std::cosf(desire().look_at.altitude);
    vector3  target = {
        magnitude * cos_altitude * std::cosf(longitude),
        magnitude * cos_altitude * std::sinf(longitude),
        magnitude * std::sinf(altitude),
    };
    target = transform_point(target, ctx().frame_world_matrix(ctx().parent_frame(binding().frame_guid_of_skeleton)));
    m_context->look_at.interpolate(time_step_in_seconds, target, m_context->animate.get_current_frames_ref(), motion_templates(), binding());
}


void  agent_action::update_aim_at(float_32_bit const  time_step_in_seconds)
{
}


void  agent_action::update_animation(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    if (m_current_time >= m_end_time)
    {
        m_animation.last_keyframe_completion_time = m_end_time;
        if (m_animation.target_keyframe_index != m_animation.end_keyframe_index)
        {
            m_animation.target_keyframe_index = m_animation.end_keyframe_index;
            m_context->animate.set_target({MOTION_TEMPLATE_NAME, m_animation.target_keyframe_index}, motion_templates());
        }
        m_context->animate.move_to_target();
    }
    else
    {
        skeletal_motion_templates::keyframes const  keyframes = motion_templates().at(MOTION_TEMPLATE_NAME).keyframes;

        if (m_current_time >= keyframes.keyframe_at(m_animation.target_keyframe_index).get_time_point())
        {
            m_context->animate.move_to_target();
            m_animation.last_keyframe_completion_time =
                keyframes.keyframe_at(m_animation.target_keyframe_index).get_time_point();
            for ( ; m_animation.target_keyframe_index < m_animation.end_keyframe_index; ++m_animation.target_keyframe_index)
                if (m_current_time < keyframes.keyframe_at(m_animation.target_keyframe_index).get_time_point())
                    break;
            m_context->animate.set_target({MOTION_TEMPLATE_NAME, m_animation.target_keyframe_index}, motion_templates());
        }

        INVARIANT(m_current_time >= m_animation.last_keyframe_completion_time);
        INVARIANT(
            keyframes.keyframe_at(m_animation.target_keyframe_index).get_time_point()
                    > m_animation.last_keyframe_completion_time
            );

        m_context->animate.interpolate(
                (m_current_time - m_animation.last_keyframe_completion_time) /
                (keyframes.keyframe_at(m_animation.target_keyframe_index).get_time_point()
                        - m_animation.last_keyframe_completion_time)
                );
    }
}


bool  agent_action::is_guard_valid() const
{
    // TODO!
    return true;
}


bool  agent_action::collect_transition_info(agent_action const&  from_action, transition_info&  info) const
{
    ASSUMPTION(&from_action != this);

    info.other_entiry_folder_guid = com::invalid_object_guid();

    transition_config const&  config = from_action.TRANSITIONS.at(NAME);
    if (config.perception_guard == nullptr)
        return true;
    transition_config::perception_guard_config const  percept = *config.perception_guard;

    if (percept.perception_kind == transition_config::PERCEPTION_KIND::SIGHT)
    {
        sight_controller::ray_casts_in_time const&  ray_casts = myself().get_sight_controller().get_directed_ray_casts_in_time();
        for (auto  ray_it = ray_casts.begin(), end = ray_casts.end(); ray_it != end; ++ray_it)
        {
            com::object_guid const  collider_guid = ray_it->second.collider_guid;

            com::object_guid const  owner_guid = ctx().owner_of_collider(collider_guid);
            if (owner_guid.kind != percept.sensor_owner_kind)
                continue;

            if (owner_guid.kind == com::OBJECT_KIND::AGENT && !percept.sensor_owner_can_be_myself &&
                    ctx().folder_of_agent(owner_guid) == binding().folder_guid_of_agent)
                continue;

            std::string const&  folder_name = ctx().name_of_folder(ctx().folder_of_collider(collider_guid));
            if (folder_name != percept.sensor_folder_name)
                continue;

            vector3 const  contact_point_in_world_space =
                    ray_it->second.ray_origin_in_world_space +
                    ray_it->second.parameter_to_coid * ray_it->second.ray_direction_in_world_space
                    ;
            com::object_guid const  frame_guid = get_frame_guid_under_agent_folder(
                    binding().folder_guid_of_agent,
                    percept.location_constraint.frame_folder,
                    ctx()
                    );
            vector3 const  contact_point_in_motion_object_space =
                    angeo::point3_to_coordinate_system(
                            contact_point_in_world_space,
                            ctx().frame_coord_system_in_world_space(frame_guid)
                            );
            switch(percept.location_constraint.shape_type)
            {
            case angeo::COLLISION_SHAPE_TYPE::BOX:
                if (!angeo::collision_point_and_bbox(
                        contact_point_in_motion_object_space,
                        percept.location_constraint.origin - percept.location_constraint.aabb_half_size,
                        percept.location_constraint.origin + percept.location_constraint.aabb_half_size
                        ))
                    continue;
                break;
            case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
                if (!angeo::is_point_inside_capsule(
                        contact_point_in_motion_object_space - percept.location_constraint.origin,
                        max_coord(percept.location_constraint.aabb_half_size) - min_coord(percept.location_constraint.aabb_half_size),
                        min_coord(percept.location_constraint.aabb_half_size)
                        ))
                    continue;
                break;
            case angeo::COLLISION_SHAPE_TYPE::SPHERE:
                if (!angeo::is_point_inside_sphere(
                        contact_point_in_motion_object_space - percept.location_constraint.origin,
                        min_coord(percept.location_constraint.aabb_half_size)
                        ))
                    continue;
                break;
            default: { UNREACHABLE(); } break;
            }

            info.other_entiry_folder_guid = ctx().folder_of(owner_guid);
            return true;
        }
    }
    else
    {
        INVARIANT(percept.perception_kind == transition_config::PERCEPTION_KIND::TOUCH);

        // TODO!
    }

    return false;
}


void  agent_action::on_transition(agent_action* const  from_action_ptr, transition_info const&  info)
{
    TMPROF_BLOCK();

    ASSUMPTION(this != from_action_ptr || is_complete());

    skeletal_motion_templates::keyframes const  keyframes = motion_templates().at(MOTION_TEMPLATE_NAME).keyframes;

    if (from_action_ptr == nullptr)
    {
        m_start_time = keyframes.start_time_point();
        m_end_time = keyframes.end_time_point();
        m_end_ghost_time = m_start_time;
        m_current_time = m_start_time;

        m_animation.end_keyframe_index = (natural_32_bit)keyframes.num_keyframes() - 1U;
        m_animation.last_keyframe_completion_time = m_start_time;
        m_animation.target_keyframe_index = 1U;

        m_context->animate.set_target({MOTION_TEMPLATE_NAME, m_animation.target_keyframe_index}, motion_templates());
    }
    else if (ONLY_INTERPOLATE_TO_MOTION_TEMPLATE)
    {
        std::pair<natural_32_bit, float_32_bit> const  transition_props =
                get_motion_template_transition_props(
                            motion_templates().transitions(),
                            { from_action_ptr->MOTION_TEMPLATE_NAME, from_action_ptr->m_animation.target_keyframe_index },
                            MOTION_TEMPLATE_NAME,
                            motion_templates().default_transition_props()
                            );

        m_start_time = keyframes.time_point_at(transition_props.first) - transition_props.second;
        m_end_time = keyframes.time_point_at(transition_props.first);
        m_end_ghost_time = m_end_time;
        m_current_time = m_start_time;

        m_animation.end_keyframe_index = transition_props.first;
        m_animation.last_keyframe_completion_time = m_start_time;
        m_animation.target_keyframe_index = transition_props.first;

        m_context->animate.set_target({MOTION_TEMPLATE_NAME, m_animation.target_keyframe_index}, motion_templates());
    }
    else if (MOTION_TEMPLATE_NAME == from_action_ptr->MOTION_TEMPLATE_NAME && !from_action_ptr->is_complete())
    {
        m_start_time = from_action_ptr->m_current_time;
        m_end_time = keyframes.end_time_point();
        m_end_ghost_time = m_start_time;
        m_current_time = m_start_time;

        m_animation.end_keyframe_index = (natural_32_bit)keyframes.num_keyframes() - 1U;
        m_animation.last_keyframe_completion_time = from_action_ptr->m_animation.last_keyframe_completion_time;
        m_animation.target_keyframe_index = from_action_ptr->m_animation.target_keyframe_index;
    }
    else if (MOTION_TEMPLATE_NAME == from_action_ptr->MOTION_TEMPLATE_NAME && IS_CYCLIC)
    {
        m_start_time = keyframes.start_time_point();
        m_end_time = keyframes.end_time_point();
        m_end_ghost_time = m_start_time;
        m_current_time = m_start_time;

        m_animation.end_keyframe_index = (natural_32_bit)keyframes.num_keyframes() - 1U;
        m_animation.last_keyframe_completion_time = m_start_time;
        m_animation.target_keyframe_index = 1U;

        m_context->animate.set_target({MOTION_TEMPLATE_NAME, m_animation.target_keyframe_index}, motion_templates());
    }
    else
    {
        std::pair<natural_32_bit, float_32_bit> const  transition_props =
                get_motion_template_transition_props(
                            motion_templates().transitions(),
                            { from_action_ptr->MOTION_TEMPLATE_NAME, from_action_ptr->m_animation.target_keyframe_index },
                            MOTION_TEMPLATE_NAME,
                            motion_templates().default_transition_props()
                            );

        m_start_time = keyframes.time_point_at(transition_props.first) - transition_props.second;
        m_end_time = keyframes.end_time_point();
        m_end_ghost_time = m_start_time + transition_props.second;
        m_current_time = m_start_time;

        m_animation.end_keyframe_index = (natural_32_bit)keyframes.num_keyframes() - 1U;
        m_animation.last_keyframe_completion_time = m_start_time;
        m_animation.target_keyframe_index = transition_props.first;

        m_context->animate.set_target({MOTION_TEMPLATE_NAME, m_animation.target_keyframe_index}, motion_templates());
    }

    INVARIANT(m_current_time >= m_start_time && m_current_time <= m_end_time);
    INVARIANT(m_animation.last_keyframe_completion_time >= m_start_time && m_animation.last_keyframe_completion_time <= m_end_time);
    INVARIANT(m_current_time >= m_animation.last_keyframe_completion_time);
    INVARIANT(m_end_time > m_start_time);
    INVARIANT(m_end_ghost_time >= m_start_time && m_end_ghost_time <= m_end_time);
    INVARIANT(m_animation.target_keyframe_index < keyframes.num_keyframes());

    if (from_action_ptr == nullptr)
    {
        matrix44  W = ctx().frame_world_matrix(binding().frame_guid_of_motion_object);
        matrix44  F;
        angeo::from_base_matrix(motion_templates().at(MOTION_TEMPLATE_NAME).reference_frames.at(0), F);
        vector3  pos;
        matrix33  rot;
        decompose_matrix44(W * F, pos, rot);
        quaternion const  ori = rotation_matrix_to_quaternion(rot);
        ctx().request_relocate_frame(binding().frame_guid_of_motion_object, pos, ori);
        ctx().request_relocate_frame(binding().frame_guid_of_ghost_object, pos, ori);
    }

    {
        com::object_guid const  skeleton_parent_frame_guid = ctx().parent_frame(binding().frame_guid_of_skeleton);
        INVARIANT(
            ctx().is_valid_frame_guid(skeleton_parent_frame_guid) &&
            (skeleton_parent_frame_guid == binding().frame_guid_of_ghost_object ||
                skeleton_parent_frame_guid == binding().frame_guid_of_motion_object)
            );
        m_ghost_object_start_coord_system = ctx().frame_coord_system(skeleton_parent_frame_guid);
        ctx().request_relocate_frame(binding().frame_guid_of_ghost_object, m_ghost_object_start_coord_system);

        if (from_action_ptr != nullptr && from_action_ptr != this)
        {
            angeo::coordinate_system const  frame =
                    skeleton_parent_frame_guid == binding().frame_guid_of_motion_object ?
                            m_ghost_object_start_coord_system :
                            ctx().frame_coord_system(binding().frame_guid_of_motion_object);

            vector3  origin;

            std::shared_ptr<transition_config::motion_object_position_config> const  pos_cfg =
                    from_action_ptr->TRANSITIONS.at(NAME).motion_object_position;
            if (pos_cfg != nullptr)
            {
                com::object_guid const  frame_guid = get_frame_guid_under_agent_folder(
                        pos_cfg->is_self_frame ? binding().folder_guid_of_agent : info.other_entiry_folder_guid,
                        pos_cfg->frame_folder,
                        ctx()
                        );
                origin = transform_point(pos_cfg->origin, ctx().frame_world_matrix(frame_guid));                
            }
            else if (from_action_ptr->MOTION_OBJECT_CONFIG != MOTION_OBJECT_CONFIG)
            {
                natural_32_bit  coord_idx;
                float_32_bit  sign;
                switch (from_action_ptr->TRANSITIONS.at(NAME).aabb_alignment)
                {
                case transition_config::AABB_ALIGNMENT::CENTER: coord_idx = 0U; sign = 0.0f; break;
                case transition_config::AABB_ALIGNMENT::X_LO: coord_idx = 0U; sign = -1.0f; break;
                case transition_config::AABB_ALIGNMENT::X_HI: coord_idx = 0U; sign = 1.0f; break;
                case transition_config::AABB_ALIGNMENT::Y_LO: coord_idx = 1U; sign = -1.0f; break;
                case transition_config::AABB_ALIGNMENT::Y_HI: coord_idx = 1U; sign = 1.0f; break;
                case transition_config::AABB_ALIGNMENT::Z_LO: coord_idx = 2U; sign = -1.0f; break;
                case transition_config::AABB_ALIGNMENT::Z_HI: coord_idx = 2U; sign = 1.0f; break;
                default: { UNREACHABLE(); } break;
                }

                float_32_bit const  delta = from_action_ptr->MOTION_OBJECT_CONFIG.aabb_half_size(coord_idx) -
                                            MOTION_OBJECT_CONFIG.aabb_half_size(coord_idx);
            
                origin = frame.origin() + (sign * delta) * vector3_unit(coord_idx);
            }

            ctx().request_relocate_frame(binding().frame_guid_of_motion_object, origin, frame.orientation());
        }

        if (is_ghost_complete())
        {
            if (skeleton_parent_frame_guid != binding().frame_guid_of_motion_object)
                ctx().request_set_parent_frame(binding().frame_guid_of_skeleton, binding().frame_guid_of_motion_object);
        }
        else if (skeleton_parent_frame_guid != binding().frame_guid_of_ghost_object)
            ctx().request_set_parent_frame(binding().frame_guid_of_skeleton, binding().frame_guid_of_ghost_object);
    }

    if (from_action_ptr == nullptr || from_action_ptr->MOTION_OBJECT_CONFIG != MOTION_OBJECT_CONFIG)
    {
        vector3  linear_velocity, angular_velocity, linear_acceleration, angular_acceleration;
        if (from_action_ptr == nullptr)
            linear_velocity = angular_velocity = linear_acceleration = angular_acceleration = vector3_zero();
        else
        {
            com::object_guid const  rigid_body_guid =
                    ctx().folder_content(binding().folder_guid_of_motion_object).content
                         .at(com::to_string(com::OBJECT_KIND::RIGID_BODY));
            com::object_guid const  collider_guid =
                    ctx().folder_content(binding().folder_guid_of_motion_object).content
                         .at(com::to_string(com::OBJECT_KIND::COLLIDER));

            ASSUMPTION(ctx().is_valid_rigid_body_guid(rigid_body_guid) && ctx().is_valid_collider_guid(collider_guid));

            linear_velocity = ctx().linear_velocity_of_rigid_body(rigid_body_guid);
            angular_velocity = ctx().angular_velocity_of_rigid_body(rigid_body_guid);
            linear_acceleration = ctx().linear_acceleration_of_rigid_body(rigid_body_guid);
            angular_acceleration = ctx().angular_acceleration_of_rigid_body(rigid_body_guid);

            ctx().request_erase_collider(collider_guid);
            ctx().request_erase_rigid_body(rigid_body_guid);
        }

        ctx().request_insert_rigid_body(
                binding().folder_guid_of_motion_object,
                MOTION_OBJECT_CONFIG.is_moveable,
                linear_velocity,
                angular_velocity,
                linear_acceleration,
                angular_acceleration,
                MOTION_OBJECT_CONFIG.mass_inverted,
                MOTION_OBJECT_CONFIG.inertia_tensor_inverted
                );
        insert_collider_to_context(
                ctx(),
                binding().folder_guid_of_motion_object,
                MOTION_OBJECT_CONFIG.shape_type,
                MOTION_OBJECT_CONFIG.aabb_half_size,
                MOTION_OBJECT_CONFIG.collision_material,
                angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT
                );
    }

    for (auto const&  name_and_props : SENSORS)
        if (from_action_ptr == nullptr || from_action_ptr->SENSORS.count(name_and_props.first) == 0UL)
        {
            com::object_guid const  under_folder_guid =
                    ctx().from_relative_path(binding().folder_guid_of_agent, name_and_props.second.under_folder);
            com::object_guid const  folder_guid = ctx().insert_folder(under_folder_guid, name_and_props.first, false);
            com::object_guid const  frame_guid = ctx().insert_frame(
                    folder_guid,
                    ctx().find_closest_frame(under_folder_guid, true),
                    name_and_props.second.frame.origin(),
                    name_and_props.second.frame.orientation()
                    );
            insert_collider_to_context(
                    ctx(),
                    folder_guid,
                    name_and_props.second.shape_type,
                    name_and_props.second.aabb_half_size,
                    angeo::COLLISION_MATERIAL_TYPE::NO_FRINCTION_NO_BOUNCING,
                    name_and_props.second.collision_class
                    );
        }
    if (from_action_ptr != nullptr)
        for (auto const&  name_and_props : from_action_ptr->SENSORS)
            if (SENSORS.count(name_and_props.first) == 0UL)
                ctx().request_erase_non_root_folder(
                        ctx().from_relative_path(
                                binding().folder_guid_of_agent,
                                name_and_props.second.under_folder + name_and_props.first + "/"
                                )
                        );
}


void  agent_action::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    apply_effects(time_step_in_seconds);
    update_time(time_step_in_seconds);
    update_ghost();
    update_animation(time_step_in_seconds);
    if (IS_LOOK_AT_ENABLED)
        update_look_at(time_step_in_seconds);
    if (IS_AIM_AT_ENABLED)
        update_aim_at(time_step_in_seconds);
    m_context->animate.commit(motion_templates(), binding());
}


std::unordered_set<com::object_guid>  agent_action::get_motion_object_collider_guids() const
{
    return {
        ctx().folder_content(binding().folder_guid_of_motion_object).content.at(com::to_string(com::OBJECT_KIND::COLLIDER))
        };
}


action_guesture::action_guesture(
        std::string const&  name_,
        boost::property_tree::ptree const&  ptree_,
        boost::property_tree::ptree const&  defaults_,
        action_execution_context_ptr const  context_
        )
    : agent_action(name_, ptree_.find("agent_action")->second, defaults_, context_)
{
}


void  action_guesture::on_transition(agent_action* const  from_action_ptr, transition_info const&  info)
{
    agent_action::on_transition(from_action_ptr, info);

    // TODO
}


void  action_guesture::next_round(float_32_bit const  time_step_in_seconds)
{
    agent_action::next_round(time_step_in_seconds);

    // TODO
}


//void  action_interpolate::next_round(float_32_bit const  time_step_in_seconds)
//{
//    angeo::coordinate_system  result;
//    angeo::interpolate_spherical(
//            ctx().frame_coord_system_in_world_space(binding().frame_guid_of_skeleton),
//            target,
//            interpolation_parameter(),
//            result
//            );
//    ctx().request_relocate_frame(binding().frame_guid_of_skeleton, result);
//    agent_action::next_round(time_step_in_seconds);
//}


action_controller::action_controller(agent_config const  config, agent*  const  myself)
    : m_context(std::make_shared<action_execution_context>(myself))
    , m_current_action(nullptr) // loaded below
    , m_available_actions() // loaded below
{
    if (m_context->state_variables().count("animation_speed") == 0UL)
        m_context->state_variables().insert({ "animation_speed", { "animation_speed", 1.0f, 0.5f, 2.0f } });
    for (auto const&  name_and_ptree : config.actions())
    {
        ASSUMPTION(name_and_ptree.second->size() == 1UL);
        std::vector<std::string>  class_names;
        auto const it = name_and_ptree.second->begin();
        ASSUMPTION(m_available_actions.count(name_and_ptree.first) == 0UL);
        if (it->first == "agent_action")
            m_available_actions[name_and_ptree.first] =
                    std::make_shared<agent_action>(name_and_ptree.first, it->second, config.defaults(), m_context);
        else if (it->first == "action_guesture")
            m_available_actions[name_and_ptree.first] =
                    std::make_shared<action_guesture>(name_and_ptree.first, it->second, config.defaults(), m_context);        
        else { UNREACHABLE(); }
    }
    ASSUMPTION(
        [this]() -> bool {
            for (auto const&  name_and_action : m_available_actions)
            {
                if (!name_and_action.second->is_cyclic() && name_and_action.second->get_transitions().empty())
                    return false;
                for (auto const&  action_name_and_props : name_and_action.second->get_transitions())
                {
                    if (action_name_and_props.first == name_and_action.first)
                        return false;
                    if (m_available_actions.count(action_name_and_props.first) == 0UL)
                        return false;
                }
            }
            return true;
        }()
    );
    m_current_action = m_available_actions.at(config.initial_action());
    m_current_action->on_transition(nullptr, agent_action::transition_info());
    m_context->animate.move_to_target();
    m_context->animate.commit(m_context->motion_templates(), m_context->binding());
}


action_controller::~action_controller()
{}


void  action_controller::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    m_current_action->next_round(time_step_in_seconds);
    if (m_current_action->is_ghost_complete())
        process_action_transitions();
}


void  action_controller::process_action_transitions()
{
    std::shared_ptr<agent_action>  best_action;
    agent_action::transition_info  best_info;
    {
        best_action = nullptr;
        float_32_bit  best_penalty = 0.0f;
        if ((m_current_action->is_cyclic() || !m_current_action->is_complete()) && m_current_action->is_guard_valid())
        {
            best_action = m_current_action;
            best_penalty = m_current_action->compute_desire_penalty();
        }
        for (auto const&  name_and_props : m_current_action->get_transitions())
        {
            std::shared_ptr<agent_action> const  action_ptr = m_available_actions.at(name_and_props.first);
            agent_action::transition_info  info;
            if (!action_ptr->collect_transition_info(*m_current_action, info))
                continue;
            float_32_bit const  penalty = action_ptr->compute_desire_penalty();
            if (best_action == nullptr || penalty < best_penalty)
            {
                best_action = action_ptr;
                best_penalty = penalty;
                best_info = info;
            }
        }
    }
    INVARIANT(best_action != nullptr);

    if (m_current_action != best_action || m_current_action->is_complete())
    {
        best_action->on_transition(&*m_current_action, best_info);
        m_current_action = best_action;
    }
}


}
