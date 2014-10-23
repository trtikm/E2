#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellab/static_state_of_neural_tissue.hpp>
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <vector>
#include <array>
#include <memory>
#include <stdexcept>

static void test_compute_kind_of_cell_from_its_position_along_columnar_axis(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue)
{
    cellab::kind_of_cell right_kind = 0U;
    natural_32_bit c_break = static_tissue->num_tissue_cells_of_cell_kind(right_kind);
    for (natural_32_bit c = 0U; c < static_tissue->num_cells_along_columnar_axis(); ++c)
    {
        if (c == c_break)
        {
            ++right_kind;
            c_break += static_tissue->num_tissue_cells_of_cell_kind(right_kind);
        }
        cellab::kind_of_cell kind =
                static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(c);

        TEST_SUCCESS(right_kind == kind);
    }
    TEST_SUCCESS(right_kind+1U == static_tissue->lowest_kind_of_sensory_cells());
    TEST_SUCCESS(c_break == static_tissue->num_cells_along_columnar_axis());
    ++right_kind;
    c_break += static_tissue->num_sensory_cells_of_cell_kind(right_kind);
    for (natural_32_bit c = 0U; c < static_tissue->num_sensory_cells(); ++c)
    {
        if (static_tissue->num_cells_along_columnar_axis() + c == c_break)
        {
            ++right_kind;
            c_break += static_tissue->num_sensory_cells_of_cell_kind(right_kind);
        }
        cellab::kind_of_cell kind =
                static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(
                    static_tissue->num_cells_along_columnar_axis() + c);

        TEST_SUCCESS(right_kind == kind);
    }
}

static void test_compute_kind_of_cell_and_relative_columnar_index_from_coordinate_along_columnar_axis(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue)
{
    TEST_SUCCESS(static_tissue->lowest_kind_of_sensory_cells() == static_tissue->num_kinds_of_tissue_cells() );
    TEST_SUCCESS(static_tissue->lowest_kind_of_sensory_cells() <= static_tissue->num_kinds_of_cells());
    natural_32_bit  num_passed_cells_of_current_kind = 0U;
    cellab::kind_of_cell  correct_kind = 0U;
    natural_32_bit  correct_relative_index = 0U;
    for (natural_32_bit  index = 0U;
         index < static_tissue->num_cells_along_columnar_axis() + static_tissue->num_sensory_cells();
         ++index)
    {
        std::pair<cellab::kind_of_cell,natural_32_bit> const pair =
            static_tissue->compute_kind_of_cell_and_relative_columnar_index_from_coordinate_along_columnar_axis(index);
        TEST_SUCCESS(pair.first == correct_kind);
        TEST_SUCCESS(pair.second == correct_relative_index);

        ++correct_relative_index;
        ++num_passed_cells_of_current_kind;
        natural_32_bit break_count =
                correct_kind < static_tissue->lowest_kind_of_sensory_cells() ?
                        static_tissue->num_tissue_cells_of_cell_kind(correct_kind) :
                        static_tissue->num_sensory_cells_of_cell_kind(correct_kind);
        if (num_passed_cells_of_current_kind == break_count)
        {
            num_passed_cells_of_current_kind = 0U;
            ++correct_kind;
            correct_relative_index = 0U;
        }
    }
}

static void test_compute_kind_of_sensory_cell_from_its_index(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue)
{
    TEST_SUCCESS(static_tissue->lowest_kind_of_sensory_cells() == static_tissue->num_kinds_of_tissue_cells() );
    TEST_SUCCESS(static_tissue->lowest_kind_of_sensory_cells() <= static_tissue->num_kinds_of_cells());
    natural_32_bit  num_passed_cells_of_current_kind = 0U;
    cellab::kind_of_cell  correct_kind = static_tissue->lowest_kind_of_sensory_cells();
    for (natural_32_bit  index = 0U; index < static_tissue->num_kinds_of_sensory_cells(); ++index)
    {
        cellab::kind_of_cell const  kind = static_tissue->compute_kind_of_sensory_cell_from_its_index(index);
        TEST_SUCCESS(kind == correct_kind);

        ++num_passed_cells_of_current_kind;
        if (num_passed_cells_of_current_kind == static_tissue->num_sensory_cells_of_cell_kind(correct_kind))
        {
            num_passed_cells_of_current_kind = 0U;
            ++correct_kind;
        }
    }
}

