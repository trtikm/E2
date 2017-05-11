#include <netexp/experiment_factory.hpp>
#include <netexp/ship_controller_flat_space.hpp>
#include <netexp/algorithm.hpp>
#include <netlab/network.hpp>
#include <netlab/network_layer_props.hpp>
#include <netlab/network_props.hpp>
#include <netlab/network_layer_arrays_of_objects.hpp>
#include <netlab/network_layers_factory.hpp>
#include <netlab/network_indices.hpp>
#include <netlab/ship_controller.hpp>
#include <netexp/incremental_initialiser_of_movement_area_centers.hpp>
#include <netlab/initialiser_of_ships_in_movement_areas.hpp>
#include <netlab/tracked_object_stats.hpp>
#include <netlab/utility.hpp>
#include <angeo/tensor_math.hpp>
#include <angeo/utility.hpp>
#include <utility/array_of_derived.hpp>
#include <utility/random.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>
#include <vector>
#include <memory>
#include <array>
#include <iomanip>

namespace netexp { namespace dbg_spiking_develop { namespace {


inline netlab::layer_index_type constexpr  num_layers() { return 7U; }

std::vector< std::array<natural_32_bit, 3ULL> > const&  num_spikers()
{
    static std::vector< std::array<natural_32_bit, 3ULL> > const  data{
        { 3U, 2U, 1U }, // 0
        { 3U, 2U, 1U }, // 1
        { 4U, 3U, 1U }, // 2
        { 4U, 3U, 1U }, // 3
        { 7U, 6U, 1U }, // 4
        { 5U, 5U, 1U }, // 5
        { 9U, 8U, 1U }  // 6
    };
    return data;
}

std::vector< std::array<natural_32_bit, 3ULL> > const&  num_docks_per_spiker()
{
    static std::vector< std::array<natural_32_bit, 3ULL> > const  data{
        { 5U, 5U, 4U }, // 0
        { 4U, 4U, 4U }, // 1
        { 4U, 4U, 4U }, // 2
        { 4U, 3U, 3U }, // 3
        { 4U, 3U, 3U }, // 4
        { 3U, 3U, 3U }, // 5
        { 3U, 3U, 3U }  // 6
    };
    return data;
}

std::vector<natural_32_bit> const&  num_ships_per_spiker()
{
    static std::vector<natural_32_bit> const  data{
        110U, // 0
         69U, // 1
         69U, // 2
         40U, // 3
         40U, // 4
         30U, // 5
         30U, // 6
    };
    return data;
}

std::vector<float_32_bit> const&  distance_of_docks()
{
    static std::vector<float_32_bit> const  data{
        10.0f, // 0
        10.0f, // 1
        10.0f, // 2
        10.0f, // 3
        10.0f, // 4
        10.0f, // 5
        10.0f, // 6
    };
    return data;
}

std::vector<vector3> const&  low_corner_of_docks()
{
    static std::vector<vector3> const  data{
        {  -75.0f,  -50.0f,   5.0f },  // 0
        {  -60.0f,  -40.0f,  55.0f },  // 1
        {  -80.0f,  -60.0f, 105.0f },  // 2
        {  -80.0f,  -45.0f, 155.0f },  // 3
        { -140.0f,  -90.0f, 195.0f },  // 4
        {  -75.0f,  -75.0f, 235.0f },  // 5
        { -135.0f, -120.0f, 275.0f },  // 6
    };
    return data;
}

std::vector<bool> const&  are_spikers_excitatory()
{
    static std::vector<bool> const  data{
         true, // 0
        false, // 1
         true, // 2
        false, // 3
         true, // 4
        false, // 5
         true, // 6
    };
    return data;
}

std::vector< std::shared_ptr<netlab::ship_controller const> > const&  ship_controllers()
{
    static std::shared_ptr<netlab::ship_controller const> const  controller =
            std::make_shared<ship_controller_flat_space const>(
                    0.0f,           //!< Docks enumerations distance for 'accelerate_into_dock' method.
                    2.0f,           //!< Docks enumerations distance for 'accelerate_from_ship' method.
                    50.0f,          //!< Acceleration to dock (constant in whole sector of dock).
                    100U,           //!< Number of time steps to stop a ship in connection distance to a dock from max seed to zero.
                    100.0f,         //!< Maximal magnitude of an acceleration applied to a ship when avoiding another ship.
                    10.0f / 5.0f    //!< Maximal distance from a ship where to be concerned about avoiding another ships.
                    );
    static std::vector< std::shared_ptr<netlab::ship_controller const> > const  data{
        controller, // 0
        controller, // 1
        controller, // 2
        controller, // 3
        controller, // 4
        controller, // 5
        controller  // 6
    };
    return data;
}

std::vector< std::vector<vector2> > const&  speed_limits()
{
    static std::vector<vector2> const  speeds{
        { 0.5f, 5.0f }, // 0
        { 0.5f, 5.0f }, // 1
        { 0.5f, 5.0f }, // 2
        { 0.5f, 5.0f }, // 3
        { 0.5f, 5.0f }, // 4
        { 0.5f, 5.0f }, // 5
        { 0.5f, 5.0f }  // 6
    };
    static std::vector< std::vector<vector2> > const  data{
        speeds, // 0
        speeds, // 1
        speeds, // 2
        speeds, // 3
        speeds, // 4
        speeds, // 5
        speeds  // 6
    };
    return data;
}

std::vector< std::vector<vector3> > const&  size_of_movement_area()
{
    static auto const  movement_area_size =
        [](std::array<natural_32_bit, 3ULL> const&  num_spikers_to_span_over, netlab::layer_index_type const  layer_index) -> vector3 {
            return vector3((float_32_bit)num_spikers_to_span_over.at(0) *
                                    (float_32_bit)num_docks_per_spiker().at(layer_index).at(0) *
                                    distance_of_docks().at(layer_index),
                           (float_32_bit)num_spikers_to_span_over.at(1) *
                                    (float_32_bit)num_docks_per_spiker().at(layer_index).at(1) *
                                    distance_of_docks().at(layer_index),
                           (float_32_bit)num_spikers_to_span_over.at(2) *
                                    (float_32_bit)num_docks_per_spiker().at(layer_index).at(2) *
                                    distance_of_docks().at(layer_index)
                           );
        };
    static std::vector<vector3> const  areas{
        movement_area_size({2U,2U,1U},0U), // 0
        movement_area_size({2U,2U,1U},1U), // 1
        movement_area_size({2U,2U,1U},2U), // 2
        movement_area_size({2U,2U,1U},3U), // 3
        movement_area_size({3U,3U,1U},4U), // 4
        movement_area_size({3U,3U,1U},5U), // 5
        movement_area_size({4U,4U,1U},6U)  // 6
    };
    static std::vector< std::vector<vector3> > const  data{
        areas,  // 0
        areas,  // 1
        areas,  // 2
        areas,  // 3
        areas,  // 4
        areas,  // 5
        areas   // 6
    };
    return data;
}

std::shared_ptr<netlab::network_props>  get_network_props()
{
    static std::shared_ptr<netlab::network_props> const  props = std::make_shared<netlab::network_props>(
            netlab::make_layer_props(
                    num_spikers(),
                    num_docks_per_spiker(),
                    num_ships_per_spiker(),
                    distance_of_docks(),
                    low_corner_of_docks(),
                    are_spikers_excitatory(),
                    ship_controllers(),
                    speed_limits(),
                    size_of_movement_area()
                    ),
            0.005f,         //!< update_time_step_in_seconds
            1.0f,           //!< spiking_potential_magnitude
            0.25f,          //!< mini_spiking_potential_magnitude
            0.025f,         //!< average_mini_spiking_period_in_seconds
            0.25f,          //!< max_connection_distance_in_meters
            1U              //!< num_threads_to_use
            );
    return props;
}


std::vector< std::vector<natural_64_bit> > const&  layers_interconnection_matrix()
{
    static std::vector< std::vector<natural_64_bit> > const  data{
        //   0   *1    2   *3    4   *5    6
        {   0U,  0U,  1U,  0U,  1U,  1U,  3U }, // 0
        {   0U,  0U,  1U,  0U,  1U,  2U,  2U }, // 1*
        {   1U,  0U,  1U,  1U,  3U,  2U,  4U }, // 2
        {   2U,  1U,  1U,  1U,  3U,  1U,  3U }, // 3*
        {   5U,  4U,  4U,  4U, 11U,  3U, 11U }, // 4
        {   2U,  2U,  2U,  2U,  7U,  1U,  9U }, // 5*
        {   8U,  5U, 12U,  5U, 16U,  6U, 20U }, // 6
    };
    return data;
}

std::vector< std::vector< std::array<natural_32_bit, 3ULL> > > const&  max_distance()
{
    static std::vector< std::vector< std::array<natural_32_bit, 3ULL> > > const  data{
        {
            // layer 0
            { 3U, 2U, 1U }, // 0
            { 3U, 2U, 1U }, // 1
            { 4U, 3U, 1U }, // 2
            { 4U, 3U, 1U }, // 3
            { 7U, 6U, 1U }, // 4
            { 5U, 5U, 1U }, // 5
            { 9U, 8U, 1U }  // 6
        },
        {
            // layer 1
            { 3U, 2U, 1U }, // 0
            { 3U, 2U, 1U }, // 1
            { 4U, 3U, 1U }, // 2
            { 4U, 3U, 1U }, // 3
            { 7U, 6U, 1U }, // 4
            { 5U, 5U, 1U }, // 5
            { 9U, 8U, 1U }  // 6
        },
        {
            // layer 2
            { 3U, 2U, 1U }, // 0
            { 3U, 2U, 1U }, // 1
            { 4U, 3U, 1U }, // 2
            { 4U, 3U, 1U }, // 3
            { 4U, 4U, 1U }, // 4
            { 4U, 4U, 1U }, // 5
            { 4U, 4U, 1U }  // 6
        },
        {
            // layer 3
            { 2U, 2U, 1U }, // 0
            { 2U, 2U, 1U }, // 1
            { 2U, 2U, 1U }, // 2
            { 2U, 2U, 1U }, // 3
            { 3U, 3U, 1U }, // 4
            { 3U, 3U, 1U }, // 5
            { 3U, 3U, 1U }  // 6
        },
        {
            // layer 4
            { 2U, 2U, 1U }, // 0
            { 2U, 2U, 1U }, // 1
            { 2U, 2U, 1U }, // 2
            { 2U, 2U, 1U }, // 3
            { 3U, 3U, 1U }, // 4
            { 3U, 3U, 1U }, // 5
            { 3U, 3U, 1U }  // 6
        },
        {
            // layer 5
            { 2U, 2U, 1U }, // 0
            { 2U, 2U, 1U }, // 1
            { 2U, 2U, 1U }, // 2
            { 2U, 2U, 1U }, // 3
            { 3U, 3U, 1U }, // 4
            { 3U, 3U, 1U }, // 5
            { 3U, 3U, 1U }  // 6
        },
        {
            // layer 6
            { 2U, 2U, 1U }, // 0
            { 2U, 2U, 1U }, // 1
            { 2U, 2U, 1U }, // 2
            { 2U, 2U, 1U }, // 3
            { 3U, 3U, 1U }, // 4
            { 3U, 3U, 1U }, // 5
            { 3U, 3U, 1U }  // 6
        }
    };
    return data;
}


struct  initialiser_of_movement_area_centers : public netexp::incremental_initialiser_of_movement_area_centers
{
    initialiser_of_movement_area_centers();

