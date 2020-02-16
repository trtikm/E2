#include <ai/env/sinet/snapshot_encoder.hpp>
#include <ai/sensory_controller.hpp>
#include <ai/sensory_controller_collision_contacts.hpp>
#include <ai/sensory_controller_ray_cast_sight.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace env { namespace sinet {


snapshot_encoder::snapshot_encoder(blackboard_agent_const_ptr const  blackboard_ptr, natural_8_bit const  start_layer_index)
    : START_LAYER_INDEX(start_layer_index)

    , desire_computed_by_cortex
        {
            { true, 0U, 100U },         // SIGN_AND_GEOMETRY_INFO ('num_units' variable is initialised later)
            START_LAYER_INDEX + 0U,     // INDEX
            0U, 0U,                     // i.e. encoders are NOT stored in a grid
            {}                          // encoders (they are all inserted later)
        }
    , regulated_desire
        {
            { true, 0U, 100U },         // SIGN_AND_GEOMETRY_INFO ('num_units' variable is initialised later)
            START_LAYER_INDEX + 1U,     // INDEX
            0U, 0U,                     // i.e. encoders are NOT stored in a grid
            {}                          // encoders (they are all inserted later)
        }
    , motion
        {
            { true, 0U, 100U },         // SIGN_AND_GEOMETRY_INFO ('num_units' variable is initialised later)
            START_LAYER_INDEX + 2U,     // INDEX
            0U, 0U,                     // i.e. encoders are NOT stored in a grid
            {}                          // encoders (they are all inserted later)
        }
    , camera
        {
            { true, 0U, 100U },         // SIGN_AND_GEOMETRY_INFO ('num_units' variable is initialised later)
            START_LAYER_INDEX + 3U,     // INDEX
            0U, 0U,                     // i.e. encoders are NOT stored in a grid
            {}                          // encoders (they are all inserted later)
        }
    , collision_contacts
        {
            { true, 0U, 100U },         // SIGN_AND_GEOMETRY_INFO ('num_units' variable is initialised later)
            START_LAYER_INDEX + 4U,     // INDEX
            1U,                         // NUM_ENCODERS_PER_GRID_CELL
            blackboard_ptr->m_sensory_controller->get_collision_contacts()->get_config().num_cells_along_any_axis,
                                        // NUM_GRID_CELLS_ALONG_ANY_AXIS
            {}                          // encoders (they are all inserted later)
        }
    , ray_casts
        {
            { true, 0U, 100U },         // SIGN_AND_GEOMETRY_INFO ('num_units' variable is initialised later)
            START_LAYER_INDEX + 5U,     // INDEX
            1U,                         // NUM_ENCODERS_PER_GRID_CELL
            std::dynamic_pointer_cast<sensory_controller_ray_cast_sight>(blackboard_ptr->m_sensory_controller->get_sight())
                    ->get_ray_cast_config().num_cells_along_any_axis,
                                        // NUM_GRID_CELLS_ALONG_ANY_AXIS
            {}                          // encoders (they are all inserted later)
        }
{
    // Next we insert encoders to each layer and we compute number of units, for each layer.

    auto const  compute_num_units = [](layer_props&  props) -> void {
        for (auto const&  encoder : props.encoders)
            props.SIGN_AND_GEOMETRY_INFO.num_units += encoder.num_units();
    };

    // --- desire_computed_by_cortex ---------------------------------------------------------------------

    // forward_unit_vector_in_local_space (3 floats)
    desire_computed_by_cortex.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // x
    desire_computed_by_cortex.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // y
    desire_computed_by_cortex.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // z

    // linear_velocity_unit_direction_in_local_space (3 floats)
    desire_computed_by_cortex.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // x
    desire_computed_by_cortex.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // y
    desire_computed_by_cortex.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // z

    // linear_speed  (1 float)
    desire_computed_by_cortex.encoders.push_back({ 8U, -10.0f, 10.0f, 0.001f, 60.0f });

    // angular_velocity_unit_axis_in_local_space (3 floats)
    desire_computed_by_cortex.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // x
    desire_computed_by_cortex.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // y
    desire_computed_by_cortex.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // z

    // angular_speed  (1 float)
    desire_computed_by_cortex.encoders.push_back({ 8U, -6.28f, 6.28f, 0.001f, 60.0f });

    // look_at_target_in_local_space (3 floats)
    desire_computed_by_cortex.encoders.push_back({ 8U, -50.0f, 50.0f, 0.001f, 60.0f });  // x
    desire_computed_by_cortex.encoders.push_back({ 8U, -50.0f, 50.0f, 0.001f, 60.0f });  // y
    desire_computed_by_cortex.encoders.push_back({ 8U, -50.0f, 50.0f, 0.001f, 60.0f });  // z

    compute_num_units(desire_computed_by_cortex);


    // --- regulated_desire ---------------------------------------------------------------------

    regulated_desire.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // x
    regulated_desire.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // y
    regulated_desire.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // z

    // linear_velocity_unit_direction_in_local_space (3 floats)
    regulated_desire.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // x
    regulated_desire.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // y
    regulated_desire.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // z

    // linear_speed  (1 float)
    regulated_desire.encoders.push_back({ 8U, -10.0f, 10.0f, 0.001f, 60.0f });

    // angular_velocity_unit_axis_in_local_space (3 floats)
    regulated_desire.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // x
    regulated_desire.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // y
    regulated_desire.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // z

    // angular_speed  (1 float)
    regulated_desire.encoders.push_back({ 8U, -6.28f, 6.28f, 0.001f, 60.0f });

    // look_at_target_in_local_space (3 floats)
    regulated_desire.encoders.push_back({ 8U, -50.0f, 50.0f, 0.001f, 60.0f });  // x
    regulated_desire.encoders.push_back({ 8U, -50.0f, 50.0f, 0.001f, 60.0f });  // y
    regulated_desire.encoders.push_back({ 8U, -50.0f, 50.0f, 0.001f, 60.0f });  // z

    compute_num_units(regulated_desire);


    // --- motion ---------------------------------------------------------------------

    // forward (3 floats)
    motion.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // x
    motion.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // y
    motion.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // z

    // up (3 floats)
    motion.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // x
    motion.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // y
    motion.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // z

    // linear_velocity (3 floats)
    motion.encoders.push_back({ 8U, -10.0f, 10.0f, 0.001f, 60.0f });  // x
    motion.encoders.push_back({ 8U, -10.0f, 10.0f, 0.001f, 60.0f });  // y
    motion.encoders.push_back({ 8U, -10.0f, 10.0f, 0.001f, 60.0f });  // z

    // angular_velocity_velocity (3 floats)
    motion.encoders.push_back({ 8U, -6.28f, 6.28f, 0.001f, 60.0f });  // x
    motion.encoders.push_back({ 8U, -6.28f, 6.28f, 0.001f, 60.0f });  // y
    motion.encoders.push_back({ 8U, -6.28f, 6.28f, 0.001f, 60.0f });  // z

    // linear_acceleration (3 floats)
    motion.encoders.push_back({ 8U, -500.0f, 500.0f, 0.001f, 60.0f });  // x
    motion.encoders.push_back({ 8U, -500.0f, 500.0f, 0.001f, 60.0f });  // y
    motion.encoders.push_back({ 8U, -500.0f, 500.0f, 0.001f, 60.0f });  // z

    // angular_acceleration (3 floats)
    motion.encoders.push_back({ 8U, -500.0f, 500.0f, 0.001f, 60.0f });  // x
    motion.encoders.push_back({ 8U, -500.0f, 500.0f, 0.001f, 60.0f });  // y
    motion.encoders.push_back({ 8U, -500.0f, 500.0f, 0.001f, 60.0f });  // z

    // gravity (3 floats)
    motion.encoders.push_back({ 8U, -10.0f, 10.0f, 0.001f, 60.0f });  // x
    motion.encoders.push_back({ 8U, -10.0f, 10.0f, 0.001f, 60.0f });  // y
    motion.encoders.push_back({ 8U, -10.0f, 10.0f, 0.001f, 60.0f });  // z

    // ideal_linear_velocity (3 floats)
    motion.encoders.push_back({ 8U, -10.0f, 10.0f, 0.001f, 60.0f });  // x
    motion.encoders.push_back({ 8U, -10.0f, 10.0f, 0.001f, 60.0f });  // y
    motion.encoders.push_back({ 8U, -10.0f, 10.0f, 0.001f, 60.0f });  // z

    // ideal_angular_velocity (3 floats)
    motion.encoders.push_back({ 8U, -6.28f, 6.28f, 0.001f, 60.0f });  // x
    motion.encoders.push_back({ 8U, -6.28f, 6.28f, 0.001f, 60.0f });  // y
    motion.encoders.push_back({ 8U, -6.28f, 6.28f, 0.001f, 60.0f });  // z

    // interpolation_param_till_destination_cursor (1 float)
    motion.encoders.push_back({ 8U, 0.0f, 1.0f, 0.001f, 60.0f });

    compute_num_units(motion);


    // --- camera ---------------------------------------------------------------------

    // camera_origin (3 floats)
    camera.encoders.push_back({ 8U, -2.0f, 2.0f, 0.001f, 60.0f });  // x
    camera.encoders.push_back({ 8U, -2.0f, 2.0f, 0.001f, 60.0f });  // y
    camera.encoders.push_back({ 8U, -2.0f, 2.0f, 0.001f, 60.0f });  // z

    // camera_x_axis (3 floats)
    camera.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // x
    camera.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // y
    camera.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // z

    // camera_y_axis (3 floats)
    camera.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // x
    camera.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // y
    camera.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // z

    // camera_z_axis (3 floats)
    camera.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // x
    camera.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // y
    camera.encoders.push_back({ 8U, -1.0f, 1.0f, 0.001f, 60.0f });  // z

    compute_num_units(camera);


    // --- collision_contacts ---------------------------------------------------------------------

    for (natural_16_bit  x = 0U; x < collision_contacts.NUM_GRID_CELLS_ALONG_ANY_AXIS; ++x)
        for (natural_16_bit  y = 0U; y < collision_contacts.NUM_GRID_CELLS_ALONG_ANY_AXIS; ++y)
            // target_in_local_space (3 floats) but the vector3 is ignored, only the event itself matters => bool value => 1 unit
            collision_contacts.encoders.push_back({ 1U, 0.0f, 0.0f, 0.1f, 60.0f });
    compute_num_units(collision_contacts);


    // --- ray_casts ---------------------------------------------------------------------

    for (natural_16_bit  x = 0U; x < ray_casts.NUM_GRID_CELLS_ALONG_ANY_AXIS; ++x)
        for (natural_16_bit  y = 0U; y < ray_casts.NUM_GRID_CELLS_ALONG_ANY_AXIS; ++y)
            // length(target_in_local_space - camera_origin) (1 float)
            ray_casts.encoders.push_back({ 8U, 0.0f, 50.0f, 0.1f, 60.0f });
    compute_num_units(ray_casts);
}


