#include <gfx/detail/from_json.hpp>
#include <boost/property_tree/json_parser.hpp>


namespace gfx {


std::string  get_sketch_id_prefix() { return "/generic/sketch/batch/"; }


bool  read_sketch_info_from_id(std::string const&  id, boost::property_tree::ptree&  output_info)
{
    if (!boost::starts_with(id, get_sketch_id_prefix()))
        return false;

    std::string const  json_string = id.substr(get_sketch_id_prefix().size());
    std::istringstream  istr(json_string);
    boost::property_tree::read_json(istr, output_info);

    return true;
}


}

namespace gfx { namespace detail {


vector3  from_json_vector3(boost::property_tree::ptree const&  ptree)
{
    return vector3{
        std::next(ptree.begin(), 0)->second.get_value<float_32_bit>(),
        std::next(ptree.begin(), 1)->second.get_value<float_32_bit>(),
        std::next(ptree.begin(), 2)->second.get_value<float_32_bit>()
    };
}


vector4  from_json_vector4(boost::property_tree::ptree const& ptree)
{
    return vector4{
        std::next(ptree.begin(), 0)->second.get_value<float_32_bit>(),
        std::next(ptree.begin(), 1)->second.get_value<float_32_bit>(),
        std::next(ptree.begin(), 2)->second.get_value<float_32_bit>(),
        std::next(ptree.begin(), 3)->second.get_value<float_32_bit>()
    };
}


}}