    void  on_next_layer(netlab::layer_index_type const  layer_index, netlab::network_props const&  props);

    void  compute_initial_movement_area_center_for_ships_of_spiker(
            netlab::layer_index_type const  spiker_layer_index,
            netlab::object_index_type const  spiker_index_into_layer,
            netlab::sector_coordinate_type const  spiker_sector_coordinate_x,
            netlab::sector_coordinate_type const  spiker_sector_coordinate_y,
            netlab::sector_coordinate_type const  spiker_sector_coordinate_c,
            netlab::network_props const&  props,
            netlab::layer_index_type&  area_layer_index,
            vector3&  area_center
            );

private:
    std::vector<natural_64_bit>  m_counts_of_centers_into_layers;
    random_generator_for_natural_32_bit  m_generator_of_spiker_layer;
    random_generator_for_natural_32_bit  m_position_generator;
};

initialiser_of_movement_area_centers::initialiser_of_movement_area_centers()
    : netexp::incremental_initialiser_of_movement_area_centers(
            [](netlab::layer_index_type const  spiker_layer_index,
               netlab::layer_index_type const  area_layer_index) {
                return max_distance().at(spiker_layer_index).at(area_layer_index);
            })
    , m_counts_of_centers_into_layers()
    , m_generator_of_spiker_layer()
    , m_position_generator()
{
    m_counts_of_centers_into_layers.reserve(num_layers());
}

void  initialiser_of_movement_area_centers::on_next_layer(
        netlab::layer_index_type const  layer_index,
        netlab::network_props const&  props
        )
{
    (void)props;
    m_counts_of_centers_into_layers = layers_interconnection_matrix().at(layer_index);
    //reset(m_generator_of_spiker_layer);
    //reset(m_position_generator);
}

void  initialiser_of_movement_area_centers::compute_initial_movement_area_center_for_ships_of_spiker(
        netlab::layer_index_type const  spiker_layer_index,
        netlab::object_index_type const  spiker_index_into_layer,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_x,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_y,
        netlab::sector_coordinate_type const  spiker_sector_coordinate_c,
        netlab::network_props const&  props,
        netlab::layer_index_type&  area_layer_index,
        vector3&  area_center
        )
{
    area_layer_index = netexp::compute_layer_index_for_area_center(m_counts_of_centers_into_layers,m_generator_of_spiker_layer);
    compute_initial_movement_area_center_for_ships_of_spiker_XYC(
            spiker_layer_index,
            spiker_index_into_layer,
            spiker_sector_coordinate_x,
            spiker_sector_coordinate_y,
            spiker_sector_coordinate_c,
            props,
            area_layer_index,
            max_distance().at(spiker_layer_index).at(area_layer_index).at(0U),
            max_distance().at(spiker_layer_index).at(area_layer_index).at(1U),
            max_distance().at(spiker_layer_index).at(area_layer_index).at(2U),
            m_position_generator,
            area_center
            );
}


struct  initialiser_of_ships_in_movement_areas : public netlab::initialiser_of_ships_in_movement_areas
{
    initialiser_of_ships_in_movement_areas();

