#include <ai/action_controller.hpp>
#include <ai/skeleton_utils.hpp>
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

namespace ai {


action_execution_context::action_execution_context(
        agent_state_variables_ptr  state_variables_,
        skeletal_motion_templates  motion_templates_,
        scene_binding_ptr  binding_
        )
    : state_variables(state_variables_)
    , motion_templates(motion_templates_)
    , animate()
    , look_at()
    , aim_at()
    , binding(binding_)
    , time_buffer(0.0f)
{}


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
    , TRANSITIONS() // loaded below
    , MOTION_OBJECT_CONFIG() // loaded below
    // MUTABLE DATA
    , m_context(context_)
    , m_start_time(0.0f)
    , m_end_time(0.0f)
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
    for (auto const&  item : ptree_.find("TRANSITIONS")->second)
        TRANSITIONS.push_back(item.second.get_value<std::string>());
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
        {
            MOTION_OBJECT_CONFIG.aabb_half_size(0) = p->get<float_32_bit>("x");
            MOTION_OBJECT_CONFIG.aabb_half_size(1) = p->get<float_32_bit>("y");
            MOTION_OBJECT_CONFIG.aabb_half_size(2) = p->get<float_32_bit>("z");
        }
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


bool  agent_action::is_guard_valid() const
{
    // TODO!
    return true;
}


float_32_bit  agent_action::compute_desire_penalty(motion_desire_props const&  props) const
{
    std::vector<float_32_bit>  props_vec, ideal_vec, weights_vec;
    as_vector(props, props_vec);
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


bool  agent_action::interpolation_parameter() const
{
    return  (m_current_time - m_end_time) / (m_start_time - m_end_time);
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


void  agent_action::update_ghost_object_frame()
{
    angeo::coordinate_system  result;
    angeo::interpolate_spherical(
            m_ghost_object_start_coord_system,
            ctx().frame_coord_system(binding().frame_guid_of_motion_object),
            interpolation_parameter(),
            result
            );
    ctx().request_relocate_frame(binding().frame_guid_of_ghost_object, result.origin(), result.orientation());
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

    m_context->animate.commit(motion_templates(), m_context->binding);
}


void  agent_action::on_transition(agent_action* const  from_action_ptr)
{
    TMPROF_BLOCK();

    ASSUMPTION(this != from_action_ptr || is_complete());

    if (from_action_ptr == nullptr)
    {
        matrix44  W = ctx().frame_world_matrix(binding().frame_guid_of_motion_object);
        matrix44  F;
        angeo::from_base_matrix(motion_templates().at(MOTION_TEMPLATE_NAME).reference_frames.at(0), F);
        vector3  pos;
        matrix33  rot;
        decompose_matrix44(W * F, pos, rot);
        ctx().request_relocate_frame(binding().frame_guid_of_motion_object, pos, rotation_matrix_to_quaternion(rot));
    }

    {
        com::object_guid const  skeleton_parent_frame_guid = ctx().parent_frame(binding().frame_guid_of_skeleton);
        if (USE_GHOST_OBJECT_FOR_SKELETON_LOCATION)
        {
            if (from_action_ptr != nullptr)
            {
                INVARIANT(
                    ctx().is_valid_frame_guid(skeleton_parent_frame_guid) &&
                    (skeleton_parent_frame_guid == binding().frame_guid_of_ghost_object ||
                     skeleton_parent_frame_guid == binding().frame_guid_of_motion_object)
                    );
                m_ghost_object_start_coord_system = ctx().frame_coord_system(skeleton_parent_frame_guid);
            }
            if (skeleton_parent_frame_guid != binding().frame_guid_of_ghost_object)
                ctx().request_set_parent_frame(binding().frame_guid_of_skeleton, binding().frame_guid_of_ghost_object);
        }
        else if (skeleton_parent_frame_guid != binding().frame_guid_of_motion_object)
            ctx().request_set_parent_frame(binding().frame_guid_of_skeleton, binding().frame_guid_of_motion_object);
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

        switch (MOTION_OBJECT_CONFIG.shape_type)
        {
        case angeo::COLLISION_SHAPE_TYPE::BOX:
            ctx().request_insert_collider_box(
                    binding().folder_guid_of_motion_object,
                    com::to_string(com::OBJECT_KIND::COLLIDER),
                    MOTION_OBJECT_CONFIG.aabb_half_size,
                    MOTION_OBJECT_CONFIG.collision_material,
                    angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT
                    );
            break;
        case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
            ctx().request_insert_collider_capsule(
                    binding().folder_guid_of_motion_object,
                    com::to_string(com::OBJECT_KIND::COLLIDER),
                    max_coord(MOTION_OBJECT_CONFIG.aabb_half_size) - min_coord(MOTION_OBJECT_CONFIG.aabb_half_size),
                    min_coord(MOTION_OBJECT_CONFIG.aabb_half_size),
                    MOTION_OBJECT_CONFIG.collision_material,
                    angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT
                    );
            break;
        case angeo::COLLISION_SHAPE_TYPE::SPHERE:
            ctx().request_insert_collider_sphere(
                    binding().folder_guid_of_motion_object,
                    com::to_string(com::OBJECT_KIND::COLLIDER),
                    min_coord(MOTION_OBJECT_CONFIG.aabb_half_size),
                    MOTION_OBJECT_CONFIG.collision_material,
                    angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT
                    );
            break;
        default: { UNREACHABLE(); break; }
        }
    }

    skeletal_motion_templates::keyframes const  keyframes = motion_templates().at(MOTION_TEMPLATE_NAME).keyframes;

    if (from_action_ptr == nullptr)
    {
        m_start_time = keyframes.start_time_point();
        m_end_time = keyframes.end_time_point();
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
        m_current_time = m_start_time;

        m_animation.end_keyframe_index = (natural_32_bit)keyframes.num_keyframes() - 1U;
        m_animation.last_keyframe_completion_time = from_action_ptr->m_animation.last_keyframe_completion_time;
        m_animation.target_keyframe_index = from_action_ptr->m_animation.target_keyframe_index;
    }
    else if (MOTION_TEMPLATE_NAME == from_action_ptr->MOTION_TEMPLATE_NAME && IS_CYCLIC)
    {
        m_start_time = keyframes.start_time_point();
        m_end_time = keyframes.end_time_point();
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
    INVARIANT(m_animation.target_keyframe_index < keyframes.num_keyframes());
}


void  agent_action::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    apply_effects(time_step_in_seconds);
    update_time(time_step_in_seconds);
    if (USE_GHOST_OBJECT_FOR_SKELETON_LOCATION)
        update_ghost_object_frame();
    update_animation(time_step_in_seconds);
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


void  action_guesture::on_transition(agent_action* const  from_action_ptr)
{
    agent_action::on_transition(from_action_ptr);

    // TODO
}


void  action_guesture::next_round(float_32_bit const  time_step_in_seconds)
{
    agent_action::next_round(time_step_in_seconds);

    // TODO
}


std::unordered_set<com::object_guid>  action_guesture::get_motion_object_collider_guids() const
{
    // TODO
    return {};
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
//    ctx().request_relocate_frame(binding().frame_guid_of_skeleton, result.origin(), result.orientation());
//    agent_action::next_round(time_step_in_seconds);
//}


action_controller::action_controller(
        agent_config const  config,
        agent_state_variables_ptr const  state_variables,
        skeletal_motion_templates const  motion_templates,
        scene_binding_ptr const  binding
        )
    : m_context(std::make_shared<action_execution_context>(state_variables, motion_templates, binding))
    , m_current_action(nullptr) // loaded below
    , m_available_actions() // loaded below
{
    if (m_context->state_variables->count("animation_speed") == 0UL)
        m_context->state_variables->insert({ "animation_speed", { "animation_speed", 1.0f, 0.5f, 2.0f } });
    for (auto const&  name_and_ptree : config.actions())
    {
        ASSUMPTION(name_and_ptree.second->size() == 1UL);
        std::vector<std::string>  class_names;
        auto const it = name_and_ptree.second->begin();
        ASSUMPTION(m_available_actions.count(name_and_ptree.first) == 0UL);
        if (it->first == "agent_action")
            m_available_actions[name_and_ptree.first] =
                    std::make_shared<agent_action>(it->first, it->second, config.defaults(), m_context);
        else if (it->first == "action_guesture")
            m_available_actions[name_and_ptree.first] =
                    std::make_shared<action_guesture>(it->first, it->second, config.defaults(), m_context);        
        else { UNREACHABLE(); }
    }
    ASSUMPTION(
        [this]() -> bool {
            for (auto const&  name_and_action : m_available_actions)
            {
                if (!name_and_action.second->is_cyclic() && name_and_action.second->get_successor_action_names().empty())
                    return false;
                for (std::string const&  action_name : name_and_action.second->get_successor_action_names())
                {
                    if (action_name == name_and_action.first)
                        return false;
                    if (m_available_actions.count(action_name) == 0UL)
                        return false;
                }
            }
            return true;
        }()
    );
    m_current_action = m_available_actions.at(config.initial_action());
    m_current_action->on_transition(nullptr);
    m_context->animate.move_to_target();
    m_context->animate.commit(m_context->motion_templates, m_context->binding);
}


action_controller::~action_controller()
{}


void  action_controller::next_round(
        float_32_bit const  time_step_in_seconds,
        motion_desire_props const&  desire
        )
{
    TMPROF_BLOCK();

    m_current_action->next_round(time_step_in_seconds);

    std::shared_ptr<agent_action>  best_action;
    {
        best_action = nullptr;
        float_32_bit  best_penalty = 0.0f;
        if ((m_current_action->is_cyclic() || !m_current_action->is_complete()) && m_current_action->is_guard_valid())
        {
            best_action = m_current_action;
            best_penalty = m_current_action->compute_desire_penalty(desire);
        }
        for (std::string const&  name : m_current_action->get_successor_action_names())
        {
            std::shared_ptr<agent_action> const  action_ptr = m_available_actions.at(name);
            if (!action_ptr->is_guard_valid())
                continue;
            float_32_bit const  penalty = action_ptr->compute_desire_penalty(desire);
            if (best_action == nullptr || penalty < best_penalty)
            {
                best_action = action_ptr;
                best_penalty = penalty;
            }
        }
    }
    INVARIANT(best_action != nullptr);

    if (m_current_action != best_action || m_current_action->is_complete())
    {
        best_action->on_transition(&*m_current_action);
        m_current_action = best_action;
    }
}


}
