#ifndef CELLAB_UTILITIES_FOR_CONSTRUCTION_OF_NEURAL_TISSUE_HPP_INCLUDED
#   define CELLAB_UTILITIES_FOR_CONSTRUCTION_OF_NEURAL_TISSUE_HPP_INCLUDED

#   include <cellab/territorial_state_of_synapse.hpp>
#   include <cellab/shift_in_coordinates.hpp>
#   include <cellab/transition_algorithms.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/instance_wrapper.hpp>
#   include <utility/invariants.hpp>
#   include <functional>
#   include <type_traits>
#   include <tuple>
#   include <memory>

namespace cellab {

struct neural_tissue;

template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling>
struct automated_binding_of_transition_functions;

}


namespace cellab { namespace private_internal_implementation_details {


template<typename class_signalling>
kind_of_cell  get_signalling_callback_function(
        std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)> const&
            get_signalling,
        shift_in_coordinates const& shift,
        instance_wrapper<class_signalling const>& signalling_instance
        )
{
    std::pair<bits_const_reference,kind_of_cell> const bits_and_kind = get_signalling(shift);
    signalling_instance.construct_instance( bits_and_kind.first );
    return bits_and_kind.second;
}

template<typename class_cell>
kind_of_cell  get_cell_callback_function(
        std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)> const&
            get_cell,
        shift_in_coordinates const& shift,
        instance_wrapper<class_cell const>& cell_instance
        )
{
    std::pair<bits_const_reference,kind_of_cell> const bits_and_kind = get_cell(shift);
    cell_instance.construct_instance( bits_and_kind.first );
    return bits_and_kind.second;
}

template<typename class_synapse>
std::pair<kind_of_cell,kind_of_cell>  get_synapse_callback_function(
        std::function<std::tuple<bits_const_reference,kind_of_cell,kind_of_cell>(natural_32_bit)> const&
            get_connected_synapse_at_index,
        natural_32_bit const index_of_synapse,
        instance_wrapper<class_synapse const>& synapse_instance
        )
{
    std::tuple<bits_const_reference,kind_of_cell,kind_of_cell> const bits_and_kind_and_kind =
            get_connected_synapse_at_index(index_of_synapse);
    synapse_instance.construct_instance( std::get<0>(bits_and_kind_and_kind) );
    return std::make_pair( std::get<1>(bits_and_kind_and_kind), std::get<2>(bits_and_kind_and_kind) );
}

template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse>
void transition_function_of_synapse_to_muscle(
        neural_tissue* const tissue,
        bits_reference& bits_of_synapse_to_be_updated,
        kind_of_synapse_to_muscle const kind_of_synapse_to_muscle_to_be_updated,
        kind_of_cell const kind_of_source_cell,
        bits_const_reference const& bits_of_source_cell
        )
{
    class_synapse  synapse_to_be_updated = bits_const_reference(bits_of_synapse_to_be_updated);
    class_cell const  source_cell = bits_of_source_cell;
    INVARIANT(dynamic_cast<class_derived_from_neural_tissue*>(tissue) != nullptr);
    static_cast<class_derived_from_neural_tissue*>(tissue)->transition_function_of_synapse_to_muscle(
                synapse_to_be_updated,
                kind_of_synapse_to_muscle_to_be_updated,
                kind_of_source_cell,
                source_cell
                );
    synapse_to_be_updated >> bits_of_synapse_to_be_updated;
}

template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling>
territorial_state_of_synapse transition_function_of_synapse_inside_tissue(
        neural_tissue* const tissue,
        bits_reference& bits_of_synapse_to_be_updated,
        kind_of_cell kind_of_source_cell,
        bits_const_reference const& bits_of_source_cell,
        kind_of_cell kind_of_territory_cell,
        bits_const_reference const& bits_of_territory_cell,
        territorial_state_of_synapse current_territorial_state_of_synapse,
        shift_in_coordinates const& shift_to_low_corner,
        shift_in_coordinates const& shift_to_high_corner,
        std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)> const&
            get_signalling
        )
{
    class_synapse  synapse_to_be_updated = bits_const_reference(bits_of_synapse_to_be_updated);
    class_cell const  source_cell = bits_of_source_cell;
    class_cell const  territory_cell = bits_of_territory_cell;
    INVARIANT(dynamic_cast<class_derived_from_neural_tissue*>(tissue) != nullptr);
    territorial_state_of_synapse const result_territorial_state =
        static_cast<class_derived_from_neural_tissue*>(tissue)->transition_function_of_synapse_inside_tissue(
                synapse_to_be_updated,
                kind_of_source_cell,
                source_cell,
                kind_of_territory_cell,
                territory_cell,
                current_territorial_state_of_synapse,
                shift_to_low_corner,
                shift_to_high_corner,
                std::bind(
                    &get_signalling_callback_function<class_signalling>,
                    std::cref(get_signalling),
                    std::placeholders::_1,
                    std::placeholders::_2
                    )
                );
    synapse_to_be_updated >> bits_of_synapse_to_be_updated;
    return result_territorial_state;
}

template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_signalling>
void transition_function_of_signalling(
        neural_tissue* const tissue,
        bits_reference& bits_of_signalling_to_be_updated,
        kind_of_cell kind_of_territory_cell,
        shift_in_coordinates const& shift_to_low_corner,
        shift_in_coordinates const& shift_to_high_corner,
        std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)> const&
            get_cell
        )
{
    class_signalling  signalling_to_be_updated = bits_const_reference(bits_of_signalling_to_be_updated);
    INVARIANT(dynamic_cast<class_derived_from_neural_tissue*>(tissue) != nullptr);
    static_cast<class_derived_from_neural_tissue*>(tissue)->transition_function_of_signalling(
                signalling_to_be_updated,
                kind_of_territory_cell,
                shift_to_low_corner,
                shift_to_high_corner,
                std::bind(
                    &get_cell_callback_function<class_cell>,
                    std::cref(get_cell),
                    std::placeholders::_1,
                    std::placeholders::_2
                    )
                );
    signalling_to_be_updated >> bits_of_signalling_to_be_updated;
}

