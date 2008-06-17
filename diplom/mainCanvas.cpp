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
{
	m_scene = scene;
	m_init = false;
	m_view = view;
		
	Matrix4fT m_transform1   = {  1.0f,  0.0f,  0.0f,  0.0f,
	                       0.0f,  1.0f,  0.0f,  0.0f,
	                       0.0f,  0.0f,  1.0f,  0.0f,
	                       0.0f,  0.0f,  0.0f,  1.0f };
	/*
	Matrix4fT m_transform2   = {-0.62f,  0.25f,  -0.75f,  0.0f,
		                       	 0.78f,  0.32f,  -0.54f,  0.0f,
		                        -0.11f, -0.91f,  -0.39f,  0.0f,
		                         0.0f,   0.0f,     0.0f,  1.0f };
	*/
	m_transform = m_transform1;
		   
	Matrix3fT idMat = {  1.0f,  0.0f,  0.0f,
	                     0.0f,  1.0f,  0.0f,
	                     0.0f,  0.0f,  1.0f };
	/*
	Matrix3fT lastRot1   = {  -0.62f,  0.25f,  -0.75f,
			                   0.78f,  0.32f,  -0.54f,
			                  -0.11f, -0.91f,  -0.39f};
	*/
	m_thisRot =idMat;
	m_lastRot =idMat;
	
	m_isDragging = false;					                    // NEW: Dragging The Mouse?
	m_isrDragging = false;
	m_arcBall = new ArcBallT(640.0f, 480.0f); 
}

void MainCanvas::init()
{
	m_scene->initGL(m_view);
	m_init = true;
}


void MainCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    render();
}

void MainCanvas::OnSize(wxSizeEvent& event)
{
    if (!m_scene->m_texAssigned)
    		SetCurrent();
	else
		SetCurrent(*m_scene->getMainGLContext());

	// this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize(event);
    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
    int w, h;
    GetClientSize(&w, &h);
    glViewport(0, 0, (GLint) w, (GLint) h);
    
    m_arcBall->setBounds((GLfloat)w, (GLfloat)h);    
}

void MainCanvas::OnMouseEvent(wxMouseEvent& event)
{
	wxCommandEvent event1( wxEVT_NAVGL_EVENT, GetId() );
	event1.SetInt(m_view);
	int clickX = event.GetPosition().x;
	int clickY = event.GetPosition().y;
	switch (m_view)
	{
		case mainView: {
			m_mousePt.s.X = clickX;
			m_mousePt.s.Y = clickY;
			
			if (event.RightIsDown())												// If Right Mouse Clicked, Reset All Rotations
		    {
				if (!m_isrDragging)												// Not Dragging
			    {
					m_isrDragging = true;										// Prepare For Dragging
					m_lastPos = event.GetPosition();
					m_picked = pick(event.GetPosition());
			    }
				else {
					if (event.Dragging()) 
					{
						int xDrag = m_lastPos.x - clickX;
						int yDrag = -(m_lastPos.y - clickY);
						GetEventHandler()->ProcessEvent( event1 );
						m_lastPos = event.GetPosition();
						Vector3fT v1 = {0,0,0};
						switch (m_picked) {
						case axial: 
							v1.s.X = 1.0;
							break;
						case coronal:
							v1.s.Y = 1.0;
							break;
						case sagittal: 
							v1.s.Z = 1.0;
							break;
						}
						Vector3fT v2;
						Vector3fMultMat4(&v2, &v1, &m_transform);
						if (xDrag == 0 && yDrag == 0) m_delta = 0;
						else m_delta = ((xDrag * xDrag)+(yDrag * yDrag))/((v2.s.X*xDrag)+(v2.s.Y*yDrag));
					}
				}
				Refresh(false);
		    }
			else {
				m_isrDragging = false;
			}
			
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
			    }
				
				float *dots = new float[8];
				Vector3fT v1 = {0,0,1};
				Vector3fT v2 = {1,1,1};
				Vector3fT view;
				
				Vector3fMultMat4(&view, &v1, &m_transform);
				dots[0] = Vector3fDot(&v2, &view);
				
				v2.s.Z = -1;
				dots[1] = Vector3fDot(&v2, &view);
				
				v2.s.Y = -1;
				dots[2] = Vector3fDot(&v2, &view);
				
				v2.s.Z = 1;
				dots[3] = Vector3fDot(&v2, &view);
				
				v2.s.X = -1;
				dots[4] = Vector3fDot(&v2, &view);
				
				v2.s.Z = -1;
				dots[5] = Vector3fDot(&v2, &view);
				
				v2.s.Y = 1;
				dots[6] = Vector3fDot(&v2, &view);
				
				v2.s.Z = 1;
				dots[7] = Vector3fDot(&v2, &view);
				
				float max = 0.0;
				int quadrant = 0;
				for (int i = 0 ; i < 8 ; ++i)
				{
					if (dots[i] > max) 
					{
						max = dots[i];
						quadrant = i+1;
					}
				}
				m_scene->setQuadrant(quadrant);
				m_scene->setLightPos(view);
				
				Refresh(false);
			}
			else 
				m_isDragging = false;
		} break;
		
		case axial:
		case coronal:
		case sagittal:
			m_clicked = event.GetPosition();
			if (event.LeftUp() || event.Dragging()) 
			{
				//event1.SetEventObject( (wxObject*) new wxPoint( event.GetPosition()) );
				GetEventHandler()->ProcessEvent( event1 );
			}
			break;
		default: ;
	}
	
}

