/////////////////////////////////////////////////////////////////////////////
// Name:            selecitonObject.cpp
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   10/26/2009
//
// Description: This is the implementation file for SelectionObject class.
//
// Last modifications:
//      by : GGirard - 17/01/2011
/////////////////////////////////////////////////////////////////////////////

#include "SelectionObject.h"

#include "SceneHelper.h"
#include "SceneManager.h"
#include "../main.h"
#include "../dataset/Anatomy.h"
#include "../dataset/DatasetManager.h"
#include "../dataset/Fibers.h"
#include "../gui/MainFrame.h"
#include "../misc/Algorithms/BSpline.h"
#include "../misc/Algorithms/ConvexGrahamHull.h"
#include "../misc/Algorithms/ConvexHullIncremental.h"
#include "../misc/IsoSurface/CIsoSurface.h"
#include "../misc/IsoSurface/TriangleMesh.h"

#include <wx/textctrl.h>
#include <wx/tglbtn.h>

#include <fstream>
#include <iostream>
#include <list>
using std::list;

#include <limits>
using std::numeric_limits;

#include <vector>
using std::vector;

#define wxDefPosition wxDefaultPosition
#define wxDefSize     wxDefaultSize

SelectionObject::SelectionObject( Vector i_center, Vector i_size )
:   m_pLabelAnatomy   ( NULL ),
    m_pCBSelectDataSet( NULL ),
    m_displayCrossSections ( CS_NOTHING   ),
    m_displayDispersionCone( DC_NOTHING   )
{
    wxColour  l_color( 240, 30, 30 );
    hitResult l_hr = { false, 0.0f, 0, NULL };

    m_center                = i_center;
    m_color                 = l_color;
    m_colorChanged          = false;
    m_fiberColor            = l_color;
    m_gfxDirty              = false;
    m_handleRadius          = 3.0f;
    m_hitResult             = l_hr;
    m_isActive              = true;
    m_objectType            = DEFAULT_TYPE;
    m_isDirty               = true;
    m_isLockedToCrosshair   = false;
    m_isMaster              = false;
    m_isNOT                 = false;
    m_isosurface            = NULL;
    m_isSelected            = false;
    m_isVisible             = true;
    m_maxCrossSectionIndex  = 0;
    m_minCrossSectionIndex  = 0;
    m_name                  = wxT( "object" );
    m_size                  = i_size;
    m_sourceAnatomy         = NULL;
    m_stepSize              = 9;
    m_threshold             = 0.0f;
    m_treeId                = NULL;
    m_boxMoved                = false;
    m_boxResized            = false;
    m_mustUpdateConvexHull  = true;

    //Distance coloring
    m_DistColoring          = false;

    m_inBox.resize( DatasetManager::getInstance()->getFibersCount(), false );
}

SelectionObject::~SelectionObject( )
{
}

void SelectionObject::lockToCrosshair()
{
    if( m_isLockedToCrosshair )
    {
        m_isLockedToCrosshair          = false;
        SceneManager::getInstance()->setBoxLock( false );
    }
    else
    {
        m_isLockedToCrosshair             = true;
        SceneManager::getInstance()->setBoxLock( true );
        SceneManager::getInstance()->setBoxAtCrosshair( this );
        SceneManager::getInstance()->updateView( (int)m_center.x , (int)m_center.y , (int)m_center.z, true );
        MyApp::frame->m_pXSlider->SetValue( (int)m_center.x );
        MyApp::frame->m_pYSlider->SetValue( (int)m_center.y );
        MyApp::frame->m_pZSlider->SetValue( (int)m_center.z );
        MyApp::frame->refreshAllGLWidgets();
    }
}

///////////////////////////////////////////////////////////////////////////
// Moves the object back.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::moveBack()
{
    if( m_center.y > DatasetManager::getInstance()->getRows() * DatasetManager::getInstance()->getVoxelY() ) 
        return;

    m_center.y = (int)m_center.y + 1.0;

    if( wxGetKeyState( WXK_SHIFT ) )
        m_center.y += m_stepSize;
    
    m_boxMoved = true;
    update();
}

///////////////////////////////////////////////////////////////////////////
// Moves the object down.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::moveDown()
{
    if( m_center.z > DatasetManager::getInstance()->getFrames() * DatasetManager::getInstance()->getVoxelZ() )
        return;

    m_center.z = (int)m_center.z + 1.0;

    if( wxGetKeyState( WXK_SHIFT ) )
        m_center.z += m_stepSize;

    m_boxMoved = true;
    update();
}

///////////////////////////////////////////////////////////////////////////
// Moves the object forward.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::moveForward()
{
    if( m_center.y < 0 ) 
        return;

    m_center.y = (int)m_center.y - 1.0;

    if( wxGetKeyState( WXK_SHIFT ) ) 
        m_center.y -= m_stepSize;

    m_boxMoved = true;
    update();
}

///////////////////////////////////////////////////////////////////////////
// Moves the object left.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::moveLeft()
{
    if( m_center.x < 0 ) 
        return;

    m_center.x = (int)m_center.x - 1.0;

    if( wxGetKeyState( WXK_SHIFT ) )
        m_center.x -= m_stepSize;

    m_boxMoved = true;
    update();
}

///////////////////////////////////////////////////////////////////////////
// Moves the object right.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::moveRight()
{
    if( m_center.x > DatasetManager::getInstance()->getColumns() * DatasetManager::getInstance()->getVoxelX() ) 
        return;

    m_center.x = (int)m_center.x + 1.0;

    if( wxGetKeyState( WXK_SHIFT ) )
        m_center.x += m_stepSize;

    m_boxMoved = true;
    update();
}

///////////////////////////////////////////////////////////////////////////
// Moves the object up.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::moveUp()
{
    if( m_center.z < 0 ) 
        return;

    m_center.z = (int)m_center.z - 1.0;

    if( wxGetKeyState( WXK_SHIFT ) )
        m_center.z -= m_stepSize;

    m_boxMoved = true;
    update();
}

void SelectionObject::processDrag( wxPoint i_click, wxPoint i_lastPos, GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] )
{
    if( m_hitResult.picked == 10 ) 
        drag( i_click, i_lastPos, i_projection, i_viewport, i_modelview );
    else 
        resize( i_click, i_lastPos, i_projection, i_viewport, i_modelview );
}

///////////////////////////////////////////////////////////////////////////
// Resize the object making it smaller in y.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::resizeBack()
{
    if( m_size.y < 2 ) 
        return;

    m_size.y -= 1.0;

    if( wxGetKeyState( WXK_SHIFT ) ) 
        m_size.y = wxMax( 1, m_size.y - m_stepSize );

    m_boxResized = true;
    update();
}

///////////////////////////////////////////////////////////////////////////
// Resize the object making it smaller in z.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::resizeDown()
{
    if( m_size.z < 2 ) 
        return;

    m_size.z -= 1.0;

    if( wxGetKeyState( WXK_SHIFT ) ) 
        m_size.z = wxMax( 1, m_size.z - m_stepSize );

    m_boxResized = true;
    update();
}

///////////////////////////////////////////////////////////////////////////
// Resize the object making it bigger in y.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::resizeForward()
{
    if( m_size.y > DatasetManager::getInstance()->getRows() ) 
        return;

    m_size.y += 1.0;

    if( wxGetKeyState ( WXK_SHIFT ) )
        m_size.y += m_stepSize;

    m_boxResized = true;
    update();
}

///////////////////////////////////////////////////////////////////////////
// Resize the object making it smaller in x.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::resizeLeft()
{
    if( m_size.x < 2 ) 
        return;

    m_size.x -= 1.0;

    if( wxGetKeyState( WXK_SHIFT ) )
        m_size.x = wxMax( 1 ,m_size.x - m_stepSize );

    m_boxResized = true;
    update();
}

///////////////////////////////////////////////////////////////////////////
// Resize the object making it bigger in x.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::resizeRight()
{
    if( m_size.x > DatasetManager::getInstance()->getColumns() )
        return;

    m_size.x += 1.0;

    if( wxGetKeyState( WXK_SHIFT ) )
        m_size.x += m_stepSize;

    m_boxResized = true;
    update();
}

///////////////////////////////////////////////////////////////////////////
// Resize the object making it bigger in z.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::resizeUp()
{
    if( m_size.z > DatasetManager::getInstance()->getFrames() )
        return;

    m_size.z += 1.0;

    if( wxGetKeyState( WXK_SHIFT ) )
        m_size.z += m_stepSize;

    m_boxResized = true;
    update();
}

///////////////////////////////////////////////////////////////////////////
// Select or unselect the object.
//
// i_flag       : Indicate if we want to select or unselect the object.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::select( bool i_flag )
{
    if( m_treeId )
    {
        if( i_flag )
        {
            MyApp::frame->m_pTreeWidget->SelectItem( m_treeId );
            MyApp::frame->m_pTreeWidget->EnsureVisible( m_treeId );
            MyApp::frame->m_pTreeWidget->SetFocus();
        }

        m_isSelected = true;
        updateStatusBar();
    }
}

///////////////////////////////////////////////////////////////////////////
// Updates the object.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::update()
{
    if( m_isLockedToCrosshair )
    {
        SceneManager::getInstance()->updateView( (int)m_center.x , (int)m_center.y , (int)m_center.z, true );
        MyApp::frame->m_pXSlider->SetValue( (int)m_center.x );
        MyApp::frame->m_pYSlider->SetValue( (int)m_center.y );
        MyApp::frame->m_pZSlider->SetValue( (int)m_center.z );
    }

    updateStatusBar();
    m_isDirty = true;
    SceneManager::getInstance()->setSelBoxChanged( true );
    MyApp::frame->refreshAllGLWidgets();

    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    // Update the min/max position in x, y and z of the object.
    m_minX = m_center.x - ( m_size.x * 0.5f * voxelX );
    m_maxX = m_center.x + ( m_size.x * 0.5f * voxelX );
    m_minY = m_center.y - ( m_size.y * 0.5f * voxelY );
    m_maxY = m_center.y + ( m_size.y * 0.5f * voxelY );
    m_minZ = m_center.z - ( m_size.z * 0.5f * voxelZ );
    m_maxZ = m_center.z + ( m_size.z * 0.5f * voxelZ );

    objectUpdate();
}

///////////////////////////////////////////////////////////////////////////
// Updates the object. If a daughter class of SelectionObject needs a specific update call, 
// then they should implement this virtual method (like SelectionEllipsoid is doing).
///////////////////////////////////////////////////////////////////////////
void SelectionObject::objectUpdate()
{
    // Nothing for the mother class!
}

///////////////////////////////////////////////////////////////////////////
// Toggles the m_isActive field.
///////////////////////////////////////////////////////////////////////////
bool SelectionObject::toggleIsActive()
{
    SceneManager::getInstance()->setSelBoxChanged( true );
    return m_isActive = !m_isActive;
}

///////////////////////////////////////////////////////////////////////////
// Set the center of the object.
//
// i_x          : The new x center value.
// i_y          : The new y center value.
// i_z          : The new z center value.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::setCenter( float i_x, float i_y, float i_z )
{
    setCenter( Vector( i_x,i_y, i_z ) );
}

///////////////////////////////////////////////////////////////////////////
// Set the center of the object.
//
// i_center     : The new center vector;
///////////////////////////////////////////////////////////////////////////
void SelectionObject::setCenter( Vector i_center )
{
     m_center  = i_center; 
     m_isDirty = true; 
     update();
}

///////////////////////////////////////////////////////////////////////////
// Sets the color of the object.
//
// i_color      : The new color of the object.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::setColor( wxColour i_color )
{
    m_color = i_color;
    update();
}

///////////////////////////////////////////////////////////////////////////
// To avoid to much complication by inserting the SelectionObject class, this 
// will simply return true if this selection object is of box type or ellipsoid type
///////////////////////////////////////////////////////////////////////////
bool SelectionObject::isSelectionObject()
{
    if( m_objectType == BOX_TYPE || m_objectType == ELLIPSOID_TYPE )
        return true;

    return false;
}

///////////////////////////////////////////////////////////////////////////
// Sets the dirtinest of the object.
//
// i_isDirty        : The new dirtinest of the the object.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::setIsDirty( bool i_isDirty ) 
{
    m_isDirty = i_isDirty;
    SceneManager::getInstance()->setSelBoxChanged( true );
}

