#include "mainCanvas.h"

//DECLARE_EVENT_TYPE(wxEVT_MY_EVENT, -1)
DECLARE_EVENT_TYPE(wxEVT_NAVGL_EVENT, -1)
DEFINE_EVENT_TYPE(wxEVT_NAVGL_EVENT)

BEGIN_EVENT_TABLE(MainCanvas, wxGLCanvas)
    EVT_SIZE(MainCanvas::OnSize)
    EVT_PAINT(MainCanvas::OnPaint)
    EVT_MOUSE_EVENTS(MainCanvas::OnMouseEvent)
    EVT_ERASE_BACKGROUND(MainCanvas::OnEraseBackground)
END_EVENT_TABLE()

MainCanvas::MainCanvas(TheScene *scene, int view, wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name, int* gl_attrib)
    : wxGLCanvas(parent, (wxGLCanvas*) NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name, gl_attrib )
{
	m_scene = scene;
	m_init = false;
	m_view = view;
	m_texturesAssigned = false;
	m_XPos = 0;
	m_YPos = 0;
	
}

void MainCanvas::init()
{
	switch (m_view)
	{
	case mainView:
		m_scene->initMainGL();
		break;
	default:
		m_scene->initNavGL();
	}
			
	m_init = true;
	if (!m_texturesAssigned)
	{
		m_scene->assignTextures();
		m_scene->initShaders();
		m_texturesAssigned = true;	
	}
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
    /*
#ifndef __WXMOTIF__
    if (GetContext())
#endif
*/
    {
        SetCurrent();
        glViewport(0, 0, (GLint) w, (GLint) h);
    }
}

void MainCanvas::OnMouseEvent(wxMouseEvent& event)
{
	switch (m_view)
	{
		case mainView: {
			static int dragging = 0;
			
			if(event.LeftIsDown())
			{
			    if(!dragging)
			    {
			        dragging = 1;
			    }
			    else
			    {
			        m_yrot += (event.GetX() - m_XPos);
			        m_xrot += (event.GetY() - m_YPos);
			        Refresh(false);
			    }
			    m_XPos = event.GetX();
			    m_YPos = event.GetY();
			}
			else
			    dragging = 0;
		} break;
		
		default: {
			m_clicked = event.GetPosition();
			if (event.LeftUp() || event.Dragging()) 
			{
				wxCommandEvent event1( wxEVT_NAVGL_EVENT, GetId() );
				event1.SetEventObject( (wxObject*) new wxPoint( event.GetPosition()) );
				event1.SetInt(m_view);
				GetEventHandler()->ProcessEvent( event1 );
			}
		}
	}
}

wxPoint MainCanvas::getMousePos()
{
	return m_clicked;
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
    
    switch (m_view)
    {
    case mainView: {
	    glPushMatrix();
	    glRotatef( m_yrot, 0.0f, 1.0f, 0.0f );
	    glRotatef( m_xrot, 1.0f, 0.0f, 0.0f );
	    m_scene->renderScene(m_view);
	    glPopMatrix();
	    break;
    }
    default:
    	m_scene->renderNavView(m_view);
    }
	glFlush();
    
    SwapBuffers();
}

void MainCanvas::setScene(TheScene *scene)
{
	m_scene = scene;
}

void MainCanvas::invalidate()
{
	m_init = false;
	m_texturesAssigned = false;
}
