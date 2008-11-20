#include "mainCanvas.h"

#include "myListCtrl.h"
#include "wx/utils.h"

#include "splinePoint.h"

#include "mainFrame.h"

DECLARE_EVENT_TYPE(wxEVT_NAVGL_EVENT, -1)
DEFINE_EVENT_TYPE(wxEVT_NAVGL_EVENT)

BEGIN_EVENT_TABLE(MainCanvas, wxGLCanvas)
    EVT_SIZE(MainCanvas::OnSize)
    EVT_PAINT(MainCanvas::OnPaint)
    EVT_MOUSE_EVENTS(MainCanvas::OnMouseEvent)
    EVT_ERASE_BACKGROUND(MainCanvas::OnEraseBackground)
END_EVENT_TABLE()

MainCanvas::MainCanvas(DatasetHelper* dh, int view, wxWindow *parent, wxWindowID id,
#ifdef CTX
    const wxPoint& pos, const wxSize& size, long style, const wxString& name, int* gl_attrib, wxGLContext*ctx)
    : wxGLCanvas(parent, ctx, id,
        wxDefaultPosition, wxDefaultSize, 0, name) // gl_attrib, pos, size, style|wxFULL_REPAINT_ON_RESIZE, name )
#else
    const wxPoint& pos, const wxSize& size, long style, const wxString& name, int* gl_attrib, wxGLCanvas*shared )
    : wxGLCanvas(parent, shared, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE, name, gl_attrib )
#endif
{
	m_init = false;
	m_view = view;
	m_dh = dh;

	m_lastRot.M[0] = -0.67698019742965698242f; m_lastRot.M[1] =  0.48420974612236022949f; m_lastRot.M[2] = -0.55429106950759887695;
	m_lastRot.M[3] =  0.73480975627899169922f; m_lastRot.M[4] =  0.40184235572814941406f; m_lastRot.M[5] = -0.54642277956008911133f;
	m_lastRot.M[6] = -0.04184586182236671448f; m_lastRot.M[7] = -0.77721565961837768555f; m_lastRot.M[8] = -0.62784034013748168945f;

	Matrix4fSetIdentity(&m_dh->m_transform);
	Matrix3fSetIdentity(&m_thisRot);
	Matrix3fMulMatrix3f(&m_thisRot, &m_lastRot);
	Matrix4fSetRotationFromMatrix3f(&m_dh->m_transform, &m_lastRot);

	m_delta = 0;
	m_arcBall = new ArcBallT(640.0f, 480.0f);
}

MainCanvas::~MainCanvas()
{
	switch(m_view) {
	case mainView :
		m_dh->printDebug(_T("execute main canvas destructor: main view"), 0);
		break;
	case axial:
		m_dh->printDebug(_T("execute main canvas destructor: axial view"), 0);
		break;
	case coronal:
		m_dh->printDebug(_T("execute main canvas destructor: coronal view"), 0);
		break;
	case sagittal:
		m_dh->printDebug(_T("execute main canvas destructor: sagittal view"), 0);
		break;
	}
}

void MainCanvas::init()
{
	m_dh->scene->initGL(m_view);
	m_init = true;
}

void MainCanvas::changeOrthoSize(int value)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho( 0, value, 0, value, -3000, 3000);
}


void MainCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    render();
}

