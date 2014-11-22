#ifndef CELLAB_NEURAL_TISSUE_HPP_INCLUDED
#   define CELLAB_NEURAL_TISSUE_HPP_INCLUDED

#   include <cellab/static_state_of_neural_tissue.hpp>
#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <cellab/transition_algorithms.hpp>
#   include <cellab/utilities_for_construction_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/noncopyable.hpp>
#   include <type_traits>
#   include <typeinfo>
#   include <memory>

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
            natural_16_bit const num_kinds_of_tissue_cells,
            natural_16_bit const num_kinds_of_sensory_cells,
            natural_16_bit const num_bits_per_cell,
            natural_16_bit const num_bits_per_synapse,
            natural_16_bit const num_bits_per_signalling,
            natural_32_bit const num_cells_along_x_axis,
            natural_32_bit const num_cells_along_y_axis,
            std::vector<natural_32_bit> const& num_tissue_cells_of_cell_kind,
            std::vector<natural_32_bit> const& num_synapses_in_territory_of_cell_kind,
            std::vector<natural_32_bit> const& num_sensory_cells_of_cell_kind,
            natural_32_bit const num_synapses_to_muscles,
            bool const is_x_axis_torus_axis,
            bool const is_y_axis_torus_axis,
            bool const is_columnar_axis_torus_axis,
            std::vector<integer_8_bit> const& x_radius_of_signalling_neighbourhood_of_cell,
            std::vector<integer_8_bit> const& y_radius_of_signalling_neighbourhood_of_cell,
            std::vector<integer_8_bit> const& columnar_radius_of_signalling_neighbourhood_of_cell,
            std::vector<integer_8_bit> const& x_radius_of_signalling_neighbourhood_of_synapse,
            std::vector<integer_8_bit> const& y_radius_of_signalling_neighbourhood_of_synapse,
            std::vector<integer_8_bit> const& columnar_radius_of_signalling_neighbourhood_of_synapse,
            std::vector<integer_8_bit> const& x_radius_of_cellular_neighbourhood_of_signalling,
            std::vector<integer_8_bit> const& y_radius_of_cellular_neighbourhood_of_signalling,
            std::vector<integer_8_bit> const& columnar_radius_of_cellular_neighbourhood_of_signalling,
            automated_binding_of_transition_functions<class_derived_from_neural_tissue,
                                                      class_cell,class_synapse,class_signalling>
                auto_binding
            );

    template<typename class_derived_from_neural_tissue,
             typename class_cell,
             typename class_synapse,
             typename class_signalling>
    neural_tissue(
            std::shared_ptr<cellab::static_state_of_neural_tissue const>
                static_state_of_tissue,
            automated_binding_of_transition_functions<class_derived_from_neural_tissue,
                                                      class_cell,class_synapse,class_signalling>
                auto_binding
            );

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
            natural_32_bit const  num_threads_avalilable_for_computation
            );

    void  apply_transition_of_synapses_of_tissue(
            natural_32_bit const  num_threads_avalilable_for_computation
            );

    void  apply_transition_of_territorial_lists_of_synapses(
            natural_32_bit const  num_threads_avalilable_for_computation
            );

    void  apply_transition_of_synaptic_migration_in_tissue(
            natural_32_bit const  num_threads_avalilable_for_computation
            );

    void  apply_transition_of_signalling_in_tissue(
            natural_32_bit const  num_threads_avalilable_for_computation
            );

    void  apply_transition_of_cells_of_tissue(
            natural_32_bit const  num_threads_avalilable_for_computation
            );

    std::size_t  get_hash_code_of_class_for_cells() const;
    std::size_t  get_hash_code_of_class_for_synapses() const;
    std::size_t  get_hash_code_of_class_for_signalling() const;

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

    std::size_t  m_hash_code_of_class_for_cells;
    std::size_t  m_hash_code_of_class_for_synapses;
    std::size_t  m_hash_code_of_class_for_signalling;
};


