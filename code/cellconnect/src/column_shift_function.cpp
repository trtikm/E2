#include <cellconnect/column_shift_function.hpp>
#include <utility/invariants.hpp>
#include <utility/assumptions.hpp>
#include <utility/checked_number_operations.hpp>
#include <utility/random.hpp>
#include <utility/timeprof.hpp>
#include <algorithm>
#include <set>

#include <utility/development.hpp>

namespace cellconnect {


static natural_16_bit const  ES_NUM_KINDS = 8U;
static natural_16_bit const  ES_MASK = 0x8000U;
static natural_16_bit const  ES_KIND_MASK = 7U;
static natural_16_bit const  ES_KIND_SHIFT = 12U;
static natural_16_bit const  ES_MAX_ID = 0xfffU;


exit_shift_kind  get_opposite_exit_shift_kind(exit_shift_kind const  kind)
{
    switch (kind)
    {
    case DIR_LEFT_UP: return DIR_RIGHT_DOWN;
    case DIR_LEFT: return DIR_RIGHT;
    case DIR_LEFT_DOWN: return DIR_RIGHT_UP;
    case DIR_DOWN: return DIR_UP;
    case DIR_RIGHT_DOWN: return DIR_LEFT_UP;
    case DIR_RIGHT: return DIR_LEFT;
    case DIR_RIGHT_UP: return DIR_LEFT_DOWN;
    case DIR_UP: return DIR_DOWN;
    default: UNREACHABLE(); return DIR_RIGHT_UP;
    }
}


shift_to_target::shift_to_target(natural_16_bit const target_row, natural_16_bit  target_column)
    : m_is_external(false)
    , m_target_row(target_row)
    , m_target_column(target_column)
    , m_exit_shift_kind((exit_shift_kind)0U)
    , m_external_shift_ID(0U)
{}

shift_to_target::shift_to_target(exit_shift_kind const  shift_kind, natural_16_bit const  shift_ID)
    : m_is_external(true)
    , m_target_row(0U)
    , m_target_column(0U)
    , m_exit_shift_kind(shift_kind)
    , m_external_shift_ID(shift_ID)
{
    ASSUMPTION((natural_16_bit)m_exit_shift_kind <= ES_NUM_KINDS);
    ASSUMPTION(m_external_shift_ID <= ES_MAX_ID);
}

natural_16_bit  shift_to_target::get_target_row() const
{
    ASSUMPTION(!is_external());
    return m_target_row;
}

natural_16_bit  shift_to_target::get_target_column() const
{
    ASSUMPTION(!is_external());
    return m_target_column;
}

exit_shift_kind  shift_to_target::get_exit_shift_kind() const
{
    ASSUMPTION(is_external());
    return m_exit_shift_kind;
}

natural_16_bit  shift_to_target::get_exit_shift_ID() const
{
    ASSUMPTION(is_external());
    return m_external_shift_ID;
}


shift_template::shift_template(natural_16_bit const num_rows, natural_16_bit const num_columns,
                               std::vector< std::pair<exit_shift_kind,natural_16_bit> > const& exit_counts)
    : m_num_rows(num_rows)
    , m_num_columns(num_columns)
    , m_data()
    , m_from_entry_specification_to_entry_coords()
{
    TMPROF_BLOCK();

    std::vector<shift_to_target>  shifts;
    {
        natural_16_bit ID = 0U;
        for (auto const& pair : exit_counts)
            for (natural_16_bit  i = 0U; i < pair.second; ++i, ++ID)
                shifts.push_back({pair.first,ID});
    }
    setup_data(shifts);
}

shift_template::shift_template(
        natural_16_bit const num_rows,
        natural_16_bit const num_columns,
        std::vector<shift_to_target> const& shifts,
        std::map< std::pair<exit_shift_kind,natural_16_bit>,std::pair<natural_16_bit,natural_16_bit>  > const&
            from_entry_specification_to_entry_coords
        )
    : m_num_rows(num_rows)
    , m_num_columns(num_columns)
    , m_data()
    , m_from_entry_specification_to_entry_coords(from_entry_specification_to_entry_coords)
{
    TMPROF_BLOCK();

    setup_data(shifts);
}

void shift_template::setup_data(std::vector<shift_to_target> const& shifts)
{
    TMPROF_BLOCK();

    natural_16_bit const  length = checked_mul_16_bit(num_rows(),num_columns());
    ASSUMPTION((natural_32_bit)length > 1U && length < ES_MASK);
    ASSUMPTION(shifts.size() <= (natural_32_bit)length)
    typedef std::pair<exit_shift_kind,natural_16_bit>  kind_and_count;
    ASSUMPTION(
        [](std::vector<shift_to_target> const& shifts) {
            std::set<kind_and_count> exits;
            for (shift_to_target const& shift : shifts)
                if (shift.is_external())
                {
                    kind_and_count const  pair(shift.get_exit_shift_kind(),shift.get_exit_shift_ID());
                    if (exits.count(pair) > 0U)
                        return false;
                    exits.insert(pair);
                }
            return true;
        }(shifts)
        );

    m_data.resize(length);
    if (shifts.size() == m_data.size())
    {
        natural_16_bit  index = 0U;
        for (natural_16_bit  r = 0U; r < num_rows(); ++r)
            for (natural_16_bit  c = 0U; c < num_columns(); ++c)
            {
                INVARIANT(index < length);
                set_shift(r,c,shifts.at(index));
                ++index;
            }
        INVARIANT(index == length);
        ASSUMPTION(check_consistency());
    }
    else
    {
        for (natural_16_bit  i = 0U; i < length; ++i)
            m_data.at(i) = i;
        for (natural_32_bit k = 0U; k < 2U; ++k)
            for (natural_32_bit p = 0U; p < (natural_32_bit)length; ++p)
            {
                natural_32_bit const  q = get_random_natural_32_bit_in_range(0U,length - 1U);
                std::swap(m_data.at(p),m_data.at(q));
            }

        ASSUMPTION(
            [](std::vector<shift_to_target> const& shifts) {
                for (shift_to_target const& shift : shifts)
                    if (!shift.is_external())
                        false;
                return true;
            }(shifts)
            );

        std::vector<shift_to_target>  selection;
        {
            for (natural_16_bit  r = 0U; r < num_rows(); ++r)
                for (natural_16_bit  c = 0U; c < num_columns(); ++c)
                    selection.push_back({r,c});
            INVARIANT(selection.size() == (natural_32_bit)length)
            for (natural_32_bit k = 0U; k < 2U; ++k)
                for (natural_32_bit p = 0U; p < (natural_32_bit)length; ++p)
                {
                    natural_32_bit const  q = get_random_natural_32_bit_in_range(0U,length - 1U);
                    std::swap(selection.at(p),selection.at(q));
                }
        }

        natural_16_bit  index = 0U;
        for (shift_to_target const& shift : shifts)
        {
            shift_to_target const&  selected = selection.at(index);
            INVARIANT(!selected.is_external());
            shift_to_target const  targeted = get_shift(selected.get_target_row(),selected.get_target_column());
            set_shift(selected.get_target_row(),selected.get_target_column(),shift);
            m_from_entry_specification_to_entry_coords.insert(
                    {
                        {get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),shift.get_exit_shift_ID()},
                        {targeted.get_target_row(),targeted.get_target_column()}
                    });
            ++index;
        }
        INVARIANT(check_consistency());
    }
}

