#include <com/device_simulator.hpp>
#include <com/simulation_context.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <algorithm>

namespace com {


device_simulator::timer::timer(
        float_32_bit const  period_in_seconds_,
        natural_8_bit const target_enable_level_,
        natural_8_bit const  current_enable_level_
        )
    : time_period(period_in_seconds_)
    , current_time(0.0f)
    , is_signalling(false)
    , target_enable_level(target_enable_level_)
    , current_enable_level(current_enable_level_)
    , request_infos()
{
    ASSUMPTION(time_period > 0.0001f && target_enable_level > 0U && current_enable_level <= target_enable_level);
}


device_simulator::sensor::sensor(
        object_guid const  collider_,
        std::unordered_set<object_guid> const&  triggers_,
        natural_8_bit const target_enable_level_,
        natural_8_bit const  current_enable_level_
        )
    : collider(collider_)
    , triggers(triggers_)
    , old_touching()
    , touching()
    , touch_begin()
    , touch_end()
    , target_enable_level(target_enable_level_)
    , current_enable_level(current_enable_level_)
    , request_infos_on_touching()
    , request_infos_on_touch_begin()
    , request_infos_on_touch_end()
{
    ASSUMPTION(
        collider != invalid_object_guid() && collider.kind == OBJECT_KIND::COLLIDER &&
        target_enable_level > 0U && current_enable_level <= target_enable_level
        );
}


device_simulator::device_simulator()
    : m_timers()
    , m_sensors()

    , m_enabled_timers()
    , m_enabled_sensors()

    , m_timer_requests_increment_enable_level()
    , m_timer_requests_decrement_enable_level()
    , m_timer_requests_reset()
    , m_sensor_requests_increment_enable_level()
    , m_sensor_requests_decrement_enable_level()

    // Locally handled request infos

    , m_request_infos_increment_enable_level_of_timer()
    , m_request_infos_decrement_enable_level_of_timer()
    , m_request_infos_reset_timer()
    , m_request_infos_increment_enable_level_of_sensor()
    , m_request_infos_decrement_enable_level_of_sensor()

    // All other request infos

