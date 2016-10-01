#include <netlab/network_object_id.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/development.hpp>

namespace netlab { namespace detail { namespace {


natural_64_bit  compress(
        natural_8_bit const  object_kind,
        natural_8_bit const  index_of_network_layer,
        natural_64_bit const  index_in_network_layer
        )
{
    NOT_IMPLEMENTED_YET();
}


}}}

namespace netlab {


network_object_id::network_object_id(
        natural_8_bit const  object_kind,
        natural_8_bit const  index_of_network_layer,
        natural_64_bit const  index_in_network_layer
        )
    : m_compressed_data(detail::compress(object_kind,index_of_network_layer,index_in_network_layer))
{}

natural_8_bit  network_object_id::object_kind() const noexcept
{
    // TODO!
    return 0;
}

natural_8_bit  network_object_id::index_of_network_layer() const noexcept
{
    // TODO!
    return 0;
}

natural_64_bit  network_object_id::index_into_network_layer() const noexcept
{
    // TODO!
    return 0;
}


}
