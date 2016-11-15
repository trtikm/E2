#include <netexp/experiment_factory.hpp>
#include <netexp/algorithm.hpp>
#include <netlab/network.hpp>
#include <netlab/network_layer_props.hpp>
#include <netlab/network_props.hpp>
#include <netlab/network_objects.hpp>
#include <netlab/network_objects_factory.hpp>
#include <netlab/network_indices.hpp>
#include <netlab/ship_controller.hpp>
#include <netlab/initialiser_of_movement_area_centers.hpp>
#include <netlab/initialiser_of_ships_in_movement_areas.hpp>
#include <netlab/tracked_object_stats.hpp>
#include <utility/array_of_derived.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/random.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>
#include <vector>
#include <memory>

namespace netexp { namespace calibration { namespace {


struct  ship_controller : public netlab::ship_controller
{
    vector3  accelerate_into_dock(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  dock_position,              //!< Coordinates in meters.
            netlab::network_layer_props const&  layer_props,
            netlab::network_props const&  props
            ) const;

    vector3  accelerate_from_ship(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  other_ship_position,        //!< Coordinates in meters.
            vector3 const&  other_ship_velocity,        //!< In meters per second.
            vector3 const&  nearest_dock_position,      //!< Coordinates in meters. It is nearest to the ship, not to the other one.
            netlab::network_layer_props const&  layer_props,
            netlab::network_props const&  props
            ) const;
};


vector3  ship_controller::accelerate_into_dock(
        vector3 const&  ship_position,              //!< Coordinates in meters.
        vector3 const&  ship_velocity,              //!< In meters per second.
        vector3 const&  dock_position,              //!< Coordinates in meters.
        netlab::network_layer_props const&  layer_props,
        netlab::network_props const&  props
        ) const
{
    TMPROF_BLOCK();

    vector3 const  doc_ship_positions_delta = dock_position - ship_position;
    float_32_bit const  distance_to_dock = length(doc_ship_positions_delta);
    vector3 const  accel_dir = (distance_to_dock < 0.0001f) ? vector3_zero() :
                                                              (1.0f / distance_to_dock) * doc_ship_positions_delta;
    float_32_bit constexpr  accel_mag = 0.0f; // TODO!

    return accel_mag * accel_dir;

    //vector3 const  tangent_velocity = ship_velocity - dot_product(ship_velocity,accel_dir) * accel_dir;
    //float_32_bit const  tangent_speed = length(tangent_velocity);
    //vector3 const  decel_dir = (tangent_speed < 0.0001f) ? vector3_zero() :
    //                                                       (-1.0f / tangent_speed) * tangent_velocity;
    //float_32_bit constexpr  decel_mag = 0.0f; // TODO!

    //return accel_mag * accel_dir + decel_mag * decel_dir;
}

vector3  ship_controller::accelerate_from_ship(
        vector3 const&  ship_position,              //!< Coordinates in meters.
        vector3 const&  ship_velocity,              //!< In meters per second.
        vector3 const&  other_ship_position,        //!< Coordinates in meters.
        vector3 const&  other_ship_velocity,        //!< In meters per second.
        vector3 const&  nearest_dock_position,      //!< Coordinates in meters. It is nearest to the ship, not to the other one.
        netlab::network_layer_props const&  layer_props,
        netlab::network_props const&  props
        ) const
{
    TMPROF_BLOCK();

    vector3 const  ship_positions_delta = ship_position - other_ship_position;
    float_32_bit const  squared_distance_of_ships = length_squared(ship_positions_delta);

    float_32_bit const  max_repulsion_accel_mag = 0.0f; // TODO!
    if (squared_distance_of_ships < 1e-6f)
    {
        vector3  acceleration;
        compute_random_vector_of_magnitude(max_repulsion_accel_mag, default_random_generator(), acceleration);
        return acceleration;
    }

    float_32_bit const  squared_max_connection_distance = props.max_connection_distance_in_meters() *
                                                          props.max_connection_distance_in_meters() ;


    float_32_bit  accel_mag;
    if (length_squared(nearest_dock_position - other_ship_position) <= squared_max_connection_distance)
    {
        float_32_bit constexpr  base_accel_mag = 0.0f; // TODO!
        accel_mag = base_accel_mag;
    }
    else
    {
        if (length_squared(nearest_dock_position - ship_position) <= squared_max_connection_distance)
            return vector3_zero();

        accel_mag = 0.0f;
    }

    float_32_bit const repulsion_distance = (1.0f / 5.0f) * layer_props.distance_of_docks_in_meters();
    float_32_bit const squared_repulsion_distance = repulsion_distance * repulsion_distance;
    if (squared_distance_of_ships < squared_repulsion_distance)
    {
        float_32_bit const  distance_of_ships = std::sqrtf(squared_distance_of_ships);
        float_32_bit const  repulsion_scale =
                exponential_increase_from_zero_to_one(1.0f - distance_of_ships / repulsion_distance,1.0f,1.0f);
        accel_mag += repulsion_scale * max_repulsion_accel_mag;

        return (accel_mag / distance_of_ships) * ship_positions_delta;
    }

    if (accel_mag > 0.0f)
    {
        float_32_bit const  distance_of_ships = std::sqrtf(squared_distance_of_ships);
        return (accel_mag / distance_of_ships) * ship_positions_delta;
    }

    return vector3_zero();
}


std::shared_ptr<netlab::network_props>  get_network_props()
{
    static std::shared_ptr<netlab::network_props> const  props = std::make_shared<netlab::network_props>(
        std::vector<netlab::network_layer_props> {
            netlab::network_layer_props {
                3U,     //!< num_spikers_along_x_axis
                3U,     //!< num_spikers_along_y_axis
                2U,     //!< num_spikers_along_c_axis

                3U,     //!< num_docks_along_x_axis_per_spiker
                2U,     //!< num_docks_along_y_axis_per_spiker
                2U,     //!< num_docks_along_c_axis_per_spiker

                15U,    //!< num_ships_per_spiker

                10.0f,  //!< distance_of_docks_in_meters

                /// low_corner_of_docks
                vector3{-45.0f,-30.0f,5.0f},

                90.0f,  //!< size_of_ship_movement_area_along_x_axis_in_meters
                60.0f,  //!< size_of_ship_movement_area_along_y_axis_in_meters
                40.0f,  //!< size_of_ship_movement_area_along_c_axis_in_meters

                0.75f, //!< min_speed_of_ship_in_meters_per_second
                2.5f,  //!< max_speed_of_ship_in_meters_per_second

                true,   //!< are_spikers_excitatory

                std::make_shared<ship_controller const>()
            },
        },

        0.001f,         //!< update_time_step_in_seconds
        0.25f,          //!< max_connection_distance_in_meters
        1U              //!< num_threads_to_use

        );
    return props;
}


struct  spiker : public netlab::spiker
{
};


struct  dock : public netlab::dock
{
};


struct  ship : public netlab::ship
{
};


struct network_objects_factory : public netlab::network_objects_factory
{
    std::unique_ptr< array_of_derived<netlab::spiker> >  create_array_of_spikers(
            netlab::layer_index_type const  layer_index,
            natural_64_bit const  num_spikers
            ) const;

