#include "./program_info.hpp"
#include "./program_options.hpp"
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/random.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <functional>
#include <vector>
#include <cmath>


void random_sequence_modulo(
        natural_32_bit const  min_number,
        natural_32_bit const  max_number,
        std::function<natural_32_bit()> random_generator,
        natural_32_bit const  num_classes,
        natural_32_bit const  sequence_size
        )
{
    ASSUMPTION(max_number > min_number);
    ASSUMPTION(sequence_size >= num_classes);
    ASSUMPTION(num_classes > 1U);

    std::vector<natural_32_bit>  big_sequence(max_number - min_number + 1,0);
    std::vector< std::vector<natural_32_bit> >  modulo_classes(num_classes);
    for (natural_32_bit i = 0U; i < num_classes; ++i)
        modulo_classes.at(i).resize(max_number - min_number + 1,0);

    for (natural_32_bit i = 0U; i < sequence_size; ++i)
    {
        natural_32_bit const  number = random_generator();
        ASSUMPTION(number >= min_number);
        ASSUMPTION(number <= max_number);

        ++big_sequence.at(number - min_number);
        ++modulo_classes.at(i % num_classes).at(number - min_number);

        TEST_PROGRESS_UPDATE();
    }

    for (natural_32_bit i = 0U; i < num_classes; ++i)
    {
        float_64_bit  error = 0;
        for (natural_32_bit j = min_number; j <= max_number; ++j)
        {
            float_64_bit const  error_j =
                    std::abs(
                        (static_cast<float_64_bit>(big_sequence.at(j - min_number)) -
                         static_cast<float_64_bit>(num_classes * modulo_classes.at(i).at(j - min_number)))
                        / (static_cast<float_64_bit>(big_sequence.at(j - min_number)) + 1.0)
                        );
            if (error_j > error)
                error = error_j;
        }
        std::cout << "err C[" << i << "] = " << error << "\n";
    }
}



void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    natural_32_bit const  min_number = 0U;
    natural_32_bit const  max_number = 10U;
    natural_32_bit const  num_classes = 20U;
    std::vector<natural_32_bit>  sequence_lenghts = {
        //100U,
        //1000U,
        //10000U,
        100000U,
        //1000000U
    };

    for (natural_32_bit i = 0U; i < sequence_lenghts.size(); ++i)
        random_sequence_modulo(
                min_number,max_number,
                std::bind(&get_random_natural_32_bit_in_range,min_number,max_number),
                num_classes,
                sequence_lenghts.at(i)
                );

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
