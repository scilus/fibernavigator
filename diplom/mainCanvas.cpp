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
    : wxGLCanvas(parent, id, gl_attrib, pos, size, style|wxFULL_REPAINT_ON_RESIZE, name )
//: wxGLCanvas(parent, (wxGLCanvas*) NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name, gl_attrib )
{
	m_scene = scene;
	m_init = false;
	m_view = view;
	m_XPos = 0;
	m_YPos = 0;
	
	Matrix4fT m_transform1   = {  1.0f,  0.0f,  0.0f,  0.0f,				// NEW: Final Transform
	                       0.0f,  1.0f,  0.0f,  0.0f,
	                       0.0f,  0.0f,  1.0f,  0.0f,
	                       0.0f,  0.0f,  0.0f,  1.0f };
	m_transform = m_transform1;
		   
	Matrix3fT idMat = {  1.0f,  0.0f,  0.0f,					// NEW: Last Rotation
	                   0.0f,  1.0f,  0.0f,
	                   0.0f,  0.0f,  1.0f };

	m_thisRot = idMat;
	m_lastRot = idMat;
	m_isClicked  = false;										// NEW: Clicking The Mouse?
	m_isRClicked = false;										// NEW: Clicking The Right Mouse Button?
	m_isDragging = false;					                    // NEW: Dragging The Mouse?
	m_arcBall = new ArcBallT(640.0f, 480.0f); 
	
	

}

void MainCanvas::init()
{
	switch (m_view)
	{
	case mainView:
		m_scene->initMainGL();
		if (!m_scene->m_mainTexAssigned)
		{
			m_scene->assignTextures();
			m_scene->m_mainTexAssigned = true;
		}
		break;
	default:
		m_scene->initNavGL();
		if (!m_scene->m_navTexAssigned)
		{
			m_scene->assignTextures();
			m_scene->m_navTexAssigned = true;
		}
	}
	m_scene->initShaders();
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
    
	switch (m_view)
	{
	case mainView:
		if (!m_scene->m_mainTexAssigned)
			SetCurrent();
		else
			SetCurrent(*m_scene->getMainGLContext());
		break;
	default:
		if (!m_scene->m_navTexAssigned)
			SetCurrent();
		else
			SetCurrent(*m_scene->getNavGLContext());
		break;
	}
    glViewport(0, 0, (GLint) w, (GLint) h);
    m_arcBall->setBounds((GLfloat)w, (GLfloat)h);    
}

void MainCanvas::OnMouseEvent(wxMouseEvent& event)
{
	wxCommandEvent event1( wxEVT_NAVGL_EVENT, GetId() );
	event1.SetInt(m_view);
	
	switch (m_view)
	{
		case mainView: {
			m_mousePt.s.X = event.GetPosition().x;
			m_mousePt.s.Y = event.GetPosition().y;
			if(event.LeftIsDown())
			{
				if (!m_isDragging)												// Not Dragging
			    {
			      	m_isDragging = true;										// Prepare For Dragging
					m_lastRot = m_thisRot;										// Set Last Static Rotation To Last Dynamic One
					m_arcBall->click(&m_mousePt);								// Update Start Vector And Prepare For Dragging
			    }
			    else
			    {
		            Quat4fT     ThisQuat;

		            m_arcBall->drag(&m_mousePt, &ThisQuat);						// Update End Vector And Get Rotation As Quaternion
		            Matrix3fSetRotationFromQuat4f(&m_thisRot, &ThisQuat);		// Convert Quaternion Into Matrix3fT
		            Matrix3fMulMatrix3f(&m_thisRot, &m_lastRot);				// Accumulate Last Rotation Into This One
		            Matrix4fSetRotationFromMatrix3f(&m_transform, &m_thisRot);	// Set Our Final Transform's Rotation From This One
		            Refresh(false);
			    }
			}
			else 
				m_isDragging = false;
			
		} break;
		
		default: 
			m_clicked = event.GetPosition();
			if (event.LeftUp() || event.Dragging()) 
			{
				event1.SetEventObject( (wxObject*) new wxPoint( event.GetPosition()) );
				GetEventHandler()->ProcessEvent( event1 );
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
	wxPaintDC dc(this);

    SetCurrent((m_view == mainView) ? *m_scene->getMainGLContext() : *m_scene->getNavGLContext());
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
    	glPushMatrix();													// NEW: Prepare Dynamic Transform
	    glMultMatrixf(m_transform.M);										// NEW: Apply Dynamic Transform
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
	if (m_view == mainView)
	{
		SetCurrent(*m_scene->getMainGLContext());
		m_scene->releaseTextures();
		m_scene->m_mainTexAssigned = false;
	}
	else 
	{
		SetCurrent(*m_scene->getNavGLContext());
		m_scene->releaseTextures();
		m_scene->m_navTexAssigned = false;
	}
	m_init = false;
}
