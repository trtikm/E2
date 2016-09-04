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

void prepare_data_vector(
        natural_32_bit const  min_number,
        natural_32_bit const  max_number,
        std::vector<natural_32_bit>& data
        )
{
    ASSUMPTION(max_number > min_number);
    data.clear();
    data.resize(max_number - min_number + 1,0);
}

void prepare_data_vector(
        natural_32_bit const  min_number,
        natural_32_bit const  max_number,
        natural_32_bit const  num_classes,
        std::vector< std::vector<natural_32_bit> >&  data
        )
{
    ASSUMPTION(max_number > min_number);
    ASSUMPTION(num_classes > 1U);
    data.clear();
    data.resize(num_classes);
    for (natural_32_bit i = 0U; i < num_classes; ++i)
        data.at(i).resize(max_number - min_number + 1,0);
}

void prepare_statistics_vector(
        natural_32_bit const  num_classes,
        std::vector<float_64_bit>&  statistics
        )
{
    ASSUMPTION(num_classes > 1U);
    statistics.clear();
    statistics.resize(num_classes,0.0L);
}

void compute_content_of_data_vectors(
        natural_32_bit const  min_number,
        natural_32_bit const  max_number,
        std::function<natural_32_bit()> random_generator,
        natural_32_bit const  num_classes,
        natural_32_bit const  sequence_size,
        std::vector<natural_32_bit>&  big_sequence,
        std::vector< std::vector<natural_32_bit> >&  modulo_classes,
        std::vector< std::vector<natural_32_bit> >&  divide_classes
        )
{
    ASSUMPTION(max_number > min_number);
    ASSUMPTION(sequence_size >= num_classes);
    ASSUMPTION(num_classes > 1U);

    for (natural_32_bit i = 0U; i < sequence_size; ++i)
    {
        natural_32_bit const  number = random_generator();
        ASSUMPTION(number >= min_number);
        ASSUMPTION(number <= max_number);

        ++big_sequence.at(number - min_number);
        ++modulo_classes.at(i % num_classes).at(number - min_number);
        ++divide_classes.at(i / (sequence_size / num_classes)).at(number - min_number);

        TEST_PROGRESS_UPDATE();
    }

//    for (natural_32_bit i = 0U; i < num_classes; ++i)
//    {
//        float_64_bit  error = 0;
//        for (natural_32_bit j = min_number; j <= max_number; ++j)
//        {
//            float_64_bit const  error_j =
//                    std::abs(
//                        (static_cast<float_64_bit>(big_sequence.at(j - min_number)) -
//                         static_cast<float_64_bit>(num_classes * modulo_classes.at(i).at(j - min_number)))
//                        / (static_cast<float_64_bit>(big_sequence.at(j - min_number)) + 1.0)
//                        );
//            if (error_j > error)
//                error = error_j;
//        }
//        std::cout << "err mod-C[" << i << "] = " << error << "\n";
//    }

//    for (natural_32_bit i = 0U; i < num_classes; ++i)
//    {
//        float_64_bit  error = 0;
//        for (natural_32_bit j = min_number; j <= max_number; ++j)
//        {
//            float_64_bit const  error_j =
//                    std::abs(
//                        (static_cast<float_64_bit>(big_sequence.at(j - min_number)) -
//                         static_cast<float_64_bit>(num_classes * divide_classes.at(i).at(j - min_number)))
//                        / (static_cast<float_64_bit>(big_sequence.at(j - min_number)) + 1.0)
//                        );
//            if (error_j > error)
//                error = error_j;
//        }
//        std::cout << "err div-C[" << i << "] = " << error << "\n";
//    }
}


float_64_bit  compute_average(
        natural_32_bit const  min_number,
        natural_32_bit const  max_number,
        std::vector<natural_32_bit> const&  data
        )
{
    ASSUMPTION(max_number > min_number);
    ASSUMPTION(data.size() == max_number - min_number + 1);
    float_64_bit  average = 0.0L;
    for (natural_32_bit j = min_number; j <= max_number; ++j)
        average += static_cast<float_64_bit>(data.at(j - min_number));
    average /= (max_number - min_number + 1);
    return average;
}

void  multiply(
        float_64_bit const  multiplier,
        std::vector<float_64_bit>&  data
        )
{
    for (natural_32_bit i = 0U; i < data.size(); ++i)
        data.at(i) *= multiplier;
}

