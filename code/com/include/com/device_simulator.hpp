#ifndef COM_DEVICE_SIMULATOR_HPP_INCLUDED
#   define COM_DEVICE_SIMULATOR_HPP_INCLUDED

#   include <com/object_guid.hpp>
#   include <angeo/tensor_math.hpp>
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

    using  timer_id = index_type;
    using  sensor_id = index_type;

    enum struct REQUEST_KIND : natural_8_bit
    {
        INCREMENT_ENABLE_LEVEL_OF_TIMER     = 0U,
        DECREMENT_ENABLE_LEVEL_OF_TIMER     = 1U,
        RESET_TIMER                         = 2U,

        INCREMENT_ENABLE_LEVEL_OF_SEBSOR    = 3U,
        DECREMENT_ENABLE_LEVEL_OF_SENSOR    = 4U,

        IMPORT_SCENE                        = 5U,
    };

    struct request_info_id
    {
        REQUEST_KIND  kind;
        index_type  index;

        bool operator==(request_info_id const&  other) const { return kind == other.kind && index == other.index; }
        bool operator!=(request_info_id const&  other) const { return !(*this == other); }
    };

    device_simulator();

    timer_id  insert_timer(
            float_32_bit const  period_in_seconds_,
            natural_8_bit const target_enable_level_ = 1,
            natural_8_bit const  current_enable_level_ = 0
            );
    void  register_request_info_to_timer(request_info_id const&  rid, timer_id const  tid);
    void  unregister_request_info_from_timer(request_info_id const&  rid, timer_id const  tid);
    void  erase_timer(timer_id const  tid);

    sensor_id  insert_sensor(
            object_guid const  collider_,
            std::unordered_set<object_guid> const&  triggers_ = {},
            natural_8_bit const target_enable_level_ = 1,
            natural_8_bit const  current_enable_level_ = 0
            );
    enum struct SENSOR_EVENT_TYPE : natural_8_bit
    {
        TOUCHING        = 0U,
        TOUCH_BEGIN     = 1U,
        TOUCH_END       = 2U,
    };
    void  register_request_info_to_sensor(request_info_id const&  rid, sensor_id const  sid, SENSOR_EVENT_TYPE const  type);
    void  unregister_request_info_from_sensor(request_info_id const&  rid, sensor_id const  sid, SENSOR_EVENT_TYPE const  type);
    void  erase_sensor(sensor_id const  sid);

    request_info_id  insert_request_info_increment_enable_level_of_timer(timer_id const  tid);
    request_info_id  insert_request_info_decrement_enable_level_of_timer(timer_id const  tid);
    request_info_id  insert_request_info_reset_timer(timer_id const  tid);
    request_info_id  insert_request_info_increment_enable_level_of_sensor(sensor_id const  sid);
    request_info_id  insert_request_info_decrement_enable_level_of_sensor(sensor_id const  sid);
    request_info_id  insert_request_info_import_scene(
            std::string const&  import_dir,
            object_guid const  under_folder_guid,
            object_guid const  relocation_frame_guid = invalid_object_guid(),
            bool const  cache_imported_scene = true,
            vector3 const&  linear_velocity = vector3_zero(),
            vector3 const&  angular_velocity = vector3_zero(),
            object_guid const  motion_frame_guid = invalid_object_guid()
            );
    void  erase_request_info(request_info_id const&  rid);

    void  clear();

    void  next_round(simulation_context const&  ctx, float_32_bit const  time_step_in_seconds);

private:

    struct  timer
    {
        timer() {}
        timer(float_32_bit const  period_in_seconds_,
              natural_8_bit const target_enable_level_,
              natural_8_bit const  current_enable_level_);

        float_32_bit  time_period;
        float_32_bit  current_time;
        bool  is_signalling;
        natural_8_bit  target_enable_level;
        natural_8_bit  current_enable_level;

        std::vector<request_info_id>  request_infos;
    };

    struct  sensor
    {
        using  collider_in_touch = std::unordered_set<object_guid>;

        sensor() {}
        sensor(object_guid const  collider_,
               std::unordered_set<object_guid> const&  triggers_,
               natural_8_bit const target_enable_level_,
               natural_8_bit const  current_enable_level_);

        object_guid  collider;
        std::unordered_set<object_guid>  triggers;
        collider_in_touch  old_touching;
        collider_in_touch  touching;
        collider_in_touch  touch_begin;
        collider_in_touch  touch_end;
        natural_8_bit  target_enable_level;
        natural_8_bit  current_enable_level;

        std::vector<request_info_id>  request_infos_on_touching;
        std::vector<request_info_id>  request_infos_on_touch_begin;
        std::vector<request_info_id>  request_infos_on_touch_end;
    };

    void  next_round_of_timers(float_32_bit const  time_step_in_seconds);
    void  next_round_of_sensors(simulation_context const&  ctx);

    void  next_round_of_timer_request_infos(simulation_context const&  ctx);
    void  next_round_of_sensor_request_infos(simulation_context const&  ctx);
    void  next_round_of_request_info(
            object_guid const  self_collider,
            object_guid const  other_collider,
            request_info_id const&  id,
            simulation_context const&  ctx
            );

    void  process_timer_requests_increment_enable_level();
    void  process_timer_requests_decrement_enable_level();
    void  process_timer_requests_reset();

    void  process_sensor_requests_increment_enable_level(simulation_context const&  ctx);
    void  process_sensor_requests_decrement_enable_level(simulation_context const&  ctx);

    dynamic_array<timer, index_type>  m_timers;
    dynamic_array<sensor, index_type>  m_sensors;

    std::unordered_set<index_type>  m_enabled_timers;
    std::unordered_map<object_guid, index_type>  m_enabled_sensors;

    struct  request_info_base
    {
        std::vector<timer_id>  timers;
        std::vector<std::pair<sensor_id, SENSOR_EVENT_TYPE> >  sensors;
    };

    request_info_base&  request_info_base_of(request_info_id const&  rid);

    template<typename T>
    struct  request_info : public request_info_base
    {
        using data_type = T;
        request_info() {}
        request_info(data_type const&  data_) : request_info_base(), data(data_) {}
        data_type  data;
    };

    struct  request_info_import_scene
    {
        std::string  import_dir;
        object_guid  under_folder_guid;
        object_guid  relocation_frame_guid;
        bool  cache_imported_scene;
        vector3  linear_velocity;
        vector3  angular_velocity;
        object_guid  motion_frame_guid;
    };

    dynamic_array<request_info<index_type>, index_type>  m_request_infos_increment_enable_level_of_timer;
    dynamic_array<request_info<index_type>, index_type>  m_request_infos_decrement_enable_level_of_timer;
    dynamic_array<request_info<index_type>, index_type>  m_request_infos_reset_timer;

    dynamic_array<request_info<index_type>, index_type>  m_request_infos_increment_enable_level_of_sensor;
    dynamic_array<request_info<index_type>, index_type>  m_request_infos_decrement_enable_level_of_sensor;

    dynamic_array<request_info<request_info_import_scene>, index_type>  m_request_infos_import_scene;

    std::vector<index_type>  m_timer_requests_increment_enable_level;
    std::vector<index_type>  m_timer_requests_decrement_enable_level;
    std::vector<index_type>  m_timer_requests_reset;

    std::vector<index_type>  m_sensor_requests_increment_enable_level;
    std::vector<index_type>  m_sensor_requests_decrement_enable_level;
};


}

#endif
