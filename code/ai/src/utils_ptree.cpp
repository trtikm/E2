#include <ai/utils_ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


void  load_ptree(boost::property_tree::ptree&  ptree, boost::filesystem::path const&  ptree_pathname)
{
    if (!boost::filesystem::is_regular_file(ptree_pathname))
        throw std::runtime_error(msgstream() << "Cannot access the file '" << ptree_pathname << "'.");
    boost::property_tree::read_json(ptree_pathname.string(), ptree);
}


std::shared_ptr<boost::property_tree::ptree>  load_ptree(boost::filesystem::path const&  ptree_pathname)
{
    std::shared_ptr<boost::property_tree::ptree>  ptree(new boost::property_tree::ptree);
    load_ptree(*ptree, ptree_pathname);
    return ptree;
}


vector3  read_vector3(boost::property_tree::ptree const&  ptree)
{
    return { ptree.get<float_32_bit>("x"), ptree.get<float_32_bit>("y"), ptree.get<float_32_bit>("z") };
}


quaternion  read_quaternion(boost::property_tree::ptree const&  ptree)
{
    return make_quaternion_wxyz(
                ptree.get<float_32_bit>("w"),
                ptree.get<float_32_bit>("x"),
                ptree.get<float_32_bit>("y"),
                ptree.get<float_32_bit>("z")
                );
}


void  read_matrix33(boost::property_tree::ptree const&  ptree, matrix33&  output)
{
    output(0,0) = ptree.get<float_32_bit>("00");
    output(0,1) = ptree.get<float_32_bit>("01");
    output(0,2) = ptree.get<float_32_bit>("02");
    output(1,0) = ptree.get<float_32_bit>("10");
    output(1,1) = ptree.get<float_32_bit>("11");
    output(1,2) = ptree.get<float_32_bit>("12");
    output(2,0) = ptree.get<float_32_bit>("20");
    output(2,1) = ptree.get<float_32_bit>("21");
    output(2,2) = ptree.get<float_32_bit>("22");
}


boost::property_tree::ptree const&  get_ptree(
        std::string const&  key,
        boost::property_tree::ptree const&  ptree,
        boost::property_tree::ptree const* const  defaults_ptr
        )
{
    auto  it = ptree.find(key);
    if (it == ptree.not_found())
    {
        ASSUMPTION(defaults_ptr != nullptr);
        it = defaults_ptr->find(key);
        ASSUMPTION(it != defaults_ptr->not_found());
    }
    return it->second;
}


boost::property_tree::ptree const&  get_ptree_or_empty(
        std::string const&  key,
        boost::property_tree::ptree const&  ptree,
        boost::property_tree::ptree const* const  defaults_ptr
        )
{
    static boost::property_tree::ptree const  empty;
    auto  it = ptree.find(key);
    if (it == ptree.not_found())
    {
        if (defaults_ptr == nullptr)
            return empty;
        it = defaults_ptr->find(key);
        if (it == defaults_ptr->not_found())
            return empty;
    }
    return it->second;
}


vector3  read_aabb_half_size(boost::property_tree::ptree const&  ptree)
{
    vector3  u = read_vector3(ptree);
    ASSUMPTION(u(0) > 0.0f && u(1) > 0.0f && u(2) > 0.0f && min_coord(u) > 0.0001f);
    return u;
}


vector3  read_aabb_half_size(
        std::string const&  aabb_key,
        std::string const&  keyframe_key,
        std::vector<vector3> const&  bboxes,
        boost::property_tree::ptree const&  ptree
        )
{
    boost::property_tree::ptree const&  p = get_ptree_or_empty(aabb_key, ptree);
    if (p.empty())
    {
        integer_32_bit const  raw_index = get_value(keyframe_key, 0, ptree);
        integer_32_bit const  index = raw_index < 0 ? (int)bboxes.size() - raw_index : raw_index;
        ASSUMPTION(index >= 0 && index < (int)bboxes.size());
        return  bboxes.at(index);
    }
    else
        return  read_aabb_half_size(p);
}


}
