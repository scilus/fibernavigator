#include "SceneManager.h"

#include "MainFrame.h"
#include "MyListCtrl.h"
#include "SelectionBox.h"
#include "SelectionEllipsoid.h"
#include "SelectionObject.h"

#include "../Logger.h"
#include "../main.h"
#include "../dataset/AnatomyHelper.h"
#include "../dataset/DatasetManager.h"
#include "../dataset/Mesh.h"
#include "../dataset/ODFs.h"
#include "../dataset/Tensors.h"
#include "../dataset/Maximas.h"
#include "../gfx/ShaderHelper.h"
#include "../gfx/TheScene.h"
#include "../misc/XmlHelper.h"

#include <wx/filename.h>
#include <wx/xml/xml.h>
#include <algorithm>
#include <assert.h>
#include <map>
using std::map;

#include <stdlib.h>
#include <time.h>
#include <vector>
using std::vector;


SceneManager * SceneManager::m_pInstance = NULL;

SceneManager::SceneManager(void)
:   m_pAnatomyHelper( NULL ),
    m_pTreeView ( NULL ),
    m_pMainFrame( NULL ),
    m_pTheScene( NULL ),
    m_geometryShadersSupported( true ),
    m_useFibersGeometryShader( false ),
    m_lighting( true ),
    m_blendAlpha( false ),
    m_blendTexOnMesh( true ),
    m_showAxial( true ),
    m_showCoronal( true ),
    m_showSagittal( true ),
    m_showCrosshair( false ),
    m_showAxes( false ),
    m_sliceX( 0.0f ),
    m_sliceY( 0.0f ),
    m_sliceZ( 0.0f ),
    m_useVBO( true ),
    m_quadrant( 6 ),
    m_segmentActive( false ),
    m_segmentMethod( FLOODFILL ),
    m_animationStep( 0 ),
    m_pointMode( false ),
    m_drawVectors( false ),
    m_normalDirection( 1.0f ),
    m_zoom( 1.0f ),
    m_moveX( 0.0f ),
    m_moveY( 0.0f ),
    m_showObjects( true ),
    m_activateObjects( true ),
    m_scnLoading( false ),
    m_scnFileLoaded( false ),
    m_scnFilename( wxT( "" ) ),
    m_scnPath( wxT( "" ) ),
    m_scheduledScreenshot( false ),
    m_screenshotName( wxT( "" ) ),
    m_screenshotPath( wxT( "" ) ),
    m_clearToBlack( true ),
    m_colorMap( 0 ),
    m_filterIsoSurface( false ),
    m_pSelTree( NULL ),
    m_selBoxChanged( true ),
    m_isRulerActive( false ),
    m_rulerFullLength( 0.0 ),
    m_rulerPartialLength( 0.0 )
{
    m_pAnatomyHelper = new AnatomyHelper();
    m_pTheScene = new TheScene();
    m_pSelTree = new SelectionTree();
    Matrix4fSetIdentity( &m_transform );
}

//////////////////////////////////////////////////////////////////////////

SceneManager * SceneManager::getInstance()
{
    if( NULL == m_pInstance )
    {
        m_pInstance = new SceneManager();
    }

    return m_pInstance;
}

//////////////////////////////////////////////////////////////////////////

