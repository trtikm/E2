#include <netlab/network_indices.hpp>

namespace netlab { namespace detail { namespace {


inline constexpr natural_32_bit  shift_for_layer() noexcept { return 8U*7U; }


}}}

namespace netlab {


compressed_layer_and_object_indices::compressed_layer_and_object_indices(
        layer_index_type const  layer_index,
        object_index_type const  object_index
        )
    : m_compressed_indices(
          (static_cast<decltype(m_compressed_indices)>((layer_index & max_layer_index())) << detail::shift_for_layer()) |
          static_cast<decltype(m_compressed_indices)>(object_index & max_object_index())
          )
{}

layer_index_type  compressed_layer_and_object_indices::layer_index() const
{
    return static_cast<layer_index_type>(m_compressed_indices >> detail::shift_for_layer());
}

object_index_type  compressed_layer_and_object_indices::object_index() const
{
    return static_cast<object_index_type>(
                m_compressed_indices & static_cast<decltype(m_compressed_indices)>(max_object_index())
                );
}


}
