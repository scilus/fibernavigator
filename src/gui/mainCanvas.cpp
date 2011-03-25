#include "mainCanvas.h"
#include "myListCtrl.h"
#include "wx/utils.h"
#include "../dataset/splinePoint.h"
#include "../misc/lic/FgeOffscreen.h"
#include "mainFrame.h"
#include "../dataset/Anatomy.h"
#include <list>

typedef std::vector<float> image1D;
typedef std::vector<image1D> image2D;
typedef std::vector<image2D> image3D;

#define MAX(a,b) ((a) > (b) ? (a) : (b)) 
#define MIN(a,b) ((a) < (b) ? (a) : (b)) 


extern const wxEventType wxEVT_NAVGL_EVENT = wxNewEventType();

BEGIN_EVENT_TABLE(MainCanvas, wxGLCanvas)
EVT_SIZE(MainCanvas::OnSize)
EVT_PAINT(MainCanvas::OnPaint)
EVT_MOUSE_EVENTS(MainCanvas::OnMouseEvent)
EVT_ERASE_BACKGROUND(MainCanvas::OnEraseBackground)
EVT_CHAR(MainCanvas::OnChar)
EVT_SHOW(MainCanvas::OnShow)
END_EVENT_TABLE()

MainCanvas::MainCanvas(DatasetHelper* dh, int view, wxWindow *parent, wxWindowID id,
#ifdef CTX
        const wxPoint& pos, const wxSize& size, long style, const wxString& name, int* gl_attrib, wxGLContext*ctx)
: wxGLCanvas(parent, ctx, id,
        wxDefaultPosition, wxDefaultSize, 0, name) // gl_attrib, pos, size, style|wxFULL_REPAINT_ON_RESIZE, name )
#else
const wxPoint& pos,const wxSize & size, long style, const wxString& name, int* gl_attrib, wxGLCanvas*shared )
: wxGLCanvas(parent, shared, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE, name, gl_attrib )
#endif
{
    m_init = false;
    m_view = view;
    m_dh = dh;
    /*
     m_lastRot.M[0] = -0.67698019742965698242f; m_lastRot.M[1] =  0.48420974612236022949f; m_lastRot.M[2] = -0.55429106950759887695;
     m_lastRot.M[3] =  0.73480975627899169922f; m_lastRot.M[4] =  0.40184235572814941406f; m_lastRot.M[5] = -0.54642277956008911133f;
     m_lastRot.M[6] = -0.04184586182236671448f; m_lastRot.M[7] = -0.77721565961837768555f; m_lastRot.M[8] = -0.62784034013748168945f;
     */
    m_lastRot.M[0] = -0.66625452041625976562f; m_lastRot.M[1] = 0.42939949035644531250f; m_lastRot.M[2] = -0.60968911647796630859f;
    m_lastRot.M[3] = -0.74149495363235473633f; m_lastRot.M[4] = -0.46842813491821289062f; m_lastRot.M[5] = 0.48037606477737426758f;
    m_lastRot.M[6] = -0.07932166755199432373f; m_lastRot.M[7] = 0.77213370800018310547f; m_lastRot.M[8] = 0.63048923015594482422f;

    Matrix4fSetIdentity(&m_dh->m_transform);
    Matrix3fSetIdentity(&m_thisRot);
    Matrix3fMulMatrix3f(&m_thisRot, &m_lastRot);
    Matrix4fSetRotationFromMatrix3f(&m_dh->m_transform, &m_lastRot);

    m_delta   = 0;
    m_arcBall = new ArcBallT(640.0f, 480.0f);

    m_orthoSizeNormal = 200;
    m_orthoModX = 0;
    m_orthoModY = 0;
    m_hitPts =Vector(0,0,0);
    m_isRulerHit = false;
}

MainCanvas::~MainCanvas()
{

}

void MainCanvas::init()
{
    m_dh->m_theScene->initGL( m_view );
    m_init = true;
}

void MainCanvas::changeOrthoSize()
{
    m_orthoSizeNormal = (int) ( wxMax( wxMax( m_dh->m_columns * m_dh->m_xVoxel, m_dh->m_rows * m_dh->m_yVoxel), m_dh->m_frames * m_dh->m_zVoxel) );

    if( m_view == MAIN_VIEW )
    {
        m_orthoModX = 0;
        m_orthoModY = 0;
        int xSize = GetSize().x;
        int ySize = GetSize().y;
        float ratio = (float) xSize / (float) ySize;
        if ( ratio > 1.0 )
            m_orthoModX = ( (int) ( m_orthoSizeNormal * ratio ) - m_orthoSizeNormal ) / 2;
        else
            m_orthoModY = ( (int) ( m_orthoSizeNormal * ( 1.0 + ( 1.0 - ratio ) ) ) - m_orthoSizeNormal ) / 2;
    }

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, m_orthoSizeNormal, 0, m_orthoSizeNormal, -500, 500 );
}

void MainCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    render();
}

void MainCanvas::OnSize( wxSizeEvent& event )
{
    // this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize( event );

    int w, h; 
    GetClientSize( &w, &h );
    m_arcBall->setBounds( (GLfloat) w, (GLfloat) h );
    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
    //glViewport( 0, 0, (GLint) w, (GLint) h );    
}

void MainCanvas::OnShow(wxShowEvent& WXUNUSED(event) )
{
#if defined( __WXMAC__ )
    SetCurrent();
#elif defined ( __WXMSW__ )
    SetCurrent();
#else
    if ( !m_dh->m_texAssigned )
    {
        wxGLCanvas::SetCurrent();
    }
    else
    {
        wxGLCanvas::SetCurrent( *m_dh->m_theScene->getMainGLContext() );
    }
#endif
    
    int w, h; 
    GetClientSize( &w, &h );
    glViewport( 0, 0, (GLint) w, (GLint) h );

    m_arcBall->setBounds( (GLfloat) w, (GLfloat) h );    
}