///////////////////////////////////////////////////////////////////////////
// Sets the fiber color of the object.
//
// i_color          : The new color of the fiber.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::setFiberColor( wxColour i_color )
{
    m_fiberColor = i_color;

    if( ! m_isMaster ) 
        return;

    m_colorChanged = true;
    update();
}

int SelectionObject::getIcon()
{
    if( m_isActive && m_isVisible ) 
        return 1;
    else if( m_isActive && ! m_isVisible )
        return -1;
    else
        return 0;
}

///////////////////////////////////////////////////////////////////////////
// Sets an object to master or not.
//
// i_isMaster       : Indicates if we want to set this object as a master or not.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::setIsMaster( bool i_isMaster )
{
    m_isMaster = i_isMaster;

    if( m_isMaster )
    {
        m_inBranch.assign( DatasetManager::getInstance()->getFibersCount(), false );
    }
}

///////////////////////////////////////////////////////////////////////////
// Sets the threshold of the object.
//
// i_threshold      : The new threshold value.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::setThreshold( float i_threshold )
{
    m_threshold = i_threshold;
    m_isDirty   = true;
    m_gfxDirty  = true;
    SceneManager::getInstance()->setSelBoxChanged( true );
}

void SelectionObject::drag( wxPoint i_click, wxPoint i_lastPos, GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] )
{
    Vector l_vs = mapMouse2World( i_click.x, i_click.y, i_projection, i_viewport, i_modelview );
    Vector l_ve = mapMouse2WorldBack( i_click.x, i_click.y, i_projection, i_viewport, i_modelview );
    Vector l_dir( l_ve.x - l_vs.x, l_ve.y - l_vs.y, l_ve.z - l_vs.z );

    Vector l_vs2 = mapMouse2World( i_lastPos.x, i_lastPos.y, i_projection, i_viewport, i_modelview );
    Vector l_ve2 = mapMouse2WorldBack( i_lastPos.x, i_lastPos.y, i_projection, i_viewport, i_modelview );
    Vector l_dir2( l_ve2.x - l_vs2.x, l_ve2.y - l_vs2.y, l_ve2.z - l_vs2.z );

    Vector l_change( ( l_vs.x + l_dir.x * m_hitResult.tmin ) - ( l_vs2.x + l_dir2.x * m_hitResult.tmin ),
                     ( l_vs.y + l_dir.y * m_hitResult.tmin ) - ( l_vs2.y + l_dir2.y * m_hitResult.tmin ),
                     ( l_vs.z + l_dir.z * m_hitResult.tmin ) - ( l_vs2.z + l_dir2.z * m_hitResult.tmin ) );

    m_center.x += l_change.x;
    m_center.y += l_change.y;
    m_center.z += l_change.z;
    
    m_boxMoved = true;
    update();
}

void SelectionObject::resize( wxPoint i_click, wxPoint i_lastPos, GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] )
{
    Vector  l_n( 0, 0, 0 );
    float l_delta = 0;

    switch( m_hitResult.picked )
    {
        case 11: l_n.x = -1.0; break;
        case 12: l_n.x =  1.0; break;
        case 13: l_n.y = -1.0; break;
        case 14: l_n.y =  1.0; break;
        case 15: l_n.z = -1.0; break;
        case 16: l_n.z =  1.0; break;
        default: break;
    }

    l_delta = wxMax( wxMin(getAxisParallelMovement( i_lastPos.x, i_lastPos.y, i_click.x, i_click.y, l_n, i_projection, i_viewport, i_modelview ), 1 ), -1 );

    if( m_hitResult.picked == 11 || m_hitResult.picked == 12 ) 
    {
        float newX = m_size.x + l_delta;
        m_size.x = wxMin( wxMax( newX, 1 ), DatasetManager::getInstance()->getColumns() );
    }
    if( m_hitResult.picked == 13 || m_hitResult.picked == 14 ) 
    {
        float newY = m_size.y + l_delta;
        m_size.y = wxMin(wxMax(newY, 1), DatasetManager::getInstance()->getRows() );
    }
    if( m_hitResult.picked == 15 || m_hitResult.picked == 16 ) 
    {
        float newZ = m_size.z + l_delta;
        m_size.z = wxMin( wxMax( newZ, 1 ), DatasetManager::getInstance()->getFrames() );
    }
    
    m_boxResized = true;
    update();
}

float SelectionObject::getAxisParallelMovement( int i_x1, int i_y1, int i_x2, int i_y2, Vector i_n, GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] )
{
    Vector l_vs = mapMouse2World( i_x1, i_y1, i_projection, i_viewport, i_modelview);
    Vector l_ve = mapMouse2World( i_x2, i_y2, i_projection, i_viewport, i_modelview );
    Vector l_dir( l_ve.x - l_vs.x, l_ve.y - l_vs.y, l_ve.z - l_vs.z );

    float l_bb = ( ( l_dir.x * l_dir.x ) + ( l_dir.y * l_dir.y ) + ( l_dir.z * l_dir.z ) );
    float l_nb = ( ( l_dir.x * i_n.x )   + ( l_dir.y * i_n.y )   + ( l_dir.z * i_n.z ) );

    return l_bb / l_nb;
}

///////////////////////////////////////////////////////////////////////////
// Draws a thick fiber on the screen.
//
// i_fiberPoints    : The points of the fiber that will be draw.
// i_thickness      : The thickness of the fiber to draw.
// i_nmTubeEdge     : This param will determine the number of edge the fake tube will have
//                    ( example -> if i_nmTubeEdge =3, then the tube will be shaped like a
//                    triangle, if i_nmTubeEdge = 4 then the tube will be shaped like a square.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::drawThickFiber( const vector< Vector > &i_fiberPoints, float i_thickness, int i_nmTubeEdge )
{
    if( i_fiberPoints.size() < 2 || !m_pToggleDisplayMeanFiber->GetValue() )
        return;

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    
    //Get the mean fiber color
    m_meanFiberColorVector.clear();
    switch ( m_meanFiberColorationMode ){
        case NORMAL_COLOR:
            setNormalColorArray( i_fiberPoints );
            break;
        case CUSTOM_COLOR:
            //Use custom color
            glColor4f( (float)m_meanFiberColor.Red() / 255.0f, (float)m_meanFiberColor.Green() / 255.0f, (float)m_meanFiberColor.Blue() / 255.0f, m_meanFiberOpacity );
            break;
        default:
            glColor4f( 0.25f, 0.25f, 0.25f, m_meanFiberOpacity ); // Grayish
    }

    Vector l_normal;

    // This vector will countain all the points forming the circles around the fiber.
    vector< vector< Vector > > l_circlesPoints;
    l_circlesPoints.resize( i_fiberPoints.size() );

    for( unsigned int i = 0; i < i_fiberPoints.size(); ++i )
    {
        // Lets get a unit-length normal for the current point.
        if( i == i_fiberPoints.size() - 1 ) // if we are on the last point of this fiber, we calculate the normal using the point before the current one.
            l_normal = i_fiberPoints[i] - i_fiberPoints[i-1];
        else // else we calculate the normal using the point after the current one.
            l_normal = i_fiberPoints[i+1] - i_fiberPoints[i];

        Helper::getCirclePoints( i_fiberPoints[i], l_normal, i_thickness, i_nmTubeEdge, l_circlesPoints[i] );
    }

    // Now with our circles points calculated, we link them using quads strip.
    for( int i = 0; i < i_nmTubeEdge; ++i )
    {
        glBegin( GL_QUAD_STRIP );
            for( unsigned int j = 0; j < l_circlesPoints.size(); ++j )
            {
                if ( m_meanFiberColorVector.size() != 0 && m_meanFiberColorVector.size() == l_circlesPoints.size() )
                    glColor4f( m_meanFiberColorVector[j][0], m_meanFiberColorVector[j][1], m_meanFiberColorVector[j][2], m_meanFiberOpacity );

                glVertex3f( l_circlesPoints[j][i].x, l_circlesPoints[j][i].y, l_circlesPoints[j][i].z );
                if( i+1 == i_nmTubeEdge )
                    glVertex3f( l_circlesPoints[j][0].x, l_circlesPoints[j][0].y, l_circlesPoints[j][0].z );
                else
                    glVertex3f( l_circlesPoints[j][i+1].x, l_circlesPoints[j][i+1].y, l_circlesPoints[j][i+1].z );
            }
        glEnd();
    }

    glDisable( GL_BLEND );

}

void SelectionObject::computeConvexHull()
{
    m_mustUpdateConvexHull = true;
//     if ( m_pToggleDisplayConvexHull->GetValue() && m_convexHullOpacity != 0)
//     {
//             vector< Vector > pts;
//             vector< vector< Vector > > l_selectedFibersPoints = getSelectedFibersPoints();
//             for (unsigned int i(0); i< l_selectedFibersPoints.size(); i++)
//                 pts.insert(pts.end(), l_selectedFibersPoints[i].begin(), l_selectedFibersPoints[i].end());
// 
//             m_hullTriangles.clear();
//             ConvexHullIncremental hull(pts);
//             hull.buildHull();
//             hull.getHullTriangles( m_hullTriangles );
//             m_mustUpdateConvexHull = false;
//     }
}

void SelectionObject::drawConvexHull()
{
//     if ( m_pToggleDisplayConvexHull->GetValue() && m_pToggleDisplayConvexHull->IsEnabled() && m_convexHullOpacity != 0)
//     {
//         if (m_mustUpdateConvexHull)
//             computeConvexHull();
// 
//         glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
//         glEnable( GL_BLEND );
//         glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
//         glColor4f( (float)m_convexHullColor.Red() / 255.0f, (float)m_convexHullColor.Green() / 255.0f, (float)m_convexHullColor.Blue() / 255.0f, m_convexHullOpacity );
// 
//         Vector normal;
//         glBegin( GL_TRIANGLES );
//         list< Face3D >::iterator it;
//             for( it = m_hullTriangles.begin(); it != m_hullTriangles.end(); it++ )
//             {
//                 //Vector normal;
//                 //normal = (p[i+1] - p[i]).Cross(p[i+2]-p[i]);
//                 //normal.normalize();
//                 //glNormal3f(normal[0], normal[1], normal[2]);
//                 glVertex3f( it->getPt1().x, it->getPt1().y, it->getPt1().z );
// 
//                 //normal = (p[i] - p[i+1]).Cross(p[i+2]-p[i+1]);
//                 //normal.normalize();
//                 //glNormal3f(normal[0], normal[1], normal[2]);
//                 glVertex3f( it->getPt2().x, it->getPt2().y, it->getPt2().z );
// 
//                 //normal = (p[i] - p[i+2]).Cross(p[i+1]-p[i+2]);
//                 //normal.normalize();
//                 //glNormal3f(normal[0], normal[1], normal[2]);
//                 glVertex3f( it->getPt3().x, it->getPt3().y, it->getPt3().z );
//             }
//         glEnd();
// 
//         glDisable( GL_BLEND );
//     }
}

void SelectionObject::updateConvexHullOpacity()
{
//     setConvexHullOpacity( ( m_pSliderConvexHullOpacity->GetValue() + (float)m_pSliderConvexHullOpacity->GetMin() ) / (float)m_pSliderConvexHullOpacity->GetMax() );
}

///////////////////////////////////////////////////////////////////////////
// Fill the color array use to display the mean fiber base on the mean fiber direction
//      i_fiberPoints : All points of the mean fiber
///////////////////////////////////////////////////////////////////////////
void SelectionObject::setNormalColorArray(const vector< Vector > &i_fiberPoints)
{
    m_meanFiberColorVector.clear();
    Vector color;

    if ( i_fiberPoints.size() < 2 )
        return;
    for ( unsigned int i(0); i < i_fiberPoints.size(); i++ )
    {
        if ( i==0 )
            color = i_fiberPoints[i+1] - i_fiberPoints[i];
        else
            color = i_fiberPoints[i] - i_fiberPoints[i-1];
        
        color.normalize();
        m_meanFiberColorVector.push_back( color );
            
    }
}


///////////////////////////////////////////////////////////////////////////
// Draws all the cross sections.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::drawCrossSections()
{
    if( m_crossSectionsPoints.size() == 0 || m_displayCrossSections == CS_NOTHING )
        return;

    // We draw the cross sections first with only a simple line around them 
    // and then fill the inside with an alpha layer.

    // Cross section line.
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDisable( GL_BLEND );
    drawCrossSectionsPolygons();

    // Cross section fill.
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    drawCrossSectionsPolygons();
    glDisable( GL_BLEND );
}

