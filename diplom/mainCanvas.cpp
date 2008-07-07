#include "mainCanvas.h"
#include "myListCtrl.h"
#include "wx/utils.h"
#include "point.h"

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
	TheDataset::m_transform = m_transform1;

	Matrix3fT idMat = {{  1.0f,  0.0f,  0.0f,
	                     0.0f,  1.0f,  0.0f,
	                     0.0f,  0.0f,  1.0f }};
	/*
	Matrix3fT lastRot1   = {  -0.62f,  0.25f,  -0.75f,
			                   0.78f,  0.32f,  -0.54f,
			                  -0.11f, -0.91f,  -0.39f};
	*/
	m_thisRot =idMat;
	m_lastRot =idMat;

	m_isDragging = false;					                    // NEW: Dragging The Mouse?
	m_isrDragging = false;
	m_delta = 0;
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
			if (event.RightIsDown())
		    {
				if (!m_isrDragging)												// Not Dragging
			    {
					m_isrDragging = true;										// Prepare For Dragging
					m_lastPos = event.GetPosition();
					m_hr = pick(event.GetPosition());
					if (wxGetKeyState(WXK_CONTROL) && (m_hr.picked > 10)) {
						((SelectionBox*)m_hr.object)->setPicked(10);
						m_hr.picked = 10;
					}
			    }
				else {
					if (event.Dragging() && m_hr.picked < 10)
					{
						int xDrag = m_lastPos.x - clickX;
						int yDrag = (m_lastPos.y - clickY);
						GetEventHandler()->ProcessEvent( event1 );

						Vector3fT n = {{0,0,0}};
						switch (m_hr.picked) {
						case axial:
							n.s.X = 1.0;
							break;
						case coronal:
							n.s.Y = 1.0;
							break;
						case sagittal:
							n.s.Z = 1.0;
							break;
						}
						if (xDrag == 0 && yDrag == 0) m_delta = 0;
						else {
							m_delta = 0;
							float delta =  wxMax(wxMin(getAxisParallelMovement(m_lastPos.x, m_lastPos.y, clickX, clickY, n ),1),-1);
							if ( delta < -0.5)
								m_delta =  -1;
							else if (delta > 0.5)
								m_delta = 1;
						}
					}
					else if (event.Dragging() && m_hr.picked >= 10 && m_hr.picked < 20)
					{
						((SelectionBox*)m_hr.object)->processDrag(event.GetPosition(), m_lastPos);
						m_scene->m_selBoxChanged = true;
					}
					else if (event.Dragging() && m_hr.picked == 20)
					{
						((Point*)m_hr.object)->drag(event.GetPosition());
					}
				}
				m_lastPos = event.GetPosition();
				Refresh(false);
		    }
			else {
				m_isrDragging = false;
			}

			if(event.LeftIsDown())
			{
				m_mousePt.s.X = clickX;
				m_mousePt.s.Y = clickY;
				if (m_scene->getPointMode() && wxGetKeyState(WXK_CONTROL)) {
					m_hr = pick(event.GetPosition());
					if (m_hr.hit && (m_hr.picked <= sagittal)) {
						m_hr.picked = 20;
						GetEventHandler()->ProcessEvent( event1 );
					}
				}
				else {
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
			            Matrix4fSetRotationFromMatrix3f(&TheDataset::m_transform, &m_thisRot);	// Set Our Final Transform's Rotation From This One
				    }

					float *dots = new float[8];
					Vector3fT v1 = {{0,0,1}};
					Vector3fT v2 = {{1,1,1}};
					Vector3fT view;

					Vector3fMultMat4(&view, &v1, &TheDataset::m_transform);
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
				GetEventHandler()->ProcessEvent( event1 );
			}
			break;
		default: ;
	}

}

float MainCanvas::getAxisParallelMovement(int x1, int y1, int x2, int y2, Vector3fT n)
{
	Vector3fT vs = mapMouse2World(x1, y1);
	Vector3fT ve = mapMouse2World(x2, y2);
	Vector3fT dir = {{ve.s.X - vs.s.X, ve.s.Y - vs.s.Y, ve.s.Z - vs.s.Z}};
	float bb = ((dir.s.X * dir.s.X) + (dir.s.Y * dir.s.Y) + (dir.s.Z * dir.s.Z));
	float nb = ((dir.s.X * n.s.X) + (dir.s.Y * n.s.Y) + (dir.s.Z * n.s.Z));
	return bb/nb;
}

Vector3fT MainCanvas::mapMouse2World(int x, int y)
{
	glPushMatrix();
	glMultMatrixf(TheDataset::m_transform.M);
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY;

	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetIntegerv( GL_VIEWPORT, viewport );

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	GLdouble posX, posY, posZ;
	gluUnProject( winX, winY, 0, modelview, projection, viewport, &posX, &posY, &posZ);
	glPopMatrix();
	Vector3fT v = {{posX, posY, posZ}};
	return v;
}


