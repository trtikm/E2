#ifndef AI_AGENT_KIND_HPP_INCLUDED
#   define AI_AGENT_KIND_HPP_INCLUDED

namespace ai {


enum AGENT_KIND
{
    // Humanoid agent controlled by human operator (via keywoard)
    MOCK_HUMAN = 0,

    // Humanoid agent controlled by robotic AI (i.e not using NNET)
    ROBOT_HUMANOID = 1,
};


}

#endif