///////////////////////////////////////////////////////////////////////////
// Draws all the cross sections.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::drawCrossSectionsPolygons()
{
    for( unsigned int i = 0; i < m_crossSectionsPoints.size(); ++i )
    {
        if( m_displayCrossSections == CS_MIN_MAX_ONLY )
            if( i != m_minCrossSectionIndex && i != m_maxCrossSectionIndex )
                continue;

        getCrossSectionAreaColor( i );
        drawPolygon( m_crossSectionsPoints[i] );
    }
}

///////////////////////////////////////////////////////////////////////////
// Draws a polygon represented by a set of points. 
//
// i_polygonPoints      : The vector containing the polygon points.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::drawPolygon( const vector< Vector > &i_polygonPoints )
{
    if( i_polygonPoints.size() < 3)
        return;

    glBegin( GL_POLYGON );

    for( unsigned int i = 0; i < i_polygonPoints.size(); ++i )
        glVertex3f( i_polygonPoints[i].x, i_polygonPoints[i].y, i_polygonPoints[i].z );

    glEnd();
}

///////////////////////////////////////////////////////////////////////////
// This will push back the coords values in the vectors for all the fibers
// flagged as m_inBranch and then we will calculate the grid params with this vector.
//
// o_gridInfo           : The structure containing the information that we need to calculate.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::calculateGridParams( FibersInfoGridParams &o_gridInfo )
{
    vector< vector< Vector > > l_selectedFibersPoints = getSelectedFibersPoints();
   
    // Once the vector is filled up with the points data we can calculate the fibers info grid items.
    o_gridInfo.m_count = l_selectedFibersPoints.size();
    getMeanFiberValue               ( l_selectedFibersPoints, 
                                      o_gridInfo.m_meanValue         );
    getMeanMaxMinFiberLength        ( l_selectedFibersPoints, 
                                      o_gridInfo.m_meanLength, 
                                      o_gridInfo.m_maxLength, 
                                      o_gridInfo.m_minLength         );
    //getMeanMaxMinFiberCrossSection  ( l_selectedFibersPoints,
    //                                  m_meanFiberPoints,
    //                                  o_gridInfo.m_meanCrossSection, 
    //                                  o_gridInfo.m_maxCrossSection,
    //                                  o_gridInfo.m_minCrossSection   );
    getFibersMeanCurvatureAndTorsion( l_selectedFibersPoints, 
                                      o_gridInfo.m_meanCurvature, 
                                      o_gridInfo.m_meanTorsion       );
    //getFiberDispersion              ( o_gridInfo.m_dispersion        );


}


///////////////////////////////////////////////////////////////////////////
// Computes the mean fiber
// Point that make the mean fiber will be in the vector m_meanFiberPoints
//
///////////////////////////////////////////////////////////////////////////
void SelectionObject::computeMeanFiber()
{
    if ( getShowFibers() && m_pToggleDisplayMeanFiber->GetValue() )
    {
        // We calculate the mean fiber of the selected fibers.
        m_meanFiberPoints.clear();
        getMeanFiber( getSelectedFibersPoints(), MEAN_FIBER_NB_POINTS, m_meanFiberPoints );
    }
    else
        m_meanFiberPoints.clear();
}


///////////////////////////////////////////////////////////////////////////
//Return all the visible fibers that pass through the selection object
//
///////////////////////////////////////////////////////////////////////////
vector< vector< Vector > > SelectionObject::getSelectedFibersPoints(){

    vector< Vector >           l_currentFiberPoints;
    vector< Vector >           l_currentSwappedFiberPoints;
    vector< vector< Vector > > l_selectedFibersPoints;
    Vector l_meanStart( 0.0f, 0.0f, 0.0f );
    vector< bool > filteredFiber;

    long index = MyApp::frame->getCurrentListIndex();
    if( -1 != index )
    {
        Fibers * pFibers = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->m_pListCtrl->GetItem( index ) );
        
        if( pFibers == NULL )
        {
            return l_selectedFibersPoints;
        }

        filteredFiber = pFibers->getFilteredFibers();

        for( unsigned int i = 0; i < m_inBranch.size(); ++i )
        {
            if( m_inBranch[i] && !filteredFiber[i] )
            {
                getFiberCoordValues( i, l_currentFiberPoints );

                // Because the direction of the fibers is not all the same, for example 2 fibers side to side on the screen
                // could have there coordinated in the data completely inversed ( first fiber 0,0,0 - 1,1,1 ),  
                // second fiber 1,1,1 - 0,0,0) we need to make sure that all the fibers are in the same order so we do a
                // verification and if the order of the fiber points are wrong we switch them.
                if( l_selectedFibersPoints.size() > 0 )
                {
                    l_meanStart.zero();
                    for( unsigned int j = 0; j < l_selectedFibersPoints.size(); ++j )
                        l_meanStart += l_selectedFibersPoints[j][0];
                    l_meanStart /= l_selectedFibersPoints.size();
                }

                // If the starting point of the current fiber is closer to the mean starting point of the rest of the fibers
                // then the order of the points of this fiber are ok, otherwise we need to flip them.
                if( ( l_meanStart - l_currentFiberPoints[0] ).getLength() <
                    ( l_meanStart - l_currentFiberPoints[l_currentFiberPoints.size() - 1] ).getLength() )
                {
                    l_selectedFibersPoints.push_back( l_currentFiberPoints );
                }
                else
                {
                    for( int k = (int)l_currentFiberPoints.size() - 1; k >= 0; --k )
                        l_currentSwappedFiberPoints.push_back( l_currentFiberPoints[k] );

                    l_selectedFibersPoints.push_back( l_currentSwappedFiberPoints );
                    l_currentSwappedFiberPoints.clear();
                }

                l_currentFiberPoints.clear();
            }
        }
    }

    return l_selectedFibersPoints;
}

///////////////////////////////////////////////////////////////////////////
// Computes the length of a given fiber.
//
// i_fiberPoints            : The fiber to compute the length.
// o_length                 : The output length.
//
// Returns true if successful, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool SelectionObject::getFiberLength( const vector< Vector > &i_fiberPoints, float &o_length )
{
    if( i_fiberPoints.size() == 0 )
        return false;

    float l_dx,l_dy, l_dz;
    o_length = 0.0;
    
    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    for( unsigned int i = 1; i < i_fiberPoints.size(); ++i )
    {
        // The values are in pixel, we need to set them in millimeters using the spacing 
        // specified in the anatomy file ( m_datasetHelper->xVoxel... ).
        l_dx = ( i_fiberPoints[i].x - i_fiberPoints[i-1].x ) * voxelX;
        l_dy = ( i_fiberPoints[i].y - i_fiberPoints[i-1].y ) * voxelY;
        l_dz = ( i_fiberPoints[i].z - i_fiberPoints[i-1].z ) * voxelZ;

        FArray currentVector( l_dx, l_dy, l_dz );
        o_length += (float)currentVector.norm();
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////
// Will calculate the mean fiber of a set of fibers.
//
// i_fibersPoints          : Set of fibers points we will use to calculate the mean fiber.
// i_nbPoints              : Number of points the mean fiber will have.
// o_meanFiberPoints       : The resulting points are store in o_meanFiberPoints.
//
// Returns true if successful, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool SelectionObject::getMeanFiber( const vector< vector< Vector > > &i_fibersPoints, 
                                          unsigned int                i_nbPoints,
                                          vector< Vector >           &o_meanFiberPoints )
{
    o_meanFiberPoints.resize( i_nbPoints );

    for( unsigned int i = 0; i < i_fibersPoints.size(); ++i )
    {
        unsigned int l_currentFiberNbPoints = i_fibersPoints[i].size();
        
        // The -1 is because we dont take into consideration the number of points but 
        // the number of vector between the points.
        
        float        l_currentFiberRatio    = (float)( i_fibersPoints[i].size() - 1 ) / ( i_nbPoints - 1 );
        
        // These two variable help to know if the fibers are in the same direction
        float comp_first_pt = abs(i_fibersPoints[i][0].x-i_fibersPoints[0][0].x) + 
                              abs(i_fibersPoints[i][0].y-i_fibersPoints[0][0].y) +
                              abs(i_fibersPoints[i][0].z-i_fibersPoints[0][0].z);

        float comp_last_pt = abs(i_fibersPoints[i][i_fibersPoints[i].size()-1].x-i_fibersPoints[0][0].x) +
                             abs(i_fibersPoints[i][i_fibersPoints[i].size()-1].y-i_fibersPoints[0][0].y) +
                             abs(i_fibersPoints[i][i_fibersPoints[i].size()-1].z-i_fibersPoints[0][0].z);

        if(comp_first_pt < comp_last_pt)    
        {

            o_meanFiberPoints[0]              += i_fibersPoints[i][0];
            o_meanFiberPoints[i_nbPoints - 1] += i_fibersPoints[i][l_currentFiberNbPoints - 1] ;

            for( unsigned int j = 1; j < i_nbPoints - 1; ++j )
            {
                float l_currentPointInterpolationRatio = l_currentFiberRatio * j;
             
                 // Calculating the ratio of the interpolation.
                int l_pointBelow = (int)l_currentPointInterpolationRatio;
                l_currentPointInterpolationRatio -= l_pointBelow;
                // Simple interpolation.
                o_meanFiberPoints[j] += ( ( i_fibersPoints[i][l_pointBelow]     * ( 1.0 - l_currentPointInterpolationRatio ) ) + 
                                        ( i_fibersPoints[i][l_pointBelow + 1] *         l_currentPointInterpolationRatio ) );
            }
        }
        else
        {

            o_meanFiberPoints[0]              += i_fibersPoints[i][l_currentFiberNbPoints - 1];
            o_meanFiberPoints[i_nbPoints - 1] += i_fibersPoints[i][0];
            
            for( unsigned int j = i_nbPoints - 2 ; j > 0 ; --j )    
            {
                float l_currentPointInterpolationRatio = l_currentFiberRatio * j;
             
                 // Calculating the ratio of the interpolation.
                int l_pointBelow = (int)l_currentPointInterpolationRatio;
                l_currentPointInterpolationRatio -= l_pointBelow;
                // Simple interpolation.
                o_meanFiberPoints[i_nbPoints-j-1] += ( ( i_fibersPoints[i][l_pointBelow]     * ( 1.0 - l_currentPointInterpolationRatio ) ) + 
                                        ( i_fibersPoints[i][l_pointBelow + 1] *           l_currentPointInterpolationRatio ) );

            }            
        }
    }

    for( unsigned int i = 0; i < o_meanFiberPoints.size(); ++i )
        o_meanFiberPoints[i] /= i_fibersPoints.size();

    return true;
}

///////////////////////////////////////////////////////////////////////////
// Check if fibers are show
//return true if they are, false otherwise
///////////////////////////////////////////////////////////////////////////
bool SelectionObject::getShowFibers()
{
    Fibers* pFibers = NULL;
    long index = MyApp::frame->getCurrentListIndex();
    if( -1 != index )
    {
        pFibers = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->m_pListCtrl->GetItem( index ) );
    }

    return NULL == pFibers ? false : pFibers->getShow();
}

