/*
 * DatasetHelper.cpp
 *
 *  Created on: 27.07.2008
 *      Author: ralph
 */
#include "DatasetHelper.h"
#include "DatasetManager.h"
#include "../gui/MainFrame.h"

#include <memory>
#include <exception>
#include <stdexcept>
#include <new>

#include "Anatomy.h"
#include "ODFs.h"
#include "Fibers.h"
#include "FibersGroup.h"

#include "KdTree.h"
#include "../main.h"
#include "Mesh.h"
#include "SplinePoint.h"
#include "Surface.h"
#include "Tensors.h"
#include "../gui/MyListCtrl.h"
#include "../gui/SceneManager.h"
#include "../gui/SelectionBox.h"
#include "../gui/SelectionEllipsoid.h"

#include "../misc/IsoSurface/CIsoSurface.h"
#include "Surface.h"
#include "../misc/nifti/nifti1_io.h"

#include "../Logger.h"

#include <algorithm>

///////////////////////////////////////////////////////////////////////////
// Constructor
DatasetHelper::DatasetHelper() :
    m_countFibers( 0    ),
    m_scnFileLoaded      ( false ),
    m_surfaceIsDirty     ( true  ),
    m_scheduledReloadShaders( true  ),
    m_scheduledScreenshot   ( false ),
    m_isRulerToolActive( false ),
    m_rulerFullLength(0),
    m_rulerPartialLength(0),
    m_clearToBlack   ( false ),
    m_filterIsoSurf  ( false ),
    m_colorMap( 0 ),
    m_showColorMapLegend( false ),
    m_boxLockIsOn  ( false ),
    m_threadsActive( 0 ),
    m_texAssigned  ( false ),
    m_selBoxChanged( true ),
    m_geforceLevel( 6 ),
    m_scenePath     ( _T( "" ) ),
    m_scnFileName   ( _T( "" ) ),
    m_screenshotPath( _T( "" ) ),
    m_screenshotName( _T( "" ) ),
    m_boxAtCrosshair    ( 0 ),
    m_lastSelectedPoint ( NULL ),
    m_lastSelectedObject( NULL )
{
}

DatasetHelper::~DatasetHelper()
{
    Logger::getInstance()->print( wxT( "Execute DatasetHelper destructor" ), LOGLEVEL_DEBUG );

    Logger::getInstance()->print( wxT( "DatasetHelper destructor done" ), LOGLEVEL_DEBUG );
}

void DatasetHelper::treeFinished()
{
    m_threadsActive--;

    if ( m_threadsActive > 0 )
        return;

    Logger::getInstance()->print( wxT( "Tree finished" ), LOGLEVEL_MESSAGE );
    SceneManager::getInstance()->updateAllSelectionObjects();
    m_selBoxChanged = true;
    MyApp::frame->refreshAllGLWidgets();
}

void DatasetHelper::deleteAllPoints()
{
    std::vector< std::vector< SplinePoint* > > l_points;

    wxTreeItemId l_id, l_childId;
    wxTreeItemIdValue l_cookie = 0;

    l_id = MyApp::frame->m_pTreeWidget->GetFirstChild( MyApp::frame->m_tPointId, l_cookie );

    while( l_id.IsOk() )
    {
        std::vector< SplinePoint* > l_b;
        l_b.push_back( (SplinePoint*)( MyApp::frame->m_pTreeWidget->GetItemData( l_id ) ) );
        wxTreeItemIdValue childcookie = 0;
        l_childId = MyApp::frame->m_pTreeWidget->GetFirstChild( l_id, childcookie );

        while( l_childId.IsOk() )
        {
            l_b.push_back( (SplinePoint*)( MyApp::frame->m_pTreeWidget->GetItemData( l_childId ) ) );
            l_childId = MyApp::frame->m_pTreeWidget->GetNextChild( l_id, childcookie );
        }

        l_id = MyApp::frame->m_pTreeWidget->GetNextChild( MyApp::frame->m_tPointId, l_cookie );
        l_points.push_back( l_b );
    }

    for( unsigned int i = 0; i < l_points.size(); ++i )
    {
        for( unsigned int j = 0; j < l_points[i].size(); ++j )
        {
            MyApp::frame->m_pTreeWidget->Delete(l_points[i][j]->GetId());      
        }
    }
}

Vector DatasetHelper::mapMouse2World( const int i_x, const int i_y,GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16])
{
    glPushMatrix();
    SceneManager::getInstance()->doMatrixManipulation();
 
    GLfloat l_winX, l_winY;

    l_winX = (float) i_x;
    l_winY = (float) i_viewport[3] - (float) i_y;

    GLdouble l_posX, l_posY, l_posZ;
    gluUnProject( l_winX, l_winY, 0, i_modelview, i_projection, i_viewport, &l_posX, &l_posY, &l_posZ );
    glPopMatrix();

    Vector l_vector( l_posX, l_posY, l_posZ );
    return l_vector;
}

Vector DatasetHelper::mapMouse2WorldBack( const int i_x, const int i_y,GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] )
{   
    GLfloat l_winX, l_winY;

    l_winX = (float) i_x;
    l_winY = (float) i_viewport[3] - (float) i_y;

    GLdouble l_posX, l_posY, l_posZ;
    gluUnProject( l_winX, l_winY, 1, i_modelview, i_projection, i_viewport, &l_posX, &l_posY, &l_posZ );

    Vector l_vector( l_posX, l_posY, l_posZ );
    return l_vector;
}

std::vector< float >* DatasetHelper::getVectorDataset()
{
    if( !DatasetManager::getInstance()->isVectorsLoaded() )
        return NULL;

    for( int i = 0; i < MyApp::frame->m_pListCtrl2->GetItemCount(); ++i )
    {
        DatasetInfo* l_datasetInfo = DatasetManager::getInstance()->getDataset( MyApp::frame->m_pListCtrl2->GetItem( i ) );
        if( l_datasetInfo->getType() == VECTORS )
        {
            Anatomy* l_anatomy = (Anatomy*)l_datasetInfo;
            return l_anatomy->getFloatDataset();
        }
    }

    return NULL;
}

TensorField* DatasetHelper::getTensorField()
{
    if( !DatasetManager::getInstance()->isTensorsFieldLoaded() )
        return NULL;

    for( unsigned int i( 0 ); i < static_cast<unsigned int>( MyApp::frame->m_pListCtrl2->GetItemCount() ); ++i )
    {
        DatasetInfo* l_datasetInfo = DatasetManager::getInstance()->getDataset( MyApp::frame->m_pListCtrl2->GetItem( i ) );
        if( l_datasetInfo->getType() == TENSOR_FIELD || l_datasetInfo->getType() == VECTORS )
        {
            Anatomy* l_anatomy = (Anatomy*) l_datasetInfo;
            return l_anatomy->getTensorField();
        }
    }

    return NULL;
}
