#ifndef NETLAB_TRACKED_OBJECT_STATS_HPP_INCLUDED
#   define NETLAB_TRACKED_OBJECT_STATS_HPP_INCLUDED

#   include <netlab/network_indices.hpp>
#   include <angeo/tensor_math.hpp>

namespace netlab {


struct  tracked_network_object_stats
{
    explicit tracked_network_object_stats(compressed_layer_and_object_indices const  indices) noexcept
        : m_indices(indices)
    {}

    virtual ~tracked_network_object_stats() {}

    compressed_layer_and_object_indices  indices() const noexcept { return m_indices; }

private:
    compressed_layer_and_object_indices  m_indices;
};


/**
 *
 *
 *
 *
 */
struct  tracked_spiker_stats : public tracked_network_object_stats
{
    explicit tracked_spiker_stats(compressed_layer_and_object_indices const  indices) noexcept
        : tracked_network_object_stats(indices)
    {}

    virtual ~tracked_spiker_stats() {}
};


/**
 *
 *
 *
 *
 */
struct  tracked_dock_stats : public tracked_network_object_stats
{
    explicit tracked_dock_stats(compressed_layer_and_object_indices const  indices) noexcept
        : tracked_network_object_stats(indices)
    {}

    virtual ~tracked_dock_stats() {}
};


/**
 *
 *
 *
 *
 */
struct  tracked_ship_stats : public tracked_network_object_stats
{
    explicit tracked_ship_stats(compressed_layer_and_object_indices const  indices) noexcept
        : tracked_network_object_stats(indices)
    {}

    virtual ~tracked_ship_stats() {}
};


}

#endif
