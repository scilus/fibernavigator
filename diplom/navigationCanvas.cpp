
#include "navigationCanvas.h"

static GLfloat xrot;
static GLfloat yrot;
static GLuint texName;

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
	
}


void NavigationCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    render();
	//display();
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
        	glTexCoord3f(0.0 - m_xOffset, 0.0 - m_yOffset, 0.5); glVertex3f(0.0,0.0,0.0);
        	glTexCoord3f(0.0 - m_xOffset, 1.0 + m_yOffset, 0.5); glVertex3f(0.0,1.0,0.0);
        	glTexCoord3f(1.0 + m_xOffset, 1.0 + m_yOffset, 0.5); glVertex3f(1.0,1.0,0.0);
        	glTexCoord3f(1.0 + m_xOffset, 0.0 - m_yOffset, 0.5); glVertex3f(1.0,0.0,0.0);
		glEnd();
		break;
	case 1:
		glBegin(GL_QUADS);
	    	glTexCoord3f(0.0 - m_xOffset, 0.5, 0.0 - m_yOffset); glVertex3f(1.0,1.0,0.0);
	    	glTexCoord3f(0.0 - m_xOffset, 0.5, 1.0 + m_yOffset); glVertex3f(1.0,0.0,0.0);
	    	glTexCoord3f(1.0 + m_xOffset, 0.5, 1.0 + m_yOffset); glVertex3f(0.0,0.0,0.0);
	    	glTexCoord3f(1.0 + m_xOffset, 0.5, 0.0 - m_yOffset); glVertex3f(0.0,1.0,0.0);
	    glEnd();
		break;
	case 2:
		glBegin(GL_QUADS);
        	glTexCoord3f(0.5, 0.0 - m_xOffset, 1.0 + m_yOffset); glVertex3f(0.0,0.0,0.0);
        	glTexCoord3f(0.5, 0.0 - m_xOffset, 0.0 - m_yOffset); glVertex3f(0.0,1.0,0.0);
        	glTexCoord3f(0.5, 1.0 + m_xOffset, 0.0 - m_yOffset); glVertex3f(1.0,1.0,0.0);
        	glTexCoord3f(0.5, 1.0 + m_xOffset, 1.0 + m_yOffset); glVertex3f(1.0,0.0,0.0);
		glEnd();
		break;
	}
	
    
        
	glFlush();
    glDisable(GL_TEXTURE_3D);
    SwapBuffers();
}

void NavigationCanvas::setDataset(TheDataset *dataset, int view)
{
	this->m_dataset = dataset;
	this->m_view = view;
	float ratio;
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

void NavigationCanvas::setxSlize(int x)
{
	this->m_xSlize = x;
}

void NavigationCanvas::setySlize(int y)
{
	this->m_ySlize = y;
}

void NavigationCanvas::setzSlize(int z)
{
	this->m_zSlize = z;
}