bool SceneManager::load(const wxString &filename)
{
    Logger::getInstance()->print( wxT( "Loading scene" ), LOGLEVEL_MESSAGE );

    if( 0 != DatasetManager::getInstance()->getDatasetCount() )
    {
        int answer = wxMessageBox( wxT("Are you sure you want to open a new scene? All objects loaded in the current scene will be deleted." ), 
            wxT( "Confirmation" ), 
            wxYES_NO | wxICON_QUESTION );

        if( answer == wxNO )
        {
            return true;
        }

        m_pMainFrame->m_pListCtrl->Clear();
        m_pMainFrame->clearCachedSceneInfo();
        
        // Clear the tree widget and the selection tree.
        m_pMainFrame->m_pTreeWidget->DeleteChildren( m_pMainFrame->m_tSelectionObjectsId );
        m_pSelTree->clear();
        
        if( 0 != DatasetManager::getInstance()->getDatasetCount() )
        {
            Logger::getInstance()->print( wxT( "Some datasets haven't been deleted when clearing the list for some reason. LOOK INTO IT!" ), LOGLEVEL_DEBUG );
        }
    }

    wxXmlDocument doc;
    if( !doc.Load( filename ) )
    {
        return false;
    }

    bool result = true;
    m_scnLoading = true;
    
    wxString scnFilename, scnPath;
    wxFileName::SplitPath( filename, NULL, &scnPath, &scnFilename, NULL );

    wxXmlNode *pRoot = doc.GetRoot();
    if( NULL != pRoot )
    {
        if( pRoot->GetName() == wxT( "theScene" ) )
        {
            // Support of the old version
            if( !loadOldVersion( pRoot, scnPath ) )
            {
                Logger::getInstance()->print( wxString::Format( wxT( "An error occured while trying to load the scene: \"%s\"" ), filename.c_str() ), LOGLEVEL_ERROR );
                result = false;
            }
        }
    }

    m_scnLoading = false;

    if( result )
    {
        m_scnFilename = scnFilename;
        m_scnPath = scnPath;
        m_scnFileLoaded = true;
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////

bool SceneManager::save( const wxString &filename )
{
    // Make sure that all anatomies have a path, or, if not the case, give the choice
    // to save them to the same directory as the scene file.
    bool unsavedAnat( false );
    vector<Anatomy *> anatomies = DatasetManager::getInstance()->getAnatomies();
    for( vector<Anatomy *>::const_iterator it = anatomies.begin(); it != anatomies.end(); ++it )
    {
        Anatomy * pAnatomy = *it;
        if( pAnatomy->getPath() == wxT("") )
        {
            unsavedAnat = true;
        }
    }
    
    if( unsavedAnat )
    {
        int answer = wxMessageBox( wxT("Some anatomy datasets are not saved on disk. If you choose to continue saving, they will be saved to the same directory as the scene file." ), 
                                  wxT( "Confirmation" ), 
                                  wxYES_NO | wxICON_QUESTION );
        
        if( answer == wxNO )
        {
            return false;
        }
    }
    
    wxXmlNode *pRoot = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "theScene" ) );
    wxXmlNode *pSlidersPosition = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "position" ) );
    wxXmlNode *pRotation = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "rotation" ) );
    wxXmlNode *pData = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "data" ) );
    wxXmlNode *pPoints = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "points" ) );
    wxXmlNode *pSelSetup = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "selection_setup" ) );

    //////////////////////////////////////////////////////////////////////////
    // ROOT
    pRoot->AddChild( pSlidersPosition );
    pRoot->AddChild( pRotation );
    pRoot->AddChild( pData );
    pRoot->AddChild( pPoints );
    pRoot->AddChild( pSelSetup );

    //////////////////////////////////////////////////////////////////////////
    // POSITION
    pSlidersPosition->AddProperty( new wxXmlProperty( wxT( "x" ), wxStrFormat( m_pMainFrame->m_pXSlider->GetValue() ) ) );
    pSlidersPosition->AddProperty( new wxXmlProperty( wxT( "y" ), wxStrFormat( m_pMainFrame->m_pYSlider->GetValue() ) ) );
    pSlidersPosition->AddProperty( new wxXmlProperty( wxT( "z" ), wxStrFormat( m_pMainFrame->m_pZSlider->GetValue() ) ) );

    //////////////////////////////////////////////////////////////////////////
    // ROTATION
    wxString rotPrecision = wxT( ".8" );
    pRotation->AddProperty( new wxXmlProperty( wxT( "rot00" ), wxStrFormat( m_transform.s.M00, rotPrecision ) ) );
    pRotation->AddProperty( new wxXmlProperty( wxT( "rot01" ), wxStrFormat( m_transform.s.M01, rotPrecision ) ) );
    pRotation->AddProperty( new wxXmlProperty( wxT( "rot02" ), wxStrFormat( m_transform.s.M02, rotPrecision ) ) );
    pRotation->AddProperty( new wxXmlProperty( wxT( "rot10" ), wxStrFormat( m_transform.s.M10, rotPrecision ) ) );
    pRotation->AddProperty( new wxXmlProperty( wxT( "rot11" ), wxStrFormat( m_transform.s.M11, rotPrecision ) ) );
    pRotation->AddProperty( new wxXmlProperty( wxT( "rot12" ), wxStrFormat( m_transform.s.M12, rotPrecision ) ) );
    pRotation->AddProperty( new wxXmlProperty( wxT( "rot20" ), wxStrFormat( m_transform.s.M20, rotPrecision ) ) );
    pRotation->AddProperty( new wxXmlProperty( wxT( "rot21" ), wxStrFormat( m_transform.s.M21, rotPrecision ) ) );
    pRotation->AddProperty( new wxXmlProperty( wxT( "rot22" ), wxStrFormat( m_transform.s.M22, rotPrecision ) ) );

    //////////////////////////////////////////////////////////////////////////
    // PREPARE DATASETS NODES
    map< DatasetIndex, wxXmlNode * > datasets;
    int count = m_pMainFrame->m_pListCtrl->GetItemCount();
    
    wxString sceneRootPath;
    wxFileName::SplitPath( filename, NULL, &sceneRootPath, NULL, NULL );
    
    for( int i = 0; i < count; ++i )
    {
        wxXmlNode *pNode = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "" ) );
        DatasetIndex index = m_pMainFrame->m_pListCtrl->GetItem( i );
        datasets[index] = pNode;

        DatasetInfo *pDS = DatasetManager::getInstance()->getDataset( index );
        if( pDS->getPath() == wxT("") )
        {
            Anatomy *pAnat = dynamic_cast< Anatomy* >(pDS);
            if( pAnat )
            {
                // Create filename and save the anatomy to it.
                srand( time( NULL ) );
                int suffix( rand() % 10000000 );

                wxString path = sceneRootPath + wxFileName::GetPathSeparator();
                path += pDS->getName() + wxT("_") + wxStrFormat( suffix ) + wxT(".nii.gz");
                
                pAnat->saveToNewFilename( path );
            }
        }
        
        pDS->save( pNode, sceneRootPath );

        wxXmlNode *pStatus = getXmlNodeByName( wxT( "status" ), pNode );
        pStatus->AddProperty( new wxXmlProperty( wxT( "position" ), wxStrFormat( i ) ) );
    }

    //////////////////////////////////////////////////////////////////////////
    // ADD DATASETS TO DATA NODE
    
    // Anatomies
    for( vector<Anatomy *>::const_iterator it = anatomies.begin(); it != anatomies.end(); ++it )
    {
        Anatomy * pAnatomy = *it;
        DatasetIndex index = DatasetManager::getInstance()->getDatasetIndex( pAnatomy );

        pData->AddChild( datasets[index] );
    }

    
    // FiberGroup
    if( DatasetManager::getInstance()->isFibersGroupLoaded() )
    {
        FibersGroup *pFibersGroup = DatasetManager::getInstance()->getFibersGroup();
        DatasetIndex index = DatasetManager::getInstance()->getDatasetIndex( pFibersGroup );

        pData->AddChild( datasets[index] );
    }

    // Fibers
    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
    {
        Fibers *pFibers = *it;
        DatasetIndex index = DatasetManager::getInstance()->getDatasetIndex( pFibers );

        pData->AddChild( datasets[index] );
    }

    // Meshes
    vector<Mesh *> meshes = DatasetManager::getInstance()->getMeshes();
    for( vector<Mesh *>::const_iterator it = meshes.begin(); it != meshes.end(); ++it )
    {
        Mesh *pMesh = *it;
        DatasetIndex index = DatasetManager::getInstance()->getDatasetIndex( pMesh );

        pData->AddChild( datasets[index] );
    }

    // ODFs
    vector<ODFs *> odfs = DatasetManager::getInstance()->getOdfs();
    for( vector<ODFs *>::const_iterator it = odfs.begin(); it != odfs.end(); ++it )
    {
        ODFs *pODFs = *it;
        DatasetIndex index = DatasetManager::getInstance()->getDatasetIndex( pODFs );

        pData->AddChild( datasets[index] );
    }

    // Maximas
    vector<Maximas *> maximas = DatasetManager::getInstance()->getMaximas();
    for( vector<Maximas *>::const_iterator it = maximas.begin(); it != maximas.end(); ++it )
    {
        Maximas *pMaximas = *it;
        DatasetIndex index = DatasetManager::getInstance()->getDatasetIndex( pMaximas );

        pData->AddChild( datasets[index] );
    }

    vector<Tensors *> tensors = DatasetManager::getInstance()->getTensors();
    for( vector<Tensors *>::const_iterator it = tensors.begin(); it != tensors.end(); ++it )
    {
        Tensors *pTensors = *it;
        DatasetIndex index = DatasetManager::getInstance()->getDatasetIndex( pTensors );

        pData->AddChild( datasets[index] );
    }

    //////////////////////////////////////////////////////////////////////////
    // SELECTION OBJECTS
    m_pSelTree->populateXMLNode( pSelSetup );

    //////////////////////////////////////////////////////////////////////////
    // SAVE DOCUMENT
    wxXmlDocument doc;
    doc.SetRoot( pRoot );

    return doc.Save( filename + ( wxT( "scn" ) != filename.AfterLast( '.' ) ? wxT( ".scn" ) : wxT( "" ) ), 2 );
}

