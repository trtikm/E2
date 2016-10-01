#include <netlab/network_props.hpp>
#include <utility/checked_number_operations.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace netlab {


network_props::network_props(
        std::vector<network_layer_props> const&  layer_props,

        float_32_bit const  update_time_step_in_seconds,

//        float_32_bit const  mini_spiking_potential_magnitude,
//        float_32_bit const  average_mini_spiking_period_in_seconds,

        float_32_bit const  max_connection_distance_in_meters,

        natural_32_bit const  num_threads_to_use
        )
    : m_layer_props(layer_props)

    , m_update_time_step_in_seconds(update_time_step_in_seconds)

//    , m_mini_spiking_potential_magnitude(mini_spiking_potential_magnitude)
//    , m_average_mini_spiking_period_in_seconds(average_mini_spiking_period_in_seconds)

    , m_max_connection_distance_in_meters(max_connection_distance_in_meters)

    , m_num_threads_to_use(num_threads_to_use)
{}


}