void MainCanvas::OnMouseEvent( wxMouseEvent& event )
{
    wxCommandEvent event1( wxEVT_NAVGL_EVENT, GetId() );
    event1.SetInt( m_view );
    int clickX = event.GetPosition().x;
    int clickY = event.GetPosition().y;
    switch ( m_view )
    {
        case MAIN_VIEW:
        {
            if ( event.LeftUp() )
            {
                if ( wxGetKeyState( WXK_SHIFT ) && !m_dh->getPointMode() )
                {
                    m_hr = pick( event.GetPosition(), false );
                    int newX = (int) ( getEventCenter().x + 0.5 );
                    int newY = (int) ( getEventCenter().y + 0.5 );
                    int newZ = (int) ( getEventCenter().z + 0.5 );
                    m_dh->updateView( newX, newY, newZ );
                    m_dh->m_mainFrame->m_xSlider->SetValue( newX );
                    m_dh->m_mainFrame->m_ySlider->SetValue( newY );
                    m_dh->m_mainFrame->m_zSlider->SetValue( newZ );
                    m_dh->m_mainFrame->refreshAllGLWidgets();
                }
                else if ( wxGetKeyState( WXK_CONTROL ) && m_dh->getPointMode() )
                {
                    m_hr = pick( event.GetPosition(),false );
                    if ( m_hr.hit && ( m_hr.picked <= SAGITTAL ) )
                    {
                        m_hr.picked = 20;
                        SplinePoint *point = new SplinePoint( getEventCenter(), m_dh );
                        wxTreeItemId pId = m_dh->m_mainFrame->m_treeWidget->AppendItem(
                                m_dh->m_mainFrame->m_tPointId, wxT("point"), -1, -1, point );
                        point->setTreeId( pId );

                        GetEventHandler()->ProcessEvent( event1 );
                    }
                }

            }

            if ( event.LeftIsDown() )
            {
                SetFocus();
                m_mousePt.s.X = clickX;
                m_mousePt.s.Y = clickY;
				
				if ( !m_dh->m_isDragging ) // Not Dragging
                {
                    m_dh->m_isDragging = true; // Prepare For Dragging
                    m_lastRot = m_thisRot; // Set Last Static Rotation To Last Dynamic One
                    m_arcBall->click( &m_mousePt ); // Update Start Vector And Prepare For Dragging
                }
                else
                {                    
                    Quat4fT ThisQuat;
                    m_arcBall->drag( &m_mousePt, &ThisQuat ); // Update End Vector And Get Rotation As Quaternion
                    Matrix3fSetRotationFromQuat4f( &m_thisRot, &ThisQuat ); // Convert Quaternion Into Matrix3fT
                    Matrix3fMulMatrix3f( &m_thisRot, &m_lastRot ); // Accumulate Last Rotation Into This One
                    Matrix4fSetRotationFromMatrix3f( &m_dh->m_transform, &m_thisRot ); // Set Our Final Transform's Rotation From This One
                }

                updateView();
                Refresh( false );

            }
            else
                m_dh->m_isDragging = false;

            if ( event.MiddleIsDown() )
            {
                if ( !m_dh->m_ismDragging)
                {
					if (m_dh->m_isSegmentActive && m_dh->m_isFloodfillActive) //FloodFill Method (1click)
					{
						m_hr = pick(event.GetPosition(), true);
						segmentTumor();
					}
					else if (m_dh->m_isRulerToolActive){ //Ruler Tool
                        //TODO HACK to be corrected
                        m_hr = pick(event.GetPosition(), true);
                    }
					else if (!m_dh->m_isRulerToolActive && !m_dh->m_isSelectBckActive && m_dh->m_isSelectObjActive && m_dh->m_isSegmentActive) //Prepare Drag for selectObj-GraphCut
					{
						m_hr = pick(event.GetPosition(), true);

						Vector current;
						current[0] = floor(m_hitPts[0]/m_dh->m_xVoxel);
						current[1] = floor(m_hitPts[1]/m_dh->m_yVoxel);
						current[2] = floor(m_hitPts[2]/m_dh->m_zVoxel);

						object.push_back(current);
						
					}
					else if (!m_dh->m_isRulerToolActive && m_dh->m_isSelectBckActive && !m_dh->m_isSelectObjActive && m_dh->m_isSegmentActive) //Prepare Drag for selectBck-GraphCut
					{
						m_hr = pick(event.GetPosition(), true);

						Vector current;
						current[0] = floor(m_hitPts[0]/m_dh->m_xVoxel);
						current[1] = floor(m_hitPts[1]/m_dh->m_yVoxel);
						current[2] = floor(m_hitPts[2]/m_dh->m_zVoxel);

						background.push_back(current);
						
					}
                    m_dh->m_ismDragging = true;
                    m_lastPos = event.GetPosition();
                }
                else  if (!m_dh->m_isRulerToolActive && !m_dh->m_isSegmentActive) //Move Scene
                {                    
                    int xDrag = m_lastPos.x - clickX;
                    int yDrag = ( m_lastPos.y - clickY );
                    m_lastPos = event.GetPosition();
                    m_dh->moveScene( xDrag, yDrag );
                    Refresh( false );
                }
				else if(!m_dh->m_isRulerToolActive && m_dh->m_isSegmentActive && m_dh->m_isSelectObjActive && !m_dh->m_isSelectBckActive) //Dragging for selectObj-Graphcut
				{
					m_hr = pick(event.GetPosition(), true);

					Vector current;
					current[0] = floor(m_hitPts[0]/m_dh->m_xVoxel);
					current[1] = floor(m_hitPts[1]/m_dh->m_yVoxel);
					current[2] = floor(m_hitPts[2]/m_dh->m_zVoxel);
					
					

					if(current[0] != object.back()[0] || current[1] != object.back()[1] || current[2] != object.back()[2])
						object.push_back(current);
				}
				else if(!m_dh->m_isRulerToolActive && m_dh->m_isSegmentActive && !m_dh->m_isSelectObjActive &&m_dh->m_isSelectBckActive) //Dragging for selectBck-Graphcut
				{
					m_hr = pick(event.GetPosition(), true);

					Vector current;
					current[0] = floor(m_hitPts[0]/m_dh->m_xVoxel);
					current[1] = floor(m_hitPts[1]/m_dh->m_yVoxel);
					current[2] = floor(m_hitPts[2]/m_dh->m_zVoxel);
					
					

					if(current[0] != background.back()[0] || current[1] != background.back()[1] || current[2] != background.back()[2])
						background.push_back(current);
				}
            }
			else
            {
                m_dh->m_ismDragging = false;
            }

            if ( event.GetWheelDelta() != 0)
            {
                m_dh->changeZoom( event.GetWheelRotation() );
                Refresh( false );
            }

            if ( event.RightIsDown() )
            {
                if ( !m_dh->m_isrDragging ) // Not Dragging
                {
                    if ( wxGetKeyState( WXK_CONTROL ) && wxGetKeyState( WXK_SHIFT ) )
                    {
                        printf( "%2.8f : %2.8f : %2.8f \n", m_dh->m_transform.M[0], m_dh->m_transform.M[1],
                                m_dh->m_transform.M[2] );
                        printf( "%2.8f : %2.8f : %2.8f \n", m_dh->m_transform.M[3], m_dh->m_transform.M[4],
                                m_dh->m_transform.M[5] );
                        printf( "%2.8f : %2.8f : %2.8f \n", m_dh->m_transform.M[6], m_dh->m_transform.M[7],
                                m_dh->m_transform.M[8] );
                    }
                    m_dh->m_isrDragging = true; // Prepare For Dragging
                    m_lastPos = event.GetPosition();
                    m_hr = pick( event.GetPosition(), false);

                    SetFocus();

                    if ( m_hr.picked == 20 )
                    {
                        if ( m_dh->m_lastSelectedPoint )
                            m_dh->m_lastSelectedPoint->unselect();
                        m_dh->m_lastSelectedPoint = ( (SplinePoint*) m_hr.object );

                        ( (SplinePoint*) m_hr.object )->select( true );
                    }
                    else if ( m_hr.picked >= 10 && m_hr.picked < 20 )
                    {
                        if ( m_dh->m_lastSelectedPoint )
                            m_dh->m_lastSelectedPoint->unselect();

                        ( (SelectionObject*) m_hr.object )->select( true );
                    }
                }
                else
                {
                    if ( event.Dragging() && m_hr.picked < 10 )
                    {
                        int xDrag = m_lastPos.x - clickX;
                        int yDrag = m_lastPos.y - clickY;

                        m_delta = 0;
                        if ( xDrag != 0 || yDrag != 0 )
                        {
                            Vector n( 0, 0, 0 );
                            switch ( m_hr.picked )
                            {
                                case AXIAL:
                                    n.z = 1.0;
                                    break;
                                case CORONAL:
                                    n.y = 1.0;
                                    break;
                                case SAGITTAL:
                                    n.x = 1.0;
                                    break;
                            }                     
                            
                            float delta = wxMax(wxMin(getAxisParallelMovement(m_lastPos.x, m_lastPos.y, clickX, clickY, n ),10),-10);
                            float mult = wxMin( m_dh->m_xVoxel, wxMin( m_dh->m_yVoxel, m_dh->m_zVoxel ) );
                            if ( mult < 1.0 )
                            {
                                delta /= mult;
                            }

                            int d = delta;

                            delta = delta-d;

                            if ( delta < -0.5 )
                            {
                                m_delta = d - 1;
                            }
                            else if ( delta > 0.5 )
                            {
                                m_delta = d + 1;
                            }
                            else
                            {
                                m_delta = d;
                            }
                        }
                        
                        GetEventHandler()->ProcessEvent( event1 );
                    }
                    else if ( event.Dragging() && m_hr.picked >= 10 && m_hr.picked < 20 )
                    {
                        ( (SelectionObject*) m_hr.object )->processDrag( event.GetPosition(), m_lastPos, m_projection, m_viewport, m_modelview);
                        m_dh->m_selBoxChanged = true;
                    }
                    else if ( event.Dragging() && m_hr.picked == 20 )
                    {
                        ( (SplinePoint*) m_hr.object )->drag( event.GetPosition(), m_projection, m_viewport, m_modelview );
                    }
                }
                m_lastPos = event.GetPosition();
                Refresh( false );
            }
            else
            {
                m_dh->m_isrDragging = false;
            }

        }
            break;

        case AXIAL:
        case CORONAL:
        case SAGITTAL:
            m_clicked = event.GetPosition();
            if ( event.LeftUp() || event.Dragging() )
            {
                GetEventHandler()->ProcessEvent( event1 );
            }
            break;
        default:
            ;
    }
}

