#include <osi/simulator.hpp>
#include <osi/opengl.hpp>
#include <e2sim/program_info.hpp>
#include <e2sim/program_options.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>


struct  simulator : public osi::simulator
{
    void  round()
    {
        glViewport(0, 0, window_width(), window_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (has_focus())
            glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        else
            glClearColor(0.75f, 0.25f, 0.25f, 1.0f);
        glFrontFace(GL_CW);
        glCullFace(GL_FRONT);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        std::vector<GLchar const*> const vshader_lines = {
            "#version 420\n",
            "in vec4  vertex;\n",
            "void main(void)\n",
            "{\n",
            "    gl_Position = vertex;\n",
            "}\n",
        };
        GLuint const  vshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vshader, (GLsizei)vshader_lines.size(), (GLchar const**)&vshader_lines.at(0), nullptr);
        glCompileShader(vshader);
        GLuint const  vshader_program = glCreateProgram();
        glProgramParameteri(vshader_program, GL_PROGRAM_SEPARABLE, GL_TRUE);
        glAttachShader(vshader_program, vshader);
        glLinkProgram(vshader_program);
        glDetachShader(vshader_program, vshader);
        glDeleteShader(vshader);

        std::vector<GLchar const*> const fshader_lines = {
            "#version 420\n",
            "out vec4  out_colour;\n",
            "void main(void)\n",
            "{\n",
            "    out_colour = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n",
            "}\n",
        };
        GLuint const  fshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fshader, (GLsizei)fshader_lines.size(), (GLchar const**)&fshader_lines.at(0), nullptr);
        glCompileShader(fshader);
        GLuint const  fshader_program = glCreateProgram();
        glProgramParameteri(fshader_program, GL_PROGRAM_SEPARABLE, GL_TRUE);
        glAttachShader(fshader_program, fshader);
        glLinkProgram(fshader_program);
        glDetachShader(fshader_program, fshader);
        glDeleteShader(fshader);

        GLuint  pipeline_id = 0;
        glGenProgramPipelines(1U, &pipeline_id);
        glUseProgramStages(pipeline_id, GL_VERTEX_SHADER_BIT, vshader_program);
        glUseProgramStages(pipeline_id, GL_FRAGMENT_SHADER_BIT, fshader_program);
        glBindProgramPipeline(pipeline_id);

        GLint const  vdata_position = glGetAttribLocation(vshader_program, "vertex");

        std::array<float_32_bit, 12> const  vbuffer_data = {
                0.0f, 0.0f, 0.0f, 1.0f,
                1.0f, 0.0f, 0.0f, 1.0f,
                0.0f, 1.0f, 0.0f, 1.0f,
        };

        GLuint  vbuffer_id = 0U;
        glGenBuffers(1U, &vbuffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbuffer_id);
        glBufferData(GL_ARRAY_BUFFER,
            (GLsizeiptr)(vbuffer_data.size() * sizeof(float_32_bit)),
            (GLvoid const*)vbuffer_data.data(),
            GL_STATIC_DRAW);

        GLuint  barrays_id = 0;
        glGenVertexArrays(1U, &barrays_id);
        glBindVertexArray(barrays_id);

        glBindBuffer(GL_ARRAY_BUFFER, vbuffer_id);
        glEnableVertexAttribArray(vdata_position);
        glVertexAttribPointer(
            vdata_position,
            4,
            GL_FLOAT,
            GL_FALSE,
            0U,
            nullptr
        );

        glBindVertexArray(barrays_id);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindProgramPipeline(0);
        glBindVertexArray(0);

        glDeleteVertexArrays(1U, &barrays_id);
        glDeleteBuffers(1U, &vbuffer_id);

        glDeleteProgramPipelines(1, &pipeline_id);
        glDeleteProgram(vshader_program);
        glDeleteProgram(fshader_program);
    }
};


void run(int argc, char* argv[])
{
    simulator s;
    osi::run(s);
}
