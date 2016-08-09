#ifndef SIMULATOR_HPP_INCLUDED
#   define SIMULATOR_HPP_INCLUDED

#   include <qtgl/real_time_simulator.hpp>


struct simulator : public qtgl::real_time_simulator
{
    simulator();
    ~simulator();
    void next_round(float_64_bit const  miliseconds_from_previous_call,
                    bool const  is_this_pure_redraw_request);
};


#endif
