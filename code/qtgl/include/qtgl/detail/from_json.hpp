#ifndef QTGL_DETAIL_FROM_JSON_HPP_INCLUDED
#   define QTGL_DETAIL_FROM_JSON_HPP_INCLUDED

#   include <qtgl/effects_config.hpp>
#   include <angeo/tensor_math.hpp>
#   include <boost/algorithm/string.hpp>
#   include <string>
#   include <boost/property_tree/ptree.hpp>

namespace qtgl { namespace detail {


vector3  from_json_vector3(boost::property_tree::ptree const&  ptree);
vector4  from_json_vector4(boost::property_tree::ptree const&  ptree);


}}

#endif