static void test_compute_kind_of_sensory_cell_and_relative_index_from_its_index(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue)
{
    TEST_SUCCESS(static_tissue->lowest_kind_of_sensory_cells() == static_tissue->num_kinds_of_tissue_cells() );
    TEST_SUCCESS(static_tissue->lowest_kind_of_sensory_cells() <= static_tissue->num_kinds_of_cells());
    natural_32_bit  num_passed_cells_of_current_kind = 0U;
    cellab::kind_of_cell  correct_kind = static_tissue->lowest_kind_of_sensory_cells();
    natural_32_bit  correct_relative_index = 0U;
    for (natural_32_bit  index = 0U; index < static_tissue->num_kinds_of_sensory_cells(); ++index)
    {
        std::pair<cellab::kind_of_cell,natural_32_bit> const pair =
            static_tissue->compute_kind_of_sensory_cell_and_relative_index_from_its_index(index);
        TEST_SUCCESS(pair.first == correct_kind);
        TEST_SUCCESS(pair.second == correct_relative_index);

        ++correct_relative_index;
        ++num_passed_cells_of_current_kind;
        if (num_passed_cells_of_current_kind == static_tissue->num_sensory_cells_of_cell_kind(correct_kind))
        {
            num_passed_cells_of_current_kind = 0U;
            ++correct_kind;
            correct_relative_index = 0U;
        }
    }
}

static void test_compute_index_of_first_sensory_cell_of_kind(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue)
{
    TEST_SUCCESS(static_tissue->lowest_kind_of_sensory_cells() == static_tissue->num_kinds_of_tissue_cells() );
    TEST_SUCCESS(static_tissue->lowest_kind_of_sensory_cells() <= static_tissue->num_kinds_of_cells());
    natural_32_bit correct_index = 0U;
    for (natural_16_bit kind = static_tissue->lowest_kind_of_sensory_cells();
         kind < static_tissue->num_kinds_of_cells();
         correct_index += static_tissue->num_sensory_cells_of_cell_kind(kind),
         ++kind)
    {
        TEST_SUCCESS( correct_index == static_tissue->compute_index_of_first_sensory_cell_of_kind(kind) );
    }
}