void MainCanvas::updateView()
{
    float *dots = new float[8];
    Vector3fT v1 =
    {
    { 0, 0, 1 } };
    Vector3fT v2 =
    {
    { 1, 1, 1 } };
    Vector3fT view;

    Vector3fMultMat4( &view, &v1, &m_dh->m_transform );
    dots[0] = Vector3fDot( &v2, &view );

    v2.s.Z = -1;
    dots[1] = Vector3fDot( &v2, &view );

    v2.s.Y = -1;
    dots[2] = Vector3fDot( &v2, &view );

    v2.s.Z = 1;
    dots[3] = Vector3fDot( &v2, &view );

    v2.s.X = -1;
    dots[4] = Vector3fDot( &v2, &view );

    v2.s.Z = -1;
    dots[5] = Vector3fDot( &v2, &view );

    v2.s.Y = 1;
    dots[6] = Vector3fDot( &v2, &view );

    v2.s.Z = 1;
    dots[7] = Vector3fDot( &v2, &view );

    float max = 0.0;
    int quadrant = 0;
    for ( int i = 0; i < 8; ++i )
    {
        if ( dots[i] > max )
        {
            max = dots[i];
            quadrant = i + 1;
        }
    }
    m_dh->m_quadrant = quadrant;
}

float MainCanvas::getAxisParallelMovement( int x1, int y1, int x2, int y2, Vector n )
{
    Vector vs = m_dh->mapMouse2World( x1, y1, m_projection, m_viewport, m_modelview);
    Vector ve = m_dh->mapMouse2World( x2, y2, m_projection, m_viewport, m_modelview);
    Vector dir( ve.x - vs.x, ve.y - vs.y, ve.z - vs.z );
    float bb = ( ( dir.x * dir.x ) + ( dir.y * dir.y ) + ( dir.z * dir.z ) );
    float nb = ( ( dir.x * n.x ) + ( dir.y * n.y ) + ( dir.z * n.z ) );
    return bb / nb;
}

