#include "mainCanvas.h"

#include "myListCtrl.h"
#include "wx/utils.h"

#include "splinePoint.h"
#include "lic/FgeOffscreen.h"

#include "mainFrame.h"

DECLARE_EVENT_TYPE(wxEVT_NAVGL_EVENT, -1)
DEFINE_EVENT_TYPE(wxEVT_NAVGL_EVENT)

BEGIN_EVENT_TABLE(MainCanvas, wxGLCanvas)
    EVT_SIZE(MainCanvas::OnSize)
    EVT_PAINT(MainCanvas::OnPaint)
    EVT_MOUSE_EVENTS(MainCanvas::OnMouseEvent)
    EVT_ERASE_BACKGROUND(MainCanvas::OnEraseBackground)
    EVT_CHAR(MainCanvas::OnChar)
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
#ifndef __WXMAC__
	// i'm still not sure which GLX version is needed, but 1.3 seems to be ok
	// have to do this here, because wxGLCanvas::GetGLXVersion doesn't return reliable results when called 
	// in the init function in main.cpp
	if ( GetGLXVersion() < 13 )
	{
		printf("Found GLX version: %d\n", GetGLXVersion());
		printf("OpenGL and a sufficient graphics card need to be installed to run this programm.\n");
		exit(false);
	}
#endif
	m_init = false;
	m_view = view;
	m_dh = dh;
/*
	m_lastRot.M[0] = -0.67698019742965698242f; m_lastRot.M[1] =  0.48420974612236022949f; m_lastRot.M[2] = -0.55429106950759887695;
	m_lastRot.M[3] =  0.73480975627899169922f; m_lastRot.M[4] =  0.40184235572814941406f; m_lastRot.M[5] = -0.54642277956008911133f;
	m_lastRot.M[6] = -0.04184586182236671448f; m_lastRot.M[7] = -0.77721565961837768555f; m_lastRot.M[8] = -0.62784034013748168945f;
*/
	m_lastRot.M[0] = -0.66625452041625976562f; m_lastRot.M[1] = 0.42939949035644531250f;  m_lastRot.M[2] = -0.60968911647796630859f;
	m_lastRot.M[3] = -0.74149495363235473633f; m_lastRot.M[4] = -0.46842813491821289062f; m_lastRot.M[5] = 0.48037606477737426758f;
	m_lastRot.M[6] = -0.07932166755199432373f; m_lastRot.M[7] = 0.77213370800018310547f;  m_lastRot.M[8] = 0.63048923015594482422f;


	Matrix4fSetIdentity(&m_dh->m_transform);
	Matrix3fSetIdentity(&m_thisRot);
	Matrix3fMulMatrix3f(&m_thisRot, &m_lastRot);
	Matrix4fSetRotationFromMatrix3f(&m_dh->m_transform, &m_lastRot);

	m_delta = 0;
	m_arcBall = new ArcBallT(640.0f, 480.0f);
	
	orthoSizeNormal = 200;
	orthoModX = 0;
	orthoModY = 0;
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

void MainCanvas::changeOrthoSize()
{
	orthoSizeNormal = wxMax(wxMax(m_dh->rows, m_dh->columns), m_dh->frames);
		
	if (m_view == mainView)
	{
		orthoModX = 0;
		orthoModY = 0;
		int xSize = GetSize().x;
		int ySize = GetSize().y;
		float ratio = (float)xSize / (float)ySize;
		if (ratio > 1.0)
			orthoModX = ((int)(orthoSizeNormal * ratio) - orthoSizeNormal)/2;
		else
			orthoModY = ((int)(orthoSizeNormal * (1.0 + (1.0 - ratio)) ) - orthoSizeNormal)/2;		
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho( 0, orthoSizeNormal, 0, orthoSizeNormal, -500, 500);
	
	
}


void MainCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    render();
}