bool  shift_template::check_consistency() const
{
    TMPROF_BLOCK();

    std::set< std::pair<exit_shift_kind,natural_16_bit> > exits;
    std::set< std::pair<natural_16_bit,natural_16_bit> > targeted;
    for (natural_16_bit  r = 0U; r < num_rows(); ++r)
        for (natural_16_bit  c = 0U; c < num_columns(); ++c)
        {
            shift_to_target const  shift = get_shift(r,c);
            if (shift.is_external())
            {
                std::pair<exit_shift_kind,natural_16_bit> const  pair{shift.get_exit_shift_kind(),shift.get_exit_shift_ID()};
                if (exits.count(pair) > 0U)
                    return false;
                exits.insert(pair);
            }
            else
            {
                std::pair<natural_16_bit,natural_16_bit> const  record{ shift.get_target_row(), shift.get_target_column() };
                if (targeted.count(record) > 0U)
                    return false;
                targeted.insert(record);
            }
        }
    if (exits.size() != m_from_entry_specification_to_entry_coords.size())
        return false;
    std::set< std::pair<natural_16_bit,natural_16_bit> > entries;
    for (auto const& value : m_from_entry_specification_to_entry_coords)
    {
        if (entries.count(value.second) > 0U)
            return false;
        entries.insert(value.second);
    }
    for (auto const& value : entries)
        if (targeted.count(value) > 0U)
            return false;
    for (auto const& value : targeted)
        if (entries.count(value) > 0U)
            return false;
    return m_data.size() - targeted.size() == exits.size();
}


