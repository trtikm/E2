#ifndef AI_ENV_STATE_HPP_INCLUDED
#   define AI_ENV_STATE_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <utility/basic_numeric_types.hpp>

namespace ai { namespace env {


struct  state
{
    state() : is_valid(false) {}
    explicit state(blackboard_const_ptr const  blackboard_ptr);
    bool  valid() const { return is_valid; }
private:
    bool is_valid;
public:
    
};


}}

#endif
