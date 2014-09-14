#ifndef CELLAB_NEURAL_TISSUE_HPP_INCLUDED
#   define CELLAB_NEURAL_TISSUE_HPP_INCLUDED

#   include <cellab/static_state_of_neural_tissue.hpp>
#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <cellab/territorial_state_of_synapse.hpp>
#   include <cellab/shift_in_coordinates.hpp>
#   include <cellab/transition_algorithms.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/instance_wrapper.hpp>
#   include <utility/invariants.hpp>
#   include <boost/noncopyable.hpp>
#   include <type_traits>
#   include <functional>
#   include <typeinfo>
#   include <tuple>
#   include <memory>
#   include <new>

namespace cellab {


struct neural_tissue;

template<typename class_derived_from_neural_tissue,
         typename class_cell = bits_reference,
         typename class_synapse = bits_reference,
         typename class_signalling = bits_reference>
struct automated_binding_of_transition_functions
{
    static_assert(
            std::is_base_of<neural_tissue,class_derived_from_neural_tissue>::value,
            "The passed class with transition functions has to be a class derived "
            "form the neural_tissue base."
            "   If this requirement is too strong for you, then use the the other "
            "constructor of the neural_tissue which does not accept an instance "
            "of this automated-binding class."
            );
    static_assert(
            ( std::is_same<class_cell,bits_reference>::value &&
              std::is_same<class_synapse,bits_reference>::value &&
              std::is_same<class_signalling,bits_reference>::value ) ||
            ( ! std::is_same<class_cell,bits_reference>::value &&
              ! std::is_same<class_synapse,bits_reference>::value &&
              ! std::is_same<class_signalling,bits_reference>::value ),
            "There are supported only two variants of the configuration. Either all your "
            "transition functions directly work with the packed information (i.e. they use "
            "bits_references only) or all your transition functions work with unpacked "
            "information (i.e. special separate classes are provided). Each such special class "
            "has to be constructible from bits_const_reference and it has to define operator>> "
            "to pack its data into a passed bits_reference."
            "   If this requirement is too strong for you, then use the the other "
            "constructor of the neural_tissue which does not accept an instance "
            "of this automated-binding class."
            );

    static bool const use_bits_references = std::is_same<class_cell,bits_reference>::value;
};


struct neural_tissue : private boost::noncopyable
{
    template<typename class_derived_from_neural_tissue,
             typename class_cell,
             typename class_synapse,
             typename class_signalling>
    neural_tissue(
            std::shared_ptr<cellab::dynamic_state_of_neural_tissue>
                dynamic_state_of_tissue,
            automated_binding_of_transition_functions<class_derived_from_neural_tissue,
                                                      class_cell,class_synapse,class_signalling>
                auto_binding
            );

    neural_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue>
            dynamic_state_of_tissue,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_to_muscle
            transition_function_of_packed_synapse_to_muscle,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue
            transition_function_of_packed_synapse_inside_tissue,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_signalling
            transition_function_of_packed_signalling,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_cell
            transition_function_of_packed_cell
        );

    virtual ~neural_tissue() {}

    std::shared_ptr<static_state_of_neural_tissue const>  get_static_state_of_neural_tissue() const;
    std::shared_ptr<dynamic_state_of_neural_tissue>  get_dynamic_state_of_neural_tissue();

    void  apply_transition_of_synapses_to_muscles(
            natural_32_bit num_avalilable_thread_for_creation_and_use
            );

    void  apply_transition_of_synapses_of_tissue(
            natural_32_bit num_avalilable_thread_for_creation_and_use
            );

    void  apply_transition_of_territorial_lists_of_synapses(
            natural_32_bit num_avalilable_thread_for_creation_and_use
            );

    void  apply_transition_of_synaptic_migration_in_tissue(
            natural_32_bit num_avalilable_thread_for_creation_and_use
            );

    void  apply_transition_of_signalling_in_tissue(
            natural_32_bit num_avalilable_thread_for_creation_and_use
            );

    void  apply_transition_of_cells_of_tissue(
            natural_32_bit num_avalilable_thread_for_creation_and_use
            );

    std::size_t get_hash_code_of_class_for_cells() const;
    std::size_t get_hash_code_of_class_for_synapses() const;
    std::size_t get_hash_code_of_class_for_signalling() const;

private:

    std::shared_ptr<cellab::dynamic_state_of_neural_tissue>
        m_dynamic_state_of_tissue;

    single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_to_muscle
        m_transition_function_of_packed_synapse_to_muscle;
    single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue
        m_transition_function_of_packed_synapse_inside_tissue;
    single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_signalling
        m_transition_function_of_packed_signalling;
    single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_cell
        m_transition_function_of_packed_cell;

