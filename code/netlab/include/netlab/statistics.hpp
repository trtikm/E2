#ifndef NETLAB_STATISTICS_HPP_INCLUDED
#   define NETLAB_STATISTICS_HPP_INCLUDED

#   include <netlab/uid.hpp>
#   include <netlab/layer.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <deque>
#   include <unordered_map>

namespace netlab {


struct  statistics
{
    struct  probe
    {
        natural_32_bit  num_spikes_received;
        natural_32_bit  num_spikes_produced;
        natural_32_bit  num_connected_input_sockets;
        natural_32_bit  num_connected_output_sockets;
        natural_32_bit  num_disconnected_input_sockets;
        natural_32_bit  num_disconnected_output_sockets;
        probe();
    };

    struct  overall
    {
        natural_32_bit  num_spikes;
        natural_32_bit  num_connected_sockets;
        natural_32_bit  num_disconnected_sockets;
        overall();
    };

    // CONSTANTS:

    natural_32_bit  NUM_ROUNDS_PER_SNAPSHOT;                        // When == 0, then statistics are NOT collected/updated!
    natural_32_bit  SNAPSHOTS_HISTORY_SIZE;
    float_32_bit  RATIO_OF_PROBED_UNITS_PER_LAYER;

    // DATA:

    natural_64_bit  num_passed_rounds;
    natural_32_bit  num_passed_rounds_in_current_snapshot;
    std::unordered_map<uid, std::deque<probe> >  probes_history;    // For each uid, the size of the deque is > 0
                                                                    // and <= SNAPSHOTS_HISTORY_SIZE + 1.
                                                                    // The front element in each deque is the probe of the
                                                                    // currently processed snapshot, for the related unit's uid.
    std::deque<overall>  overall_history;   // The size is > 0 and <= SNAPSHOTS_HISTORY_SIZE + 1. The front element
                                            // is the one of the currently processed snapshot.

    // FUNCTIONS:

    statistics();
    statistics(
        natural_32_bit const  NUM_ROUNDS_PER_SNAPSHOT_,
        natural_32_bit const  SNAPSHOTS_HISTORY_SIZE_,
        float_32_bit const  RATIO_OF_PROBED_UNITS_PER_LAYER_
        );

    bool  enabled() const { return NUM_ROUNDS_PER_SNAPSHOT != 0U; }
    void  on_next_round();
    void  on_spike_received(uid const  id);
    void  on_spike_produced(uid const  id);
    void  on_connect(uid const  iid, uid const  oid);
    void  on_disconnect(
            std::vector<input_socket> const&  inputs,
            uid const  input_socket_id,
            std::vector<output_socket> const&  outputs,
            uid const  output_socket_id
            );
};


}

#endif
