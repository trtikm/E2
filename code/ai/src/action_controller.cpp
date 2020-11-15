#include <ai/action_controller.hpp>
#include <ai/agent.hpp>
#include <ai/skeleton_utils.hpp>
#include <ai/utils_ptree.hpp>
#include <angeo/collide.hpp>
#include <angeo/linear_segment_curve.hpp>
#include <angeo/utility.hpp>
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
    , disabled_colliding_with_our_motion_object()
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
    return  shape_type == other.shape_type &&
            are_equal_3d(aabb_half_size, other.aabb_half_size, 0.001f) &&
            aabb_alignment == other.aabb_alignment &&
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
    , MOTION_TEMPLATE_NAME(get_value<std::string>("MOTION_TEMPLATE_NAME", ptree_))
    , ONLY_INTERPOLATE_TO_MOTION_TEMPLATE(get_value<bool>("ONLY_INTERPOLATE_TO_MOTION_TEMPLATE", false, ptree_))
    , USE_MOTION_TEMPLATE_FOR_LOCATION_INTERPOLATION(get_value<bool>("USE_MOTION_TEMPLATE_FOR_LOCATION_INTERPOLATION", false, ptree_))
    , IS_CYCLIC(get_value<bool>("IS_CYCLIC", false, ptree_))
    , IS_LOOK_AT_ENABLED(get_value<bool>("IS_LOOK_AT_ENABLED", false, ptree_))
    , IS_AIM_AT_ENABLED(get_value<bool>("IS_AIM_AT_ENABLED", false, ptree_))
    , DEFINE_SKELETON_SYNC_SOURCE_IN_WORLD_SPACE(get_value<bool>("DEFINE_SKELETON_SYNC_SOURCE_IN_WORLD_SPACE", false, ptree_))
    , SENSORS() // loaded below
    , TRANSITIONS() // loaded below
    , MOTION_OBJECT_CONFIG() // loaded below
    , AABB_HALF_SIZE() // loaded below
    // MUTABLE DATA
    , m_context(context_)
    , m_start_time(0.0f)
    , m_end_time(0.0f)
    , m_end_ghost_time(0.0f)
    , m_end_interpolation_time(0.0f)
    , m_current_time(0.0f)
    , m_animation{0.0f, 0U, 0U}
{
    ASSUMPTION(motion_templates().motions_map().count(MOTION_TEMPLATE_NAME) != 0UL);
    ASSUMPTION(!(IS_CYCLIC && USE_MOTION_TEMPLATE_FOR_LOCATION_INTERPOLATION));
    ASSUMPTION(ONLY_INTERPOLATE_TO_MOTION_TEMPLATE == false || USE_MOTION_TEMPLATE_FOR_LOCATION_INTERPOLATION == false);
    load_desire(get_ptree("DESIRE", ptree_), get_ptree_or_empty("DESIRE", defaults_));
    load_effects(get_ptree_or_empty("EFFECTS", ptree_), get_ptree_or_empty("EFFECTS", defaults_));
    load_motion_object_config(get_ptree_or_empty("MOTION_OBJECT_CONFIG", ptree_), get_ptree_or_empty("MOTION_OBJECT_CONFIG", defaults_));
    AABB_HALF_SIZE =
            read_aabb_half_size("AABB_HALF_SIZE", "AABB_HALF_SIZE_FROM_KEYFRAME", motion_templates().at(MOTION_TEMPLATE_NAME).bboxes, ptree_);
    load_sensors(get_ptree_or_empty("SENSORS", ptree_));
    load_transitions(get_ptree("TRANSITIONS", ptree_), get_ptree_or_empty("TRANSITIONS", defaults_));
}


agent_action::AABB_ALIGNMENT  agent_action::aabb_alignment_from_string(std::string const&  alignment_name)
{
    static std::unordered_map<std::string, AABB_ALIGNMENT>  alignments {
        { "CENTER", AABB_ALIGNMENT::CENTER },
        { "X_LO", AABB_ALIGNMENT::X_LO },
        { "X_HI", AABB_ALIGNMENT::X_HI },
        { "Y_LO", AABB_ALIGNMENT::Y_LO },
        { "Y_HI", AABB_ALIGNMENT::Y_HI },
        { "Z_LO", AABB_ALIGNMENT::Z_LO },
        { "Z_HI", AABB_ALIGNMENT::Z_HI }
    };
    auto const  it = alignments.find(alignment_name);
    ASSUMPTION(it != alignments.end());
    return it->second;
}


void  agent_action::load_desire(
        boost::property_tree::ptree const&  ptree,
        boost::property_tree::ptree const&  defaults
        )
{
    load(DESIRE.ideal, get_ptree("ideal", ptree), get_ptree_or_empty("ideal", defaults));
    load(DESIRE.weights, get_ptree("weights", ptree), get_ptree_or_empty("weights", defaults));
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
    MOTION_OBJECT_CONFIG.shape_type = angeo::as_collision_shape_type(get_value<std::string>("shape_type", ptree, &defaults));
    MOTION_OBJECT_CONFIG.aabb_half_size =
            read_aabb_half_size("aabb_half_size", "aabb_half_size_from_keyframe", motion_templates().at(MOTION_TEMPLATE_NAME).bboxes, ptree);
    MOTION_OBJECT_CONFIG.aabb_alignment = aabb_alignment_from_string(get_value("aabb_alignment", "Z_LO", ptree, &defaults));
    MOTION_OBJECT_CONFIG.collision_material =
            angeo::read_collison_material_from_string(get_value<std::string>("collision_material", ptree, &defaults));
    MOTION_OBJECT_CONFIG.mass_inverted = get_value<float_32_bit>("mass_inverted", ptree, &defaults);
    read_matrix33(get_ptree("inertia_tensor_inverted", ptree, &defaults), MOTION_OBJECT_CONFIG.inertia_tensor_inverted);
    MOTION_OBJECT_CONFIG.is_moveable = get_value<bool>("is_moveable", ptree, &defaults);
    MOTION_OBJECT_CONFIG.offset_from_center_of_agent_aabb =
            get_value<vector3>("offset_from_center_of_agent_aabb", vector3_zero(), &read_vector3, ptree, &defaults);
}


void  agent_action::load_sensors(boost::property_tree::ptree const&  ptree)
{
    for (auto const&  name_and_props : ptree)
    {
        sensor_config  sc;
        sc.name = name_and_props.first;
        sc.under_folder = get_value<std::string>("under_folder", name_and_props.second);
        ASSUMPTION(ctx().is_valid_folder_guid(ctx().from_relative_path(binding().folder_guid_of_agent, sc.under_folder)));
        sc.shape_type = angeo::as_collision_shape_type(get_value<std::string>("shape_type", name_and_props.second));
        sc.collision_class = angeo::read_collison_class_from_string(get_value<std::string>("collision_class", name_and_props.second));
        sc.aabb_half_size = read_aabb_half_size(get_ptree("aabb_half_size", name_and_props.second));
        sc.frame.set_origin(read_vector3(get_ptree("origin", name_and_props.second)));
        sc.frame.set_orientation(read_quaternion(get_ptree("orientation", name_and_props.second)));
        SENSORS.insert({ name_and_props.first, sc });
    }
}


