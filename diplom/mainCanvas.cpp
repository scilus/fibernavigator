#include "mainCanvas.h"

static GLuint texName;
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

MainCanvas::MainCanvas(wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name, int* gl_attrib)
    : wxGLCanvas(parent, (wxGLCanvas*) NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name, gl_attrib )
{
    m_init = false;
    m_texture_loaded = false;
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
			GL_LUMINANCE_ALPHA, 
			m_dataset->getColumns(), 
			m_dataset->getRows(),
			m_dataset->getFrames(),
			0, 
			GL_LUMINANCE_ALPHA, 
			GL_DOUBLE,
			m_texture);
			//m_dataset->getData());
	
	m_clicked = wxPoint(this->m_width/2, this->m_height/2);
	m_xSlize = 0.5;
	m_ySlize = 0.5;
	m_zSlize = 0.5;
	m_xTexture = 0.5;
	m_yTexture = 0.5;
	m_zTexture = 0.5;
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

	    //printf("%f %f %d\n", event.GetX(), event.GetY(), (int)event.LeftIsDown());
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
	float ratio;
	switch (dir)
	{
	case 0:
		m_xSlize = slize;
		ratio = (float)m_dataset->getColumns()/(float)m_dataset->getRows();
		if ( ratio > 0.0) 
			m_xTexture = (m_xSlize / ratio) + (1.0 - (1.0/ratio))/2.0;
		break;
	case 1:
		m_ySlize = slize;
		break;
	case 2:
		m_zSlize = slize;
		ratio = (float)m_dataset->getFrames()/(float)m_dataset->getRows();
		if ( ratio > 0.0) 
			m_zTexture = (m_zSlize / ratio) + (1.0 - (1.0/ratio))/2.0;
		break;
	}
	
	render();
}

void MainCanvas::OnEraseBackground( wxEraseEvent& WXUNUSED(event) )
{
    // Do nothing, to avoid flashing.
}

void MainCanvas::render()
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
    
    glPushMatrix();
    glRotatef( yrot, 0.0f, 1.0f, 0.0f );
    glRotatef( xrot, 1.0f, 0.0f, 0.0f );
    
	glEnable(GL_TEXTURE_3D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_3D, texName);
	glBegin(GL_QUADS);
		glTexCoord3f(m_xTexture, 0.0 - m_xOffset2, 0.0 - m_yOffset2); glVertex3f(m_xSlize -0.5, -0.5, -0.5);
    	glTexCoord3f(m_xTexture, 0.0 - m_xOffset2, 1.0 + m_yOffset2); glVertex3f(m_xSlize -0.5, -0.5, 0.5);
    	glTexCoord3f(m_xTexture, 1.0 + m_xOffset2, 1.0 + m_yOffset2); glVertex3f(m_xSlize -0.5, 0.5, 0.5);
    	glTexCoord3f(m_xTexture, 1.0 + m_xOffset2, 0.0 - m_yOffset2); glVertex3f(m_xSlize -0.5, 0.5, -0.5);
    glEnd();
    glBegin(GL_QUADS);
    	glTexCoord3f(0.0 - m_xOffset1, m_ySlize, 0.0 - m_yOffset1); glVertex3f(-0.5, m_ySlize -0.5, -0.5);
    	glTexCoord3f(0.0 - m_xOffset1, m_ySlize, 1.0 + m_yOffset1); glVertex3f(-0.5, m_ySlize -0.5, 0.5);
    	glTexCoord3f(1.0 + m_xOffset1, m_ySlize, 1.0 + m_yOffset1); glVertex3f(0.5, m_ySlize -0.5, 0.5);
    	glTexCoord3f(1.0 + m_xOffset1, m_ySlize, 0.0 - m_yOffset1); glVertex3f(0.5, m_ySlize -0.5, -0.5);
    glEnd();
    glBegin(GL_QUADS);
    	glTexCoord3f(0.0 - m_xOffset0, 0.0 - m_yOffset0, m_zTexture); glVertex3f(-0.5, -0.5, m_zSlize -0.5);
    	glTexCoord3f(0.0 - m_xOffset0, 1.0 + m_yOffset0, m_zTexture); glVertex3f(-0.5, 0.5, m_zSlize -0.5);
    	glTexCoord3f(1.0 + m_xOffset0, 1.0 + m_yOffset0, m_zTexture); glVertex3f(0.5, 0.5, m_zSlize -0.5);
    	glTexCoord3f(1.0 + m_xOffset0, 0.0 - m_yOffset0, m_zTexture); glVertex3f(0.5, -0.5, m_zSlize -0.5);
     glEnd();
	glDisable(GL_TEXTURE_3D);
	
	glPopMatrix();
	
	glFlush();
    
    SwapBuffers();
}

void MainCanvas::setDataset(TheDataset *dataset)
{
	m_dataset = dataset;
	
	double *temp = m_dataset->getData();
	int size = m_dataset->getLength();
	m_texture = new double[size *2];
	
	for (int i = 0 ; i < size; ++i)
	{
		m_texture[2*i] = temp[i];
		m_texture[(2*i)+1] =  temp[i] == 0.0 ? 0 : 255;
	}
	
		
	float xSize = (float)dataset->getColumns();
	float ySize = (float)dataset->getRows();
	float zSize = (float)dataset->getFrames();
	
	float ratio0 = xSize/ySize;
	float ratio1 = xSize/zSize;
	float ratio2 = ySize/zSize;

	m_xOffset0 = (wxMax (0, 1.0 - ratio0))/2.0;
	m_yOffset0 = (wxMax (0, ratio0 - 1.0))/2.0;
	m_xOffset1 = (wxMax (0, 1.0 - ratio1))/2.0;
	m_yOffset1 = (wxMax (0, ratio1 - 1.0))/2.0;
	m_xOffset2 = (wxMax (0, 1.0 - ratio2))/2.0;
	m_yOffset2 = (wxMax (0, ratio2 - 1.0))/2.0;
	
	m_xOffset0 = wxMax(m_xOffset0, m_xOffset1);
	m_xOffset1 = wxMax(m_xOffset0, m_xOffset1);
	
	m_yOffset1 = wxMax(m_yOffset1, m_yOffset2);
	m_yOffset2 = wxMax(m_yOffset1, m_yOffset2);

	m_texture_loaded = true;
}

wxPoint MainCanvas::getMousePos()
{
	return m_clicked;
}