    std::unique_ptr< array_of_derived<netlab::dock> >  create_array_of_docks(
            netlab::layer_index_type const  layer_index,
            natural_64_bit const  num_docks
            ) const;

    std::unique_ptr< array_of_derived<netlab::ship> >  create_array_of_ships(
            netlab::layer_index_type const  layer_index,
            natural_64_bit const  num_ships
            ) const;
};

std::unique_ptr< array_of_derived<netlab::spiker> >  network_objects_factory::create_array_of_spikers(
        netlab::layer_index_type const  layer_index,
        natural_64_bit const  num_spikers
        ) const
{
    (void)layer_index;
    return make_array_of_derived<netlab::spiker,spiker>(num_spikers);
}

std::unique_ptr< array_of_derived<netlab::dock> >  network_objects_factory::create_array_of_docks(
        netlab::layer_index_type const  layer_index,
        natural_64_bit const  num_docks
        ) const
{
    (void)layer_index;
    return make_array_of_derived<netlab::dock,dock>(num_docks);
}

std::unique_ptr< array_of_derived<netlab::ship> >  network_objects_factory::create_array_of_ships(
        netlab::layer_index_type const  layer_index,
        natural_64_bit const  num_ships
        ) const
{
    (void)layer_index;
    return make_array_of_derived<netlab::ship,ship>(num_ships);
}


struct  initialiser_of_movement_area_centers : public netlab::initialiser_of_movement_area_centers
{
    initialiser_of_movement_area_centers();

    void  on_next_layer(netlab::layer_index_type const  layer_index, netlab::network_props const&  props);

