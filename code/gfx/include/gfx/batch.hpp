#ifndef GFX_BATCH_HPP_INCLUDED
#   define GFX_BATCH_HPP_INCLUDED

#   include <gfx/batch_available_resources.hpp>
#   include <gfx/buffer.hpp>
#   include <gfx/shader.hpp>
#   include <gfx/texture.hpp>
#   include <gfx/effects_config.hpp>
#   include <gfx/modelspace.hpp>
#   include <gfx/skeleton_alignment.hpp>
#   include <gfx/draw_state.hpp>
#   include <utility/async_resource_load.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_set>
#   include <functional>
#   include <memory>
#   include <utility>

namespace gfx { namespace detail {


struct  batch_instancing_data
{
    shaders_binding  m_shaders_binding;
    std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>  m_buffers;
};

using  batch_instancing_data_ptr = std::unique_ptr<batch_instancing_data>;


struct batch_data
{
    batch_data(
            async::finalise_load_on_destroy_ptr const  finaliser,
            boost::filesystem::path const&  path,
            effects_config const  effects_,
            std::string const&  skin_name_
            )
        : m_buffers_binding()
        , m_shaders_binding()
        , m_textures_binding()
        , m_effects_config(effects_)
        , m_draw_state()
        , m_modelspace()
        , m_skeleton_alignment()
        , m_available_resources()
        , m_instancing_data()
        , m_skin_name(skin_name_)
        , m_id(path.string())
        , m_ready(false)
    {
        m_available_resources.insert_load_request(
                path.string(),
                async::finalise_load_on_destroy::create(std::bind(&batch_data::load, this, std::placeholders::_1), finaliser)
                );
    }

    batch_data(
            async::finalise_load_on_destroy_ptr const  finaliser,
            buffers_binding const  buffers_binding_,
            textures_binding const  textures_binding_,
            texcoord_binding const&  texcoord_binding_,
            effects_config const  effects_,
            draw_state const  draw_state_,
            modelspace const  modelspace_,
            skeleton_alignment const  skeleton_alignment_,
            batch_available_resources const  resources_,
            std::string const&  skin_name_,
            std::string const&  id_
            );

    batch_data(
            async::finalise_load_on_destroy_ptr,
            buffers_binding const  buffers_binding_,
            shaders_binding const  shaders_binding_,
            textures_binding const  textures_binding_,
            effects_config const  effects_,
            draw_state const  draw_state_,
            modelspace const  modelspace_,
            skeleton_alignment const  skeleton_alignment_,
            batch_available_resources const  resources_,
            batch_instancing_data const&  instancing_data_,
            std::string const& skin_name_,
            std::string const&  id_
            )
        : m_buffers_binding(buffers_binding_)
        , m_shaders_binding(shaders_binding_)
        , m_textures_binding(textures_binding_)
        , m_effects_config(effects_)
        , m_draw_state(draw_state_)
        , m_modelspace(modelspace_)
        , m_skeleton_alignment(skeleton_alignment_)
        , m_available_resources(resources_)
        , m_instancing_data(make_instancing_data_from(instancing_data_))
        , m_skin_name(skin_name_)
        , m_id(id_)
        , m_ready(false)
    {
        ASSUMPTION(modelspace_.empty() == skeleton_alignment_.empty());
    }

    ~batch_data();

    buffers_binding  get_buffers_binding() const { return m_buffers_binding; }
    shaders_binding  get_shaders_binding() const { return m_shaders_binding; }
    textures_binding  get_textures_binding() const { return m_textures_binding; }
    effects_config  get_effects_config() const { return m_effects_config; }
    draw_state  get_draw_state() const { return m_draw_state; }
    modelspace  get_modelspace() const { return m_modelspace; }
    skeleton_alignment  get_skeleton_alignment() const { return m_skeleton_alignment; }
    batch_available_resources  get_available_resources() const { return m_available_resources; }
    std::string const&  get_skin_name() const { return m_skin_name; }
    batch_instancing_data const*  get_instancing_data_ptr() const { return m_instancing_data.get(); }
    std::string const&  get_id() const { return m_id; }

    bool  is_attached_to_skeleton() const { return !get_modelspace().empty(); }
    bool  has_instancing_data() const { return m_instancing_data != nullptr; }
    bool  is_translucent() const;
    bool  ready() const { return m_ready; }
    void  set_ready() { m_ready = true; }

private:

    static inline batch_instancing_data_ptr  make_instancing_data_from(batch_instancing_data const&  instancing_data)
    {
        return instancing_data.m_buffers.empty() || instancing_data.m_shaders_binding.empty() ?
                    nullptr : std::make_unique<batch_instancing_data>(instancing_data);
    }