    void  on_next_layer(netlab::layer_index_type const  layer_index, netlab::network_props const&  props)
    { reset(random_generator()); }

    void  on_next_area(
            netlab::layer_index_type const  layer_index,
            netlab::object_index_type const  spiker_index,
            netlab::network_props const&  props
            )
    {}

    void  compute_ship_position_and_velocity_in_movement_area(
            vector3 const&  center,
            natural_32_bit const  ship_index_in_the_area,
            netlab::layer_index_type const  home_layer_index,
            netlab::layer_index_type const  area_layer_index,
            netlab::network_props const&  props,
            vector3&  ship_position,
            vector3&  ship_velocity 
            );

    random_generator_for_natural_32_bit&  random_generator() noexcept { return m_generator; }

private:
    random_generator_for_natural_32_bit  m_generator;
};

initialiser_of_ships_in_movement_areas::initialiser_of_ships_in_movement_areas()
    : netlab::initialiser_of_ships_in_movement_areas()
    , m_generator()
{}

void  initialiser_of_ships_in_movement_areas::compute_ship_position_and_velocity_in_movement_area(
        vector3 const&  center,
        natural_32_bit const  ship_index_in_the_area,
        netlab::layer_index_type const  home_layer_index,
        netlab::layer_index_type const  area_layer_index,
        netlab::network_props const&  props,
        vector3&  ship_position,
        vector3&  ship_velocity 
        )
{
    (void)ship_index_in_the_area;

    netlab::network_layer_props const&  layer_props = props.layer_props().at(home_layer_index);

    compute_random_ship_position_in_movement_area(
            center,
            0.5f * layer_props.size_of_ship_movement_area_along_x_axis_in_meters(area_layer_index),
            0.5f * layer_props.size_of_ship_movement_area_along_y_axis_in_meters(area_layer_index),
            0.5f * layer_props.size_of_ship_movement_area_along_c_axis_in_meters(area_layer_index),
            random_generator(),
            ship_position
            );
    compute_random_ship_velocity_in_movement_area(
            layer_props.min_speed_of_ship_in_meters_per_second(area_layer_index),
            layer_props.max_speed_of_ship_in_meters_per_second(area_layer_index),
            random_generator(),
            ship_velocity
            );
}


struct layer_of_spikers : public netlab::layer_of_spikers
{
    using  self = layer_of_spikers;
    using  super = netlab::layer_of_spikers;