static void test_shift_in_coordinates(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue)
{
    typedef cellab::tissue_coordinates coords;
    typedef cellab::shift_in_coordinates shift;

    natural_32_bit const xL = 0U;
    natural_32_bit const xM = static_tissue->num_cells_along_x_axis() / 2U;
    natural_32_bit const xH = static_tissue->num_cells_along_x_axis() - 1U;
    natural_32_bit const yL = 0U;
    natural_32_bit const yM = static_tissue->num_cells_along_y_axis() / 2U;
    natural_32_bit const yH = static_tissue->num_cells_along_y_axis() - 1U;
    natural_32_bit const cL = 0U;
    natural_32_bit const cM = static_tissue->num_cells_along_columnar_axis() / 2U;
    natural_32_bit const cH = static_tissue->num_cells_along_columnar_axis() - 1U;

    std::array<natural_32_bit const,3> X = { xL, xM, xH };
    std::array<natural_32_bit const,3> Y = { yL, yM, yH };
    std::array<natural_32_bit const,3> C = { cL, cM, cH };

    struct local {
        static coords  shift_coords(coords const& c,shift const& s,
                                    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue)
        {
            return cellab::shift_coordinates(c,s,static_tissue->num_cells_along_x_axis(),
                                                 static_tissue->num_cells_along_y_axis(),
                                                 static_tissue->num_cells_along_columnar_axis() );
        }
    };

    for (natural_32_bit i = 0U; i < X.size(); ++i)
        for (natural_32_bit j = 0U; j < Y.size(); ++j)
            for (natural_32_bit k = 0U; k < C.size(); ++k)
                for (integer_8_bit u = -1; u < 2; ++u)
                    for (integer_8_bit v = -1; v < 2; ++v)
                        for (integer_8_bit w = -1; w < 2; ++w)
                        {
                            natural_32_bit x,y,c;

                                 if (X.at(i) == xL && u == -1 &&  static_tissue->is_x_axis_torus_axis()) x = xH;
                            else if (X.at(i) == xL && u == -1 && !static_tissue->is_x_axis_torus_axis()) continue;
                            else if (X.at(i) == xH && u ==  1 &&  static_tissue->is_x_axis_torus_axis()) x = xL;
                            else if (X.at(i) == xH && u ==  1 && !static_tissue->is_x_axis_torus_axis()) continue;
                            else x = X.at(i) + u;

                                 if (Y.at(j) == yL && v == -1 &&  static_tissue->is_y_axis_torus_axis()) y = yH;
                            else if (Y.at(j) == yL && v == -1 && !static_tissue->is_y_axis_torus_axis()) continue;
                            else if (Y.at(j) == yH && v ==  1 &&  static_tissue->is_y_axis_torus_axis()) y = yL;
                            else if (Y.at(j) == yH && v ==  1 && !static_tissue->is_y_axis_torus_axis()) continue;
                            else y = Y.at(j) + v;

                                 if (C.at(k) == cL && w == -1 &&  static_tissue->is_columnar_axis_torus_axis()) c = cH;
                            else if (C.at(k) == cL && w == -1 && !static_tissue->is_columnar_axis_torus_axis()) continue;
                            else if (C.at(k) == cH && w ==  1 &&  static_tissue->is_columnar_axis_torus_axis()) c = cL;
                            else if (C.at(k) == cH && w ==  1 && !static_tissue->is_columnar_axis_torus_axis()) continue;
                            else c = C.at(k) + w;
                            TEST_SUCCESS(coords(x,y,c) == local::shift_coords(coords(X.at(i),Y.at(j),C.at(k)),shift(u,v,w),static_tissue));
                        }
}

static void test_find_bits_of_cell(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue =
            dynamic_tissue->get_static_state_of_neural_tissue();
    natural_16_bit const num_bits_per_cell = static_tissue->num_bits_per_cell();
    for (natural_16_bit kind = 0U; kind < static_tissue->num_kinds_of_cells(); ++kind)
    {
        natural_32_bit const num_cells_of_kind =
                kind < static_tissue->lowest_kind_of_sensory_cells() ?
                        static_tissue->num_tissue_cells_of_cell_kind(kind) :
                        static_tissue->num_sensory_cells_of_cell_kind(kind);
        TEST_SUCCESS( num_cells_of_kind > 0U );
        std::array<natural_32_bit,3> const relative_indices = {
                0U, num_cells_of_kind / 2U, num_cells_of_kind - 1U
                };
        for (natural_32_bit i = 0U; i < relative_indices.size(); ++i)
        {
            natural_32_bit const index = relative_indices.at(i);
            natural_32_bit const xL = 0U;
            natural_32_bit const xM = static_tissue->num_cells_along_x_axis() / 2U;
            natural_32_bit const xH = static_tissue->num_cells_along_x_axis() - 1U;
            natural_32_bit const yL = 0U;
            natural_32_bit const yM = static_tissue->num_cells_along_y_axis() / 2U;
            natural_32_bit const yH = static_tissue->num_cells_along_y_axis() - 1U;
            TEST_SUCCESS(dynamic_tissue->find_bits_of_cell(xL,yL,kind,index).num_bits() == num_bits_per_cell);
            TEST_SUCCESS(dynamic_tissue->find_bits_of_cell(xL,yM,kind,index).num_bits() == num_bits_per_cell);
            TEST_SUCCESS(dynamic_tissue->find_bits_of_cell(xL,yH,kind,index).num_bits() == num_bits_per_cell);
            TEST_SUCCESS(dynamic_tissue->find_bits_of_cell(xM,yL,kind,index).num_bits() == num_bits_per_cell);
            TEST_SUCCESS(dynamic_tissue->find_bits_of_cell(xM,yM,kind,index).num_bits() == num_bits_per_cell);
            TEST_SUCCESS(dynamic_tissue->find_bits_of_cell(xM,yH,kind,index).num_bits() == num_bits_per_cell);
            TEST_SUCCESS(dynamic_tissue->find_bits_of_cell(xH,yL,kind,index).num_bits() == num_bits_per_cell);
            TEST_SUCCESS(dynamic_tissue->find_bits_of_cell(xH,yM,kind,index).num_bits() == num_bits_per_cell);
            TEST_SUCCESS(dynamic_tissue->find_bits_of_cell(xH,yH,kind,index).num_bits() == num_bits_per_cell);
        }
    }
}

