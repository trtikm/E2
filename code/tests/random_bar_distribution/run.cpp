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


template<typename T>
static T  compute_sum_of_bars(std::vector<T> const&  bars)
{
    T sum = 0;
    for (T  value : bars)
        sum += value;
    return sum;
}

template<typename T>
static void  compute_expected_results(
        std::vector<T> const&  bars,
        T const  sum_of_bars,
        natural_64_bit const  num_trials,
        std::vector<float_64_bit>&  results
        )
{
    ASSUMPTION(results.empty());
    ASSUMPTION(sum_of_bars > 0UL);
    for (natural_64_bit i = 0UL; i < bars.size(); ++i)
        results.push_back(
            static_cast<float_64_bit>(num_trials) * static_cast<float_64_bit>(bars.at(i)) / static_cast<float_64_bit>(sum_of_bars)
            );
    INVARIANT(results.size() == bars.size());
}

static  bar_random_distribution  make_distribution(std::vector<natural_64_bit> const&  bars)
{
    return make_bar_random_distribution_from_count_bars(bars);
}

static  bar_random_distribution  make_distribution(std::vector<float_32_bit> const&  bars)
{
    return make_bar_random_distribution_from_size_bars(bars);
}


template<typename T>
struct  test_case
{
    using  bar_element_type = T;

    test_case(
            std::vector<bar_element_type> const&  bars,
            natural_64_bit const  num_trials,
            natural_32_bit const  seed
            );

    std::vector<bar_element_type> const&  bars() const noexcept { return m_bars; }
    bar_element_type  sum_of_bars() const noexcept { return m_sum_of_bars; }
    natural_64_bit  num_trials() const noexcept { return m_num_trials; }
    bar_random_distribution const&  distribution() const noexcept { return m_distribution; }
    natural_32_bit  seed() const noexcept { return m_seed; }
    std::vector<float_64_bit> const&  expected_results() const noexcept { return m_expected_results; }
    std::vector<bar_element_type> const&  results() const noexcept { return m_results; }

    void  run();
    void  check_results(float_32_bit  acceptable_percentage_error_per_bar);

private:
    std::vector<bar_element_type>  m_bars;
    bar_element_type  m_sum_of_bars;
    natural_64_bit  m_num_trials;
    bar_random_distribution  m_distribution;
    natural_32_bit  m_seed;
    std::vector<float_64_bit>  m_expected_results;
    std::vector<bar_element_type>  m_results;
};

template<typename T>
test_case<T>::test_case(
        std::vector<bar_element_type> const&  bars,
        natural_64_bit const  num_trials,
        natural_32_bit const  seed
        )
    : m_bars(bars)
    , m_sum_of_bars(compute_sum_of_bars(m_bars))
    , m_num_trials(num_trials)
    , m_distribution(make_distribution(m_bars))
    , m_seed(seed)
    , m_expected_results()
    , m_results()
{
    ASSUMPTION(m_num_trials >= m_sum_of_bars);
    ASSUMPTION(m_seed > 0U);
}

template<typename T>
void  test_case<T>::run()
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

template<typename T>
void  test_case<T>::check_results(float_32_bit  acceptable_percentage_error_per_bar)
{
    acceptable_percentage_error_per_bar = acceptable_percentage_error_per_bar / 100.0f;
    ASSUMPTION(acceptable_percentage_error_per_bar >= 0.0f && acceptable_percentage_error_per_bar <= 1.0f);
    INVARIANT(m_results.size() == m_expected_results.size());
    bar_element_type const  sum_of_bars = compute_sum_of_bars(m_results);
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

//if (!(m_results.at(i) >= m_expected_results.at(i) * (1.0f - acceptable_percentage_error_per_bar) &&
//    m_results.at(i) <= m_expected_results.at(i) * (1.0f + acceptable_percentage_error_per_bar)))
//{
//    int iii = 0;
//}

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
    
    std::vector< std::vector<natural_64_bit> > const  count_bars_vector = {
        { 0UL, 0UL, 10UL, 0UL, 5UL, 15UL, 15UL, 8UL, 2UL, 0UL, 0UL, 50UL, 25UL, 12UL, 6UL, 3UL },
        { 1UL },
        { 1UL, 1UL },
        { 100UL },
        { 100UL, 0UL },
        { 0UL, 100UL },
        { 10UL, 50UL },
        { 40UL, 20UL },
        { 80UL, 60UL, 0UL },
        { 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL, 10UL },
    };

    std::vector< std::vector<float_32_bit> > const  size_bars_vector = {
        //{ 0.0f, 0.0f, 5.3f, 6.123f, 13.2f, 6.0f, 3.333f, 1.4f, 0.0f, 0.0f, 0.0f, 12345.42f, 1234.045f, 123.23f, 12.441f, 1.001f, 0.0f },
        { 0.0f, 0.0f, 5.3f, 6.123f, 13.2f, 6.0f, 3.333f, 1.4f, 0.0f, 0.0f, 0.0f, 23.454f, 12.045f, 7.331f, 4.441f, 1.001f, 0.0f },
        { 0.001f },
        { 100.012f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.25f, 1.0f, 0.0f },
        { 123.123f, 123.123f, 123.123f, 123.123f, 123.123f, 123.123f, 123.123f, 123.123f, 123.123f },
    };

    std::vector< std::pair<natural_64_bit,float_32_bit> > const  trials_errors_vector = {
        { 500000UL, 2.50f }, { 1000000UL, 1.55f }
    };

    std::vector<natural_32_bit> const  seeds_vector = {
        1U, 7U, 31U, 101U, 1023U, 12321U
    };

    for (auto const&  bars : count_bars_vector)
        for (auto const  trial_error : trials_errors_vector)
            for (auto const seed : seeds_vector)
            {
                test_case<natural_64_bit>  test(bars, trial_error.first,seed);
                test.run();
                test.check_results(trial_error.second);

                TEST_PROGRESS_UPDATE();
            }

    for (auto const& bars : size_bars_vector)
        for (auto const trial_error : trials_errors_vector)
            for (auto const seed : seeds_vector)
            {
                test_case<float_32_bit>  test(bars, trial_error.first, seed);
                test.run();
                test.check_results(trial_error.second);

                TEST_PROGRESS_UPDATE();
            }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