hitResult MainCanvas::pick( wxPoint click, bool isRuler)
{
    //glPushMatrix();

    //m_dh->doMatrixManipulation();

    
    //GLdouble modelview[16];    
    GLfloat winX, winY;

    //glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    

    winX = (float) click.x;
    winY = (float) m_viewport[3] - (float) click.y;

    gluUnProject( winX, winY, 0, m_modelview, m_projection, m_viewport, &m_pos1X, &m_pos1Y, &m_pos1Z );
    gluUnProject( winX, winY, 1, m_modelview, m_projection, m_viewport, &m_pos2X, &m_pos2Y, &m_pos2Z );

    //glPopMatrix();
    Ray *ray = new Ray( m_pos1X, m_pos1Y, m_pos1Z, m_pos2X, m_pos2Y, m_pos2Z );

    float xx = (m_dh->m_xSlize+0.5) * m_dh->m_xVoxel;
    float yy = (m_dh->m_ySlize+0.5) * m_dh->m_yVoxel;
    float zz = (m_dh->m_zSlize+0.5) * m_dh->m_zVoxel;

    float xPos = m_dh->m_columns / 2 * m_dh->m_xVoxel;
    float yPos = m_dh->m_rows    / 2 * m_dh->m_yVoxel;
    float zPos = m_dh->m_frames  / 2 * m_dh->m_zVoxel;

    float xSize = m_dh->m_columns * m_dh->m_xVoxel;
    float ySize = m_dh->m_rows    * m_dh->m_yVoxel;
    float zSize = m_dh->m_frames  * m_dh->m_zVoxel;

    BoundingBox *bb = new BoundingBox( xPos, yPos, zPos, xSize, ySize, zSize );

    /**
     * check if one of the 3 planes is picked
     */
    float tpicked = 0;
    int picked = 0;
    hitResult hr =
    { false, 0.0f, 0, NULL };
    if ( m_dh->m_showAxial )
    {
        bb->setSizeZ( 0.0001f );
        bb->setCenterZ( zz );
        hr = bb->hitTest( ray );
        if ( hr.hit )
        {
            tpicked = hr.tmin;
            picked = AXIAL;
			if (m_dh->m_isRulerToolActive || m_dh->m_isSegmentActive){
                m_hitPts = bb->hitCoordinate(ray,CORONAL);
                m_isRulerHit = isRuler;
            }
        }
        bb->setSizeZ( zSize );
        bb->setCenterZ( zPos );
    }

    if ( m_dh->m_showCoronal )
    {
        bb->setSizeY( 0.0001f );
        bb->setCenterY( yy );
        hr = bb->hitTest( ray );
        if ( hr.hit )
        {
            if ( picked == 0  || hr.tmin < tpicked)
            {
                picked = CORONAL;
                tpicked = hr.tmin;
                if (m_dh->m_isRulerToolActive || m_dh->m_isSegmentActive){
                    m_hitPts = bb->hitCoordinate(ray,AXIAL);
                    m_isRulerHit = isRuler;
                }
            }            
        }
        bb->setSizeY( ySize );
        bb->setCenterY( yPos );
    }

    if ( m_dh->m_showSagittal )
    {
        bb->setSizeX( 0.0001f );
        bb->setCenterX( xx );
        hr = bb->hitTest( ray );
        if ( hr.hit )
        {
            if ( picked == 0 || hr.tmin < tpicked)
            {
                picked = SAGITTAL;
                tpicked = hr.tmin;
                if (m_dh->m_isRulerToolActive || m_dh->m_isSegmentActive){
                    m_hitPts = bb->hitCoordinate(ray,SAGITTAL);
                    m_isRulerHit = isRuler;
                }                
            }
        }
    }

    if ( picked != 0 )
    {
        hr.tmin = tpicked;
        hr.picked = picked;
    }

    /*
     * check for hits with the selection object sizers
     */
    if ( m_dh->m_showObjects )
    {
        std::vector< std::vector< SelectionObject* > > l_selectionObjects = m_dh->getSelectionObjects();
        for ( unsigned int i = 0; i < l_selectionObjects.size(); ++i )
        {
            for ( unsigned int j = 0; j < l_selectionObjects[i].size(); ++j )
            {
                hitResult hr1 = l_selectionObjects[i][j]->hitTest( ray );
                if ( hr1.hit && !hr.hit )
                    hr = hr1;
                else if ( hr1.hit && hr.hit && ( hr1.tmin < hr.tmin ) )
                    hr = hr1;
            }
        }
    }
    /*
     * check for hits with points for spline surface
     */
    if ( m_dh->m_pointMode )
    {
        wxTreeItemId id, childid;
        wxTreeItemIdValue cookie = 0;
        id = m_dh->m_mainFrame->m_treeWidget->GetFirstChild( m_dh->m_mainFrame->m_tPointId, cookie );
        while ( id.IsOk() )
        {
            SplinePoint *point = (SplinePoint*) ( m_dh->m_mainFrame->m_treeWidget->GetItemData( id ) );
            hitResult hr1 = point->hitTest( ray );
            if ( hr1.hit && !hr.hit )
                hr = hr1;
            else if ( hr1.hit && hr.hit && ( hr1.tmin < hr.tmin ) )
                hr = hr1;

            id = m_dh->m_mainFrame->m_treeWidget->GetNextChild( m_dh->m_mainFrame->m_tPointId, cookie );
        }
    }

    return hr;

}

void MainCanvas::OnEraseBackground( wxEraseEvent& WXUNUSED(event)  )
{
    // Do nothing, to avoid flashing.
}

