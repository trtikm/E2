#ifndef GFX_DETAIL_FROM_JSON_HPP_INCLUDED
#   define GFX_DETAIL_FROM_JSON_HPP_INCLUDED

#   include <gfx/effects_config.hpp>
#   include <angeo/tensor_math.hpp>
#   include <boost/algorithm/string.hpp>
#   include <string>
#   include <boost/property_tree/ptree.hpp>

namespace gfx { namespace detail {


vector3  from_json_vector3(boost::property_tree::ptree const&  ptree);
vector4  from_json_vector4(boost::property_tree::ptree const&  ptree);


}}

#endif