void MainCanvas::OnSize(wxSizeEvent& event)
{
#ifndef __WXMAC__
    if (!m_dh->m_texAssigned)
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
			if(event.LeftUp())
			{
				if (wxGetKeyState(WXK_CONTROL) && !m_dh->getPointMode()) {
					m_hr = pick(event.GetPosition());
					int newX = (int)(getEventCenter().x + 0.5);
					int newY = (int)(getEventCenter().y + 0.5);
					int newZ = (int)(getEventCenter().z + 0.5);
					m_dh->updateView( newX, newY , newZ);
					m_dh->mainFrame->m_xSlider->SetValue(newX);
					m_dh->mainFrame->m_ySlider->SetValue(newY);
					m_dh->mainFrame->m_zSlider->SetValue(newZ);
					m_dh->mainFrame->refreshAllGLWidgets();
				}
				else if (wxGetKeyState(WXK_CONTROL) && m_dh->getPointMode()) {
					m_hr = pick(event.GetPosition());
					if (m_hr.hit && (m_hr.picked <= sagittal)) {
						m_hr.picked = 20;
						SplinePoint *point = new SplinePoint(getEventCenter(), m_dh);
						wxTreeItemId pId = m_dh->mainFrame->m_treeWidget->AppendItem(m_dh->mainFrame->m_tPointId, wxT("point"),-1, -1, point);
						point->setTreeId(pId);

						GetEventHandler()->ProcessEvent( event1 );
					}
				}

			}
			
			if(event.LeftIsDown())
			{
				//SetFocus();
				m_mousePt.s.X = clickX;
				m_mousePt.s.Y = clickY;

				if (!m_dh->m_isDragging)												// Not Dragging
				{
					m_dh->m_isDragging = true;										// Prepare For Dragging
					m_lastRot = m_thisRot;										// Set Last Static Rotation To Last Dynamic One
					m_arcBall->click(&m_mousePt);								// Update Start Vector And Prepare For Dragging
				}
				else
				{
					Quat4fT     ThisQuat;
					m_arcBall->drag(&m_mousePt, &ThisQuat);						// Update End Vector And Get Rotation As Quaternion
					Matrix3fSetRotationFromQuat4f(&m_thisRot, &ThisQuat);		// Convert Quaternion Into Matrix3fT
					Matrix3fMulMatrix3f(&m_thisRot, &m_lastRot);				// Accumulate Last Rotation Into This One
					Matrix4fSetRotationFromMatrix3f(&m_dh->m_transform, &m_thisRot);	// Set Our Final Transform's Rotation From This One
				}
	
				updateView();
				Refresh(false);
					
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
					if (wxGetKeyState(WXK_CONTROL) && wxGetKeyState(WXK_SHIFT))
					{
						printf("%2.8f : %2.8f : %2.8f \n", m_dh->m_transform.M[0], m_dh->m_transform.M[1], m_dh->m_transform.M[2]);
						printf("%2.8f : %2.8f : %2.8f \n", m_dh->m_transform.M[3], m_dh->m_transform.M[4], m_dh->m_transform.M[5]);
						printf("%2.8f : %2.8f : %2.8f \n", m_dh->m_transform.M[6], m_dh->m_transform.M[7], m_dh->m_transform.M[8]);
					}
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
					
					SetFocus();
				}
				else 
				{
					if (event.Dragging() && m_hr.picked < 10)
					{
						int xDrag = m_lastPos.x - clickX;
						int yDrag = (m_lastPos.y - clickY);
						
						Vector n ( 0, 0, 0 );
						switch (m_hr.picked) {
						case axial:
							n.z = 1.0;
							break;
						case coronal:
							n.y = 1.0;
							break;
						case sagittal:
							n.x = 1.0;
							break;
						}
						if (xDrag == 0 && yDrag == 0) 
							m_delta = 0;
						else 
						{
							m_delta = 0;
							float delta =  wxMax(wxMin(getAxisParallelMovement(m_lastPos.x, m_lastPos.y, clickX, clickY, n ),1),-1);
							if ( delta < -0.5)
								m_delta =  -1;
							else if (delta > 0.5)
								m_delta = 1;
							else m_delta = 0;
						}
						GetEventHandler()->ProcessEvent( event1 );
					}
					else if (event.Dragging() && m_hr.picked >= 10 && m_hr.picked < 20)
					{
						((SelectionBox*)m_hr.object)->processDrag(event.GetPosition(), m_lastPos);
						m_dh->m_selBoxChanged = true;
					}
					else if (event.Dragging() && m_hr.picked == 20)
					{
						((SplinePoint*)m_hr.object)->drag(event.GetPosition());
					}
				}
				m_lastPos = event.GetPosition();
				Refresh(false);
		    }
			else 
			{
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

float MainCanvas::getAxisParallelMovement(int x1, int y1, int x2, int y2, Vector n)
{
	Vector vs = m_dh->mapMouse2World(x1, y1);
	Vector ve = m_dh->mapMouse2World(x2, y2);
	Vector dir ( ve.x - vs.x, ve.y - vs.y, ve.z - vs.z );
	float bb = ((dir.x * dir.x) + (dir.y * dir.y) + (dir.z * dir.z));
	float nb = ((dir.x * n.x) + (dir.y * n.y) + (dir.z * n.z));
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
	if (m_dh->showAxial) 
	{
		bb->setSizeZ(0.01f);
		bb->setCenterZ(zz);
		hr = bb->hitTest(ray);
		if (hr.hit) 
		{
			tpicked = hr.tmin;
			picked = axial;
		}
		bb->setSizeZ(m_dh->frames);
		bb->setCenterZ(m_dh->frames/2);
	}

	if (m_dh->showCoronal) 
	{
		bb->setSizeY(0.01f);
		bb->setCenterY(yy);
		hr = bb->hitTest(ray);
		if (hr.hit) {
			if (picked == 0) 
			{
				picked = coronal;
				tpicked = hr.tmin;
			}
			else 
			{
				if (hr.tmin < tpicked) 
				{
					picked = coronal;
					tpicked = hr.tmin;
				}
			}
		}
		bb->setSizeY(m_dh->rows);
		bb->setCenterY(m_dh->rows/2);
	}

	if (m_dh->showSagittal) 
	{
		bb->setSizeX(0.01f);
		bb->setCenterX(xx);
		hr = bb->hitTest(ray);
		if (hr.hit) {
			if (picked == 0) 
			{
				picked = sagittal;
				tpicked = hr.tmin;
			}
			else 
			{
				if (hr.tmin < tpicked) 
				{
					picked = sagittal;
					tpicked = hr.tmin;
				}
			}
		}
	}
	if (picked != 0) 
	{
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
    	if ( m_dh->scheduledScreenshot)
    	{
    		int size = 0;
    		switch (m_dh->geforceLevel)
    		{
    		case 6:
    			size = 2000;
    			break;
    		case 8:
    			size = 4000;
    			break;
    		default:
    			size = 1000;
    			break;
    		}
    		
    		FgeOffscreen fbo(size, size, true);;
    		fbo.setClearColor(1.0, 1.0, 1.0);
    		fbo.activate();
    		
        	glMatrixMode(GL_PROJECTION);
        	glLoadIdentity();
        	glOrtho( 0, orthoSizeNormal, 0, orthoSizeNormal, -500, 500);
        	glViewport(0, 0, size, size);
        	
        	glPushMatrix();
        	m_dh->doMatrixManipulation();
        	m_dh->scene->renderScene();
    	    glPopMatrix();
    	    
    	    fbo.getTexObject(1)->saveImageToPPM((m_dh->m_screenshotName).mb_str());
    		fbo.deactivate();
    		m_dh->scheduledScreenshot = false;
    	}
    	else
    	{
    		glMatrixMode(GL_PROJECTION);
	    	glLoadIdentity();
	    	glOrtho( - orthoModX, orthoSizeNormal + orthoModX, - orthoModY, orthoSizeNormal + orthoModY, -500, 500);
	    	
	    	glPushMatrix();
	    	m_dh->doMatrixManipulation();
	    	
	    	m_dh->scene->renderScene();
	    	//renderTestRay();
		    glPopMatrix();
    	}
	   
	    break;
    }
    default:
    	glMatrixMode(GL_PROJECTION);
    	glLoadIdentity();
    	glOrtho( 0, orthoSizeNormal, 0, orthoSizeNormal, -500, 500);
    	m_dh->scene->renderNavView(m_view);
    }
	//glFlush();

	SwapBuffers();
}