void MainCanvas::render()
{   
    wxPaintDC dc( this );
    
#ifndef __WXMAC__
    SetCurrent(*m_dh->m_theScene->getMainGLContext());
#else
    SetCurrent();
#endif
    int w, h;
    GetClientSize( &w, &h );
    glViewport( 0, 0, (GLint) w, (GLint) h );

    // Init OpenGL once, but after SetCurrent
    if ( ! m_init )
    {
        init();
    }

    /* clear color and depth buffers */
#ifdef __WXMAC__
    if (m_dh->m_clearToBlack){
       glClearColor( 0.0f, 0.0f, 0.0f, 0.0f);
    } else {
       glClearColor( 1.0f, 1.0f, 1.0f, 0.0f);
    }
#endif
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glColor3f( 1.0, 1.0, 1.0 );

    switch ( m_view )
    {
        case MAIN_VIEW:
        {
            if ( m_dh->m_scheduledScreenshot )
            {
                int size = 0;
                switch ( m_dh->m_geforceLevel )
                {
                    case 6:
                        size = 2048;
                        break;
                    case 8:
                        size = 4096;
                        break;
                    case 99:
                        size = 1024;
                        break;
                    default:
                        size = 1024;
                        break;
                }

                FgeOffscreen fbo( size, size, true );
                ;
                fbo.setClearColor( 1.0, 1.0, 1.0 );
                fbo.activate();

                glMatrixMode( GL_PROJECTION );
                glLoadIdentity();
                glOrtho( 0, m_orthoSizeNormal, 0, m_orthoSizeNormal, -500, 500 );
                glViewport( 0, 0, size, size );

                glPushMatrix();
                m_dh->doMatrixManipulation();
                m_dh->m_theScene->renderScene();
                glPopMatrix();

                fbo.getTexObject( 1 )->saveImageToPPM( ( m_dh->m_screenshotName ).mb_str() );
                fbo.deactivate();
                m_dh->m_scheduledScreenshot = false;
            }
            else
            {
                glMatrixMode( GL_PROJECTION );
                glLoadIdentity();
                glOrtho( -m_orthoModX, m_orthoSizeNormal + m_orthoModX, -m_orthoModY, m_orthoSizeNormal + m_orthoModY, -500, 500 );

                glPushMatrix();
                m_dh->doMatrixManipulation();

                m_dh->m_theScene->renderScene();

                //add the hit Point to ruler point list
				if ( m_dh->m_isRulerToolActive && !m_dh->m_isSegmentActive && !m_dh->m_ismDragging && m_isRulerHit && (m_hr.picked == AXIAL || m_hr.picked == CORONAL || m_hr.picked == SAGITTAL)){
                    if (m_dh->m_rulerPts.size()>0 ){
                        Vector lastPts = m_dh->m_rulerPts.back();
                        if( lastPts != m_hitPts){                            
                            m_dh->m_rulerPts.push_back(m_hitPts);                            
                        }
                    } else {
                        m_dh->m_rulerPts.push_back(m_hitPts);
                    }
                    m_isRulerHit = false;
                }
                //renderTestRay();
                if (m_dh->m_isShowAxes)
                {
                    renderAxes();
                }
                if (m_dh->m_isRulerToolActive){
                    renderRulerDisplay();
                }
				if(m_dh->m_isSegmentActive)
				{
					
				}

                //save context for picking
                glGetDoublev( GL_PROJECTION_MATRIX, m_projection );
                glGetIntegerv( GL_VIEWPORT,m_viewport );
                glGetDoublev( GL_MODELVIEW_MATRIX, m_modelview );


                glPopMatrix();
            }

            break;
        }
        default:
            glMatrixMode( GL_PROJECTION );
            glLoadIdentity();
            glOrtho( 0, m_orthoSizeNormal, 0, m_orthoSizeNormal, -500, 500 );

            if ( m_dh->m_mainFrame->m_listCtrl->GetItemCount() != 0 )
            {
                m_dh->m_anatomyHelper->renderNav( m_view, m_dh->m_shaderHelper->m_textureShader );
                if ( m_dh->GLError() )
                    m_dh->printGLError( wxT( "render nav view" ) );
            }
    }    
    //glFlush();
    SwapBuffers();  
}

void MainCanvas::invalidate()
{
    printf("invalidate\n");
    if ( m_dh->m_texAssigned )
    {
#ifndef __WXMAC__
        SetCurrent( *m_dh->m_theScene->getMainGLContext() );
#else
        SetCurrent();
#endif
        //m_dh->m_theScene->releaseTextures();
        m_dh->m_texAssigned = false;
    }
    m_init = false;
}

void::MainCanvas::renderRulerDisplay()
{
    glColor3f( 0.0f, 0.6f, 0.95f );
    glLineWidth (5);    
    float sphereSize = 0.35f;
    if (m_dh->m_rulerPts.size() > 0){        
        Vector pts;
        Vector lastPts = m_dh->m_rulerPts[0];
        m_dh->m_rulerFullLength = 0;
        for (unsigned int i=0; i < m_dh->m_rulerPts.size();i++){
            if (i== m_dh->m_rulerPts.size()-1){
                glColor3f( 0.0f, 1.0f, 1.0f );
                sphereSize = 0.4f;
            }
            pts = m_dh->m_rulerPts[i];
            
            glBegin (GL_LINES);          
                glVertex3f (lastPts.x, lastPts.y, lastPts.z);
                glVertex3f (pts.x, pts.y, pts.z);
            glEnd ();    

            
            m_dh->m_theScene->drawSphere( pts.x, pts.y, pts.z, sphereSize);
            
            m_dh->m_rulerPartialLength = (lastPts - pts).getLength();
            m_dh->m_rulerFullLength += m_dh->m_rulerPartialLength;
            lastPts = pts;
        }
    }
    glLineWidth (1);
}

void MainCanvas::renderAxes()
{    
    glLineWidth (10);
        glColor3f( 1.0, 0.0, 0.0 );
        glBegin( GL_LINES );
            glVertex3f( 0, 0, 0);
            glVertex3f( 10, 0, 0);        
        glEnd();
        glColor3f( 0.0, 1.0, 0.0 );
        glBegin( GL_LINES );
            glVertex3f( 0, 0, 0);
            glVertex3f( 0, 10, 0);        
        glEnd();
        glColor3f( 0.0, 0.0, 1.0 );
        glBegin( GL_LINES );
            glVertex3f( 0, 0, 0);
            glVertex3f( 0, 0, 10);        
        glEnd();
    glLineWidth (1);
}

