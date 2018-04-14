#ifndef QTGL_SHADER_DATA_LINKERS_HPP_INCLUDED
#   define QTGL_SHADER_DATA_LINKERS_HPP_INCLUDED

#   include <qtgl/shader_data_bindings.hpp>
#   include <qtgl/batch.hpp>
#   include <qtgl/texture.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>

namespace qtgl {


void  compose_skeleton_binding_data_with_frame_of_keyframe_animation(
        modelspace const  pose,
        skeleton_alignment const  alignment,
        std::vector<matrix44> const&  frame,
        std::vector<matrix44>&  output
        );


void  compose_skeleton_binding_data_with_frame_of_keyframe_animation(
        modelspace const  modelspace,
        skeleton_alignment const  alignment,
        std::vector<matrix44>&  frame  // the results will be composed with the current data.
        );


}

namespace qtgl {


struct vertex_shader_uniform_data_provider_base
{
    vertex_shader_uniform_data_provider_base(batch const  batch_)
        : m_batch(batch_)
    {
        ASSUMPTION(
            !get_batch().is_attached_to_skeleton() ||
            (get_batch().get_modelspace().loaded_successfully() &&
             get_batch().get_skeleton_alignment().loaded_successfully())
            );
    }

    virtual ~vertex_shader_uniform_data_provider_base() {}

    batch  get_batch() const { return m_batch; }

    virtual vector4 const&  get_DIFFUSE_COLOUR() const = 0;
    virtual matrix44 const&  get_TRANSFORM_MATRIX_TRANSPOSED() const = 0;
    virtual std::vector<matrix44> const&  get_TRANSFORM_MATRICES_TRANSPOSED() const = 0;

    natural_32_bit  get_NUM_MATRICES_PER_VERTEX() const
    {
        return get_batch().get_buffers_binding().num_matrices_per_vertex();
    }

private:
    batch  m_batch;
};


struct vertex_shader_uniform_data_provider : public vertex_shader_uniform_data_provider_base
{
    vertex_shader_uniform_data_provider(
            batch const  batch_,
            std::vector<matrix44> const&  transformations,
            vector4 const&  diffuse_colour = vector4{0.5f, 0.5f, 0.5f, 1.0f}
            );

    virtual ~vertex_shader_uniform_data_provider() {}

    vector4 const&  get_DIFFUSE_COLOUR() const override final { return m_diffuse_colour; }
    matrix44 const&  get_TRANSFORM_MATRIX_TRANSPOSED() const override final { return m_transformations.front(); }
    std::vector<matrix44> const&  get_TRANSFORM_MATRICES_TRANSPOSED() const override final { return m_transformations; }

private:
    std::vector<matrix44>  m_transformations;
    vector4  m_diffuse_colour;
};


}

namespace qtgl {


struct fragment_shader_uniform_data_provider_base
{
    virtual ~fragment_shader_uniform_data_provider_base() {}
    virtual vector4 const&  get_FOG_COLOUR() const = 0;
    virtual vector4 const&  get_AMBIENT_COLOUR() const = 0;
    virtual vector4 const&  get_DIFFUSE_COLOUR() const = 0;
    virtual vector4 const&  get_SPECULAR_COLOUR() const = 0;
    virtual vector3 const&  get_DIRECTIONAL_LIGHT_DIRECTION() const = 0;
    virtual vector4 const&  get_DIRECTIONAL_LIGHT_COLOUR() const = 0;
};


struct fragment_shader_uniform_data_provider : public fragment_shader_uniform_data_provider_base
{
    fragment_shader_uniform_data_provider()
        : m_fog_colour(0.25f, 0.25f, 0.25f, 1.0f)
        , m_ambient_colour(0.25f, 0.25f, 0.25f, 1.0f)
        , m_diffuse_colour(0.5f, 0.5f, 0.5f, 1.0f)
        , m_specular_colour(1.0f, 1.0f, 1.0f, 1.0f)
        , m_directional_light_direction(0.0f, 0.0f, -1.0f)
        , m_directional_light_colour(1.0f, 1.0f, 1.0f, 1.0f)
    {}

    fragment_shader_uniform_data_provider(
            vector4 const&  fog_colour,
            vector4 const&  ambient_colour,
            vector4 const&  diffuse_colour,
            vector4 const&  specular_colour,
            vector3 const&  directional_light_direction,
            vector4 const&  directional_light_colour
            )
        : m_fog_colour(fog_colour)
        , m_ambient_colour(ambient_colour)
        , m_diffuse_colour(diffuse_colour)
        , m_specular_colour(specular_colour)
        , m_directional_light_direction(directional_light_direction)
        , m_directional_light_colour(directional_light_colour)
    {}

    virtual ~fragment_shader_uniform_data_provider() {}

    vector4 const&  get_FOG_COLOUR() const override final { return m_fog_colour; }
    vector4 const&  get_AMBIENT_COLOUR() const override final { return m_ambient_colour; }
    vector4 const&  get_DIFFUSE_COLOUR() const override final { return m_diffuse_colour; }
    vector4 const&  get_SPECULAR_COLOUR() const override final { return m_specular_colour; }
    vector3 const&  get_DIRECTIONAL_LIGHT_DIRECTION() const override final { return m_directional_light_direction; }
    vector4 const&  get_DIRECTIONAL_LIGHT_COLOUR() const override final { return m_directional_light_colour; }

private:

    vector4  m_fog_colour;
    vector4  m_ambient_colour;
    vector4  m_diffuse_colour;
    vector4  m_specular_colour;
    vector3  m_directional_light_direction;
    vector4  m_directional_light_colour;
};


}

namespace qtgl {


struct fragment_shader_output_texture_provider_base
{
    virtual ~fragment_shader_output_texture_provider_base() {}
    virtual texture  get_texture_for_BINDING_OUT_TEXTURE_POSITION() const = 0;
    virtual texture  get_texture_for_BINDING_OUT_TEXTURE_NORMAL() const = 0;
    virtual texture  get_texture_for_BINDING_OUT_TEXTURE_DIFFUSE() const = 0;
    virtual texture  get_texture_for_BINDING_OUT_TEXTURE_SPECULAR() const = 0;
};


struct fragment_shader_output_texture_provider : public fragment_shader_output_texture_provider_base
{
    fragment_shader_output_texture_provider()
        : m_position()
        , m_normal()
        , m_diffuse()
        , m_specular()
    {}

    fragment_shader_output_texture_provider(
            texture const  position,
            texture const  normal,
            texture const  diffuse,
            texture const  specular
            )
        : m_position(position)
        , m_normal(normal)
        , m_diffuse(diffuse)
        , m_specular(specular)
    {}

    virtual ~fragment_shader_output_texture_provider() {}

    texture  get_texture_for_BINDING_OUT_TEXTURE_POSITION() const override final { return m_position; }
    texture  get_texture_for_BINDING_OUT_TEXTURE_NORMAL() const override final { return m_normal; }
    texture  get_texture_for_BINDING_OUT_TEXTURE_DIFFUSE() const override final { return m_diffuse; }
    texture  get_texture_for_BINDING_OUT_TEXTURE_SPECULAR() const override final { return m_specular; }

private:

    texture  m_position;
    texture  m_normal;
    texture  m_diffuse;
    texture  m_specular;
};


}

#endif