shift_to_target  shift_template::get_shift(natural_16_bit const  row_index, natural_16_bit const  column_index) const
{
    ASSUMPTION(row_index < num_rows() && column_index < num_columns());
    natural_16_bit const  index = get_index(row_index,column_index);
    natural_16_bit const  raw_shift_value = m_data.at(index);
    if (((natural_32_bit)raw_shift_value & ES_MASK) == 0U)
        return get_coords(raw_shift_value);
    return { (exit_shift_kind)((raw_shift_value & ES_KIND_MASK) >> ES_KIND_SHIFT), (natural_16_bit)(raw_shift_value & ES_MAX_ID) };
}

void  shift_template::set_shift(natural_16_bit const  row_index, natural_16_bit const  column_index, shift_to_target const& shift)
{
    ASSUMPTION(row_index < num_rows() && column_index < num_columns());
    natural_16_bit const  index = get_index(row_index,column_index);
    if (!shift.is_external())
        m_data.at(index) = get_index(shift.get_target_row(),shift.get_target_column());
    else
        m_data.at(index) = ES_MASK | ((natural_16_bit)shift.get_exit_shift_kind() << ES_KIND_SHIFT) | shift.get_exit_shift_ID();
}

std::pair<natural_16_bit,natural_16_bit> const&  shift_template::get_entry_shift(exit_shift_kind const direction_from_adjacent_matrix,
                                                                                 natural_16_bit const  shift_ID) const
{
    auto const  iter = m_from_entry_specification_to_entry_coords.find({direction_from_adjacent_matrix,shift_ID});
    ASSUMPTION(iter != m_from_entry_specification_to_entry_coords.end());
    return iter->second;
}


layout_of_shift_templates::layout_of_shift_templates(natural_16_bit const  num_rows, natural_16_bit const  num_columns,
                                                     std::vector<natural_16_bit> const& matrix_of_indices_of_shift_templates)
    : m_num_rows(num_rows)
    , m_num_columns(num_columns)
    , m_layout(matrix_of_indices_of_shift_templates)
    , m_num_templates(*std::max_element(matrix_of_indices_of_shift_templates.begin(),matrix_of_indices_of_shift_templates.end()) + 1U)
{
    ASSUMPTION(m_layout.size() > 0U && m_layout.size() == (natural_32_bit)checked_mul_16_bit(m_num_rows,m_num_columns))
    ASSUMPTION(
        [](std::vector<natural_16_bit> const& indices_vector, natural_16_bit const  num_templates) {
            std::set<natural_16_bit> const  indices(indices_vector.begin(),indices_vector.end());
            if (indices.size() != (natural_32_bit)num_templates)
                return false;
            for (natural_16_bit  index = 0U; index < num_templates; ++index)
                if (indices.count(index) == 0U)
                    return false;
            return true;
        }(m_layout,m_num_templates)
        );
}

natural_16_bit layout_of_shift_templates::get_template_index(const natural_16_bit row_index, const natural_16_bit column_index) const
{
    ASSUMPTION(row_index < num_rows() && column_index < num_columns());
    natural_16_bit const  index = get_index(row_index,column_index);
    return m_layout.at(index);
}


repetition_block::repetition_block(natural_16_bit const  block_begin_index, natural_16_bit const  block_end_index,
                                   natural_16_bit const  block_num_repetitions)
    : m_block_begin_index(block_begin_index)
    , m_block_end_index(block_end_index)
    , m_block_num_repetitions(block_num_repetitions)
{
    ASSUMPTION(m_block_begin_index < m_block_end_index);
    ASSUMPTION(m_block_num_repetitions > 0U);
}


column_shift_function::column_shift_function()
    : m_use_identity_function(true)
    , m_num_cells_along_x_axis(0U)
    , m_num_cells_along_y_axis(0U)
    , m_shift_templates()
    , m_scale_of_shift_templates_along_row_axis(0U)
    , m_scale_of_shift_templates_along_column_axis(0U)
    , m_layout_of_shift_templates({1U,1U,{0U}})
    , m_repetition_blocks_of_rows_in_layout_of_shift_templates()
    , m_repetition_blocks_of_columns_in_layout_of_shift_templates()
{}