void compute_absolute_error_from_average(
        natural_32_bit const  min_number,
        natural_32_bit const  max_number,
        natural_32_bit const  num_classes,
        std::vector< std::vector<natural_32_bit> > const&  data,
        std::vector<float_64_bit>&  statistics
        )
{
    ASSUMPTION(max_number > min_number);
    ASSUMPTION(num_classes > 1U);
    for (natural_32_bit i = 0U; i < num_classes; ++i)
    {
        float_64_bit const  average = compute_average(min_number,max_number,data.at(i));
        for (natural_32_bit j = min_number; j <= max_number; ++j)
            statistics.at(i) += std::abs(static_cast<float_64_bit>(data.at(i).at(j - min_number)) - average);
        statistics.at(i) /= (max_number - min_number + 1);
    }
}

void present_statistics_vector(
        std::string const& title,
        natural_32_bit const  min_number,
        natural_32_bit const  max_number,
        natural_32_bit const  num_classes,
        natural_32_bit const  sequence_size,
        std::vector<float_64_bit> const&  statistics
        )
{
    ASSUMPTION(max_number > min_number);
    ASSUMPTION(sequence_size >= num_classes);
    ASSUMPTION(num_classes > 1U);
    std::cout << title << "_BEGIN\n"
              << "MIN_NUMBER: " << min_number << "\n"
              << "MAX_NUMBER: " << max_number << "\n"
              << "NUM_CLASSES: " << num_classes << "\n"
              << "SEQUENCE_SIZE: " << sequence_size << "\n"
              << "SEQUENCE_SIZE_PER_CLASS: " << sequence_size / num_classes << "\n"
              ;
    for (natural_32_bit i = 0U; i < num_classes; ++i)
        std::cout << "CLASS_VALUE[" << i << "]: " << statistics.at(i) << "\n";
    std::cout << title << "_END\n\n\n";
}

void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    natural_32_bit const  min_number = 0U;
    natural_32_bit const  max_number = 9U;
    natural_32_bit const  num_classes = 20U;
    std::vector<natural_32_bit>  sequence_lenghts = {
        //100U,
        //1000U,
        //10000U,
        100000U,
        //1000000U
    };
    std::vector<natural_32_bit>  big_sequence;
    std::vector< std::vector<natural_32_bit> >  modulo_classes;
    std::vector< std::vector<natural_32_bit> >  divide_classes;
    std::vector<float_64_bit>  modulo_statistics;
    std::vector<float_64_bit>  divide_statistics;

    for (natural_32_bit i = 0U; i < sequence_lenghts.size(); ++i)
    {
        prepare_data_vector(min_number,max_number,big_sequence);
        prepare_data_vector(min_number,max_number,num_classes,modulo_classes);
        prepare_data_vector(min_number,max_number,num_classes,divide_classes);

        compute_content_of_data_vectors(
                min_number,max_number,
                std::bind(&get_random_natural_32_bit_in_range,min_number,max_number,default_random_generator()),
                num_classes,
                sequence_lenghts.at(i),
                big_sequence,
                modulo_classes,
                divide_classes
                );

        prepare_statistics_vector(num_classes,modulo_statistics);
        compute_absolute_error_from_average(min_number,max_number,num_classes,modulo_classes,modulo_statistics);
        present_statistics_vector("MODULO_ABSOLUTE_ERROR",min_number,max_number,num_classes,sequence_lenghts.at(i),modulo_statistics);
        multiply(num_classes / (float_64_bit)sequence_lenghts.at(i),modulo_statistics);
        present_statistics_vector("MODULO_RELATIVE_ERROR",min_number,max_number,num_classes,sequence_lenghts.at(i),modulo_statistics);

        prepare_statistics_vector(num_classes,divide_statistics);
        compute_absolute_error_from_average(min_number,max_number,num_classes,divide_classes,divide_statistics);
        present_statistics_vector("DIVIDE_ABSOLUTE_ERROR",min_number,max_number,num_classes,sequence_lenghts.at(i),divide_statistics);
        multiply(num_classes / (float_64_bit)sequence_lenghts.at(i),divide_statistics);
        present_statistics_vector("MODULO_RELATIVE_ERROR",min_number,max_number,num_classes,sequence_lenghts.at(i),divide_statistics);
    }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
