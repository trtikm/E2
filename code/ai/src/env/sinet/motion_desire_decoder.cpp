#include <ai/env/sinet/motion_desire_decoder.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <algorithm>

namespace ai { namespace env { namespace sinet {


motion_desire_decoder::motion_desire_decoder(motion_desire_props&  props, natural_8_bit const  layer_index)
    : LAYER_INDEX(layer_index)
    , SEPARATOR_UNIT_INDICES()
    , decoders()
{
    natural_16_bit  unit_index = 0U;
    SEPARATOR_UNIT_INDICES.push_back(unit_index);

    auto register_number = [this, &unit_index](float_32_bit&  number) -> void {
        decoders.push_back(number_decoder(
            0.0f, 0.1f, 0.1f, 0.5f, 0.1f, 0.1f, // TODO: use proper values instead of these!!!
            &number
        ));

        natural_16_bit const  NUM_UNITS_PER_ENCODER = 20U;
        unit_index += NUM_UNITS_PER_ENCODER;
        SEPARATOR_UNIT_INDICES.push_back(unit_index);
    };

    auto register_vector = [&register_number](vector3&  u) -> void {
        register_number(u(0));
        register_number(u(1));
        register_number(u(2));
    };

    auto register_vector4 = [&register_number](vector4&  u) -> void {
        register_number(u(0));
        register_number(u(1));
        register_number(u(2));
        register_number(u(3));
    };

    register_vector4(props.speed);
    register_vector(props.look_at_target);

    INVARIANT(SEPARATOR_UNIT_INDICES.size() == decoders.size() + 1UL);
}


void  motion_desire_decoder::next_round(
        float_32_bit const  time_step_in_seconds,
        std::vector<netlab::simple::uid> const&  output_events
        )
{
    for (netlab::simple::uid  uid : output_events)
    {
        if (uid.layer != LAYER_INDEX || uid.unit >= SEPARATOR_UNIT_INDICES.back())
            continue;
        auto const  end = std::upper_bound(SEPARATOR_UNIT_INDICES.cbegin(), SEPARATOR_UNIT_INDICES.cend(), uid.unit);
        INVARIANT(end != SEPARATOR_UNIT_INDICES.cbegin());
        auto const  begin = std::prev(end);
        INVARIANT(uid.unit >= *begin && uid.unit < *end);
        natural_16_bit const  num_units = *end - *begin;
        natural_16_bit const  unit_idx = uid.unit - *begin;
        auto const  decoder_idx = std::distance(SEPARATOR_UNIT_INDICES.cbegin(), begin);
        INVARIANT((std::size_t)decoder_idx < decoders.size());
        decoders.at(decoder_idx).on_spike(unit_idx <= (num_units >> 1) ? 1.0f : -1.0f);
    }
    for (number_decoder&  decoder : decoders)
        decoder.next_round(time_step_in_seconds);
}


}}}