column_shift_function::column_shift_function(
        natural_32_bit const  num_cells_along_x_axis,
        natural_32_bit const  num_cells_along_y_axis,
        std::vector<shift_template> const&  shift_templates,
        natural_32_bit const  scale_of_shift_templates_along_row_axis,
        natural_32_bit const  scale_of_shift_templates_along_column_axis,
        layout_of_shift_templates const&  layout_of_shift_templates,
        std::vector<repetition_block> const&  repetition_blocks_of_rows_in_layout_of_shift_templates,
        std::vector<repetition_block> const&  repetition_blocks_of_columns_in_layout_of_shift_templates
        )
    : m_use_identity_function(false)
    , m_num_cells_along_x_axis(num_cells_along_x_axis)
    , m_num_cells_along_y_axis(num_cells_along_y_axis)
    , m_shift_templates(shift_templates)
    , m_scale_of_shift_templates_along_row_axis(scale_of_shift_templates_along_row_axis)
    , m_scale_of_shift_templates_along_column_axis(scale_of_shift_templates_along_column_axis)
    , m_layout_of_shift_templates(layout_of_shift_templates)
    , m_repetition_blocks_of_rows_in_layout_of_shift_templates()
    , m_repetition_blocks_of_columns_in_layout_of_shift_templates()
{
    TMPROF_BLOCK();

    ASSUMPTION(m_num_cells_along_x_axis > 0U);
    ASSUMPTION(m_num_cells_along_y_axis > 0U);
    ASSUMPTION(m_shift_templates.size() == (natural_32_bit)m_layout_of_shift_templates.num_templates());
    ASSUMPTION(m_scale_of_shift_templates_along_row_axis > 0U);
    ASSUMPTION(m_scale_of_shift_templates_along_column_axis > 0U);
    ASSUMPTION(check_layout_consistency());

    build_repetitions(repetition_blocks_of_rows_in_layout_of_shift_templates,m_layout_of_shift_templates.num_rows(),
                      m_repetition_blocks_of_rows_in_layout_of_shift_templates);
    build_repetitions(repetition_blocks_of_columns_in_layout_of_shift_templates,m_layout_of_shift_templates.num_columns(),
                      m_repetition_blocks_of_columns_in_layout_of_shift_templates);

    ASSUMPTION(check_consistency_between_layout_and_tissue());
}

void  column_shift_function::build_repetitions(std::vector<repetition_block> const&  reps, natural_16_bit const  size,
                                               std::vector<repetition_block>&  output)
{
    ASSUMPTION(reps.size() <= (natural_32_bit)size);
    ASSUMPTION(
            [](std::vector<repetition_block> const&  reps, natural_16_bit size) {
                for (natural_32_bit  i = 1U; i < reps.size(); ++i)
                    if (reps.at(i - 1U).block_end_index() > reps.at(i).block_begin_index())
                        return false;
                if (!reps.empty() && reps.back().block_end_index() > size)
                    return false;
                return true;
            }(reps,size)
            );
    ASSUMPTION(output.empty());

    natural_16_bit  index = 0U;
    natural_16_bit  reps_index = 0U;
    while (index < size)
        if ((natural_32_bit)reps_index < reps.size() && index == reps.at(reps_index).block_begin_index())
        {
            output.push_back(reps.at(reps_index));
            index = reps.at(reps_index).block_end_index();
            ++reps_index;
        }
        else
        {
            output.push_back({index,index + 1U,1U});
            ++index;
        }
    INVARIANT(index == size);
    INVARIANT((natural_32_bit)reps_index == reps.size());
    INVARIANT(!output.empty());
    INVARIANT(
            [](std::vector<repetition_block> const&  reps, natural_16_bit size) {
                if (reps.at(0U).block_begin_index() != 0U)
                    return false;
                for (natural_32_bit  i = 1U; i < reps.size(); ++i)
                    if (reps.at(i - 1U).block_end_index() != reps.at(i).block_begin_index())
                        return false;
                if (reps.back().block_end_index() != size)
                    return false;
                return true;
            }(reps,size)
            );
}


