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

bool DatasetHelper::getSelectedFiberDataset( Fibers* &io_f )
{
    io_f = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->getCurrentListItem() );
    return NULL != io_f;
}

bool DatasetHelper::getFibersGroupDataset( FibersGroup* &io_fg )
{
    io_fg = DatasetManager::getInstance()->getFibersGroup();
    return NULL != io_fg;
}

bool DatasetHelper::getSurfaceDataset( Surface *&io_s )
{
    io_s = DatasetManager::getInstance()->getSurface();
    return NULL != io_s;
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

bool DatasetHelper::getTextureDataset( vector< DatasetInfo* > &o_types )
{
    vector<Anatomy *> v = DatasetManager::getInstance()->getAnatomies();
    o_types = vector<DatasetInfo *>(v.begin(), v.end());
    return true;
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


void DatasetHelper::doLicMovie( int i_mode )
{
    wxString l_caption          = wxT( "Choose a file" );
    wxString l_wildcard         = wxT( "PPM files (*.ppm)|*.ppm|*.*|*.*" );
    wxString defaultDir         = wxEmptyString;
    wxString l_defaultFileName  = wxEmptyString;
    wxFileDialog l_dialog( MyApp::frame, l_caption, defaultDir, l_defaultFileName, l_wildcard, wxSAVE );
    wxString l_tmpFileName      = _T( "" );

    l_dialog.SetFilterIndex( 0 );
    l_dialog.SetDirectory( m_screenshotPath );
    l_dialog.SetFilename( _T( "image" ) );

    if( l_dialog.ShowModal() == wxID_OK )
    {
        m_screenshotPath = l_dialog.GetDirectory();
        l_tmpFileName    = l_dialog.GetPath();
    }
    else
        return;

    int l_gf = m_geforceLevel;
    m_geforceLevel = 99;

    switch( i_mode )
    {
        case 0:
        {
            float columns = DatasetManager::getInstance()->getColumns();
            for( int i( 0 ); i < columns; ++i )
            {
                m_screenshotName = l_tmpFileName + wxString::Format( wxT( "%03d" ), i ) + _T( ".ppm" );
                createLicSliceSag( i );
            }
            break;
        }

        case 1:
        {
            float rows = DatasetManager::getInstance()->getRows();
            for( int i = 0; i < rows; ++i )
            {
                m_screenshotName = l_tmpFileName + wxString::Format( wxT( "%03d" ), i ) + _T( ".ppm" );
                createLicSliceCor( i );
            }
            break;
        }
        case 2:
        {
            float frames = DatasetManager::getInstance()->getFrames();
            for ( int i = 0; i < frames; ++i )
            {
                m_screenshotName = l_tmpFileName + wxString::Format( wxT( "%03d" ), i ) + _T( ".ppm" );
                createLicSliceAxi( i );
            }
            break;
        }
        default:
            break;
    }

    m_geforceLevel = l_gf;
}

void DatasetHelper::licMovieHelper()
{
    unsigned int index = DatasetManager::getInstance()->createSurface();
    Surface* l_surface = (Surface *)DatasetManager::getInstance()->getDataset( index );
    l_surface->execute();

#ifdef __WXMAC__
    // insert at zero is a well-known bug on OSX, so we append there...
    // http://trac.wxwidgets.org/ticket/4492
    //long l_id = MyApp::frame->m_pListCtrl->GetItemCount();
    long l_id = MyApp::frame->m_pListCtrl2->GetItemCount();
#else
    long l_id = 0;
#endif

    // TODO: Verify this. Added to list, rendered, then removed from list. Is this necessary?
    MyApp::frame->m_pListCtrl2->InsertItem( index );

    l_surface->activateLIC();

    m_scheduledScreenshot = true;
    MyApp::frame->m_pMainGL->render();
    MyApp::frame->m_pMainGL->render();

    MyApp::frame->m_pListCtrl2->DeleteItem( l_id );
}

void DatasetHelper::createLicSliceSag( int i_slize )
{
    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    int l_xs = (int)( i_slize * voxelX );

    //delete all existing points
    MyApp::frame->m_pTreeWidget->DeleteChildren( MyApp::frame->m_tPointId );

    float rows   = DatasetManager::getInstance()->getRows();
    float frames = DatasetManager::getInstance()->getFrames();

    for( int i = 0; i < 11; ++i )
    {
        for( int j = 0; j < 11; ++j )
        {
            int yy = (int)( rows   / 10 * voxelY * i );
            int zz = (int)( frames / 10 * voxelZ * j );

            // create the l_point
            SplinePoint* l_point = new SplinePoint( l_xs, yy, zz, this );

            if ( i == 0 || i == 10 || j == 0 || j == 10 )
            {
                wxTreeItemId l_treeId = MyApp::frame->m_pTreeWidget->AppendItem( MyApp::frame->m_tPointId, wxT( "boundary point" ), -1, -1, l_point );
                l_point->setTreeId( l_treeId );
                l_point->setIsBoundary( true );
            }
        }
    }
    licMovieHelper();
}

void DatasetHelper::createLicSliceCor( int i_slize )
{
    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    int l_ys = (int)( i_slize * voxelY );

    //delete all existing points
    MyApp::frame->m_pTreeWidget->DeleteChildren( MyApp::frame->m_tPointId );

    float columns = DatasetManager::getInstance()->getColumns();
    float frames  = DatasetManager::getInstance()->getFrames();

    for( int i = 0; i < 11; ++i )
    {
        for( int j = 0; j < 11; ++j )
        {
            int xx = (int) ( columns / 10 * voxelX * i );
            int zz = (int) ( frames  / 10 * voxelZ * j );

            // create the point
            SplinePoint* l_point = new SplinePoint( xx, l_ys, zz, this );

            if( i == 0 || i == 10 || j == 0 || j == 10 )
            {
                wxTreeItemId l_treeId = MyApp::frame->m_pTreeWidget->AppendItem( MyApp::frame->m_tPointId, wxT( "boundary point" ), -1, -1, l_point );
                l_point->setTreeId( l_treeId );
                l_point->setIsBoundary( true );
            }
        }
    }

    licMovieHelper();
}

void DatasetHelper::createLicSliceAxi( int i_slize )
{
    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    int l_zs = (int)( i_slize * voxelZ );

    //delete all existing points
    MyApp::frame->m_pTreeWidget->DeleteChildren( MyApp::frame->m_tPointId );

    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();

    for( int i = 0; i < 11; ++i )
    {
        for( int j = 0; j < 11; ++j )
        {
            int xx = (int)( columns / 10 * voxelX * i );
            int yy = (int)( rows    / 10 * voxelY * j );

            // create the l_point
            SplinePoint *l_point = new SplinePoint( xx, yy, l_zs, this );

            if( i == 0 || i == 10 || j == 0 || j == 10 )
            {
                wxTreeItemId l_treeId = MyApp::frame->m_pTreeWidget->AppendItem( MyApp::frame->m_tPointId, wxT( "boundary point" ), -1, -1, l_point );
                l_point->setTreeId( l_treeId );
                l_point->setIsBoundary( true );
            }
        }
    }

    licMovieHelper();
}
