#include <ai/action_controller.hpp>
#include <ai/cortex.hpp>
#include <ai/sensory_controller.hpp>
#include <ai/skeleton_utils.hpp>
#include <ai/detail/rigid_body_motion.hpp>
#include <ai/detail/expression_evaluator.hpp>
#include <ai/detail/ideal_velocity_buider.hpp>
#include <ai/detail/collider_utils.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <angeo/friction_coefficients.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <queue>
#include <functional>

namespace ai { namespace detail { namespace {


natural_32_bit  choose_next_motion_action(
        std::vector<skeletal_motion_templates::transition_info> const&  possibilities,
        blackboard_agent_ptr const  bb,
        eval::context const&  ctx
        )
{
    TMPROF_BLOCK();

    float_32_bit  best_rank = 0.0f;
    natural_32_bit  best_index = (natural_32_bit)possibilities.size();
    for (natural_32_bit  i = 0U; i != possibilities.size(); ++i)
    {
        skeletal_motion_templates::transition_info const&  info = possibilities.at(i);

        float_32_bit const  guard_valid_cost_addon = info.guard->get_child("valid").get_value<float_32_bit>();
        float_32_bit const  guard_invalid_cost_addon = info.guard->get_child("invalid").get_value<float_32_bit>();

        float_32_bit  rank =
                std::fabs(guard_valid_cost_addon - guard_invalid_cost_addon) > 1e-5f &&
                detail::get_satisfied_motion_guarded_actions(
                        bb->m_motion_templates.at(info.cursor.motion_name).actions.at(info.cursor.keyframe_index),
                        bb->m_sensory_controller->get_collision_contacts()->get_collision_contacts_map(),
                        bb->m_action_controller->get_motion_object_motion(),
                        *ctx.desire_props_ptr,
                        bb->m_action_controller->get_environment_linear_velocity(),
                        bb->m_action_controller->get_environment_angular_velocity(),
                        bb->m_action_controller->get_environment_acceleration_coef(),
                        bb->m_action_controller->get_external_linear_acceleration(),
                        bb->m_action_controller->get_external_angular_acceleration(),
                        nullptr,
                        nullptr
                        )
                ? guard_valid_cost_addon : guard_invalid_cost_addon;

        float_32_bit  error = 0.0f;
        float_32_bit  max_error = 0.0f;
        for (auto const&  null_and_child : *info.desire)
        {
            float_32_bit const  expr_value = evaluate_scalar_expression(null_and_child.second.get_child("expression"), ctx);
            float_32_bit const  ideal_value = null_and_child.second.get_child("value").get_value<float_32_bit>();
            error += std::fabs(ideal_value - expr_value);
            max_error += std::max(std::fabs(1.0f - ideal_value), std::fabs(0.0f - ideal_value));
        }

        if (max_error > 0.0f)
            rank += 1.0f - error / max_error;

        if (best_index == possibilities.size() || best_rank < rank)
        {
            best_rank = rank;
            best_index = i;
        }
    }
    INVARIANT(best_index < possibilities.size());
    return best_index;
}


}}}

namespace ai {


action_controller::intepolation_state::intepolation_state()
    : frames()
    , free_bones_look_at(nullptr)
    , collider(nullptr)
    , mass_distribution(nullptr)
    , disjunction_of_guarded_actions()
{}


action_controller::action_controller(blackboard_agent_weak_ptr const  blackboard_)
    : m_motion_object_motion()
    , m_environment_linear_velocity(vector3_zero())
    , m_environment_angular_velocity(vector3_zero())
    , m_environment_acceleration_coef(0.0f)
    , m_external_linear_acceleration(vector3_zero())
    , m_external_angular_acceleration(vector3_zero())

    , m_blackboard(blackboard_)

    , m_interpolator_animation(blackboard_)
    , m_interpolator_look_at(blackboard_)
    , m_interpolator_matter(blackboard_)

    , m_dst_cursor{ get_blackboard()->m_motion_templates.transitions().initial_motion_name(), 0U }

