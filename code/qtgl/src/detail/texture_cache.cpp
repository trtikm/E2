#include <qtgl/detail/texture_cache.hpp>
#include <qtgl/detail/resource_loader.hpp>
#include <qtgl/texture_generators.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>
#include <functional>

namespace qtgl { namespace detail {


static size_t  boost_path_hasher(boost::filesystem::path const&  path)
{
    std::size_t seed = 0ULL;
    boost::hash_combine(seed,path.string());
    return seed;
}

static bool  textures_props_equal(texture_properties_ptr const  props0, texture_properties_ptr const  props1)
{
    return *props0 == *props1;
}

static size_t  textures_props_hasher(texture_properties_ptr const  props)
{
    return hasher_of_texture_properties(*props);
}


texture_cache&  texture_cache::instance()
{
    static texture_cache  tc;
    return tc;
}

texture_cache::texture_cache()
    : m_cached_properties(10ULL,&qtgl::detail::boost_path_hasher)
    , m_cached_textures(10ULL,&qtgl::detail::textures_props_hasher,&qtgl::detail::textures_props_equal)
    , m_pending_properties()
    , m_pending_textures()
    , m_failed_properties(10ULL,&qtgl::detail::boost_path_hasher)
    , m_mutex()
    , m_dummy_texture(create_chessboard_texture())
{}

void  texture_cache::receiver_props(boost::filesystem::path const&  texture_file,
                                    texture_properties_ptr const  texture_props,
                                    std::string const&  error_message)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    if (error_message.empty())
        m_pending_properties.push_back({texture_file,texture_props});
    else
        m_failed_properties[texture_file] = error_message;
}

void  texture_cache::receiver(texture_image_properties&  image_props, texture_properties_ptr const  texture_props)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_pending_textures.push_back({std::move(image_props),texture_props});
}

void texture_cache::clear(bool destroy_also_dummy_texture)
{
    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_pending_properties.clear();
    m_pending_textures.clear();
    m_cached_properties.clear();
    m_cached_textures.clear();
    m_failed_properties.clear();
    if (destroy_also_dummy_texture)
        m_dummy_texture.reset();
}

void  texture_cache::insert_load_request(boost::filesystem::path const&  texture_file)
{
    TMPROF_BLOCK();

    if (find(texture_file).operator bool())
        return;

    resource_loader::instance().insert_texture_request(
                texture_file,
                std::bind(&texture_cache::receiver_props,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)
                );
}

void  texture_cache::insert_load_request(texture_properties_ptr const  props)
{
    TMPROF_BLOCK();

    if (!find(props).expired())
        return;

    if (props->image_file() == chessboard_texture_imaginary_image_path())
        insert(create_chessboard_texture(props));
    else
        resource_loader::instance().insert_texture_request(
                    props,
                    std::bind(&texture_cache::receiver,this,std::placeholders::_1,std::placeholders::_2)
                    );
}

bool  texture_cache::insert(texture_ptr const  texture)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    return m_cached_textures.insert({texture->properties(),texture}).second;
}

texture_properties_ptr  texture_cache::find(boost::filesystem::path const&  texture_file)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    while (!m_pending_properties.empty())
    {
        m_cached_properties.insert(m_pending_properties.back());
        m_pending_properties.pop_back();
    }
    auto const  it = m_cached_properties.find(texture_file);
    if (it == m_cached_properties.cend())
        return {};
    return it->second;
}

std::weak_ptr<texture const>  texture_cache::find(texture_properties_ptr const  props)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    while (!m_pending_textures.empty())
    {
        texture_ptr const  ptexture = texture::create(m_pending_textures.back().first,m_pending_textures.back().second);
        m_cached_textures.insert({ptexture->properties(),ptexture});
        m_pending_textures.pop_back();
    }
    auto const  it = m_cached_textures.find(props);
    if (it == m_cached_textures.cend())
        return {};
    return it->second;
}


}}