void  agent_action::load_transitions(
        boost::property_tree::ptree const&  ptree,
        boost::property_tree::ptree const&  defaults
        )
{
    for (auto const&  name_and_props : ptree)
    {
        transition_config  tc;
        if (name_and_props.second.count("perception_guard") != 0UL)
        {
            boost::property_tree::ptree const&  guard = get_ptree("perception_guard", name_and_props.second);
            tc.perception_guard = std::make_shared<agent_action::transition_config::perception_guard_config>();
            tc.perception_guard->perception_kind =
                    get_value<std::string>("perception_kind", guard) == "SIGHT" ?
                            transition_config::PERCEPTION_KIND::SIGHT :
                            transition_config::PERCEPTION_KIND::TOUCH ;
            tc.perception_guard->sensor_folder_name = get_value<std::string>("sensor_folder_name", guard);
            ASSUMPTION(!tc.perception_guard->sensor_folder_name.empty() &&
                       tc.perception_guard->sensor_folder_name.back() != '/');
            tc.perception_guard->sensor_owner_kind =
                    com::read_object_kind_from_string(get_value<std::string>("sensor_owner_kind", guard));
            ASSUMPTION(tc.perception_guard->sensor_owner_kind == com::OBJECT_KIND::AGENT ||
                       tc.perception_guard->sensor_owner_kind == com::OBJECT_KIND::SENSOR);
            if (tc.perception_guard->sensor_owner_kind == com::OBJECT_KIND::AGENT)
            tc.perception_guard->sensor_owner_can_be_myself = tc.perception_guard->sensor_owner_kind == com::OBJECT_KIND::AGENT ?
                    guard.get<bool>("sensor_owner_can_be_myself") : false;
            boost::property_tree::ptree const&  location = get_ptree("location_constraint", guard);
            tc.perception_guard->location_constraint.frame_folder = get_value("frame_folder", "motion_object/", location);
            tc.perception_guard->location_constraint.shape_type = angeo::as_collision_shape_type(get_value("shape_type", "BOX", location));
            ASSUMPTION(
                tc.perception_guard->location_constraint.shape_type == angeo::COLLISION_SHAPE_TYPE::BOX ||
                tc.perception_guard->location_constraint.shape_type == angeo::COLLISION_SHAPE_TYPE::CAPSULE ||
                tc.perception_guard->location_constraint.shape_type == angeo::COLLISION_SHAPE_TYPE::SPHERE
                );
            tc.perception_guard->location_constraint.frame.set_origin(get_value<vector3>("origin", vector3_zero(), &read_vector3, location));
            tc.perception_guard->location_constraint.frame.set_orientation(
                    get_value<quaternion>("orientation", quaternion_identity(), &read_quaternion, location)
                    );
            tc.perception_guard->location_constraint.aabb_half_size = read_aabb_half_size(get_ptree("aabb_half_size", location));
        }
        else
            tc.perception_guard = nullptr;

        if (name_and_props.second.count("motion_object_location") != 0UL)
        {
            boost::property_tree::ptree const&  location = get_ptree("motion_object_location", name_and_props.second);
            tc.motion_object_location = std::make_shared<agent_action::transition_config::motion_object_location_config>();
            tc.motion_object_location->is_self_frame = get_value<std::string>("frame_owner", location) == "SELF";
            tc.motion_object_location->frame_folder = get_value<std::string>("frame_folder", location);
            ASSUMPTION(!tc.motion_object_location->frame_folder.empty() &&
                       tc.motion_object_location->frame_folder.back() == '/');
            tc.motion_object_location->frame.set_origin(read_vector3(get_ptree("origin", location)));
            tc.motion_object_location->frame.set_orientation(read_quaternion(get_ptree("orientation", location)));
            for (auto const&  empty_and_rel_path : get_ptree_or_empty("disable_colliding_with", location))
            {
                std::string  rel_path = get_value<std::string>(empty_and_rel_path.second);
                ASSUMPTION(!rel_path.empty());
                if (rel_path.back() == '/')
                    rel_path += com::to_string(com::OBJECT_KIND::COLLIDER);
                tc.motion_object_location->relative_paths_to_colliders_for_disable_colliding.push_back(rel_path);
            }

            tc.aabb_alignment = AABB_ALIGNMENT::Z_LO; // 'aabb_alignment' is not used, when 'motion_object_location' is specified.
        }
        else
        {
            tc.motion_object_location = nullptr;
            tc.aabb_alignment = aabb_alignment_from_string(get_value<std::string>("aabb_alignment", name_and_props.second, &defaults));
        }
        tc.is_available_only_in_action_end = get_value<bool>("is_available_only_in_action_end", false, name_and_props.second);
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
    return  m_end_interpolation_time - m_start_time < 0.0001f ||  m_current_time >= m_end_interpolation_time ?
                1.0f : (m_current_time - m_start_time) / (m_end_interpolation_time - m_start_time);
}


void  agent_action::apply_effects(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    // TODO!
}


void  agent_action::update_time(float_32_bit const  time_step_in_seconds)
{
    if (m_current_time >= m_end_time)
        return;

    m_current_time += (is_ghost_complete() ? state_variables().at("animation_speed").get_value() : 1.0f) * time_step_in_seconds;
    m_current_time += m_context->time_buffer;
    m_context->time_buffer = 0.0f;

    if (m_current_time > m_end_time)
    {
        m_context->time_buffer += m_current_time - m_end_time;
        m_current_time = m_end_time;
    }
    else
        m_context->time_buffer = 0.0f;
}


void  agent_action::update_skeleton_sync()
{
    INVARIANT(
        ctx().parent_frame(binding().frame_guid_of_skeleton) == binding().frame_guid_of_motion_object &&
        ctx().parent_frame(binding().frame_guid_of_skeleton_sync_target) == binding().frame_guid_of_motion_object
        );

    angeo::coordinate_system  result;
    if (ctx().parent_frame(binding().frame_guid_of_skeleton_sync_source) == binding().frame_guid_of_motion_object)
        angeo::interpolate_spherical(
                ctx().frame_coord_system(binding().frame_guid_of_skeleton_sync_source),
                ctx().frame_coord_system(binding().frame_guid_of_skeleton_sync_target),
                interpolation_parameter(),
                result
                );
    else
    {
        angeo::coordinate_system  in_world;
        angeo::interpolate_spherical(
                ctx().frame_coord_system_in_world_space(binding().frame_guid_of_skeleton_sync_source),
                ctx().frame_coord_system_in_world_space(binding().frame_guid_of_skeleton_sync_target),
                interpolation_parameter(),
                in_world
                );
        angeo::to_coordinate_system(ctx().frame_coord_system_in_world_space(binding().frame_guid_of_motion_object), in_world, result);
    }
    ctx().request_relocate_frame(binding().frame_guid_of_skeleton, result);
}


void  agent_action::update_look_at(float_32_bit const  time_step_in_seconds, transition_info const* const  info_ptr)
{
    float_32_bit const  longitude = 0.5f * PI() * (desire().look_at.longitude - 1.0f);
    float_32_bit const  altitude = 0.5f * PI() * desire().look_at.altitude;
    float_32_bit const  magnitude = (0.5f * (1.0f + desire().look_at.magnitude)) *
                                    myself().get_sight_controller().get_camera()->far_plane();
    float_32_bit const  cos_altitude = std::cosf(desire().look_at.altitude);
    vector3 const  target_in_skeleton_space = {
        magnitude * cos_altitude * std::cosf(longitude),
        magnitude * cos_altitude * std::sinf(longitude),
        magnitude * std::sinf(altitude),
    };
    m_context->look_at.interpolate(
            time_step_in_seconds,
            target_in_skeleton_space,
            info_ptr == nullptr ? m_context->animate.get_current_frames_ref() : m_context->animate.get_target_frames_ref(),
            motion_templates()
            );
}


void  agent_action::update_aim_at(float_32_bit const  time_step_in_seconds, transition_info const* const  info_ptr)
{
    // TODO!
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


bool  agent_action::collect_transition_info(agent_action const* const  from_action_ptr, transition_info&  info) const
{
    ASSUMPTION(from_action_ptr != this);

    if (from_action_ptr != nullptr && from_action_ptr->TRANSITIONS.at(NAME).is_available_only_in_action_end && !from_action_ptr->is_complete())
        return false;

    if (!collect_other_entiry_folder_guid(from_action_ptr, info))
        return false;

    collect_motion_object_relocation_frame(from_action_ptr, info);

    angeo::coordinate_system const  relocated_agent_frame {
        angeo::point3_from_coordinate_system(-MOTION_OBJECT_CONFIG.offset_from_center_of_agent_aabb, info.motion_object_relocation_frame),
        info.motion_object_relocation_frame.orientation()
    };

    vector3 const  aabb_half_size = {
            // TODO: Load the coord scales from action's config JSON.
            0.75f * AABB_HALF_SIZE(0),
            0.75f * AABB_HALF_SIZE(1),
            0.75f * AABB_HALF_SIZE(2)
            }; 

    std::shared_ptr<transition_config::motion_object_location_config> const  pos_cfg =
            from_action_ptr == nullptr ? nullptr : from_action_ptr->TRANSITIONS.at(NAME).motion_object_location;

    // Fast check whether the aabb in the relocated frame is inside aabb at the original location.
    if (from_action_ptr != nullptr &&
        pos_cfg == nullptr &&
        aabb_half_size(0) <= from_action_ptr->AABB_HALF_SIZE(0) &&
        aabb_half_size(1) <= from_action_ptr->AABB_HALF_SIZE(1) &&
        aabb_half_size(2) <= from_action_ptr->AABB_HALF_SIZE(2) &&
        are_equal(
                relocated_agent_frame.orientation(),
                ctx().frame_coord_system_in_world_space(binding().frame_guid_of_motion_object).orientation(),
                0.001f
                )
        )
        return true;

    std::unordered_set<com::object_guid>  ignored_collider_guids;
    if (from_action_ptr != nullptr)
        from_action_ptr->get_colliders_to_be_ignored_in_empty_space_check(ignored_collider_guids);
    if (pos_cfg != nullptr)
        for (std::string const&  rel_path : pos_cfg->relative_paths_to_colliders_for_disable_colliding)
        {
            com::object_guid const  collider_guid = ctx().from_relative_path(info.other_entiry_folder_guid, rel_path);
            INVARIANT(ctx().is_valid_collider_guid(collider_guid));
            ignored_collider_guids.insert(collider_guid);
        }

    return is_empty_space(relocated_agent_frame, aabb_half_size, MOTION_OBJECT_CONFIG.shape_type, ignored_collider_guids);
}


bool  agent_action::collect_other_entiry_folder_guid(agent_action const* const  from_action_ptr, transition_info&  info) const
{
    info.other_entiry_folder_guid = com::invalid_object_guid();
    if (from_action_ptr == nullptr)
        return true;

    transition_config const&  config = from_action_ptr->TRANSITIONS.at(NAME);
    if (config.perception_guard == nullptr)
        return true;
    transition_config::perception_guard_config const  percept = *config.perception_guard;

    if (percept.perception_kind == transition_config::PERCEPTION_KIND::SIGHT)
    {
        std::unordered_set<com::object_guid>  visited_collider_guids;
        sight_controller::ray_casts_in_time const&  ray_casts = myself().get_sight_controller().get_directed_ray_casts_in_time();
        for (auto  ray_it = ray_casts.begin(), end = ray_casts.end(); ray_it != end; ++ray_it)
        {
            com::object_guid const  collider_guid = ray_it->second.collider_guid;
            if (!ctx().is_valid_collider_guid(collider_guid))
                continue;
            if (visited_collider_guids.count(collider_guid) != 0UL)
                continue;
            visited_collider_guids.insert(collider_guid);

            com::object_guid const  owner_guid = ctx().owner_of_collider(collider_guid);
            if (owner_guid.kind != percept.sensor_owner_kind)
                continue;

            if (owner_guid.kind == com::OBJECT_KIND::AGENT && !percept.sensor_owner_can_be_myself &&
                    ctx().folder_of_agent(owner_guid) == binding().folder_guid_of_agent)
                continue;

            std::string const&  folder_name = ctx().name_of_folder(ctx().folder_of_collider(collider_guid));
            if (folder_name != percept.sensor_folder_name)
                continue;

            vector3  contact_point_in_constraint_space;
            {
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
                contact_point_in_constraint_space =
                        angeo::point3_to_coordinate_system(contact_point_in_motion_object_space, percept.location_constraint.frame);
            }

            switch(percept.location_constraint.shape_type)
            {
            case angeo::COLLISION_SHAPE_TYPE::BOX:
                if (!angeo::collision_point_and_bbox(
                        contact_point_in_constraint_space,
                        -percept.location_constraint.aabb_half_size,
                        percept.location_constraint.aabb_half_size
                        ))
                    continue;
                break;
            case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
                if (!angeo::is_point_inside_capsule(
                        contact_point_in_constraint_space,
                        max_coord(percept.location_constraint.aabb_half_size) - min_coord(percept.location_constraint.aabb_half_size),
                        min_coord(percept.location_constraint.aabb_half_size)
                        ))
                    continue;
                break;
            case angeo::COLLISION_SHAPE_TYPE::SPHERE:
                if (!angeo::is_point_inside_sphere(contact_point_in_constraint_space, min_coord(percept.location_constraint.aabb_half_size)))
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


void  agent_action::collect_motion_object_relocation_frame(agent_action const* const  from_action_ptr, transition_info&  info) const
{
    if (from_action_ptr == nullptr)
    {
        matrix44  W = ctx().frame_world_matrix(binding().frame_guid_of_motion_object);
        matrix44  F;
        angeo::from_base_matrix(motion_templates().at(MOTION_TEMPLATE_NAME).reference_frames.at(0), F);
        vector3  pos;
        matrix33  rot;
        decompose_matrix44(W * F, pos, rot);
        info.motion_object_relocation_frame.set_origin(pos);
        info.motion_object_relocation_frame.set_orientation(rotation_matrix_to_quaternion(rot));
        return;
    }

    ASSUMPTION(from_action_ptr != this);

    info.motion_object_relocation_frame = ctx().frame_coord_system_in_world_space(binding().frame_guid_of_motion_object);

    std::shared_ptr<transition_config::motion_object_location_config> const  pos_cfg =
            from_action_ptr->TRANSITIONS.at(NAME).motion_object_location;
    if (pos_cfg != nullptr)
    {
        com::object_guid const  frame_guid = get_frame_guid_under_agent_folder(
                pos_cfg->is_self_frame ? binding().folder_guid_of_agent : info.other_entiry_folder_guid,
                pos_cfg->frame_folder,
                ctx()
                );
        angeo::from_coordinate_system(
                ctx().frame_coord_system_in_world_space(frame_guid),
                pos_cfg->frame,
                info.motion_object_relocation_frame
                );
    }
    else if (!are_equal_3d(AABB_HALF_SIZE, from_action_ptr->AABB_HALF_SIZE, 0.001f) ||
                from_action_ptr->MOTION_OBJECT_CONFIG != MOTION_OBJECT_CONFIG)
    {
        auto const  get_shift_vector =
            [](AABB_ALIGNMENT const  alignment, vector3 const&  aabb, vector3 const&  aabb_aligned) -> vector3 {
                natural_32_bit  coord_idx;
                float_32_bit  sign;
                switch (alignment)
                {
                case AABB_ALIGNMENT::CENTER: coord_idx = 0U; sign = 0.0f; break;
                case AABB_ALIGNMENT::X_LO: coord_idx = 0U; sign = -1.0f; break;
                case AABB_ALIGNMENT::X_HI: coord_idx = 0U; sign = 1.0f; break;
                case AABB_ALIGNMENT::Y_LO: coord_idx = 1U; sign = -1.0f; break;
                case AABB_ALIGNMENT::Y_HI: coord_idx = 1U; sign = 1.0f; break;
                case AABB_ALIGNMENT::Z_LO: coord_idx = 2U; sign = -1.0f; break;
                case AABB_ALIGNMENT::Z_HI: coord_idx = 2U; sign = 1.0f; break;
                default: { UNREACHABLE(); } break;
                }
                return (sign * (aabb(coord_idx) - aabb_aligned(coord_idx))) * vector3_unit(coord_idx);
            };

        vector3 const  from_motion_object_aabb_shift = get_shift_vector(
                from_action_ptr->MOTION_OBJECT_CONFIG.aabb_alignment,
                from_action_ptr->AABB_HALF_SIZE,
                from_action_ptr->MOTION_OBJECT_CONFIG.aabb_half_size
                );
        vector3 const  agent_aabb_shift = get_shift_vector(
                from_action_ptr->TRANSITIONS.at(NAME).aabb_alignment,
                from_action_ptr->AABB_HALF_SIZE,
                AABB_HALF_SIZE
                );
        vector3 const  motion_object_aabb_shift = get_shift_vector(
                MOTION_OBJECT_CONFIG.aabb_alignment,
                AABB_HALF_SIZE,
                MOTION_OBJECT_CONFIG.aabb_half_size
                );

        info.motion_object_relocation_frame.set_origin(
                info.motion_object_relocation_frame.origin()
                - from_motion_object_aabb_shift
                + agent_aabb_shift
                + motion_object_aabb_shift
                );
    }
}


void  agent_action::get_colliders_to_be_ignored_in_empty_space_check(std::unordered_set<com::object_guid>&  ignored_collider_guids) const
{
    ignored_collider_guids.insert(
            ctx().folder_content(binding().folder_guid_of_motion_object).content.at(com::to_string(com::OBJECT_KIND::COLLIDER))
            );
}


bool  agent_action::is_empty_space(
        angeo::coordinate_system const&  frame_in_world_space,
        vector3 const&  aabb_half_size,
        angeo::COLLISION_SHAPE_TYPE const  shape_type,
        std::unordered_set<com::object_guid> const&  ignored_collider_guids
        ) const
{
    auto const  acceptor = [](com::collision_contact const&) -> bool { return false; }; // we need to enumerate only till the first contact.
    auto const  filter = [&ignored_collider_guids](com::object_guid const  collider_guid, angeo::COLLISION_CLASS const cc) -> bool {
        switch (cc)
        {
        case angeo::COLLISION_CLASS::STATIC_OBJECT:
        case angeo::COLLISION_CLASS::COMMON_MOVEABLE_OBJECT:
        case angeo::COLLISION_CLASS::HEAVY_MOVEABLE_OBJECT:
        case angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT:
            return ignored_collider_guids.count(collider_guid) == 0UL;
        default:
            return false;
        }
    };
    matrix44  world_matrix;
    angeo::from_base_matrix(frame_in_world_space, world_matrix);
    switch (shape_type)
    {
    case angeo::COLLISION_SHAPE_TYPE::BOX:
        return ctx().compute_contacts_with_box(aabb_half_size, world_matrix, true, true, acceptor, filter) == 0U;
    case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
        return ctx().compute_contacts_with_capsule(
                    max_coord(aabb_half_size) - min_coord(aabb_half_size),
                    min_coord(aabb_half_size),
                    world_matrix,
                    true,
                    true,
                    acceptor,
                    filter
                    ) == 0U;
    case angeo::COLLISION_SHAPE_TYPE::SPHERE:
        return ctx().compute_contacts_with_sphere(min_coord(aabb_half_size), world_matrix, true, true, acceptor, filter) == 0U;
    default: { UNREACHABLE(); break; }
    }
    return true;
}


void  agent_action::on_transition(agent_action* const  from_action_ptr, transition_info const* const  info_ptr)
{
    TMPROF_BLOCK();

    ASSUMPTION((this != from_action_ptr || is_complete()) && ((this == from_action_ptr) == (info_ptr == nullptr)));

    skeletal_motion_templates::keyframes const  keyframes = motion_templates().at(MOTION_TEMPLATE_NAME).keyframes;

    if (from_action_ptr == nullptr)
    {
        m_start_time = keyframes.start_time_point();
        m_end_time = keyframes.end_time_point();
        m_end_ghost_time = m_start_time;
        m_end_interpolation_time = m_end_ghost_time;
        m_current_time = m_start_time;

        m_animation.end_keyframe_index = (natural_32_bit)keyframes.num_keyframes() - 1U;
        m_animation.last_keyframe_completion_time = m_start_time;
        m_animation.target_keyframe_index = 1U;

        m_context->animate.set_target({MOTION_TEMPLATE_NAME, m_animation.target_keyframe_index}, motion_templates());
    }
    else if (from_action_ptr == this)
    {
        if (IS_CYCLIC && is_complete())
        {
            m_start_time = keyframes.start_time_point();
            m_end_time = keyframes.end_time_point();
            m_end_ghost_time = m_start_time;
            m_end_interpolation_time = m_end_ghost_time;
            m_current_time = m_start_time;

            m_animation.end_keyframe_index = (natural_32_bit)keyframes.num_keyframes() - 1U;
            m_animation.last_keyframe_completion_time = m_start_time;
            m_animation.target_keyframe_index = 1U;

            m_context->animate.set_target({MOTION_TEMPLATE_NAME, m_animation.target_keyframe_index}, motion_templates());
        }
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
        m_end_interpolation_time = m_end_ghost_time;
        m_current_time = m_start_time;

        m_animation.end_keyframe_index = transition_props.first;
        m_animation.last_keyframe_completion_time = m_start_time;
        m_animation.target_keyframe_index = transition_props.first;

        m_context->animate.set_target({MOTION_TEMPLATE_NAME, m_animation.target_keyframe_index}, motion_templates());
        if (IS_LOOK_AT_ENABLED && info_ptr != nullptr)
            update_look_at(m_end_ghost_time - m_start_time, info_ptr);
        if (IS_AIM_AT_ENABLED && info_ptr != nullptr)
            update_aim_at(m_end_ghost_time - m_start_time, info_ptr);
    }
    else if (USE_MOTION_TEMPLATE_FOR_LOCATION_INTERPOLATION)
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
        m_end_interpolation_time = m_end_time;
        m_current_time = m_start_time;

        m_animation.end_keyframe_index = (natural_32_bit)keyframes.num_keyframes() - 1U;
        m_animation.last_keyframe_completion_time = m_start_time;
        m_animation.target_keyframe_index = transition_props.first;

        m_context->animate.set_target({MOTION_TEMPLATE_NAME, m_animation.target_keyframe_index}, motion_templates());
        if (IS_LOOK_AT_ENABLED && info_ptr != nullptr)
            update_look_at(m_end_ghost_time - m_start_time, info_ptr);
        if (IS_AIM_AT_ENABLED && info_ptr != nullptr)
            update_aim_at(m_end_ghost_time - m_start_time, info_ptr);
    }
    else if (MOTION_TEMPLATE_NAME == from_action_ptr->MOTION_TEMPLATE_NAME)
    {
        if (!from_action_ptr->is_complete())
        {
            m_start_time = from_action_ptr->m_current_time;
            m_end_time = keyframes.end_time_point();
            m_end_ghost_time = m_start_time;
            m_end_interpolation_time = m_end_ghost_time;
            m_current_time = m_start_time;

            m_animation.end_keyframe_index = (natural_32_bit)keyframes.num_keyframes() - 1U;
            m_animation.last_keyframe_completion_time = from_action_ptr->m_animation.last_keyframe_completion_time;
            m_animation.target_keyframe_index = from_action_ptr->m_animation.target_keyframe_index;
        }
        else if (IS_CYCLIC)
        {
            m_start_time = keyframes.start_time_point();
            m_end_time = keyframes.end_time_point();
            m_end_ghost_time = m_start_time;
            m_end_interpolation_time = m_end_ghost_time;
            m_current_time = m_start_time;

            m_animation.end_keyframe_index = (natural_32_bit)keyframes.num_keyframes() - 1U;
            m_animation.last_keyframe_completion_time = m_start_time;
            m_animation.target_keyframe_index = 1U;

            m_context->animate.set_target({MOTION_TEMPLATE_NAME, m_animation.target_keyframe_index}, motion_templates());
        }
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
        m_end_interpolation_time = m_end_ghost_time;
        m_current_time = m_start_time;

        m_animation.end_keyframe_index = (natural_32_bit)keyframes.num_keyframes() - 1U;
        m_animation.last_keyframe_completion_time = m_start_time;
        m_animation.target_keyframe_index = transition_props.first;

        m_context->animate.set_target({MOTION_TEMPLATE_NAME, m_animation.target_keyframe_index}, motion_templates());
        if (IS_LOOK_AT_ENABLED && info_ptr != nullptr)
            update_look_at(m_end_ghost_time - m_start_time, info_ptr);
        if (IS_AIM_AT_ENABLED && info_ptr != nullptr)
            update_aim_at(m_end_ghost_time - m_start_time, info_ptr);
    }

    INVARIANT(m_current_time >= m_start_time && m_current_time <= m_end_time);
    INVARIANT(m_animation.last_keyframe_completion_time >= m_start_time && m_animation.last_keyframe_completion_time <= m_end_time);
    INVARIANT(m_current_time >= m_animation.last_keyframe_completion_time);
    INVARIANT(m_end_time > m_start_time);
    INVARIANT(m_end_ghost_time >= m_start_time && m_end_ghost_time <= m_end_time);
    INVARIANT(m_animation.target_keyframe_index < keyframes.num_keyframes());

    INVARIANT(ctx().parent_frame(binding().frame_guid_of_skeleton) == binding().frame_guid_of_motion_object);

    if (from_action_ptr != this)
    {
        // Relocations of frames

        ctx().request_relocate_frame(binding().frame_guid_of_motion_object, info_ptr->motion_object_relocation_frame);
        ctx().request_relocate_frame(
                binding().frame_guid_of_skeleton_sync_target,
                -MOTION_OBJECT_CONFIG.offset_from_center_of_agent_aabb,
                quaternion_identity()
                );

        angeo::coordinate_system  ghost_start_frame;
        if (from_action_ptr != nullptr)
            angeo::to_coordinate_system(
                    info_ptr->motion_object_relocation_frame,
                    ctx().frame_coord_system_in_world_space(binding().frame_guid_of_skeleton),
                    ghost_start_frame
                    );
        if (DEFINE_SKELETON_SYNC_SOURCE_IN_WORLD_SPACE)
        {
            if (ctx().parent_frame(binding().frame_guid_of_skeleton_sync_source) != com::invalid_object_guid())
                ctx().request_set_parent_frame(binding().frame_guid_of_skeleton_sync_source, com::invalid_object_guid());
            ctx().request_relocate_frame(
                    binding().frame_guid_of_skeleton_sync_source,
                    ctx().frame_coord_system_in_world_space(binding().frame_guid_of_skeleton)
                    );
        }
        else
        {
            if (ctx().parent_frame(binding().frame_guid_of_skeleton_sync_source) != binding().frame_guid_of_motion_object)
                ctx().request_set_parent_frame(binding().frame_guid_of_skeleton_sync_source, binding().frame_guid_of_motion_object);
            ctx().request_relocate_frame(binding().frame_guid_of_skeleton_sync_source, ghost_start_frame);
        }
        ctx().request_relocate_frame(binding().frame_guid_of_skeleton, ghost_start_frame);
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
            linear_acceleration = ctx().initial_linear_acceleration_of_rigid_body(rigid_body_guid);
            angular_acceleration = ctx().initial_angular_acceleration_of_rigid_body(rigid_body_guid);

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

    if (from_action_ptr != nullptr && from_action_ptr != this)
    {
        while (!m_context->disabled_colliding_with_our_motion_object.empty())
        {
            action_execution_context::scene_object_relative_path const&  rel_path =
                    m_context->disabled_colliding_with_our_motion_object.back();
            if (ctx().is_valid_collider_guid(ctx().from_relative_path(rel_path.base_folder_guid, rel_path.relative_path)))
                ctx().request_enable_colliding(
                            binding().folder_guid_of_motion_object, com::to_string(com::OBJECT_KIND::COLLIDER),
                            rel_path.base_folder_guid, rel_path.relative_path,
                            true
                            );
            m_context->disabled_colliding_with_our_motion_object.pop_back();
        }

        std::shared_ptr<transition_config::motion_object_location_config> const  pos_cfg =
                from_action_ptr->TRANSITIONS.at(NAME).motion_object_location;
        if (pos_cfg != nullptr)
            for (std::string const&  rel_path : pos_cfg->relative_paths_to_colliders_for_disable_colliding)
            {
                INVARIANT(info_ptr->other_entiry_folder_guid != com::invalid_object_guid());
                m_context->disabled_colliding_with_our_motion_object.push_back({ info_ptr->other_entiry_folder_guid, rel_path });
                ctx().request_enable_colliding(
                        binding().folder_guid_of_motion_object, com::to_string(com::OBJECT_KIND::COLLIDER),
                        info_ptr->other_entiry_folder_guid, rel_path,
                        false
                        );
            }
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
    update_skeleton_sync();
    update_animation(time_step_in_seconds);
    if (IS_LOOK_AT_ENABLED && is_ghost_complete())
        update_look_at(time_step_in_seconds);
    if (IS_AIM_AT_ENABLED && is_ghost_complete())
        update_aim_at(time_step_in_seconds);
    m_context->animate.commit(motion_templates(), binding());
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


void  action_guesture::on_transition(agent_action* const  from_action_ptr, transition_info const* const  info_ptr)
{
    agent_action::on_transition(from_action_ptr, info_ptr);

    ctx().request_set_rigid_body_angular_velocity(
            binding().folder_guid_of_motion_object,
            com::to_string(com::OBJECT_KIND::RIGID_BODY),
            vector3_zero()
            );
    state_variables().at("animation_speed").set_value(1.0f);
}


void  action_guesture::next_round(float_32_bit const  time_step_in_seconds)
{
    agent_action::next_round(time_step_in_seconds);
}


action_roller::action_roller(
        std::string const&  name_,
        boost::property_tree::ptree const&  ptree_,
        boost::property_tree::ptree const&  defaults_,
        action_execution_context_ptr const  context_
        )
    : agent_action(name_, ptree_.find("agent_action")->second, defaults_, context_)
    , ROLLER_CONFIG() // loaded below
    , m_roller_folder_guid(com::invalid_object_guid())
    , m_roller_frame_guid(com::invalid_object_guid())
    , m_roller_joint_ccids{
            angeo::invalid_custom_constraint_id(),
            angeo::invalid_custom_constraint_id(),
            angeo::invalid_custom_constraint_id()
            }
    , m_desire_move_forward_to_linear_speed{{ { -1.0f, 0.0f }, { 1.0f, 0.0f } }}  // loaded below
    , m_desire_move_left_to_linear_speed{{ { -1.0f, 0.0f }, { 1.0f, 0.0f } }}  // loaded below
    , m_desire_move_turn_ccw_to_angular_speed{{ { -1.0f, 0.0f }, { 1.0f, 0.0f } }}  // loaded below
    , m_angular_speed_to_animation_speed{{ { -1.0f, 1.0f }, { 1.0f, 1.0f } }} // loaded below
    , m_animation_speed_subject(ANIMATION_SPEED_SUBJECT::ROLLER) // loaded below
{
    ASSUMPTION(max_coord(MOTION_OBJECT_CONFIG.aabb_half_size) == MOTION_OBJECT_CONFIG.aabb_half_size(2));

    float_32_bit const  total = 2.0f * MOTION_OBJECT_CONFIG.aabb_half_size(2);
    float_32_bit const  part = min_coord(MOTION_OBJECT_CONFIG.aabb_half_size);
    ASSUMPTION(part > 0.0001f && total - part > 0.0001f);
    float_32_bit const  fraction = part / total + 0.5f * (1.0f - part / total) * part / (total - part);
    ASSUMPTION(fraction > 0.0001f && fraction < 0.9999f);
    float_32_bit const  mass = MOTION_OBJECT_CONFIG.mass_inverted < 0.000001f || MOTION_OBJECT_CONFIG.mass_inverted > 1000000.0f ?
                                    65.0f : 1.0f / MOTION_OBJECT_CONFIG.mass_inverted;

    ROLLER_CONFIG.roller_radius = part;
    ROLLER_CONFIG.roller_mass_inverted = 1.0f / (mass * fraction);

    MOTION_OBJECT_CONFIG.aabb_half_size(2) -= 0.5f * part;
    ASSUMPTION(
            MOTION_OBJECT_CONFIG.aabb_half_size(2) > 0.0001f &&
            MOTION_OBJECT_CONFIG.aabb_half_size(2) > 2.0f * ROLLER_CONFIG.roller_radius
            );
    MOTION_OBJECT_CONFIG.mass_inverted = 1.0f / (mass * (1.0f - fraction));
    MOTION_OBJECT_CONFIG.inertia_tensor_inverted = matrix33_zero();
    MOTION_OBJECT_CONFIG.is_moveable = true;
    MOTION_OBJECT_CONFIG.offset_from_center_of_agent_aabb = 0.5f * ROLLER_CONFIG.roller_radius * vector3_unit_z();

    auto  ptree_it = ptree_.find("desire_move_forward_to_linear_speed");
    if (ptree_it != ptree_.not_found())
        angeo::load(m_desire_move_forward_to_linear_speed, ptree_it->second);

    ptree_it = ptree_.find("desire_move_left_to_linear_speed");
    if (ptree_it != ptree_.not_found())
        angeo::load(m_desire_move_left_to_linear_speed, ptree_it->second);

    ptree_it = ptree_.find("desire_move_turn_ccw_to_angular_speed");
    if (ptree_it != ptree_.not_found())
        angeo::load(m_desire_move_turn_ccw_to_angular_speed, ptree_it->second);

    ptree_it = ptree_.find("roller_linear_speed_to_animation_speed");
    if (ptree_it != ptree_.not_found())
    {
        angeo::load(m_angular_speed_to_animation_speed, ptree_it->second);
        for (vector2&  point : m_angular_speed_to_animation_speed.points)
            point(0) /= ROLLER_CONFIG.roller_radius;
        m_animation_speed_subject = ANIMATION_SPEED_SUBJECT::ROLLER;
    }
    else
    {
        ptree_it = ptree_.find("motion_object_angular_speed_to_animation_speed");
        if (ptree_it != ptree_.not_found())
            angeo::load(m_angular_speed_to_animation_speed, ptree_it->second);
        m_animation_speed_subject = ANIMATION_SPEED_SUBJECT::MOTION_OBJECT;
    }
}


bool  action_roller::roller_object_config::operator==(roller_object_config const&  other) const
{
    return  are_equal(roller_radius, other.roller_radius, 0.001f) &&
            are_equal(roller_mass_inverted, other.roller_mass_inverted, 0.001f);
}


void  action_roller::get_colliders_to_be_ignored_in_empty_space_check(std::unordered_set<com::object_guid>&  ignored_collider_guids) const
{
    agent_action::get_colliders_to_be_ignored_in_empty_space_check(ignored_collider_guids);
    ignored_collider_guids.insert(
            ctx().folder_content(m_roller_folder_guid).content.at(com::to_string(com::OBJECT_KIND::COLLIDER))
            );
}


void  action_roller::on_transition(agent_action* const  from_action_ptr, transition_info const* const  info_ptr)
{
    TMPROF_BLOCK();

    agent_action::on_transition(from_action_ptr, info_ptr);

    if (from_action_ptr == this)
        return;

    action_roller* const  from_roller_ptr = from_action_ptr == nullptr ? nullptr : dynamic_cast<action_roller*>(from_action_ptr);

    if (from_roller_ptr == nullptr)
    {
        m_roller_folder_guid = ctx().insert_folder(binding().folder_guid_of_agent, "roller", false);
        m_roller_frame_guid = ctx().insert_frame(
                m_roller_folder_guid,
                com::invalid_object_guid(),
                info_ptr->motion_object_relocation_frame.origin()
                        - max_coord(MOTION_OBJECT_CONFIG.aabb_half_size) * angeo::axis_z(info_ptr->motion_object_relocation_frame),
                info_ptr->motion_object_relocation_frame.orientation()
                );
        ctx().request_insert_rigid_body(
                m_roller_folder_guid,
                true,
                vector3_zero(),
                vector3_zero(),
                vector3_zero(),
                vector3_zero(),
                ROLLER_CONFIG.roller_mass_inverted,
                matrix33_zero()
                );
        ctx().request_insert_collider_sphere(
                m_roller_folder_guid,
                com::to_string(com::OBJECT_KIND::COLLIDER),
                ROLLER_CONFIG.roller_radius,
                MOTION_OBJECT_CONFIG.collision_material,
                angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT
                );
        ctx().request_enable_colliding(
                binding().folder_guid_of_motion_object, com::to_string(com::OBJECT_KIND::COLLIDER),
                m_roller_folder_guid, com::to_string(com::OBJECT_KIND::COLLIDER),
                false
                );
        create_custom_constraint_ids();

        return;
    }

    m_roller_folder_guid = from_roller_ptr->m_roller_folder_guid;
    m_roller_frame_guid = from_roller_ptr->m_roller_frame_guid;
    for (natural_32_bit  i = 0U, n = (natural_32_bit)m_roller_joint_ccids.size(); i != n; ++i)
    {
        INVARIANT(
            m_roller_joint_ccids.at(i) == angeo::invalid_custom_constraint_id() &&
            from_roller_ptr->m_roller_joint_ccids.at(i) != angeo::invalid_custom_constraint_id()
            );
        std::swap(m_roller_joint_ccids.at(i), from_roller_ptr->m_roller_joint_ccids.at(i));
    }

    if (get_motion_object_config() != from_roller_ptr->get_motion_object_config() || ROLLER_CONFIG != from_roller_ptr->ROLLER_CONFIG)
    {
        vector3  linear_velocity, angular_velocity, linear_acceleration, angular_acceleration;
        {
            com::object_guid const  rigid_body_guid =
                    ctx().folder_content(m_roller_folder_guid).content.at(com::to_string(com::OBJECT_KIND::RIGID_BODY));
            com::object_guid const  collider_guid =
                    ctx().folder_content(m_roller_folder_guid).content.at(com::to_string(com::OBJECT_KIND::COLLIDER));

            ASSUMPTION(ctx().is_valid_rigid_body_guid(rigid_body_guid) && ctx().is_valid_collider_guid(collider_guid));

            linear_velocity = ctx().linear_velocity_of_rigid_body(rigid_body_guid);
            angular_velocity = ctx().angular_velocity_of_rigid_body(rigid_body_guid);
            linear_acceleration = ctx().initial_linear_acceleration_of_rigid_body(rigid_body_guid);
            angular_acceleration = ctx().initial_angular_acceleration_of_rigid_body(rigid_body_guid);

            ctx().request_erase_collider(collider_guid);
            ctx().request_erase_rigid_body(rigid_body_guid);
        }

        ctx().request_insert_rigid_body(
                m_roller_folder_guid,
                true,
                linear_velocity,
                angular_velocity,
                linear_acceleration,
                angular_acceleration,
                ROLLER_CONFIG.roller_mass_inverted,
                matrix33_zero()
                );
        ctx().request_insert_collider_sphere(
                m_roller_folder_guid,
                com::to_string(com::OBJECT_KIND::COLLIDER),
                ROLLER_CONFIG.roller_radius,
                MOTION_OBJECT_CONFIG.collision_material,
                angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT
                );
        ctx().request_enable_colliding(
                binding().folder_guid_of_motion_object, com::to_string(com::OBJECT_KIND::COLLIDER),
                m_roller_folder_guid, com::to_string(com::OBJECT_KIND::COLLIDER),
                false
                );

        INVARIANT([this]() -> bool {
            for (angeo::custom_constraint_id const  ccid : m_roller_joint_ccids)
                if (ccid == angeo::invalid_custom_constraint_id())
                    return false;
            return true;
            }());
    }

    ctx().request_relocate_frame(
            m_roller_frame_guid,
            info_ptr->motion_object_relocation_frame.origin()
                    - max_coord(MOTION_OBJECT_CONFIG.aabb_half_size) * angeo::axis_z(info_ptr->motion_object_relocation_frame),
            info_ptr->motion_object_relocation_frame.orientation()
            );
}


void  action_roller::next_round(float_32_bit const  time_step_in_seconds)
{
    agent_action::next_round(time_step_in_seconds);

    insert_joint_between_roller_and_motion_object();

    angeo::coordinate_system_explicit const&  motion_object_frame =
            ctx().frame_explicit_coord_system_in_world_space(binding().frame_guid_of_motion_object);

    vector3 const  motion_object_angular_velocity =
            m_desire_move_turn_ccw_to_angular_speed(desire().move.turn_ccw) * motion_object_frame.basis_vector_z();
    ctx().request_set_rigid_body_angular_velocity(
            ctx().folder_content(binding().folder_guid_of_motion_object).content.at(com::to_string(com::OBJECT_KIND::RIGID_BODY)),
            motion_object_angular_velocity
            );

    vector3 const  roller_angular_velocity =
            (m_desire_move_forward_to_linear_speed(desire().move.forward) / ROLLER_CONFIG.roller_radius)
                * motion_object_frame.basis_vector_x();
    ctx().request_set_rigid_body_angular_velocity(
            ctx().folder_content(m_roller_folder_guid).content.at(com::to_string(com::OBJECT_KIND::RIGID_BODY)),
            roller_angular_velocity
            );

    vector3  angular_velocity_for_animation_speed;
    switch (m_animation_speed_subject)
    {
    case ANIMATION_SPEED_SUBJECT::ROLLER: angular_velocity_for_animation_speed = roller_angular_velocity; break;
    case ANIMATION_SPEED_SUBJECT::MOTION_OBJECT: angular_velocity_for_animation_speed = motion_object_angular_velocity; break;
    default: { UNREACHABLE(); } break;
    }
    state_variables().at("animation_speed").set_value(m_angular_speed_to_animation_speed(length(angular_velocity_for_animation_speed)));
}


void  action_roller::get_custom_folders(std::unordered_map<com::object_guid, on_custom_folder_erase_func>&  folders)
{
    folders.insert({ m_roller_folder_guid, [this](){ release_custom_constraint_ids(); }});
}


void  action_roller::create_custom_constraint_ids()
{
    for (angeo::custom_constraint_id&  ccid : m_roller_joint_ccids)
    {
        INVARIANT(ccid == angeo::invalid_custom_constraint_id());
        ccid = ctx().acquire_fresh_custom_constraint_id_from_physics();
    }
}


void  action_roller::release_custom_constraint_ids()
{
    for (angeo::custom_constraint_id&  ccid : m_roller_joint_ccids)
    {
        INVARIANT(ccid != angeo::invalid_custom_constraint_id());
        ctx().release_acquired_custom_constraint_id_back_to_physics(ccid);
        ccid = angeo::invalid_custom_constraint_id();
    }
}


void  action_roller::insert_joint_between_roller_and_motion_object() const
{
    angeo::coordinate_system_explicit const&  motion_object_frame =
            ctx().frame_explicit_coord_system_in_world_space(binding().frame_guid_of_motion_object);
    vector3 const  roller_joint = ctx().frame_coord_system_in_world_space(m_roller_frame_guid).origin();
    vector3 const  body_joint =
            motion_object_frame.origin() - max_coord(MOTION_OBJECT_CONFIG.aabb_half_size) * motion_object_frame.basis_vector_z();
    vector3 const  joint_delta = body_joint - roller_joint;
    vector3 const  joint = roller_joint + 0.5f * joint_delta;
    float_32_bit const  joint_separation_distance = length(joint_delta);
    vector3  unit_constraint_vector[3];
    {
        if (joint_separation_distance < 0.0001f)
            unit_constraint_vector[0] = vector3_unit_z();
        else
            unit_constraint_vector[0] = (1.0f / joint_separation_distance) * joint_delta;
        angeo::compute_tangent_space_of_unit_vector(
                unit_constraint_vector[0],
                unit_constraint_vector[1],
                unit_constraint_vector[2]
                );
    }
    float_32_bit const  bias[3] {
            10.0f * joint_separation_distance,
            0.0f,
            0.0f
            };
    for (natural_32_bit i = 0U; i != 3U; ++i)
        ctx().request_early_insertion_of_custom_constraint_to_physics(
                m_roller_joint_ccids[i],
                ctx().folder_content(m_roller_folder_guid).content.at(com::to_string(com::OBJECT_KIND::RIGID_BODY)),
                unit_constraint_vector[i],
                cross_product(joint - roller_joint, unit_constraint_vector[i]),
                ctx().folder_content(binding().folder_guid_of_motion_object).content.at(com::to_string(com::OBJECT_KIND::RIGID_BODY)),
                -unit_constraint_vector[i],
                -cross_product(joint - motion_object_frame.origin(), unit_constraint_vector[i]),
                bias[i],
                -std::numeric_limits<float_32_bit>::max(),
                std::numeric_limits<float_32_bit>::max(),
                0.0f
                );
}


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
        else if (it->first == "action_roller")
            m_available_actions[name_and_ptree.first] =
                    std::make_shared<action_roller>(name_and_ptree.first, it->second, config.defaults(), m_context);        
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
 }


action_controller::~action_controller()
{}


void  action_controller::initialise()
{
    agent_action::transition_info  info;
    m_current_action->collect_transition_info(nullptr, info);
    m_current_action->on_transition(nullptr, &info);
    m_context->animate.move_to_target();
    m_context->animate.commit(m_context->motion_templates(), m_context->binding());
}


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
    {
        best_action = m_current_action;
        float_32_bit  best_penalty = m_current_action->compute_desire_penalty();
        for (auto const&  name_and_props : m_current_action->get_transitions())
        {
            std::shared_ptr<agent_action> const  action_ptr = m_available_actions.at(name_and_props.first);
            float_32_bit const  penalty = action_ptr->compute_desire_penalty();
            if (best_action == nullptr || penalty < best_penalty)
            {
                best_action = action_ptr;
                best_penalty = penalty;
            }
        }
    }

    agent_action::transition_info  info;
    if (m_current_action != best_action && !best_action->collect_transition_info(&*m_current_action, info))
        best_action = m_current_action;

    if (m_current_action != best_action || m_current_action->is_complete())
    {
        best_action->on_transition(&*m_current_action, m_current_action == best_action ? nullptr : &info);
        if (m_current_action != best_action)
        {
            std::unordered_map<com::object_guid, agent_action::on_custom_folder_erase_func>  old_custom_folders, new_custom_folders;
            m_current_action->get_custom_folders(old_custom_folders);
            best_action->get_custom_folders(new_custom_folders);
            for (auto const&  guid_and_func : old_custom_folders)
                if (new_custom_folders.count(guid_and_func.first) == 0UL)
                {
                    if (guid_and_func.second.operator bool()) // Do we have a valid function to call?
                        guid_and_func.second(); // Erase/release data related to the erased folder 'guid_and_func.first'.
                    m_context->ctx().request_erase_non_root_folder(guid_and_func.first);
                }
        }
        m_current_action = best_action;
    }
}


}