//////////////////////////////////////////////////////////////////////////

void SceneManager::updateView( const float x, const float y, const float z, bool semaphore )
{
    m_sliceX = x;
    m_sliceY = y;
    m_sliceZ = z;

    vector<ODFs *> odfs = DatasetManager::getInstance()->getOdfs();
    for( vector<ODFs *>::iterator it = odfs.begin(); it != odfs.end(); ++it )
    {
        (*it)->refreshSlidersValues();
    }

    vector<Tensors *> tensors = DatasetManager::getInstance()->getTensors();
    for( vector<Tensors *>::iterator it = tensors.begin(); it != tensors.end(); ++it )
    {
        (*it)->refreshSlidersValues();
    }

    vector<Maximas *> maximas = DatasetManager::getInstance()->getMaximas();
    for( vector<Maximas *>::iterator it = maximas.begin(); it != maximas.end(); ++it )
    {
        (*it)->refreshSlidersValues();
    }
}

//////////////////////////////////////////////////////////////////////////

void SceneManager::changeZoom( const int z )
{
    float delta = (int)m_zoom * 0.1f;
    if( 0 <= z )
    {
        m_zoom = std::min( 50.0f, m_zoom + delta );
    }
    else
    {
        m_zoom = std::max( 1.0f, m_zoom - delta );
    }
}