///////////////////////////////////////////////////////////////////////////
// Fills the o_fiberPoints vector with the points that compose a given fiber.
//
// i_fiberIndex             : The index of the given fiber.
// o_fiberPoints            : The vector that will be filled with the points of the given fiber.
//
// Returns true if successful, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool SelectionObject::getFiberCoordValues( int i_fiberIndex, vector< Vector > &o_fiberPoints )
{
    long index = MyApp::frame->getCurrentListIndex();
    if( -1 != index )
    {
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->m_pListCtrl->GetItem( index ) );

        if( pFibers == NULL || i_fiberIndex < 0 || i_fiberIndex >= (int)m_inBranch.size() )
            return false;

        int l_index = pFibers->getStartIndexForLine( i_fiberIndex ) * 3;
        Vector l_point3D;

        for( int i = 0; i < pFibers->getPointsPerLine( i_fiberIndex ); ++i )
        {
            l_point3D.x = pFibers->getPointValue( l_index );
            l_point3D.y = pFibers->getPointValue( l_index + 1);
            l_point3D.z = pFibers->getPointValue( l_index + 2 );
            o_fiberPoints.push_back( l_point3D );

            l_index += 3;
        }

        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
// Will set o_count with the number of fibers that are flagged as being inBranch.
//
// o_count          : The output count.
//
// Returns true if successful, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool SelectionObject::getFibersCount( int &o_count )
{
    o_count = 0;

    for( unsigned int i = 0; i < m_inBranch.size() ; ++i )
        if( m_inBranch[i] ) 
            ++o_count;

    return true;
}

///////////////////////////////////////////////////////////////////////////
// Computes the mean value of a given set of fibers. This value is calculated 
// from the anatomy that is already loaded.
//
// i_fibersPoints           : The given set of fibers.
// o_meanValue              : The output mean value.
//
// Returns true if successful, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool SelectionObject::getMeanFiberValue( const vector< vector< Vector > > &fibersPoints, float &computedMeanValue )
{
    computedMeanValue = 0.0f;

    if( fibersPoints.size() == 0 )
    {
        return false;
    }
    
    Anatomy *pCurrentAnatomy( NULL );

    vector< Anatomy* > datasets = DatasetManager::getInstance()->getAnatomies();
    
    if( m_pCBSelectDataSet == NULL && datasets.size() > 0 )
    {
        // Select the first dataset in the list
        pCurrentAnatomy = datasets[0];
    }
    else if( m_pCBSelectDataSet != NULL && 
             m_pCBSelectDataSet->GetCount() > 0 && 
             m_pCBSelectDataSet->GetCurrentSelection() != -1 )
    {
        // TODO: Verify if the selection is the right index
        // Get the currently selected dataset.
        pCurrentAnatomy = datasets[ m_pCBSelectDataSet->GetSelection() ];
    }
    else
    {
        return false;
    }

    unsigned int pointsCount( 0 );
    unsigned int datasetPos ( 0 );

    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    for( unsigned int i = 0; i < fibersPoints.size(); ++i )
    {
        for( unsigned int j = 0; j < fibersPoints[i].size(); ++j )
        {
            datasetPos = ( static_cast<int>( fibersPoints[i][j].x / voxelX ) +
                           static_cast<int>( fibersPoints[i][j].y / voxelY ) * pCurrentAnatomy->getColumns() +
                           static_cast<int>( fibersPoints[i][j].z / voxelZ ) * pCurrentAnatomy->getColumns() * pCurrentAnatomy->getRows() ) * pCurrentAnatomy->getBands();
            
            for (int i(0); i < pCurrentAnatomy->getBands(); i++)
            {
                computedMeanValue += (* ( pCurrentAnatomy->getFloatDataset() ) )[datasetPos + i];
            }

            ++pointsCount;
        }
    }
    
    computedMeanValue /= pointsCount;
    return true;
}

///////////////////////////////////////////////////////////////////////////
// Computes the mean, max and min length for a given set of fibers.
//
// i_selectedFibersPoints       : The given set of fibers.
// o_meanLength                 : The output mean length.
// o_maxLength                  : The output max length.
// o_minLength                  : The output min length.
//
// Returns true if successful, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool SelectionObject::getMeanMaxMinFiberLength( const vector< vector< Vector > > &i_fibersPoints, 
                                                      float                      &o_meanLength,
                                                      float                      &o_maxLength,
                                                      float                      &o_minLength )
{
    if( i_fibersPoints.size() <= 0 )
    {
        o_meanLength = 0.0f;
        o_maxLength  = 0.0f;
        o_minLength  = 0.0f;
        return false;
    }

    float currentFiberLength;

    o_meanLength = 0.0f;
    o_maxLength  = 0.0f;
    o_minLength  = numeric_limits<float>::max();
    
    for( unsigned int i = 0; i < i_fibersPoints.size(); i++ )
    {
        currentFiberLength = 0.0f;

        getFiberLength( i_fibersPoints[i], currentFiberLength );

        o_meanLength += currentFiberLength;

        if( currentFiberLength > o_maxLength )
            o_maxLength = currentFiberLength;

        if( currentFiberLength < o_minLength )
            o_minLength = currentFiberLength;
    }
    
    o_meanLength /= i_fibersPoints.size();    

    return true;
}

///////////////////////////////////////////////////////////////////////////
// Computes the mean, max and the min cross section for a given set of fibers.
//
// i_fibersPoints           : The given set of fibers we are going to calculate the cross section for.
// i_meanFiberPoints        : The mean fiber we are going to use to generate the plane for the cross section calculation.
// o_meanCrossSection       : The output mean cross section.
// o_maxCrossSection        : The output max cross section.
// o_minCrossSection        : The output min cross section.
//
// Returns true if successful, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool SelectionObject::getMeanMaxMinFiberCrossSection( const vector< vector< Vector > > &i_fibersPoints,
                                                      const vector< Vector >           &i_meanFiberPoints, 
                                                            float                      &o_meanCrossSection,
                                                            float                      &o_maxCrossSection,
                                                            float                      &o_minCrossSection )
{
    m_crossSectionsPoints.clear();
    m_crossSectionsAreas.clear();
    m_crossSectionsNormals.clear();
    m_crossSectionsPoints.resize ( i_meanFiberPoints.size() );
    m_crossSectionsAreas.resize  ( i_meanFiberPoints.size() );
    m_crossSectionsNormals.resize( i_meanFiberPoints.size() );

    o_meanCrossSection = 0.0f;
    o_maxCrossSection  = 0.0f;

    // We need at least 3 fibers to get 3 points to be able to calculate a convex hull!
    if( i_fibersPoints.size() < 3 )
    {
        o_minCrossSection  = 0.0f;
        return false;
    }

    o_minCrossSection  = numeric_limits<float>::max();

    vector< Vector > l_maxHullPts, l_maxOriginalPts, l_minHullPts, l_minOriginalPts;

    // For each points on the mean fiber we calculate a plane perpendicular with it.
    for( unsigned int i = 0; i < i_meanFiberPoints.size(); ++i )
    {        
        vector < Vector > l_intersectionPoints;
        Vector l_pointOnPlane, l_planeNormal;
        l_pointOnPlane = i_meanFiberPoints[i];

        // When we are at the last point of the mean fiber, since i_meanFiberPoints[i + 1] 
        // will not exist, we calculate the vector with the previous point.
        if( i == i_meanFiberPoints.size() - 1 )
            l_planeNormal = i_meanFiberPoints[i] - i_meanFiberPoints[i - 1];
        else
            l_planeNormal = i_meanFiberPoints[i + 1] - i_meanFiberPoints[i];

        l_planeNormal.normalize();

        // We get the points intersecting the current plane for all the selectedFibers.
        for( unsigned int j = 0; j < i_fibersPoints.size(); ++j )
            getFiberPlaneIntersectionPoint( i_fibersPoints[j], l_pointOnPlane, l_planeNormal, l_intersectionPoints );

        // We need at least 3 points to get a valid cross section.

        if( l_intersectionPoints.size() < 3 )
            continue;

        vector < Vector > original3Dpts = l_intersectionPoints;

        // Once we have those 3d points, we need to map them to 2d so we can calculate the convex hull.
        if( ! ( Helper::convert3DPlanePointsTo2D( l_planeNormal, l_intersectionPoints ) ) )
            continue;

        ConvexGrahamHull hull( l_intersectionPoints );
       
        // Let's build the convex hull.
        vector < Vector > l_hullPoints;
        if( ! ( hull.buildHull() ) )
            continue;
        hull.getHullPoints( l_hullPoints );
        // Computing the surface area of the hull.
        double l_hullArea = 0.0f;
        if( ! ( hull.area( l_hullArea ) ) )
            continue;

        o_meanCrossSection += l_hullArea;

        // To be able to see the cross section on the screen we save the points in m_crossSectionsPoints
        // and the area value in m_crossSectionsAreas, we also save the normals of the planes inside
        // m_crossSectionsNormals to be able to draw the dispersion cone.
        vector< Vector > tmp;
        Helper::convert2DPlanePointsTo3D( original3Dpts, l_hullPoints, tmp );
        m_crossSectionsPoints[i]  = tmp;
        m_crossSectionsAreas[i]   = l_hullArea;
        m_crossSectionsNormals[i] = l_planeNormal;

        // Maximum Cross Section.
        if( l_hullArea > o_maxCrossSection )
        {
            o_maxCrossSection = (float)l_hullArea;
            m_maxCrossSectionIndex = i;
        }

        // Minimum Cross Section.
        if( l_hullArea < o_minCrossSection )
        {
            o_minCrossSection = (float)l_hullArea;
            m_minCrossSectionIndex = i;
        }
    }
     
    o_meanCrossSection /= i_meanFiberPoints.size();

    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    // We want to return the values in millimeters so we need to multiply them by the spacing in the anatomy file.
    float l_spacing = voxelX * voxelY * voxelZ;

    o_maxCrossSection  *= l_spacing;
    o_minCrossSection  *= l_spacing;
    o_meanCrossSection *= l_spacing;

    return true;
}

///////////////////////////////////////////////////////////////////////////
// Will return the intersection point between a fiber and a plane.
//
// i_fiberPoints            : Containt the points information of the fiber.
// i_pointOnPlane           : A point on the plane.
// i_planeNormal            : The normal of the plane.
// o_intersectionPoints     : Output vector that will containt the intersection point.
//
// Returns true if successful, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool SelectionObject::getFiberPlaneIntersectionPoint( const vector< Vector > &i_fiberPoints,
                                                      const Vector           &i_pointOnPlane,
                                                      const Vector           &i_planeNormal,
                                                            vector< Vector > &o_intersectionPoints )
{
    float l_minDistance = numeric_limits<float>::max();
    Vector l_tmp;
    bool l_intersected = false;
    
    for( unsigned int i = 0; i < i_fiberPoints.size() - 1; ++i )
    {
        Vector l_intersectionPoint;
        // Check if there is an intersection point between the plane and the line.
        if( Helper::getIntersectionPoint( i_fiberPoints[i], i_fiberPoints[i + 1], i_pointOnPlane, i_planeNormal, l_intersectionPoint ) )
        {
            // If there was more that 1 point of the current fiber that was in contact with the plane,
            // we only keep the one closest to the mean fiber and since i_pointOnPlane is actually the
            // mean fiber point used to generate the plane, we use it.
            float  l_distance = ( l_intersectionPoint - i_pointOnPlane ).getLength();

            if( l_distance < l_minDistance )
            {
                l_minDistance = l_distance;
                l_tmp         = l_intersectionPoint;
                l_intersected = true;
            }
        }
    }

    // Only push back a point if there was at least 1 intersection.
    if( l_intersected )
        o_intersectionPoints.push_back( l_tmp );

    return true;
}

///////////////////////////////////////////////////////////////////////////
// Computes the mean curvature and mean torsion for a given set of fibers.
//
// i_fibersVector           : The vector containing the fibers points.
// o_meanCurvature          : The output mean curvature of the fibers.
// o_meanTorsion            : The output mean torsion of the fibers.
//
// Returns true if successful, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool SelectionObject::getFibersMeanCurvatureAndTorsion( const vector< vector< Vector > > &i_fiberVector, 
                                                              float                      &o_meanCurvature, 
                                                              float                      &o_meanTorsion )
{

   o_meanCurvature = 0.0f;
   o_meanTorsion   = 0.0f;

   if( i_fiberVector.size() == 0 )
    return false;

	//Curvature and torsion are now calculated from mean fiber
   vector< Vector > meanFiberPoint;
	getMeanFiber(i_fiberVector, MEAN_FIBER_NB_POINTS, meanFiberPoint);
	getFiberMeanCurvatureAndTorsion(meanFiberPoint, o_meanCurvature, o_meanTorsion );


	//Curvature and torsion are now calculated from mean fiber
   //float l_currentFiberCurvature, l_currentFiberTorsion;
   //for( unsigned int i = 0; i < i_fiberVector.size(); ++i )
   //{
   //    l_currentFiberCurvature = 0.0f;
   //    l_currentFiberTorsion   = 0.0f;
   //    getFiberMeanCurvatureAndTorsion( i_fiberVector[i], l_currentFiberCurvature, l_currentFiberTorsion );
   //    o_meanCurvature += l_currentFiberCurvature;
   //    o_meanTorsion   += l_currentFiberTorsion;
   //}

   //o_meanCurvature /= i_fiberVector.size();
   //o_meanTorsion   /= i_fiberVector.size();

   return true;
}


