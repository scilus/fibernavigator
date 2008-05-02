
#include "navigationCanvas.h"

static GLuint texName;

DECLARE_EVENT_TYPE(wxEVT_MY_EVENT, -1)
DEFINE_EVENT_TYPE(wxEVT_MY_EVENT)

BEGIN_EVENT_TABLE(NavigationCanvas, wxGLCanvas)
    EVT_SIZE(NavigationCanvas::OnSize)
    EVT_PAINT(NavigationCanvas::OnPaint)
    EVT_CHAR(NavigationCanvas::OnChar)
    EVT_MOUSE_EVENTS(NavigationCanvas::OnMouseEvent)
    EVT_ERASE_BACKGROUND(NavigationCanvas::OnEraseBackground)
END_EVENT_TABLE()

NavigationCanvas::NavigationCanvas(wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxGLCanvas(parent, (wxGLCanvas*) NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name )
{
    m_init = false;
    m_texture_loaded = false;
}

void NavigationCanvas::init()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_3D, texName);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexImage3D(GL_TEXTURE_3D, 
			0, 
			GL_RGB, 
			m_dataset->getColumns(), 
			m_dataset->getRows(),
			m_dataset->getFrames(),
			0, 
			GL_LUMINANCE, 
			GL_UNSIGNED_BYTE, 
			m_dataset->getData());
	
	m_clicked = wxPoint(this->m_width/2, this->m_height/2);
	m_Slize = 0.5;
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
	if (event.LeftUp() || event.Dragging()) 
	{
		m_clicked = event.GetPosition();
	
		render();
	
		wxCommandEvent event1( wxEVT_MY_EVENT, GetId() );
		event1.SetEventObject( (wxObject*) new wxPoint( event.GetPosition()) );
		event1.SetInt(m_view);
		GetEventHandler()->ProcessEvent( event1 );
	}
}

void NavigationCanvas::updateView(wxPoint pos, float slize)
{
	m_clicked = pos;
	m_Slize = slize;
	render();
}

void NavigationCanvas::OnEraseBackground( wxEraseEvent& WXUNUSED(event) )
{
    // Do nothing, to avoid flashing.
}

void NavigationCanvas::render()
{
	if (!m_texture_loaded) return;
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
    
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_3D, texName);
	
	switch (m_view)
	{
	case 0:
		glBegin(GL_QUADS);
        	glTexCoord3f(1.0 + m_xOffset, 1.0 + m_yOffset, m_Slize); glVertex3f(0.0,0.0,0.0);
        	glTexCoord3f(1.0 + m_xOffset, 0.0 - m_yOffset, m_Slize); glVertex3f(0.0,1.0,0.0);
        	glTexCoord3f(0.0 - m_xOffset, 0.0 - m_yOffset, m_Slize); glVertex3f(1.0,1.0,0.0);
        	glTexCoord3f(0.0 - m_xOffset, 1.0 + m_yOffset, m_Slize); glVertex3f(1.0,0.0,0.0);
		glEnd();
		break;
	case 1:
		glBegin(GL_QUADS);
	    	glTexCoord3f(0.0 - m_xOffset, m_Slize, 0.0 - m_yOffset); glVertex3f(1.0,1.0,0.0);
	    	glTexCoord3f(0.0 - m_xOffset, m_Slize, 1.0 + m_yOffset); glVertex3f(1.0,0.0,0.0);
	    	glTexCoord3f(1.0 + m_xOffset, m_Slize, 1.0 + m_yOffset); glVertex3f(0.0,0.0,0.0);
	    	glTexCoord3f(1.0 + m_xOffset, m_Slize, 0.0 - m_yOffset); glVertex3f(0.0,1.0,0.0);
	    glEnd();
		break;
	case 2:
		glBegin(GL_QUADS);
        	glTexCoord3f(m_Slize, 0.0 - m_xOffset, 1.0 + m_yOffset); glVertex3f(0.0,0.0,0.0);
        	glTexCoord3f(m_Slize, 0.0 - m_xOffset, 0.0 - m_yOffset); glVertex3f(0.0,1.0,0.0);
        	glTexCoord3f(m_Slize, 1.0 + m_xOffset, 0.0 - m_yOffset); glVertex3f(1.0,1.0,0.0);
        	glTexCoord3f(m_Slize, 1.0 + m_xOffset, 1.0 + m_yOffset); glVertex3f(1.0,0.0,0.0);
		glEnd();
		break;
	}
	glDisable(GL_TEXTURE_3D);
	
	float xline = (float)m_clicked.x/(float)this->m_width;
	float yline = 1.0 - (float)m_clicked.y/(float)this->m_height;
    
	glColor3f(1.0, 0.0, 0.0);
	glBegin (GL_LINES);
		glVertex3f (0.0, yline, 0.1);
		glVertex3f (1.0, yline, 0.1);
		glVertex3f (xline, 0.0, 0.1);
		glVertex3f (xline, 1.0, 0.1);
	glEnd();
	glColor3f(1.0, 1.0, 1.0);
        
	glFlush();
    
    SwapBuffers();
}

void NavigationCanvas::setDataset(TheDataset *dataset, int view)
{
	this->m_dataset = dataset;
	this->m_view = view;
	float ratio = 1.0;
	switch (view)
	{
		case 0:
			ratio = (float)dataset->getColumns()/(float)dataset->getRows();
			break;
		case 1:
			ratio = (float)dataset->getColumns()/(float)dataset->getFrames();
			break;
		case 2:
			ratio = (float)dataset->getRows()/(float)dataset->getFrames();
			break;
	}
	m_xOffset = (wxMax (0, 1.0 - ratio))/2.0;
	m_yOffset = (wxMax (0, ratio - 1.0))/2.0;

	m_texture_loaded = true;
}

wxPoint NavigationCanvas::getMousePos()
{
	return m_clicked;
}


