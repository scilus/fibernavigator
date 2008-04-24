
#include "MyGLCanvas.h"

static GLfloat xrot;
static GLfloat yrot;

GLboolean smooth = GL_TRUE;
GLboolean lighting = GL_TRUE;
GLboolean use_vertex_arrays = GL_FALSE;


BEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
    EVT_SIZE(MyGLCanvas::OnSize)
    EVT_PAINT(MyGLCanvas::OnPaint)
    EVT_CHAR(MyGLCanvas::OnChar)
    EVT_MOUSE_EVENTS(MyGLCanvas::OnMouseEvent)
    EVT_ERASE_BACKGROUND(MyGLCanvas::OnEraseBackground)
END_EVENT_TABLE()

MyGLCanvas::MyGLCanvas(wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxGLCanvas(parent, (wxGLCanvas*) NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name )
{
    m_init = false;
}


void MyGLCanvas::Init(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 6.0, 0.0, 6.0, -1.0, 1.0);
}


void MyGLCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    Render();
}

void MyGLCanvas::OnSize(wxSizeEvent& event)
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

void MyGLCanvas::OnChar(wxKeyEvent& event)
{
    switch( event.GetKeyCode() )
    {
    case WXK_ESCAPE:
        wxTheApp->ExitMainLoop();
        return;

    Refresh(false);
    }
}

void MyGLCanvas::OnMouseEvent(wxMouseEvent& event)
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

void MyGLCanvas::OnEraseBackground( wxEraseEvent& WXUNUSED(event) )
{
    // Do nothing, to avoid flashing.
}

void MyGLCanvas::Render()
{
    wxPaintDC dc(this);

#ifndef __WXMOTIF__
    if (!GetContext()) return;
#endif

    SetCurrent();
    // Init OpenGL once, but after SetCurrent
    if (!m_init)
    {
        Init();
        m_init = true;
    }
     /* clear color and depth buffers */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0, 1.0, 1.0);
    
    glBegin(GL_POLYGON);
    	glVertex2f(1.0,1.0);
    	glVertex2f(4.0,1.0);
    	glVertex2f(4.0,4.0);
    	glVertex2f(1.0,4.0);
    glEnd();
    
    glFlush();
    SwapBuffers();
}

void MyGLCanvas::generateTexture(wxImage *image)
{
	unsigned char *imageData = image->GetData();
	int width = image->GetWidth();
	int height = image->GetHeight();
	GLubyte texImage[width][height][4];
	for (int x = 0 ; x < width ; ++x)
	{
		for (int y = 0 ; y < height ; ++y)
		{
			texImage[x][y][0] = (GLubyte) imageData[x*y];
			texImage[x][y][1] = (GLubyte) imageData[x*y+1];
			texImage[x][y][2] = (GLubyte) imageData[x*y+2];
			texImage[x][y][3] = (GLubyte) 255;
		}
	}
	
}