    layer_of_spikers(netlab::layer_index_type const  layer_index, netlab::object_index_type const  num_spikers_in_the_layer)
        : super(layer_index, num_spikers_in_the_layer)
        , m_potential(num_spikers_in_the_layer, get_resting_potential())
    {}

    float_32_bit  get_potential(netlab::object_index_type const  spiker_index) const override { return m_potential.at(spiker_index); }

    float_32_bit  get_resting_potential() const override { return s_resting_potential.at(super::layer_index()); }
    float_32_bit  get_firing_potential() const override { return s_firing_potential.at(super::layer_index()); }
    float_32_bit  get_saturation_potential() const { return s_saturation_potential.at(super::layer_index()); }

    float_32_bit  get_potential_cooling_coef() const { return s_potential_cooling_coef.at(super::layer_index()); }

    void  integrate_spiking_potential(
            netlab::object_index_type const  spiker_index,
            float_32_bit const  time_delta_in_seconds,
            netlab::network_props const&  props
            ) override
    {
        auto &potential = get_potential_ref(spiker_index);
        float_32_bit const  potential_derivative = get_potential_cooling_coef() * (potential - self::get_resting_potential());
        potential = potential + time_delta_in_seconds * potential_derivative;
    }

    bool  on_arrival_of_postsynaptic_potential(
            netlab::object_index_type const  spiker_index,
            float_32_bit const  potential_delta,
            netlab::network_props const&  props
            ) override
    {
        auto &potential = get_potential_ref(spiker_index);
        potential += potential_delta;
        return potential >= self::get_firing_potential();
    }

