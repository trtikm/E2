#include "./program_info.hpp"
#include "./program_options.hpp"
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include "./window.hpp"
#include <QGuiApplication>
#include <QScreen>
#include <QColor>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <qmath.h>
#include <memory>


class Renderer
{
public:
    void initialize(xqtgl::window*  surface);
    void render(xqtgl::window* const  surface, xqtgl::opengl_api* const f);

private:
    QColor m_backgroundColor;
};

void Renderer::initialize(xqtgl::window*  surface)
{
    m_backgroundColor = QColor::fromRgbF(0.1f, 0.1f, 0.2f, 1.0f);
    m_backgroundColor.setRed(qrand() % 64);
    m_backgroundColor.setGreen(qrand() % 128);
    m_backgroundColor.setBlue(qrand() % 256);
}

void Renderer::render(xqtgl::window* const  surface, xqtgl::opengl_api* const f)
{
    QColor color = QColor(100, 255, 0);

    QSize viewSize = surface->size();

    f->glViewport(0, 0, viewSize.width() * surface->devicePixelRatio(), viewSize.height() * surface->devicePixelRatio());
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    f->glClearColor(m_backgroundColor.redF(), m_backgroundColor.greenF(), m_backgroundColor.blueF(), m_backgroundColor.alphaF());
    f->glFrontFace(GL_CW);
    f->glCullFace(GL_FRONT);
    f->glEnable(GL_CULL_FACE);
    f->glEnable(GL_DEPTH_TEST);

    std::vector<GLchar const*> const vshader_lines = {
        "#version 420\n",
        "in vec4  vertex;\n",
        "void main(void)\n",
        "{\n",
        "    gl_Position = vertex;\n",
        "}\n",
    };
    GLuint const  vshader = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vshader, (GLsizei)vshader_lines.size(), (GLchar const**)&vshader_lines.at(0), nullptr);
    f->glCompileShader(vshader);
    GLuint const  vshader_program = f->glCreateProgram();
    f->glProgramParameteri(vshader_program, GL_PROGRAM_SEPARABLE, GL_TRUE);
    f->glAttachShader(vshader_program, vshader);
    f->glLinkProgram(vshader_program);
    f->glDetachShader(vshader_program, vshader);
    f->glDeleteShader(vshader);

    std::vector<GLchar const*> const fshader_lines = {
        "#version 420\n",
        "out vec4  out_colour;\n",
        "void main(void)\n",
        "{\n",
        "    out_colour = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n",
        "}\n",
    };
    GLuint const  fshader = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fshader, (GLsizei)fshader_lines.size(), (GLchar const**)&fshader_lines.at(0), nullptr);
    f->glCompileShader(fshader);
    GLuint const  fshader_program = f->glCreateProgram();
    f->glProgramParameteri(fshader_program, GL_PROGRAM_SEPARABLE, GL_TRUE);
    f->glAttachShader(fshader_program, fshader);
    f->glLinkProgram(fshader_program);
    f->glDetachShader(fshader_program, fshader);
    f->glDeleteShader(fshader);

    GLuint  pipeline_id = 0;
    f->glGenProgramPipelines(1U, &pipeline_id);
    f->glUseProgramStages(pipeline_id, GL_VERTEX_SHADER_BIT, vshader_program);
    f->glUseProgramStages(pipeline_id, GL_FRAGMENT_SHADER_BIT, fshader_program);
    f->glBindProgramPipeline(pipeline_id);

    GLint const  vdata_position = f->glGetAttribLocation(vshader_program, "vertex");

    std::array<float_32_bit, 12> const  vbuffer_data = {
            0.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
    };

    GLuint  vbuffer_id = 0U;
    f->glGenBuffers(1U, &vbuffer_id);
    f->glBindBuffer(GL_ARRAY_BUFFER, vbuffer_id);
    f->glBufferData(GL_ARRAY_BUFFER,
                    (GLsizeiptr)(vbuffer_data.size() * sizeof(float_32_bit)),
                    (GLvoid const*)vbuffer_data.data(),
                    GL_STATIC_DRAW);

    GLuint  barrays_id = 0;
    f->glGenVertexArrays(1U, &barrays_id);
    f->glBindVertexArray(barrays_id);

    f->glBindBuffer(GL_ARRAY_BUFFER, vbuffer_id);
    f->glEnableVertexAttribArray(vdata_position);
    f->glVertexAttribPointer(
            vdata_position,
            4,
            GL_FLOAT,
            GL_FALSE,
            0U,
            nullptr
            );

    f->glBindVertexArray(barrays_id);

    f->glDrawArrays(GL_TRIANGLES, 0, 3);

    f->glBindProgramPipeline(0);
    f->glBindVertexArray(0);

    f->glDeleteVertexArrays(1U, &barrays_id);
    f->glDeleteBuffers(1U, &vbuffer_id);

    f->glDeleteProgramPipelines(1, &pipeline_id);
    f->glDeleteProgram(vshader_program);
    f->glDeleteProgram(fshader_program);
}


std::shared_ptr<Renderer>  renderer;

bool  init(xqtgl::window*  surface)
{
    renderer = std::make_shared<Renderer>();
    if (!renderer)
        return false;
    renderer->initialize(surface);
    return true;
}

bool  step()
{
    return true;
}

void  draw(xqtgl::window* const  surface, xqtgl::opengl_api* const f)
{
    renderer->render(surface,f);
}

void  fini()
{
    renderer.reset();
}


void run()
{
    TMPROF_BLOCK();

    int argc = 1;
    char* argv[] = { "" };
    QGuiApplication app(argc, argv);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    QPoint center = QPoint(screenGeometry.center().x(), screenGeometry.top() + 80);
    QSize windowSize(400, 320);
    int delta = 40;

    QList<QWindow *> windows;
    xqtgl::window *windowA = new xqtgl::window(&init,&step,&draw,&fini);
    windowA->setGeometry(QRect(center, windowSize).translated(-windowSize.width() - delta / 2, 0));
    windowA->setTitle(QStringLiteral("Window"));
    windowA->setVisible(true);
    windows.prepend(windowA);

    app.exec();

    qDeleteAll(windows);

}