    , m_ideal_linear_velocity_in_world_space(vector3_zero())
    , m_ideal_angular_velocity_in_world_space(vector3_zero())
    , m_motion_action_data()
{
    TMPROF_BLOCK();

    angeo::coordinate_system  agent_frame;
    {
        angeo::coordinate_system  tmp;
        get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_bone_nids.front(), false, tmp);
        agent_frame.set_origin(tmp.origin());
        get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_self_rid.get_node_id(), false, tmp);
        agent_frame.set_orientation(tmp.orientation());
    }
    scene::node_id  nid(
            detail::get_motion_object_nid(get_blackboard()->m_scene, OBJECT_KIND::AGENT, get_blackboard()->m_self_rid.get_node_id())
            );
    if (!get_blackboard()->m_scene->has_scene_node(nid))
        nid = detail::create_motion_scene_node(
                    get_blackboard()->m_scene,
                    nid,
                    agent_frame,
                    m_interpolator_matter.get_current_collider(),
                    *m_interpolator_matter.get_current_mass_distribution()
                    );

    m_motion_object_motion = detail::rigid_body_motion(get_blackboard()->m_scene, nid, get_blackboard()->m_motion_templates.directions());
    m_external_linear_acceleration =
        get_blackboard()->m_scene->get_external_linear_acceleration_of_rigid_body_of_scene_node(m_motion_object_motion.nid);
    m_external_angular_acceleration =
        get_blackboard()->m_scene->get_external_angular_acceleration_of_rigid_body_of_scene_node(m_motion_object_motion.nid);

    get_blackboard()->m_scene->register_to_collision_contacts_stream(m_motion_object_motion.nid, get_blackboard()->m_self_id);
}


action_controller::~action_controller()
{
    TMPROF_BLOCK();

    if (m_motion_object_motion.nid.valid())
    {
        get_blackboard()->m_scene->unregister_from_collision_contacts_stream(m_motion_object_motion.nid, get_blackboard()->m_self_id);
        detail::destroy_motion_scene_node(get_blackboard()->m_scene, m_motion_object_motion.nid);
    }
}


void  action_controller::initialise()
{
}


void  action_controller::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    m_interpolator_animation.next_round(time_step_in_seconds);
    m_interpolator_look_at.next_round(
            time_step_in_seconds,
            get_blackboard()->m_cortex->get_motion_desire_props().look_at_target_in_local_space,
            m_motion_object_motion.frame,
            m_interpolator_animation.get_current_frames_ref()
            );
    m_interpolator_matter.next_round(time_step_in_seconds);

    m_interpolator_animation.commit(m_interpolator_matter.get_interpolated_offset());
    m_interpolator_look_at.commit();
    m_interpolator_matter.commit(m_motion_object_motion);

    INVARIANT(
        m_interpolator_animation.done() == m_interpolator_look_at.done() &&
        m_interpolator_animation.done() == m_interpolator_matter.done()
        );

    if (m_interpolator_animation.done())
    {
        float_32_bit  consumed_time = 0.0f;
        detail::ideal_velocity_buider  ideal_velocity_buider(m_dst_cursor, get_blackboard()->m_motion_templates);
        detail::eval::context const  ctx{
                &get_blackboard()->m_cortex->get_motion_desire_props(),
                get_blackboard()->m_motion_templates.directions(),
                time_step_in_seconds
                };
        while (time_step_in_seconds >= consumed_time)
        {
            std::vector<skeletal_motion_templates::transition_info>  successors;
            get_blackboard()->m_motion_templates.get_successor_keyframes(m_dst_cursor, successors);
            skeletal_motion_templates::transition_info const&  best_transition = successors.at(
                    successors.size() == 1UL ? 0U : detail::choose_next_motion_action(successors, get_blackboard(), ctx)
                    );
            m_dst_cursor = best_transition.cursor;
            consumed_time += best_transition.time_in_seconds;
            ideal_velocity_buider.extend(best_transition.cursor, best_transition.time_in_seconds);
        }

        ideal_velocity_buider.close(
                m_motion_object_motion.frame,
                m_ideal_linear_velocity_in_world_space,
                m_ideal_angular_velocity_in_world_space
                );

        m_interpolator_animation.set_target(m_dst_cursor, consumed_time);
        m_interpolator_look_at.set_target(m_dst_cursor, consumed_time);
        m_interpolator_matter.set_target(m_dst_cursor, consumed_time);
    }

    // And finally, we update dynamics of agent's motion object (forces and torques).
    std::vector<skeletal_motion_templates::guarded_actions_ptr>  satisfied_guarded_actions;
    std::vector<scene::collicion_contant_info_ptr>  contacts_in_normal_cone;
    bool const  has_satisfied_guard = detail::get_satisfied_motion_guarded_actions(
            get_blackboard()->m_motion_templates.motions_map().at(m_dst_cursor.motion_name).actions.at(m_dst_cursor.keyframe_index),
            get_blackboard()->m_sensory_controller->get_collision_contacts()->get_collision_contacts_map(),
            m_motion_object_motion,
            get_blackboard()->m_cortex->get_motion_desire_props(),
            m_environment_linear_velocity,
            m_environment_angular_velocity,
            m_environment_acceleration_coef,
            m_external_linear_acceleration,
            m_external_angular_acceleration,
            &satisfied_guarded_actions,
            &contacts_in_normal_cone
            );

    compute_environment_motion(contacts_in_normal_cone);
    if (has_satisfied_guard)
        execute_satisfied_motion_guarded_actions(
                satisfied_guarded_actions,
                time_step_in_seconds,
                m_ideal_linear_velocity_in_world_space,
                m_ideal_angular_velocity_in_world_space,
                m_environment_linear_velocity,
                m_environment_angular_velocity,
                m_environment_acceleration_coef,
                m_external_linear_acceleration,
                m_external_angular_acceleration,
                get_blackboard()->m_cortex->get_motion_desire_props(),
                m_motion_action_data,
                m_motion_object_motion,
                m_motion_action_data
                );
    m_motion_object_motion.commit_velocities(get_blackboard()->m_scene, m_motion_object_motion.nid);
    m_motion_object_motion.commit_accelerations(get_blackboard()->m_scene, m_motion_object_motion.nid);
}


