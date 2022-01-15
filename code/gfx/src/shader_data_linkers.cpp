#include <gfx/shader_data_linkers.hpp>
#include <utility/invariants.hpp>
#include <utility/assumptions.hpp>
#include <utility/timeprof.hpp>
#include <utility/config.hpp>
#include <algorithm>
#include <iterator>

namespace gfx {


void  compose_skeleton_binding_data_with_frame_of_keyframe_animation(
        modelspace const  pose,
        skeleton_alignment const  alignment,
        std::vector<matrix44> const&  frame,
        std::vector<matrix44>&  output
        )
{
    ASSUMPTION(pose.loaded_successfully());
    ASSUMPTION(alignment.loaded_successfully());
    ASSUMPTION(frame.size() == pose.get_coord_systems().size());

    output.resize(frame.size());

    matrix44 A;
    angeo::from_base_matrix(alignment.get_skeleton_alignment(), A);
    for (std::size_t i = 0UL; i != pose.get_coord_systems().size(); ++i)
    {
        matrix44 M;
        angeo::to_base_matrix(pose.get_coord_systems().at(i), M);
        output.at(i) = frame.at(i) * M * A;
    }
}


void  compose_skeleton_binding_data_with_frame_of_keyframe_animation(
        modelspace const  modelspace,
        skeleton_alignment const  alignment,
        std::vector<matrix44>&  frame  // the results will be composed with the current data.
        )
{
    ASSUMPTION(modelspace.loaded_successfully());
    ASSUMPTION(alignment.loaded_successfully());
    ASSUMPTION(frame.size() == modelspace.get_coord_systems().size());

    matrix44 A;
    angeo::from_base_matrix(alignment.get_skeleton_alignment(), A);
    for (std::size_t i = 0UL; i != modelspace.get_coord_systems().size(); ++i)
    {
        matrix44 M;
        angeo::to_base_matrix(modelspace.get_coord_systems().at(i), M);
        M *= A;
        frame.at(i) *= M;
    }
}


}

namespace gfx {


vertex_shader_instanced_data_provider::vertex_shader_instanced_data_provider()
    : m_batch()
    , m_from_model_to_camera_matrices()
    , m_diffuse_colours()
    , m_buffers()
    , m_num_instances(0U)
{}


vertex_shader_instanced_data_provider::vertex_shader_instanced_data_provider(batch const  batch_)
    : m_batch(batch_)
    , m_from_model_to_camera_matrices(std::make_unique<std::vector<natural_8_bit> >())
    , m_diffuse_colours(std::make_unique<std::vector<natural_8_bit> >())
    , m_buffers()
    , m_num_instances(0U)
{
    ASSUMPTION(!get_batch().is_attached_to_skeleton());
}


void  vertex_shader_instanced_data_provider::insert_from_model_to_camera_matrix(matrix44 const&  from_model_to_camera_matrix)
{
    TMPROF_BLOCK();
    ASSUMPTION(m_batch.get_instancing_data_ptr()->m_buffers.count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA) != 0UL);
    matrix44  Z = transpose44(from_model_to_camera_matrix);
    std::copy(
        (natural_8_bit const*)Z.data(),
        (natural_8_bit const*)Z.data() + 16U * sizeof(float_32_bit),
        std::back_inserter(*m_from_model_to_camera_matrices)
        );
}


void  vertex_shader_instanced_data_provider::insert_diffuse_colour(vector4 const&  diffuse_colour)
{
    TMPROF_BLOCK();
    ASSUMPTION(m_batch.get_instancing_data_ptr()->m_buffers.count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_INSTANCED_DIFFUSE_COLOUR) != 0UL);
    std::copy(
        (natural_8_bit const*)diffuse_colour.data(),
        (natural_8_bit const*)diffuse_colour.data() + 4U * sizeof(float_32_bit),
        std::back_inserter(*m_diffuse_colours)
        );
}