void MainCanvas::invalidate()
{
	if (m_dh->m_texAssigned) {
#ifndef __WXMAC__
		SetCurrent(*m_dh->scene->getMainGLContext());
#else
        SetCurrent();
#endif
		//m_dh->scene->releaseTextures();
		m_dh->m_texAssigned = false;
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
	Vector dir ( m_pos2X - m_pos1X, m_pos2Y- m_pos1Y, m_pos2Z - m_pos1Z );
	m_dh->scene->drawSphere(m_pos1X + m_hr.tmin*dir.x, m_pos1Y + m_hr.tmin*dir.y, m_pos1Z + m_hr.tmin*dir.z, 3.0);
}

Vector MainCanvas::getEventCenter()
{
	Vector dir ( m_pos2X - m_pos1X, m_pos2Y- m_pos1Y, m_pos2Z - m_pos1Z );
	Vector center ( m_pos1X + m_hr.tmin*dir.x, m_pos1Y + m_hr.tmin*dir.y, m_pos1Z + m_hr.tmin*dir.z );
	/*
	printf("front: %f : %f : %f\n", m_pos1X, m_pos1Y, m_pos1Z);
	printf("back : %f : %f : %f\n", m_pos2X, m_pos2Y, m_pos2Z);
	printf("tmin : %f\n", m_hr.tmin);
	printf("dir  : %f : %f : %f\n", dir.x, dir.y, dir.z);
	printf("pos  : %f : %f : %f\n==========================================\n", center.x, center.y, center.z);
*/
	return center;
}

void MainCanvas::setRotation()
{
	m_thisRot.s.M00 = m_dh->m_transform.s.M00;
	m_thisRot.s.M01 = m_dh->m_transform.s.M01;
	m_thisRot.s.M02 = m_dh->m_transform.s.M02;
	m_thisRot.s.M10 = m_dh->m_transform.s.M10;
	m_thisRot.s.M11 = m_dh->m_transform.s.M11;
	m_thisRot.s.M12 = m_dh->m_transform.s.M12;
	m_thisRot.s.M20 = m_dh->m_transform.s.M20;
	m_thisRot.s.M21 = m_dh->m_transform.s.M21;
	m_thisRot.s.M22 = m_dh->m_transform.s.M22;

	Matrix4fSetIdentity(&m_dh->m_transform);
	Matrix3fSetIdentity(&m_lastRot);
	Matrix4fSetRotationFromMatrix3f(&m_dh->m_transform, &m_thisRot);

	
	updateView();
	m_dh->mainFrame->refreshAllGLWidgets();
}

void MainCanvas::OnChar(wxKeyEvent& event)
{
	int w, h;
	GetClientSize(&w, &h);
	Quat4fT     ThisQuat;
	
	if ( wxGetKeyState(WXK_CONTROL) )
	{
		m_mousePt.s.X = w/2;
		m_mousePt.s.Y = h/2;
		m_lastRot = m_thisRot;										// Set Last Static Rotation To Last Dynamic One
		m_arcBall->click(&m_mousePt);								// Update Start Vector And Prepare For Dragging
	}
	
	switch( event.GetKeyCode() )
		{
		case WXK_LEFT:
			if ( wxGetKeyState(WXK_CONTROL) )
			{
				m_mousePt.s.X = w/2 - 2;
				m_mousePt.s.Y = h/2;
				m_arcBall->drag(&m_mousePt, &ThisQuat);						// Update End Vector And Get Rotation As Quaternion
				Matrix3fSetRotationFromQuat4f(&m_thisRot, &ThisQuat);		// Convert Quaternion Into Matrix3fT
				Matrix3fMulMatrix3f(&m_thisRot, &m_lastRot);				// Accumulate Last Rotation Into This One
				Matrix4fSetRotationFromMatrix3f(&m_dh->m_transform, &m_thisRot);	// Set Our Final Transform's Rotation From This One
			}
			else
				m_dh->mainFrame->m_xSlider->SetValue( wxMax(0, m_dh->mainFrame->m_xSlider->GetValue() - 1));
			break;
		case WXK_RIGHT:
			if ( wxGetKeyState(WXK_CONTROL) )
			{
				m_mousePt.s.X = w/2 + 2;
				m_mousePt.s.Y = h/2;
				m_arcBall->drag(&m_mousePt, &ThisQuat);						// Update End Vector And Get Rotation As Quaternion
				Matrix3fSetRotationFromQuat4f(&m_thisRot, &ThisQuat);		// Convert Quaternion Into Matrix3fT
				Matrix3fMulMatrix3f(&m_thisRot, &m_lastRot);				// Accumulate Last Rotation Into This One
				Matrix4fSetRotationFromMatrix3f(&m_dh->m_transform, &m_thisRot);	// Set Our Final Transform's Rotation From This One
			}
			else
			m_dh->mainFrame->m_xSlider->SetValue( wxMin(m_dh->mainFrame->m_xSlider->GetValue() + 1, m_dh->columns));
			break;
		case WXK_DOWN:
			if ( wxGetKeyState(WXK_CONTROL) )
			{
				m_mousePt.s.X = w/2;
				m_mousePt.s.Y = h/2 - 2;
				m_arcBall->drag(&m_mousePt, &ThisQuat);						// Update End Vector And Get Rotation As Quaternion
				Matrix3fSetRotationFromQuat4f(&m_thisRot, &ThisQuat);		// Convert Quaternion Into Matrix3fT
				Matrix3fMulMatrix3f(&m_thisRot, &m_lastRot);				// Accumulate Last Rotation Into This One
				Matrix4fSetRotationFromMatrix3f(&m_dh->m_transform, &m_thisRot);	// Set Our Final Transform's Rotation From This One
			}
			else
			m_dh->mainFrame->m_ySlider->SetValue( wxMax(0, m_dh->mainFrame->m_ySlider->GetValue() - 1));
			break;
		case WXK_UP:
			if ( wxGetKeyState(WXK_CONTROL) )
			{
				m_mousePt.s.X = w/2;
				m_mousePt.s.Y = h/2 + 2;
				m_arcBall->drag(&m_mousePt, &ThisQuat);						// Update End Vector And Get Rotation As Quaternion
				Matrix3fSetRotationFromQuat4f(&m_thisRot, &ThisQuat);		// Convert Quaternion Into Matrix3fT
				Matrix3fMulMatrix3f(&m_thisRot, &m_lastRot);				// Accumulate Last Rotation Into This One
				Matrix4fSetRotationFromMatrix3f(&m_dh->m_transform, &m_thisRot);	// Set Our Final Transform's Rotation From This One				
			}
			else
			m_dh->mainFrame->m_ySlider->SetValue( wxMin(m_dh->mainFrame->m_ySlider->GetValue() + 1, m_dh->rows));
			break;
		case WXK_PAGEDOWN:
			m_dh->mainFrame->m_zSlider->SetValue( wxMax(0, m_dh->mainFrame->m_zSlider->GetValue() - 1));
			break;
		case WXK_PAGEUP:
			m_dh->mainFrame->m_zSlider->SetValue( wxMin(m_dh->mainFrame->m_zSlider->GetValue() + 1, m_dh->frames));
			break;
		case WXK_HOME:
			m_dh->mainFrame->m_xSlider->SetValue( m_dh->columns / 2);
			m_dh->mainFrame->m_ySlider->SetValue( m_dh->rows / 2);
			m_dh->mainFrame->m_zSlider->SetValue( m_dh->frames / 2);
			break;
		default:
			event.Skip();
			return;
		}
	m_dh->updateView(m_dh->mainFrame->m_xSlider->GetValue(), m_dh->mainFrame->m_ySlider->GetValue(), m_dh->mainFrame->m_zSlider->GetValue());
	m_dh->mainFrame->refreshAllGLWidgets();
}
