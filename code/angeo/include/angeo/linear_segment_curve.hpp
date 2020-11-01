#ifndef ANGEO_LINEAR_SEGMENT_CURVE_HPP_INCLUDED
#   define ANGEO_LINEAR_SEGMENT_CURVE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <vector>

namespace angeo {


struct  linear_segment_curve
{
    float_32_bit  operator()(float_32_bit const  x) const;

    void  sort();
    std::vector<vector2>  points;   // Must be in the increasing order in the first coordinate !!!!!
                                    // Call 'sort()' method to ensure that!
};


void  load(linear_segment_curve&  curve, boost::property_tree::ptree const&  ptree);


}

#endif
