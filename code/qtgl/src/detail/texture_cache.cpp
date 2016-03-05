#include <qtgl/detail/texture_cache.hpp>
#include <qtgl/texture_generators.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/filesystem.hpp>
#include <algorithm>

namespace qtgl { namespace detail {


bool  textures_props_equal(texture_properties_ptr const  props0, texture_properties_ptr const  props1)
{
    return *props0 == *props1;
}

size_t  textures_props_hasher(texture_properties_ptr const  props)
{
    return hasher_of_texture_properties(*props);
}


texture_cache&  texture_cache::instance()
{
    static texture_cache  tc;
    return tc;
}

texture_cache::texture_cache()
    : m_cached_textures(10ULL,&qtgl::detail::textures_props_hasher,&qtgl::detail::textures_props_equal)
    , m_mutex()
    , m_dummy_texture(create_chessboard_texture())
{}

void texture_cache::clear(bool destroy_also_dummy_texture)
{
    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_cached_textures.clear();
    if (destroy_also_dummy_texture)
        m_dummy_texture.reset();
}

void  texture_cache::insert_load_request(texture_properties_ptr const  props)
{
    if (!find(props).expired())
        return;

    if (props->image_file() == chessboard_texture_imaginary_image_path())
        insert(props,create_chessboard_texture(props));
    else
    {
        ASSUMPTION(boost::filesystem::exists(props->image_file()));
        ASSUMPTION(boost::filesystem::is_regular_file(props->image_file()));

        // TODO: the following implementation is temporary one! Implement it properly: asynchronously on another thread.

        texture_image_properties const  image_props = load_texture_image_file(props->image_file());
        texture_ptr const  ptexture = texture::create(image_props,props);
        insert(props,ptexture);
    }
}

bool  texture_cache::insert(texture_properties_ptr const  props, texture_ptr const  texture)
{
    std::lock_guard<std::mutex> const  lock(m_mutex);
    return m_cached_textures.insert({props,texture}).second;
}

std::weak_ptr<texture const>  texture_cache::texture_cache::find(texture_properties_ptr const  props) const
{
    std::lock_guard<std::mutex> const  lock(m_mutex);

    auto const  it = m_cached_textures.find(props);
    if (it == m_cached_textures.cend())
        return {};
    return it->second;
}


}}
