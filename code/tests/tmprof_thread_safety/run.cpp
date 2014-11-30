#include "./program_info.hpp"
#include "./program_options.hpp"
#include <utility/basic_numeric_types.hpp>
#include <utility/invariants.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <vector>
#include <thread>


natural_64_bit  sum_number(natural_64_bit const number)
{
    TMPROF_BLOCK();

    if (number == 0ULL)
        return number;
    natural_64_bit  const value = number + sum_number(number - 1ULL);
    return value;
}

natural_64_bit  some_computation(natural_64_bit const number)
{
    TMPROF_BLOCK();

    if (number == 0ULL)
        return number;
    natural_64_bit  const value = number + some_computation(number - 1ULL);
    return 2ULL * value - 10ULL;
}


void thread_computation(natural_64_bit const number)
{
    TMPROF_BLOCK();

    natural_64_bit volatile result = 0ULL;

    {
        TMPROF_BLOCK();
        result += sum_number(number);
    }
    {
        TMPROF_BLOCK();
        result += some_computation(number);
    }

    TEST_PROGRESS_UPDATE();
}

natural_64_bit  get_num_hits_of(std::string const& function_name)
{
    tmprof::RecordReaders records;
    tmprof::write(records);
    for (auto it = records.begin(); it != records.end(); ++it)
        if (it->functionName().find(function_name) != std::string::npos)
            return it->totalExecutions();
    UNREACHABLE();
}

float_64_bit  get_time_of(std::string const& function_name)
{
    tmprof::RecordReaders records;
    tmprof::write(records);
    for (auto it = records.begin(); it != records.end(); ++it)
        if (it->functionName().find(function_name) != std::string::npos)
            return it->totalDuration();
    UNREACHABLE();
}

void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    natural_64_bit const  num_numbers = 100;

    for (natural_64_bit  i = 0; i < num_numbers; ++i)
        thread_computation(i);

    natural_64_bit const  num_hits_of_sum_number_single_thread = get_num_hits_of("sum_number");
    TEST_SUCCESS(num_hits_of_sum_number_single_thread > 0ULL);
    float_64_bit const  time_of_sum_number_single_thread = get_num_hits_of("sum_number");
    TEST_SUCCESS(time_of_sum_number_single_thread > 0.0);

    natural_64_bit const  num_hits_of_some_computation_single_thread = get_num_hits_of("some_computation");
    TEST_SUCCESS(num_hits_of_some_computation_single_thread > 0ULL);
    float_64_bit const  time_of_some_computation_single_thread = get_num_hits_of("some_computation");
    TEST_SUCCESS(time_of_some_computation_single_thread > 0.0);

    std::vector<std::thread> threads;
    for (natural_64_bit i = 1LLU; i < num_numbers; ++i)
        threads.push_back( std::thread(&thread_computation,i) );
    thread_computation(0ULL);
    for(std::thread& thread : threads)
        thread.join();

    natural_64_bit const  num_hits_of_sum_number_multi_thread = get_num_hits_of("sum_number");
    TEST_SUCCESS(num_hits_of_sum_number_multi_thread == 2ULL * num_hits_of_sum_number_single_thread);
    float_64_bit const  time_of_sum_number_multi_thread = get_num_hits_of("sum_number");
    TEST_SUCCESS(time_of_sum_number_multi_thread > time_of_sum_number_single_thread);

    natural_64_bit const  num_hits_of_some_computation_multi_thread = get_num_hits_of("some_computation");
    TEST_SUCCESS(num_hits_of_some_computation_multi_thread == 2ULL * num_hits_of_some_computation_single_thread);
    float_64_bit const  time_of_some_computation_multi_thread = get_num_hits_of("some_computation");
    TEST_SUCCESS(time_of_some_computation_multi_thread > time_of_some_computation_single_thread);

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