///////////////////////////////////////////////////////////////////////////
// Computes the curvature and torsion for a given fiber. The reason there is only one
// function to calculate these 2 params,  is because they both use the derivative 
// calculated with the BSpline class. It would have been a waste of calculation time
// to calculate those derivative twice.
//
// i_fiberPoints        : The fiber points vector.
// o_curvature          : The output curvature of the fiber.
// o_torsion            : The output torsion of the fiber.
//
// Returns true if successful, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool SelectionObject::getFiberMeanCurvatureAndTorsion( const vector< Vector > &i_fiberPoints, float &o_curvature, float &o_torsion )
{
    // If we do not have at least 5 points, we cannot do the B-Spline algorithm.
    if( i_fiberPoints.size() < 5 )
        return false;

    Vector l_firstDerivative, l_secondDerivative, l_thirdDerivative;
    double l_currentCurvature, l_currentTorsion;
    double l_progression     = 0.0f;
    int    l_index           = 0;

    for( unsigned int i = 0; i < i_fiberPoints.size(); ++i )
    {
        if     ( i == 0 )                        { l_progression = 0.0f;  l_index = 2;                        } // The first point of the fiber.
        else if( i == 1 )                        { l_progression = 0.25f; l_index = 2;                        } // The second point of the fiber.
        else if( i == i_fiberPoints.size() - 2 ) { l_progression = 0.75f; l_index = i_fiberPoints.size() - 3; } // The before last point of the fiber.
        else if( i == i_fiberPoints.size() - 1 ) { l_progression = 1.0f;  l_index = i_fiberPoints.size() - 3; } // The last point of the fiber.
        else                                     { l_progression = 0.5f;  l_index = i;                        } // For every other points of the fiber.

        getProgressionCurvatureAndTorsion( i_fiberPoints[l_index - 2], 
                                           i_fiberPoints[l_index - 1], 
                                           i_fiberPoints[l_index], 
                                           i_fiberPoints[l_index + 1], 
                                           i_fiberPoints[l_index + 2],
                                           l_progression, 
                                           l_currentCurvature, 
                                           l_currentTorsion );

        o_curvature += l_currentCurvature;
        o_torsion   += l_currentTorsion;
     }

    o_curvature /= i_fiberPoints.size();
    o_torsion   /= i_fiberPoints.size();

    return true;
}
    
///////////////////////////////////////////////////////////////////////////
// Computes the curvature and the torsion for a given set of 5 points. Calculating both
// at the same time saves some computation time since they both use the same derivatives. 
//
// i_point0                 : The first point of the set.
// i_point1                 : The second point of the set.
// i_point2                 : The third point of the set.
// i_point3                 : The fourth point of the set.
// i_point4                 : The fifth point of the set.
// i_progression            : The progression on the spline we want to have the curvature and torsion calculated for.
// o_curvature              : The calculated curvature.
// o_torsion                : The calculated torsion.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::getProgressionCurvatureAndTorsion( const Vector &i_point0, 
                                                         const Vector &i_point1, 
                                                         const Vector &i_point2, 
                                                         const Vector &i_point3, 
                                                         const Vector &i_point4,
                                                               double  i_progression,
                                                               double &o_curvature,
                                                               double &o_torsion )
{
    // We have to use 5 points for the BSpline because the torsion required derivative or the third order.
    BSpline l_BSpline( INTERPOLATION_ON_5_POINTS );
    Vector l_firstDerivative, l_secondDerivative, l_thirdDerivative;
    
    l_BSpline.getDerivativeOrder1( i_progression, i_point0, i_point1, i_point2, i_point3, i_point4, l_firstDerivative  );
    l_BSpline.getDerivativeOrder2( i_progression, i_point0, i_point1, i_point2, i_point3, i_point4, l_secondDerivative );
    l_BSpline.getDerivativeOrder3( i_progression, i_point0, i_point1, i_point2, i_point3, i_point4, l_thirdDerivative  );

    o_curvature = Helper::calculateCurvature( l_firstDerivative, l_secondDerivative );
    o_torsion   = Helper::calculateTorsion( l_firstDerivative, l_secondDerivative, l_thirdDerivative );
}

///////////////////////////////////////////////////////////////////////////
// Computes the curvature for a given set of 5 points. 
//
// i_point0                 : The first point of the set.
// i_point1                 : The second point of the set.
// i_point2                 : The third point of the set.
// i_point3                 : The fourth point of the set.
// i_point4                 : The fifth point of the set.
// i_progression            : The progression on the spline we want to have the curvature calculated for.
// o_curvature              : The calculated curvature.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::getProgressionCurvature( const Vector &i_point0, 
                                               const Vector &i_point1, 
                                               const Vector &i_point2, 
                                               const Vector &i_point3, 
                                               const Vector &i_point4,
                                                     double  i_progression,
                                                     double &o_curvature )
{
    // The calculation of the curvature could be done with INTERPOLATION_ON_4_POINTS since
    // we do not need the derivative or the third order, but its easier to use 5 points.
    BSpline l_BSpline( INTERPOLATION_ON_5_POINTS );
    Vector l_firstDerivative, l_secondDerivative;
    
    l_BSpline.getDerivativeOrder1( i_progression, i_point0, i_point1, i_point2, i_point3, i_point4, l_firstDerivative  );
    l_BSpline.getDerivativeOrder2( i_progression, i_point0, i_point1, i_point2, i_point3, i_point4, l_secondDerivative );

    o_curvature = Helper::calculateCurvature( l_firstDerivative, l_secondDerivative );
}

///////////////////////////////////////////////////////////////////////////
// Computes the torsion for a given set of 5 points.
//
// i_point0                 : The first point of the set.
// i_point1                 : The second point of the set.
// i_point2                 : The third point of the set.
// i_point3                 : The fourth point of the set.
// i_point4                 : The fifth point of the set.
// i_progression            : The progression on the spline we want to have the curvature and torsion calculated for.
// o_torsion                : The calculated torsion.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::getProgressionTorsion( const Vector &i_point0, 
                                             const Vector &i_point1, 
                                             const Vector &i_point2, 
                                             const Vector &i_point3, 
                                             const Vector &i_point4,
                                                   double  i_progression,
                                                   double &o_torsion )
{
    // We have to use 5 points for the BSpline because the torsion required derivative or the third order.
    BSpline l_BSpline( INTERPOLATION_ON_5_POINTS );
    Vector l_firstDerivative, l_secondDerivative, l_thirdDerivative;
    
    l_BSpline.getDerivativeOrder1( i_progression, i_point0, i_point1, i_point2, i_point3, i_point4, l_firstDerivative  );
    l_BSpline.getDerivativeOrder2( i_progression, i_point0, i_point1, i_point2, i_point3, i_point4, l_secondDerivative );
    l_BSpline.getDerivativeOrder3( i_progression, i_point0, i_point1, i_point2, i_point3, i_point4, l_thirdDerivative  );

    o_torsion = Helper::calculateTorsion( l_firstDerivative, l_secondDerivative, l_thirdDerivative );
}

///////////////////////////////////////////////////////////////////////////
// We compute the dispersion in the following manner:
// We create the two tightest circles we can fit around the min and max cross sections.
// ( m_crossSectionsPoints[m_minCrossSectionIndex], m_crossSectionsPoints[m_maxCrossSectionIndex] ).
// We then create a cone using these two circles.
// A portion of the cone's height is equal to the length of the mean fiber ( m_meanFiberPoints )
// between the min and max cross section planes.
//
// o_dispersion         : The output mean dipersion of the given fiber.
//
// Returns true if successful, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool SelectionObject::getFiberDispersion( float &o_dispersion )
{
    o_dispersion = 0.0f;

    if( m_meanFiberPoints.size()                             == 0 ||  
        m_crossSectionsPoints.size()                         == 0 ||
        m_crossSectionsPoints[m_maxCrossSectionIndex].size() == 0 ||
        m_crossSectionsPoints[m_minCrossSectionIndex].size() == 0 )
    {
        return false;
    }

    vector< Vector > l_fiberPoints;

    if( m_minCrossSectionIndex < m_maxCrossSectionIndex )
    {
        for( unsigned int i = m_minCrossSectionIndex; i <= m_maxCrossSectionIndex; ++i )
            l_fiberPoints.push_back( m_meanFiberPoints[i] ) ;
    }
    else if( m_minCrossSectionIndex > m_maxCrossSectionIndex )
    {
        for( unsigned int i = m_maxCrossSectionIndex; i <= m_minCrossSectionIndex; ++i )
            l_fiberPoints.push_back( m_meanFiberPoints[i] ) ;
    }
    else
        return false;
     
    float l_coneLength = 0.0f;
    getFiberLength( l_fiberPoints, l_coneLength );

    // Calculating the max and min cross section circle radius.
    float l_maxRadius = getMaxDistanceBetweenPoints( m_crossSectionsPoints[m_maxCrossSectionIndex] ) / 2.0f;
    float l_minRadius = getMaxDistanceBetweenPoints( m_crossSectionsPoints[m_minCrossSectionIndex] ) / 2.0f;

    l_coneLength = l_coneLength / ( 1.0f - l_minRadius / l_maxRadius );

    float hypotenuse = sqrt( l_coneLength*l_coneLength + l_maxRadius*l_maxRadius );

    // We calculate the dispersion angle in degrees.
    o_dispersion = asin( l_maxRadius / hypotenuse ) * 180.0f / M_PI;

    return true;
}

///////////////////////////////////////////////////////////////////////////
// This fonction will calculate the distance between all the points in i_points
// and return the biggest one, it will also return the index of the 2 points that
// gave this distance.
//
// i_points             : The vector that contains the points we will calculate the distance from.
// o_firstPointIndex    : The first point index forming the pair of points with the biggest distance.
// o_secondPointIndex   : The second point index forming the pair of points with the biggest distance.
//
// Returns the max distance found.
///////////////////////////////////////////////////////////////////////////
float SelectionObject::getMaxDistanceBetweenPoints( const vector< Vector > &i_points, int* o_firstPointIndex, int* o_secondPointIndex )
{
    float l_currentDistance, l_maxDistance = 0.0f;
    
    for( unsigned int i = 0; i < i_points.size(); ++i )
        for( unsigned int j = 0; j < i_points.size(); ++j )
            if( i > j )
            {
                l_currentDistance = ( i_points[i] - i_points[j] ).getLength();
                
                if( l_currentDistance > l_maxDistance )
                {
                    l_maxDistance = l_currentDistance;
                    if( o_firstPointIndex  ) *o_firstPointIndex  = i;
                    if( o_secondPointIndex ) *o_secondPointIndex = j;
                }
            }

    return l_maxDistance;
}

///////////////////////////////////////////////////////////////////////////
// This is the basic draw function that all the selection object share.
// After doing some basic calculation this function will call drawObject()
// to let its subclass do there specific draw.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::draw()
{
    if( !m_isActive || !isSelectionObject())
    {
        if ( m_objectType == CISO_SURFACE_TYPE && m_isSelected)
            drawFibersInfo();
        return;
    }

    // We only display those if the current box is the selected one and if we are supposed to display them.
    if( m_isSelected )
        drawFibersInfo();
    
    if( ! m_isVisible )
        return;

    GLfloat l_color[] = { 0.5f, 0.5f, 0.5f, 0.5f };

    if( m_isMaster )
    {
        l_color[0] = 0.0f; // Red
        l_color[1] = 1.0f; // Green
        l_color[2] = 1.0f; // Blue
    }
    else 
    {
        if ( ! m_isNOT )
        {
            l_color[0] = 0.0f; // Red
            l_color[1] = 1.0f; // Green
            l_color[2] = 0.0f; // Blue
        }
        else
        {
            l_color[0] = 1.0f; // Red
            l_color[1] = 0.0f; // Green
            l_color[2] = 0.0f; // Blue
        }
    }

    if( m_isSelected )
        l_color[3] = 0.4f; // Alpha
    else
        l_color[3] = 0.2f; // Alpha

    // Because each type of selection object is unique, this function will
    // draw the selection object according to its specifications.
    drawObject( l_color );
}

void SelectionObject::drawIsoSurface()
{
    if( ! m_isActive || ! m_isVisible ) 
        return;

    if( m_gfxDirty )
    {
        m_isosurface->setThreshold( m_threshold );
        m_isosurface->GenerateWithThreshold();
        m_gfxDirty = false;
    }

    glColor3ub( m_color.Red(), m_color.Green(), m_color.Blue() );
    m_isosurface->draw();
}