static void test_find_bits_of_cell_in_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue =
            dynamic_tissue->get_static_state_of_neural_tissue();
    natural_16_bit const num_bits_per_cell = static_tissue->num_bits_per_cell();
    std::array<natural_32_bit,3> const coords_x = {
            0U,
            static_tissue->num_cells_along_x_axis() / 2U,
            static_tissue->num_cells_along_x_axis() - 1U
            };
    std::array<natural_32_bit,3> const coords_y = {
            0U,
            static_tissue->num_cells_along_y_axis() / 2U,
            static_tissue->num_cells_along_y_axis() - 1U
            };

    std::array<natural_32_bit,3> const coords_z = {
            0U,
            static_tissue->num_cells_along_columnar_axis() / 2U,
            static_tissue->num_cells_along_columnar_axis() - 1U
            };
    for (natural_32_bit i = 0U; i < coords_x.size(); ++i)
        for (natural_32_bit j = 0U; j < coords_y.size(); ++j)
            for (natural_32_bit k = 0U; k < coords_z.size(); ++k)
                TEST_SUCCESS(
                    dynamic_tissue->find_bits_of_cell_in_tissue(
                            coords_x.at(i),coords_y.at(j),coords_z.at(k)
                            ).num_bits()
                    == num_bits_per_cell
                    );
}

static void test_find_bits_of_synapse_in_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue =
            dynamic_tissue->get_static_state_of_neural_tissue();
    natural_16_bit const num_bits_per_synapse = static_tissue->num_bits_per_synapse();
    std::array<natural_32_bit,3> const coords_x = {
            0U,
            static_tissue->num_cells_along_x_axis() / 2U,
            static_tissue->num_cells_along_x_axis() - 1U
            };
    std::array<natural_32_bit,3> const coords_y = {
            0U,
            static_tissue->num_cells_along_y_axis() / 2U,
            static_tissue->num_cells_along_y_axis() - 1U
            };

    std::array<natural_32_bit,3> const coords_c = {
            0U,
            static_tissue->num_cells_along_columnar_axis() / 2U,
            static_tissue->num_cells_along_columnar_axis() - 1U
            };
    for (natural_32_bit i = 0U; i < coords_x.size(); ++i)
        for (natural_32_bit j = 0U; j < coords_y.size(); ++j)
            for (natural_32_bit k = 0U; k < coords_c.size(); ++k)
            {
                cellab::kind_of_cell const kind =
                        static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(
                                coords_c.at(k)
                                );
                natural_32_bit const num_synapses =
                        static_tissue->num_synapses_in_territory_of_cell_kind(kind);
                std::array<natural_32_bit,3> const synapse_indices = {
                        0U, num_synapses / 2U, num_synapses - 1U
                        };
                for (natural_32_bit s = 0U; s < synapse_indices.size(); ++s)
                    TEST_SUCCESS(
                        dynamic_tissue->find_bits_of_synapse_in_tissue(
                                coords_x.at(i),coords_y.at(j),coords_c.at(k),synapse_indices.at(k)
                                ).num_bits()
                        == num_bits_per_synapse
                        );
            }
}

