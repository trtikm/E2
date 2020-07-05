#include <com/simulation_context.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <sstream>
#include <algorithm>

namespace com { namespace detail {


std::string  join(std::vector<std::string> const&  path)
{
    std::stringstream  sstr;
    std::string  xsep;
    for (auto const&  elem : path)
    {
        sstr << xsep << elem;
        xsep = '/';
    }
    return sstr.str();
}


void  split(std::vector<std::string>&  output, std::string const&  path)
{
    std::string  buffer;
    for (auto c : path)
        if (c == '/')
        {
            if (!buffer.empty())
            {
                output.push_back(buffer);
                buffer.clear();
            }
        }
        else
            buffer.push_back(c);
    if (!buffer.empty())
        output.push_back(buffer);
}


}}

namespace com {


simulation_context_ptr  simulation_context::create()
{
    return std::shared_ptr<simulation_context>(new simulation_context);
}


simulation_context::node_ptr  simulation_context::get_node(object_id const  oid) const
{
    switch (oid.type_id)
    {
    case OBJECT_TYPE_ID::FRAME_OF_REFERENCE: return frames.at(oid.instance_index).second;
    case OBJECT_TYPE_ID::COLLIDER: return colliders.at(oid.instance_index).second;
    case OBJECT_TYPE_ID::RIGID_BODY: return rigid_bodies.at(oid.instance_index).second;
    case OBJECT_TYPE_ID::SENSOR: return sensors.at(oid.instance_index).second;
    case OBJECT_TYPE_ID::DEVICE: return devices.at(oid.instance_index).second;
    case OBJECT_TYPE_ID::AGENT: return agents.at(oid.instance_index).second;
    default: return nullptr;
    }
}


object_id  simulation_context::find(std::string const&  relative_path, node_ptr  node) const
{
    TMPROF_BLOCK();

    if (node == nullptr)
        return invalid_object_id();

    std::vector<std::string>  path;
    detail::split(path, relative_path);
    std::reverse(path.begin(), path.end());

    while (!path.empty())
    {
        if (path.back() == ".")
        {}
        else if (path.back() == "..")
        {
            node = node->parent;
            if (node == nullptr)
                return invalid_object_id();
        }
        else
        {
            auto const  it = node->children.find(path.back());
            if (it == node->children.end())
                return invalid_object_id();
            node = it->second;
        }

        path.pop_back();
    }

    return node->oid;
}


}
