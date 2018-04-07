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
                            //        BINDING_IN_INDICES_OF_MATRICES
                            //        BINDING_IN_WEIGHTS_OF_MATRICES
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
                                    // Full path-name of the corresponding resource file on the disk, or any other string
                                    // (typically "") in case of generated buffer (e.g. for grid, lines, sphere, box)
                                >
                        >;

    batch_available_resources_data(boost::filesystem::path const&  path, async::finalise_load_on_destroy_ptr);

    batch_available_resources_data(
            async::finalise_load_on_destroy_ptr,
            buffers_dictionaty_type const&  buffers_,
            textures_dictionary_type const&  textures_
            );

    buffers_dictionaty_type const&  buffers() const { return m_buffers; }
    textures_dictionary_type const&  textures() const { return m_textures; }

private:

    buffers_dictionaty_type  m_buffers;
    textures_dictionary_type  m_textures;
};


}}

namespace qtgl {


struct  batch_available_resources : public async::resource_accessor<detail::batch_available_resources_data>
{
    using  buffers_dictionaty_type = detail::batch_available_resources_data::buffers_dictionaty_type;
    using  textures_dictionary_type = detail::batch_available_resources_data::textures_dictionary_type;

    batch_available_resources() : async::resource_accessor<detail::batch_available_resources_data>()
    {}

    batch_available_resources(boost::filesystem::path const&  path)
        : async::resource_accessor<detail::batch_available_resources_data>(path.string(), 1U)
    {}

    batch_available_resources(
            buffers_dictionaty_type const&  buffers_,
            textures_dictionary_type const&  textures_,
            std::string const&  key = ""
            )
        : async::resource_accessor<detail::batch_available_resources_data>(
                key,
                async::notification_callback_type(),
                buffers_,
                textures_
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
                key,
                async::notification_callback_type(),
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
                    }()
                )
    {}

    buffers_dictionaty_type const&  buffers() const { return resource().buffers(); }
    textures_dictionary_type const&  textures() const { return resource().textures(); }
};


}

#endif
