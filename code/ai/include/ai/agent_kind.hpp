#ifndef AI_AGENT_KIND_HPP_INCLUDED
#   define AI_AGENT_KIND_HPP_INCLUDED

namespace ai {


enum AGENT_KIND
{
    // An agent controlled by a human operator (via keywoard)
    MOCK = 0,

    // Humanoid agent controlled by robotic AI (i.e not using NNET)
    ROBOT_HUMANOID = 1,
};


}

#endif
