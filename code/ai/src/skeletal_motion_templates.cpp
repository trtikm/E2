#include <ai/skeletal_motion_templates.hpp>
#include <ai/skeleton_utils.hpp>
#include <utility/hash_combine.hpp>
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


bool  operator==(
        skeletal_motion_templates::motion_template_cursor const&  left,
        skeletal_motion_templates::motion_template_cursor const&  right
        )
{
    return left.motion_name == right.motion_name && left.keyframe_index == right.keyframe_index;
}


bool  operator<(
        skeletal_motion_templates::motion_template_cursor const&  left,
        skeletal_motion_templates::motion_template_cursor const&  right
        )
{
    return left.motion_name < right.motion_name || (left.motion_name == right.motion_name && left.keyframe_index < right.keyframe_index);
}


std::size_t skeletal_motion_templates::motion_template_cursor::hasher::operator()(
        skeletal_motion_templates::motion_template_cursor const&  value
        ) const
{
    std::size_t seed = 0;
    ::hash_combine(seed, value.motion_name);
    ::hash_combine(seed, value.keyframe_index);
    return seed;
}


}
