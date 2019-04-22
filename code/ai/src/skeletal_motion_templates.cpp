#include <ai/skeletal_motion_templates.hpp>
#include <ai/skeleton_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


skeletal_motion_templates::skeletal_motion_templates()
    : motions_map()
    , is_loaded(false)
{}


bool  skeletal_motion_templates::is_ready() const
{
    if (is_loaded == false)
    {
        for (auto const&  elem : motions_map)
            if (!elem.second.loaded_successfully())
                return false;
        is_loaded = true;
    }
    return true;
}


bool  skeletal_motion_templates::wait_till_loaded_is_finished() const
{
    if (is_loaded == false)
    {
        for (auto const& elem : motions_map)
        {
            while (!elem.second.is_load_finished()) {}
            if (!elem.second.loaded_successfully())
                return false;
        }
        is_loaded = true;
    }
    return true;
}


}
