#ifndef QTGL_BATCH_AVAILABLE_RESOURCES_HPP_INCLUDED
#   define QTGL_BATCH_AVAILABLE_RESOURCES_HPP_INCLUDED

#   include <qtgl/shader_data_bindings.hpp>
#   include <utility/async_resource_load.hpp>
#   include <string>
#   include <unordered_map>
#   include <type_traits>

namespace qtgl { namespace detail {


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
                            //        TEXTURE_SAMPLER_POSITION
                        std::pair<
                                VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION,
                                    // Only allowed values are:
                                    //        BINDING_IN_TEXCOORD0
                                    //        BINDING_IN_TEXCOORD1
                                    //        BINDING_IN_TEXCOORD2
                                    //        BINDING_IN_TEXCOORD3
                                    //        BINDING_IN_TEXCOORD4
                                    //        BINDING_IN_TEXCOORD5
                                    //        BINDING_IN_TEXCOORD6
                                    //        BINDING_IN_TEXCOORD7
                                    //        BINDING_IN_TEXCOORD8
                                    //        BINDING_IN_TEXCOORD9
                                std::string
                                    // Full path-name of the corresponding the texture file on the disk, or any other string
                                    // (typically "") in case of generated texture.
                                >
                        >;

    // Stores pathnames of all 3 files defining the skeletal data of the batch
    struct  skeletal_info
    {
        skeletal_info(
                std::string const&  alignment_,     // Pathname of the spatial alignment file.
                std::string const&  indices_,       // Pathname of the indices of matrices file.
                std::string const&  weights_        // Pathname of the weights of matrices file.
                )
            : m_alignment(alignment_)
            , m_indices(indices_)
            , m_weights(weights_)
        {}

        std::string const&  alignment() const { return m_alignment; }
        std::string const&  indices() const { return m_indices; }
        std::string const&  weights() const { return  m_weights; }

    private:
        std::string  m_alignment;
        std::string  m_indices;
        std::string  m_weights;
    };

    using  skeletal_dictionary_type =
                std::unordered_map<
                    std::string,
                        // A full pathname of a directory under which is a skeleton and its animations.
                        // Typically, the directory is of the form '<some-path>/animation/skeletal/<skeleton-name>'.
                    skeletal_info
                        // Holds pathnames of files containing batch-specific data related to the skeleton above.
                    >;

    batch_available_resources_data(async::finalise_load_on_destroy_ptr const  finaliser);

    batch_available_resources_data(
            async::finalise_load_on_destroy_ptr,
            buffers_dictionaty_type const&  buffers_,
            textures_dictionary_type const&  textures_,
            skeletal_dictionary_type const&  skeletal_,
            std::string const&  index_buffer
            );

    buffers_dictionaty_type const&  buffers() const { return m_buffers; }
    textures_dictionary_type const&  textures() const { return m_textures; }
    skeletal_dictionary_type const&  skeletal() const { return m_skeletal; }
    std::string const&  index_buffer() const { return m_index_buffer; }
    std::string const&  get_root_dir() const { return m_root_dir; }

private:

    buffers_dictionaty_type  m_buffers;
    textures_dictionary_type  m_textures;
    skeletal_dictionary_type  m_skeletal;
    std::string  m_index_buffer;
    std::string  m_root_dir;
};


}}

namespace qtgl {


struct  batch_available_resources : public async::resource_accessor<detail::batch_available_resources_data>
{
    using  buffers_dictionaty_type = detail::batch_available_resources_data::buffers_dictionaty_type;
    using  textures_dictionary_type = detail::batch_available_resources_data::textures_dictionary_type;
    using  skeletal_info = detail::batch_available_resources_data::skeletal_info;
    using  skeletal_dictionary_type = detail::batch_available_resources_data::skeletal_dictionary_type;

    batch_available_resources() : async::resource_accessor<detail::batch_available_resources_data>()
    {}

    batch_available_resources(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr)
        : async::resource_accessor<detail::batch_available_resources_data>(
            {"qtgl::batch_available_resources", path.string()},
            1U,
            parent_finaliser
            )
    {}

    void  insert_load_request(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr)
    {
        async::resource_accessor<detail::batch_available_resources_data>::insert_load_request(
            { "qtgl::batch_available_resources", path.string() },
            1U,
            parent_finaliser
            );
    }

    batch_available_resources(
            buffers_dictionaty_type const&  buffers_,
            textures_dictionary_type const&  textures_,
            skeletal_dictionary_type const&  skeletal_,
            std::string const&  index_buffer,
            std::string const&  key = ""
            )
        : async::resource_accessor<detail::batch_available_resources_data>(
                key.empty() ? async::key_type("qtgl::batch_available_resources") :
                              async::key_type{ "qtgl::batch_available_resources", key },
                nullptr,
                buffers_,
                textures_,
                skeletal_,
                index_buffer
                )
    {}

    template<typename B, typename T>
    batch_available_resources(
            std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, B> const&  buffers_binding_,
            std::unordered_map<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME, T> const&  textures_binding_,
            texcoord_binding const& texcoord_binding_,
            std::string const&  key = ""
            )
        : async::resource_accessor<detail::batch_available_resources_data>(
                key.empty() ? async::key_type("qtgl::batch_available_resources") :
                              async::key_type{ "qtgl::batch_available_resources", key },
                nullptr,
                [&buffers_binding_]() -> buffers_dictionaty_type {
                        buffers_dictionaty_type  result;
                        for (auto const&  elem : buffers_binding_)
                            result.insert({elem.first, ""});
                        return result;
                    }(),
                [&textures_binding_, &texcoord_binding_]() -> textures_dictionary_type {
                        textures_dictionary_type  result;
                        for (auto const&  elem : textures_binding_)
                        {
                            auto const  it = texcoord_binding_.find(elem.first);
                            ASSUMPTION(it != texcoord_binding_.cend());
                            result.insert({elem.first, {it->second,""}});
                        }
                        return result;
                    }(),
                skeletal_dictionary_type{},
                std::string()
                )
    {}

    buffers_dictionaty_type const&  buffers() const { return resource().buffers(); }
    textures_dictionary_type const&  textures() const { return resource().textures(); }
    skeletal_dictionary_type const&  skeletal() const { return resource().skeletal(); }
    std::string const&  index_buffer() const { return resource().index_buffer(); }
    std::string const&  root_dir() const { return resource().get_root_dir(); }
};


}

#endif
