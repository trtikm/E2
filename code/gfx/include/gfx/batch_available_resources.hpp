#ifndef GFX_BATCH_AVAILABLE_RESOURCES_HPP_INCLUDED
#   define GFX_BATCH_AVAILABLE_RESOURCES_HPP_INCLUDED

#   include <gfx/shader_data_bindings.hpp>
#   include <gfx/draw_state.hpp>
#   include <utility/async_resource_load.hpp>
#   include <string>
#   include <unordered_map>
#   include <map>
#   include <type_traits>

namespace gfx { namespace detail {


struct  batch_available_resources_data  final
{
    using  buffers_dictionaty_type =
                std::unordered_map<
                        VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION,
                            // Only allowed values are:
                            //        BINDING_IN_POSITION
                            //        BINDING_IN_DIFFUSE
                            //        BINDING_IN_SPECULAR
                            //        BINDING_IN_NORMAL
                            //        BINDING_IN_TANGENT
                            //        BINDING_IN_BITANGENT
                        std::string
                            // Full path-name of the corresponding resource file on the disk, or any other string
                            // (typically "") in case of generated buffer (e.g. for grid, lines, sphere, box)
                        >;
    using  textures_dictionary_type =
                std::unordered_map<
                        FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME,
                            // Only texure sampler values are allowed, i.e.:
                            //        TEXTURE_SAMPLER_DIFFUSE 
                            //        TEXTURE_SAMPLER_SPECULAR
                            //        TEXTURE_SAMPLER_NORMAL
                        std::pair<
                                VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION,
                                    // Only allowed values are:
                                    //        BINDING_IN_TEXCOORD0
                                    //        BINDING_IN_TEXCOORD1
                                std::string
                                    // Full path-name of the corresponding the texture file on the disk, or any other string
                                    // (typically "") in case of generated texture.
                                >
                        >;

    // Stores pathnames of all 3 files defining the skeletal data of the batch
    struct  skeletal_info
    {
        skeletal_info(
                std::string const&  skeleton_name_, // Name of the skeleton the files below correspond to.
                std::string const&  alignment_,     // Full path-name of the spatial alignment file.
                std::string const&  indices_,       // Full path-name of the indices of matrices file.
                std::string const&  weights_        // Full path-name of the weights of matrices file.
                )
            : m_skeleton_name(skeleton_name_)
            , m_alignment(alignment_)
            , m_indices(indices_)
            , m_weights(weights_)
        {
            ASSUMPTION(!m_skeleton_name.empty());
        }

        std::string const&  skeleton_name() const { return m_skeleton_name; }
        std::string const&  alignment() const { return m_alignment; }
        std::string const&  indices() const { return m_indices; }
        std::string const&  weights() const { return  m_weights; }

    private:
        std::string  m_skeleton_name;
        std::string  m_alignment;
        std::string  m_indices;
        std::string  m_weights;
    };

    using  skeletal_info_const_ptr = std::shared_ptr<skeletal_info const>;

    struct  alpha_testing_props
    {
        alpha_testing_props()
            : alpha_testing_props(false, 0.0f)
        {}

        explicit alpha_testing_props(float_32_bit const  alpha_test_constant_)
            : alpha_testing_props(alpha_test_constant_ > 0.0f, alpha_test_constant_)
        {}

        explicit alpha_testing_props(bool const  use_alpha_testing_, float_32_bit const  alpha_test_constant_)
            : m_use_alpha_testing(use_alpha_testing_)
            , m_alpha_test_constant(alpha_test_constant_)
        {
            ASSUMPTION(m_alpha_test_constant >= 0.0f);
        }

        bool  use_alpha_testing() const { return m_use_alpha_testing; }
        float_32_bit  alpha_test_constant() const { return m_alpha_test_constant; }

    private:
        bool  m_use_alpha_testing;
        float_32_bit  m_alpha_test_constant;
    };

    struct  skin_type
    {
        skin_type(
                textures_dictionary_type const& textures_,
                draw_state const  draw_state_,
                alpha_testing_props const&  alpha_testing_props_
                )
            : m_textures(textures_)
            , m_draw_state(draw_state_)
            , m_alpha_testing_props(alpha_testing_props_)
        {
            ASSUMPTION(!m_draw_state.empty());
        }

        textures_dictionary_type const& textures() const { return m_textures; }
        draw_state  get_draw_state() const { return m_draw_state; }
        alpha_testing_props  const& alpha_testing() const { return m_alpha_testing_props; }

    private:
        textures_dictionary_type  m_textures;
        draw_state  m_draw_state;
        alpha_testing_props  m_alpha_testing_props;
    };

    using  skins_dictionary = std::map<std::string, skin_type>;

    batch_available_resources_data(async::finalise_load_on_destroy_ptr const  finaliser);

    batch_available_resources_data(
            async::finalise_load_on_destroy_ptr,
            buffers_dictionaty_type const&  buffers_,
            skeletal_info_const_ptr const&  skeletal_,
            std::string const&  batch_pathname_,
            std::string const&  data_root_dir_,
            std::string const&  mesh_path_,
            natural_8_bit const  num_indices_per_primitive_,
            skins_dictionary const&  skins_
            );

