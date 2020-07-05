#ifndef COM_SIMULATION_CONTEXT_HPP_INCLUDED
#   define COM_SIMULATION_CONTEXT_HPP_INCLUDED

#   include <com/frame_of_reference.hpp>
#   include <angeo/collision_object_id.hpp>
#   include <angeo/rigid_body.hpp>
#   include <ai/object_id.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <unordered_map>
#   include <vector>
#   include <memory>

namespace com {


enum struct  OBJECT_TYPE_ID : natural_8_bit
{
    NONE,
    FRAME_OF_REFERENCE,
    COLLIDER,
    RIGID_BODY,
    SENSOR,
    DEVICE,
    AGENT,
};


struct  object_id
{
    OBJECT_TYPE_ID  type_id;
    natural_16_bit  instance_index;
};


inline object_id  invalid_object_id() { return { OBJECT_TYPE_ID::NONE, 0U }; }


struct simulation_context;
using  simulation_context_ptr = std::shared_ptr<simulation_context>;


struct simulation_context
{
    struct  node_type;
    using  node_ptr = std::shared_ptr<node_type>;
    using  nodes_map = std::unordered_map<std::string, node_ptr>;

    struct  node_type
    {
        node_type() : parent(nullptr), children(), oid(invalid_object_id()) {}
        node_ptr  parent;
        nodes_map  children;
        object_id  oid;
    };

    static simulation_context_ptr  create();

    object_id  find(std::string const&  absolute_path) const { return find(absolute_path, root); }
    object_id  find(std::string const&  relative_path, object_id const  search_root) const { return find(relative_path, get_node(search_root)); }

private:

    node_ptr  get_node(object_id const  oid) const;
    object_id  find(std::string const&  relative_path, node_ptr  node) const;

    node_ptr  root;
    std::vector<std::pair<frame_id, node_ptr> >  frames;
    std::vector<std::pair<angeo::collision_object_id, node_ptr> >  colliders;
    std::vector<std::pair<angeo::rigid_body_id, node_ptr> >  rigid_bodies;
    std::vector<std::pair<ai::object_id, node_ptr> >  sensors;
    std::vector<std::pair<ai::object_id, node_ptr> >  devices;
    std::vector<std::pair<ai::object_id, node_ptr> >  agents;

    simulation_context() {}
    simulation_context(simulation_context const&) = delete;
    simulation_context(simulation_context const&&) = delete;
    simulation_context& operator=(simulation_context const&) = delete;
    simulation_context& operator=(simulation_context const&&) = delete;
};


}

#endif
