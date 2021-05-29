#ifndef GFX_SHADER_DATA_LINKERS_HPP_INCLUDED
#   define GFX_SHADER_DATA_LINKERS_HPP_INCLUDED

#   include <gfx/shader_data_bindings.hpp>
#   include <gfx/batch.hpp>
#   include <gfx/buffer.hpp>
#   include <gfx/texture.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <unordered_map>

namespace gfx {


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

namespace gfx {


struct vertex_shader_instanced_data_provider
{
    vertex_shader_instanced_data_provider();
    explicit vertex_shader_instanced_data_provider(batch const  batch_);

    batch  get_batch() const { return m_batch; }

    void  insert_from_model_to_camera_matrix(matrix44 const&  from_model_to_camera_matrix);
    void  insert_diffuse_colour(vector4 const&  diffuse_colour);

    natural_32_bit  get_num_instances() const { return m_num_instances; }

    bool  make_current() const;

private:
    bool  compute_num_instances() const;

    batch  m_batch;
    mutable std::unique_ptr<std::vector<natural_8_bit> >  m_from_model_to_camera_matrices;
    mutable std::unique_ptr<std::vector<natural_8_bit> >  m_diffuse_colours;
    mutable std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, buffer>  m_buffers;
    mutable natural_32_bit  m_num_instances;
};


}

namespace gfx {


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

    virtual matrix44 const&  get_MATRIX_FROM_MODEL_TO_CAMERA() const = 0;
    virtual matrix44 const&  get_MATRIX_FROM_CAMERA_TO_CLIPSPACE() const = 0;
    virtual std::vector<matrix44> const&  get_MATRICES_FROM_MODEL_TO_CAMERA() const = 0;
    natural_32_bit  get_NUM_MATRICES_PER_VERTEX() const
    {
        return get_batch().get_buffers_binding().num_matrices_per_vertex();
    }
    virtual std::vector<matrix44>&  MATRICES_FROM_MODEL_TO_CAMERA_ref() = 0;

    virtual vector3 const&  get_AMBIENT_COLOUR() const = 0;
    virtual vector4 const&  get_DIFFUSE_COLOUR() const = 0;
    virtual vector4 const&  get_SPECULAR_COLOUR() const = 0;
    virtual vector3 const&  get_DIRECTIONAL_LIGHT_DIRECTION() const = 0;
    virtual vector3 const&  get_DIRECTIONAL_LIGHT_COLOUR() const = 0;

    virtual vector4 const&  get_FOG_COLOUR() const = 0;
    virtual float  get_FOG_NEAR() const = 0;
    virtual float  get_FOG_FAR() const = 0;

private:
    batch  m_batch;
};


struct vertex_shader_uniform_data_provider : public vertex_shader_uniform_data_provider_base
{
    vertex_shader_uniform_data_provider(
            batch const  batch_,
            std::vector<matrix44> const&  matrices_from_model_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            vector4 const&  diffuse_colour = vector4{0.5f, 0.5f, 0.5f, 1.0f},
            vector3 const&  ambient_colour = vector3{ 0.25f, 0.25f, 0.25f },
            vector4 const&  specular_colour = vector4{ 1.0f, 1.0f, 1.0f, 2.0f },
            vector3 const&  directional_light_direction = vector3{ 0.0f, 0.0f, -1.0f }, // WARNING! This should be in camera space!!!!!
            vector3 const&  directional_light_colour = vector3{ 1.0f, 1.0f, 1.0f },
            vector4 const&  fog_colour = vector4{ 0.25f, 0.25f, 0.25f, 2.0f },
            float const  fog_near = 0.25f,
            float const  fog_far = 1000.0f
            );

    virtual ~vertex_shader_uniform_data_provider() {}

    matrix44 const&  get_MATRIX_FROM_MODEL_TO_CAMERA() const override { return m_matrices_from_model_to_camera.front(); }
    matrix44 const&  get_MATRIX_FROM_CAMERA_TO_CLIPSPACE() const override { return m_matrix_from_camera_to_clipspace; }
    std::vector<matrix44> const&  get_MATRICES_FROM_MODEL_TO_CAMERA() const override { return m_matrices_from_model_to_camera; }
    std::vector<matrix44>&  MATRICES_FROM_MODEL_TO_CAMERA_ref() override { return m_matrices_from_model_to_camera; }

    vector3 const&  get_AMBIENT_COLOUR() const override { return m_ambient_colour; }
    vector4 const&  get_DIFFUSE_COLOUR() const override { return m_diffuse_colour; }
    vector4 const&  get_SPECULAR_COLOUR() const override { return m_specular_colour; }
    vector3 const&  get_DIRECTIONAL_LIGHT_DIRECTION() const override { return m_directional_light_direction; }
    vector3 const&  get_DIRECTIONAL_LIGHT_COLOUR() const override { return m_directional_light_colour; }

