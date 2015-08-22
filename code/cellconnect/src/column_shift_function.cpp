#include <cellconnect/column_shift_function.hpp>
#include <utility/invariants.hpp>
#include <utility/assumptions.hpp>
#include <utility/checked_number_operations.hpp>
#include <algorithm>

#include <utility/development.hpp>

namespace cellconnect {


static natural_16_bit  mk_shift(natural_16_bit const ID, natural_16_bit const dir)
{
    ASSUMPTION(ID < 0x800U);
    ASSUMPTION(dir < 16U);
    return (1U << 15U) | (dir << 11U) | ID;

}

static natural_16_bit  num_rows(shift_template const&  tpt) { return std::get<0>(tpt); }

static natural_16_bit  num_columns(shift_template const&  tpt) { return std::get<1>(tpt); }

static std::vector<natural_16_bit> const&  get_data(shift_template const&  tpt) { return std::get<2>(tpt); }

static natural_16_bit  get_elem(shift_template const&  tpt, natural_16_bit const  index)
{
    ASSUMPTION(index < get_data(tpt).size());
    return get_data(tpt).at(index);
}


}

namespace cellconnect {


natural_16_bit  goto_left_up(natural_16_bit const ID) { return mk_shift(ID,0U); }
natural_16_bit  goto_left(natural_16_bit const ID) { return mk_shift(ID,1U); }
natural_16_bit  goto_left_down(natural_16_bit const ID) { return mk_shift(ID,2U); }
natural_16_bit  goto_down(natural_16_bit const ID) { return mk_shift(ID,3U); }
natural_16_bit  goto_right_down(natural_16_bit const ID) { return mk_shift(ID,4U); }
natural_16_bit  goto_right(natural_16_bit const ID) { return mk_shift(ID,5U); }
natural_16_bit  goto_right_up(natural_16_bit const ID) { return mk_shift(ID,6U); }
natural_16_bit  goto_up(natural_16_bit const ID) { return mk_shift(ID,7U); }

natural_16_bit  gofrom_left_up(natural_16_bit const ID) { return mk_shift(ID,0U + 8U); }
natural_16_bit  gofrom_left(natural_16_bit const ID) { return mk_shift(ID,1U + 8U); }
natural_16_bit  gofrom_left_down(natural_16_bit const ID) { return mk_shift(ID,2U + 8U); }
natural_16_bit  gofrom_down(natural_16_bit const ID) { return mk_shift(ID,3U + 8U); }
natural_16_bit  gofrom_right_down(natural_16_bit const ID) { return mk_shift(ID,4U + 8U); }
natural_16_bit  gofrom_right(natural_16_bit const ID) { return mk_shift(ID,5U + 8U); }
natural_16_bit  gofrom_right_up(natural_16_bit const ID) { return mk_shift(ID,6U + 8U); }
natural_16_bit  gofrom_up(natural_16_bit const ID) { return mk_shift(ID,7U + 8U); }


bool  compute_dimestions_repetitions_and_scales_for_templates_along_one_axis_of_tissue(
        natural_32_bit const  num_tissue_cells_along_the_axis,
        natural_16_bit const  dimension_of_the_central_template,
        natural_8_bit const  num_repetitions_of_the_central_template,
        natural_32_bit&  scale_of_all_templates,
        natural_16_bit&  dimension_of_the_front_template,
        natural_8_bit&  num_repetitions_of_the_front_template,
        natural_16_bit&  dimension_of_the_tail_template,
        natural_8_bit&  num_repetitions_of_the_tail_template
        )
{
    ASSUMPTION(num_tissue_cells_along_the_axis > 2U);
    ASSUMPTION(dimension_of_the_central_template > 1U);
    ASSUMPTION((natural_32_bit)dimension_of_the_central_template <= num_tissue_cells_along_the_axis - 2U);
    ASSUMPTION(num_repetitions_of_the_central_template > 0U);
    ASSUMPTION((natural_32_bit)num_repetitions_of_the_central_template * (natural_32_bit)dimension_of_the_central_template
               <= num_tissue_cells_along_the_axis - 2U);

    natural_32_bit const  central_coef =
            (natural_32_bit)num_repetitions_of_the_central_template * (natural_32_bit)dimension_of_the_central_template;
    for (natural_32_bit addon = 2U; addon <= 2U * central_coef; ++addon)
        if (num_tissue_cells_along_the_axis % (central_coef + addon) == 0U)
        {
            scale_of_all_templates = num_tissue_cells_along_the_axis / (central_coef + addon);
            for (dimension_of_the_front_template = dimension_of_the_central_template;
                 dimension_of_the_front_template >= 1U;
                 --dimension_of_the_front_template)
                for (num_repetitions_of_the_front_template = 1U;
                     num_repetitions_of_the_front_template <= num_repetitions_of_the_central_template;
                     ++num_repetitions_of_the_front_template)
                {
                    natural_32_bit const  front_coef = (natural_32_bit)num_repetitions_of_the_front_template *
                                                       (natural_32_bit)dimension_of_the_front_template;
                    if (front_coef + (natural_32_bit)dimension_of_the_front_template <= addon)
                        for (dimension_of_the_tail_template = dimension_of_the_central_template;
                             dimension_of_the_tail_template >= dimension_of_the_front_template;
                             --dimension_of_the_tail_template)
                            if ((addon - front_coef) % dimension_of_the_tail_template == 0U)
                            {
                                num_repetitions_of_the_tail_template = (addon - front_coef) / dimension_of_the_tail_template;
                                return true;
                            }
                }
            UNREACHABLE();
        }

    scale_of_all_templates = 0U;
    dimension_of_the_front_template = 0U;
    num_repetitions_of_the_front_template = 0U;
    dimension_of_the_tail_template = 0U;
    num_repetitions_of_the_tail_template = 0U;

    return false;
}


column_shift_function::column_shift_function()
    : m_use_identity_function(true)
    , m_num_cells_along_x_axis(0U)
    , m_num_cells_along_y_axis(0U)
    , m_shift_templates()
    , m_num_rows_in_layouts(0U)
    , m_num_columns_in_layouts(0U)
    , m_layout_of_templates()
    , m_layout_of_scales_along_x_axis_of_tissue()
    , m_layout_of_scales_along_y_axis_of_tissue()
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
        std::vector<natural_32_bit> const&  layout_of_scales_along_x_axis_of_tissue,
        std::vector<natural_32_bit> const&  layout_of_scales_along_y_axis_of_tissue,
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
    , m_layout_of_scales_along_x_axis_of_tissue(layout_of_scales_along_x_axis_of_tissue)
    , m_layout_of_scales_along_y_axis_of_tissue(layout_of_scales_along_y_axis_of_tissue)
    , m_layout_of_row_repetitions(layout_of_row_repetitions)
    , m_layout_of_column_repetitions(layout_of_column_repetitions)
{
    ASSUMPTION(m_num_cells_along_x_axis > 0U);
    ASSUMPTION(m_num_cells_along_y_axis > 0U);

    ASSUMPTION(!shift_templates.empty() && shift_templates.size() < (1U << 15U));
    ASSUMPTION(
        [](std::vector<shift_template> const&  shift_templates){
            for (shift_template const& tpt : shift_templates)
            {
                if (num_rows(tpt) < 2U && num_columns(tpt) < 2U)
                    return false;
                if (checked_mul_16_bit(num_rows(tpt),num_columns(tpt)) != get_data(tpt).size())
                    return false;
                if (get_data(tpt).empty())
                    return false;
            }
            return true;
        }(shift_templates)
        );

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


}
