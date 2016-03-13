#include "./program_info.hpp"
#include "./program_options.hpp"
#include <ode/euler.hpp>
#include <plot/plot.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
//#include <functional>
#include <vector>


float_64_bit dt()
{
    return 1.0;
}


float_64_bit ds(float_64_bit v)
{
    return v;
}

float_64_bit dv(float_64_bit a)
{
    return a;
}

float_64_bit da()
{
    return 0.0;
}

void test_euler_01()
{
    float_64_bit  h = 0.5;

    std::vector<float_64_bit>  t { 0.0 };
    std::vector<float_64_bit>  s { 0.0 };
    std::vector<float_64_bit>  v { 0.0 };
    std::vector<float_64_bit>  a { 1.0 };
    std::vector<float_64_bit>  exact { 0.0 };
    while (t.back() < 10.0)
    {
        float_64_bit  t1 = ode::euler(h,t.back(),&dt);
        float_64_bit  s1 = ode::euler(h,s.back(),&ds,v.back());
        float_64_bit  v1 = ode::euler(h,v.back(),&dv,a.back());
        float_64_bit  a1 = ode::euler(h,a.back(),&da);
        float_64_bit  exact1 = s.front() + v.front() * t1 + 0.5 * a.back() * t1 * t1;

        t.push_back(t1);
        s.push_back(s1);
        v.push_back(v1);
        a.push_back(a1);
        exact.push_back(exact1);
    }

    {
        plot::functions2d<float_64_bit> plt(2U);
        plt.title() = "test euler 01: t -> s,exact";
        plt.x_axis_label() = "t";
        plt.y_axis_label() = "s,exact";
        plt.x() = t;
        plt.f(0U) = s;
        plt.f_style(0U) = {plot::DRAW_STYLE_2D::POINTS_CROSS, plot::DRAW_STYLE_2D::LINES_SOLID};
        plt.f_legend(0U) = "s";
        plt.f(1U) = exact;
        plt.f_style(1U) = {plot::DRAW_STYLE_2D::POINTS_STAR, plot::DRAW_STYLE_2D::LINES_LONG_DASHES};
        plt.f_legend(1U) = "exact";

        plot::draw(plt,
                   "./ode_solvers/test_euler_01/from_t_to_s_exact.plt",
                   "./ode_solvers/test_euler_01/from_t_to_s_exact.svg"
                   );
    }

}


void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    test_euler_01();

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