    void  compute_movement_area_center_for_ships_of_spiker(
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
    std::vector<bar_random_distribution>  m_distribution_of_spiker_layer;
    random_generator_for_natural_32_bit  m_generator_of_spiker_layer;
    std::vector<float_32_bit>  m_max_distance_x;
    std::vector<float_32_bit>  m_max_distance_y;
    std::vector<float_32_bit>  m_max_distance_c;
    random_generator_for_natural_32_bit  m_position_generator;
};

initialiser_of_movement_area_centers::initialiser_of_movement_area_centers()
    : m_distribution_of_spiker_layer({
            make_bar_random_distribution_from_count_bars({
                get_network_props()->layer_props().at(0UL).num_spikers()
                })
            })
    , m_generator_of_spiker_layer()
    , m_max_distance_x({
            get_network_props()->layer_props().at(0UL).num_spikers_along_x_axis() *
                       get_network_props()->layer_props().at(0UL).distance_of_spikers_along_x_axis_in_meters()
            })
    , m_max_distance_y({
           get_network_props()->layer_props().at(0UL).num_spikers_along_y_axis() *
                      get_network_props()->layer_props().at(0UL).distance_of_spikers_along_y_axis_in_meters()
           })
    , m_max_distance_c({
           get_network_props()->layer_props().at(0UL).num_spikers_along_c_axis() *
                      get_network_props()->layer_props().at(0UL).distance_of_spikers_along_c_axis_in_meters()
           })
    , m_position_generator()
{
    ASSUMPTION(
        [](std::vector<bar_random_distribution> const&  distributions, natural_64_bit const  size) -> bool {
            if (distributions.size() != size)
                return false;
            for (auto const&  D : distributions)
                if (get_num_bars(D) != size)
                    return false;
            return true;
        }(m_distribution_of_spiker_layer,get_network_props()->layer_props().size())
        );
    ASSUMPTION(m_max_distance_x.size() == get_network_props()->layer_props().size());
    ASSUMPTION(m_max_distance_y.size() == get_network_props()->layer_props().size());
    ASSUMPTION(m_max_distance_c.size() == get_network_props()->layer_props().size());
}

void  initialiser_of_movement_area_centers::on_next_layer(
        netlab::layer_index_type const  layer_index,
        netlab::network_props const&  props
        )
{
    (void)layer_index;
    (void)props;
    reset(m_generator_of_spiker_layer);
    reset(m_position_generator);
}

void  initialiser_of_movement_area_centers::compute_movement_area_center_for_ships_of_spiker(
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
    (void)spiker_index_into_layer;
    compute_center_of_movement_area_for_ships_of_spiker(
                spiker_layer_index,
                props.layer_props().at(spiker_layer_index).spiker_sector_centre(
                    spiker_sector_coordinate_x,
                    spiker_sector_coordinate_y,
                    spiker_sector_coordinate_c
                    ),
                props,
                m_distribution_of_spiker_layer.at(spiker_layer_index),
                m_generator_of_spiker_layer,
                m_max_distance_x,
                m_max_distance_y,
                m_max_distance_c,
                m_position_generator,
                area_layer_index,
                area_center
                );
}


struct  initialiser_of_ships_in_movement_areas : public netlab::initialiser_of_ships_in_movement_areas
{
    initialiser_of_ships_in_movement_areas();

    void  on_next_layer(netlab::layer_index_type const  layer_index, netlab::network_props const&  props) { reset(random_generator()); }

    void  on_next_area(
            netlab::layer_index_type const  layer_index,
            netlab::object_index_type const  spiker_index,
            netlab::network_props const&  props
            )
    {}

    void  compute_ship_position_and_velocity_in_movement_area(
            vector3 const&  center,
            natural_32_bit const  ship_index_in_the_area,
            netlab::network_layer_props const&  layer_props,
            netlab::ship&  ship_reference
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
        netlab::network_layer_props const&  layer_props,
        netlab::ship&  ship_reference
        )
{
    (void)ship_index_in_the_area;

    compute_random_ship_position_in_movement_area(
            center,
            0.5f * layer_props.size_of_ship_movement_area_along_x_axis_in_meters(),
            0.5f * layer_props.size_of_ship_movement_area_along_y_axis_in_meters(),
            0.5f * layer_props.size_of_ship_movement_area_along_c_axis_in_meters(),
            random_generator(),
            ship_reference.get_position_nonconst_reference()
            );
    compute_random_ship_velocity_in_movement_area(
            layer_props.min_speed_of_ship_in_meters_per_second(),
            layer_props.max_speed_of_ship_in_meters_per_second(),
            random_generator(),
            ship_reference.get_velocity_nonconst_reference()
            );
}


struct  tracked_spiker_stats : public netlab::tracked_spiker_stats
{
};


struct  tracked_dock_stats : public netlab::tracked_dock_stats
{
};


struct  tracked_ship_stats : public netlab::tracked_ship_stats
{
};


std::shared_ptr<netlab::network>  create_network()
{
    return std::make_shared<netlab::network>(
        get_network_props(),
        network_objects_factory(),
        initialiser_of_movement_area_centers(),
        initialiser_of_ships_in_movement_areas()
        );
}


std::shared_ptr<netlab::tracked_spiker_stats>  create_tracked_spiker_stats()
{
    return std::make_shared<tracked_spiker_stats>();
}

std::shared_ptr<netlab::tracked_dock_stats>  create_tracked_dock_stats()
{
    return std::make_shared<tracked_dock_stats>();
}

std::shared_ptr<netlab::tracked_ship_stats>  create_tracked_ship_stats()
{
    return std::make_shared<tracked_ship_stats>();
}


}}}

namespace netexp { namespace calibration {


void __register__(netexp::experiment_factory&  factory)
{
    factory.register_experiment(
            "calibration",
            netexp::calibration::create_network,
            netexp::calibration::create_tracked_spiker_stats,
            netexp::calibration::create_tracked_dock_stats,
            netexp::calibration::create_tracked_ship_stats,
            "This is an artificial experiment. It purpose is to support the "
            "development and tunnig of the 'netlab'' library. Therefore, it is "
            "not an experiment in true sense. Do not include it into your research."
            );
}


}}