//////////////////////////////////////////////////////////////////////////

void SceneManager::moveScene ( const int x, const int y )
{
    float columns = DatasetManager::getInstance()->getColumns();
    float frames  = DatasetManager::getInstance()->getFrames();
    float rows    = DatasetManager::getInstance()->getRows();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    float max = std::max( columns * voxelX, std::max( rows * voxelY, frames * voxelZ ) );
    float div = 500.0f / max;

    m_moveX -= (float)x / div;
    m_moveY += (float)y / div;
}

//////////////////////////////////////////////////////////////////////////

void SceneManager::doMatrixManipulation()
{
    Logger::getInstance()->printIfGLError( wxT( "Before SceneManager::doMatrixManipulation" ) );

    float columns = DatasetManager::getInstance()->getColumns();
    float frames  = DatasetManager::getInstance()->getFrames();
    float rows    = DatasetManager::getInstance()->getRows();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    float max = std::max( columns * voxelX, std::max( rows * voxelY, frames * voxelZ ) ) * 0.5f;
    glTranslatef( max + m_moveX, max + m_moveY, max );
    glScalef( m_zoom, m_zoom, m_zoom );
    glMultMatrixf( m_transform.M );
    glTranslatef( -columns * voxelX * 0.5f, -rows * voxelY * 0.5f, -frames * voxelZ * 0.5f );

    Logger::getInstance()->printIfGLError( wxT( "SceneManager::doMatrixManipulation" ) );
}

//////////////////////////////////////////////////////////////////////////