void  action_controller::compute_environment_motion(std::vector<scene::collicion_contant_info_ptr> const&  contacts_in_normal_cone)
{
    if (contacts_in_normal_cone.empty())
        return;
    clear_environment_motion();

    float_32_bit const  max_accel = std::max(1.0f, length(m_external_linear_acceleration));

    for (scene::collicion_contant_info_ptr  contact_ptr : contacts_in_normal_cone)
    {
        m_environment_linear_velocity += get_blackboard()->m_scene->get_linear_velocity_of_collider_at_point(
                contact_ptr->other_coid,
                contact_ptr->contact_point_in_world_space
                );
        m_environment_acceleration_coef +=
                angeo::get_static_friction_coefficient(
                        contact_ptr->self_material == angeo::COLLISION_MATERIAL_TYPE::NO_FRINCTION_NO_BOUNCING ?
                                    angeo::COLLISION_MATERIAL_TYPE::LEATHER :
                                    contact_ptr->self_material,
                        contact_ptr->other_material
                        )
                * (contact_ptr->normal_force_magnitude / max_accel);
    }
    m_environment_acceleration_coef /= (float_32_bit)contacts_in_normal_cone.size();
    m_environment_acceleration_coef = std::min(m_environment_acceleration_coef, 1.0f);
    m_environment_linear_velocity /= (float_32_bit)contacts_in_normal_cone.size();
}


void  action_controller::clear_environment_motion()
{
    m_environment_linear_velocity = vector3_zero();
    m_environment_angular_velocity = vector3_zero();
    m_environment_acceleration_coef = 0.0f;
}


void  action_controller::synchronise_motion_object_motion_with_scene()
{
    TMPROF_BLOCK();

    m_motion_object_motion.update_frame_with_forward_and_up_directions(get_blackboard()->m_scene, get_blackboard()->m_motion_templates.directions());
    m_motion_object_motion.update_linear_velocity(get_blackboard()->m_scene);
    m_motion_object_motion.update_angular_velocity(get_blackboard()->m_scene);

    m_external_linear_acceleration =
        get_blackboard()->m_scene->get_external_linear_acceleration_of_rigid_body_of_scene_node(m_motion_object_motion.nid);
    m_external_angular_acceleration =
        get_blackboard()->m_scene->get_external_angular_acceleration_of_rigid_body_of_scene_node(m_motion_object_motion.nid);

    // We clear all forces the agent introduced in the previous frame.
    m_motion_object_motion.set_linear_acceleration(m_external_linear_acceleration);
    m_motion_object_motion.set_angular_acceleration(m_external_angular_acceleration);

    // Synchronise agent's position in the world space according to its motion object in the previous time step
    m_motion_object_motion.commit_frame(get_blackboard()->m_scene, get_blackboard()->m_self_rid.get_node_id());
}


}
