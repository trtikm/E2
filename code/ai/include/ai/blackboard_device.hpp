#ifndef AI_BLACKBOARD_DEVICE_HPP_INCLUDED
#   define AI_BLACKBOARD_DEVICE_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <ai/device_id.hpp>
#   include <ai/device_kind.hpp>
#   include <memory>

namespace ai {


/// A storage of data shared by all modules of a device.
/// NOTE: This is NOT an implementation of the 'blackboard' AI approach.
///       Only the idea of a shared storage is used here.
struct blackboard_device : public blackboard
{
    virtual ~blackboard_device();
    bool are_all_modules_released() const;

    device_id  m_device_id;
    DEVICE_KIND  m_device_kind;
    scene::node_id  m_device_nid;
};


using  blackboard_device_ptr = std::shared_ptr<blackboard_device>;
using  blackboard_device_const_ptr = std::shared_ptr<blackboard_device const>;

using  blackboard_device_weak_ptr = std::weak_ptr<blackboard_device>;
using  blackboard_device_weak_const_ptr = std::weak_ptr<blackboard_device const>;


}

#endif
