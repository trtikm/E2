#ifndef COM_DEVICE_SIMULATOR_HPP_INCLUDED
#   define COM_DEVICE_SIMULATOR_HPP_INCLUDED

#   include <com/object_guid.hpp>
#   include <com/import_scene_props.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/dynamic_array.hpp>
#   include <vector>
#   include <unordered_map>
#   include <unordered_set>
#   include <utility>

namespace com { struct  simulation_context; }

namespace com {


struct  device_simulator
{
    using  index_type = natural_16_bit;

    using  timer_id = index_type;
    using  sensor_id = index_type;

    enum struct REQUEST_KIND : natural_8_bit
    {
        INCREMENT_ENABLE_LEVEL_OF_TIMER     = 0U,
        DECREMENT_ENABLE_LEVEL_OF_TIMER     = 1U,
        RESET_TIMER                         = 2U,

        INCREMENT_ENABLE_LEVEL_OF_SENSOR    = 3U,
        DECREMENT_ENABLE_LEVEL_OF_SENSOR    = 4U,

        IMPORT_SCENE                        = 5U,
        ERASE_FOLDER                        = 6U,

        RIGID_BODY_SET_LINEAR_VELOCITY      = 7U,
        RIGID_BODY_SET_ANGULAR_VELOCITY     = 8U,

        UPDATE_RADIAL_FORCE_FIELD           = 9U,
        UPDATE_LINEAR_FORCE_FIELD           = 10U,
        LEAVE_FORCE_FIELD                   = 11U,
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
    bool  is_valid_timer_id(timer_id const  tid) const;
    bool  is_timer_enabled(timer_id const  tid) const;
    void  register_request_info_to_timer(request_info_id const&  rid, timer_id const  tid);
    void  unregister_request_info_from_timer(request_info_id const&  rid, timer_id const  tid);
    std::vector<request_info_id> const&  request_infos_of_timer(timer_id const  tid) const;
    void  erase_timer(timer_id const  tid);

    sensor_id  insert_sensor(
            object_guid const  collider_,
            std::unordered_set<object_guid> const&  triggers_ = {},
            natural_8_bit const target_enable_level_ = 1,
            natural_8_bit const  current_enable_level_ = 0
            );
    void  insert_trigger_collider_to_sensor(sensor_id const  sid, object_guid const  collider_guid);
    void  erase_trigger_collider_from_sensor(sensor_id const  sid, object_guid const  collider_guid);
    bool  is_valid_sensor_id(sensor_id const  sid) const;
    bool  is_sensor_enabled(sensor_id const  sid) const;
    enum struct SENSOR_EVENT_TYPE : natural_8_bit
    {
        TOUCHING        = 0U,
        TOUCH_BEGIN     = 1U,
        TOUCH_END       = 2U,
    };
    void  register_request_info_to_sensor(request_info_id const&  rid, sensor_id const  sid, SENSOR_EVENT_TYPE const  type);
    void  unregister_request_info_from_sensor(request_info_id const&  rid, sensor_id const  sid, SENSOR_EVENT_TYPE const  type);
    std::vector<request_info_id> const&  request_infos_touching_of_sensor(sensor_id const  sid) const;
    std::vector<request_info_id> const&  request_infos_touch_begin_of_sensor(sensor_id const  sid) const;
    std::vector<request_info_id> const&  request_infos_touch_end_of_sensor(sensor_id const  sid) const;
    void  erase_sensor(sensor_id const  sid);

    request_info_id  insert_request_info_increment_enable_level_of_timer(timer_id const  tid);
    request_info_id  insert_request_info_decrement_enable_level_of_timer(timer_id const  tid);
    request_info_id  insert_request_info_reset_timer(timer_id const  tid);
    request_info_id  insert_request_info_increment_enable_level_of_sensor(sensor_id const  sid);
    request_info_id  insert_request_info_decrement_enable_level_of_sensor(sensor_id const  sid);
    request_info_id  insert_request_info_import_scene(import_scene_props const&  props);
    request_info_id  insert_request_info_erase_folder(object_guid const  folder_guid);
    request_info_id  insert_request_info_rigid_body_set_linear_velocity(object_guid const  rb_guid, vector3 const&  linear_velocity);
    request_info_id  insert_request_info_rigid_body_set_angular_velocity(object_guid const  rb_guid, vector3 const&  angular_velocity);
    request_info_id  insert_request_info_update_radial_force_field(
            float_32_bit const  multiplier = 1.0f,
            float_32_bit const  exponent = 1.0f,
            float_32_bit const  min_radius = 0.001f,
            bool const  use_mass = true
            );
    request_info_id  insert_request_info_update_linear_force_field(
            vector3 const&  acceleration = vector3(0.0f, 0.0f, -9.81f),
            bool const  use_mass = true
            );
    request_info_id  insert_request_info_leave_force_field();
    bool  is_valid_request_info_id(request_info_id const&  rid) const;
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

    std::vector<index_type>  m_timer_requests_increment_enable_level;
    std::vector<index_type>  m_timer_requests_decrement_enable_level;
    std::vector<index_type>  m_timer_requests_reset;

    std::vector<index_type>  m_sensor_requests_increment_enable_level;
    std::vector<index_type>  m_sensor_requests_decrement_enable_level;

    /////////////////////////////////////////////////////////////////////////////////////
    // REQUEST INFOS
    /////////////////////////////////////////////////////////////////////////////////////

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

    //
    // Locally handled request infos
    //

    dynamic_array<request_info<index_type>, index_type>  m_request_infos_increment_enable_level_of_timer;
    dynamic_array<request_info<index_type>, index_type>  m_request_infos_decrement_enable_level_of_timer;
    dynamic_array<request_info<index_type>, index_type>  m_request_infos_reset_timer;
    dynamic_array<request_info<index_type>, index_type>  m_request_infos_increment_enable_level_of_sensor;
    dynamic_array<request_info<index_type>, index_type>  m_request_infos_decrement_enable_level_of_sensor;

    //
    // All other request infos
    //

    struct  request_info_update_radial_force_field
    {
        float_32_bit  multiplier;
        float_32_bit  exponent;
        float_32_bit  min_radius;
        bool  use_mass;
    };

    dynamic_array<request_info<import_scene_props>, index_type>  m_request_infos_import_scene;
    dynamic_array<request_info<object_guid>, index_type>  m_request_infos_erase_folder;
    dynamic_array<request_info<std::pair<object_guid, vector3> >, index_type>  m_request_infos_rigid_body_set_linear_velocity;
    dynamic_array<request_info<std::pair<object_guid, vector3> >, index_type>  m_request_infos_rigid_body_set_angular_velocity;
    dynamic_array<request_info<request_info_update_radial_force_field>, index_type>  m_request_infos_update_radial_force_field;
    dynamic_array<request_info<std::pair<vector3, bool> >, index_type>  m_request_infos_update_linear_force_field;
    dynamic_array<request_info_base, index_type>  m_request_infos_leave_force_field;
};


}

#endif
