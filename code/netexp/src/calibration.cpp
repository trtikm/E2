#include <netexp/calibration.hpp>
#include <netlab/network_layer_props.hpp>
#include <netlab/network_props.hpp>
#include <netlab/network_objects.hpp>
#include <netlab/network_objects_factory.hpp>
#include <netlab/network_object_id.hpp>
#include <netlab/ship_controller.hpp>
#include <netlab/initialiser_of_movement_area_centers.hpp>
#include <netlab/initialiser_of_ships_in_movement_areas.hpp>
#include <utility/array_of_derived.hpp>
#include <utility/tensor_math.hpp>
#include <utility/random.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>
#include <vector>
#include <algorithm>

namespace netexp { namespace calibration { namespace detail { namespace {


struct  ship_controller : public netlab::ship_controller
{
    vector3  accelerate_into_space_box(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  space_box_center,           //!< Coordinates in meters.
            float_32_bit const  space_box_length_x,     //!< Length is in meters.
            float_32_bit const  space_box_length_y,     //!< Length is in meters.
            float_32_bit const  space_box_length_c      //!< Length is in meters.
            ) const;

    vector3  accelerate_into_dock(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  dock_position,              //!< Coordinates in meters.
            float_32_bit const  inter_docks_distance    //!< Distance between docks in meters.
            ) const;

    vector3  accelerate_from_ship(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  other_ship_position,        //!< Coordinates in meters.
            vector3 const&  other_ship_velocity,        //!< In meters per second.
            vector3 const&  nearest_dock_position,      //!< Coordinates in meters. It is nearest to the ship, not to the other one.
            float_32_bit const  inter_docks_distance    //!< Distance between docks in meters.
            ) const;
};


vector3  ship_controller::accelerate_into_space_box(
        vector3 const&  ship_position,              //!< Coordinates in meters.
        vector3 const&  ship_velocity,              //!< In meters per second.
        vector3 const&  space_box_center,           //!< Coordinates in meters.
        float_32_bit const  space_box_length_x,     //!< Length is in meters.
        float_32_bit const  space_box_length_y,     //!< Length is in meters.
        float_32_bit const  space_box_length_c      //!< Length is in meters.
        ) const
{
    NOT_IMPLEMENTED_YET();
}

vector3  ship_controller::accelerate_into_dock(
        vector3 const&  ship_position,              //!< Coordinates in meters.
        vector3 const&  ship_velocity,              //!< In meters per second.
        vector3 const&  dock_position,              //!< Coordinates in meters.
        float_32_bit const  inter_docks_distance    //!< Distance between docks in meters.
        ) const
{
    NOT_IMPLEMENTED_YET();
}

vector3  ship_controller::accelerate_from_ship(
        vector3 const&  ship_position,              //!< Coordinates in meters.
        vector3 const&  ship_velocity,              //!< In meters per second.
        vector3 const&  other_ship_position,        //!< Coordinates in meters.
        vector3 const&  other_ship_velocity,        //!< In meters per second.
        vector3 const&  nearest_dock_position,      //!< Coordinates in meters. It is nearest to the ship, not to the other one.
        float_32_bit const  inter_docks_distance    //!< Distance between docks in meters.
        ) const
{
    NOT_IMPLEMENTED_YET();
}


std::shared_ptr<netlab::network_props>  create_network_props()
{
    return std::make_shared<netlab::network_props>(
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
                vector3{-45.0f,-30.0f,-20.0f},

                90.0f,  //!< size_of_ship_movement_area_along_x_axis_in_meters
                60.0f,  //!< size_of_ship_movement_area_along_y_axis_in_meters
                40.0f,  //!< size_of_ship_movement_area_along_c_axis_in_meters

                0.003f, //!< min_speed_of_ship_in_meters_per_second
                0.01f,  //!< max_speed_of_ship_in_meters_per_second

                true,   //!< are_spikers_excitatory

                std::make_shared<ship_controller const>()
            },
        },

        0.001f,         //!< update_time_step_in_seconds
        0.25f,          //!< max_connection_distance_in_meters
        1U              //!< num_threads_to_use

        );
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
            natural_8_bit const  layer_index,
            natural_64_bit const  num_spikers
            ) const;

    std::unique_ptr< array_of_derived<netlab::dock> >  create_array_of_docks(
            natural_8_bit const  layer_index,
            natural_64_bit const  num_docks
            ) const;