///////////////////////////////////////////////////////////////////////////
// Updates the status bar at the button of the application window.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::updateStatusBar()
{
    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    MyApp::frame->GetStatusBar()->SetStatusText( m_name, 1 );
    MyApp::frame->GetStatusBar()->SetStatusText( wxString::Format( wxT( "Position: %.2f, %.2f, %.2f  Size: %.2f, %.2f, %.2f" ),
                                                                 m_center.x, m_center.y, m_center.z,
                                                                 m_size.x * voxelX, m_size.y * voxelY, m_size.z * voxelZ ),
                                                                 2 );
}

///////////////////////////////////////////////////////////////////////////
// Will draw the fibers info (the mean fiber, the cross sections and the dispersion cone)
// if this selection object is of type MASTER_OBJECT, if not then its of type CHILD_OBJECT, 
// we get its master object because the master object is the one coitaining the fibers that are
// in branch between itself and its child object.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::drawFibersInfo()
{
    if( ! m_isMaster )
    {
        wxTreeCtrl*      l_treeWidget   = MyApp::frame->m_pTreeWidget;
        SelectionObject* l_masterObject = (SelectionObject*)( l_treeWidget->GetItemData( l_treeWidget->GetItemParent( m_treeId ) ) );

        l_masterObject->drawFibersInfo();
        return;
    }

    glDisable( GL_DEPTH_TEST);
    
    // Draw the mean fiber.
    drawThickFiber( m_meanFiberPoints, (float)THICK_FIBER_THICKNESS/100.0f, THICK_FIBER_NB_TUBE_EDGE );
    drawConvexHull();
    drawCrossSections();
    drawDispersionCone();

    glEnable( GL_DEPTH_TEST);
}

