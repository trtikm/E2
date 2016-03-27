#include "./program_info.hpp"
#include "./program_options.hpp"
#include <ode/solvers.hpp>
#include <plot/plot.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/msgstream.hpp>
#include <vector>


void test_euler_01()
{
    struct local
    {
        static float_64_bit dt()
        {
            return 1.0;
        }


        static float_64_bit ds(float_64_bit v)
        {
            return v;
        }

        static float_64_bit dv(float_64_bit a)
        {
            return a;
        }

        static float_64_bit da()
        {
            return 0.0;
        }
    };

    float_64_bit  h = 0.5;

    std::vector<float_64_bit>  t { 0.0 };
    std::vector<float_64_bit>  s { 0.0 };
    std::vector<float_64_bit>  v { 0.0 };
    std::vector<float_64_bit>  a { 1.0 };
    std::vector<float_64_bit>  exact { 0.0 };
    while (t.back() < 10.0)
    {
        float_64_bit  t1 = ode::euler(h,t.back(),&local::dt);
        float_64_bit  s1 = ode::euler(h,s.back(),&local::ds,v.back());
        float_64_bit  v1 = ode::euler(h,v.back(),&local::dv,a.back());
        float_64_bit  a1 = ode::euler(h,a.back(),&local::da);
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
    TEST_PROGRESS_UPDATE();
}


void test_synapse_euler()
{
    struct local
    {
        static float_64_bit dt()
        {
            return 1.0;
        }

        static float_64_bit dV_m(float_64_bit V_m, float_64_bit g_syn,
                                 float_64_bit c_m, float_64_bit g_L, float_64_bit E_syn)
        {
            return -(g_L * V_m + g_syn * (V_m - E_syn)) / c_m;
        }

        static float_64_bit dg_syn(float_64_bit g_syn, float_64_bit tau_syn)
        {
            return -g_syn / tau_syn;
        }
    };

    float_64_bit const  h = 1.0 / 10.0;

    float_64_bit const  c_m = 1.0;
    float_64_bit const  g_L = 1.0;
    float_64_bit const  E_syn = 10.0;
    float_64_bit const  tau_syn = 1.0;

    std::vector<float_64_bit>  t { 0.0 };
    std::vector<float_64_bit>  V_m { 0.0 };
    std::vector<float_64_bit>  g_syn { 1.0 };
    while (t.back() < 10.0)
    {
        float_64_bit  t1 = ode::euler(h,t.back(),&local::dt);
        float_64_bit  V_m1 = ode::euler(h,V_m.back(),&local::dV_m,V_m.back(),g_syn.back(),c_m,g_L,E_syn);
        float_64_bit  g_syn1 = ode::euler(h,g_syn.back(),&local::dg_syn,g_syn.back(),tau_syn);

        t.push_back(t1);
        V_m.push_back(V_m1);
        g_syn.push_back(g_syn1);
    }

    {
        plot::functions2d<float_64_bit> plt(2U);
        plt.title() = "synapse euler: t -> Vm,gsyn";
        plt.x_axis_label() = "t";
        plt.y_axis_label() = "Vm, gsyn";
        plt.x() = t;
        plt.f(0U) = V_m;
        plt.f_style(0U) = {plot::DRAW_STYLE_2D::POINTS_CROSS, plot::DRAW_STYLE_2D::LINES_SOLID};
        plt.f_legend(0U) = "Vm";
        plt.f(1U) = g_syn;
        plt.f_style(1U) = {plot::DRAW_STYLE_2D::POINTS_STAR, plot::DRAW_STYLE_2D::LINES_LONG_DASHES};
        plt.f_legend(1U) = "gsyn";

        plot::draw(plt,
                   "./ode_solvers/test_synapse_euler/from_t_to_Vm_gsyn.plt",
                   "./ode_solvers/test_synapse_euler/from_t_to_Vm_gsyn.svg"
                   );
    }

    TEST_PROGRESS_UPDATE();
}

void test_synapse_inhibitory_exact()
{
    struct local
    {
        static float_64_bit dt()
        {
            return 1.0;
        }

        static float_64_bit dV_m(float_64_bit V_m, float_64_bit g_syn,
                                 float_64_bit c_m, float_64_bit g_L, float_64_bit E_syn)
        {
            return -(g_L * V_m + g_syn * (V_m - E_syn)) / c_m;
        }

        static float_64_bit dg_syn(float_64_bit g_syn, float_64_bit tau_syn)
        {
            return -g_syn / tau_syn;
        }
    };

    float_64_bit const  h = 1.0 / 10.0;

    float_64_bit const  c_m = 50.0;
    float_64_bit const  g_L = 10.0;
    float_64_bit const  E_syn = -75.0;
    float_64_bit const  tau_syn = 5.0;
    float_64_bit const  g_syn_fast = 40.0;

    std::vector<float_64_bit>  t { 0.0 };
    std::vector<float_64_bit>  V_m { 0.0 };
    std::vector<float_64_bit>  g_syn { g_syn_fast };
    std::vector<float_64_bit>  exact { 0.0 };
    while (t.back() < 10.0)
    {
        float_64_bit  t1 = ode::euler(h,t.back(),&local::dt);
        float_64_bit  V_m1 = ode::euler(h,V_m.back(),&local::dV_m,V_m.back(),g_syn.back(),c_m,g_L,E_syn);
        float_64_bit  g_syn1 = ode::euler(h,g_syn.back(),&local::dg_syn,g_syn.back(),tau_syn);
        float_64_bit  exact1 = g_syn_fast * std::exp(-t1 / tau_syn);

        t.push_back(t1);
        V_m.push_back(V_m1);
        g_syn.push_back(g_syn1);
        exact.push_back(exact1);
    }

    {
        plot::functions2d<float_64_bit> plt(3U);
        plt.title() = "synapse inhibitory exact: t -> Vm,gsyn,exact";
        plt.x_axis_label() = "t";
        plt.y_axis_label() = "Vm, gsyn, exact";
        plt.x() = t;
        plt.f(0U) = V_m;
        plt.f_style(0U) = {plot::DRAW_STYLE_2D::POINTS_CROSS, plot::DRAW_STYLE_2D::LINES_SOLID};
        plt.f_legend(0U) = "Vm";
        plt.f(1U) = g_syn;
        plt.f_style(1U) = {plot::DRAW_STYLE_2D::POINTS_STAR, plot::DRAW_STYLE_2D::LINES_LONG_DASHES};
        plt.f_legend(1U) = "gsyn";
        plt.f(2U) = exact;
        plt.f_style(2U) = {plot::DRAW_STYLE_2D::POINTS_EMPTY_BOX, plot::DRAW_STYLE_2D::LINES_DASH_DASHES};
        plt.f_legend(2U) = "exact";

        plot::draw(plt,
                   "./ode_solvers/test_synapse_inhibitory_exact/from_t_to_Vm_gsyn_exact.plt",
                   "./ode_solvers/test_synapse_inhibitory_exact/from_t_to_Vm_gsyn_exact.svg"
                   );
    }

    TEST_PROGRESS_UPDATE();
}

void test_synapse_excitatory_exact()
{
    struct local
    {
        static float_64_bit dt()
        {
            return 1.0;
        }

        static float_64_bit dV_m(float_64_bit V_m, float_64_bit g_syn,
                                 float_64_bit c_m, float_64_bit g_L, float_64_bit E_syn)
        {
            return -(g_L * V_m + g_syn * (V_m - E_syn)) / c_m;
        }

        static float_64_bit dg_syn(float_64_bit g_syn, float_64_bit tau_syn)
        {
            return -g_syn / tau_syn;
        }
    };

    float_64_bit const  h = 1.0 / 100.0;

    float_64_bit const  c_m = 5.0;
    float_64_bit const  g_L = 10.0;
    float_64_bit const  E_syn = 10.0;
    float_64_bit const  tau_raise = 0.09;
    float_64_bit const  tau_decay = 1.5;
    float_64_bit const  g_syn_ampa = 72.0;
    float_64_bit const  N = 1.273;

    std::vector<float_64_bit>  t { 0.0 };
    std::vector<float_64_bit>  V_m { 0.0 };
    std::vector<float_64_bit>  g_syn { g_syn_ampa *N };
    std::vector<float_64_bit>  exact { 0.0 };
    while (t.back() < 10.0)
    {
        float_64_bit  t1 = ode::euler(h,t.back(),&local::dt);
        float_64_bit  V_m1 = ode::euler(h,V_m.back(),&local::dV_m,V_m.back(),g_syn.back(),c_m,g_L,E_syn);
        float_64_bit  g_syn1 = ode::euler(h,g_syn.back(),&local::dg_syn,g_syn.back(),tau_decay);
        float_64_bit  exact1 = g_syn_ampa * N * (std::exp(-t1 / tau_decay) - std::exp(-t1 / tau_raise));

        t.push_back(t1);
        V_m.push_back(V_m1);
        g_syn.push_back(g_syn1);
        exact.push_back(exact1);
    }

    {
        plot::functions2d<float_64_bit> plt(3U);
        plt.title() = "synapse excitatory exact: t -> Vm,gsyn,exact";
        plt.x_axis_label() = "t";
        plt.y_axis_label() = "Vm, gsyn, exact";
        plt.x() = t;
        plt.f(0U) = V_m;
        plt.f_style(0U) = {plot::DRAW_STYLE_2D::POINTS_CROSS, plot::DRAW_STYLE_2D::LINES_SOLID};
        plt.f_legend(0U) = "Vm";
        plt.f(1U) = g_syn;
        plt.f_style(1U) = {plot::DRAW_STYLE_2D::POINTS_STAR, plot::DRAW_STYLE_2D::LINES_LONG_DASHES};
        plt.f_legend(1U) = "gsyn";
        plt.f(2U) = exact;
        plt.f_style(2U) = {plot::DRAW_STYLE_2D::POINTS_EMPTY_BOX, plot::DRAW_STYLE_2D::LINES_DASH_DASHES};
        plt.f_legend(2U) = "exact";

        plot::draw(plt,
                   "./ode_solvers/test_synapse_excitatory_exact/from_t_to_Vm_gsyn_exact.plt",
                   "./ode_solvers/test_synapse_excitatory_exact/from_t_to_Vm_gsyn_exact.svg"
                   );
    }

    TEST_PROGRESS_UPDATE();
}

void test_neuron_hodgkin_huxley_euler()
{
    struct local
    {
        static float_64_bit dt()
        {
            return 1.0;
        }

        static float_64_bit dV(float_64_bit V, float_64_bit n, float_64_bit m, float_64_bit h,
                               float_64_bit g_K, float_64_bit g_Na, float_64_bit g_L,
                               float_64_bit E_K, float_64_bit E_Na, float_64_bit E_L,
                               float_64_bit C, float_64_bit I)
        {
            return (- g_K * std::pow(n,4.0) * (V - E_K)
                    - g_Na * std::pow(m,3.0) * h * (V - E_Na)
                    - g_L * (V - E_L)
                    + I) / C;
        }

        static float_64_bit dn(float_64_bit n, float_64_bit V)
        {
            return ((0.1 - 0.01 * V) / (std::exp(1.0 - 0.1 * V) - 1.0)) * (1.0 - n) -
                   (0.125 * std::exp(-V / 80.0)) * n;
        }

        static float_64_bit dm(float_64_bit m, float_64_bit V)
        {
            return ((2.5 - 0.1 * V) / (std::exp(2.5 - 0.1 * V) - 1.0)) * (1.0 - m) -
                   (4.0 * std::exp(-V / 18.0)) * m;
        }

        static float_64_bit dh(float_64_bit h, float_64_bit V)
        {
            return (0.07 * std::exp(-V / 20.0)) * (1.0 - h) -
                   (1.0 / (std::exp(3.0 - 0.1 * V) + 1.0)) * h;
        }
    };

    float_64_bit const  hh = 1.0 / 100.0;

    float_64_bit const  g_K = 36.0;
    float_64_bit const  g_Na = 120.0;
    float_64_bit const  g_L = 0.3;
    float_64_bit const  E_K = -12.0;
    float_64_bit const  E_Na = 115.0;
    float_64_bit const  E_L = 10.613;
    float_64_bit const  C = 1.0;

    float_64_bit const  I = 10.0;

    std::vector<float_64_bit>  t { 0.0 };
    std::vector<float_64_bit>  V { 0.0 };
    std::vector<float_64_bit>  n { 0.317729 };
    std::vector<float_64_bit>  m { 0.052955 };
    std::vector<float_64_bit>  h { 0.595945 };
    while (t.back() < 50.0)
    {
        float_64_bit  t1 = ode::euler(hh,t.back(),&local::dt);
        float_64_bit  V1 = ode::euler(hh,V.back(),&local::dV,V.back(),n.back(),m.back(),h.back(),
                                        g_K,g_Na,g_L,E_K,E_Na,E_L,C,I);
        float_64_bit  n1 = ode::euler(hh,n.back(),&local::dn,n.back(),V.back());
        float_64_bit  m1 = ode::euler(hh,m.back(),&local::dm,m.back(),V.back());
        float_64_bit  h1 = ode::euler(hh,h.back(),&local::dh,h.back(),V.back());

        t.push_back(t1);
        V.push_back(V1);
        n.push_back(n1);
        m.push_back(m1);
        h.push_back(h1);
    }

    {
        plot::functions2d<float_64_bit> plt(1U);
        plt.title() = "neuron hodgkin-huxley euler: t -> V";
        plt.x_axis_label() = "t";
        plt.y_axis_label() = "V";
        plt.x() = t;
        plt.f(0U) = V;
        plt.f_style(0U) = {/*plot::DRAW_STYLE_2D::POINTS_CROSS, */plot::DRAW_STYLE_2D::LINES_SOLID};
        plt.f_legend(0U) = "V";

        plot::draw(plt,
                   "./ode_solvers/test_neuron_hodgkin_huxley_euler/from_t_to_V.plt",
                   "./ode_solvers/test_neuron_hodgkin_huxley_euler/from_t_to_V.svg"
                   );
    }
    TEST_PROGRESS_UPDATE();
}

//void test_neuron_wilson_euler()
//{
//    struct local
//    {
//        static float_64_bit dt()
//        {
//            return 1.0;
//        }

//        static float_64_bit dV(float_64_bit V, float_64_bit R, float_64_bit T, float_64_bit H,
//                               float_64_bit g_K, float_64_bit g_T, float_64_bit g_H,
//                               float_64_bit E_K, float_64_bit E_Na, float_64_bit E_T, float_64_bit E_H,
//                               float_64_bit C, float_64_bit I)
//        {
//            return (- (17.8 + 0.476 * V + 0.00338 * V * V) * (V - E_Na)
//                    - g_K * R * (V - E_K)
//                    - g_T * T * (V - E_T)
//                    - g_H * H * (V - E_H)
//                    + I) / C;
//        }

//        static float_64_bit dR(float_64_bit R, float_64_bit V, float_64_bit tau_R)
//        {
//            return -(R - (1.24 + 0.037 * V + 0.00032 * V * V)) / tau_R;
//        }

//        static float_64_bit dT(float_64_bit T, float_64_bit V, float_64_bit tau_T)
//        {
//            return -(T - (4.205 + 0.116 * V + 0.0008 * V * V)) / tau_T;
//        }

//        static float_64_bit dH(float_64_bit H, float_64_bit T, float_64_bit tau_H)
//        {
//            return -(H - 3.0 * T) / tau_H;
//        }
//    };

//    std::vector<float_64_bit> const  g_K =   {   0.26,   26.0,    26.0  }; // [default]
//    std::vector<float_64_bit> const  g_T =   {   0.0225,  0.1,          };
//    std::vector<float_64_bit> const  g_H =   {   0.095,   5.0,          };
//    std::vector<float_64_bit> const  E_K =   { -95.0,   -95.0,   -95.0  }; // mV [default]
//    std::vector<float_64_bit> const  E_Na =  {  50.0,    50.0,    50.0  }; // mV [default]
//    std::vector<float_64_bit> const  E_T =   { 120.0,   120.0,   120.0  }; // mV [default]
//    std::vector<float_64_bit> const  E_H =   { -95.0,                   };
//    std::vector<float_64_bit> const  tau_R = {   4.2,     4.2,    45.0  }; // ms [default]
//    std::vector<float_64_bit> const  tau_T = {  14.0,    14.0,    14.0  }; // ms [default]
//    std::vector<float_64_bit> const  tau_H = {  45.0,                   };
//    std::vector<float_64_bit> const  C =     { 100.0,   100.0,   100.0  }; // uF/cm [default]
//    std::vector<float_64_bit> const  I =     {   0.0,     1.0,     1.0  }; // nA [default]

//    std::vector<std::string> const  names = { "fast-spiking","regular-spiking","bursting" };

//    float_64_bit const  h = 1.0 / 100.0;

//    for (natural_32_bit  i = 0U; i < 1U; ++i)
//    {

//        std::vector<float_64_bit>  t { 0.0 };
//        std::vector<float_64_bit>  V {-80.0 };
//        std::vector<float_64_bit>  R { 0.0 };
//        std::vector<float_64_bit>  T { 0.0 };
//        std::vector<float_64_bit>  H { 0.0 };
//        while (t.back() < 200.0)
//        {
//            float_64_bit  t1 = ode::euler(h,t.back(),&local::dt);
//            float_64_bit  V1 = ode::euler(h,V.back(),&local::dV,V.back(),R.back(),T.back(),H.back(),
//                                          g_K.at(i),g_T.at(i),g_H.at(i),
//                                          E_K.at(i),E_Na.at(i),E_T.at(i),E_H.at(i),
//                                          C.at(i),I.at(i));
//            float_64_bit  R1 = ode::euler(h,R.back(),&local::dR,R.back(),V.back(),tau_R.at(i));
//            float_64_bit  T1 = ode::euler(h,T.back(),&local::dT,T.back(),V.back(),tau_T.at(i));
//            float_64_bit  H1 = ode::euler(h,H.back(),&local::dH,H.back(),T.back(),tau_H.at(i));

//            t.push_back(t1);
//            V.push_back(V1);
//            R.push_back(R1);
//            T.push_back(T1);
//            H.push_back(H1);
//        }

//        {
//            plot::functions2d<float_64_bit> plt(1U);
//            plt.title() = msgstream() << "neuron Wilson euler [" << names.at(i) << "]: t -> V";
//            plt.x_axis_label() = "t";
//            plt.y_axis_label() = "V";
//            plt.x() = t;
//            plt.f(0U) = V;
//            plt.f_style(0U) = {/*plot::DRAW_STYLE_2D::POINTS_CROSS, */plot::DRAW_STYLE_2D::LINES_SOLID};
//            plt.f_legend(0U) = "V";

//            plot::draw(plt,
//                       (msgstream() << "./ode_solvers/test_neuron_wilson_euler/from_t_to_V_" << names.at(i) << ".plt").get(),
//                       (msgstream() << "./ode_solvers/test_neuron_wilson_euler/from_t_to_V_" << names.at(i) << ".svg").get()
//                       );
//        }
//    }
//    TEST_PROGRESS_UPDATE();
//}

void test_neuron_wilson_euler()
{
    struct local
    {
        static float_64_bit dt()
        {
            return 1.0;
        }

        static float_64_bit dV(float_64_bit V, float_64_bit R, float_64_bit T, float_64_bit H,
                               float_64_bit g_K, float_64_bit g_T, float_64_bit g_H,
                               float_64_bit E_K, float_64_bit E_Na, float_64_bit E_T, float_64_bit E_H,
                               float_64_bit C, float_64_bit I)
        {
            return (- (17.8 + 47.6 * V + 33.8 * V * V) * (V - E_Na)
                    - g_K * R * (V - E_K)
                    - g_T * T * (V - E_T)
                    - g_H * H * (V - E_H)
                    + I) / C;
        }

        static float_64_bit dR(float_64_bit R, float_64_bit V, float_64_bit tau_R)
        {
            return -(R - (1.24 + 3.7 * V + 3.2 * V * V)) / tau_R;
        }

        static float_64_bit dT(float_64_bit T, float_64_bit V, float_64_bit tau_T)
        {
            return -(T - (4.205 + 11.6 * V + 8.0 * V * V)) / tau_T;
        }

        static float_64_bit dH(float_64_bit H, float_64_bit T, float_64_bit tau_H)
        {
            return -(H - 3.0 * T) / tau_H;
        }
    };

    std::vector<float_64_bit> const  g_K =   {  26.0,    26.0,   26.0  }; // [default]
    std::vector<float_64_bit> const  g_T =   {   0.25,    0.1,    2.25 };
    std::vector<float_64_bit> const  g_H =   {   0.0,     5.0,    9.5  };
    std::vector<float_64_bit> const  E_K =   {  -0.95,   -0.95,  -0.95 }; // mV [default]
    std::vector<float_64_bit> const  E_Na =  {   0.5,     0.5,    0.5  }; // mV [default]
    std::vector<float_64_bit> const  E_T =   {   1.2,     1.2,    1.2  }; // mV [default]
    std::vector<float_64_bit> const  E_H =   {  -0.95,   -0.95,  -0.95 };
    std::vector<float_64_bit> const  tau_R = {   1.5,     4.2,    4.2  }; // ms [default]
    std::vector<float_64_bit> const  tau_T = {  14.0,    14.0,   14.0  }; // ms [default]
    std::vector<float_64_bit> const  tau_H = {  45.0,    45.0,   45.0  };
    std::vector<float_64_bit> const  C =     {   1.0,     1.0,    1.0  }; // uF/cm [default]
    std::vector<float_64_bit> const  I =     {   1.0,     1.0,    1.0  }; // nA [default]

    std::vector<std::string> const  names = { "fast-spiking","regular-spiking","bursting" };

    float_64_bit const  h = 1.0 / 100.0;

    for (natural_32_bit  i = 0U; i < 3U; ++i)
    {

        std::vector<float_64_bit>  t { 0.0 };
        std::vector<float_64_bit>  V {-1.0 };
        std::vector<float_64_bit>  R { 0.0 };
        std::vector<float_64_bit>  T { 0.0 };
        std::vector<float_64_bit>  H { 0.0 };
        while (t.back() < 200.0)
        {
            float_64_bit  t1 = ode::euler(h,t.back(),&local::dt);
            float_64_bit  V1 = ode::euler(h,V.back(),&local::dV,V.back(),R.back(),T.back(),H.back(),
                                          g_K.at(i),g_T.at(i),g_H.at(i),
                                          E_K.at(i),E_Na.at(i),E_T.at(i),E_H.at(i),
                                          C.at(i),I.at(i));
            float_64_bit  R1 = ode::euler(h,R.back(),&local::dR,R.back(),V.back(),tau_R.at(i));
            float_64_bit  T1 = ode::euler(h,T.back(),&local::dT,T.back(),V.back(),tau_T.at(i));
            float_64_bit  H1 = ode::euler(h,H.back(),&local::dH,H.back(),T.back(),tau_H.at(i));

            t.push_back(t1);
            V.push_back(V1);
            R.push_back(R1);
            T.push_back(T1);
            H.push_back(H1);
        }

        {
            plot::functions2d<float_64_bit> plt(1U);
            plt.title() = msgstream() << "neuron Wilson euler [" << names.at(i) << "]: t -> V";
            plt.x_axis_label() = "t";
            plt.y_axis_label() = "V";
            plt.x() = t;
            plt.f(0U) = V; for (auto& elem : plt.f(0U)) elem *= 100.0;
            plt.f_style(0U) = {/*plot::DRAW_STYLE_2D::POINTS_CROSS, */plot::DRAW_STYLE_2D::LINES_SOLID};
            plt.f_legend(0U) = "V";

            plot::draw(plt,
                       (msgstream() << "./ode_solvers/test_neuron_wilson_euler/from_t_to_V_" << names.at(i) << ".plt").get(),
                       (msgstream() << "./ode_solvers/test_neuron_wilson_euler/from_t_to_V_" << names.at(i) << ".svg").get()
                       );
        }

        TEST_PROGRESS_UPDATE();
    }
}

void test_neuron_leaky_integrate_and_fire_euler()
{
    struct local
    {
        static float_64_bit dt()
        {
            return 1.0;
        }

        static float_64_bit dV(float_64_bit V, float_64_bit E_L, float_64_bit tau_m, float_64_bit RI)
        {
            return (RI - (V - E_L)) / tau_m;
        }
    };

    float_64_bit const  h = 1.0 / 10.0;

    float_64_bit const  tau_m = 10.0; // ms
    float_64_bit const  E_L = -65.0; // mV
    float_64_bit const  firing_treshold = -55.0; // mV

    std::vector<float_64_bit>  t { 0.0 };
    std::vector<float_64_bit>  V { E_L };
    while (t.back() < 100.0)
    {
        float_64_bit const  RI = t.back() < 10.0 || t.back() > 60.0 ? 0.0 : 12.0;

        float_64_bit const  t1 = ode::euler(h,t.back(),&local::dt);
        float_64_bit const  V1 = V.back() < firing_treshold ?
                                        ode::euler(h,V.back(),&local::dV,V.back(),E_L,tau_m,RI) :
                                        E_L;

        t.push_back(t1);
        V.push_back(V1);
    }

    {
        plot::functions2d<float_64_bit> plt(1U);
        plt.title() = "neuron leaky integrate and fire euler: t -> V";
        plt.x_axis_label() = "t";
        plt.y_axis_label() = "V";
        plt.x() = t;
        plt.f(0U) = V;
        plt.f_style(0U) = {/*plot::DRAW_STYLE_2D::POINTS_CROSS, */plot::DRAW_STYLE_2D::LINES_SOLID};
        plt.f_legend(0U) = "V";

        plot::draw(plt,
                   "./ode_solvers/test_neuron_leaky_integrate_and_fire_euler/from_t_to_V.plt",
                   "./ode_solvers/test_neuron_leaky_integrate_and_fire_euler/from_t_to_V.svg"
                   );
    }
    TEST_PROGRESS_UPDATE();
}

//void test_neuron_izhikevich_euler()
//{
//    struct local
//    {
//        static float_64_bit dt()
//        {
//            return 1.0;
//        }

//        static float_64_bit dV(float_64_bit V, float_64_bit U, float_64_bit I)
//        {
//            return 0.04 * V * V + 5 * V + 140 - U + I;
//        }

//        static float_64_bit dU(float_64_bit U, float_64_bit V, float_64_bit a, float_64_bit b)
//        {
//            return a * (b * V - U);
//        }
//    };

//    std::vector<float_64_bit> const  a = {   0.02,   0.1,   0.02,   0.02 };
//    std::vector<float_64_bit> const  b = {   0.2,    0.2,   0.2,    0.2  };
//    std::vector<float_64_bit> const  c = { -65.0,  -65.0, -65.0,  -50.0  };
//    std::vector<float_64_bit> const  d = {   2.0,    2.0,   8.0,    2.0  };

//    std::vector<float_64_bit> const  V0 = {-70.0,  -70.0, -70.0,  -70.0  };
//    std::vector<float_64_bit> const  U0 = {-14.0,  -14.0, -14.0,  -14.0  };

//    std::vector<std::string> const  names = { "default","fast-spiking","regular-spiking","bursting" };

//    float_64_bit const  h = 1.0 / 1.0;

//    for (natural_32_bit  i = 0U; i < names.size(); ++i)
//    {
//        std::vector<float_64_bit>  t { 0.0 };
//        std::vector<float_64_bit>  V { V0.at(i) }; //c.at(i) };
//        std::vector<float_64_bit>  U { U0.at(i) }; //b.at(i)*c.at(i) };
//        while (t.back() < 200.0)
//        {
//            float_64_bit const  I = t.back() < 10.0 || t.back() > 150.0 ? 0.0 : 10.0;

//            float_64_bit const  t1 = ode::euler(h,t.back(),&local::dt);
//            float_64_bit const  V1 = V.back() < 30.0 ? ode::euler(h,V.back(),&local::dV,V.back(),U.back(),I) :
//                                                       c.at(i);
//            float_64_bit const  U1 = V.back() < 30.0 ? ode::euler(h,U.back(),&local::dU,U.back(),V.back(),a.at(i),b.at(i)) :
//                                                       U.back() + d.at(i);

//            t.push_back(t1);
//            V.push_back(V1);
//            U.push_back(U1);
//        }

//        {
//            plot::functions2d<float_64_bit> plt(1U);
//            plt.title() = msgstream() << "neuron izhikevich euler [" << names.at(i) << "]: t -> V";
//            plt.x_axis_label() = "t";
//            plt.y_axis_label() = "V";
//            plt.x() = t;
//            plt.f(0U) = V;
//            plt.f_style(0U) = {/*plot::DRAW_STYLE_2D::POINTS_CROSS, */plot::DRAW_STYLE_2D::LINES_SOLID};
//            plt.f_legend(0U) = "V";

//            plot::draw(plt,
//                       (msgstream() << "./ode_solvers/test_neuron_izhikevich_euler/from_t_to_V_"
//                                    << names.at(i) << ".plt").get(),
//                       (msgstream() << "./ode_solvers/test_neuron_izhikevich_euler/from_t_to_V_"
//                                    << names.at(i) << ".svg").get()
//                       );
//        }

//        TEST_PROGRESS_UPDATE();
//    }
//}

void test_neuron_izhikevich_euler()
{
    enum
    {
        t = 0U,
        V = 1U,
        U = 2U
    };

    struct local
    {
        static float_64_bit dt(std::vector<float_64_bit> const&)
        {
            return 1.0;
        }

        static float_64_bit dV(std::vector<float_64_bit> const&  vars, float_64_bit const  I)
        {
            return 0.04 * vars[V] * vars[V] + 5 * vars[V] + 140 - vars[U] + I;
        }

        static float_64_bit dU(std::vector<float_64_bit> const&  vars, float_64_bit a, float_64_bit b)
        {
            return a * (b * vars[V] - vars[U]);
        }
    };

    std::vector<float_64_bit> const  a = {   0.02,   0.1,   0.02,   0.02 };
    std::vector<float_64_bit> const  b = {   0.2,    0.2,   0.2,    0.2  };
    std::vector<float_64_bit> const  c = { -65.0,  -65.0, -65.0,  -50.0  };
    std::vector<float_64_bit> const  d = {   2.0,    2.0,   8.0,    2.0  };

    std::vector<float_64_bit> const  V0 = {-70.0,  -70.0, -70.0,  -70.0  };
    std::vector<float_64_bit> const  U0 = {-14.0,  -14.0, -14.0,  -14.0  };

    std::vector< std::pair<ode::solver_function_type1,std::string> > const  solvers = {
        { &ode::euler, "euler" },
        { &ode::midpoint, "midpoint" },
        { &ode::runge_kutta_4, "runge-kutta-4" },
    };

    std::vector<std::string> const  names = { "default","fast-spiking","regular-spiking","bursting" };

    float_64_bit const  h = 1.0 / 10.0;

    for (std::pair<ode::solver_function_type1,std::string>  solver : solvers)
        for (natural_32_bit  i = 0U; i < names.size(); ++i)
        {
            std::vector<float_64_bit>  vars(3U);
            vars[t] = 0.0;
            vars[V] = V0.back();
            vars[U] = U0.back();

            std::vector<float_64_bit>  ts { vars[t] };
            std::vector<float_64_bit>  Vs { vars[V] };
            std::vector<float_64_bit>  Us { vars[U] };
            while (vars[t] < 200.0)
            {
                float_64_bit const  I = vars[t] < 10.0 || vars[t] > 150.0 ? 0.0 : 10.0;

                std::vector<ode::derivation_function_type> const  system {
                        &local::dt,
                        vars[V] < 30.0 ? std::bind(&local::dV,std::placeholders::_1,I) :
                                         ode::constant_derivative((c.at(i) - vars[V]) / h),
                        vars[V] < 30.0 ? std::bind(&local::dU,std::placeholders::_1,a.at(i),b.at(i)) :
                                         ode::constant_derivative(d.at(i) / h)
                        };

                solver.first(h,system,vars);

                ts.push_back(vars[t]);
                Vs.push_back(vars[V]);
                Us.push_back(vars[U]);
            }

            {
                plot::functions2d<float_64_bit> plt(1U);
                plt.title() = msgstream() << "neuron izhikevich " << solver.second << " [" << names.at(i) << "]: t -> V";
                plt.x_axis_label() = "t";
                plt.y_axis_label() = "V";
                plt.x() = ts;
                plt.f(0U) = Vs;
                plt.f_style(0U) = {/*plot::DRAW_STYLE_2D::POINTS_CROSS, */plot::DRAW_STYLE_2D::LINES_SOLID};
                plt.f_legend(0U) = "V";

                plot::draw(plt,
                           (msgstream() << "./ode_solvers/test_neuron_izhikevich_" << solver.second << "/from_t_to_V_"
                                        << names.at(i) << ".plt").get(),
                           (msgstream() << "./ode_solvers/test_neuron_izhikevich_" << solver.second << "/from_t_to_V_"
                                        << names.at(i) << ".svg").get()
                           );
            }

            TEST_PROGRESS_UPDATE();
        }
}

void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    test_euler_01();
    test_synapse_euler();
    test_synapse_inhibitory_exact();
    test_synapse_excitatory_exact();
    test_neuron_hodgkin_huxley_euler();
    test_neuron_wilson_euler();
    test_neuron_leaky_integrate_and_fire_euler();
    test_neuron_izhikevich_euler();

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
