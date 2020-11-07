#ifndef AI_UTILS_PTREE_HPP_INCLUDED
#   define AI_UTILS_PTREE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <utility/assumptions.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <boost/filesystem/path.hpp>
#   include <functional>
#   include <memory>
#   include <vector>
#   include <string>

namespace ai {


void  load_ptree(boost::property_tree::ptree&  ptree, boost::filesystem::path const&  ptree_pathname);
std::shared_ptr<boost::property_tree::ptree>  load_ptree(boost::filesystem::path const&  ptree_pathname);


vector3  read_vector3(boost::property_tree::ptree const&  ptree);
quaternion  read_quaternion(boost::property_tree::ptree const&  ptree);
void  read_matrix33(boost::property_tree::ptree const&  ptree, matrix33&  output);


boost::property_tree::ptree const&  get_ptree(
        std::string const&  key,
        boost::property_tree::ptree const&  ptree,
        boost::property_tree::ptree const* const  defaults_ptr = nullptr
        );


boost::property_tree::ptree const&  get_ptree_or_empty(
        std::string const&  key,
        boost::property_tree::ptree const&  ptree,
        boost::property_tree::ptree const* const  defaults_ptr = nullptr
        );


template<typename T>
T  get_value(
        std::string const&  key,
        boost::property_tree::ptree const&  ptree,
        boost::property_tree::ptree const* const  defaults_ptr = nullptr
        )
{
    auto  it = ptree.find(key);
    if (it == ptree.not_found())
    {
        ASSUMPTION(defaults_ptr != nullptr);
        it = defaults_ptr->find(key);
        ASSUMPTION(it != defaults_ptr->not_found());
    }
    return it->second.get_value<T>();
}


template<typename T>
T  get_value(
        std::string const&  key,
        T const&  default_value,
        boost::property_tree::ptree const&  ptree,
        boost::property_tree::ptree const* const  defaults_ptr = nullptr
        )
{
    auto  it = ptree.find(key);
    if (it == ptree.not_found())
    {
        if (defaults_ptr == nullptr)
            return default_value;
        it = defaults_ptr->find(key);
        if (it == defaults_ptr->not_found())
            return default_value;
    }
    return it->second.get_value<T>();
}


template<typename T>
T  get_value(
        std::string const&  key,
        T const&  default_value,
        std::function<T(boost::property_tree::ptree const&)> const&  transformer,
        boost::property_tree::ptree const&  ptree,
        boost::property_tree::ptree const* const  defaults_ptr = nullptr
        )
{
    auto  it = ptree.find(key);
    if (it == ptree.not_found())
    {
        if (defaults_ptr == nullptr)
            return default_value;
        it = defaults_ptr->find(key);
        if (it == defaults_ptr->not_found())
            return default_value;
    }
    return transformer(it->second);
}


inline std::string  get_value(
        std::string const&  key,
        char const* const  default_value,
        boost::property_tree::ptree const&  ptree,
        boost::property_tree::ptree const* const  defaults_ptr = nullptr
        )
{
    return get_value<std::string>(key, std::string(default_value), ptree, defaults_ptr);
}


vector3  read_aabb_half_size(boost::property_tree::ptree const&  ptree);


vector3  read_aabb_half_size(
        std::string const&  aabb_key,
        std::string const&  keyframe_key,
        std::vector<vector3> const&  bboxes,
        boost::property_tree::ptree const&  ptree
        );


}

#endif
