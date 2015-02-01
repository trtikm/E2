#include <cellconnect/utility/access_to_byte_buffer_of_tissue_cell.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/endian.hpp>
#include <utility>

namespace cellconnect { namespace {


std::pair<natural_32_bit,natural_32_bit>  decompose_byte_shift_to_unit_index_and_byte_shift_in_the_unit(
        natural_32_bit const  shift_in_bytes,
        natural_32_bit const  size_of_unit_in_bytes
        )
{
    ASSUMPTION(size_of_unit_in_bytes > 0U);
    return std::pair<natural_32_bit,natural_32_bit>(
                    shift_in_bytes / size_of_unit_in_bytes,
                    shift_in_bytes % size_of_unit_in_bytes
                    );
}


natural_8_bit*  compute_pointer_to_byte_in_buffer_of_tissue_cell(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        natural_32_bit const  x_coord_of_tissue_cell,
        natural_32_bit const  y_coord_of_tissue_cell,
        natural_32_bit const  columnar_coord_of_tissue_cell,
        natural_32_bit const  index_of_byte_where_to_start_reading
        )
{
    ASSUMPTION(dynamic_state_ptr.operator bool());

    std::shared_ptr<cellab::static_state_of_neural_tissue const> static_state_ptr =
            dynamic_state_ptr->get_static_state_of_neural_tissue();

    ASSUMPTION(static_state_ptr.operator bool());
    ASSUMPTION(x_coord_of_tissue_cell < static_state_ptr->num_cells_along_x_axis());
    ASSUMPTION(y_coord_of_tissue_cell < static_state_ptr->num_cells_along_y_axis());
    ASSUMPTION(columnar_coord_of_tissue_cell < static_state_ptr->num_cells_along_columnar_axis());

    cellab::kind_of_cell const  kind_of_tissue_cell =
        static_state_ptr->compute_kind_of_cell_from_its_position_along_columnar_axis(
                columnar_coord_of_tissue_cell
                );

    ASSUMPTION(index_of_byte_where_to_start_reading < compute_size_of_byte_buffer_of_tissue_cell(dynamic_state_ptr,kind_of_tissue_cell));

    ASSUMPTION(dynamic_state_ptr->num_bits_per_delimiter_number(kind_of_tissue_cell) % 8U == 0U);
    ASSUMPTION(static_state_ptr->num_bits_per_synapse() % 8U == 0U);
    ASSUMPTION(cellab::num_of_bits_to_store_territorial_state_of_synapse() % 8U == 0U);
    ASSUMPTION(static_state_ptr->num_bits_per_cell() % 8U == 0U);
    ASSUMPTION(static_state_ptr->num_bits_per_signalling() % 8U == 0U);

    natural_32_bit  shift_begin = 0U;
    natural_32_bit  shift_end =
        cellab::num_delimiters() * dynamic_state_ptr->num_bits_per_delimiter_number(kind_of_tissue_cell) / 8U;
    if (index_of_byte_where_to_start_reading < shift_end)
    {
        std::pair<natural_32_bit,natural_32_bit> const  unit_index_and_ptr_shift =
                decompose_byte_shift_to_unit_index_and_byte_shift_in_the_unit(
                        index_of_byte_where_to_start_reading - shift_begin,
                        dynamic_state_ptr->num_bits_per_delimiter_number(kind_of_tissue_cell) / 8U
                        );
        natural_8_bit*  byte_ptr =
                dynamic_state_ptr->find_bits_of_delimiter_between_teritorial_lists(
                        x_coord_of_tissue_cell,
                        y_coord_of_tissue_cell,
                        columnar_coord_of_tissue_cell,
                        unit_index_and_ptr_shift.first
                        ).first_byte_ptr();
        return byte_ptr + unit_index_and_ptr_shift.second;
    }

    natural_32_bit const  num_synapses_in_territory =
        static_state_ptr->num_synapses_in_territory_of_cell_kind(kind_of_tissue_cell);

    shift_begin = shift_end;
    shift_end += num_synapses_in_territory * static_state_ptr->num_bits_per_synapse() / 8U;
    if (index_of_byte_where_to_start_reading < shift_end)
    {
        std::pair<natural_32_bit,natural_32_bit> const  unit_index_and_ptr_shift =
                decompose_byte_shift_to_unit_index_and_byte_shift_in_the_unit(
                        index_of_byte_where_to_start_reading - shift_begin,
                        static_state_ptr->num_bits_per_synapse() / 8U
                        );
        natural_8_bit*  byte_ptr =
                dynamic_state_ptr->find_bits_of_synapse_in_tissue(
                        x_coord_of_tissue_cell,
                        y_coord_of_tissue_cell,
                        columnar_coord_of_tissue_cell,
                        unit_index_and_ptr_shift.first
                        ).first_byte_ptr();
        return byte_ptr + unit_index_and_ptr_shift.second;
    }

    shift_begin = shift_end;
    shift_end += num_synapses_in_territory * cellab::num_of_bits_to_store_territorial_state_of_synapse() / 8U;
    if (index_of_byte_where_to_start_reading < shift_end)
    {
        std::pair<natural_32_bit,natural_32_bit> const  unit_index_and_ptr_shift =
                decompose_byte_shift_to_unit_index_and_byte_shift_in_the_unit(
                        index_of_byte_where_to_start_reading - shift_begin,
                        cellab::num_of_bits_to_store_territorial_state_of_synapse() / 8U
                        );
        natural_8_bit*  byte_ptr =
                dynamic_state_ptr->find_bits_of_territorial_state_of_synapse_in_tissue(
                        x_coord_of_tissue_cell,
                        y_coord_of_tissue_cell,
                        columnar_coord_of_tissue_cell,
                        unit_index_and_ptr_shift.first
                        ).first_byte_ptr();
        return byte_ptr + unit_index_and_ptr_shift.second;
    }

    shift_begin = shift_end;
    shift_end += static_state_ptr->num_bits_per_cell() / 8U;
    if (index_of_byte_where_to_start_reading < shift_end)
    {
        natural_8_bit*  byte_ptr =
                dynamic_state_ptr->find_bits_of_cell_in_tissue(
                        x_coord_of_tissue_cell,
                        y_coord_of_tissue_cell,
                        columnar_coord_of_tissue_cell
                        ).first_byte_ptr();
        return byte_ptr + (index_of_byte_where_to_start_reading - shift_begin);
    }

    shift_begin = shift_end;
    shift_end += static_state_ptr->num_bits_per_signalling() / 8U;
    if (index_of_byte_where_to_start_reading < shift_end)
    {
        natural_8_bit*  byte_ptr =
                dynamic_state_ptr->find_bits_of_signalling(
                        x_coord_of_tissue_cell,
                        y_coord_of_tissue_cell,
                        columnar_coord_of_tissue_cell
                        ).first_byte_ptr();
        return byte_ptr + (index_of_byte_where_to_start_reading - shift_begin);
    }

    UNREACHABLE();
}


}}

