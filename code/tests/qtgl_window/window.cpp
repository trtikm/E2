#include "./window.hpp"
#include <utility/log.hpp>
#include <utility/assumptions.hpp>
#include <qtgl/gui_utils.hpp>

namespace xqtgl {


window::window(init_fn_type const&  init_fn,
               step_fn_type const&  step_fn,
               draw_fn_type const&  draw_fn,
               fini_fn_type const&  fini_fn)
    : m_context(nullptr)
    , m_gl_logger(nullptr)
    , m_initialised(false)
    , m_finished(false)
    , m_init_fn(init_fn)
    , m_step_fn(step_fn)
    , m_draw_fn(draw_fn)
    , m_fini_fn(fini_fn)
{
    QSurfaceFormat format;
//    format.setDepthBufferSize(16);
    format.setDepthBufferSize( 24 );
    format.setMajorVersion(4U);
    format.setMinorVersion(2U);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setOption(QSurfaceFormat::DebugContext);
    format.setSwapInterval(0);
    format.setSamples(0);

    m_context = new QOpenGLContext(this);
    m_context->setFormat(format);
    m_context->create();

    setSurfaceType(QWindow::OpenGLSurface);
    setFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    //setGeometry(QRect(10, 10, 640, 480));

    setFormat(format);

    create();
}

void window::exposeEvent(QExposeEvent *)
{
    QTimer::singleShot(10, this, &window::on_timer);
}

void window::on_timer()
{
    if (m_finished)
        return;

    if (m_context->makeCurrent(this))
    {
        if (!m_initialised)
        {
            if (m_gl_logger == nullptr && m_context->hasExtension(QByteArrayLiteral("GL_KHR_debug")))
            {
                m_gl_logger = new QOpenGLDebugLogger(this);
                m_gl_logger->initialize();
            }
            if (!m_init_fn(this))
            {
                m_finished = true;
                return;
            }
            m_initialised = true;
        }
        opengl_api* const  api = dynamic_cast<opengl_api*>(m_context->versionFunctions());
        //opengl_api* const  api = m_context->functions();
        ASSUMPTION(api != nullptr);
        m_draw_fn(this,api);
        m_context->swapBuffers(this);
        if (!m_step_fn())
        {
            m_finished = true;
            return;
        }
        if (m_gl_logger != nullptr)
        {
            const QList<QOpenGLDebugMessage> messages = m_gl_logger->loggedMessages();
            for (const QOpenGLDebugMessage &message : messages)
                LOG(error,qtgl::to_string(message.message()));
        }
    }
    QTimer::singleShot(10, this, &window::on_timer);
}


}