template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling>
neural_tissue::neural_tissue(
        natural_16_bit const num_kinds_of_tissue_cells,
        natural_16_bit const num_kinds_of_sensory_cells,
        natural_16_bit const num_bits_per_cell,
        natural_16_bit const num_bits_per_synapse,
        natural_16_bit const num_bits_per_signalling,
        natural_32_bit const num_cells_along_x_axis,
        natural_32_bit const num_cells_along_y_axis,
        std::vector<natural_32_bit> const& num_tissue_cells_of_cell_kind,
        std::vector<natural_32_bit> const& num_synapses_in_territory_of_cell_kind,
        std::vector<natural_32_bit> const& num_sensory_cells_of_cell_kind,
        natural_32_bit const num_synapses_to_muscles,
        bool const is_x_axis_torus_axis,
        bool const is_y_axis_torus_axis,
        bool const is_columnar_axis_torus_axis,
        std::vector<integer_8_bit> const& x_radius_of_signalling_neighbourhood_of_cell,
        std::vector<integer_8_bit> const& y_radius_of_signalling_neighbourhood_of_cell,
        std::vector<integer_8_bit> const& columnar_radius_of_signalling_neighbourhood_of_cell,
        std::vector<integer_8_bit> const& x_radius_of_signalling_neighbourhood_of_synapse,
        std::vector<integer_8_bit> const& y_radius_of_signalling_neighbourhood_of_synapse,
        std::vector<integer_8_bit> const& columnar_radius_of_signalling_neighbourhood_of_synapse,
        std::vector<integer_8_bit> const& x_radius_of_cellular_neighbourhood_of_signalling,
        std::vector<integer_8_bit> const& y_radius_of_cellular_neighbourhood_of_signalling,
        std::vector<integer_8_bit> const& columnar_radius_of_cellular_neighbourhood_of_signalling,
        automated_binding_of_transition_functions<class_derived_from_neural_tissue,
                                                  class_cell,class_synapse,class_signalling>
            auto_binding
        )
    : m_dynamic_state_of_tissue(
            new dynamic_state_of_neural_tissue(
                    std::shared_ptr<static_state_of_neural_tissue>(new static_state_of_neural_tissue(
                            num_kinds_of_tissue_cells,
                            num_kinds_of_sensory_cells,
                            num_bits_per_cell,
                            num_bits_per_synapse,
                            num_bits_per_signalling,
                            num_cells_along_x_axis,
                            num_cells_along_y_axis,
                            num_tissue_cells_of_cell_kind,
                            num_synapses_in_territory_of_cell_kind,
                            num_sensory_cells_of_cell_kind,
                            num_synapses_to_muscles,
                            is_x_axis_torus_axis,
                            is_y_axis_torus_axis,
                            is_columnar_axis_torus_axis,
                            x_radius_of_signalling_neighbourhood_of_cell,
                            y_radius_of_signalling_neighbourhood_of_cell,
                            columnar_radius_of_signalling_neighbourhood_of_cell,
                            x_radius_of_signalling_neighbourhood_of_synapse,
                            y_radius_of_signalling_neighbourhood_of_synapse,
                            columnar_radius_of_signalling_neighbourhood_of_synapse,
                            x_radius_of_cellular_neighbourhood_of_signalling,
                            y_radius_of_cellular_neighbourhood_of_signalling,
                            columnar_radius_of_cellular_neighbourhood_of_signalling
                            ))
                    )
            )
    , m_transition_function_of_packed_synapse_to_muscle(
            private_internal_implementation_details::bind_transition_function_of_synapse_to_muscle<
                class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,
                typename std::conditional<auto_binding.use_bits_references,std::true_type,std::false_type>::type
                >::get_function(this)
            //private_internal_implementation_details::bind_transition_function_of_synapse_to_muscle(this,auto_binding)
            )
    , m_transition_function_of_packed_synapse_inside_tissue(
            private_internal_implementation_details::bind_transition_function_of_synapse_inside_tissue<
                class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,
                typename std::conditional<auto_binding.use_bits_references,std::true_type,std::false_type>::type
                >::get_function(this)
            //private_internal_implementation_details::bind_transition_function_of_synapse_inside_tissue(this,auto_binding)
            )
    , m_transition_function_of_packed_signalling(
            private_internal_implementation_details::bind_transition_function_of_signalling<
                class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,
                typename std::conditional<auto_binding.use_bits_references,std::true_type,std::false_type>::type
                >::get_function(this)
            //private_internal_implementation_details::bind_transition_function_of_signalling(this,auto_binding)
            )
    , m_transition_function_of_packed_cell(
            private_internal_implementation_details::bind_transition_function_of_cell<
                class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,
                typename std::conditional<auto_binding.use_bits_references,std::true_type,std::false_type>::type
                >::get_function(this)
            //private_internal_implementation_details::transition_function_of_cell(this,auto_binding)
            )
    , m_hash_code_of_class_for_cells(typeid(class_cell).hash_code())
    , m_hash_code_of_class_for_synapses(typeid(class_synapse).hash_code())
    , m_hash_code_of_class_for_signalling(typeid(class_signalling).hash_code())
{

}