    void  load(async::finalise_load_on_destroy_ptr const  finaliser);

    buffers_binding  m_buffers_binding;
    shaders_binding  m_shaders_binding;
    textures_binding  m_textures_binding;
    effects_config  m_effects_config;
    draw_state  m_draw_state;
    modelspace  m_modelspace;
    skeleton_alignment  m_skeleton_alignment;
    batch_available_resources  m_available_resources;
    batch_instancing_data_ptr  m_instancing_data;
    std::string  m_skin_name;
    std::string  m_id;
    bool  m_ready;
};


}}

namespace gfx {


using batch_instancing_data = detail::batch_instancing_data;
using batch_instancing_data_ptr = detail::batch_instancing_data_ptr;


struct batch : public async::resource_accessor<detail::batch_data>
{
    using  super = async::resource_accessor<detail::batch_data>;

    batch()
        : super()
    {}

    batch(  boost::filesystem::path const&  path,
            effects_config const  effects,
            std::string const&  skin_name = "default",
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : super(
            {"gfx::batch", "PATH=" + path.string() + ",SKIN=" + skin_name + ",EFFECTS=" + effects.key().get_unique_id()},
            parent_finaliser,
            path,
            effects,
            skin_name
            )
    {}

    template<typename T>
    batch(  std::string const&  id,
            buffers_binding const  buffers_binding_,
            std::unordered_map<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME, T> const&  textures_binding_map_,
            texcoord_binding const&  texcoord_binding_,
            effects_config const  effects_,
            draw_state const  draw_state_,
            modelspace const  modelspace_,
            skeleton_alignment const  skeleton_alignment_,
            batch_available_resources::alpha_testing_props const&  alpha_testing_props_,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : super(
            id.empty() ? async::key_type("gfx::batch") : async::key_type{ "gfx::batch", id },
            parent_finaliser,
            buffers_binding_,
            textures_binding(textures_binding_map_),
            texcoord_binding_,
            effects_,
            draw_state_,
            modelspace_,
            skeleton_alignment_,
            batch_available_resources(
                buffers_binding_.get_buffers(),
                textures_binding_map_,
                texcoord_binding_,
                draw_state_,
                alpha_testing_props_
                ),
            "default",
            id
            )
    {}

    batch(  std::string const&  id,
            buffers_binding const  buffers_binding_,
            shaders_binding const  shaders_binding_,
            textures_binding const  textures_binding_,
            effects_config const  effects_,
            draw_state const  draw_state_,
            modelspace const  modelspace_,
            skeleton_alignment const  skeleton_alignment_,
            batch_available_resources const  resources_,
            batch_instancing_data const&  instancing_data,
            std::string const&  skin_name_,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : super(
            id.empty() ? async::key_type("gfx::batch") : async::key_type{ "gfx::batch", id },
            parent_finaliser,
            buffers_binding_,
            shaders_binding_,
            textures_binding_,
            effects_,
            draw_state_,
            modelspace_,
            skeleton_alignment_,
            resources_,
            instancing_data,
            skin_name_,
            id
            )
    {}

    buffers_binding  get_buffers_binding() const { return resource().get_buffers_binding(); }
    shaders_binding  get_shaders_binding() const { return resource().get_shaders_binding(); }
    textures_binding  get_textures_binding() const { return resource().get_textures_binding(); }
    effects_config  get_effects_config() const { return resource().get_effects_config(); }
    draw_state  get_draw_state() const { return resource().get_draw_state(); }
    modelspace  get_modelspace() const { return resource().get_modelspace(); }
    skeleton_alignment  get_skeleton_alignment() const { return resource().get_skeleton_alignment(); }
    batch_available_resources  get_available_resources() const { return resource().get_available_resources(); }
    std::string const&  get_skin_name() const { return resource().get_skin_name(); }
    batch_instancing_data const*  get_instancing_data_ptr() const { return resource().get_instancing_data_ptr(); }

    std::string  uid() const { return key().get_data_type_name() + key().get_unique_id(); }
    std::string const&  get_id() const { return resource().get_id(); }

    bool  is_attached_to_skeleton() const { return resource().is_attached_to_skeleton(); }
    bool  has_instancing_data() const { return resource().has_instancing_data(); }
    bool  is_translucent() const { return resource().is_translucent(); }
    bool  ready() const;

    bool  make_current(draw_state const&  previous_state, bool const  for_instancing) const;

private:

    void  set_ready() { resource().set_ready(); }
};


bool  make_current(batch const&  binding, bool const  for_instancing = false);
bool  make_current(batch const&  binding, draw_state const&  previous_state, bool const  for_instancing = false);


}

#endif
