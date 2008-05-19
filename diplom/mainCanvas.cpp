#include "mainCanvas.h"

static GLfloat xrot;
static GLfloat yrot;


DECLARE_EVENT_TYPE(wxEVT_MY_EVENT, -1)

BEGIN_EVENT_TABLE(MainCanvas, wxGLCanvas)
    EVT_SIZE(MainCanvas::OnSize)
    EVT_PAINT(MainCanvas::OnPaint)
    EVT_CHAR(MainCanvas::OnChar)
    EVT_MOUSE_EVENTS(MainCanvas::OnMouseEvent)
    EVT_ERASE_BACKGROUND(MainCanvas::OnEraseBackground)
END_EVENT_TABLE()

MainCanvas::MainCanvas(TheScene *scene, wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name, int* gl_attrib)
    : wxGLCanvas(parent, (wxGLCanvas*) NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name, gl_attrib )
{
	m_scene = scene;
	m_init = false;
	
}

void MainCanvas::init()
{
	m_scene->initMainGL();		
	
	m_clicked = wxPoint(this->m_width/2, this->m_height/2);
	
	m_init = true;
}


void MainCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    render();
}

void MainCanvas::OnSize(wxSizeEvent& event)
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

void MainCanvas::OnChar(wxKeyEvent& event)
{
    switch( event.GetKeyCode() )
    {
    case WXK_ESCAPE:
        wxTheApp->ExitMainLoop();
        return;
    }
    
}

void MainCanvas::OnMouseEvent(wxMouseEvent& event)
{
	static int dragging = 0;
	static float last_x, last_y;
	
	if(event.LeftIsDown())
	{
	    if(!dragging)
	    {
	        dragging = 1;
	    }
	    else
	    {
	        yrot += (event.GetX() - last_x)*1.0;
	        xrot += (event.GetY() - last_y)*1.0;
	        Refresh(false);
	    }
	    last_x = event.GetX();
	    last_y = event.GetY();
	}
	else
	    dragging = 0;
}

void MainCanvas::OnEraseBackground( wxEraseEvent& WXUNUSED(event) )
{
    // Do nothing, to avoid flashing.
}

void MainCanvas::render()
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
    }
     /* clear color and depth buffers */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0, 1.0, 1.0);
    
    glPushMatrix();
    glRotatef( yrot, 0.0f, 1.0f, 0.0f );
    glRotatef( xrot, 1.0f, 0.0f, 0.0f );
    
    m_scene->renderScene();
    
    glPopMatrix();
	
	glFlush();
    
    SwapBuffers();
}

wxPoint MainCanvas::getMousePos()
{
	return m_clicked;
}

void MainCanvas::setScene(TheScene *scene)
{
	m_scene = scene;
}