    std::ostream&  get_info_text(
            netlab::object_index_type const  spiker_index,
            std::ostream&  ostr,
            std::string const&  shift = ""
            ) const override
    {
        super::get_info_text(spiker_index, ostr, shift);
        ostr << shift
             << std::fixed << std::setprecision(3) 
             << "Potential: " << self::get_potential(spiker_index) << "[~"
             << ( self::get_potential(spiker_index) >= self::get_resting_potential() ?
                    ((self::get_potential(spiker_index) - self::get_resting_potential()) /
                        (self::get_firing_potential() - self::get_resting_potential())) :
                    -((self::get_potential(spiker_index) - self::get_saturation_potential()) /
                        (self::get_resting_potential() - self::get_saturation_potential())) )
             << "%]\n"
             ;
        return ostr;
    }

private:
    float_32_bit&  get_potential_ref(netlab::object_index_type const  spiker_index) { return m_potential.at(spiker_index); }

    static std::vector<float_32_bit> const  s_resting_potential;
    static std::vector<float_32_bit> const  s_firing_potential;
    static std::vector<float_32_bit> const  s_saturation_potential;

    static std::vector<float_32_bit> const  s_potential_cooling_coef;

    std::vector<float_32_bit>  m_potential;
};

std::vector<float_32_bit> const  layer_of_spikers::s_resting_potential
{
    0.0f,       // 0
    0.0f,       // 1
    0.0f,       // 2
    0.0f,       // 3
    0.0f,       // 4
    0.0f,       // 5
    0.0f,       // 6
};

std::vector<float_32_bit> const  layer_of_spikers::s_firing_potential
{
    1.0f,       // 0
    1.0f,       // 1
    1.0f,       // 2
    1.0f,       // 3
    1.0f,       // 4
    1.0f,       // 5
    1.0f,       // 6
};

std::vector<float_32_bit> const  layer_of_spikers::s_saturation_potential
{
    -1.0f,      // 0
    -1.0f,      // 1
    -1.0f,      // 2
    -1.0f,      // 3
    -1.0f,      // 4
    -1.0f,      // 5
    -1.0f,      // 6
};

std::vector<float_32_bit> const  layer_of_spikers::s_potential_cooling_coef
{
    -0.02f,     // 0
    -0.02f,     // 1
    -0.02f,     // 2
    -0.02f,     // 3
    -0.02f,     // 4
    -0.02f,     // 5
    -0.02f,     // 6
};


struct layer_of_docks : public netlab::layer_of_docks
{
    using  self = layer_of_spikers;
    using  super = netlab::layer_of_docks;