template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling>
void transition_function_of_cell(
        neural_tissue* const tissue,
        bits_reference& bits_of_cell_to_be_updated,
        kind_of_cell kind_of_cell_to_be_updated,
        natural_32_bit num_of_synapses_connected_to_the_cell,
        std::function<std::tuple<bits_const_reference,kind_of_cell,kind_of_cell>(natural_32_bit)> const&
            get_connected_synapse_at_index,
        shift_in_coordinates const& shift_to_low_corner,
        shift_in_coordinates const& shift_to_high_corner,
        std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)> const&
            get_signalling
        )
{
    class_cell  cell_to_be_updated = bits_const_reference(bits_of_cell_to_be_updated);
    INVARIANT(dynamic_cast<class_derived_from_neural_tissue*>(tissue) != nullptr);
    static_cast<class_derived_from_neural_tissue*>(tissue)->transition_function_of_cell(
                cell_to_be_updated,
                kind_of_cell_to_be_updated,
                num_of_synapses_connected_to_the_cell,
                std::bind(
                    &get_synapse_callback_function<class_synapse>,
                    std::cref(get_connected_synapse_at_index),
                    std::placeholders::_1,
                    std::placeholders::_2
                    ),
                shift_to_low_corner,
                shift_to_high_corner,
                std::bind(
                    &get_signalling_callback_function<class_signalling>,
                    std::cref(get_signalling),
                    std::placeholders::_1,
                    std::placeholders::_2
                    )
                );
    cell_to_be_updated >> bits_of_cell_to_be_updated;
}

template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling,
         typename use_bits_references = std::true_type>
struct bind_transition_function_of_synapse_to_muscle
{
    static single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_to_muscle
    get_function( neural_tissue* const tissue)
    {
        return std::bind(
                    &class_derived_from_neural_tissue::transition_function_of_packed_synapse_to_muscle,
                    static_cast<class_derived_from_neural_tissue*>(tissue),
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4
                    );
    }
};

template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling>
struct bind_transition_function_of_synapse_to_muscle<
            class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,std::false_type
            >
{
    static single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_to_muscle
    get_function( neural_tissue* const tissue )
    {
        return std::bind(
                    &transition_function_of_synapse_to_muscle<
                        class_derived_from_neural_tissue,class_cell,class_synapse
                        >,
                    tissue,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4
                    );
    }
};

template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling,
         typename use_bits_references = std::true_type>
struct bind_transition_function_of_synapse_inside_tissue
{
    static single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue
    get_function( neural_tissue* const tissue)
    {
        return std::bind(
                    &class_derived_from_neural_tissue::transition_function_of_synapse_inside_tissue,
                    static_cast<class_derived_from_neural_tissue*>(tissue),
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4,
                    std::placeholders::_5,
                    std::placeholders::_6,
                    std::placeholders::_7,
                    std::placeholders::_8,
                    std::placeholders::_9
                    );
    }
};

template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling>
struct bind_transition_function_of_synapse_inside_tissue<
            class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,std::false_type
            >
{
    static single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue
    get_function( neural_tissue* const tissue )
    {
        return std::bind(
                    &private_internal_implementation_details::transition_function_of_synapse_inside_tissue<
                          class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling
                          >,
                    tissue,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4,
                    std::placeholders::_5,
                    std::placeholders::_6,
                    std::placeholders::_7,
                    std::placeholders::_8,
                    std::placeholders::_9
                    );
    }
};

template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling,
         typename use_bits_references = std::true_type>
struct bind_transition_function_of_signalling
{
    static single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_signalling
    get_function( neural_tissue* const tissue)
    {
        return std::bind(
                    &class_derived_from_neural_tissue::transition_function_of_signalling,
                    static_cast<class_derived_from_neural_tissue*>(tissue),
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4,
                    std::placeholders::_5
                    );
    }
};

template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling>
struct bind_transition_function_of_signalling<
            class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,std::false_type
            >
{
    static single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_signalling
    get_function( neural_tissue* const tissue )
    {
        return std::bind(
                    &private_internal_implementation_details::transition_function_of_signalling<
                        class_derived_from_neural_tissue,class_cell,class_signalling
                        >,
                    tissue,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4,
                    std::placeholders::_5
                    );
    }
};

template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling,
         typename use_bits_references = std::true_type>
struct bind_transition_function_of_cell
{
    static single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_cell
    get_function( neural_tissue* const tissue)
    {
        return std::bind(
                    &class_derived_from_neural_tissue::transition_function_of_cell,
                    static_cast<class_derived_from_neural_tissue*>(tissue),
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4,
                    std::placeholders::_5,
                    std::placeholders::_6,
                    std::placeholders::_7
                    );
    }
};

template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling>
struct bind_transition_function_of_cell<
            class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,std::false_type
            >
{
    static single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_cell
    get_function( neural_tissue* const tissue )
    {
        return std::bind(
                    &private_internal_implementation_details::transition_function_of_cell<
                        class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling
                        >,
                    tissue,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4,
                    std::placeholders::_5,
                    std::placeholders::_6,
                    std::placeholders::_7
                    );
    }
};


}}

#endif
