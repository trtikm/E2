#ifndef E2_XQTGL_WINDOW_HPP_INCLUDED
#   define E2_XQTGL_WINDOW_HPP_INCLUDED

#   include <QWindow>
#   include <QOpenGLContext>
#   include <QOpenGLFunctions>
#   include <QOpenGLFunctions_4_2_Core>
#   include <QOpenGLFunctions_3_3_Core>
#   include <QOpenGLDebugLogger>
#   include <QTimer>
#   include <functional>

namespace xqtgl {


typedef QOpenGLFunctions_4_2_Core  opengl_api;
//typedef QOpenGLFunctions  opengl_api;


class window : public QWindow
{
    Q_OBJECT

public:

    using  init_fn_type = std::function<bool(window*)>;
    using  step_fn_type = std::function<bool()>;
    using  draw_fn_type = std::function<void(window*,opengl_api*)>;
    using  fini_fn_type = std::function<void()>;

    window(init_fn_type const&  init_fn,
           step_fn_type const&  step_fn,
           draw_fn_type const&  draw_fn,
           fini_fn_type const&  fini_fn);

    virtual ~window()
    {
        m_fini_fn();
    }

private slots:
    void on_timer();

private:

    void exposeEvent(QExposeEvent *event) Q_DECL_OVERRIDE;

    QOpenGLContext*  m_context;
    QOpenGLDebugLogger*  m_gl_logger;
    bool  m_initialised;
    bool  m_finished;
    init_fn_type m_init_fn;
    step_fn_type m_step_fn;
    draw_fn_type m_draw_fn;
    fini_fn_type m_fini_fn;
};


}

#endif