static void test_find_bits_of_territorial_state_of_synapse_in_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue =
            dynamic_tissue->get_static_state_of_neural_tissue();
    std::array<natural_32_bit,3> const coords_x = {
            0U,
            static_tissue->num_cells_along_x_axis() / 2U,
            static_tissue->num_cells_along_x_axis() - 1U
            };
    std::array<natural_32_bit,3> const coords_y = {
            0U,
            static_tissue->num_cells_along_y_axis() / 2U,
            static_tissue->num_cells_along_y_axis() - 1U
            };

    std::array<natural_32_bit,3> const coords_c = {
            0U,
            static_tissue->num_cells_along_columnar_axis() / 2U,
            static_tissue->num_cells_along_columnar_axis() - 1U
            };
    for (natural_32_bit i = 0U; i < coords_x.size(); ++i)
        for (natural_32_bit j = 0U; j < coords_y.size(); ++j)
            for (natural_32_bit k = 0U; k < coords_c.size(); ++k)
            {
                cellab::kind_of_cell const kind =
                        static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(
                                coords_c.at(k)
                                );
                natural_32_bit const num_synapses =
                        static_tissue->num_synapses_in_territory_of_cell_kind(kind);
                std::array<natural_32_bit,3> const synapse_indices = {
                        0U, num_synapses / 2U, num_synapses - 1U
                        };
                for (natural_32_bit s = 0U; s < synapse_indices.size(); ++s)
                    TEST_SUCCESS(
                        dynamic_tissue->find_bits_of_territorial_state_of_synapse_in_tissue(
                                coords_x.at(i),coords_y.at(j),coords_c.at(k),synapse_indices.at(k)
                                ).num_bits()
                        == cellab::num_of_bits_to_store_territorial_state_of_synapse()
                        );
            }
}

static void test_find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue =
            dynamic_tissue->get_static_state_of_neural_tissue();
    natural_8_bit const num_bits_of_all_coords = 3U * dynamic_tissue->num_bits_per_source_cell_coordinate();
    std::array<natural_32_bit,3> const coords_x = {
            0U,
            static_tissue->num_cells_along_x_axis() / 2U,
            static_tissue->num_cells_along_x_axis() - 1U
            };
    std::array<natural_32_bit,3> const coords_y = {
            0U,
            static_tissue->num_cells_along_y_axis() / 2U,
            static_tissue->num_cells_along_y_axis() - 1U
            };

    std::array<natural_32_bit,3> const coords_c = {
            0U,
            static_tissue->num_cells_along_columnar_axis() / 2U,
            static_tissue->num_cells_along_columnar_axis() - 1U
            };
    for (natural_32_bit i = 0U; i < coords_x.size(); ++i)
        for (natural_32_bit j = 0U; j < coords_y.size(); ++j)
            for (natural_32_bit k = 0U; k < coords_c.size(); ++k)
            {
                cellab::kind_of_cell const kind =
                        static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(
                                coords_c.at(k)
                                );
                natural_32_bit const num_synapses =
                        static_tissue->num_synapses_in_territory_of_cell_kind(kind);
                std::array<natural_32_bit,3> const synapse_indices = {
                        0U, num_synapses / 2U, num_synapses - 1U
                        };
                for (natural_32_bit s = 0U; s < synapse_indices.size(); ++s)
                    TEST_SUCCESS(
                        dynamic_tissue->find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
                                coords_x.at(i),coords_y.at(j),coords_c.at(k),synapse_indices.at(k)
                                ).num_bits()
                        == num_bits_of_all_coords
                        );
            }
}

