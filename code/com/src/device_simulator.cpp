#include <com/device_simulator.hpp>
#include <com/simulation_context.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>

namespace com {


device_simulator::timer::timer(float_32_bit const  period_in_seconds_,
                               natural_8_bit const target_enable_level_, natural_8_bit const  current_enable_level_)
    : time_period(period_in_seconds_)
    , current_time(0.0f)
    , is_signalling(false)
    , target_enable_level(target_enable_level_)
    , current_enable_level(current_enable_level_)
    , request_infos()
{
    ASSUMPTION(time_period > 0.0001f && target_enable_level > 0U && current_enable_level <= target_enable_level);
}


device_simulator::sensor::sensor(object_guid const  collider_, std::unordered_set<object_guid> const&  triggers_,
                                 natural_8_bit const target_enable_level_, natural_8_bit const  current_enable_level_)
    : collider(collider_)
    , triggers(triggers_)
    , old_touching()
    , touching()
    , touch_begin()
    , touch_end()
    , target_enable_level(target_enable_level_)
    , current_enable_level(current_enable_level_)
    , request_infos()
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

    , m_request_infos_increment_enable_level_of_timer()
    , m_request_infos_decrement_enable_level_of_timer()
    , m_request_infos_reset_timer()

    , m_request_infos_increment_enable_level_of_sensor()
    , m_request_infos_decrement_enable_level_of_sensor()

    , m_timer_requests_increment_enable_level()
    , m_timer_requests_decrement_enable_level()
    , m_timer_requests_reset()

    , m_sensor_requests_increment_enable_level()
    , m_sensor_requests_decrement_enable_level()
{}


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
    
        s.touch_begin.clear();
        for (object_guid  other_collider : s.touching)
            if (s.old_touching.count(other_collider) == 0UL)
                s.touch_begin.insert(other_collider);

        s.touch_end.clear();
        for (object_guid  other_collider : s.old_touching)
            if (s.touching.count(other_collider) == 0UL)
                s.touch_end.insert(other_collider);
    }
}


void  device_simulator::next_round_of_timer_request_infos(simulation_context const&  ctx)
{
    for (index_type  idx : m_enabled_timers)
    {
        timer&  t = m_timers.at(idx);
        for (request_info_id  id : t.request_infos)
            next_round_of_request_info(invalid_object_guid(), invalid_object_guid(), id, ctx);
    }
}


void  device_simulator::next_round_of_sensor_request_infos(simulation_context const&  ctx)
{
    for (auto const&  guid_and_idx : m_enabled_sensors)
    {
        sensor&  s = m_sensors.at(guid_and_idx.second);
        for (auto const&  kind_and_id : s.request_infos)
        {
            sensor::collider_in_touch const*  other_colliders;
            switch (kind_and_id.first)
            {
            case sensor::TOUCHING: other_colliders = &s.touching; break;
            case sensor::BEGIN: other_colliders = &s.touch_begin; break;
            case sensor::END: other_colliders = &s.touch_end; break;
            default: UNREACHABLE(); break;
            }
            for (object_guid  other_collider : *other_colliders)
                next_round_of_request_info(s.collider, other_collider, kind_and_id.second, ctx);
        }
    }
}


void  device_simulator::next_round_of_request_info(object_guid const  self_collider, object_guid const  other_collider,
                                                   request_info_id const&  id, simulation_context const&  ctx)
{
    switch (id.kind)
    {
    case INCREMENT_ENABLE_LEVEL_OF_TIMER:
        m_timer_requests_increment_enable_level.push_back(m_request_infos_increment_enable_level_of_timer.at(id.index).index);
        break;
    case DECREMENT_ENABLE_LEVEL_OF_TIMER:
        m_timer_requests_decrement_enable_level.push_back(m_request_infos_decrement_enable_level_of_timer.at(id.index).index);
        break;
    case RESET_TIMER:
        m_timer_requests_reset.push_back(m_request_infos_reset_timer.at(id.index).index);
        break;

    case INCREMENT_ENABLE_LEVEL_OF_SEBSOR:
        m_sensor_requests_increment_enable_level.push_back(m_request_infos_increment_enable_level_of_sensor.at(id.index).index);
        break;
    case DECREMENT_ENABLE_LEVEL_OF_SENSOR:
        m_sensor_requests_decrement_enable_level.push_back(m_request_infos_decrement_enable_level_of_sensor.at(id.index).index);
        break;

    default: UNREACHABLE(); break;
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