void MainCanvas::OnSize(wxSizeEvent& event)
{
#ifndef __WXMAC__
    if (!m_dh->scene->m_texAssigned)
    		wxGLCanvas::SetCurrent();
#ifndef __WXMAC__
	else
		wxGLCanvas::SetCurrent(*m_dh->scene->getMainGLContext());
#else
    this->SetCurrent();
#endif
#else
    SetCurrent();
#endif

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
			if(event.LeftIsDown())
			{
				m_mousePt.s.X = clickX;
				m_mousePt.s.Y = clickY;

				if (wxGetKeyState(WXK_CONTROL)) {
					m_hr = pick(event.GetPosition());
					m_dh->updateView((int)getEventCenter().s.X , (int)getEventCenter().s.Y , (int)getEventCenter().s.Z);
					m_dh->mainFrame->m_xSlider->SetValue((int)getEventCenter().s.X);
					m_dh->mainFrame->m_ySlider->SetValue((int)getEventCenter().s.Y);
					m_dh->mainFrame->m_zSlider->SetValue((int)getEventCenter().s.Z);
					m_dh->mainFrame->refreshAllGLWidgets();
				}

				if (m_dh->getPointMode() && wxGetKeyState(WXK_CONTROL)) {
					m_hr = pick(event.GetPosition());
					if (m_hr.hit && (m_hr.picked <= sagittal)) {
						m_hr.picked = 20;
						SplinePoint *point = new SplinePoint(getEventCenter(), m_dh);
						wxTreeItemId pId = m_dh->mainFrame->m_treeWidget->AppendItem(m_dh->mainFrame->m_tPointId, wxT("point"),-1, -1, point);
						point->setTreeId(pId);

						GetEventHandler()->ProcessEvent( event1 );
					}
				}

				else {
					if (!m_dh->m_isDragging)												// Not Dragging
					{
						m_dh->m_isDragging = true;										// Prepare For Dragging
						m_lastRot = m_thisRot;										// Set Last Static Rotation To Last Dynamic One
						m_arcBall->click(&m_mousePt);								// Update Start Vector And Prepare For Dragging
					}
					else
					{
						orthonormalize();
						Quat4fT     ThisQuat;
						m_arcBall->drag(&m_mousePt, &ThisQuat);						// Update End Vector And Get Rotation As Quaternion
						Matrix3fSetRotationFromQuat4f(&m_thisRot, &ThisQuat);		// Convert Quaternion Into Matrix3fT
						Matrix3fMulMatrix3f(&m_thisRot, &m_lastRot);				// Accumulate Last Rotation Into This One
						Matrix4fSetRotationFromMatrix3f(&m_dh->m_transform, &m_thisRot);	// Set Our Final Transform's Rotation From This One
					}

					updateView();
					Refresh(false);
				}
			}
			else
				m_dh->m_isDragging = false;


			if (event.MiddleIsDown())
			{
				if (!m_dh->m_ismDragging)
				{
					m_dh->m_ismDragging = true;
					m_lastPos = event.GetPosition();
				}
				else
				{
					int xDrag = m_lastPos.x - clickX;
					int yDrag = (m_lastPos.y - clickY);
					m_lastPos = event.GetPosition();
					m_dh->moveScene(xDrag, yDrag);
					Refresh(false);
				}
			}
			else
			{
				m_dh->m_ismDragging = false;
			}

			if (event.GetWheelDelta() != 0)
			{
				m_dh->changeZoom(event.GetWheelRotation());
				Refresh(false);
			}

			if (event.RightIsDown())
		    {
				if (!m_dh->m_isrDragging)												// Not Dragging
			    {
					m_dh->m_isrDragging = true;										// Prepare For Dragging
					m_lastPos = event.GetPosition();
					m_hr = pick(event.GetPosition());

					if (m_hr.picked == 20)
					{
						if (m_dh->lastSelectedPoint) m_dh->lastSelectedPoint->unselect();
						m_dh->lastSelectedPoint = ((SplinePoint*)m_hr.object);

						((SplinePoint*)m_hr.object)->select(true);
					}
					else if (m_hr.picked >= 10 && m_hr.picked < 20)
					{
						if (m_dh->lastSelectedPoint) m_dh->lastSelectedPoint->unselect();

						((SelectionBox*)m_hr.object)->select(true);
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
							n.s.Z = 1.0;
							break;
						case coronal:
							n.s.Y = 1.0;
							break;
						case sagittal:
							n.s.X = 1.0;
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
						m_dh->scene->m_selBoxChanged = true;
					}
					else if (event.Dragging() && m_hr.picked == 20)
					{
						((SplinePoint*)m_hr.object)->drag(event.GetPosition());
					}
				}
				m_lastPos = event.GetPosition();
				Refresh(false);
		    }
			else {
				m_dh->m_isrDragging = false;
			}

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

void MainCanvas::updateView()
{
	float *dots = new float[8];
	Vector3fT v1 = {{0,0,1}};
	Vector3fT v2 = {{1,1,1}};
	Vector3fT view;

	Vector3fMultMat4(&view, &v1, &m_dh->m_transform);
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
	m_dh->quadrant = quadrant;
}

float MainCanvas::getAxisParallelMovement(int x1, int y1, int x2, int y2, Vector3fT n)
{
	Vector3fT vs = m_dh->mapMouse2World(x1, y1);
	Vector3fT ve = m_dh->mapMouse2World(x2, y2);
	Vector3fT dir = {{ve.s.X - vs.s.X, ve.s.Y - vs.s.Y, ve.s.Z - vs.s.Z}};
	float bb = ((dir.s.X * dir.s.X) + (dir.s.Y * dir.s.Y) + (dir.s.Z * dir.s.Z));
	float nb = ((dir.s.X * n.s.X) + (dir.s.Y * n.s.Y) + (dir.s.Z * n.s.Z));
	return bb/nb;
}

hitResult MainCanvas::pick(wxPoint click)
{
	glPushMatrix();

	m_dh->doMatrixManipulation();

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
	BoundingBox *bb = new BoundingBox(m_dh->columns/2, m_dh->rows/2, m_dh->frames/2, m_dh->columns, m_dh->rows, m_dh->frames);

	float xx = m_dh->xSlize;
	float yy = m_dh->ySlize;
	float zz = m_dh->zSlize;

	/**
	 * check if one of the 3 planes is picked
	 */
	float tpicked = 0;
	int picked = 0;
	hitResult hr = {false, 0.0f, 0, NULL};
	if (m_dh->showAxial) {
		bb->setSizeZ(0.01f);
		bb->setCenterZ(zz);
		hr = bb->hitTest(ray);
		if (hr.hit) {
			tpicked = hr.tmin;
			picked = axial;
		}
		bb->setSizeZ(m_dh->frames);
		bb->setCenterZ(m_dh->frames/2);
	}

	if (m_dh->showCoronal) {
		bb->setSizeY(0.01f);
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
		bb->setSizeY(m_dh->rows);
		bb->setCenterY(m_dh->rows/2);
	}

	if (m_dh->showSagittal) {
		bb->setSizeX(0.01f);
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
	if (picked != 0) {
		hr.tmin = tpicked;
		hr.picked = picked;
	}

	/*
	 * check for hits with the selection box sizers
	 */
	if (m_dh->showBoxes)
	{
		std::vector<std::vector<SelectionBox*> > boxes = m_dh->getSelectionBoxes();
		for (unsigned int i = 0 ; i < boxes.size() ; ++i)
		{
			for (unsigned int j = 0 ; j < boxes[i].size() ; ++j)
			{
				hitResult hr1 = boxes[i][j]->hitTest(ray);
				if (hr1.hit && !hr.hit) hr = hr1;
				else if (hr1.hit && hr.hit && (hr1.tmin < hr.tmin)) hr = hr1;
			}
		}
	}
	/*
	 * check for hits with points for spline surface
	 */
	if (m_dh->pointMode)
	{
		wxTreeItemId id, childid;
		wxTreeItemIdValue cookie = 0;
		id = m_dh->mainFrame->m_treeWidget->GetFirstChild(m_dh->mainFrame->m_tPointId, cookie);
		while ( id.IsOk() )
		{
			SplinePoint *point = (SplinePoint*)(m_dh->mainFrame->m_treeWidget->GetItemData(id));
			hitResult hr1 = point->hitTest(ray);
			if (hr1.hit && !hr.hit) hr = hr1;
			else if (hr1.hit && hr.hit && (hr1.tmin < hr.tmin)) hr = hr1;

			id = m_dh->mainFrame->m_treeWidget->GetNextChild(m_dh->mainFrame->m_tPointId, cookie);
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

#ifndef __WXMAC__
    SetCurrent(*m_dh->scene->getMainGLContext());
#else
    SetCurrent();
#endif
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
    	// TODO marker
    	glPushMatrix();
    	m_dh->doMatrixManipulation();
    	m_dh->scene->renderScene();
    	//renderTestRay();
	    glPopMatrix();
	    break;
    }
    default:
    	m_dh->scene->renderNavView(m_view);
    }
	//glFlush();

	SwapBuffers();
}

void MainCanvas::invalidate()
{
	if (m_dh->scene->m_texAssigned) {
#ifndef __WXMAC__
		SetCurrent(*m_dh->scene->getMainGLContext());
#else
        SetCurrent();
#endif
		//m_dh->scene->releaseTextures();
		m_dh->scene->m_texAssigned = false;
	}
	m_init = false;
}


void MainCanvas::renderTestRay()
{
	if (m_hr.tmin == 0)
		glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
		glVertex3f(m_pos1X, m_pos1Y, m_pos1Z);
		glVertex3f(m_pos2X, m_pos2Y, m_pos2Z);
	glEnd();
	Vector3fT dir = {{m_pos2X - m_pos1X, m_pos2Y- m_pos1Y, m_pos2Z - m_pos1Z}};
	m_dh->scene->drawSphere(m_pos1X + m_hr.tmin*dir.s.X, m_pos1Y + m_hr.tmin*dir.s.Y, m_pos1Z + m_hr.tmin*dir.s.Z, 3.0);
}

Vector3fT MainCanvas::getEventCenter()
{
	Vector3fT dir = {{m_pos2X - m_pos1X, m_pos2Y- m_pos1Y, m_pos2Z - m_pos1Z}};
	Vector3fT center = {{m_pos1X + m_hr.tmin*dir.s.X, m_pos1Y + m_hr.tmin*dir.s.Y, m_pos1Z + m_hr.tmin*dir.s.Z}};
	return center;
}

void MainCanvas::setRotation()
{
	m_lastRot.s.M00 = m_dh->m_transform.s.M00;
	m_lastRot.s.M01 = m_dh->m_transform.s.M01;
	m_lastRot.s.M02 = m_dh->m_transform.s.M02;
	m_lastRot.s.M10 = m_dh->m_transform.s.M10;
	m_lastRot.s.M11 = m_dh->m_transform.s.M11;
	m_lastRot.s.M12 = m_dh->m_transform.s.M12;
	m_lastRot.s.M20 = m_dh->m_transform.s.M20;
	m_lastRot.s.M21 = m_dh->m_transform.s.M21;
	m_lastRot.s.M22 = m_dh->m_transform.s.M22;

	orthonormalize();

	Matrix4fSetIdentity(&m_dh->m_transform);
	Matrix3fSetIdentity(&m_thisRot);
	Matrix3fMulMatrix3f(&m_thisRot, &m_lastRot);
	Matrix4fSetRotationFromMatrix3f(&m_dh->m_transform, &m_lastRot);
	updateView();
	m_dh->mainFrame->refreshAllGLWidgets();
}

void MainCanvas::orthonormalize()
{
	F::FVector v1 = F::FVector(m_lastRot.s.M00, m_lastRot.s.M01, m_lastRot.s.M02);
	F::FVector v2 = F::FVector(m_lastRot.s.M10, m_lastRot.s.M11, m_lastRot.s.M12);
	F::FVector v3 = F::FVector(m_lastRot.s.M20, m_lastRot.s.M21, m_lastRot.s.M22);

	F::FVector u1 = v1;
	F::FVector u2 = v2 - ( ( scalar(v2, u1)/ scalar(u1,u1) ) * u1 );
	F::FVector u3 = v3 - ( ( scalar(v3, u1)/ scalar(u1,u1) ) * u1 ) - ( ( scalar(v3, u2)/ scalar(u2,u2) ) * u2 );

	m_lastRot.s.M00 = u1[0];
	m_lastRot.s.M01 = u1[1];
	m_lastRot.s.M02 = u1[2];
	m_lastRot.s.M10 = u2[0];
	m_lastRot.s.M11 = u2[1];
	m_lastRot.s.M12 = u2[2];
	m_lastRot.s.M20 = u3[0];
	m_lastRot.s.M21 = u3[1];
	m_lastRot.s.M22 = u3[2];

}

double MainCanvas::scalar(const F::FVector& v1, const F::FVector& v2)
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void MainCanvas::testRender(GLuint tex)
{
	wxPaintDC dc(this);

#ifndef __WXMAC__
	SetCurrent(*m_dh->scene->getMainGLContext());
#else
    SetCurrent();
#endif

	int w, h;
	GetClientSize(&w, &h);
	glViewport(0, 0, (GLint) w, (GLint) h);

	/* clear color and depth buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3f(1.0, 1.0, 1.0);

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0001f);

	glEnable(GL_TEXTURE_2D);

	// bind texture
	glBindTexture(GL_TEXTURE_2D, tex);

	// setup wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// we do not want linear/mip mapped texture filters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// texture environment
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	int x = m_dh->columns;
	int y = m_dh->rows;

	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 1.0); glVertex3f( 0, 0, 0);
		glTexCoord2f(0.0, 0.0); glVertex3f( 0, y, 0);
		glTexCoord2f(1.0, 0.0); glVertex3f( x, y, 0);
		glTexCoord2f(1.0, 1.0); glVertex3f( x, 0, 0);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	glPopAttrib();

	glFlush();

	SwapBuffers();
}
