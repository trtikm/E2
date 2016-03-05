#include <qtgl/detail/resource_loader.hpp>
#include <qtgl/detail/texture_cache.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/filesystem.hpp>

namespace qtgl { namespace detail {


resource_loader&  resource_loader::instance()
{
    static resource_loader  rl;
    return rl;
}

resource_loader::resource_loader()
    : m_mutex()
{}

void  resource_loader::clear()
{
}

void  resource_loader::insert(texture_properties_ptr const  props, std::function<void(texture_ptr)> const&  receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);

    ASSUMPTION(boost::filesystem::exists(props->image_file()));
    ASSUMPTION(boost::filesystem::is_regular_file(props->image_file()));

    texture_image_properties const  image_props = load_texture_image_file(props->image_file());
    receiver(texture::create(image_props,props));
}


}}
