#include <ai/motion_desire_props.hpp>
#include <ai/utils_ptree.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <algorithm>

namespace ai {


float_32_bit  clip_to_valid_desire_range(float_32_bit const x)
{
    return std::max(-1.0f, std::min(x, 1.0f));
}


void  as_vector(motion_desire_props const&  props, std::vector<float_32_bit>&  output)
{
    output.reserve(15UL);
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


void  from_vector(std::vector<float_32_bit> const&  data, motion_desire_props&  output_props)
{
    INVARIANT(data.size() == 15UL);
    output_props.move.forward = clip_to_valid_desire_range(data.at(0));
    output_props.move.left = clip_to_valid_desire_range(data.at(1));
    output_props.move.up = clip_to_valid_desire_range(data.at(2));
    output_props.move.turn_ccw = clip_to_valid_desire_range(data.at(3));
    output_props.guesture.subject.head = clip_to_valid_desire_range(data.at(4));
    output_props.guesture.subject.tail = clip_to_valid_desire_range(data.at(5));
    output_props.guesture.sign.head = clip_to_valid_desire_range(data.at(6));
    output_props.guesture.sign.tail = clip_to_valid_desire_range(data.at(7));
    output_props.guesture.sign.intensity = clip_to_valid_desire_range(data.at(8));
    output_props.look_at.longitude = clip_to_valid_desire_range(data.at(9));
    output_props.look_at.altitude = clip_to_valid_desire_range(data.at(10));
    output_props.look_at.magnitude = clip_to_valid_desire_range(data.at(11));
    output_props.aim_at.longitude = clip_to_valid_desire_range(data.at(12));
    output_props.aim_at.altitude = clip_to_valid_desire_range(data.at(13));
    output_props.aim_at.magnitude = clip_to_valid_desire_range(data.at(14));
}


void  load(
        motion_desire_props&  props,
        boost::property_tree::ptree const&  ptree_,
        boost::property_tree::ptree const&  defaults_
        )
{
    props.move.forward = get_value<float_32_bit>("move:forward", ptree_, &defaults_);
    props.move.left = get_value<float_32_bit>("move:left", ptree_, &defaults_);
    props.move.up = get_value<float_32_bit>("move:up", ptree_, &defaults_);
    props.move.turn_ccw = get_value<float_32_bit>("move:turn_ccw", ptree_, &defaults_);
    props.guesture.subject.head = get_value<float_32_bit>("guesture:subject:head", ptree_, &defaults_);
    props.guesture.subject.tail = get_value<float_32_bit>("guesture:subject:tail", ptree_, &defaults_);
    props.guesture.sign.head = get_value<float_32_bit>("guesture:sign:head", ptree_, &defaults_);
    props.guesture.sign.tail = get_value<float_32_bit>("guesture:sign:tail", ptree_, &defaults_);
    props.guesture.sign.intensity = get_value<float_32_bit>("guesture:sign:intensity", ptree_, &defaults_);
    props.look_at.longitude = get_value<float_32_bit>("look_at:longitude", ptree_, &defaults_);
    props.look_at.altitude = get_value<float_32_bit>("look_at:altitude", ptree_, &defaults_);
    props.look_at.magnitude = get_value<float_32_bit>("look_at:magnitude", ptree_, &defaults_);
    props.aim_at.longitude = get_value<float_32_bit>("aim_at:longitude", ptree_, &defaults_);
    props.aim_at.altitude = get_value<float_32_bit>("aim_at:altitude", ptree_, &defaults_);
    props.aim_at.magnitude = get_value<float_32_bit>("aim_at:magnitude", ptree_, &defaults_);
}


}