hitResult MainCanvas::pick(wxPoint click)
{
	glPushMatrix();
	glMultMatrixf(TheDataset::m_transform.M);
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
	Ray *ray = new Ray( m_pos1X, m_pos1Y, m_pos1Z, m_pos2X, m_pos2Y, m_pos2Z );
	BoundingBox *bb = new BoundingBox(0,0,0, TheDataset::columns, TheDataset::rows, TheDataset::frames);

	float xx = m_scene->m_xSlize - TheDataset::columns/2;
	float yy = m_scene->m_ySlize - TheDataset::rows/2;
	float zz = m_scene->m_zSlize - TheDataset::frames/2;

	/**
	 * check if one of the 3 planes is picked
	 */
	float tpicked = 0;
	int picked = 0;
	hitResult hr;
	if (m_scene->m_showAxial) {
		bb->setSizeZ(0);
		bb->setCenterZ(zz);
		hr = bb->hitTest(ray);
		if (hr.hit) {
			tpicked = hr.tmin;
			picked = axial;
		}
		bb->setSizeZ(TheDataset::frames);
		bb->setCenterZ(0);
	}
	if (m_scene->m_showCoronal) {
		bb->setSizeY(0);
		bb->setCenterY(yy);
		hr = bb->hitTest(ray);
		if (hr.hit) {
			if (picked == 0) {
				picked = coronal;
				tpicked = hr.tmin;
			}
			else {
				if (hr.tmin < tpicked) {
					picked = coronal;
					tpicked = hr.tmin;
				}
			}
		}
		bb->setSizeY(TheDataset::rows);
		bb->setCenterY(0);
	}
	if (m_scene->m_showSagittal) {
		bb->setSizeX(0);
		bb->setCenterX(xx);
		hr = bb->hitTest(ray);
		if (hr.hit) {
			if (picked == 0) {
				picked = sagittal;
				tpicked = hr.tmin;
			}
			else {
				if (hr.tmin < tpicked) {
					picked = sagittal;
					tpicked = hr.tmin;
				}
			}
		}
	}
	if (hr.hit) {
		hr.tmin = tpicked;
		hr.picked = picked;
	}

	/*
	 * check for hits with the selection box sizers
	 */
	std::vector<std::vector<SelectionBox*> > boxes = m_scene->getSelectionBoxes();
	for (uint i = 0 ; i < boxes.size() ; ++i)
	{
		for (uint j = 0 ; j < boxes[i].size() ; ++j)
		{
			hitResult hr1 = boxes[i][j]->hitTest(ray);
			if (hr1.hit && !hr.hit) hr = hr1;
			else if (hr1.hit && hr.hit && (hr1.tmin < hr.tmin)) hr = hr1;
		}
	}

	if (m_scene->getPointMode()) {
		int countPoints = m_scene->m_treeWidget->GetChildrenCount(m_scene->m_tPointId, true);
		wxTreeItemId id, childid;
		wxTreeItemIdValue cookie = 0;
		for (int i = 0 ; i < countPoints ; ++i)
		{
			id = m_scene->m_treeWidget->GetNextChild(m_scene->m_tPointId, cookie);
			Point *point = (Point*)((MyTreeItemData*)m_scene->m_treeWidget->GetItemData(id))->getData();
			hitResult hr1 = point->hitTest(ray);
			if (hr1.hit && !hr.hit) hr = hr1;
			else if (hr1.hit && hr.hit && (hr1.tmin < hr.tmin)) hr = hr1;
		}
	}

	return hr;


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
    	glMultMatrixf(TheDataset::m_transform.M);										// NEW: Apply Dynamic Transform
    	m_scene->renderScene();
    	//renderTestRay();
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
	Vector3fT dir = {{m_pos2X - m_pos1X, m_pos2Y- m_pos1Y, m_pos2Z - m_pos1Z}};
	m_scene->drawSphere(m_pos1X + m_hr.tmin*dir.s.X, m_pos1Y + m_hr.tmin*dir.s.Y, m_pos1Z + m_hr.tmin*dir.s.Z, 3.0);
}

Vector3fT MainCanvas::getEventCenter()
{
	Vector3fT dir = {{m_pos2X - m_pos1X, m_pos2Y- m_pos1Y, m_pos2Z - m_pos1Z}};
	Vector3fT center = {{m_pos1X + m_hr.tmin*dir.s.X, m_pos1Y + m_hr.tmin*dir.s.Y, m_pos1Z + m_hr.tmin*dir.s.Z}};
	return center;
}
