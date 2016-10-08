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

    , m_max_coods_along_c_axis(layer_props.size())
{
    ASSUMPTION(!m_layer_props.empty());
    ASSUMPTION(m_update_time_step_in_seconds < 0.01f);
    ASSUMPTION(m_update_time_step_in_seconds > 0.0001f);
    ASSUMPTION(m_max_connection_distance_in_meters > 0.0f);
    ASSUMPTION(m_num_threads_to_use > 0U);

    for (natural_32_bit  i = 0UL; i < m_layer_props.size(); ++i)
    {
        ASSUMPTION(i == 0UL || m_max_coods_along_c_axis.at(i-1UL) <= m_layer_props.at(i).low_corner_of_docks()(2));
        m_max_coods_along_c_axis.at(i) = m_layer_props.at(i).high_corner_of_docks()(2);
    }
}


natural_8_bit  network_props::find_layer_index(float_32_bit const  coord_along_c_axis)
{
    auto const  it = std::lower_bound(m_max_coods_along_c_axis.cbegin(),m_max_coods_along_c_axis.cend(),coord_along_c_axis);
    return static_cast<natural_8_bit>(
                it == m_max_coods_along_c_axis.cend() ? m_max_coods_along_c_axis.size() - 1UL :
                                                        std::distance(m_max_coods_along_c_axis.cbegin(),it)
                );
}


}