    buffers_dictionaty_type const&  buffers() const { return m_buffers; }
    skeletal_info_const_ptr const&  skeletal() const { return m_skeletal; }
    std::string const&  batch_pathname() const { return m_batch_pathname; }
    std::string const&  data_root_dir() const { return m_data_root_dir; }
    std::string const&  mesh_path() const { return m_mesh_path; }
    natural_8_bit  num_indices_per_primitive() const { return m_num_indices_per_primitive; }
    bool  has_index_buffer() const { return m_num_indices_per_primitive == 0U; }
    skins_dictionary const&  skins() const { return m_skins; }

private:

    buffers_dictionaty_type  m_buffers;
    skeletal_info_const_ptr  m_skeletal;
    std::string  m_batch_pathname;  // Full path-name
    std::string  m_data_root_dir;   // Full path-name 
    std::string  m_mesh_path;       // A path-name relative to m_data_root_dir
    natural_8_bit  m_num_indices_per_primitive;  // 0 (index buffer), 1 (points), 2 (lines), or 3 (triangles)
    skins_dictionary  m_skins;
};


}}

namespace gfx {


struct  batch_available_resources : public async::resource_accessor<detail::batch_available_resources_data>
{
    using  buffers_dictionaty_type = detail::batch_available_resources_data::buffers_dictionaty_type;
    using  textures_dictionary_type = detail::batch_available_resources_data::textures_dictionary_type;
    using  skeletal_info = detail::batch_available_resources_data::skeletal_info;
    using  skeletal_info_const_ptr = detail::batch_available_resources_data::skeletal_info_const_ptr;
    using  alpha_testing_props = detail::batch_available_resources_data::alpha_testing_props;
    using  skin_type = detail::batch_available_resources_data::skin_type;
    using  skins_dictionary = detail::batch_available_resources_data::skins_dictionary;

    batch_available_resources() : async::resource_accessor<detail::batch_available_resources_data>()
    {}

    batch_available_resources(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr)
        : async::resource_accessor<detail::batch_available_resources_data>(
            {"gfx::batch_available_resources", path.string()},
            1U,
            parent_finaliser
            )
    {}

    void  insert_load_request(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
    {
        async::resource_accessor<detail::batch_available_resources_data>::insert_load_request(
            { "gfx::batch_available_resources", path.string() },
            1U,
            parent_finaliser
            );
    }

    batch_available_resources(
            buffers_dictionaty_type const&  buffers_,
            skeletal_info_const_ptr const&  skeletal_,
            std::string const&  batch_pathname_,
            std::string const&  data_root_dir_,
            std::string const&  mesh_path_,
            natural_8_bit const  num_indices_per_primitive_,
            skins_dictionary const&  skins_,
            std::string const&  key = "",
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::batch_available_resources_data>(
                key.empty() ? async::key_type("gfx::batch_available_resources") :
                              async::key_type{ "gfx::batch_available_resources", key },
                parent_finaliser,
                buffers_,
                skeletal_,
                batch_pathname_,
                data_root_dir_,
                mesh_path_,
                num_indices_per_primitive_,
                skins_
                )
    {}

    template<typename B, typename T>
    batch_available_resources(
            std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, B> const&  buffers_binding_,
            std::unordered_map<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME, T> const&  textures_binding_,
            texcoord_binding const&  texcoord_binding_,
            draw_state const  draw_state_,
            alpha_testing_props const&  alpha_testing_props_,
            std::string const&  key = "",
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::batch_available_resources_data>(
                key.empty() ? async::key_type("gfx::batch_available_resources") :
                              async::key_type{"gfx::batch_available_resources", key },
                parent_finaliser,
                [&buffers_binding_]() -> buffers_dictionaty_type {
                        buffers_dictionaty_type  result;
                        for (auto const&  elem : buffers_binding_)
                            result.insert({elem.first, ""});
                        return result;
                    }(),
                skeletal_info_const_ptr{},
                std::string(),
                std::string(),
                std::string(),
                0U,
                skins_dictionary{
                    {
                        "default",
                        skin_type{
                            [&textures_binding_, &texcoord_binding_]() -> textures_dictionary_type {
                                    textures_dictionary_type  result;
                                    for (auto const& elem : textures_binding_)
                                    {
                                        auto const  it = texcoord_binding_.find(elem.first);
                                        ASSUMPTION(it != texcoord_binding_.cend());
                                        result.insert({elem.first, {it->second,""}});
                                    }
                                    return result;
                                }(),
                                draw_state_,
                                alpha_testing_props_
                            }
                        }
                    }
                )
    {}

    buffers_dictionaty_type const&  buffers() const { return resource().buffers(); }
    skeletal_info_const_ptr const&  skeletal() const { return resource().skeletal(); }
    std::string const&  batch_pathname() const { return resource().batch_pathname(); }
    std::string const&  data_root_dir() const { return resource().data_root_dir(); }
    std::string const&  mesh_path() const { return resource().mesh_path(); }
    natural_8_bit  num_indices_per_primitive() const { return resource().num_indices_per_primitive(); }
    bool  has_index_buffer() const { return resource().has_index_buffer(); }
    skins_dictionary const&  skins() const { return resource().skins(); }
};


}

#endif