bool  column_shift_function::check_layout_consistency() const
{
    for (natural_16_bit r = 0U; r < m_layout_of_shift_templates.num_rows(); ++r)
    {
        natural_16_bit const  dim = get_shift_template(r,0U).num_rows();
        for (natural_16_bit c = 1U; c < m_layout_of_shift_templates.num_columns(); ++c)
            if (get_shift_template(r,c).num_rows() != dim)
                return false;
    }
    for (natural_16_bit c = 0U; c < m_layout_of_shift_templates.num_columns(); ++c)
    {
        natural_16_bit const  dim = get_shift_template(0U,c).num_columns();
        for (natural_16_bit r = 1U; r < m_layout_of_shift_templates.num_rows(); ++r)
            if (get_shift_template(r,c).num_columns() != dim)
                return false;
    }
    for (natural_16_bit r = 0U; r < m_layout_of_shift_templates.num_rows(); ++r)
        for (natural_16_bit c = 0U; c < m_layout_of_shift_templates.num_columns(); ++c)
        {
            shift_template const&  tpt = get_shift_template(r,c);
            for (natural_16_bit  tr = 0U; tr < tpt.num_rows(); ++tr)
                for (natural_16_bit  tc = 0U; tc <tpt. num_columns(); ++tc)
                {
                    shift_to_target const  shift = tpt.get_shift(tr,tc);
                    if (shift.is_external())
                        switch (shift.get_exit_shift_kind())
                        {
                        case DIR_LEFT_UP:
                            if (r == 0U)
                                return false;
                            if (c == 0U)
                                return false;
                            get_shift_template(r - 1U, c - 1U).get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                                               shift.get_exit_shift_ID());
                            break;
                        case DIR_LEFT:
                            if (c == 0U)
                                return false;
                            get_shift_template(r + 0U, c - 1U).get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                                               shift.get_exit_shift_ID());
                            break;
                        case DIR_LEFT_DOWN:
                            if (r == m_layout_of_shift_templates.num_rows() - 1U)
                                return false;
                            if (c == 0U)
                                return false;
                            get_shift_template(r + 1U, c - 1U).get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                                               shift.get_exit_shift_ID());
                            break;
                        case DIR_DOWN:
                            if (r == m_layout_of_shift_templates.num_rows() - 1U)
                                return false;
                            get_shift_template(r + 1U, c + 0U).get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                                               shift.get_exit_shift_ID());
                            break;
                        case DIR_RIGHT_DOWN:
                            if (r == m_layout_of_shift_templates.num_rows() - 1U)
                                return false;
                            if (c == m_layout_of_shift_templates.num_columns() - 1U)
                                return false;
                            get_shift_template(r + 1U, c + 1U).get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                                               shift.get_exit_shift_ID());
                            break;
                        case DIR_RIGHT:
                            if (c == m_layout_of_shift_templates.num_columns() - 1U)
                                return false;
                            get_shift_template(r + 0U, c + 1U).get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                                               shift.get_exit_shift_ID());
                            break;
                        case DIR_RIGHT_UP:
                            if (r == 0U)
                                return false;
                            if (c == m_layout_of_shift_templates.num_columns() - 1U)
                                return false;
                            get_shift_template(r - 1U, c + 1U).get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                                               shift.get_exit_shift_ID());
                            break;
                        case DIR_UP:
                            if (r == 0U)
                                return false;
                            get_shift_template(r - 1U, c + 0U).get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                                               shift.get_exit_shift_ID());
                            break;
                        default:
                            return false;
                        }
                }
        }

    return true;
}

bool  column_shift_function::check_consistency_between_layout_and_tissue() const
{
    natural_32_bit  num_cells_x = 0U;
    for (natural_16_bit  block_index = 0U; block_index < num_row_repetition_blocks(); ++block_index)
        num_cells_x += num_repetition_block_cells_along_x_axis(block_index);
    if (num_cells_x != num_cells_along_x_axis())
        return false;

    natural_32_bit  num_cells_y = 0U;
    for (natural_16_bit  block_index = 0U; block_index < num_column_repetition_blocks(); ++block_index)
        num_cells_y += num_repetition_block_cells_along_y_axis(block_index);
    if (num_cells_y != num_cells_along_y_axis())
        return false;

    return true;
}