static void test_find_bits_of_signalling(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue =
            dynamic_tissue->get_static_state_of_neural_tissue();
    std::array<natural_32_bit,3> const coords_x = {
            0U,
            static_tissue->num_cells_along_x_axis() / 2U,
            static_tissue->num_cells_along_x_axis() - 1U
            };
    std::array<natural_32_bit,3> const coords_y = {
            0U,
            static_tissue->num_cells_along_y_axis() / 2U,
            static_tissue->num_cells_along_y_axis() - 1U
            };

    std::array<natural_32_bit,3> const coords_z = {
            0U,
            static_tissue->num_cells_along_columnar_axis() / 2U,
            static_tissue->num_cells_along_columnar_axis() - 1U
            };
    for (natural_32_bit i = 0U; i < coords_x.size(); ++i)
        for (natural_32_bit j = 0U; j < coords_y.size(); ++j)
            for (natural_32_bit k = 0U; k < coords_z.size(); ++k)
                TEST_SUCCESS(
                    dynamic_tissue->find_bits_of_signalling(
                            coords_x.at(i),coords_y.at(j),coords_z.at(k)
                            ).num_bits()
                    == static_tissue->num_bits_per_signalling()
                    );
}

static void test_find_bits_of_delimiter_between_teritorial_lists(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue =
            dynamic_tissue->get_static_state_of_neural_tissue();
    std::array<natural_32_bit,3> const coords_x = {
            0U,
            static_tissue->num_cells_along_x_axis() / 2U,
            static_tissue->num_cells_along_x_axis() - 1U
            };
    std::array<natural_32_bit,3> const coords_y = {
            0U,
            static_tissue->num_cells_along_y_axis() / 2U,
            static_tissue->num_cells_along_y_axis() - 1U
            };
    for (natural_32_bit i = 0U; i < coords_x.size(); ++i)
        for (natural_32_bit j = 0U; j < coords_y.size(); ++j)
        {
            natural_32_bit coord_c = 0U;
            for (cellab::kind_of_cell kind = 0U; kind < static_tissue->num_kinds_of_tissue_cells(); ++kind)
            {
                for (natural_8_bit delim_index = 0U; delim_index < cellab::num_delimiters(); ++delim_index)
                {
                    TEST_SUCCESS(
                        dynamic_tissue->find_bits_of_delimiter_between_teritorial_lists(
                                coords_x.at(i),coords_y.at(j),coord_c,delim_index
                                ).num_bits()
                        == dynamic_tissue->num_bits_per_delimiter_number(kind)
                        );

                }
                coord_c += static_tissue->num_tissue_cells_of_cell_kind(kind);
            }
        }
}

static void test_find_bits_of_sensory_cell(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue =
            dynamic_tissue->get_static_state_of_neural_tissue();
    for (natural_32_bit c = 0U; c < static_tissue->num_sensory_cells(); ++c)
        TEST_SUCCESS(
            dynamic_tissue->find_bits_of_sensory_cell(c).num_bits()
            == static_tissue->num_bits_per_cell()
            );
}

static void test_find_bits_of_synapse_to_muscle(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue =
            dynamic_tissue->get_static_state_of_neural_tissue();
    for (natural_32_bit s = 0U; s < static_tissue->num_synapses_to_muscles(); ++s)
        TEST_SUCCESS(
            dynamic_tissue->find_bits_of_synapse_to_muscle(s).num_bits()
            == static_tissue->num_bits_per_synapse()
            );
}

static void test_static_state(std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue)
{
    test_compute_kind_of_cell_from_its_position_along_columnar_axis(static_tissue);
    test_compute_kind_of_cell_and_relative_columnar_index_from_coordinate_along_columnar_axis(static_tissue);
    test_compute_kind_of_sensory_cell_from_its_index(static_tissue);
    test_compute_kind_of_sensory_cell_and_relative_index_from_its_index(static_tissue);
    test_compute_index_of_first_sensory_cell_of_kind(static_tissue);
}