void MainCanvas::renderTestRay()
{
    if ( m_hr.tmin == 0 ){
        glColor3f( 1.0, 0.0, 0.0 );
    }
    glBegin( GL_LINES );
    glVertex3f( m_pos1X, m_pos1Y, m_pos1Z );
    glVertex3f( m_pos2X, m_pos2Y, m_pos2Z );
    glEnd();
    Vector dir( m_pos2X - m_pos1X, m_pos2Y - m_pos1Y, m_pos2Z - m_pos1Z );
    m_dh->m_theScene->drawSphere( m_pos1X + m_hr.tmin * dir.x, 
                                  m_pos1Y + m_hr.tmin * dir.y,
                                  m_pos1Z + m_hr.tmin * dir.z,
                                  3.0 * m_dh->m_xVoxel );
}

Vector MainCanvas::getEventCenter()
{
    Vector dir( m_pos2X - m_pos1X, m_pos2Y - m_pos1Y, m_pos2Z - m_pos1Z );
    Vector center( m_pos1X + m_hr.tmin * dir.x, m_pos1Y + m_hr.tmin * dir.y, m_pos1Z + m_hr.tmin * dir.z );
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

    Matrix4fSetIdentity( &m_dh->m_transform );
    Matrix3fSetIdentity( &m_lastRot );
    Matrix4fSetRotationFromMatrix3f( &m_dh->m_transform, &m_thisRot );

    updateView();
    m_dh->m_mainFrame->refreshAllGLWidgets();
}

void MainCanvas::OnChar( wxKeyEvent& event )
{
    int w, h;
    GetClientSize( &w, &h );
    Quat4fT ThisQuat;
        
    if ( wxGetKeyState( WXK_SHIFT ) )
    {
        m_mousePt.s.X = w / 2;
        m_mousePt.s.Y = h / 2;
        m_lastRot = m_thisRot; // Set Last Static Rotation To Last Dynamic One
        m_arcBall->click( &m_mousePt ); // Update Start Vector And Prepare For Dragging
    }

    switch ( event.GetKeyCode() )
    {
        case WXK_LEFT:
            if ( wxGetKeyState( WXK_CONTROL ) )
            {
                m_mousePt.s.X = w / 2 - 2;
                m_mousePt.s.Y = h / 2;
                m_arcBall->drag( &m_mousePt, &ThisQuat ); // Update End Vector And Get Rotation As Quaternion
                Matrix3fSetRotationFromQuat4f( &m_thisRot, &ThisQuat ); // Convert Quaternion Into Matrix3fT
                Matrix3fMulMatrix3f( &m_thisRot, &m_lastRot ); // Accumulate Last Rotation Into This One
                Matrix4fSetRotationFromMatrix3f( &m_dh->m_transform, &m_thisRot ); // Set Our Final Transform's Rotation From This One
            } else if (m_dh->m_isRulerToolActive && m_dh->m_rulerPts.size()>0){
                m_dh->m_rulerPts.back().x -= m_dh->m_xVoxel;
            } else {
                m_dh->m_mainFrame->m_xSlider->SetValue( wxMax(0, m_dh->m_mainFrame->m_xSlider->GetValue() - 1) );
            }
            break;
        case WXK_RIGHT:
            if ( wxGetKeyState( WXK_CONTROL ) )
            {
                m_mousePt.s.X = w / 2 + 2;
                m_mousePt.s.Y = h / 2;
                m_arcBall->drag( &m_mousePt, &ThisQuat ); // Update End Vector And Get Rotation As Quaternion
                Matrix3fSetRotationFromQuat4f( &m_thisRot, &ThisQuat ); // Convert Quaternion Into Matrix3fT
                Matrix3fMulMatrix3f( &m_thisRot, &m_lastRot ); // Accumulate Last Rotation Into This One
                Matrix4fSetRotationFromMatrix3f( &m_dh->m_transform, &m_thisRot ); // Set Our Final Transform's Rotation From This One
            }
            else if (m_dh->m_isRulerToolActive && m_dh->m_rulerPts.size()>0){
                m_dh->m_rulerPts.back().x += m_dh->m_xVoxel;
            } else {
                m_dh->m_mainFrame->m_xSlider->SetValue(
                        wxMin(m_dh->m_mainFrame->m_xSlider->GetValue() + 1, m_dh->m_columns) );
            }
            break;
        case WXK_DOWN:
            if ( wxGetKeyState( WXK_CONTROL ) )
            {
                m_mousePt.s.X = w / 2;
                m_mousePt.s.Y = h / 2 - 2;
                m_arcBall->drag( &m_mousePt, &ThisQuat ); // Update End Vector And Get Rotation As Quaternion
                Matrix3fSetRotationFromQuat4f( &m_thisRot, &ThisQuat ); // Convert Quaternion Into Matrix3fT
                Matrix3fMulMatrix3f( &m_thisRot, &m_lastRot ); // Accumulate Last Rotation Into This One
                Matrix4fSetRotationFromMatrix3f( &m_dh->m_transform, &m_thisRot ); // Set Our Final Transform's Rotation From This One
            }
            else if (m_dh->m_isRulerToolActive && m_dh->m_rulerPts.size()>0){
                m_dh->m_rulerPts.back().y += m_dh->m_yVoxel;
            } else {
                m_dh->m_mainFrame->m_ySlider->SetValue( wxMax(0, m_dh->m_mainFrame->m_ySlider->GetValue() - 1) );
            }
            break;
        case WXK_UP:
            if ( wxGetKeyState( WXK_CONTROL ) )
            {
                m_mousePt.s.X = w / 2;
                m_mousePt.s.Y = h / 2 + 2;
                m_arcBall->drag( &m_mousePt, &ThisQuat ); // Update End Vector And Get Rotation As Quaternion
                Matrix3fSetRotationFromQuat4f( &m_thisRot, &ThisQuat ); // Convert Quaternion Into Matrix3fT
                Matrix3fMulMatrix3f( &m_thisRot, &m_lastRot ); // Accumulate Last Rotation Into This One
                Matrix4fSetRotationFromMatrix3f( &m_dh->m_transform, &m_thisRot ); // Set Our Final Transform's Rotation From This One
            }
            else if (m_dh->m_isRulerToolActive && m_dh->m_rulerPts.size()>0){
                m_dh->m_rulerPts.back().y -= m_dh->m_yVoxel;
            } else {
                m_dh->m_mainFrame->m_ySlider->SetValue(
                        wxMin(m_dh->m_mainFrame->m_ySlider->GetValue() + 1, m_dh->m_rows) );
            }
            break;
        case WXK_PAGEDOWN:
            if (m_dh->m_isRulerToolActive && m_dh->m_rulerPts.size()>0){
                m_dh->m_rulerPts.back().z -= m_dh->m_zVoxel;
            } else {
                m_dh->m_mainFrame->m_zSlider->SetValue( wxMax( 0, m_dh->m_mainFrame->m_zSlider->GetValue() - 1 ) );
            }
            break;
        case WXK_PAGEUP:
            if (m_dh->m_isRulerToolActive && m_dh->m_rulerPts.size()>0){
                m_dh->m_rulerPts.back().z += m_dh->m_zVoxel;
            } else {
                m_dh->m_mainFrame->m_zSlider->SetValue( wxMin( m_dh->m_mainFrame->m_zSlider->GetValue() + 1, m_dh->m_frames ) );
            }
            break;
        case WXK_HOME:
            m_dh->m_mainFrame->m_xSlider->SetValue( m_dh->m_columns / 2 );
            m_dh->m_mainFrame->m_ySlider->SetValue( m_dh->m_rows / 2 );
            m_dh->m_mainFrame->m_zSlider->SetValue( m_dh->m_frames / 2 );
            break;
        case WXK_DELETE:
            if (m_dh->m_isRulerToolActive && m_dh->m_rulerPts.size()>0){
                m_dh->m_rulerPts.pop_back();
            }
            break;
        case WXK_INSERT:
            if (m_dh->m_isRulerToolActive && m_dh->m_rulerPts.size()>0){
                m_dh->m_rulerPts.push_back(m_dh->m_rulerPts.back());
            } else {
                m_dh->m_rulerPts.push_back(Vector(m_dh->m_columns*m_dh->m_xVoxel/2,m_dh->m_rows*m_dh->m_yVoxel/2,m_dh->m_frames*m_dh->m_zVoxel/2));
            }
            break;
        case WXK_END:
            m_dh->m_rulerPts.clear();
            break;
        default:
            event.Skip();
            return;
    }

    m_dh->updateView( m_dh->m_mainFrame->m_xSlider->GetValue(), 
                      m_dh->m_mainFrame->m_ySlider->GetValue(),
                      m_dh->m_mainFrame->m_zSlider->GetValue() );
    m_dh->m_mainFrame->refreshAllGLWidgets();
}