std::pair<natural_32_bit,natural_32_bit>  column_shift_function::operator()(natural_32_bit const  x_coord,
                                                                            natural_32_bit const  y_coord) const
{
    if (is_identity_function())
        return std::make_pair(x_coord,y_coord);

    ASSUMPTION(x_coord < num_cells_along_x_axis());
    ASSUMPTION(y_coord < num_cells_along_y_axis());

    natural_16_bit  layout_row_index, template_row_index/*, template_row_version*/;
    {
        natural_32_bit  current_x_coord = 0U;
        natural_16_bit  current_row_block_index = 0U;
        while ((x_coord - current_x_coord) / num_repetition_block_cells_along_x_axis(current_row_block_index) >=
               (natural_32_bit)get_row_repetition_block(current_row_block_index).block_num_repetitions())
        {
            current_x_coord += (natural_32_bit)get_row_repetition_block(current_row_block_index).block_num_repetitions() *
                               num_repetition_block_cells_along_x_axis(current_row_block_index);
            INVARIANT(current_x_coord <= x_coord);
            ++current_row_block_index;
        }
        repetition_block const&  row_block = get_row_repetition_block(current_row_block_index);
        layout_row_index = row_block.block_begin_index();
        while (x_coord - current_x_coord >= num_shift_template_cells_along_x_axis(layout_row_index))
        {
            current_x_coord += num_shift_template_cells_along_x_axis(layout_row_index);
            INVARIANT(current_x_coord <= x_coord);
            ++layout_row_index;
            INVARIANT(layout_row_index < row_block.block_end_index());
        }
        template_row_index = (x_coord - current_x_coord) / scale_along_row_axis();
        //template_row_version = (x_coord - current_x_coord) % scale_along_row_axis();
    }

    natural_16_bit  layout_column_index, template_column_index/*, template_column_version*/;
    {
        natural_32_bit  current_y_coord = 0U;
        natural_16_bit  current_column_block_index = 0U;
        while ((y_coord - current_y_coord) / num_repetition_block_cells_along_y_axis(current_column_block_index) >=
               (natural_32_bit)get_column_repetition_block(current_column_block_index).block_num_repetitions())
        {
            current_y_coord += (natural_32_bit)get_column_repetition_block(current_column_block_index).block_num_repetitions() *
                               num_repetition_block_cells_along_y_axis(current_column_block_index);
            INVARIANT(current_y_coord <= y_coord);
            ++current_column_block_index;
        }
        repetition_block const&  column_block = get_column_repetition_block(current_column_block_index);
        layout_column_index = column_block.block_begin_index();
        while (y_coord - current_y_coord >= num_shift_template_cells_along_y_axis(layout_column_index))
        {
            current_y_coord += num_shift_template_cells_along_y_axis(layout_column_index);
            INVARIANT(current_y_coord <= y_coord);
            ++layout_column_index;
            INVARIANT(layout_column_index < column_block.block_end_index());
        }
        template_column_index = (y_coord - current_y_coord) / scale_along_column_axis();
        //template_column_version = (y_coord - current_y_coord) % scale_along_column_axis();
    }

    integer_64_bit  target_x, target_y;
    {
        integer_64_bit  row_shift, column_shift;
        {
            shift_template const&  source_template = get_shift_template(layout_row_index,layout_column_index);
            shift_to_target const  shift = source_template.get_shift(template_row_index,template_column_index);
            if (shift.is_external())
            {
                switch (shift.get_exit_shift_kind())
                {
                case DIR_LEFT_UP:
                {
                    shift_template const&  entry_template = get_shift_template(layout_row_index - 1U,layout_column_index - 1U);
                    std::pair<natural_16_bit,natural_16_bit> const&  entry_coords =
                            entry_template.get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                           shift.get_exit_shift_ID());
                    row_shift = (integer_64_bit)entry_coords.first - (integer_64_bit)template_row_index
                                    - (integer_64_bit)entry_template.num_rows();
                    column_shift = (integer_64_bit)entry_coords.second - (integer_64_bit)template_column_index
                                        - (integer_64_bit)entry_template.num_columns();
                    break;
                }
                case DIR_LEFT:
                {
                    shift_template const&  entry_template = get_shift_template(layout_row_index + 0U,layout_column_index - 1U);
                    std::pair<natural_16_bit,natural_16_bit> const&  entry_coords =
                            entry_template.get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                           shift.get_exit_shift_ID());
                    row_shift = (integer_64_bit)entry_coords.first - (integer_64_bit)template_row_index;
                    column_shift = (integer_64_bit)entry_coords.second - (integer_64_bit)template_column_index
                                        - (integer_64_bit)entry_template.num_columns();
                    break;
                }
                case DIR_LEFT_DOWN:
                {
                    shift_template const&  entry_template = get_shift_template(layout_row_index + 1U,layout_column_index - 1U);
                    std::pair<natural_16_bit,natural_16_bit> const&  entry_coords =
                            entry_template.get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                           shift.get_exit_shift_ID());
                    row_shift = (integer_64_bit)entry_coords.first - (integer_64_bit)template_row_index
                                    + (integer_64_bit)source_template.num_rows();
                    column_shift = (integer_64_bit)entry_coords.second - (integer_64_bit)template_column_index
                                        - (integer_64_bit)entry_template.num_columns();
                    break;
                }
                case DIR_DOWN:
                {
                    shift_template const&  entry_template = get_shift_template(layout_row_index + 1U,layout_column_index + 0U);
                    std::pair<natural_16_bit,natural_16_bit> const&  entry_coords =
                            entry_template.get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                           shift.get_exit_shift_ID());
                    row_shift = (integer_64_bit)entry_coords.first - (integer_64_bit)template_row_index
                                    + (integer_64_bit)source_template.num_rows();
                    column_shift = (integer_64_bit)entry_coords.second - (integer_64_bit)template_column_index;
                    break;
                }
                case DIR_RIGHT_DOWN:
                {
                    shift_template const&  entry_template = get_shift_template(layout_row_index + 1U,layout_column_index + 1U);
                    std::pair<natural_16_bit,natural_16_bit> const&  entry_coords =
                            entry_template.get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                           shift.get_exit_shift_ID());
                    row_shift = (integer_64_bit)entry_coords.first - (integer_64_bit)template_row_index
                                    + (integer_64_bit)source_template.num_rows();
                    column_shift = (integer_64_bit)entry_coords.second - (integer_64_bit)template_column_index
                                        + (integer_64_bit)source_template.num_columns();
                    break;
                }
                case DIR_RIGHT:
                {
                    shift_template const&  entry_template = get_shift_template(layout_row_index + 0U,layout_column_index + 1U);
                    std::pair<natural_16_bit,natural_16_bit> const&  entry_coords =
                            entry_template.get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                           shift.get_exit_shift_ID());
                    row_shift = (integer_64_bit)entry_coords.first - (integer_64_bit)template_row_index;
                    column_shift = (integer_64_bit)entry_coords.second - (integer_64_bit)template_column_index
                                        + (integer_64_bit)source_template.num_columns();
                    break;
                }
                case DIR_RIGHT_UP:
                {
                    shift_template const&  entry_template = get_shift_template(layout_row_index - 1U,layout_column_index + 1U);
                    std::pair<natural_16_bit,natural_16_bit> const&  entry_coords =
                            entry_template.get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                           shift.get_exit_shift_ID());
                    row_shift = (integer_64_bit)entry_coords.first - (integer_64_bit)template_row_index
                                    - (integer_64_bit)entry_template.num_rows();
                    column_shift = (integer_64_bit)entry_coords.second - (integer_64_bit)template_column_index
                                        + (integer_64_bit)source_template.num_columns();
                    break;
                }
                case DIR_UP:
                {
                    shift_template const&  entry_template = get_shift_template(layout_row_index - 1U,layout_column_index + 0U);
                    std::pair<natural_16_bit,natural_16_bit> const&  entry_coords =
                            entry_template.get_entry_shift(get_opposite_exit_shift_kind(shift.get_exit_shift_kind()),
                                                           shift.get_exit_shift_ID());
                    row_shift = (integer_64_bit)entry_coords.first - (integer_64_bit)template_row_index
                                    - (integer_64_bit)entry_template.num_rows();
                    column_shift = (integer_64_bit)entry_coords.second - (integer_64_bit)template_column_index;
                    break;
                }
                default: UNREACHABLE(); break;
                }
            }
            else
            {
                row_shift = (integer_64_bit)shift.get_target_row() - (integer_64_bit)template_row_index;
                column_shift = (integer_64_bit)shift.get_target_column() - (integer_64_bit)template_column_index;
            }
        }

        target_x = (integer_64_bit)x_coord + (integer_64_bit)scale_along_row_axis() * row_shift;
        INVARIANT(target_x >= 0LL && target_x < (integer_64_bit)num_cells_along_x_axis());

        target_y = (integer_64_bit)y_coord + (integer_64_bit)scale_along_column_axis() * column_shift;
        INVARIANT(target_y >= 0LL && target_y < (integer_64_bit)num_cells_along_y_axis());
    }

    return { (natural_32_bit)target_x, (natural_32_bit)target_y };
}