    , m_request_infos_import_scene()
    , m_request_infos_erase_folder()
    , m_request_infos_rigid_body_set_linear_velocity()
    , m_request_infos_rigid_body_set_angular_velocity()
    , m_request_infos_update_radial_force_field()
    , m_request_infos_update_linear_force_field()
    , m_request_infos_leave_force_field()
{}


device_simulator::timer_id  device_simulator::insert_timer(
        float_32_bit const  period_in_seconds_,
        natural_8_bit const target_enable_level_,
        natural_8_bit const  current_enable_level_
        )
{
    return m_timers.insert({period_in_seconds_, target_enable_level_, current_enable_level_});
}


bool  device_simulator::is_valid_timer_id(timer_id const  tid) const
{
    return m_timers.valid(tid);
}


void  device_simulator::register_request_info_to_timer(request_info_id const&  rid, timer_id const  tid)
{
    ASSUMPTION(
        is_valid_request_info_id(rid) &&
        is_valid_timer_id(tid) &&
        [](std::vector<request_info_id> const&  infos, request_info_id const&  rid) {
            return std::find(infos.begin(), infos.end(), rid) == infos.end();
        }(m_timers.at(tid).request_infos, rid)
        );
    m_timers.at(tid).request_infos.push_back(rid);
    request_info_base_of(rid).timers.push_back(tid);
}


void  device_simulator::unregister_request_info_from_timer(request_info_id const&  rid, timer_id const  tid)
{
    ASSUMPTION(is_valid_request_info_id(rid) && is_valid_timer_id(tid));
    auto&  infos = m_timers.at(tid).request_infos;
    infos.erase(std::find(infos.begin(), infos.end(), rid));

    request_info_base&  info_base = request_info_base_of(rid);
    info_base.timers.erase(std::find(info_base.timers.begin(), info_base.timers.end(), tid));
}


std::vector<device_simulator::request_info_id> const&  device_simulator::request_infos_of_timer(timer_id const  tid) const
{
    ASSUMPTION(is_valid_timer_id(tid));
    return m_timers.at(tid).request_infos;
}


void  device_simulator::erase_timer(timer_id const  tid)
{
    ASSUMPTION(is_valid_timer_id(tid));
    timer&  t = m_timers.at(tid);
    while (!t.request_infos.empty())
        unregister_request_info_from_timer(t.request_infos.back(), tid);
    m_timers.erase(tid);
}


device_simulator::sensor_id  device_simulator::insert_sensor(
        object_guid const  collider_,
        std::unordered_set<object_guid> const&  triggers_,
        natural_8_bit const target_enable_level_,
        natural_8_bit const  current_enable_level_
        )
{
    return m_sensors.insert({collider_, triggers_, target_enable_level_, current_enable_level_});
}


bool  device_simulator::is_valid_sensor_id(sensor_id const  sid) const
{
    return m_sensors.valid(sid);
}


void  device_simulator::register_request_info_to_sensor(
        request_info_id const&  rid,
        sensor_id const  sid,
        SENSOR_EVENT_TYPE const  type
        )
{
    ASSUMPTION(is_valid_request_info_id(rid) && is_valid_sensor_id(sid));
    std::vector<request_info_id>*  infos;
    switch (type)
    {
    case SENSOR_EVENT_TYPE::TOUCHING: infos = &m_sensors.at(sid).request_infos_on_touching; break;
    case SENSOR_EVENT_TYPE::TOUCH_BEGIN: infos = &m_sensors.at(sid).request_infos_on_touch_begin; break;
    case SENSOR_EVENT_TYPE::TOUCH_END: infos = &m_sensors.at(sid).request_infos_on_touch_end; break;
    default: UNREACHABLE(); break;
    }
    ASSUMPTION(std::find(infos->begin(), infos->end(), rid) == infos->end());
    infos->push_back(rid);
    request_info_base_of(rid).sensors.push_back({sid, type});
}


void  device_simulator::unregister_request_info_from_sensor(
        request_info_id const&  rid,
        sensor_id const  sid,
        SENSOR_EVENT_TYPE const  type
        )
{
    ASSUMPTION(is_valid_request_info_id(rid) && is_valid_sensor_id(sid));
    std::vector<request_info_id>*  infos;
    switch (type)
    {
    case SENSOR_EVENT_TYPE::TOUCHING: infos = &m_sensors.at(sid).request_infos_on_touching; break;
    case SENSOR_EVENT_TYPE::TOUCH_BEGIN: infos = &m_sensors.at(sid).request_infos_on_touch_begin; break;
    case SENSOR_EVENT_TYPE::TOUCH_END: infos = &m_sensors.at(sid).request_infos_on_touch_end; break;
    default: UNREACHABLE(); break;
    }
    infos->erase(std::find(infos->begin(), infos->end(), rid));

    request_info_base&  info_base = request_info_base_of(rid);
    info_base.sensors.erase(
            std::find(info_base.sensors.begin(), info_base.sensors.end(), std::pair<sensor_id, SENSOR_EVENT_TYPE>{ sid, type })
            );
}


std::vector<device_simulator::request_info_id> const&  device_simulator::request_infos_touching_of_sensor(sensor_id const  sid) const
{
    ASSUMPTION(is_valid_sensor_id(sid));
    return m_sensors.at(sid).request_infos_on_touching;
}


std::vector<device_simulator::request_info_id> const&  device_simulator::request_infos_touch_begin_of_sensor(sensor_id const  sid) const
{
    ASSUMPTION(is_valid_sensor_id(sid));
    return m_sensors.at(sid).request_infos_on_touch_begin;
}


std::vector<device_simulator::request_info_id> const&  device_simulator::request_infos_touch_end_of_sensor(sensor_id const  sid) const
{
    ASSUMPTION(is_valid_sensor_id(sid));
    return m_sensors.at(sid).request_infos_on_touch_end;
}


void  device_simulator::erase_sensor(sensor_id const  sid)
{
    ASSUMPTION(is_valid_sensor_id(sid));
    sensor&  s = m_sensors.at(sid);
    while (!s.request_infos_on_touching.empty())
        unregister_request_info_from_sensor(s.request_infos_on_touching.back(), sid, SENSOR_EVENT_TYPE::TOUCHING);
    while (!s.request_infos_on_touch_begin.empty())
        unregister_request_info_from_sensor(s.request_infos_on_touch_begin.back(), sid, SENSOR_EVENT_TYPE::TOUCH_BEGIN);
    while (!s.request_infos_on_touch_end.empty())
        unregister_request_info_from_sensor(s.request_infos_on_touch_end.back(), sid, SENSOR_EVENT_TYPE::TOUCH_END);
    m_sensors.erase(sid);
}


device_simulator::request_info_id  device_simulator::insert_request_info_increment_enable_level_of_timer(timer_id const  tid)
{
    ASSUMPTION(is_valid_timer_id(tid));
    return { REQUEST_KIND::INCREMENT_ENABLE_LEVEL_OF_TIMER, m_request_infos_increment_enable_level_of_timer.insert(tid) };
}


device_simulator::request_info_id  device_simulator::insert_request_info_decrement_enable_level_of_timer(timer_id const  tid)
{
    ASSUMPTION(is_valid_timer_id(tid));
    return { REQUEST_KIND::DECREMENT_ENABLE_LEVEL_OF_TIMER, m_request_infos_decrement_enable_level_of_timer.insert(tid) };
}


device_simulator::request_info_id  device_simulator::insert_request_info_reset_timer(timer_id const  tid)
{
    ASSUMPTION(is_valid_timer_id(tid));
    return { REQUEST_KIND::RESET_TIMER, m_request_infos_reset_timer.insert(tid) };
}


device_simulator::request_info_id  device_simulator::insert_request_info_increment_enable_level_of_sensor(sensor_id const  sid)
{
    ASSUMPTION(is_valid_sensor_id(sid));
    return { REQUEST_KIND::INCREMENT_ENABLE_LEVEL_OF_SEBSOR, m_request_infos_increment_enable_level_of_sensor.insert(sid) };
}


device_simulator::request_info_id  device_simulator::insert_request_info_decrement_enable_level_of_sensor(sensor_id const  sid)
{
    ASSUMPTION(is_valid_sensor_id(sid));
    return { REQUEST_KIND::DECREMENT_ENABLE_LEVEL_OF_SENSOR, m_request_infos_decrement_enable_level_of_sensor.insert(sid) };
}


device_simulator::request_info_id  device_simulator::insert_request_info_import_scene(
        std::string const&  import_dir,
        object_guid const  under_folder_guid,
        object_guid const  relocation_frame_guid,
        bool const  cache_imported_scene,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        object_guid const  motion_frame_guid
        )
{
    return {
        REQUEST_KIND::IMPORT_SCENE,
        m_request_infos_import_scene.insert(request_info_import_scene{
            import_dir, under_folder_guid, relocation_frame_guid, cache_imported_scene,
            linear_velocity, angular_velocity, motion_frame_guid
            })
    };
}


device_simulator::request_info_id  device_simulator::insert_request_info_erase_folder(object_guid const  folder_guid)
{
    return { REQUEST_KIND::ERASE_FOLDER, m_request_infos_erase_folder.insert(folder_guid) };
}


device_simulator::request_info_id  device_simulator::insert_request_info_rigid_body_set_linear_velocity(
        object_guid const  rb_guid,
        vector3 const&  linear_velocity
        )
{
    return {
        REQUEST_KIND::RIGID_BODY_SET_LINEAR_VELOCITY,
        m_request_infos_rigid_body_set_linear_velocity.insert(std::pair<object_guid, vector3>{rb_guid, linear_velocity })
    };
}


device_simulator::request_info_id  device_simulator::insert_request_info_rigid_body_set_angular_velocity(
        object_guid const  rb_guid,
        vector3 const&  angular_velocity
        )
{
    return {
        REQUEST_KIND::RIGID_BODY_SET_ANGULAR_VELOCITY,
        m_request_infos_rigid_body_set_angular_velocity.insert(std::pair<object_guid, vector3>{rb_guid, angular_velocity })
    };
}


device_simulator::request_info_id  device_simulator::insert_request_info_update_radial_force_field(
        float_32_bit const  multiplier,
        float_32_bit const  exponent,
        float_32_bit const  min_radius,
        bool const  use_mass
        )
{
    ASSUMPTION(multiplier > 0.0f && exponent >= 1.0f && min_radius >= 0.001f);
    return {
        REQUEST_KIND::UPDATE_RADIAL_FORCE_FIELD,
        m_request_infos_update_radial_force_field.insert(request_info_update_radial_force_field{
                multiplier, exponent, min_radius, use_mass
                })
    };
}


device_simulator::request_info_id  device_simulator::insert_request_info_update_linear_force_field(
        vector3 const&  acceleration,
        bool const  use_mass
        )
{
    return {
        REQUEST_KIND::UPDATE_LINEAR_FORCE_FIELD,
        m_request_infos_update_linear_force_field.insert(std::pair<vector3, bool>{acceleration, use_mass})
    };
}


device_simulator::request_info_id  device_simulator::insert_request_info_leave_force_field()
{
    return { REQUEST_KIND::LEAVE_FORCE_FIELD, m_request_infos_leave_force_field.insert({}) };
}


bool  device_simulator::is_valid_request_info_id(request_info_id const&  rid) const
{
    switch (rid.kind)
    {
    // Locally handled request infos

    case REQUEST_KIND::INCREMENT_ENABLE_LEVEL_OF_TIMER:
        return m_request_infos_increment_enable_level_of_timer.valid(rid.index);
    case REQUEST_KIND::DECREMENT_ENABLE_LEVEL_OF_TIMER:
        return m_request_infos_decrement_enable_level_of_timer.valid(rid.index);
    case REQUEST_KIND::RESET_TIMER:
        return m_request_infos_reset_timer.valid(rid.index);
    case REQUEST_KIND::INCREMENT_ENABLE_LEVEL_OF_SEBSOR:
        return m_request_infos_increment_enable_level_of_sensor.valid(rid.index);
    case REQUEST_KIND::DECREMENT_ENABLE_LEVEL_OF_SENSOR:
        return m_request_infos_decrement_enable_level_of_sensor.valid(rid.index);

    // All other request infos

    case REQUEST_KIND::IMPORT_SCENE:
        return m_request_infos_import_scene.valid(rid.index);
    case REQUEST_KIND::ERASE_FOLDER:
        return m_request_infos_erase_folder.valid(rid.index);
    case REQUEST_KIND::RIGID_BODY_SET_LINEAR_VELOCITY:
        return m_request_infos_rigid_body_set_linear_velocity.valid(rid.index);
    case REQUEST_KIND::RIGID_BODY_SET_ANGULAR_VELOCITY:
        return m_request_infos_rigid_body_set_angular_velocity.valid(rid.index);
    case REQUEST_KIND::UPDATE_RADIAL_FORCE_FIELD:
        return m_request_infos_update_radial_force_field.valid(rid.index);
    case REQUEST_KIND::UPDATE_LINEAR_FORCE_FIELD:
        return m_request_infos_update_linear_force_field.valid(rid.index);
    case REQUEST_KIND::LEAVE_FORCE_FIELD:
        return m_request_infos_leave_force_field.valid(rid.index);

    default: UNREACHABLE(); break;
    }
}


void  device_simulator::erase_request_info(request_info_id const&  rid)
{
    request_info_base&  info_base = request_info_base_of(rid);
    while (!info_base.timers.empty())
        unregister_request_info_from_timer(rid, info_base.timers.back());
    while (!info_base.sensors.empty())
        unregister_request_info_from_sensor(rid, info_base.sensors.back().first, info_base.sensors.back().second);

    switch (rid.kind)
    {
    // Locally handled request infos

    case REQUEST_KIND::INCREMENT_ENABLE_LEVEL_OF_TIMER:
        m_request_infos_increment_enable_level_of_timer.erase(rid.index);
        break;
    case REQUEST_KIND::DECREMENT_ENABLE_LEVEL_OF_TIMER:
        m_request_infos_decrement_enable_level_of_timer.erase(rid.index);
        break;
    case REQUEST_KIND::RESET_TIMER:
        m_request_infos_reset_timer.erase(rid.index);
        break;
    case REQUEST_KIND::INCREMENT_ENABLE_LEVEL_OF_SEBSOR:
        m_request_infos_increment_enable_level_of_sensor.erase(rid.index);
        break;
    case REQUEST_KIND::DECREMENT_ENABLE_LEVEL_OF_SENSOR:
        m_request_infos_decrement_enable_level_of_sensor.erase(rid.index);
        break;

    // All other request infos

    case REQUEST_KIND::IMPORT_SCENE:
        m_request_infos_import_scene.erase(rid.index);
        break;
    case REQUEST_KIND::ERASE_FOLDER:
        m_request_infos_erase_folder.erase(rid.index);
        break;
    case REQUEST_KIND::RIGID_BODY_SET_LINEAR_VELOCITY:
        m_request_infos_rigid_body_set_linear_velocity.erase(rid.index);
        break;
    case REQUEST_KIND::RIGID_BODY_SET_ANGULAR_VELOCITY:
        m_request_infos_rigid_body_set_angular_velocity.erase(rid.index);
        break;
    case REQUEST_KIND::UPDATE_RADIAL_FORCE_FIELD:
        m_request_infos_update_radial_force_field.erase(rid.index);
        break;
    case REQUEST_KIND::UPDATE_LINEAR_FORCE_FIELD:
        m_request_infos_update_linear_force_field.erase(rid.index);
        break;
    case REQUEST_KIND::LEAVE_FORCE_FIELD:
        m_request_infos_leave_force_field.erase(rid.index);
        break;

    default: UNREACHABLE(); break;
    }
}


device_simulator::request_info_base&  device_simulator::request_info_base_of(request_info_id const&  rid)
{
    ASSUMPTION(is_valid_request_info_id(rid));

    switch (rid.kind)
    {
    // Locally handled request infos

    case REQUEST_KIND::INCREMENT_ENABLE_LEVEL_OF_TIMER:
        return m_request_infos_increment_enable_level_of_timer.at(rid.index);
    case REQUEST_KIND::DECREMENT_ENABLE_LEVEL_OF_TIMER:
        return m_request_infos_decrement_enable_level_of_timer.at(rid.index);
    case REQUEST_KIND::RESET_TIMER:
        return m_request_infos_reset_timer.at(rid.index);
    case REQUEST_KIND::INCREMENT_ENABLE_LEVEL_OF_SEBSOR:
        return m_request_infos_increment_enable_level_of_sensor.at(rid.index);
    case REQUEST_KIND::DECREMENT_ENABLE_LEVEL_OF_SENSOR:
        return m_request_infos_decrement_enable_level_of_sensor.at(rid.index);

    // All other request infos

    case REQUEST_KIND::IMPORT_SCENE:
        return m_request_infos_import_scene.at(rid.index);
    case REQUEST_KIND::ERASE_FOLDER:
        return m_request_infos_erase_folder.at(rid.index);
    case REQUEST_KIND::RIGID_BODY_SET_LINEAR_VELOCITY:
        return m_request_infos_rigid_body_set_linear_velocity.at(rid.index);
    case REQUEST_KIND::RIGID_BODY_SET_ANGULAR_VELOCITY:
        return m_request_infos_rigid_body_set_angular_velocity.at(rid.index);
    case REQUEST_KIND::UPDATE_RADIAL_FORCE_FIELD:
        return m_request_infos_update_radial_force_field.at(rid.index);
    case REQUEST_KIND::UPDATE_LINEAR_FORCE_FIELD:
        return m_request_infos_update_linear_force_field.at(rid.index);
    case REQUEST_KIND::LEAVE_FORCE_FIELD:
        return m_request_infos_leave_force_field.at(rid.index);

    default: UNREACHABLE(); break;
    }
}


void  device_simulator::next_round_of_request_info(
        object_guid const  self_collider,
        object_guid const  other_collider,
        request_info_id const&  rid,
        simulation_context const&  ctx
        )
{
    switch (rid.kind)
    {
    // Locally handled request infos

    case REQUEST_KIND::INCREMENT_ENABLE_LEVEL_OF_TIMER:
        m_timer_requests_increment_enable_level.push_back(m_request_infos_increment_enable_level_of_timer.at(rid.index).data);
        break;
    case REQUEST_KIND::DECREMENT_ENABLE_LEVEL_OF_TIMER:
        m_timer_requests_decrement_enable_level.push_back(m_request_infos_decrement_enable_level_of_timer.at(rid.index).data);
        break;
    case REQUEST_KIND::RESET_TIMER:
        m_timer_requests_reset.push_back(m_request_infos_reset_timer.at(rid.index).data);
        break;
    case REQUEST_KIND::INCREMENT_ENABLE_LEVEL_OF_SEBSOR:
        m_sensor_requests_increment_enable_level.push_back(m_request_infos_increment_enable_level_of_sensor.at(rid.index).data);
        break;
    case REQUEST_KIND::DECREMENT_ENABLE_LEVEL_OF_SENSOR:
        m_sensor_requests_decrement_enable_level.push_back(m_request_infos_decrement_enable_level_of_sensor.at(rid.index).data);
        break;

    // All other request infos

    case REQUEST_KIND::IMPORT_SCENE: {
        request_info_import_scene const&  data = m_request_infos_import_scene.at(rid.index).data;
        ctx.request_import_scene_from_directory(
                data.import_dir, data.under_folder_guid, data.relocation_frame_guid, data.cache_imported_scene,
                data.linear_velocity, data.angular_velocity, data.motion_frame_guid
                );
        } break;
    case REQUEST_KIND::ERASE_FOLDER:
        ctx.request_erase_non_root_folder(m_request_infos_erase_folder.at(rid.index).data);
        break;
    case REQUEST_KIND::RIGID_BODY_SET_LINEAR_VELOCITY: {
        std::pair<object_guid, vector3> const&  data = m_request_infos_rigid_body_set_linear_velocity.at(rid.index).data;
        ctx.request_set_rigid_body_linear_velocity(data.first, data.second);
        } break;
    case REQUEST_KIND::RIGID_BODY_SET_ANGULAR_VELOCITY: {
        std::pair<object_guid, vector3> const&  data = m_request_infos_rigid_body_set_angular_velocity.at(rid.index).data;
        ctx.request_set_rigid_body_angular_velocity(data.first, data.second);
        } break;
    case REQUEST_KIND::UPDATE_RADIAL_FORCE_FIELD:
        {
            object_guid const  rb_guid = ctx.rigid_body_of_collider(other_collider);
            if (ctx.is_valid_rigid_body_guid(rb_guid))
            {
                vector3  acceleration;
                {
                    request_info_update_radial_force_field const&  data = m_request_infos_update_radial_force_field.at(rid.index).data;

                    vector3 const  rb_origin = ctx.frame_coord_system_in_world_space(ctx.frame_of_rigid_body(rb_guid)).origin();
                    vector3 const  field_origin = ctx.frame_coord_system_in_world_space(ctx.frame_of_collider(self_collider)).origin();

                    vector3  origin_delta = field_origin - rb_origin;
                    float_32_bit  distance = length(origin_delta);
                    if (distance < 1e-3f)
                    {
                        distance = 1e-3f;
                        origin_delta = distance * vector3_unit_z();
                    }
                    if (distance < data.min_radius)
                    {
                        origin_delta *= (data.min_radius / distance);
                        distance = data.min_radius;
                    }

                    float_32_bit const  inverted_mass = data.use_mass ? ctx.inverted_mass_of_rigid_body(rb_guid) : 1.0f;
                    float_32_bit const  magnitude = inverted_mass * data.multiplier * std::powf(distance, data.exponent);

                    acceleration = (magnitude / distance) * origin_delta;
                }
                ctx.request_set_rigid_body_linear_acceleration_from_source(rb_guid, self_collider, acceleration);
            }
        }
        break;
    case REQUEST_KIND::UPDATE_LINEAR_FORCE_FIELD:
        {
            object_guid const  rb_guid = ctx.rigid_body_of_collider(other_collider);
            if (ctx.is_valid_rigid_body_guid(rb_guid))
            {
                std::pair<vector3, bool> const&  data = m_request_infos_update_linear_force_field.at(rid.index).data;
                float_32_bit const  inverted_mass = data.second ? ctx.inverted_mass_of_rigid_body(rb_guid) : 1.0f;
                ctx.request_set_rigid_body_linear_acceleration_from_source(rb_guid, self_collider, inverted_mass * data.first);
            }
        }
        break;
    case REQUEST_KIND::LEAVE_FORCE_FIELD: {
        object_guid const  rb_guid = ctx.rigid_body_of_collider(other_collider);
        if (ctx.is_valid_rigid_body_guid(rb_guid))
            ctx.request_remove_rigid_body_linear_acceleration_from_source(rb_guid, self_collider);
        } break;

    default: UNREACHABLE(); break;
    }
}


void  device_simulator::clear()
{
    m_timers.clear();
    m_sensors.clear();

    m_enabled_timers.clear();
    m_enabled_sensors.clear();

    m_timer_requests_increment_enable_level.clear();
    m_timer_requests_decrement_enable_level.clear();
    m_timer_requests_reset.clear();
    m_sensor_requests_increment_enable_level.clear();
    m_sensor_requests_decrement_enable_level.clear();

    // Locally handled request infos

    m_request_infos_increment_enable_level_of_timer.clear();
    m_request_infos_decrement_enable_level_of_timer.clear();
    m_request_infos_reset_timer.clear();
    m_request_infos_increment_enable_level_of_sensor.clear();
    m_request_infos_decrement_enable_level_of_sensor.clear();

    // All other request infos

    m_request_infos_import_scene.clear();
    m_request_infos_erase_folder.clear();
    m_request_infos_rigid_body_set_linear_velocity.clear();
    m_request_infos_rigid_body_set_angular_velocity.clear();
    m_request_infos_update_radial_force_field.clear();
    m_request_infos_update_linear_force_field.clear();
    m_request_infos_leave_force_field.clear();
}


void  device_simulator::next_round(simulation_context const&  ctx, float_32_bit const  time_step_in_seconds)
{
    next_round_of_timers(time_step_in_seconds);
    next_round_of_sensors(ctx);

    next_round_of_timer_request_infos(ctx);
    next_round_of_sensor_request_infos(ctx);

    process_timer_requests_increment_enable_level();
    process_timer_requests_decrement_enable_level();
    process_timer_requests_reset();

    process_sensor_requests_increment_enable_level(ctx);
    process_sensor_requests_decrement_enable_level(ctx);
}



void  device_simulator::next_round_of_timers(float_32_bit const  time_step_in_seconds)
{
    for (index_type  idx : m_enabled_timers)
    {
        timer&  t = m_timers.at(idx);
        t.is_signalling = t.current_time < t.time_period && t.time_period <= t.current_time + time_step_in_seconds;
        if (t.current_time < t.time_period)
            t.current_time += time_step_in_seconds;
    }
}


void  device_simulator::next_round_of_sensors(simulation_context const&  ctx)
{
    for (auto const&  guid_and_idx : m_enabled_sensors)
    {
        sensor&  s = m_sensors.at(guid_and_idx.second);

        s.touching.swap(s.old_touching);

        s.touching.clear();
        for (natural_32_bit  contact_idx : ctx.collision_contacts_of_collider(s.collider))
        {
            object_guid const  touching_collider = ctx.get_collision_contact(contact_idx).other_collider(s.collider);
            if (s.triggers.empty() || s.triggers.count(touching_collider) != 0UL)
                s.touching.insert(touching_collider);
        }
    
        if (!s.request_infos_on_touch_begin.empty())
        {
            s.touch_begin.clear();
            for (object_guid  other_collider : s.touching)
                if (s.old_touching.count(other_collider) == 0UL)
                    s.touch_begin.insert(other_collider);
        }

        if (!s.request_infos_on_touch_end.empty())
        {
            s.touch_end.clear();
            for (object_guid  other_collider : s.old_touching)
                if (s.touching.count(other_collider) == 0UL)
                    s.touch_end.insert(other_collider);
        }
    }
}


void  device_simulator::next_round_of_timer_request_infos(simulation_context const&  ctx)
{
    for (index_type  idx : m_enabled_timers)
    {
        timer const&  t = m_timers.at(idx);
        for (request_info_id const&  id : t.request_infos)
            next_round_of_request_info(invalid_object_guid(), invalid_object_guid(), id, ctx);
    }
}


void  device_simulator::next_round_of_sensor_request_infos(simulation_context const&  ctx)
{
    for (auto const&  guid_and_idx : m_enabled_sensors)
    {
        sensor const&  s = m_sensors.at(guid_and_idx.second);

        for (request_info_id const&  id : s.request_infos_on_touching)
            for (object_guid  other_collider : s.touching)
                next_round_of_request_info(s.collider, other_collider, id, ctx);

        for (request_info_id const&  id : s.request_infos_on_touch_begin)
            for (object_guid  other_collider : s.touch_begin)
                next_round_of_request_info(s.collider, other_collider, id, ctx);

        for (request_info_id const&  id : s.request_infos_on_touch_end)
            for (object_guid  other_collider : s.touch_end)
                next_round_of_request_info(s.collider, other_collider, id, ctx);
    }
}


void  device_simulator::process_timer_requests_increment_enable_level()
{
    for (index_type  idx : m_timer_requests_increment_enable_level)
    {
        timer&  t = m_timers.at(idx);
        if (t.current_enable_level < t.target_enable_level)
        {
            ++t.current_enable_level;
            if (t.current_enable_level == t.target_enable_level)
                m_enabled_timers.insert(idx);
        }
    }
}


void  device_simulator::process_timer_requests_decrement_enable_level()
{
    for (index_type  idx : m_timer_requests_decrement_enable_level)
    {
        timer&  t = m_timers.at(idx);
        if (t.current_enable_level == t.target_enable_level)
            m_enabled_timers.erase(idx);
        if (t.current_enable_level > 0U)
            --t.current_enable_level;
    }
}


void  device_simulator::process_timer_requests_reset()
{
    for (index_type  idx : m_timer_requests_reset)
    {
        timer&  t = m_timers.at(idx);
        t.current_time = 0.0f;
    }
}


void  device_simulator::process_sensor_requests_increment_enable_level(simulation_context const&  ctx)
{
    for (index_type  idx : m_sensor_requests_increment_enable_level)
    {
        sensor&  s = m_sensors.at(idx);
        if (s.current_enable_level < s.target_enable_level)
        {
            ++s.current_enable_level;
            if (s.current_enable_level == s.target_enable_level)
            {
                m_enabled_sensors.insert({ s.collider, idx });
                ctx.request_enable_collider(s.collider, true);
            }
        }
    }
}


void  device_simulator::process_sensor_requests_decrement_enable_level(simulation_context const&  ctx)
{
    for (index_type  idx : m_sensor_requests_decrement_enable_level)
    {
        sensor&  s = m_sensors.at(idx);
        if (s.current_enable_level == s.target_enable_level)
        {
            m_enabled_sensors.erase(s.collider);
            ctx.request_enable_collider(s.collider, false);
        }
        if (s.current_enable_level > 0U)
            --s.current_enable_level;
    }
}


}