    std::size_t m_hash_code_of_class_for_cells;
    std::size_t m_hash_code_of_class_for_synapses;
    std::size_t m_hash_code_of_class_for_signalling;
};


namespace private_internal_implementation_details {

template<typename class_signalling>
kind_of_cell  get_signalling_callback_function(
        std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)> const&
            get_signalling,
        shift_in_coordinates const& shift,
        instance_wrapper<class_signalling>& signalling_instance
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
        instance_wrapper<class_cell>& cell_instance
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
        instance_wrapper<class_synapse>& synapse_instance
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
        kind_of_cell const kind_of_source_cell,
        bits_const_reference const& bits_of_source_cell
        )
{
    class_synapse  synapse_to_be_updated( bits_const_reference(bits_of_synapse_to_be_updated) );
    class_cell const  source_cell( bits_of_source_cell );
    INVARIANT(dynamic_cast<class_derived_from_neural_tissue*>(tissue) != nullptr);
    static_cast<class_derived_from_neural_tissue*>(tissue)->transition_function_of_synapse_to_muscle(
                synapse_to_be_updated,
                kind_of_source_cell,
                source_cell
                );
    synapse_to_be_updated >> bits_of_synapse_to_be_updated;
}

template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling>
void transition_function_of_synapse_inside_tissue(
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
    class_synapse  synapse_to_be_updated( bits_const_reference(bits_of_synapse_to_be_updated) );
    class_cell const  source_cell( bits_of_source_cell );
    class_cell const  territory_cell( bits_of_territory_cell );
    INVARIANT(dynamic_cast<class_derived_from_neural_tissue*>(tissue) != nullptr);
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
    class_signalling  signalling_to_be_updated( bits_const_reference(bits_of_signalling_to_be_updated) );
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
    class_cell  cell_to_be_updated( bits_const_reference(bits_of_cell_to_be_updated) );
    INVARIANT(dynamic_cast<class_derived_from_neural_tissue*>(tissue) != nullptr);
    static_cast<class_derived_from_neural_tissue*>(tissue)->transition_function_of_synapse_inside_tissue(
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

}


template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling>
neural_tissue::neural_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue>
            dynamic_state_of_tissue,
        automated_binding_of_transition_functions<class_derived_from_neural_tissue,
                                                  class_cell,class_synapse,class_signalling>
            auto_binding
        )
    : m_dynamic_state_of_tissue(dynamic_state_of_tissue)
    , m_transition_function_of_packed_synapse_to_muscle(
        auto_binding.use_bits_references ?
                std::bind(
                    &class_derived_from_neural_tissue::transition_function_of_synapse_to_muscle,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3
                    )
            :
                std::bind(
                    &private_internal_implementation_details::transition_function_of_synapse_to_muscle<
                        class_derived_from_neural_tissue,class_cell,class_synapse
                        >,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3
                    )
        )
    , m_transition_function_of_packed_synapse_inside_tissue(
        auto_binding.use_bits_references ?
                std::bind(
                    &class_derived_from_neural_tissue::transition_function_of_synapse_inside_tissue,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4,
                    std::placeholders::_5,
                    std::placeholders::_6,
                    std::placeholders::_7,
                    std::placeholders::_8,
                    std::placeholders::_9
                    )
            :
                std::bind(
                    &private_internal_implementation_details::transition_function_of_synapse_inside_tissue<
                          class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling
                          >,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4,
                    std::placeholders::_5,
                    std::placeholders::_6,
                    std::placeholders::_7,
                    std::placeholders::_8,
                    std::placeholders::_9
                    )
        )
    , m_transition_function_of_packed_signalling(
        auto_binding.use_bits_references ?
                std::bind(
                    &class_derived_from_neural_tissue::transition_function_of_signalling,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4,
                    std::placeholders::_5
                    )
            :
                std::bind(
                    &private_internal_implementation_details::transition_function_of_signalling<
                        class_derived_from_neural_tissue,class_cell,class_signalling
                        >,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4,
                    std::placeholders::_5
                    )
        )
    , m_transition_function_of_packed_cell(
        auto_binding.use_bits_references ?
                std::bind(
                    &class_derived_from_neural_tissue::transition_function_of_cell,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4,
                    std::placeholders::_5
                    )
            :
                std::bind(
                    &private_internal_implementation_details::transition_function_of_cell<
                        class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling
                        >,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3,
                    std::placeholders::_4,
                    std::placeholders::_5
                    )
        )
    , m_hash_code_of_class_for_cells(typeid(class_cell).hash_code())
    , m_hash_code_of_class_for_synapses(typeid(class_synapse).hash_code())
    , m_hash_code_of_class_for_signalling(typeid(class_signalling).hash_code())
{
    ASSUMPTION(m_dynamic_state_of_tissue.operator bool());
}


}

#endif