    layer_of_docks(netlab::layer_index_type const  layer_index, netlab::object_index_type const  num_docks_in_the_layer)
        : super(layer_index, num_docks_in_the_layer)
    {}

    float_32_bit  on_arrival_of_postsynaptic_potential(
            netlab::object_index_type const  dock_index,
            float_32_bit const  potential_delta,
            vector3 const&  spiker_position,
            vector3 const&  dock_position,
            netlab::layer_index_type const  layer_index_of_spiker_owning_the_connected_ship,
            netlab::network_props const&  props
            ) override
    { return potential_delta; }

    float_32_bit  on_arrival_of_presynaptic_potential(
            netlab::object_index_type const  dock_index,
            float_32_bit const  potential_of_the_spiker_owning_the_connected_ship,
            netlab::layer_index_type const  layer_index_of_spiker_owning_the_connected_ship,
            netlab::network_props const&  props
            ) override
    { return props.spiking_potential_magnitude(); }

    float_32_bit  on_arrival_of_mini_spiking_potential(
            netlab::object_index_type const  dock_index,
            bool const  is_it_mini_spike_from_excitatory_spiker,
            vector3 const&  spiker_position,
            vector3 const&  dock_position,
            netlab::network_props const&  props
            ) const override
    { return props.mini_spiking_potential_magnitude() * (is_it_mini_spike_from_excitatory_spiker ? 1.0f : -1.0f); }

    float_32_bit  compute_potential_of_spiker_at_dock(
            netlab::object_index_type const  dock_index,
            float_32_bit const  potential_of_spiker,
            vector3 const&  spiker_position,
            vector3 const&  dock_position,
            netlab::network_props const&  props
            ) const
    { return potential_of_spiker; }

    std::ostream&  get_info_text(
            netlab::object_index_type const  dock_index,
            std::ostream&  ostr,
            std::string const&  shift = ""
            ) const override
    {
        super::get_info_text(dock_index, ostr, shift);
        return ostr;
    }

};


struct layer_of_ships : public netlab::layer_of_ships
{
    using  self = layer_of_spikers;
    using  super = netlab::layer_of_ships;