///////////////////////////////////////////////////////////////////////////
// This function will draw the dispersion cone formed by the selected fibers.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::drawDispersionCone()
{
    // This vector will contain all the dispersion circle to draw depending
    // on the m_displayDispersionCone currently selected.
    vector< vector< Vector > > l_allCirclesPoints;
    
    switch( m_displayDispersionCone )
    {
        case DC_NOTHING : return;
        case DC_MIN_MAX_ONLY : 
        {
            l_allCirclesPoints.resize( m_crossSectionsPoints.size() );
            getDispersionCircle( m_crossSectionsPoints[m_minCrossSectionIndex],
                                 m_crossSectionsNormals[m_minCrossSectionIndex],
                                 l_allCirclesPoints[m_minCrossSectionIndex] );
            getDispersionCircle( m_crossSectionsPoints[m_maxCrossSectionIndex],
                                 m_crossSectionsNormals[m_maxCrossSectionIndex],
                                 l_allCirclesPoints[m_maxCrossSectionIndex] );
            drawSimpleCircles( l_allCirclesPoints );
            break;
        }
        case DC_EVERYTHING :
        case DC_FULL_LINE_CONE :
        case DC_FULL_CONE :
        {
            l_allCirclesPoints.resize( m_crossSectionsPoints.size() );
            for( unsigned int i = 0; i < m_crossSectionsPoints.size(); ++i )
                getDispersionCircle( m_crossSectionsPoints[i], m_crossSectionsNormals[i], l_allCirclesPoints[i] );
            
            switch( m_displayDispersionCone )
            {
                case DC_EVERYTHING     : drawSimpleCircles( l_allCirclesPoints );       break;
                case DC_FULL_LINE_CONE : drawTube( l_allCirclesPoints, GL_LINE_LOOP );  break;
                case DC_FULL_CONE      : drawTube( l_allCirclesPoints, GL_QUAD_STRIP ); break;
                default : return;
            }
        }
        default : return;
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will draw multiple circles formed by the points inside i_allCirclesPoints. 
//
// i_circlesPoints          : The points forming the circles we need to draw.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::drawSimpleCircles( const vector< vector< Vector > > &i_circlesPoints )
{
    // For all the emelement that should be drawn.
    for( unsigned int i = 0; i < i_circlesPoints.size(); ++i )
    {
        if( i_circlesPoints[i].size() <= 0 )
            continue;

        getCrossSectionAreaColor( i );

        // Form simple circle around all the cross sections.
        glBegin( GL_LINE_LOOP );
            for( unsigned int j = 0; j < i_circlesPoints[i].size(); ++j )
                glVertex3f( i_circlesPoints[i][j].x, i_circlesPoints[i][j].y, i_circlesPoints[i][j].z );

            // This is to actually have a full circle the first point need to be the last point.
            glVertex3f( i_circlesPoints[i][0].x, i_circlesPoints[i][0].y, i_circlesPoints[i][0].z );
        glEnd();
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will draw a cone formed by a series of circles. 
// The circles all need to have the same amount of points.
//
// i_allCirclesPoints           : The points forming the circles we will draw the cone with.
// i_tubeType                   : The type of primitive that we will use to render the tube.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::drawTube( const vector< vector< Vector > > &i_allCirclesPoints, GLenum i_tubeType )
{           
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                 
    unsigned int l_secondCircleIndex;

    for( unsigned int i = 0; i < i_allCirclesPoints.size(); ++i )
    {
        // Make sure that the circle at index i actually has some points.
        if( i_allCirclesPoints[i].size() <= 0 )
            continue;
        else // Find the closest circle from the circle at index i that also has some points.
        {
            for( l_secondCircleIndex = i+1; l_secondCircleIndex < i_allCirclesPoints.size(); ++l_secondCircleIndex )
            {
                if( i_allCirclesPoints[l_secondCircleIndex].size() <= 0 )
                    continue;
                else
                    break;
            }
            // make sure that we did found a second circle with some points, 
            if( ! ( l_secondCircleIndex < i_allCirclesPoints.size() ) )
                break;
        }

        // Make the tube by linking two circles together.
        glBegin( i_tubeType );
            for( unsigned int k = 0; k < i_allCirclesPoints[i].size(); ++k )
            {                
                getCrossSectionAreaColor( i );
                glVertex3f( i_allCirclesPoints[i][k].x, i_allCirclesPoints[i][k].y, i_allCirclesPoints[i][k].z );

                getCrossSectionAreaColor( l_secondCircleIndex );
                glVertex3f( i_allCirclesPoints[l_secondCircleIndex][k].x,
                            i_allCirclesPoints[l_secondCircleIndex][k].y,
                            i_allCirclesPoints[l_secondCircleIndex][k].z );
            } 

            // This is to actually have a full tube the first point need to be the last point.
            getCrossSectionAreaColor( i );
            glVertex3f( i_allCirclesPoints[i][0].x, i_allCirclesPoints[i][0].y, i_allCirclesPoints[i][0].z );

            getCrossSectionAreaColor( l_secondCircleIndex );
            glVertex3f( i_allCirclesPoints[l_secondCircleIndex][0].x,
                        i_allCirclesPoints[l_secondCircleIndex][0].y,
                        i_allCirclesPoints[l_secondCircleIndex][0].z );
        glEnd();
    }
    
    glDisable( GL_BLEND );
}

///////////////////////////////////////////////////////////////////////////
// This function will calculate the points forming a circle formed by the arguments.
// The circle will be the smallest circle possible to fit around the points inside i_crossSectionPoints.
//
// i_crossSectionPoints         : The points representing the cross section we want to draw a circle around.
// i_crossSectionNormal         : The value of the normal of this cross section.
// o_circlePoints               : This vector will contain all the points generated for this circle.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::getDispersionCircle( const vector< Vector > &i_crossSectionPoints, 
                                           const Vector           &i_crossSectionNormal, 
                                                 vector< Vector > &o_circlePoints )
{
    if( i_crossSectionPoints.size() < 3 )
        return;
    
    int l_firstPointIndex, l_secondPointIndex;

    // The radius of the circle is the half of the distance between the 2 points forming the max distance.
    float l_radius = getMaxDistanceBetweenPoints( i_crossSectionPoints, &l_firstPointIndex, &l_secondPointIndex ) / 2.0f;

    // The center of the circle is the center of the vector formed by the the 2 points forming the max distance.
    Vector l_center = ( ( i_crossSectionPoints[l_secondPointIndex] - i_crossSectionPoints[l_firstPointIndex] ) / 2.0f )
                      + i_crossSectionPoints[l_firstPointIndex];

    Helper::getCirclePoints( l_center, i_crossSectionNormal, l_radius, DISPERSION_CONE_NB_TUBE_EDGE, o_circlePoints );
}

///////////////////////////////////////////////////////////////////////////
// This function will set a color depending on the cross section index passed in argument.
// The cross section with the maximum area will be pure blue, and the cross section with 
// the minimum area will be pure red, all the other will be a mix of red and blue.
//
// i_index          : The index representing the position of the cross section we
//                    want to use to set the color.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::getCrossSectionAreaColor( unsigned int i_index )
{
    float l_u = ( m_crossSectionsAreas[i_index] - m_crossSectionsAreas[m_minCrossSectionIndex] ) /
                ( m_crossSectionsAreas[m_maxCrossSectionIndex] - m_crossSectionsAreas[m_minCrossSectionIndex] );

    glColor4f( 1.0f - l_u, 0.0f, l_u, 0.35f );
}

void SelectionObject::FlipNormals()
{
    if( m_isosurface != NULL && 
        m_isosurface->m_tMesh != NULL)
    {
        m_isosurface->m_tMesh->flipNormals();
        m_isosurface->clean();
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the correct information in the fiber info grid.
///////////////////////////////////////////////////////////////////////////
void SelectionObject::SetFiberInfoGridValues()
{
    if( !m_isMaster )
    {
        wxTreeCtrl*      l_treeWidget   = MyApp::frame->m_pTreeWidget;
        SelectionObject* l_masterObject = (SelectionObject*)( l_treeWidget->GetItemData( l_treeWidget->GetItemParent( m_treeId ) ) );
        l_masterObject->SetFiberInfoGridValues();
        return;
    }
    if( m_pToggleCalculatesFibersInfo->GetValue() )
    {
        FibersInfoGridParams l_params;
        calculateGridParams( l_params );

        m_pGridFibersInfo->SetCellValue( 0,  0, wxString::Format( wxT( "%d" ),   l_params.m_count            ) );
        m_pGridFibersInfo->SetCellValue( 1,  0, wxString::Format( wxT( "%.2f" ), l_params.m_meanValue        ) );
        m_pGridFibersInfo->SetCellValue( 2,  0, wxString::Format( wxT( "%.2f" ), l_params.m_meanLength       ) );
        m_pGridFibersInfo->SetCellValue( 3,  0, wxString::Format( wxT( "%.2f" ), l_params.m_minLength        ) );
        m_pGridFibersInfo->SetCellValue( 4,  0, wxString::Format( wxT( "%.2f" ), l_params.m_maxLength        ) );
        m_pGridFibersInfo->SetCellValue( 5,  0, wxString::Format( wxT( "%.5f" ), l_params.m_meanCurvature    ) );
        m_pGridFibersInfo->SetCellValue( 6,  0, wxString::Format( wxT( "%.5f" ), l_params.m_meanTorsion      ) );
    }
}

void SelectionObject::setShowConvexHullOption( bool i_val )
{
//     m_pLblConvexHullOpacity->Show( i_val );
//     m_pSliderConvexHullOpacity->Show( i_val);
//     m_pBtnSelectConvexHullColor->Enable( i_val );
}

void SelectionObject::UpdateMeanValueTypeBox()
{
    vector< Anatomy* > dataSets = DatasetManager::getInstance()->getAnatomies();
    
    if( dataSets.size() != m_pCBSelectDataSet->GetCount() )
    {
        int oldIndex = -1;
        wxString oldName = m_pCBSelectDataSet->GetStringSelection();
        
        m_pCBSelectDataSet->Clear();
        
        for( unsigned int i( 0 ); i < dataSets.size(); ++i )
        {
            m_pCBSelectDataSet->Insert( dataSets[i]->getName().BeforeFirst('.'), m_pCBSelectDataSet->GetCount() );
            if( oldName == dataSets[i]->getName().BeforeFirst('.') )
            {
                oldIndex = i;
            }
        }
        
        if( oldIndex < 0 )
        {
            oldIndex = 0;
        }
        else if( static_cast<unsigned int>( oldIndex ) >= m_pCBSelectDataSet->GetCount() )
        {
            oldIndex = m_pCBSelectDataSet->GetCount() - 1;
        }
        
        m_pCBSelectDataSet->SetSelection(oldIndex);
    }
}

void SelectionObject::setShowMeanFiberOption( bool i_val )
{
    m_pLblColoring->Show( i_val );
    m_pRadCustomColoring->Show( i_val );
    m_pRadNormalColoring->Show( i_val );
    m_pLblMeanFiberOpacity->Show( i_val );
    m_pSliderMeanFiberOpacity->Show( i_val );
}

void SelectionObject::updateMeanFiberOpacity()
{
    setMeanFiberOpacity( ( m_pSliderMeanFiberOpacity->GetValue() + (float)m_pSliderMeanFiberOpacity->GetMin() ) / (float)m_pSliderMeanFiberOpacity->GetMax() );
}

void SelectionObject::createPropertiesSizer( PropertiesWindow *pParent )
{
    SceneObject::createPropertiesSizer( pParent );

    wxBoxSizer *pBoxMain = new wxBoxSizer( wxVERTICAL );

    m_meanFiberOpacity = 0.35f;
    m_convexHullOpacity = 0.35f;
    m_meanFiberColorationMode = NORMAL_COLOR;

    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    //////////////////////////////////////////////////////////////////////////

    wxImage bmpDelete(          MyApp::iconsPath + wxT( "delete.png" ),      wxBITMAP_TYPE_PNG );
    wxImage bmpColor(           MyApp::iconsPath + wxT( "colorSelect.png" ), wxBITMAP_TYPE_PNG );
    wxImage bmpMeanFiberColor(  MyApp::iconsPath + wxT( "colorSelect.png" ), wxBITMAP_TYPE_PNG );
    wxImage bmpConvexHullColor( MyApp::iconsPath + wxT( "colorSelect.png" ), wxBITMAP_TYPE_PNG );

    wxButton *pBtnChangeName          = new wxButton( pParent, wxID_ANY, wxT( "Rename" ) );
    wxButton *pBtnFlipNormal          = new wxButton( pParent, wxID_ANY, wxT( "Flip Normal" ) );
    wxButton *pBtnSelectColorFibers   = new wxButton( pParent, wxID_ANY, wxT( "Select Fibers Color" ) );
    wxButton *pBtnNewColorVolume      = new wxButton( pParent, wxID_ANY, wxT( "New Color map" ) );
    wxButton *pBtnNewDensityVolume    = new wxButton( pParent, wxID_ANY, wxT( "New Density map" ) );
    wxButton *pBtnSetAsDistanceAnchor = new wxButton( pParent, wxID_ANY, wxT( "Set As Anchor" ) );
    //m_pbtnDisplayCrossSections      = new wxButton( pParent, wxID_ANY, wxT( "Display Cross Section (C.S.)" ) );
    //m_pbtnDisplayDispersionTube     = new wxButton( pParent, wxID_ANY, wxT( "Display Dispersion Tube" ) );
    wxBitmapButton *pBtnDelete      = new wxBitmapButton( pParent, wxID_ANY, bmpDelete, wxDefPosition, wxSize( 20, -1 ) );
    wxBitmapButton *pBtnSelectColor = new wxBitmapButton( pParent, wxID_ANY, bmpColor );
    m_pBtnSelectMeanFiberColor      = new wxBitmapButton( pParent, wxID_ANY, bmpMeanFiberColor );
//     m_pBtnSelectConvexHullColor     = new wxBitmapButton( pParent, wxID_ANY, bmpConvexHullColor );
    m_pToggleVisibility           = new wxToggleButton( pParent, wxID_ANY, wxT( "Visible" ), wxDefPosition, wxSize( 20, -1 ) );
    m_pToggleActivate             = new wxToggleButton( pParent, wxID_ANY, wxT( "Activate" ), wxDefPosition, wxSize( 20, -1 ) );
    wxToggleButton *pToggleAndNot = new wxToggleButton( pParent, wxID_ANY, wxT( "And / Not" ) );
    m_pToggleCalculatesFibersInfo = new wxToggleButton( pParent, wxID_ANY, wxT( "Calculate Fibers Stats" ) );
    m_pToggleDisplayMeanFiber     = new wxToggleButton( pParent, wxID_ANY, wxT( "Display Mean Fiber" ) );
//     m_pToggleDisplayConvexHull    = new wxToggleButton( pParent, wxID_ANY, wxT( "Display convex hull" ) );
    m_pLblColoring          = new wxStaticText( pParent, wxID_ANY, wxT( "Coloring" ) );
    m_pLblMeanFiberOpacity  = new wxStaticText( pParent, wxID_ANY, wxT( "Opacity" ) );
//     m_pLblConvexHullOpacity = new wxStaticText( pParent, wxID_ANY, wxT( "Opacity" ) );
    m_pRadCustomColoring = new wxRadioButton( pParent, wxID_ANY, _T( "Custom" ), wxDefPosition, wxDefSize, wxRB_GROUP );
    m_pRadNormalColoring = new wxRadioButton( pParent, wxID_ANY, _T( "Normal" ) );
    m_pSliderMeanFiberOpacity  = new wxSlider( pParent, wxID_ANY, 35, 0, 100, wxDefPosition, wxSize( 40, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
//     m_pSliderConvexHullOpacity = new wxSlider( pParent, wxID_ANY, 35, 0, 100, wxDefPosition, wxSize( 40, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pTxtName  = new wxTextCtrl( pParent, wxID_ANY, getName(), wxDefPosition, wxDefSize, wxTE_CENTRE | wxTE_READONLY );
    m_pTxtBoxX  = new wxTextCtrl( pParent, wxID_ANY, wxString::Format( wxT( "%.2f" ), m_center.x ), wxDefPosition, wxSize( 10, -1 ) );
    m_pTxtBoxY  = new wxTextCtrl( pParent, wxID_ANY, wxString::Format( wxT( "%.2f" ), m_center.y ), wxDefPosition, wxSize( 10, -1 ) );
    m_pTxtBoxZ  = new wxTextCtrl( pParent, wxID_ANY, wxString::Format( wxT( "%.2f" ), m_center.z ), wxDefPosition, wxSize( 10, -1 ) );
    m_pTxtSizeX = new wxTextCtrl( pParent, wxID_ANY, wxString::Format( wxT( "%.2f" ), m_size.x * voxelX ), wxDefPosition, wxSize( 10, -1 ) );
    m_pTxtSizeY = new wxTextCtrl( pParent, wxID_ANY, wxString::Format( wxT( "%.2f" ), m_size.y * voxelY ), wxDefPosition, wxSize( 10, -1 ) );
    m_pTxtSizeZ = new wxTextCtrl( pParent, wxID_ANY, wxString::Format( wxT( "%.2f" ), m_size.z * voxelZ ), wxDefPosition, wxSize( 10, -1 ) );

    //////////////////////////////////////////////////////////////////////////

    wxFont font = m_pTxtName->GetFont();
    font.SetPointSize(10);
    font.SetWeight( wxFONTWEIGHT_BOLD );
    m_pTxtName->SetFont( font );
    m_pTxtName->SetBackgroundColour( *wxLIGHT_GREY );

    //////////////////////////////////////////////////////////////////////////

    pBoxMain->Add( m_pTxtName, 0, wxALIGN_CENTER | wxEXPAND | wxALL, 1 );

    wxBoxSizer *pBoxSizer = new wxBoxSizer( wxHORIZONTAL );
    pBoxSizer->Add( m_pToggleVisibility, 1, wxEXPAND | wxALL, 1 );
    pBoxSizer->Add( m_pToggleActivate,   1, wxEXPAND | wxALL, 1 );
    pBoxSizer->Add( pBtnDelete,          1, wxEXPAND | wxALL, 1 );
    pBoxMain->Add( pBoxSizer, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    //////////////////////////////////////////////////////////////////////////

    pBoxSizer = new wxBoxSizer( wxHORIZONTAL );
    pBoxSizer->Add( pToggleAndNot,  1, wxEXPAND | wxALL, 1 );
    pBoxSizer->Add( pBtnChangeName, 1, wxEXPAND | wxALL, 1 );
    pBoxMain->Add( pBoxSizer, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    //////////////////////////////////////////////////////////////////////////

    pBoxSizer = new wxBoxSizer( wxHORIZONTAL );
    pBoxSizer->Add( pBtnFlipNormal,  3, wxEXPAND | wxALL, 1 );
    pBoxSizer->Add( pBtnSelectColor, 1, wxEXPAND | wxALL, 1 );
    pBoxMain->Add( pBoxSizer, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    //////////////////////////////////////////////////////////////////////////

    pBoxMain->Add( pBtnSelectColorFibers,   0, wxEXPAND | wxLEFT | wxRIGHT, 24 );
    pBoxMain->Add( pBtnNewColorVolume,      0, wxEXPAND | wxLEFT | wxRIGHT, 24 );
    pBoxMain->Add( pBtnNewDensityVolume,    0, wxEXPAND | wxLEFT | wxRIGHT, 24 );
    pBoxMain->Add( pBtnSetAsDistanceAnchor, 0, wxEXPAND | wxLEFT | wxRIGHT, 24 );

    pBoxMain->AddSpacer( 8 );

    pBoxMain->Add( m_pToggleCalculatesFibersInfo, 0, wxEXPAND | wxLEFT | wxRIGHT, 24 );

    pBoxMain->AddSpacer( 2 );

    //////////////////////////////////////////////////////////////////////////

    m_pGridFibersInfo = new wxGrid( pParent, wxID_ANY );
    m_pGridFibersInfo->SetRowLabelAlignment( wxALIGN_LEFT, wxALIGN_CENTER );
    font = m_pGridFibersInfo->GetFont();
    font.SetPointSize( 8 );
    font.SetWeight( wxFONTWEIGHT_BOLD );
    m_pGridFibersInfo->SetFont( font );
    m_pGridFibersInfo->SetColLabelSize( 2 );
    m_pGridFibersInfo->CreateGrid( 7, 1, wxGrid::wxGridSelectCells );
    m_pGridFibersInfo->SetColLabelValue( 0, wxT( "" ) );
    m_pGridFibersInfo->SetRowLabelValue( 0, wxT( "Count" ) );
    m_pGridFibersInfo->SetRowLabelValue( 1, wxT( "Mean Value" ) );
    m_pGridFibersInfo->SetRowLabelValue( 2, wxT( "Mean Length (mm)" ) );
    m_pGridFibersInfo->SetRowLabelValue( 3, wxT( "Min Length (mm)" ) );
    m_pGridFibersInfo->SetRowLabelValue( 4, wxT( "Max Length (mm)" ) );
//     m_pGridFibersInfo->SetRowLabelValue( 5, wxT( "Mean C. S. (mm)" ) );
//     m_pGridFibersInfo->SetRowLabelValue( 6, wxT( "Min C. S. (mm)" ) );
//     m_pGridFibersInfo->SetRowLabelValue( 7, wxT( "Max C. S. (mm)" ) );
    m_pGridFibersInfo->SetRowLabelValue( 5, wxT( "Mean Curvature" ) );
    m_pGridFibersInfo->SetRowLabelValue( 6, wxT( "Mean Torsion" ) );
//     m_pGridFibersInfo->SetRowLabelValue( 10, wxT( "Dispersion" ) );

    m_pGridFibersInfo->SetRowLabelSize( 120 );

    pBoxMain->Add( m_pGridFibersInfo, 0, wxALIGN_CENTER | wxALL, 0 );

    pBoxMain->AddSpacer( 2 );

    //////////////////////////////////////////////////////////////////////////

// Because of a bug on the Windows version of this, we currently do not use this wxChoice on Windows.
// Will have to be fixed.
#ifndef __WXMSW__    
    m_pCBSelectDataSet = new wxChoice( pParent, wxID_ANY, wxDefaultPosition, wxSize( 140, -1 ) );
    m_pLabelAnatomy = new wxStaticText( pParent, wxID_ANY, wxT("Anatomy file : ") );
    pBoxSizer = new wxBoxSizer( wxHORIZONTAL );
    pBoxSizer->Add( m_pLabelAnatomy, 0, wxEXPAND, 0 );
    pBoxSizer->Add( m_pCBSelectDataSet, 0, wxEXPAND, 0 );

    pBoxMain->Add( pBoxSizer, 0, wxEXPAND, 0 );
    pParent->Connect( m_pCBSelectDataSet->GetId(), wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( PropertiesWindow::OnMeanComboBoxSelectionChange ) );
    pBoxMain->AddSpacer( 2 );
#endif

    //////////////////////////////////////////////////////////////////////////

    pBoxSizer = new wxBoxSizer( wxHORIZONTAL );
    pBoxSizer->Add( m_pToggleDisplayMeanFiber,  3, wxEXPAND, 0 );
    pBoxSizer->Add( m_pBtnSelectMeanFiberColor, 1, wxEXPAND, 0 );
    pBoxMain->Add( pBoxSizer, 0, wxEXPAND, 0 );

    pBoxMain->AddSpacer( 8 );

    //////////////////////////////////////////////////////////////////////////

    wxBoxSizer *pBoxColoring = new wxBoxSizer( wxVERTICAL );
    pBoxColoring->Add( m_pLblColoring, 0, wxALIGN_LEFT | wxALL, 1 );

    wxBoxSizer *pBoxColoringRadios = new wxBoxSizer( wxVERTICAL );
    pBoxColoringRadios->Add( m_pRadCustomColoring, 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxColoringRadios->Add( m_pRadNormalColoring, 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxColoring->Add( pBoxColoringRadios, 0, wxALIGN_LEFT | wxLEFT, 32 );

    pBoxMain->Add( pBoxColoring, 0, wxFIXED_MINSIZE | wxEXPAND | wxTOP | wxBOTTOM, 8 );

    //////////////////////////////////////////////////////////////////////////

    pBoxSizer = new wxBoxSizer( wxHORIZONTAL );
    pBoxSizer->Add( m_pLblMeanFiberOpacity,    0, wxEXPAND, 0 );
    pBoxSizer->Add( m_pSliderMeanFiberOpacity, 1, wxEXPAND, 0 );
    pBoxMain->Add( pBoxSizer, 0, wxEXPAND | wxALL, 1 );

    //////////////////////////////////////////////////////////////////////////

    //m_pPropertiesSizer->Add( m_pbtnDisplayCrossSections,  0,wxALIGN_CENTER );
    //m_pPropertiesSizer->Add( m_pbtnDisplayDispersionTube, 0,wxALIGN_CENTER );

    //////////////////////////////////////////////////////////////////////////

    m_pToggleCalculatesFibersInfo->Enable( getIsMaster() ); //bug with some fibers dataset sets

//     pBoxSizer = new wxBoxSizer( wxHORIZONTAL );
//     pBoxSizer->Add( m_pToggleDisplayConvexHull,  3, wxALIGN_CENTER | wxEXPAND | wxALL, 1 );
//     pBoxSizer->Add( m_pBtnSelectConvexHullColor, 1, wxALIGN_CENTER | wxEXPAND | wxALL, 1 );
//     pBoxMain->Add( pBoxSizer, 0, wxEXPAND | wxALL, 1 );

    //////////////////////////////////////////////////////////////////////////

//     pBoxSizer = new wxBoxSizer( wxHORIZONTAL );
//     pBoxSizer->Add( m_pLblConvexHullOpacity, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
//     pBoxSizer->Add( m_pSliderConvexHullOpacity, 1, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );
//     pBoxMain->Add( pBoxSizer, 0, wxEXPAND | wxALL, 1 );

    //////////////////////////////////////////////////////////////////////////

    pBoxMain->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Position" ) ), 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 1 );
    pBoxSizer = new wxBoxSizer( wxHORIZONTAL );
    pBoxSizer->Add( new wxStaticText( pParent, wxID_ANY, wxT( "x:" ) ), 0, wxALL, 1 );
    pBoxSizer->Add( m_pTxtBoxX, 1, wxEXPAND | wxALL, 1 );
    pBoxSizer->Add( new wxStaticText( pParent, wxID_ANY, wxT( "y:" ) ), 0, wxALL, 1 );
    pBoxSizer->Add( m_pTxtBoxY, 1, wxEXPAND | wxALL, 1 );
    pBoxSizer->Add( new wxStaticText( pParent, wxID_ANY, wxT( "z:" ) ), 0, wxALL, 1 );
    pBoxSizer->Add( m_pTxtBoxZ, 1, wxEXPAND | wxALL, 1 );
    pBoxMain->Add( pBoxSizer, 0, wxEXPAND, 0 );

    pBoxMain->AddSpacer( 8 );

    //////////////////////////////////////////////////////////////////////////

    pBoxMain->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Size" ) ), 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 1 );
    pBoxSizer = new wxBoxSizer( wxHORIZONTAL );
    pBoxSizer->Add( new wxStaticText( pParent, wxID_ANY, wxT( "x:" ) ), 0, wxALL, 1 );
    pBoxSizer->Add( m_pTxtSizeX, 1, wxEXPAND | wxALL, 1 );
    pBoxSizer->Add( new wxStaticText( pParent, wxID_ANY, wxT( "y:" ) ), 0, wxALL, 1 );
    pBoxSizer->Add( m_pTxtSizeY, 1, wxEXPAND | wxALL, 1 );
    pBoxSizer->Add( new wxStaticText( pParent, wxID_ANY, wxT( "z:" ) ), 0, wxALL, 1 );
    pBoxSizer->Add( m_pTxtSizeZ, 1, wxEXPAND | wxALL, 1 );
    pBoxMain->Add( pBoxSizer, 0, wxEXPAND, 0 );

    //////////////////////////////////////////////////////////////////////////

    m_pRadNormalColoring->SetValue( true );
    pBtnNewColorVolume->Enable( getIsMaster() );
    pBtnNewDensityVolume->Enable( getIsMaster() );
    pToggleAndNot->Enable( !getIsMaster() && m_objectType != CISO_SURFACE_TYPE );
    pBtnFlipNormal->Enable( m_objectType == CISO_SURFACE_TYPE );
    pBtnSelectColor->Enable( m_objectType == CISO_SURFACE_TYPE );
    pBtnSetAsDistanceAnchor->Enable( m_objectType == CISO_SURFACE_TYPE );

    m_pPropertiesSizer->Add( pBoxMain, 1, wxFIXED_MINSIZE | wxEXPAND, 0 );

    //////////////////////////////////////////////////////////////////////////

    pParent->Connect( pBtnChangeName->GetId(),          wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnRenameBox ) );
    pParent->Connect( pBtnFlipNormal->GetId(),          wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnVoiFlipNormals ) );
    pParent->Connect( pBtnSelectColorFibers->GetId(),   wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnAssignColor ) );
    pParent->Connect( pBtnNewColorVolume->GetId(),      wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnCreateFibersColorTexture ) );
    pParent->Connect( pBtnNewDensityVolume->GetId(),    wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnCreateFibersDensityTexture ) );
    pParent->Connect( pBtnSetAsDistanceAnchor->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnDistanceAnchorSet ) );
    pParent->Connect( pBtnDelete->GetId(),              wxEVT_COMMAND_BUTTON_CLICKED, wxTreeEventHandler(    PropertiesWindow::OnDeleteTreeItem ) );
    pParent->Connect( pBtnSelectColor->GetId(),         wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnColorRoi ) );
    pParent->Connect( m_pBtnSelectMeanFiberColor->GetId(),  wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnMeanFiberColorChange ) );
//     pParent->Connect( m_pBtnSelectConvexHullColor->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnConvexHullColorChange ) );
    //pParent->Connect( m_pbtnDisplayCrossSections->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnDisplayCrossSections ) );
    //pParent->Connect( m_pbtnDisplayDispersionTube->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnDisplayDispersionTube ) );
    pParent->Connect( m_pToggleVisibility->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnToggleShowSelectionObject ) );
    pParent->Connect( m_pToggleActivate->GetId(),   wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxTreeEventHandler(    PropertiesWindow::OnActivateTreeItem ) );
    pParent->Connect( pToggleAndNot->GetId(),       wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnToggleAndNot ) );
    pParent->Connect( m_pToggleCalculatesFibersInfo->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnDisplayFibersInfo ) );
    pParent->Connect( m_pToggleDisplayMeanFiber->GetId(),     wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnDisplayMeanFiber ) );
//     pParent->Connect( m_pToggleDisplayConvexHull->GetId(),    wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnDisplayConvexHull ) );
    pParent->Connect( m_pRadCustomColoring->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnCustomMeanFiberColoring ) );
    pParent->Connect( m_pRadNormalColoring->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnNormalMeanFiberColoring ) );
    pParent->Connect( m_pSliderMeanFiberOpacity->GetId(),  wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler( PropertiesWindow::OnMeanFiberOpacityChange ) );