shift_template const&  column_shift_function::get_shift_template(natural_16_bit const  layout_row_index,
                                                                 natural_16_bit const  layout_column_index) const
{
    ASSUMPTION(layout_row_index < num_layout_rows());
    ASSUMPTION(layout_column_index < num_layout_columns());
    natural_16_bit const  index = m_layout_of_shift_templates.get_template_index(layout_row_index,layout_column_index);
    return m_shift_templates.at(index);
}

repetition_block const& column_shift_function::get_row_repetition_block(natural_16_bit const  row_block_index) const
{
    ASSUMPTION(row_block_index < m_repetition_blocks_of_rows_in_layout_of_shift_templates.size());
    return m_repetition_blocks_of_rows_in_layout_of_shift_templates.at(row_block_index);
}

repetition_block const& column_shift_function::get_column_repetition_block(natural_16_bit const  column_block_index) const
{
    ASSUMPTION(column_block_index < m_repetition_blocks_of_columns_in_layout_of_shift_templates.size());
    return m_repetition_blocks_of_columns_in_layout_of_shift_templates.at(column_block_index);

}

natural_32_bit  column_shift_function::num_repetition_block_cells_along_x_axis(natural_16_bit const  row_block_index) const
{
    repetition_block const&  block = get_row_repetition_block(row_block_index);
    natural_16_bit  summary_dimension = 0U;
    for (natural_16_bit row = block.block_begin_index(); row < block.block_end_index(); ++row)
        summary_dimension += get_shift_template(row,0U).num_rows();
    return scale_along_row_axis() * (natural_32_bit)summary_dimension;
}