bool  vertex_shader_instanced_data_provider::make_current() const
{
    TMPROF_BLOCK();

    if (m_batch.empty() || !m_batch.has_instancing_data())
        return true;

    if (m_buffers.empty())
    {
        if (compute_num_instances() == false)
            return false; // The provider has inconsistent data -> cannot compute number of instances -> failure.

        for (auto const buffer_type : m_batch.get_instancing_data_ptr()->m_buffers)
            switch (buffer_type)
            {
            case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA:
                m_buffers.insert({
                    buffer_type,
                    buffer(
                        0U,
                        16U,
                        (natural_32_bit)(m_from_model_to_camera_matrices->size() / (16U * sizeof(float_32_bit))),
                        sizeof(float_32_bit),
                        false,
                        m_from_model_to_camera_matrices,
                        nullptr
                        )
                    });
                break;
            case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_INSTANCED_DIFFUSE_COLOUR:
                m_buffers.insert({
                    buffer_type,
                    buffer(
                        0U,
                        4U,
                        (natural_32_bit)(m_diffuse_colours->size() / (4U * sizeof(float_32_bit))),
                        sizeof(float_32_bit),
                        false,
                        m_diffuse_colours,
                        nullptr
                        )
                    });
                break;
            default: UNREACHABLE();
            }
    }
    for (auto&  location_and_buffer : m_buffers)
        if (location_and_buffer.second.make_current(
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
                    m_batch.get_instancing_data_ptr()->m_shaders_binding.get_locations().at(location_and_buffer.first),
#else
                    value(location_and_buffer.first),
#endif
                    true) == false)
            return false;

    return true;
}


bool  vertex_shader_instanced_data_provider::compute_num_instances() const
{
    TMPROF_BLOCK();

    if (!m_batch.has_instancing_data() || !m_buffers.empty())
        return true;

    integer_32_bit  common_size = -1;
    for (auto const  buffer_type : m_batch.get_instancing_data_ptr()->m_buffers)
    {
        integer_32_bit  current_size;
        switch (buffer_type)
        {
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA:
            current_size = (integer_32_bit)m_from_model_to_camera_matrices->size() / (16 * sizeof(float_32_bit));
            break;
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_INSTANCED_DIFFUSE_COLOUR:
            current_size = (integer_32_bit)m_diffuse_colours->size() / (4 * sizeof(float_32_bit));
            break;
        default: UNREACHABLE();
        }
        if (common_size == -1)
            common_size = current_size;
        else if (common_size != current_size)
            return false;
    }
    m_num_instances = (natural_32_bit)common_size;
    return true;
}


}

namespace gfx {


vertex_shader_uniform_data_provider::vertex_shader_uniform_data_provider(
        batch const  batch_,
        std::vector<matrix44> const&  matrices_from_model_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        vector4 const&  diffuse_colour,
        vector3 const&  ambient_colour,
        vector4 const&  specular_colour,
        vector3 const&  directional_light_direction,
        vector3 const&  directional_light_colour,
        vector4 const&  fog_colour,
        float const  fog_near,
        float const  fog_far
        )
    : vertex_shader_uniform_data_provider_base(batch_)
    , m_matrices_from_model_to_camera()
    , m_matrix_from_camera_to_clipspace(matrix_from_camera_to_clipspace)
    , m_ambient_colour(ambient_colour)
    , m_diffuse_colour(diffuse_colour)
    , m_specular_colour(specular_colour)
    , m_directional_light_direction(directional_light_direction)
    , m_directional_light_colour(directional_light_colour)
    , m_fog_colour(fog_colour)
    , m_fog_near(fog_near)
    , m_fog_far(fog_far)
{
    if (get_batch().is_attached_to_skeleton())
    {
        if (matrices_from_model_to_camera.size() == 1UL)
            m_matrices_from_model_to_camera.resize(
                    get_batch().get_modelspace().get_coord_systems().size(),
                    matrices_from_model_to_camera.front()
                    );
        else
            m_matrices_from_model_to_camera = matrices_from_model_to_camera;
    }
    else if (matrices_from_model_to_camera.size() == 1UL)
        m_matrices_from_model_to_camera.push_back(matrices_from_model_to_camera.front());
    else
    {
        // The case of instancing: The matrices must be set via instanced data provider, i.e. not here.
        ASSUMPTION(matrices_from_model_to_camera.empty());
    }
}


}