//     pParent->Connect( m_pSliderConvexHullOpacity->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler( PropertiesWindow::OnConvexHullOpacityChange ) );
    pParent->Connect( m_pTxtBoxX->GetId(),  wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( PropertiesWindow::OnBoxPositionX ) );
    pParent->Connect( m_pTxtBoxY->GetId(),  wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( PropertiesWindow::OnBoxPositionY ) );
    pParent->Connect( m_pTxtBoxZ->GetId(),  wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( PropertiesWindow::OnBoxPositionZ ) );
    pParent->Connect( m_pTxtSizeX->GetId(), wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( PropertiesWindow::OnBoxSizeX ) );
    pParent->Connect( m_pTxtSizeY->GetId(), wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( PropertiesWindow::OnBoxSizeY ) );
    pParent->Connect( m_pTxtSizeZ->GetId(), wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( PropertiesWindow::OnBoxSizeZ ) );
}

//////////////////////////////////////////////////////////////////////////

void SelectionObject::updatePropertiesSizer()
{
    SceneObject::updatePropertiesSizer();

    m_pToggleVisibility->SetValue( getIsVisible() );
    m_pToggleActivate->SetValue( getIsActive() );
    m_pTxtName->SetValue( getName() );
    m_pToggleCalculatesFibersInfo->Enable( getShowFibers() );
    m_pGridFibersInfo->Enable( getShowFibers() && m_pToggleCalculatesFibersInfo->GetValue() );
    m_pToggleDisplayMeanFiber->Enable( getShowFibers() );
//     m_pToggleDisplayConvexHull->Enable( getShowFibers() );
//     setShowConvexHullOption( m_pToggleDisplayConvexHull->GetValue() );

    m_pBtnSelectMeanFiberColor->Enable( m_pToggleDisplayMeanFiber->GetValue() );
    setShowMeanFiberOption( m_pToggleDisplayMeanFiber->GetValue() );

// Because of a bug on the Windows version of this, we currently do not use this wxChoice on Windows.
// Will have to be fixed.
#ifndef __WXMSW__
    m_pCBSelectDataSet->Show( m_pToggleCalculatesFibersInfo->GetValue() );
    m_pLabelAnatomy->Show( m_pToggleCalculatesFibersInfo->GetValue() );
    if( m_pToggleCalculatesFibersInfo->GetValue() )
    {
        UpdateMeanValueTypeBox();
    }
#endif

    if( !getShowFibers() && m_meanFiberPoints.size() > 0 )
    {
        //Hide the mean fiber if fibers are invisible and the box is moved
        computeMeanFiber();
    }

    //m_pbtnDisplayDispersionTube->Enable(m_pToggleCalculatesFibersInfo->GetValue());
    //m_pbtnDisplayCrossSections->Enable(m_pToggleCalculatesFibersInfo->GetValue());

    if( m_boxMoved )
    {
        m_pTxtBoxX->ChangeValue(wxString::Format( wxT( "%.2f" ), m_center.x));
        m_pTxtBoxY->ChangeValue(wxString::Format( wxT( "%.2f" ), m_center.y));
        m_pTxtBoxZ->ChangeValue(wxString::Format( wxT( "%.2f" ), m_center.z));
        m_boxMoved = false;
    }

    if( m_boxResized )
    {
        float voxelX = DatasetManager::getInstance()->getVoxelX();
        float voxelY = DatasetManager::getInstance()->getVoxelY();
        float voxelZ = DatasetManager::getInstance()->getVoxelZ();

        m_pTxtSizeX->ChangeValue( wxString::Format( wxT( "%.2f"), m_size.x * voxelX ) );
        m_pTxtSizeY->ChangeValue( wxString::Format( wxT( "%.2f"), m_size.y * voxelY ) );
        m_pTxtSizeZ->ChangeValue( wxString::Format( wxT( "%.2f"), m_size.z * voxelZ ) );
        m_boxResized = false;
    }
}