bool SceneManager::loadOldVersion( wxXmlNode * pRoot, const wxString &rootPath  )
{
    Logger::getInstance()->print( wxT( "Loading format 1.0" ), LOGLEVEL_DEBUG );

    unsigned int errors( 0 );

    double rotationMatrix[16] = { 1, 0, 0, 0, 
        0, 1, 0, 0, 
        0, 0, 1, 0, 
        0, 0, 0, 1 };

    long sliceX  = 0;
    long sliceY  = 0;
    long sliceZ  = 0;

    wxXmlNode *pChild = pRoot->GetChildren();
    while( NULL != pChild )
    {
        wxString nodeName = pChild->GetName();
        if( wxT( "position" ) == nodeName )
        {
            pChild->GetPropVal( wxT( "x" ), wxT( "1" ) ).ToLong( &sliceX );
            pChild->GetPropVal( wxT( "y" ), wxT( "1" ) ).ToLong( &sliceY );
            pChild->GetPropVal( wxT( "z" ), wxT( "1" ) ).ToLong( &sliceZ );
        }
        else if( wxT( "rotation" ) == nodeName )
        {
            pChild->GetPropVal( wxT( "rot00" ), wxT( "1" ) ).ToDouble( &rotationMatrix[0] );
            pChild->GetPropVal( wxT( "rot10" ), wxT( "1" ) ).ToDouble( &rotationMatrix[4] );
            pChild->GetPropVal( wxT( "rot20" ), wxT( "1" ) ).ToDouble( &rotationMatrix[8] );
            pChild->GetPropVal( wxT( "rot01" ), wxT( "1" ) ).ToDouble( &rotationMatrix[1] );
            pChild->GetPropVal( wxT( "rot11" ), wxT( "1" ) ).ToDouble( &rotationMatrix[5] );
            pChild->GetPropVal( wxT( "rot21" ), wxT( "1" ) ).ToDouble( &rotationMatrix[9] );
            pChild->GetPropVal( wxT( "rot02" ), wxT( "1" ) ).ToDouble( &rotationMatrix[2] );
            pChild->GetPropVal( wxT( "rot12" ), wxT( "1" ) ).ToDouble( &rotationMatrix[6] );
            pChild->GetPropVal( wxT( "rot22" ), wxT( "1" ) ).ToDouble( &rotationMatrix[10] );
        }
        else if( wxT( "anatomy" ) == nodeName )
        {
            long columns, rows, frames;

            pChild->GetPropVal( wxT( "columns" ), wxT( "1" ) ).ToLong( &columns );
            pChild->GetPropVal( wxT( "rows" ), wxT( "1" ) ).ToLong( &rows );
            pChild->GetPropVal( wxT( "frames" ), wxT( "1" ) ).ToLong( &frames );

            if( DatasetManager::getInstance()->isAnatomyLoaded() )
            {
                int curColumns = DatasetManager::getInstance()->getColumns();
                int curRows    = DatasetManager::getInstance()->getRows();
                int curFrames  = DatasetManager::getInstance()->getFrames();

                if( columns != curColumns || rows != curRows || frames != curFrames )
                {
                    Logger::getInstance()->print( wxT( "Dimensions of loaded files must be the same" ), LOGLEVEL_ERROR );
                    return false;
                }
            }
        }
        else if( wxT( "data" ) == nodeName )
        {
            map<long, DatasetIndex> realPositions;

            wxXmlNode *pDatasetNode = pChild->GetChildren();
            while( pDatasetNode )
            {
                bool     active         = true;
                bool     isFiberGroup   = false;
                bool     showFS         = true;
                bool     useTex         = true;
                double   alpha          = 1.00;
                double   threshold      = 0.00;
                long     position       = 0;
                wxString name;
                wxString path;

                wxXmlNode *pAttribute = pDatasetNode->GetChildren();
                while( NULL != pAttribute )
                {
                    if( wxT( "status" ) == pAttribute->GetName() )
                    {
                        isFiberGroup = pAttribute->GetPropVal( wxT( "isFiberGroup" ), wxT( "no" ) ) == wxT( "yes" );
                        name         = pAttribute->GetPropVal( wxT( "name" ), wxT( "" ) );
                        useTex       = pAttribute->GetPropVal( wxT( "useTex" ), wxT( "yes" ) ) == wxT( "yes" );
                        showFS       = pAttribute->GetPropVal( wxT( "showFS" ), wxT( "yes" ) ) == wxT( "yes" );
                        active       = pAttribute->GetPropVal( wxT( "active" ), wxT( "yes" ) ) == wxT( "yes" );
                        
                        pAttribute->GetPropVal( wxT( "alpha" ), wxT( "1.0" ) ).ToDouble( &alpha );
                        pAttribute->GetPropVal( wxT( "threshold" ), wxT( "0.0" ) ).ToDouble( &threshold );
                        pAttribute->GetPropVal( wxT( "position" ), wxT( "-1" ) ).ToLong( &position );
                    }
                    else if( wxT( "path" ) == pAttribute->GetName() )
                    {
                        path = pAttribute->GetNodeContent();
                    }

                    pAttribute = pAttribute->GetNext();
                }

                wxString extension = path.AfterLast( '.' );
                if( wxT( "gz" ) == extension )
                {
                    extension = path.BeforeLast( '.' ).AfterLast( '.' );
                }

                DatasetIndex index;
                if( isFiberGroup )
                {
                    if( !DatasetManager::getInstance()->isFibersGroupLoaded() )
                    {
                        index = DatasetManager::getInstance()->createFibersGroup();
                    }
                }
                else
                {
                    wxFileName fullDatasetPath( rootPath + wxFileName::GetPathSeparator() + path );
                    index = DatasetManager::getInstance()->load( fullDatasetPath.GetFullPath(), extension );
                }

                if( index.isOk() )
                {
                    DatasetInfo *pDataset = DatasetManager::getInstance()->getDataset( index );
                    pDataset->setName( name );
                    pDataset->setShow( active );
                    pDataset->setShowFS( showFS );
                    pDataset->setUseTex( useTex );
                    pDataset->setAlpha( alpha );
                    pDataset->setThreshold( threshold );

                    realPositions[position] = index;
                }
                else
                {
                    ++errors;
                }

                pDatasetNode = pDatasetNode->GetNext();
            }

            // Insert datasets into list
            vector<DatasetIndex> v;
            v.reserve( realPositions.size() );

            for( map<long, DatasetIndex>::iterator it = realPositions.begin(); it != realPositions.end(); ++it )
            {
                if( -1 != it->first )
                {
                    v.push_back( it->second );
                }
            }

            m_pMainFrame->m_pListCtrl->InsertItemRange( v );
        }
        else if( wxT( "selection_setup" ) == nodeName )
        {
            m_pSelTree->loadFromXMLNode( pChild );

            // Build the selection tree widget content.
            if( !m_pSelTree->isEmpty() )
            {
                m_pMainFrame->buildSelectionViewFromSelectionTree( m_pSelTree );
            }
            
        }
        else
        {
            Logger::getInstance()->print( wxString::Format( wxT( "Unsupported node name: %s" ), nodeName.c_str() ), LOGLEVEL_WARNING );
        }

        pChild = pChild->GetNext();
    }

    if( DatasetManager::getInstance()->isAnatomyLoaded() )
    {
        m_pMainFrame->updateSliders();
        m_pMainFrame->m_pXSlider->SetValue( sliceX );
        m_pMainFrame->m_pYSlider->SetValue( sliceY );
        m_pMainFrame->m_pZSlider->SetValue( sliceZ );
        updateView( sliceX, sliceY, sliceZ );
    }
    
    // Add fiber datasets to the selection tree.
    if( DatasetManager::getInstance()->isFibersLoaded() )
    {
        vector< Fibers* > curFibers = DatasetManager::getInstance()->getFibers();
        for( vector< Fibers* >::iterator fibIt( curFibers.begin() ); fibIt != curFibers.end(); ++fibIt )
        {
            m_pSelTree->addFiberDataset( (*fibIt)->getDatasetIndex(), (*fibIt)->getLineCount() );
        }
    }

//     m_transform.s.M00 = rotationMatrix[0];
//     m_transform.s.M10 = rotationMatrix[4];
//     m_transform.s.M20 = rotationMatrix[8];
//     m_transform.s.M01 = rotationMatrix[1];
//     m_transform.s.M11 = rotationMatrix[5];
//     m_transform.s.M21 = rotationMatrix[9];
//     m_transform.s.M02 = rotationMatrix[2];
//     m_transform.s.M12 = rotationMatrix[6];
//     m_transform.s.M22 = rotationMatrix[10];
//     m_pMainFrame->m_pMainGL->setRotation();

    return 0 == errors;
}


//////////////////////////////////////////////////////////////////////////

SceneManager::~SceneManager(void)
{
    Logger::getInstance()->print( wxT( "Executing SceneManager destructor" ), LOGLEVEL_DEBUG );

    delete m_pAnatomyHelper;
    m_pAnatomyHelper = NULL;
    
    delete m_pSelTree;
    m_pSelTree = NULL;
    
    delete m_pTheScene;
    m_pTheScene = NULL;
    
    m_pInstance = NULL;
    Logger::getInstance()->print( wxT( "SceneManager destructor done" ), LOGLEVEL_DEBUG );
}
