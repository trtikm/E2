#include <qtgl/shader_generators.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <vector>
#include <string>

namespace qtgl { namespace vertex_program_generators { namespace transform_3D_vertices {


boost::filesystem::path  imaginary_shader_file() noexcept
{
    return "/gtgl/generated_vertex_programs/transform_3D_vertices";
}

static std::vector<std::string> const&  shader_lines()
{
    static std::string const  IN_POSITION =
            binding_location_name(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION);
    static std::string const  OUT_POSITION =
            binding_location_name(vertex_shader_output_buffer_binding_location::BINDING_OUT_POSITION);
    static std::string const  MATRIX =
            uniform_name(vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED);

    static std::vector<std::string> const  lines{
        msgstream() << "layout(location=" << IN_POSITION << ") in vec3  in_position;\n" << msgstream::end(),
        msgstream() << "layout(location=" << OUT_POSITION << ") out vec4  out_position;\n" << msgstream::end(),
        msgstream() << "uniform mat4 " << MATRIX << ";\n" << msgstream::end(),
        msgstream() << "void main() {\n" << msgstream::end(),
        msgstream() << "    out_position = vec4(in_position,1.0f) * " << MATRIX << ";\n" << msgstream::end(),
        msgstream() << "}\n" << msgstream::end(),
    };

    return lines;
}

vertex_program_properties const&  properties()
{
    static vertex_program_properties const  props(shader_lines());
    return props;
}

vertex_program_ptr  create()
{
    std::string  error_message;
    vertex_program_ptr const  program = vertex_program::create(shader_lines(),error_message);
    INVARIANT(error_message.empty());
    return program;
}


}}}

namespace qtgl { namespace fragment_program_generators { namespace pink_colour {


boost::filesystem::path  imaginary_shader_file() noexcept
{
    return "/gtgl/generated_fragment_programs/pink_colour";
}

static std::vector<std::string> const&  shader_lines()
{
    static std::string const  OUT_COLOUR =
            binding_location_name(fragment_shader_output_buffer_binding_location::BINDING_OUT_COLOUR);

    static std::vector<std::string> const  lines{
        msgstream() << "layout(location=" << OUT_COLOUR << ") out vec4  out_colour;\n" << msgstream::end(),
        msgstream() << "void main() {\n" << msgstream::end(),
        msgstream() << "    out_colour = vec4(1.0, 0.0784, 0.5764, 1.0);\n" << msgstream::end(),
        msgstream() << "}\n" << msgstream::end(),
    };

    return lines;
}

fragment_program_properties const&  properties()
{
    static fragment_program_properties const  props(shader_lines());
    return props;
}

fragment_program_ptr  create()
{
    std::string  error_message;
    fragment_program_ptr const  program = fragment_program::create(shader_lines(),error_message);
    INVARIANT(error_message.empty());
    return program;
}


}}}