void  snapshot_encoder::next_round(
        float_32_bit const  time_step_in_seconds,
        snapshot const&  ss,
        std::unordered_set<netlab::simple::uid>&  spiking_units
        )
{
    TMPROF_BLOCK();

    struct  input_units_updater
    {
        float_32_bit  TIME_STEP_IN_SECONDS;
        natural_16_bit  encoder_index;
        natural_16_bit  unit_index;
        layer_props*  props;
        std::unordered_set<netlab::simple::uid>*  spiking_units;
        std::unordered_set<natural_16_bit>  visited_encoder_indices;

        void  begin(layer_props& p)
        {
            encoder_index = 0U;
            unit_index = 0U;
            props = &p;
            visited_encoder_indices.clear();
        }

        void  update_next(float_32_bit const* const  value_ptr)
        {
            std::unordered_set<natural_8_bit>  indices;
            number_encoder& encoder = props->encoders.at(encoder_index);
            encoder.next_round(TIME_STEP_IN_SECONDS, value_ptr, indices);
            for (natural_8_bit idx : indices)
                spiking_units->insert({ props->INDEX, (natural_16_bit)(unit_index + idx), 0U });
            visited_encoder_indices.insert(encoder_index);
            ++encoder_index;
            unit_index += encoder.num_units();
        };

        void  skip_next()
        {
            unit_index += props->encoders.at(encoder_index).num_units();
            ++encoder_index;
        }

        void  update_next(float_32_bit const  value)
        {
            update_next(&value);
        };

        void update_next(vector3 const&  value)
        {
            update_next(value(0));
            update_next(value(1));
            update_next(value(2));
        };

        void  begin_grid_cell(natural_16_bit const  x, natural_16_bit const  y)
        {
            ASSUMPTION(x < props->NUM_GRID_CELLS_ALONG_ANY_AXIS && y < props->NUM_GRID_CELLS_ALONG_ANY_AXIS);
            encoder_index = props->NUM_ENCODERS_PER_GRID_CELL * (y * props->NUM_GRID_CELLS_ALONG_ANY_AXIS + x);
            unit_index = encoder_index * props->encoders.front().num_units();
        }

        void  end_grid_cell()
        {
            INVARIANT(unit_index == encoder_index * props->encoders.front().num_units());
        }

        void  update_grid_cell(natural_16_bit const  x, natural_16_bit const  y, float_32_bit const  value)
        {
            begin_grid_cell(x, y);
            update_next(value);
            end_grid_cell();
        }

        void  end()
        {
            if (props->NUM_ENCODERS_PER_GRID_CELL > 0U)
            {
                encoder_index = 0U;
                unit_index = 0U;
                natural_16_bit  n = (natural_16_bit)props->encoders.size();
                while (encoder_index != n)
                    if (visited_encoder_indices.count(encoder_index) == 0UL)
                        update_next(nullptr);
                    else
                        skip_next();
            }
            INVARIANT(visited_encoder_indices.size() == props->encoders.size() &&
                      unit_index == props->SIGN_AND_GEOMETRY_INFO.num_units);
        }
    };
    input_units_updater  updater{ time_step_in_seconds, 0U, 0U, nullptr, &spiking_units };


    // --- desire_computed_by_cortex ---------------------------------------------------------------------

    updater.begin(desire_computed_by_cortex);
    updater.update_next(ss.desire_computed_by_cortex.forward_unit_vector_in_local_space);
    updater.update_next(ss.desire_computed_by_cortex.linear_velocity_unit_direction_in_local_space);
    updater.update_next(ss.desire_computed_by_cortex.linear_speed);
    updater.update_next(ss.desire_computed_by_cortex.angular_velocity_unit_axis_in_local_space);
    updater.update_next(ss.desire_computed_by_cortex.angular_speed);
    updater.update_next(ss.desire_computed_by_cortex.look_at_target_in_local_space);
    updater.end();


    // --- regulated_desire ---------------------------------------------------------------------

    updater.begin(regulated_desire);
    updater.update_next(ss.regulated_desire.forward_unit_vector_in_local_space);
    updater.update_next(ss.regulated_desire.linear_velocity_unit_direction_in_local_space);
    updater.update_next(ss.regulated_desire.linear_speed);
    updater.update_next(ss.regulated_desire.angular_velocity_unit_axis_in_local_space);
    updater.update_next(ss.regulated_desire.angular_speed);
    updater.update_next(ss.regulated_desire.look_at_target_in_local_space);
    updater.end();


    // --- motion ---------------------------------------------------------------------

    updater.begin(motion);
    updater.update_next(ss.forward);
    updater.update_next(ss.up);
    updater.update_next(ss.linear_velocity);
    updater.update_next(ss.angular_velocity);
    updater.update_next(ss.linear_acceleration);
    updater.update_next(ss.angular_acceleration);
    updater.update_next(ss.external_linear_acceleration);
    updater.update_next(ss.ideal_linear_velocity);
    updater.update_next(ss.ideal_angular_velocity);
    updater.update_next(ss.interpolation_param_till_destination_cursor);
    updater.end();


    // --- camera ---------------------------------------------------------------------

    updater.begin(camera);
    updater.update_next(ss.camera_origin);
    updater.update_next(ss.camera_x_axis);
    updater.update_next(ss.camera_y_axis);
    updater.update_next(ss.camera_z_axis);
    updater.end();


    // --- collision_contacts ---------------------------------------------------------------------

    updater.begin(collision_contacts);
    for (snapshot::collision_contact_event const& event : ss.collision_contact_events)
        updater.update_grid_cell(event.cell_x, event.cell_y, 0.0f);
    updater.end();


    // --- ray_casts ---------------------------------------------------------------------

    updater.begin(ray_casts);
    for (snapshot::collision_contact_event const& event : ss.collision_contact_events)
        updater.update_grid_cell(event.cell_x, event.cell_y, length(event.target_in_local_space - ss.camera_origin));
    updater.end();
}


void  snapshot_encoder::get_layers_configs(std::vector<netlab::simple::network_layer::config::sign_and_geometry>&  configs) const
{
    configs.push_back(desire_computed_by_cortex.SIGN_AND_GEOMETRY_INFO);
    configs.push_back(regulated_desire.SIGN_AND_GEOMETRY_INFO);
    configs.push_back(motion.SIGN_AND_GEOMETRY_INFO);
    configs.push_back(camera.SIGN_AND_GEOMETRY_INFO);
    configs.push_back(collision_contacts.SIGN_AND_GEOMETRY_INFO);
    configs.push_back(ray_casts.SIGN_AND_GEOMETRY_INFO);
}


}}}