    vector4 const&  get_FOG_COLOUR() const override { return m_fog_colour; }
    float  get_FOG_NEAR() const override { return m_fog_near; }
    float  get_FOG_FAR() const override { return m_fog_far; }

private:
    std::vector<matrix44>  m_matrices_from_model_to_camera;
    matrix44  m_matrix_from_camera_to_clipspace;
    vector3  m_ambient_colour;
    vector4  m_diffuse_colour;
    vector4  m_specular_colour;
    vector3  m_directional_light_direction;
    vector3  m_directional_light_colour;

    vector4  m_fog_colour;
    float  m_fog_near;
    float  m_fog_far;
};


}

namespace gfx {


struct fragment_shader_uniform_data_provider_base
{
    virtual ~fragment_shader_uniform_data_provider_base() {}

    virtual vector3 const&  get_AMBIENT_COLOUR() const = 0;
    virtual vector4 const&  get_DIFFUSE_COLOUR() const = 0;
    virtual vector4 const&  get_SPECULAR_COLOUR() const = 0;
    virtual vector3 const&  get_DIRECTIONAL_LIGHT_DIRECTION() const = 0;
    virtual vector3 const&  get_DIRECTIONAL_LIGHT_COLOUR() const = 0;

    virtual vector4 const&  get_FOG_COLOUR() const = 0;
    virtual float  get_FOG_NEAR() const = 0;
    virtual float  get_FOG_FAR() const = 0;
};


struct fragment_shader_uniform_data_provider : public fragment_shader_uniform_data_provider_base
{
    fragment_shader_uniform_data_provider()
        : m_ambient_colour(0.25f, 0.25f, 0.25f)
        , m_diffuse_colour(0.5f, 0.5f, 0.5f, 1.0f)
        , m_specular_colour(1.0f, 1.0f, 1.0f, 2.0f)
        , m_directional_light_direction(0.0f, 0.0f, -1.0f)
        , m_directional_light_colour(1.0f, 1.0f, 1.0f)
        , m_fog_colour(0.25f, 0.25f, 0.25f, 2.0f)
        , m_fog_near(0.25f)
        , m_fog_far(1000.0f)
    {}

    fragment_shader_uniform_data_provider(
            vector4 const&  diffuse_colour,
            vector3 const&  ambient_colour,
            vector4 const&  specular_colour,
            vector3 const&  directional_light_direction,
            vector3 const&  directional_light_colour,
            vector4  fog_colour = vector4{ 0.25f, 0.25f, 0.25f, 2.0f },
            float  fog_near = 0.25f,
            float  fog_far = 1000.0f
            )
        : m_ambient_colour(ambient_colour)
        , m_diffuse_colour(diffuse_colour)
        , m_specular_colour(specular_colour)
        , m_directional_light_direction(directional_light_direction)
        , m_directional_light_colour(directional_light_colour)
        , m_fog_colour(fog_colour)
        , m_fog_near(fog_near)
        , m_fog_far(fog_far)
    {}

    virtual ~fragment_shader_uniform_data_provider() {}

    vector3 const&  get_AMBIENT_COLOUR() const override { return m_ambient_colour; }
    vector4 const&  get_DIFFUSE_COLOUR() const override { return m_diffuse_colour; }
    vector4 const&  get_SPECULAR_COLOUR() const override { return m_specular_colour; }
    vector3 const&  get_DIRECTIONAL_LIGHT_DIRECTION() const override { return m_directional_light_direction; }
    vector3 const&  get_DIRECTIONAL_LIGHT_COLOUR() const override { return m_directional_light_colour; }

    vector4 const&  get_FOG_COLOUR() const override { return m_fog_colour; }
    float  get_FOG_NEAR() const override { return m_fog_near; }
    float  get_FOG_FAR() const override { return m_fog_far; }

private:

    vector3  m_ambient_colour;
    vector4  m_diffuse_colour;
    vector4  m_specular_colour;
    vector3  m_directional_light_direction;
    vector3  m_directional_light_colour;

    vector4  m_fog_colour;
    float  m_fog_near;
    float  m_fog_far;
};


}

namespace gfx {


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

    texture  get_texture_for_BINDING_OUT_TEXTURE_POSITION() const override { return m_position; }
    texture  get_texture_for_BINDING_OUT_TEXTURE_NORMAL() const override { return m_normal; }
    texture  get_texture_for_BINDING_OUT_TEXTURE_DIFFUSE() const override { return m_diffuse; }
    texture  get_texture_for_BINDING_OUT_TEXTURE_SPECULAR() const override { return m_specular; }

private:

    texture  m_position;
    texture  m_normal;
    texture  m_diffuse;
    texture  m_specular;
};


}

#endif
