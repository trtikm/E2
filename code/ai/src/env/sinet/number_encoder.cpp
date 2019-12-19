#include <ai/env/sinet/number_encoder.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace env { namespace sinet {


number_encoder::number_encoder(
        natural_8_bit const  num_units,
        float_32_bit const  min_value,
        float_32_bit const  max_value,
        float_32_bit const  value_update_life_time_in_seconds,
        float_32_bit const  event_frequency_in_Hz
        )
    : MIN_VALUE(min_value)
    , MAX_VALUE(max_value)
    , VALUE_UPDATE_LIFE_TIME_IN_SECONDS(value_update_life_time_in_seconds)
    , EVENT_DURATION_IN_SECONDS(1.0f / event_frequency_in_Hz)
    , unit_states()
{
    ASSUMPTION(num_units > 0U &&min_value <= max_value && value_update_life_time_in_seconds >= 0.0f && event_frequency_in_Hz > 1.0f);

    unit_states.resize(num_units);
    for (unit_state& state : unit_states)
    {
        state.seconds_since_last_update = VALUE_UPDATE_LIFE_TIME_IN_SECONDS + 1.0f;
        state.seconds_since_last_event = EVENT_DURATION_IN_SECONDS;
    }
}


void  number_encoder::next_round(
        float_32_bit const  time_step_in_seconds,
        float_32_bit const* const  new_value_ptr,
        std::unordered_set<natural_8_bit>&  spiking_units
        )
{
    TMPROF_BLOCK();

    for (unit_state& state : unit_states)
        if (state.seconds_since_last_update <= VALUE_UPDATE_LIFE_TIME_IN_SECONDS)
            state.seconds_since_last_update += time_step_in_seconds;

    if (new_value_ptr != nullptr)
    {
        float_32_bit  new_value_in_range_01;
        if (*new_value_ptr >= MAX_VALUE)
            new_value_in_range_01 = 1.0f;
        else if (*new_value_ptr <= MIN_VALUE)
            new_value_in_range_01 = 0.0f;
        else
        {
            INVARIANT(MIN_VALUE < MAX_VALUE);
            new_value_in_range_01 = (*new_value_ptr - MIN_VALUE) / (MAX_VALUE - MIN_VALUE);
        }
        std::vector<natural_8_bit>  updated_units;
        decompose_value_to_units(new_value_in_range_01, (natural_8_bit)unit_states.size(), updated_units);
        for (natural_8_bit  idx : updated_units)
            unit_states.at(idx).seconds_since_last_update = 0.0f;
    }

    for (natural_8_bit  idx = 0U; idx != unit_states.size(); ++idx)
    {
        unit_state&  state = unit_states.at(idx);
        if (state.seconds_since_last_update <= VALUE_UPDATE_LIFE_TIME_IN_SECONDS)
            for (state.seconds_since_last_event += time_step_in_seconds;
                 state.seconds_since_last_event >= EVENT_DURATION_IN_SECONDS;
                 state.seconds_since_last_event -= EVENT_DURATION_IN_SECONDS)
            {
                spiking_units.insert(idx);
            }
    }
}


number_unary_encoder::number_unary_encoder(
        natural_8_bit const  num_units,
        float_32_bit const  min_value,
        float_32_bit const  max_value,
        float_32_bit const  value_update_life_time_in_seconds,
        float_32_bit const  event_frequency_in_Hz
        )
    : number_encoder(num_units, min_value, max_value, value_update_life_time_in_seconds, event_frequency_in_Hz)
{}


void  number_unary_encoder::decompose_value_to_units(
        float_32_bit const  value_in_range_01,
        natural_8_bit const  num_units,
        std::vector<natural_8_bit>&  units
        ) const
{
    natural_8_bit const  index = (natural_8_bit)((natural_32_bit)(value_in_range_01 * (num_units - 1U) + 0.5f) & 0xffU);
    units.push_back(index);
}


number_binary_encoder::number_binary_encoder(
        natural_8_bit const  num_units,
        float_32_bit const  min_value,
        float_32_bit const  max_value,
        float_32_bit const  value_update_life_time_in_seconds,
        float_32_bit const  event_frequency_in_Hz
        )
    : number_encoder(num_units, min_value, max_value, value_update_life_time_in_seconds, event_frequency_in_Hz)
{
    ASSUMPTION(num_units <= 8U * sizeof(natural_8_bit));
}


void  number_binary_encoder::decompose_value_to_units(
        float_32_bit const  value_in_range_01,
        natural_8_bit const  num_units,
        std::vector<natural_8_bit>&  units
        ) const
{
    natural_8_bit  value = (natural_8_bit)((natural_32_bit)(value_in_range_01 * ((1U << num_units) - 1U) + 0.5f) & 0xffU);
    for (natural_8_bit  idx = 0U; value != 0U; value >>= 1U, ++idx)
    {
        if ((value & 1U) != 0U)
            units.push_back(idx);
    }
}


}}}