    std::unique_ptr< array_of_derived<netlab::ship> >  create_array_of_ships(
            natural_8_bit const  layer_index,
            natural_64_bit const  num_ships
            ) const;
};

std::unique_ptr< array_of_derived<netlab::spiker> >  network_objects_factory::create_array_of_spikers(
        natural_8_bit const  layer_index,
        natural_64_bit const  num_spikers
        ) const
{
    (void)layer_index;
    return make_array_of_derived<netlab::spiker,spiker>(num_spikers);
}

std::unique_ptr< array_of_derived<netlab::dock> >  network_objects_factory::create_array_of_docks(
        natural_8_bit const  layer_index,
        natural_64_bit const  num_docks
        ) const
{
    (void)layer_index;
    return make_array_of_derived<netlab::dock,dock>(num_docks);
}

std::unique_ptr< array_of_derived<netlab::ship> >  network_objects_factory::create_array_of_ships(
        natural_8_bit const  layer_index,
        natural_64_bit const  num_ships
        ) const
{
    (void)layer_index;
    return make_array_of_derived<netlab::ship,ship>(num_ships);
}


struct  initialiser_of_movement_area_centers : public netlab::initialiser_of_movement_area_centers
{
    void  compute_movement_area_center_for_ships_of_spiker(
            natural_8_bit const  spiker_layer_index,
            natural_64_bit const  spiker_index_into_layer,
            natural_32_bit const  spiker_sector_coordinate_x,
            natural_32_bit const  spiker_sector_coordinate_y,
            natural_32_bit const  spiker_sector_coordinate_c,
            netlab::network_props const&  props,
            natural_8_bit&  area_layer_index,
            vector3&  area_center
            );
};

void  initialiser_of_movement_area_centers::compute_movement_area_center_for_ships_of_spiker(
        natural_8_bit const  spiker_layer_index,
        natural_64_bit const  spiker_index_into_layer,
        natural_32_bit const  spiker_sector_coordinate_x,
        natural_32_bit const  spiker_sector_coordinate_y,
        natural_32_bit const  spiker_sector_coordinate_c,
        netlab::network_props const&  props,
        natural_8_bit&  area_layer_index,
        vector3&  area_center
        )
{
    NOT_IMPLEMENTED_YET();
}


struct  initialiser_of_ships_in_movement_areas : public netlab::initialiser_of_ships_in_movement_areas
{
    void  compute_ship_position_and_velocity_in_movement_area(
            vector3 const&  center,
            natural_32_bit const  ship_index_in_the_area,
            netlab::network_layer_props const&  layer_props,
            netlab::ship&  ship_reference
            );
};

void  initialiser_of_ships_in_movement_areas::compute_ship_position_and_velocity_in_movement_area(
        vector3 const&  center,
        natural_32_bit const  ship_index_in_the_area,
        netlab::network_layer_props const&  layer_props,
        netlab::ship&  ship_reference
        )
{
    NOT_IMPLEMENTED_YET();
}


}}}}

namespace netexp { namespace calibration {


std::shared_ptr<netlab::network>  create_network()
{
    return std::make_shared<netlab::network>(
        detail::create_network_props(),
        detail::network_objects_factory(),
        detail::initialiser_of_movement_area_centers(),
        detail::initialiser_of_ships_in_movement_areas()
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


NETEXP_REGISTER_EXPERIMENT(
        "callibration",
        netexp::calibration::create_network,
        netexp::calibration::create_tracked_spiker_stats,
        netexp::calibration::create_tracked_dock_stats,
        netexp::calibration::create_tracked_ship_stats
        )


}}

namespace netexp {


experiment_factory&  experiment_factory::instance()
{
    static experiment_factory  ef;
    return ef;
}

bool  experiment_factory::register_experiment(
        std::string const&  experiment_unique_name,
        network_creator const&  network_creator_fn,
        tracked_spiker_stats_creator const&  spiker_stats_creator_fn,
        tracked_dock_stats_creator const&  dock_stats_creator_fn,
        tracked_ship_stats_creator const&  ship_stats_creator_fn
        )
{
    auto const  it = m_network_creators.find(experiment_unique_name);
    if (it != m_network_creators.cend())
        return false;
    m_network_creators.insert({experiment_unique_name,network_creator_fn});
    m_spiker_stats_creators.insert({experiment_unique_name,spiker_stats_creator_fn});
    m_dock_stats_creators.insert({experiment_unique_name,dock_stats_creator_fn});
    m_ship_stats_creators.insert({experiment_unique_name,ship_stats_creator_fn});
    return true;
}

void  experiment_factory::get_names_of_registered_experiments(std::vector<std::string>&  output)
{
    for (auto it = m_network_creators.cbegin(); it != m_network_creators.cend(); ++it)
        output.push_back(it->first);
}


std::shared_ptr<netlab::network>  experiment_factory::create_network(
        std::string const&  experiment_unique_name
        ) const
{
    auto const  it = m_network_creators.find(experiment_unique_name);
    ASSUMPTION(it != m_network_creators.cend());
    return it->second();
}

std::shared_ptr<netlab::tracked_spiker_stats>  experiment_factory::create_tracked_spiker_stats(
        std::string const&  experiment_unique_name
        ) const
{
    auto const  it = m_spiker_stats_creators.find(experiment_unique_name);
    ASSUMPTION(it != m_spiker_stats_creators.cend());
    return it->second();
}

std::shared_ptr<netlab::tracked_dock_stats>  experiment_factory::create_tracked_dock_stats(
        std::string const&  experiment_unique_name
        ) const
{
    auto const  it = m_dock_stats_creators.find(experiment_unique_name);
    ASSUMPTION(it != m_dock_stats_creators.cend());
    return it->second();
}

std::shared_ptr<netlab::tracked_ship_stats>  experiment_factory::create_tracked_ship_stats(
        std::string const&  experiment_unique_name
        ) const
{
    auto const  it = m_ship_stats_creators.find(experiment_unique_name);
    ASSUMPTION(it != m_ship_stats_creators.cend());
    return it->second();
}


}