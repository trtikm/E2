#ifndef AI_ENV_SINET_NUMBER_ENCODER_HPP_INCLUDED
#   define AI_ENV_SINET_NUMBER_ENCODER_HPP_INCLUDED

#   include <netlab/simple_network.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <unordered_set>

namespace ai { namespace env { namespace sinet {


struct  number_encoder
{
    number_encoder(
            natural_8_bit const  num_units,
            float_32_bit const  min_value,
            float_32_bit const  max_value,
            float_32_bit const  value_update_life_time_in_seconds,
            float_32_bit const  event_frequency_in_Hz
            );
    virtual ~number_encoder() {}

    void  next_round(
            float_32_bit const  time_step_in_seconds,
            float_32_bit const* const  new_value_ptr,   // pass nullptr, when no new value is available.
            std::unordered_set<natural_8_bit>&  spiking_units
            );

    natural_8_bit  num_units() const { return (natural_8_bit)unit_states.size(); }

protected:

    virtual void  decompose_value_to_units(
            float_32_bit const  value_in_range_01,
            natural_8_bit const  num_units,
            std::vector<natural_8_bit>&  units
            ) const = 0;

private:

    struct  unit_state
    {
        float_32_bit  seconds_since_last_update;
        float_32_bit  seconds_since_last_event;
    };

    // CONSTANTS:

    float_32_bit  MIN_VALUE;
    float_32_bit  MAX_VALUE;
    float_32_bit  VALUE_UPDATE_LIFE_TIME_IN_SECONDS;
    float_32_bit  EVENT_DURATION_IN_SECONDS;

    // DATA:

    std::vector<unit_state>  unit_states;
};


struct  number_unary_encoder : public number_encoder
{
    number_unary_encoder(
            natural_8_bit const  num_units,
            float_32_bit const  min_value,
            float_32_bit const  max_value,
            float_32_bit const  value_update_life_time_in_seconds,
            float_32_bit const  event_frequency_in_Hz
            );

protected:

    void  decompose_value_to_units(
            float_32_bit const  value_in_range_01,
            natural_8_bit const  num_units,
            std::vector<natural_8_bit>& units
            ) const override;
};


struct  number_binary_encoder : public number_encoder
{
    number_binary_encoder(
            natural_8_bit const  num_units,
            float_32_bit const  min_value,
            float_32_bit const  max_value,
            float_32_bit const  value_update_life_time_in_seconds,
            float_32_bit const  event_frequency_in_Hz
            );

protected:

    void  decompose_value_to_units(
            float_32_bit const  value_in_range_01,
            natural_8_bit const  num_units,
            std::vector<natural_8_bit>& units
            ) const override;
};


}}}

#endif
