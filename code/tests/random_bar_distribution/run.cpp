#include "./program_info.hpp"
#include "./program_options.hpp"
#include <utility/random.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <vector>


static natural_64_bit  compute_sum_of_bars(std::vector<natural_64_bit> const&  bars)
{
    natural_64_bit sum = 0UL;
    for (natural_64_bit  value : bars)
        sum += value;
    return sum;
}

static void  compute_expected_results(
        std::vector<natural_64_bit> const&  bars,
        natural_64_bit const  sum_of_bars,
        natural_64_bit const  num_trials,
        std::vector<float_64_bit>&  results
        )
{
    ASSUMPTION(results.empty());
    if (sum_of_bars == 0UL)
        results.resize(bars.size(),0UL);
    else
        for (natural_64_bit i = 0UL; i < bars.size(); ++i)
            results.push_back(
                static_cast<float_64_bit>(num_trials) * static_cast<float_64_bit>(bars.at(i)) / static_cast<float_64_bit>(sum_of_bars)
                );
    INVARIANT(results.size() == bars.size());
}

static void  compute_expected_results(
    std::vector<natural_64_bit> const&  bars,
    natural_64_bit const  num_trials,
    std::vector<float_64_bit>&  results
    )
{
    compute_expected_results(bars, compute_sum_of_bars(bars), num_trials, results);
}


struct  test_case
{
    test_case(
            std::vector<natural_64_bit> const&  bars,
            natural_64_bit const  num_trials_nultiplier,
            natural_32_bit const  seed
            );

    std::vector<natural_64_bit> const&  bars() const noexcept { return m_bars; }
    natural_64_bit  sum_of_bars() const noexcept { return m_sum_of_bars; }
    natural_64_bit  num_trials() const noexcept { return m_num_trials; }
    bar_random_distribution const&  distribution() const noexcept { return m_distribution; }
    natural_32_bit  seed() const noexcept { return m_seed; }
    std::vector<float_64_bit> const&  expected_results() const noexcept { return m_expected_results; }
    std::vector<natural_64_bit> const&  results() const noexcept { return m_results; }

    void  run();
    void  check_results(float_32_bit  acceptable_percentage_error_per_bar);

private:
    std::vector<natural_64_bit>  m_bars;
    natural_64_bit  m_sum_of_bars;
    natural_64_bit  m_num_trials;
    bar_random_distribution  m_distribution;
    natural_32_bit  m_seed;
    std::vector<float_64_bit>  m_expected_results;
    std::vector<natural_64_bit>  m_results;
};

test_case::test_case(
        std::vector<natural_64_bit> const&  bars,
        natural_64_bit const  num_trials_nultiplier,
        natural_32_bit const  seed
        )
    : m_bars(bars)
    , m_sum_of_bars(compute_sum_of_bars(m_bars))
    , m_num_trials(m_sum_of_bars * num_trials_nultiplier)
    , m_distribution(make_bar_random_distribution_from_count_bars(m_bars))
    , m_seed(seed)
    , m_expected_results()
    , m_results()
{
    ASSUMPTION(m_num_trials >= m_sum_of_bars);
    ASSUMPTION(m_seed > 0U);
}

void  test_case::run()
{
    compute_expected_results(m_bars, m_sum_of_bars, m_num_trials, m_expected_results);

    random_generator_for_natural_32_bit  generator;
    reset(generator,m_seed);
    m_results.resize(m_bars.size(),0UL);
    for (natural_64_bit i = 0UL; i < m_num_trials; ++i)
    {
        natural_32_bit const  index = get_random_bar_index(m_distribution,generator);
        INVARIANT(index < m_results.size());
        ++m_results.at(index);
    }
}

void  test_case::check_results(float_32_bit  acceptable_percentage_error_per_bar)
{
    acceptable_percentage_error_per_bar = acceptable_percentage_error_per_bar / 100.0f;
    ASSUMPTION(acceptable_percentage_error_per_bar >= 0.0f && acceptable_percentage_error_per_bar <= 1.0f);
    INVARIANT(m_results.size() == m_expected_results.size());
    natural_64_bit const  sum_of_bars = compute_sum_of_bars(m_results);
    TEST_SUCCESS(sum_of_bars == m_num_trials);
    float_64_bit max_bar_error = 0.0;
    natural_64_bit max_bar_error_index = m_results.size();
    for (natural_64_bit i = 0UL; i < m_results.size(); ++i)
    {
        float_64_bit const error =
            (m_expected_results.at(i) > 0.0 ?
                1.0 - m_results.at(i) / m_expected_results.at(i) :
                static_cast<float_64_bit>(m_results.at(i)));
        if (error > max_bar_error)
        {
            max_bar_error = error;
            max_bar_error_index = i;
        }

        if (!(m_results.at(i) >= m_expected_results.at(i) * (1.0f - acceptable_percentage_error_per_bar) &&
            m_results.at(i) <= m_expected_results.at(i) * (1.0f + acceptable_percentage_error_per_bar)))
        {
            int iii = 0;
        }

        TEST_SUCCESS(m_results.at(i) >= m_expected_results.at(i) * (1.0f - acceptable_percentage_error_per_bar) &&
                     m_results.at(i) <= m_expected_results.at(i) * (1.0f + acceptable_percentage_error_per_bar) );
    }

if (max_bar_error > acceptable_percentage_error_per_bar)
{
    int iii = 0;
}

}


void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();
    
    std::vector< std::vector<natural_64_bit> > const  bars_vector = {
        { 0UL, 0UL, 10UL, 0UL, 5UL, 15UL, 15UL, 8UL, 2UL, 0UL, 0UL, 50UL, 25UL, 12UL, 6UL, 3UL },
        { 0UL },
        { 0UL, 0UL,},
        { 100UL },
        { 100UL, 0UL },
        { 0UL, 100UL },
        { 10UL, 50UL },
        { 40UL, 20UL },
        { 80UL, 60UL, 0UL },
        { 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL },
    };

    std::vector<natural_64_bit> const  num_trials_multipliers = {
        //1UL, 2UL, 10UL,
        //100UL,
        1000UL
    };

    std::vector<natural_32_bit> const  seeds = {
        1U, 7U, 31U, 101U, 1023U, 12321U
    };

    for (auto const&  bars : bars_vector)
        for (auto const  trials_mult : num_trials_multipliers)
            for (auto const seed : seeds)
            {
                test_case  test(bars,trials_mult,seed);
                test.run();
                test.check_results(1.0f);
                TEST_PROGRESS_UPDATE();
            }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
