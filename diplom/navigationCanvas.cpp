#include "navigationCanvas.h"

DECLARE_EVENT_TYPE(wxEVT_MY_EVENT, -1)
DEFINE_EVENT_TYPE(wxEVT_MY_EVENT)

BEGIN_EVENT_TABLE(NavigationCanvas, wxGLCanvas)
    EVT_SIZE(NavigationCanvas::OnSize)
    EVT_PAINT(NavigationCanvas::OnPaint)
    EVT_CHAR(NavigationCanvas::OnChar)
    EVT_MOUSE_EVENTS(NavigationCanvas::OnMouseEvent)
    EVT_ERASE_BACKGROUND(NavigationCanvas::OnEraseBackground)
END_EVENT_TABLE()

NavigationCanvas::NavigationCanvas(TheScene *scene, int view, wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name, int* gl_attrib)
    : wxGLCanvas(parent, (wxGLCanvas*) NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name, gl_attrib )
{
	m_scene = scene;
	m_view = view;
    m_init = false;
}

void NavigationCanvas::init()
{
	m_scene->initNavGL();
}


void NavigationCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    render();
}

void NavigationCanvas::OnSize(wxSizeEvent& event)
{
	// this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize(event);

    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
    int w, h;
    GetClientSize(&w, &h);
#ifndef __WXMOTIF__
    if (GetContext())
#endif
    {
        SetCurrent();
        glViewport(0, 0, (GLint) w, (GLint) h);
    }
}

void NavigationCanvas::OnChar(wxKeyEvent& event)
{
    switch( event.GetKeyCode() )
    {
    case WXK_ESCAPE:
        wxTheApp->ExitMainLoop();
        return;

    Refresh(false);
    }
}

void NavigationCanvas::OnMouseEvent(wxMouseEvent& event)
{
	m_clicked = event.GetPosition();
	
	if (event.LeftUp() || event.Dragging()) 
	{
		wxCommandEvent event1( wxEVT_MY_EVENT, GetId() );
		event1.SetEventObject( (wxObject*) new wxPoint( event.GetPosition()) );
		event1.SetInt(m_view);
		GetEventHandler()->ProcessEvent( event1 );
	}
}


void NavigationCanvas::OnEraseBackground( wxEraseEvent& WXUNUSED(event) )
{
    // Do nothing, to avoid flashing.
}

void NavigationCanvas::render()
{
	if (m_scene->nothing_loaded) return;
	wxPaintDC dc(this);

#ifndef __WXMOTIF__
    if (!GetContext()) return;
#endif

    SetCurrent();
    // Init OpenGL once, but after SetCurrent
    if (!m_init)
    {
        init();
        m_init = true;
    }
     /* clear color and depth buffers */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0, 1.0, 1.0);
    
	m_scene->renderNavView(m_view);
        
	glFlush();
    
    SwapBuffers();
}

void NavigationCanvas::setScene(TheScene *scene, int view)
{
	this->m_scene = scene;
	this->m_view = view;
	m_init = false;
}

wxPoint NavigationCanvas::getMousePos()
{
	return m_clicked;
}


