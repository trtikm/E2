#ifndef QTGL_BATCH_HPP_INCLUDED
#   define QTGL_BATCH_HPP_INCLUDED

#   include <qtgl/draw_state.hpp>
#   include <qtgl/buffer.hpp>
#   include <qtgl/shader.hpp>
#   include <qtgl/texture.hpp>
#   include <qtgl/modelspace.hpp>
#   include <utility/async_resource_load.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_set>
#   include <memory>
#   include <utility>

namespace qtgl { namespace detail {


struct batch_data
{
    batch_data(boost::filesystem::path const&  path, async::finalise_load_on_destroy_ptr);

    batch_data(
            async::finalise_load_on_destroy_ptr,
            buffers_binding const  buffers_binding,
            shaders_binding const  shaders_binding,
            textures_binding const  textures_binding,
            draw_state_ptr const  draw_state,
            modelspace const  modelspace
            )
    {
        initialise(buffers_binding, shaders_binding, textures_binding, draw_state, modelspace);
    }

    ~batch_data();

    buffers_binding  get_buffers_binding() const { return m_buffers_binding; }
    shaders_binding  get_shaders_binding() const { return m_shaders_binding; }
    textures_binding  get_textures_binding() const { return m_textures_binding; }
    draw_state_ptr  get_draw_state() const { return m_draw_state; }
    modelspace  get_modelspace() const { return m_modelspace; }

    bool  ready() const { return m_ready; }
    void  set_ready() { m_ready = true; }

private:

    void  initialise(
            buffers_binding const  buffers_binding,
            shaders_binding const  shaders_binding,
            textures_binding const  textures_binding,
            draw_state_ptr const  draw_state,
            modelspace const  modelspace
            );

    buffers_binding  m_buffers_binding;
    shaders_binding  m_shaders_binding;
    textures_binding  m_textures_binding;
    draw_state_ptr  m_draw_state;
    modelspace  m_modelspace;
    bool  m_ready;
};


}}

namespace qtgl {


struct batch : public async::resource_accessor<detail::batch_data>
{
    batch() : async::resource_accessor<detail::batch_data>()
    {}

    batch(boost::filesystem::path const&  path)
        : async::resource_accessor<detail::batch_data>(path.string(), 1U)
    {}

    batch(  boost::filesystem::path const&  path,
            buffers_binding const  buffers_binding,
            shaders_binding const  shaders_binding,
            textures_binding const  textures_binding,
            draw_state_ptr const  draw_state,
            modelspace const  modelspace
            )
        : async::resource_accessor<detail::batch_data>(
            path.string(),
            async::notification_callback_type(),
            buffers_binding,
            shaders_binding,
            textures_binding,
            draw_state,
            modelspace
            )
    {}

    buffers_binding  get_buffers_binding() const { return resource().get_buffers_binding(); }
    shaders_binding  get_shaders_binding() const { return resource().get_shaders_binding(); }
    textures_binding  get_textures_binding() const { return resource().get_textures_binding(); }
    draw_state_ptr  get_draw_state() const { return resource().get_draw_state(); }
    modelspace  get_modelspace() const { return resource().get_modelspace(); }

    bool  ready() const;

    bool  make_current(draw_state const* const  previous_state) const;

private:

    void  set_ready() { resource().set_ready(); }
};


bool  make_current(batch const&  binding);
bool  make_current(batch const&  binding, draw_state const&  previous_state);


}

#endif