    layer_of_ships(netlab::layer_index_type const  layer_index, netlab::object_index_type const  num_ships_in_the_layer)
        : super(layer_index, num_ships_in_the_layer)
    {}

    float_32_bit  on_arrival_of_presynaptic_potential(
            netlab::object_index_type const  ship_index,
            float_32_bit const  potential_of_the_other_spiker_at_connected_dock,
            netlab::layer_index_type const  area_layer_index,
            netlab::network_props const&  props
            ) override
    { return props.spiking_potential_magnitude(); }

    void  on_arrival_of_postsynaptic_potential(
            netlab::object_index_type const  ship_index,
            float_32_bit const  potential_of_the_spiker_owning_the_connected_dock,
            netlab::layer_index_type const  area_layer_index,
            netlab::network_props const&  props
            ) override
    {}

    std::ostream&  get_info_text(
            netlab::object_index_type const  ship_index,
            std::ostream&  ostr,
            std::string const&  shift = ""
            ) const override
    {
        super::get_info_text(ship_index, ostr, shift);
        return ostr;
    }

};


struct network_layers_factory : public netlab::network_layers_factory
{
    std::unique_ptr<netlab::layer_of_spikers>  create_layer_of_spikers(
            netlab::layer_index_type const  layer_index,
            netlab::object_index_type const  num_spikers
            ) const override
    { return std::make_unique<layer_of_spikers>(layer_index,num_spikers); }

    std::unique_ptr<netlab::layer_of_docks>  create_layer_of_docks(
            netlab::layer_index_type const  layer_index,
            netlab::object_index_type const  num_docks
            ) const override
    { return std::make_unique<layer_of_docks>(layer_index,num_docks); }

    std::unique_ptr<netlab::layer_of_ships>  create_layer_of_ships(
            netlab::layer_index_type const  layer_index,
            netlab::object_index_type const  num_ships
            ) const override
    { return std::make_unique<layer_of_ships>(layer_index,num_ships); }

};


struct  tracked_spiker_stats : public netlab::tracked_spiker_stats
{
    tracked_spiker_stats(netlab::compressed_layer_and_object_indices const  indices)
        : netlab::tracked_spiker_stats(indices)
    {}
};


struct  tracked_dock_stats : public netlab::tracked_dock_stats
{
    tracked_dock_stats(netlab::compressed_layer_and_object_indices const  indices)
        : netlab::tracked_dock_stats(indices)
    {}
};


struct  tracked_ship_stats : public netlab::tracked_ship_stats
{
    tracked_ship_stats(netlab::compressed_layer_and_object_indices const  indices)
        : netlab::tracked_ship_stats(indices)
    {}
};


}}}


NETEXP_DEFINE_EXPERIMENT_REGISTERATION_FUNCTION(
        dbg_spiking_develop,
        "dbg_spiking_develop",
        []() { return get_network_props(); },
        []() { return std::make_shared<network_layers_factory>(); },
        []() { return std::make_shared<initialiser_of_movement_area_centers>(); },
        []() { return std::make_shared<initialiser_of_ships_in_movement_areas>(); },
        [](netlab::compressed_layer_and_object_indices const  indices){ return std::make_shared<tracked_spiker_stats>(indices); },
        [](netlab::compressed_layer_and_object_indices const  indices) { return std::make_shared<tracked_dock_stats>(indices); },
        [](netlab::compressed_layer_and_object_indices const  indices) { return std::make_shared<tracked_ship_stats>(indices); },
        "This is an artificial experiment. It purpose is to support the "
        "development and tunnig of spiking of spiker objects of 'netlab' library. "
        "Therefore, it is not an experiment in true sense. Do not include it into "
        "your research."
        );
