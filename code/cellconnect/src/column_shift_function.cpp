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

natural_16_bit  shift_to_target::get_external_shift_ID() const
{
    ASSUMPTION(is_external());
    return m_external_shift_ID;
}


shift_template::shift_template(natural_16_bit const num_rows, natural_16_bit const num_columns,
                               std::vector< std::pair<exit_shift_kind,natural_16_bit> > const& exit_counts)
    : m_num_rows(num_rows)
    , m_num_columns(num_columns)
    , m_data()
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
        std::vector<shift_to_target> const& shifts,
        natural_16_bit const num_rows,
        natural_16_bit const num_columns
        )
    : m_num_rows(num_rows)
    , m_num_columns(num_columns)
    , m_data()
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
                    kind_and_count const  pair(shift.get_exit_shift_kind(),shift.get_external_shift_ID());
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

        natural_16_bit  index = 0U;
        for (shift_to_target const& shift : shifts)
        {
            shift_to_target const&  selected = selection.at(index);
            set_shift(selected.get_target_row(),selected.get_target_column(),shift);
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
                std::pair<exit_shift_kind,natural_16_bit> const  pair{shift.get_exit_shift_kind(),shift.get_external_shift_ID()};
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
        m_data.at(index) = ES_MASK | ((natural_16_bit)shift.get_exit_shift_kind() << ES_KIND_SHIFT) | shift.get_external_shift_ID();
}


column_shift_function::column_shift_function()
    : m_use_identity_function(true)
    , m_num_cells_along_x_axis(0U)
    , m_num_cells_along_y_axis(0U)
    , m_shift_templates()
    , m_num_rows_in_layouts(0U)
    , m_num_columns_in_layouts(0U)
    , m_layout_of_templates()
    , m_layout_of_scales_along_row_axis()
    , m_layout_of_scales_along_column_axis()
    , m_layout_of_row_repetitions()
    , m_layout_of_column_repetitions()
{}

column_shift_function::column_shift_function(
        natural_32_bit const  num_cells_along_x_axis,
        natural_32_bit const  num_cells_along_y_axis,
        std::vector<shift_template> const&  shift_templates,
        natural_16_bit const  num_rows_in_layouts,
        natural_16_bit const  num_columns_in_layouts,
        std::vector<natural_16_bit> const&  layout_of_templates,
        std::vector<natural_32_bit> const&  layout_of_scales_along_row_axis,
        std::vector<natural_32_bit> const&  layout_of_scales_along_column_axis,
        std::vector< std::pair<natural_16_bit,natural_32_bit> > const&  layout_of_row_repetitions,
        std::vector< std::pair<natural_16_bit,natural_32_bit> > const&  layout_of_column_repetitions
        )
    : m_use_identity_function(false)
    , m_num_cells_along_x_axis(num_cells_along_x_axis)
    , m_num_cells_along_y_axis(num_cells_along_y_axis)
    , m_shift_templates(shift_templates)
    , m_num_rows_in_layouts(num_rows_in_layouts)
    , m_num_columns_in_layouts(num_columns_in_layouts)
    , m_layout_of_templates(layout_of_templates)
    , m_layout_of_scales_along_row_axis(layout_of_scales_along_row_axis)
    , m_layout_of_scales_along_column_axis(layout_of_scales_along_column_axis)
    , m_layout_of_row_repetitions(layout_of_row_repetitions)
    , m_layout_of_column_repetitions(layout_of_column_repetitions)
{
    TMPROF_BLOCK();

    ASSUMPTION(m_num_cells_along_x_axis > 0U);
    ASSUMPTION(m_num_cells_along_y_axis > 0U);
    ASSUMPTION(!shift_templates.empty() && shift_templates.size() < (1U << 15U));

    NOT_IMPLEMENTED_YET();
}


std::pair<natural_32_bit,natural_32_bit>  column_shift_function::operator()(natural_32_bit const  x_coord, natural_32_bit const  y_coord) const
{
    if (m_use_identity_function)
        return std::make_pair(x_coord,y_coord);

    ASSUMPTION(x_coord < m_num_cells_along_x_axis);
    ASSUMPTION(y_coord < m_num_cells_along_y_axis);

    NOT_IMPLEMENTED_YET();
}


void  compute_tissue_axis_length_and_template_scale(
        natural_32_bit const  desired_number_of_cells_along_one_axis_of_tissue,
        natural_16_bit const  dimension_of_template,
        natural_8_bit const  num_repetitions_of_template,
        natural_32_bit&  num_tissue_cells_along_the_axis,
        natural_32_bit&  scale_of_template
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(desired_number_of_cells_along_one_axis_of_tissue > 0U);
    ASSUMPTION(dimension_of_template > 0U);
    ASSUMPTION(num_repetitions_of_template > 0U);

    natural_32_bit const  coef = (natural_32_bit)num_repetitions_of_template * (natural_32_bit)dimension_of_template;
    scale_of_template = desired_number_of_cells_along_one_axis_of_tissue / coef;
    if (scale_of_template == 0U || coef < 2U * (desired_number_of_cells_along_one_axis_of_tissue % coef))
        ++scale_of_template;
    INVARIANT(scale_of_template > 0U);
    num_tissue_cells_along_the_axis = coef * scale_of_template;
}


}