/*
	Returns the element at position [x][y][z] in 3D space
*/
float MainCanvas::getElement(int i,int j,int k, std::vector<float>* vect)
{
	return (*vect)[i+(j*m_dh->m_columns)+(k*m_dh->m_rows*m_dh->m_columns)];
}
/*
	Kmeans Segmentation
*/
void MainCanvas::KMeans(float means[2],float stddev[2],float apriori[2], std::vector<float>* src, std::vector<float>* label)
{
	/* Segment current image with kmeans */

	/* Variables */

	float lastMeans[2];
	float nbPixel[2];
	bool stop;
	int labelClass;
	int length = m_dh->m_columns * m_dh->m_rows * m_dh->m_frames;

	/* Step 0 : Take two random pixels */
	means[0] = 0.f;
	means[1] = 0.f;
	/*
		The two first means must not be equal.
		The higher or equal is to ensure that the first class
		is sea, and the second is land.
	*/
	while(means[0] == means[1])
	{
		int randX = rand()%(length-1);
		srand(time(NULL));
		means[0] = 0.0f;//src->at(length/2);//getElement(object[0][0],object[0][1],object[0][2],src);	// Mean of the first class

		randX = rand()%(length-1);
		srand(time(NULL));
		means[1] = 1.0f; //src->at(randX);//getElement(background[0][0],background[0][1],background[0][2],src); // // Mean of the second class
	}

	if (means[0] > means[1])
	{
		SWAP(means[0], means[1], float);
	}

	lastMeans[0] = 0.f; lastMeans[1] = 0.f;
	stop = false;
	do
	{
		/* Step 1 : For each pixel, find its class */
		for (int x = 0; x < length; ++x)
		{
			
			if (SQR((src->at(x)-means[0])) < SQR((src->at(x)-means[1])))
			{
				label->at(x) = 0.f;
			}
			else
			{
				label->at(x) = 1.f;
			}
			
		}

		/* Step 2 : Reinitialize means */
		means[0] = 0;	means[1] = 0;
		nbPixel[0] = 0; nbPixel[1] = 0;

		/* Step 3 : Compute the new mean values */
		for (int x = 0; x < length; ++x)
		{
			
			labelClass = label->at(x);
			means[labelClass] = means[labelClass] + src->at(x);
			nbPixel[labelClass] += 1;
			
		}

		/* Step 4 : Compute average value */
		means[0] = means[0]/nbPixel[0];
		means[1] = means[1]/nbPixel[1];
		/*
			End condition : We stop if the difference between last mean values and current
			mean values is less than 1%
		*/
		stop = (fabs(means[0]-lastMeans[0]) <= means[0] / 100) && (fabs(means[1]-lastMeans[1]) <= means[1] / 100);
		lastMeans[0] = means[0];
		lastMeans[1] = means[1];

		
	} while (!stop);

	/* Estimate the std dev and the proportion of each class */
	stddev[0] = 0;
	stddev[1] = 0;
	for (int x = 0; x < length; ++x)
	{
		
		labelClass = label->at(x);
		/* Compute standard deviation */
		stddev[labelClass] = stddev[labelClass] + SQR((src->at(x)-means[labelClass]));
	
	}

	stddev[0] = sqrt(stddev[0]/nbPixel[0]);
	stddev[1] = sqrt(stddev[1]/nbPixel[1]);

	apriori[0] = nbPixel[0]/ (length);
	apriori[1] = nbPixel[1]/ (length);

}
/*
	Floodfill method using a threshold range
*/
void MainCanvas::floodFill(std::vector<float>* src, std::vector<float>* result, Vector click, float range)
{
	//Get the user clicked voxel
	double xClick = floor(click[0]/m_dh->m_xVoxel);
	double yClick = floor(click[1]/m_dh->m_yVoxel);
	double zClick = floor(click[2]/m_dh->m_zVoxel);
	
	std::cout << "FloodFill" << endl;

	//Intensity of the current voxel
	float value = getElement(xClick,yClick,zClick,src);
	float upBracket = value+range;
	float downBracket = value-range;

	std::list<Vector> toVisit;
	int north, south, east, west, front, back, x, y, z;
	float NorthV, EastV, SouthV, WestV, FrontV, BackV;
	float resultNorth, resultEast, resultSouth, resultWest, resultFront, resultBack;

	//Add pixel to the top
	toVisit.push_front(Vector(xClick,yClick,zClick));

	//While there's still pixel to visit
	while(!toVisit.empty())
	{
		x = toVisit.front()[0];
		y = toVisit.front()[1];
		z = toVisit.front()[2];
		toVisit.pop_front();

		result->at(x+(y*m_dh->m_columns)+(z*m_dh->m_rows*m_dh->m_columns)) = 1.0f; //Mark as read

		north = MAX(0,y-1);
		south = MIN(m_dh->m_rows-1,y+1);
		east = MIN(m_dh->m_columns-1,x+1);
		west = MAX(0,x-1);
		front = MAX(0,z-1);
		back = MIN(m_dh->m_frames-1,z+1);

		NorthV = getElement(x,north,z,src);
		SouthV = getElement(x,south,z,src);
		EastV = getElement(east,y,z,src);
		WestV = getElement(west,y,z,src);
		FrontV = getElement(x,y,front,src);
		BackV = getElement(x,y,back,src);

		resultNorth = getElement(x,north,z,result);
		resultSouth = getElement(x,south,z,result);
		resultEast = getElement(east,y,z,result); 
		resultWest = getElement(west,y,z,result);
		resultFront = getElement(x,y,front,result);
		resultBack = getElement(x,y,back,result);
		
		if(NorthV >= downBracket && NorthV < upBracket && resultNorth != 1.0f) //North
			toVisit.push_front(Vector(x,north,z));

		if(SouthV >= downBracket && SouthV < upBracket && resultSouth != 1.0f) //South
			toVisit.push_front(Vector(x,south,z));

		if(EastV >= downBracket && EastV < upBracket && resultEast != 1.0f) //East
			toVisit.push_front(Vector(east,y,z));

		if(WestV >= downBracket && WestV < upBracket && resultWest != 1.0f) //West
			toVisit.push_front(Vector(west,y,z));

		if(FrontV >= downBracket && FrontV < upBracket && resultFront != 1.0f) //Front
			toVisit.push_front(Vector(x,y,front));

		if(BackV >= downBracket && BackV < upBracket && resultBack != 1.0f)  //Back
			toVisit.push_front(Vector(x,y,back));
	}

}
/*
	Graph Cut segmentation
*/

