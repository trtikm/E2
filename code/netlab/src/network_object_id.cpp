#include <netlab/network_object_id.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/development.hpp>

namespace netlab { namespace detail { namespace {


constexpr natural_64_bit  shift_for_kind() noexcept { return 8UL*7UL; }
constexpr natural_64_bit  shift_for_layer() noexcept { return 8UL*6UL; }


}}}

namespace netlab {


network_object_id::network_object_id(
        natural_8_bit const  object_kind,
        natural_8_bit const  index_of_network_layer,
        natural_64_bit const  index_in_network_layer
        )
    : m_compressed_data(
          (static_cast<natural_64_bit>((object_kind & max_num_object_kinds())) << detail::shift_for_kind()) |
          (static_cast<natural_64_bit>((index_of_network_layer & max_num_network_layers())) << detail::shift_for_layer()) |
          (index_in_network_layer & max_num_objects_in_a_layer())
          )
{}

natural_8_bit  network_object_id::object_kind() const noexcept
{
    return static_cast<natural_8_bit>(m_compressed_data >> detail::shift_for_kind());
}

natural_8_bit  network_object_id::index_of_network_layer() const noexcept
{
    return static_cast<natural_8_bit>(m_compressed_data >> detail::shift_for_layer());
}

natural_64_bit  network_object_id::index_into_network_layer() const noexcept
{
    return m_compressed_data & max_num_objects_in_a_layer();
}


}