natural_32_bit  column_shift_function::num_repetition_block_cells_along_y_axis(natural_16_bit const  column_block_index) const
{
    repetition_block const&  block = get_column_repetition_block(column_block_index);
    natural_16_bit  summary_dimension = 0U;
    for (natural_16_bit column = block.block_begin_index(); column < block.block_end_index(); ++column)
        summary_dimension += get_shift_template(0U,column).num_columns();
    return scale_along_column_axis() * (natural_32_bit)summary_dimension;
}

natural_32_bit  column_shift_function::num_shift_template_cells_along_x_axis(natural_16_bit const  layout_row_index) const
{
    return scale_along_row_axis() * get_shift_template(layout_row_index,0U).num_rows();
}

natural_32_bit  column_shift_function::num_shift_template_cells_along_y_axis(natural_16_bit const  layout_column_index) const
{
    return scale_along_column_axis() * get_shift_template(0U,layout_column_index).num_columns();
}


void  compute_tissue_axis_length_and_template_scale(
        natural_32_bit const  desired_number_of_cells_along_one_axis_of_tissue,
        natural_16_bit const  largest_template_dimension,
        natural_16_bit const  num_repetitions_of_largest_template_dimension,
        natural_32_bit&  num_tissue_cells_along_the_axis,
        natural_32_bit&  scale_of_all_template_dimensions
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(desired_number_of_cells_along_one_axis_of_tissue > 0U);
    ASSUMPTION(largest_template_dimension > 0U);
    ASSUMPTION(num_repetitions_of_largest_template_dimension > 0U);

    natural_32_bit const  coef = (natural_32_bit)num_repetitions_of_largest_template_dimension *
                                 (natural_32_bit)largest_template_dimension;
    scale_of_all_template_dimensions = desired_number_of_cells_along_one_axis_of_tissue / coef;
    if (scale_of_all_template_dimensions == 0U || coef < 2U * (desired_number_of_cells_along_one_axis_of_tissue % coef))
        ++scale_of_all_template_dimensions;
    INVARIANT(scale_of_all_template_dimensions > 0U);
    num_tissue_cells_along_the_axis = coef * scale_of_all_template_dimensions;
}


}