static void test_dynamic_state(std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    test_find_bits_of_cell(dynamic_tissue);
    test_find_bits_of_cell_in_tissue(dynamic_tissue);
    test_find_bits_of_synapse_in_tissue(dynamic_tissue);
    test_find_bits_of_territorial_state_of_synapse_in_tissue(dynamic_tissue);
    test_find_bits_of_coords_of_source_cell_of_synapse_in_tissue(dynamic_tissue);
    test_find_bits_of_signalling(dynamic_tissue);
    test_find_bits_of_delimiter_between_teritorial_lists(dynamic_tissue);
    test_find_bits_of_sensory_cell(dynamic_tissue);
    test_find_bits_of_synapse_to_muscle(dynamic_tissue);
}

void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    for (natural_16_bit tissue_cell_kinds = 1U; tissue_cell_kinds < 6U; ++tissue_cell_kinds)
    {
        for (natural_16_bit sensory_cell_kinds = 1U; sensory_cell_kinds < 3U; ++sensory_cell_kinds)
        {
            natural_16_bit const num_bits_per_cell = 1U;
            natural_16_bit const num_bits_per_synapse = 1U;
            natural_16_bit const num_bits_per_signalling = 1U;

            for (natural_32_bit cells_x = 1U; cells_x < 10000U; cells_x += 500U)
            {
                for (natural_32_bit cells_y = 1U; cells_y < 10000U; cells_y += 500U)
                {
                    std::vector<natural_32_bit> num_tissue_cells_of_cell_kind;
                    std::vector<natural_32_bit> num_synapses_in_territory_of_cell_kind;
                    for (natural_16_bit i = 1U; i <= tissue_cell_kinds; ++i)
                    {
                        num_tissue_cells_of_cell_kind.push_back(i);
                        num_synapses_in_territory_of_cell_kind.push_back(10U+i);
                    }

                    std::vector<natural_32_bit> num_sensory_cells_of_cell_kind;
                    for (natural_16_bit i = 0U; i < sensory_cell_kinds; ++i)
                        num_sensory_cells_of_cell_kind.push_back((i%2 == 0) ? 1U : 5000U);

                    natural_32_bit const num_synapses_to_muscles = 6000U;

                    for (natural_8_bit torus_x = 0U; torus_x < 2U; ++torus_x)
                        for (natural_8_bit torus_y = 0U; torus_y < 2U; ++torus_y)
                            for (natural_8_bit torus_c = 0U; torus_c < 2U; ++torus_c)
                            {
                                bool const is_x_axis_torus_axis = torus_x == 1;
                                bool const is_y_axis_torus_axis = torus_y == 1;
                                bool const is_c_axis_torus_axis = torus_c == 1;

                                for (natural_8_bit radius = 1U; radius < 3U; ++radius)
                                {
                                    std::vector<integer_8_bit> x_radius_of_signalling_neighbourhood_of_cell;
                                    std::vector<integer_8_bit> y_radius_of_signalling_neighbourhood_of_cell;
                                    std::vector<integer_8_bit> columnar_radius_of_signalling_neighbourhood_of_cell;
                                    std::vector<integer_8_bit> x_radius_of_signalling_neighbourhood_of_synapse;
                                    std::vector<integer_8_bit> y_radius_of_signalling_neighbourhood_of_synapse;
                                    std::vector<integer_8_bit> columnar_radius_of_signalling_neighbourhood_of_synapse;
                                    std::vector<integer_8_bit> x_radius_of_cellular_neighbourhood_of_signalling;
                                    std::vector<integer_8_bit> y_radius_of_cellular_neighbourhood_of_signalling;
                                    std::vector<integer_8_bit> columnar_radius_of_cellular_neighbourhood_of_signalling;
                                    for (natural_16_bit i = 0U; i < tissue_cell_kinds; ++i)
                                    {
                                        x_radius_of_signalling_neighbourhood_of_cell.push_back(radius);
                                        y_radius_of_signalling_neighbourhood_of_cell.push_back(radius);
                                        columnar_radius_of_signalling_neighbourhood_of_cell.push_back(radius);
                                        x_radius_of_signalling_neighbourhood_of_synapse.push_back(radius);
                                        y_radius_of_signalling_neighbourhood_of_synapse.push_back(radius);
                                        columnar_radius_of_signalling_neighbourhood_of_synapse.push_back(radius);
                                        x_radius_of_cellular_neighbourhood_of_signalling.push_back(radius);
                                        y_radius_of_cellular_neighbourhood_of_signalling.push_back(radius);
                                        columnar_radius_of_cellular_neighbourhood_of_signalling.push_back(radius);
                                    }

                                    std::shared_ptr<cellab::static_state_of_neural_tissue const> static_tissue;
                                    try
                                    {
                                        static_tissue = std::shared_ptr<cellab::static_state_of_neural_tissue const>(
                                                    new cellab::static_state_of_neural_tissue(
                                                        tissue_cell_kinds,
                                                        sensory_cell_kinds,
                                                        num_bits_per_cell,
                                                        num_bits_per_synapse,
                                                        num_bits_per_signalling,
                                                        cells_x,
                                                        cells_y,
                                                        num_tissue_cells_of_cell_kind,
                                                        num_synapses_in_territory_of_cell_kind,
                                                        num_sensory_cells_of_cell_kind,
                                                        num_synapses_to_muscles,
                                                        is_x_axis_torus_axis,
                                                        is_y_axis_torus_axis,
                                                        is_c_axis_torus_axis,
                                                        x_radius_of_signalling_neighbourhood_of_cell,
                                                        y_radius_of_signalling_neighbourhood_of_cell,
                                                        columnar_radius_of_signalling_neighbourhood_of_cell,
                                                        x_radius_of_signalling_neighbourhood_of_synapse,
                                                        y_radius_of_signalling_neighbourhood_of_synapse,
                                                        columnar_radius_of_signalling_neighbourhood_of_synapse,
                                                        x_radius_of_cellular_neighbourhood_of_signalling,
                                                        y_radius_of_cellular_neighbourhood_of_signalling,
                                                        columnar_radius_of_cellular_neighbourhood_of_signalling
                                                        ));
                                    }
                                    catch(...)
                                    {
//                                        LOG(testing,"Cannot create static state of tissue : "
//                                                    "num_kinds_of_tissue_cells = " << tissue_cell_kinds << ",   " <<
//                                                    "num_kinds_of_sensory_cells = " << sensory_cell_kinds << ",   " <<
//                                                    "num_cells_along_x_axis = " << cells_x << ",   " <<
//                                                    "num_cells_along_y_axis = " << cells_y << ",   " <<
//                                                    "radius = " << (natural_32_bit)radius
//                                                    );
                                        continue;
                                    }
                                    test_static_state(static_tissue);
                                    test_shift_in_coordinates(static_tissue);

                                    std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_tissue;
                                    try
                                    {
                                        dynamic_tissue = std::shared_ptr<cellab::dynamic_state_of_neural_tissue>(
                                                    new cellab::dynamic_state_of_neural_tissue(static_tissue)
                                                    );
                                    }
                                    catch(...)
                                    {
//                                        LOG(testing,"Cannot create dynamic state of tissue : "
//                                                    "num_kinds_of_tissue_cells = " << tissue_cell_kinds << ",   " <<
//                                                    "num_kinds_of_sensory_cells = " << sensory_cell_kinds << ",   " <<
//                                                    "num_cells_along_x_axis = " << cells_x << ",   " <<
//                                                    "num_cells_along_y_axis = " << cells_y << ",   " <<
//                                                    "radius = " << (natural_32_bit)radius
//                                                    );
                                        continue;
                                    }
                                    test_dynamic_state(dynamic_tissue);
                                }
                            }

                    TEST_PROGRESS_UPDATE();
                }
            }
        }
    }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
