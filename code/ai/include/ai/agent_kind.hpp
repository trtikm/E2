#ifndef AI_AGENT_KIND_HPP_INCLUDED
#   define AI_AGENT_KIND_HPP_INCLUDED

namespace ai {


enum AGENT_KIND
{
    // An agent controlled by a human operator (via keywoard)
    MOCK = 0,

    // An agent controlled by a cortex choosing most sttationary actions.
    STATIONARY = 1,

    // An agent controlled by a cortex choosing actions randomly.
    RANDOM = 2,

    // An agent controlled by an experimental (prototyped) cortex.
    ROBOT = 3,
};


}

#endif
