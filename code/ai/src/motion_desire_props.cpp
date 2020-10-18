#include <ai/motion_desire_props.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai {


void  as_vector(motion_desire_props const&  props, std::vector<float_32_bit>&  output)
{
    output.push_back(props.move.forward);
    output.push_back(props.move.left);
    output.push_back(props.move.up);
    output.push_back(props.move.turn_ccw);
    output.push_back(props.guesture.subject.head);
    output.push_back(props.guesture.subject.tail);
    output.push_back(props.guesture.sign.head);
    output.push_back(props.guesture.sign.tail);
    output.push_back(props.guesture.sign.intensity);
    output.push_back(props.look_at.longitude);
    output.push_back(props.look_at.altitude);
    output.push_back(props.look_at.magnitude);
    output.push_back(props.aim_at.longitude);
    output.push_back(props.aim_at.altitude);
    output.push_back(props.aim_at.magnitude);
}


void  load(
        motion_desire_props&  props,
        boost::property_tree::ptree const&  ptree_,
        boost::property_tree::ptree const&  defaults_
        )
{
    auto get = [&ptree_, &defaults_](std::string const&  property_name) -> float_32_bit {
        auto  it = ptree_.find(property_name);
        if (it == ptree_.not_found())
            it = defaults_.find(property_name);
        ASSUMPTION(it != defaults_.not_found());
        return it->second.get_value<float_32_bit>();
    };
    props.move.forward = get("move:forward");
    props.move.left = get("move:left");
    props.move.up = get("move:up");
    props.move.turn_ccw = get("move:turn_ccw");
    props.guesture.subject.head = get("guesture:subject:head");
    props.guesture.subject.tail = get("guesture:subject:tail");
    props.guesture.sign.head = get("guesture:sign:head");
    props.guesture.sign.tail = get("guesture:sign:tail");
    props.guesture.sign.intensity = get("guesture:sign:intensity");
    props.look_at.longitude = get("look_at:longitude");
    props.look_at.altitude = get("look_at:altitude");
    props.look_at.magnitude = get("look_at:magnitude");
    props.aim_at.longitude = get("aim_at:longitude");
    props.aim_at.altitude = get("aim_at:altitude");
    props.aim_at.magnitude = get("aim_at:magnitude");
}


}
