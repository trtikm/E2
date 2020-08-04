#ifndef AIOLD_BLACKBOARD_DEVICE_HPP_INCLUDED
#   define AIOLD_BLACKBOARD_DEVICE_HPP_INCLUDED

#   include <aiold/blackboard.hpp>
#   include <aiold/device_kind.hpp>
#   include <memory>

namespace aiold {


/// A storage of data shared by all modules of a device.
/// NOTE: This is NOT an implementation of the 'blackboard' AI approach.
///       Only the idea of a shared storage is used here.
struct blackboard_device : public blackboard
{
    ~blackboard_device();

    DEVICE_KIND  m_device_kind;
};


using  blackboard_device_ptr = std::shared_ptr<blackboard_device>;
using  blackboard_device_const_ptr = std::shared_ptr<blackboard_device const>;

using  blackboard_device_weak_ptr = std::weak_ptr<blackboard_device>;
using  blackboard_device_weak_const_ptr = std::weak_ptr<blackboard_device const>;


}

#endif