int MainCanvas::pick(wxPoint click)
{
	glPushMatrix();
	glMultMatrixf(m_transform.M);	
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY;

	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetIntegerv( GL_VIEWPORT, viewport );

	winX = (float)click.x;
	winY = (float)viewport[3] - (float)click.y;
	
	gluUnProject( winX, winY, 0, modelview, projection, viewport, &m_pos1X, &m_pos1Y, &m_pos1Z);
	gluUnProject( winX, winY, 1, modelview, projection, viewport, &m_pos2X, &m_pos2Y, &m_pos2Z);
	glPopMatrix();
	
	float x = (float)TheDataset::columns/2;
	float y = (float)TheDataset::rows/2;
	float z = (float)TheDataset::frames/2;
	
	float xx = m_scene->m_xSlize - x;
	float yy = m_scene->m_ySlize - y;
	float zz = m_scene->m_zSlize - z;
	
	m_tpicked = 0;
	int picked = 0;
	if (testBB(-x, -y, zz, x, y, zz)) {
		m_tpicked = m_tmin;
		picked = axial;
	}
	if (testBB(-x, yy, -z, x, yy, z)) {
		if (picked == 0) {
			picked = coronal;
			m_tpicked = m_tmin;
		}
		else {
			if (m_tmin < m_tpicked) {
				picked = coronal;
				m_tpicked = m_tmin;
			}
		}
	}
	if (testBB(xx, -y, -z, xx, y, z)) {
		if (picked == 0) {
			picked = sagittal;
			m_tpicked = m_tmin;
		}
		else {
			if (m_tmin < m_tpicked) {
				picked = sagittal;
				m_tpicked = m_tmin;
			}
		}
	}
	return picked;
}

bool MainCanvas::testBB(float bx1, float by1, float bz1, float bx2, float by2, float bz2)
{
	float tymin, tymax, tzmin, tzmax;
	float dirx = m_pos2X - m_pos1X;
	if (dirx >= 0) {
		m_tmin = ( bx1 - m_pos1X)/dirx;
		m_tmax = ( bx2 - m_pos1X)/dirx;
	}
	else {
		m_tmin = ( bx2 - m_pos1X)/dirx;
		m_tmax = ( bx1 - m_pos1X)/dirx;
	}
	float diry = m_pos2Y - m_pos1Y;
	if (diry >= 0) {
		tymin = ( by1 - m_pos1Y)/diry;
		tymax = ( by2 - m_pos1Y)/diry;
	}
	else {
		tymin = ( by2 - m_pos1Y)/diry;
		tymax = ( by1 - m_pos1Y)/diry;
	}
	if ( (m_tmin > tymax) || (tymin > m_tmax)) return false;
	if (tymin > m_tmin) m_tmin = tymin;
	if (tymax < m_tmax) m_tmax = tymax;
	float dirz = m_pos2Z - m_pos1Z;
	if (dirz >= 0) {
		tzmin = ( bz1 - m_pos1Z)/dirz;
		tzmax = ( bz2 - m_pos1Z)/dirz;
	}
	else {
		tzmin = ( bz2 - m_pos1Z)/dirz;
		tzmax = ( bz1 - m_pos1Z)/dirz;
	}
	if ( (m_tmin > tzmax) || (tzmin > m_tmax)) return false;
	if (tzmin > m_tmin) m_tmin = tzmin;
	if (tzmax < m_tmax) m_tmax = tzmax;
	
	if (m_tmin > m_tmax) return false;
	return true;
}

void MainCanvas::OnEraseBackground( wxEraseEvent& WXUNUSED(event) )
{
    // Do nothing, to avoid flashing.
}

void MainCanvas::render()
{
	wxPaintDC dc(this);

    SetCurrent(*m_scene->getMainGLContext());
	
	int w, h;
    GetClientSize(&w, &h);
    glViewport(0, 0, (GLint) w, (GLint) h);
	
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
    	glMultMatrixf(m_transform.M);										// NEW: Apply Dynamic Transform
    	m_scene->renderScene();
    	
    	renderTestRay();
    		
    	
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
	if (m_scene->m_texAssigned) {
		SetCurrent(*m_scene->getMainGLContext());
		m_scene->releaseTextures();
		m_scene->m_texAssigned = false;
	}
	m_init = false;
}


void MainCanvas::renderTestRay()
{
	glBegin(GL_LINES);
		glVertex3f(m_pos1X, m_pos1Y, m_pos1Z);
		glVertex3f(m_pos2X, m_pos2Y, m_pos2Z);
	glEnd();
	Vector3fT dir = {m_pos2X - m_pos1X, m_pos2Y- m_pos1Y, m_pos2Z - m_pos1Z};
	m_scene->drawSphere(m_pos1X + m_tpicked*dir.s.X, m_pos1Y + m_tpicked*dir.s.Y, m_pos1Z + m_tpicked*dir.s.Z, 3.0);
}

