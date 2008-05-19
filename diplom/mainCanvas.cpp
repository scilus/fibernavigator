#include "mainCanvas.h"

static GLuint tex1;
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
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum( -0.52, 0.52, -0.52, 0.52, 5.0, 25.0 );
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef( 0.0, 0.0, -6.0 );
	
	
	glShadeModel(GL_FLAT);
	glEnable(GL_DOUBLEBUFFER);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glAlphaFunc(GL_GREATER,0.1f); // adjust your prefered threshold here
	glEnable(GL_ALPHA_TEST);

	if (m_scene->tex1_loaded)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glGenTextures(1, &tex1);
		glBindTexture(GL_TEXTURE_3D, tex1);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexImage3D(GL_TEXTURE_3D, 
				0, 
				GL_LUMINANCE_ALPHA, 
				m_scene->m_dataset->getColumns(), 
				m_scene->m_dataset->getRows(),
				m_scene->m_dataset->getFrames(),
				0, 
				GL_LUMINANCE_ALPHA, 
				GL_FLOAT,
				m_scene->m_texture);
	}		
	
	m_clicked = wxPoint(this->m_width/2, this->m_height/2);
	
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

void MainCanvas::updateView(int dir, float slize)
{
	m_scene->updateView(dir, slize);
	render();
}

void MainCanvas::OnEraseBackground( wxEraseEvent& WXUNUSED(event) )
{
    // Do nothing, to avoid flashing.
}

void MainCanvas::render()
{
	if (!m_scene->tex1_loaded) return;
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
    
    glPushMatrix();
    glRotatef( yrot, 0.0f, 1.0f, 0.0f );
    glRotatef( xrot, 1.0f, 0.0f, 0.0f );
    
    glEnable(GL_TEXTURE_3D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_3D, tex1);
	
    m_scene->renderScene();
    
    glDisable(GL_TEXTURE_3D);
	
	
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