template<typename class_derived_from_neural_tissue,
         typename class_cell,
         typename class_synapse,
         typename class_signalling>
neural_tissue::neural_tissue(
        std::shared_ptr<cellab::static_state_of_neural_tissue const>
            static_state_of_tissue,
        automated_binding_of_transition_functions<class_derived_from_neural_tissue,
                                                  class_cell,class_synapse,class_signalling>
            auto_binding
        )
    : m_dynamic_state_of_tissue(
            new dynamic_state_of_neural_tissue( static_state_of_tissue )
            )
    , m_transition_function_of_packed_synapse_to_muscle(
            private_internal_implementation_details::bind_transition_function_of_synapse_to_muscle<
                class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,
                typename std::conditional<auto_binding.use_bits_references,std::true_type,std::false_type>::type
                >::get_function(this)
            //private_internal_implementation_details::bind_transition_function_of_synapse_to_muscle(this,auto_binding)
            )
    , m_transition_function_of_packed_synapse_inside_tissue(
            private_internal_implementation_details::bind_transition_function_of_synapse_inside_tissue<
                class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,
                typename std::conditional<auto_binding.use_bits_references,std::true_type,std::false_type>::type
                >::get_function(this)
            //private_internal_implementation_details::bind_transition_function_of_synapse_inside_tissue(this,auto_binding)
            )
    , m_transition_function_of_packed_signalling(
            private_internal_implementation_details::bind_transition_function_of_signalling<
                class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,
                typename std::conditional<auto_binding.use_bits_references,std::true_type,std::false_type>::type
                >::get_function(this)
            //private_internal_implementation_details::bind_transition_function_of_signalling(this,auto_binding)
            )
    , m_transition_function_of_packed_cell(
            private_internal_implementation_details::bind_transition_function_of_cell<
                class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,
                typename std::conditional<auto_binding.use_bits_references,std::true_type,std::false_type>::type
                >::get_function(this)
            //private_internal_implementation_details::transition_function_of_cell(this,auto_binding)
            )
    , m_hash_code_of_class_for_cells(typeid(class_cell).hash_code())
    , m_hash_code_of_class_for_synapses(typeid(class_synapse).hash_code())
    , m_hash_code_of_class_for_signalling(typeid(class_signalling).hash_code())
{
    ASSUMPTION(m_dynamic_state_of_tissue.operator bool());
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
            private_internal_implementation_details::bind_transition_function_of_synapse_to_muscle<
                class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,
                typename std::conditional<auto_binding.use_bits_references,std::true_type,std::false_type>::type
                >::get_function(this)
            //private_internal_implementation_details::bind_transition_function_of_synapse_to_muscle(this,auto_binding)
            )
    , m_transition_function_of_packed_synapse_inside_tissue(
            private_internal_implementation_details::bind_transition_function_of_synapse_inside_tissue<
                class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,
                typename std::conditional<auto_binding.use_bits_references,std::true_type,std::false_type>::type
                >::get_function(this)
            //private_internal_implementation_details::bind_transition_function_of_synapse_inside_tissue(this,auto_binding)
            )
    , m_transition_function_of_packed_signalling(
            private_internal_implementation_details::bind_transition_function_of_signalling<
                class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,
                typename std::conditional<auto_binding.use_bits_references,std::true_type,std::false_type>::type
                >::get_function(this)
            //private_internal_implementation_details::bind_transition_function_of_signalling(this,auto_binding)
            )
    , m_transition_function_of_packed_cell(
            private_internal_implementation_details::bind_transition_function_of_cell<
                class_derived_from_neural_tissue,class_cell,class_synapse,class_signalling,
                typename std::conditional<auto_binding.use_bits_references,std::true_type,std::false_type>::type
                >::get_function(this)
            //private_internal_implementation_details::transition_function_of_cell(this,auto_binding)
            )
    , m_hash_code_of_class_for_cells(typeid(class_cell).hash_code())
    , m_hash_code_of_class_for_synapses(typeid(class_synapse).hash_code())
    , m_hash_code_of_class_for_signalling(typeid(class_signalling).hash_code())
{
    ASSUMPTION(m_dynamic_state_of_tissue.operator bool());
}


}

#endif