namespace cellconnect {


natural_32_bit  compute_size_of_byte_buffer_of_tissue_cell(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue const> const  dynamic_state_ptr,
        cellab::kind_of_cell const  kind_of_tissue_cell
        )
{
    ASSUMPTION(dynamic_state_ptr.operator bool());

    std::shared_ptr<cellab::static_state_of_neural_tissue const> static_state_ptr =
            dynamic_state_ptr->get_static_state_of_neural_tissue();

    ASSUMPTION(static_state_ptr.operator bool());
    ASSUMPTION(kind_of_tissue_cell < static_state_ptr->num_kinds_of_tissue_cells());

    ASSUMPTION(dynamic_state_ptr->num_bits_per_delimiter_number(kind_of_tissue_cell) % 8U == 0U);
    ASSUMPTION(static_state_ptr->num_bits_per_synapse() % 8U == 0U);
    ASSUMPTION(cellab::num_of_bits_to_store_territorial_state_of_synapse() % 8U == 0U);
    ASSUMPTION(static_state_ptr->num_bits_per_cell() % 8U == 0U);
    ASSUMPTION(static_state_ptr->num_bits_per_signalling() % 8U == 0U);

    natural_32_bit const  num_synapses_in_territory =
        static_state_ptr->num_synapses_in_territory_of_cell_kind(kind_of_tissue_cell);

    natural_32_bit const  num_bytes_in_delimiters =
        cellab::num_delimiters() * dynamic_state_ptr->num_bits_per_delimiter_number(kind_of_tissue_cell) / 8U;

    natural_32_bit const  num_bytes_in_synapses =
            num_synapses_in_territory * static_state_ptr->num_bits_per_synapse() / 8U;

    natural_32_bit const  num_bytes_in_states_of_synapses =
            num_synapses_in_territory * cellab::num_of_bits_to_store_territorial_state_of_synapse() / 8U;

    natural_32_bit const  num_bytes_in_cell =
            static_state_ptr->num_bits_per_cell() / 8U;

    natural_32_bit const  num_bytes_in_signalling =
            static_state_ptr->num_bits_per_signalling() / 8U;

    natural_32_bit const  total_num_of_bytes =
            num_bytes_in_delimiters + num_bytes_in_synapses + num_bytes_in_states_of_synapses +
            num_bytes_in_cell + num_bytes_in_signalling;

    return total_num_of_bytes;
}


natural_32_bit  read_from_byte_buffer_of_tissue_cell(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue const> const  dynamic_state_ptr,
        natural_32_bit const  x_coord_of_tissue_cell,
        natural_32_bit const  y_coord_of_tissue_cell,
        natural_32_bit const  columnar_coord_of_tissue_cell,
        natural_32_bit const  index_of_byte_where_to_start_reading,
        natural_8_bit const  num_bytes_to_read
        )
{
    ASSUMPTION(num_bytes_to_read <= sizeof(natural_32_bit));

    natural_32_bit  value = 0U;
    natural_8_bit* value_byte_ptr = reinterpret_cast<natural_8_bit*>(&value);
    if (!is_this_little_endian_machine())
        value_byte_ptr += sizeof(natural_32_bit) - num_bytes_to_read;
    for (natural_8_bit  i = 0U; i < num_bytes_to_read; ++i)
        *(value_byte_ptr + i) =
                *compute_pointer_to_byte_in_buffer_of_tissue_cell(
                        std::const_pointer_cast<cellab::dynamic_state_of_neural_tissue>(dynamic_state_ptr),
                        x_coord_of_tissue_cell,
                        y_coord_of_tissue_cell,
                        columnar_coord_of_tissue_cell,
                        index_of_byte_where_to_start_reading + i
                        );
    return value;
}


void  write_to_byte_buffer_of_tissue_cell(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        natural_32_bit const  x_coord_of_tissue_cell,
        natural_32_bit const  y_coord_of_tissue_cell,
        natural_32_bit const  columnar_coord_of_tissue_cell,
        natural_32_bit const  index_of_byte_where_to_start_writing,
        natural_8_bit const  num_bytes_to_write,
        natural_32_bit const  value_to_be_written
        )
{
    ASSUMPTION(num_bytes_to_write <= sizeof(value_to_be_written));

    natural_8_bit const* value_byte_ptr = reinterpret_cast<natural_8_bit const*>(&value_to_be_written);
    if (!is_this_little_endian_machine())
        value_byte_ptr += sizeof(value_to_be_written) - num_bytes_to_write;
    for (natural_8_bit  i = 0U; i < num_bytes_to_write; ++i)
        *compute_pointer_to_byte_in_buffer_of_tissue_cell(
                dynamic_state_ptr,
                x_coord_of_tissue_cell,
                y_coord_of_tissue_cell,
                columnar_coord_of_tissue_cell,
                index_of_byte_where_to_start_writing + i
                ) = *(value_byte_ptr + i);
}


}
