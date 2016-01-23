#include "./program_info.hpp"
#include "./program_options.hpp"
#include <utility/basic_numeric_types.hpp>
#include <utility/invariants.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>


static void  never_executed() { TMPROF_BLOCK(); }


static bool  foo_paused = true;

void  FOO()
{
    while (foo_paused)
    {}

    {
        TMPROF_BLOCK();
    }
}


natural_64_bit  sum_number_non_recursive(natural_64_bit const number)
{
    TMPROF_BLOCK();

    std::this_thread::sleep_for( std::chrono::milliseconds(2));

    natural_64_bit  result = 0ULL;
    for (natural_64_bit  value = 1ULL; value < number; ++value)
        result += value;
    return result;
}

natural_64_bit  some_computation_non_recursive(natural_64_bit const number)
{
    TMPROF_BLOCK();

    std::this_thread::sleep_for( std::chrono::milliseconds(2));

    natural_64_bit  result = 0ULL;
    for (natural_64_bit  value = 1ULL; value < number; ++value)
        result = value + 2ULL * result - 10ULL;
    return result;
}



natural_64_bit  sum_number(natural_64_bit const number)
{
    TMPROF_BLOCK();

    std::this_thread::sleep_for( std::chrono::milliseconds(2));

    if (number == 0ULL)
        return number;
    natural_64_bit  const value = number + sum_number(number - 1ULL);
    return value;
}

natural_64_bit  some_computation(natural_64_bit const number)
{
    TMPROF_BLOCK();

    std::this_thread::sleep_for( std::chrono::milliseconds(2));

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
        result += sum_number_non_recursive(number);
    }
    {
        TMPROF_BLOCK();
        result += some_computation_non_recursive(number);
    }

    {
        TMPROF_BLOCK();
        result += sum_number(number);
    }
    {
        TMPROF_BLOCK();
        result += some_computation(number);
    }
}

natural_64_bit  get_num_hits_of(std::string const& function_name)
{
    std::vector<time_profile_data_of_block> records;
    copy_time_profile_data_of_all_measured_blocks_into_vector(records);
    for (auto it = records.cbegin(); it != records.cend(); ++it)
        if (it->function_name().find(function_name) != std::string::npos)
            return it->number_of_executions();
    UNREACHABLE();
}

float_64_bit  get_time_of(std::string const& function_name)
{
    std::vector<time_profile_data_of_block> records;
    copy_time_profile_data_of_all_measured_blocks_into_vector(records);
    for (auto it = records.cbegin(); it != records.cend(); ++it)
        if (it->function_name().find(function_name) != std::string::npos)
            return it->summary_duration_of_all_executions_in_seconds();
    UNREACHABLE();
}

void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    void (*suppress_never_used_warning)() = &never_executed;
    (void)suppress_never_used_warning;


    std::vector<std::thread> foo_threads;
    for (natural_32_bit i = 0U; i < 100; ++i)
        foo_threads.push_back(std::thread(&FOO));
    foo_paused = false;
    for (auto& foo_thread : foo_threads)
        foo_thread.join();
    foo_threads.clear();

    TEST_PROGRESS_UPDATE();

    std::vector<time_profile_data_of_block> data;
    copy_time_profile_data_of_all_measured_blocks_into_vector(data,false);
    natural_32_bit  foo_count = 0U;
    for (auto const& record : data)
        if (record.function_name() == "FOO")
            ++foo_count;
    TEST_SUCCESS(foo_count == 1U);

    TEST_PROGRESS_UPDATE();

    natural_64_bit const  num_numbers = 100;

    for (natural_64_bit  i = 0; i < num_numbers; ++i)
    {
        thread_computation(i);
        TEST_PROGRESS_UPDATE();
    }

    natural_64_bit const  num_hits_of_sum_number_single_thread = get_num_hits_of("sum_number");
    TEST_SUCCESS(num_hits_of_sum_number_single_thread > 0ULL);
    float_64_bit const  time_of_sum_number_single_thread = (float_64_bit)get_num_hits_of("sum_number");
    TEST_SUCCESS(time_of_sum_number_single_thread > 0.0);

    natural_64_bit const  num_hits_of_some_computation_single_thread = get_num_hits_of("some_computation");
    TEST_SUCCESS(num_hits_of_some_computation_single_thread > 0ULL);
    float_64_bit const  time_of_some_computation_single_thread = (float_64_bit)get_num_hits_of("some_computation");
    TEST_SUCCESS(time_of_some_computation_single_thread > 0.0);

    std::vector<std::thread> threads;
    for (natural_64_bit i = 1ULL; i < num_numbers; ++i)
        threads.push_back( std::thread(&thread_computation,i) );
    thread_computation(0ULL);

    TEST_PROGRESS_UPDATE();

    for(std::thread& thread : threads)
        thread.join();

    TEST_PROGRESS_UPDATE();

    natural_64_bit const  num_hits_of_sum_number_multi_thread = get_num_hits_of("sum_number");
    TEST_SUCCESS(num_hits_of_sum_number_multi_thread == 2ULL * num_hits_of_sum_number_single_thread);
    float_64_bit const  time_of_sum_number_multi_thread = (float_64_bit)get_num_hits_of("sum_number");
    TEST_SUCCESS(time_of_sum_number_multi_thread > time_of_sum_number_single_thread);

    natural_64_bit const  num_hits_of_some_computation_multi_thread = get_num_hits_of("some_computation");
    TEST_SUCCESS(num_hits_of_some_computation_multi_thread == 2ULL * num_hits_of_some_computation_single_thread);
    float_64_bit const  time_of_some_computation_multi_thread = (float_64_bit)get_num_hits_of("some_computation");
    TEST_SUCCESS(time_of_some_computation_multi_thread > time_of_some_computation_single_thread);

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