void MainCanvas::graphCut(std::vector<float>* src, std::vector<float>* result)
{
	int x,y,z;
	int a,b,c;
	
	while(!object.empty())
	{
		x = object.back()[0];
		y = object.back()[1];
		z = object.back()[2];
		object.pop_back();

		result->at(x+(y*m_dh->m_columns)+(z*m_dh->m_rows*m_dh->m_columns)) = 1.0f;
	}

	while(!background.empty())
	{
		a = background.back()[0];
		b = background.back()[1];
		c = background.back()[2];
		background.pop_back();
	
		result->at(a+(b*m_dh->m_columns)+(c*m_dh->m_rows*m_dh->m_columns)) = 0.5f;
	}

}


/* 
	Segment selected area 
*/
void MainCanvas::segmentTumor()
{
	std::cout << "Segment method:" << endl;
	
	int dataLength = m_dh->m_rows * m_dh->m_columns * m_dh->m_frames;

	// get selected l_anatomy dataset
    long l_item = m_dh->m_mainFrame->m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	Anatomy* l_info = (Anatomy*)m_dh->m_mainFrame->m_listCtrl->GetItemData( l_item );
	
	//1D vector with the normalized brightness ( 0 to 1 )
	std::vector<float>* flatVoxels = l_info->getFloatDataset();
	std::vector<float>* flatTumor = new std::vector<float>;
	flatTumor->resize(dataLength);
	 
	
	//Segmentation methods
	//Case 0 : Floodfill
	//Case 1 : Graph Cut
	//Case 2 : KMeans
	switch(m_dh->m_SegmentMethod)
	{
		case 0 :
			{
				float threshold = l_info->getFloodThreshold();
				floodFill(flatVoxels, flatTumor, m_hitPts, threshold);
				break;
			}

		case 1 :
			graphCut(flatVoxels, flatTumor);
			break;

		case 2 :
			{
				float means[2], stddev[2], apriori[2];
				KMeans(means,stddev,apriori,flatVoxels,flatTumor);
				break;
			}
	}

	
	//Create a new anatomy for the tumor
	std::cout << "Creating anatomy" << std::endl;
	Anatomy* l_newAnatomy = new Anatomy(m_dh, flatTumor, 0);
	l_newAnatomy->setType(OVERLAY);
	l_newAnatomy->setName( l_info->getName().BeforeFirst( '.' ) + _T( " (Segment)" ) );
    m_dh->m_mainFrame->m_listCtrl->InsertItem( 0, wxT( "" ), 0 );
    m_dh->m_mainFrame->m_listCtrl->SetItem( 0, 1, l_newAnatomy->getName() );
    m_dh->m_mainFrame->m_listCtrl->SetItem( 0, 2, wxT( "0.00") );
    m_dh->m_mainFrame->m_listCtrl->SetItem( 0, 3, wxT( ""), 1 );
    m_dh->m_mainFrame->m_listCtrl->SetItemData( 0, (long)l_newAnatomy );
    m_dh->m_mainFrame->m_listCtrl->SetItemState( 0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
	
	
}

