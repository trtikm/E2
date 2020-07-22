#ifndef COM_DEVICE_SIMULATOR_HPP_INCLUDED
#   define COM_DEVICE_SIMULATOR_HPP_INCLUDED

#   include <com/object_guid.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/dynamic_array.hpp>
#   include <vector>
#   include <unordered_map>
#   include <unordered_set>

namespace com { struct  simulation_context; }

namespace com {


struct  device_simulator
{
    using  device_id = natural_16_bit;

    using  index_type = natural_16_bit;

    device_simulator();
    void  next_round(simulation_context const&  ctx, float_32_bit const  time_step_in_seconds);

private:

    enum REQUEST_KIND
    {
        INCREMENT_ENABLE_LEVEL_OF_TIMER     = 0U,
        DECREMENT_ENABLE_LEVEL_OF_TIMER     = 1U,
        RESET_TIMER                         = 2U,

        INCREMENT_ENABLE_LEVEL_OF_SEBSOR    = 3U,
        DECREMENT_ENABLE_LEVEL_OF_SENSOR    = 4U,
    };

    struct request_info_id
    {
        REQUEST_KIND  kind;
        index_type  index;
    };

    struct  request_info_increment_enable_level_of_timer { index_type  index; };
    struct  request_info_decrement_enable_level_of_timer { index_type  index; };
    struct  request_info_reset_timer { index_type  index; };

    struct  request_info_increment_enable_level_of_sensor { index_type  index; };
    struct  request_info_decrement_enable_level_of_sensor { index_type  index; };


    struct  timer
    {
        timer(float_32_bit const  period_in_seconds_,
              natural_8_bit const target_enable_level_, natural_8_bit const  current_enable_level_);

        float_32_bit  time_period;
        float_32_bit  current_time;
        bool  is_signalling;
        natural_8_bit  target_enable_level;
        natural_8_bit  current_enable_level;

        std::vector<request_info_id>  request_infos;
    };

    struct  sensor
    {
        enum TOUCH_KIND
        {
            TOUCHING    = 0U,
            BEGIN       = 1U,
            END         = 2U,
        };

        using  collider_in_touch = std::unordered_set<object_guid>;

        sensor(object_guid const  collider_, std::unordered_set<object_guid> const&  triggers_,
               natural_8_bit const target_enable_level_, natural_8_bit const  current_enable_level_);

        object_guid  collider;
        std::unordered_set<object_guid>  triggers;
        collider_in_touch  old_touching;
        collider_in_touch  touching;
        collider_in_touch  touch_begin;
        collider_in_touch  touch_end;
        natural_8_bit  target_enable_level;
        natural_8_bit  current_enable_level;

        std::vector<std::pair<TOUCH_KIND, request_info_id> >  request_infos;
    };

    void  next_round_of_timers(float_32_bit const  time_step_in_seconds);
    void  next_round_of_sensors(simulation_context const&  ctx);

    void  next_round_of_timer_request_infos(simulation_context const&  ctx);
    void  next_round_of_sensor_request_infos(simulation_context const&  ctx);
    void  next_round_of_request_info(object_guid const  self_collider, object_guid const  other_collider,
                                     request_info_id const&  id, simulation_context const&  ctx);

    void  process_timer_requests_increment_enable_level();
    void  process_timer_requests_decrement_enable_level();
    void  process_timer_requests_reset();

    void  process_sensor_requests_increment_enable_level(simulation_context const&  ctx);
    void  process_sensor_requests_decrement_enable_level(simulation_context const&  ctx);

    dynamic_array<timer, index_type>  m_timers;
    dynamic_array<sensor, index_type>  m_sensors;

    std::unordered_set<index_type>  m_enabled_timers;
    std::unordered_map<object_guid, index_type>  m_enabled_sensors;

    dynamic_array<request_info_increment_enable_level_of_timer, index_type>  m_request_infos_increment_enable_level_of_timer;
    dynamic_array<request_info_increment_enable_level_of_timer, index_type>  m_request_infos_decrement_enable_level_of_timer;
    dynamic_array<request_info_reset_timer, index_type>  m_request_infos_reset_timer;

    dynamic_array<request_info_increment_enable_level_of_sensor, index_type>  m_request_infos_increment_enable_level_of_sensor;
    dynamic_array<request_info_increment_enable_level_of_sensor, index_type>  m_request_infos_decrement_enable_level_of_sensor;

    std::vector<index_type>  m_timer_requests_increment_enable_level;
    std::vector<index_type>  m_timer_requests_decrement_enable_level;
    std::vector<index_type>  m_timer_requests_reset;

    std::vector<index_type>  m_sensor_requests_increment_enable_level;
    std::vector<index_type>  m_sensor_requests_decrement_enable_level;
};


}

#endif
