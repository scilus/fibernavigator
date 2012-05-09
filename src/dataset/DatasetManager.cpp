#include "DatasetManager.h"

#include "Anatomy.h"
#include "Fibers.h"
#include "Mesh.h"
#include "ODFs.h"
#include "Tensors.h"

#include "../Logger.h"
#include "../gui/SceneManager.h"
#include "../misc/nifti/nifti1_io.h"

#include <wx/filename.h>

#include <map>
using std::map;

#include <vector>
using std::vector;

namespace
{
    const float THRESHOLD = 0.0f; 
    const float ALPHA     = 1.0f;
    const bool  SHOW      = true;
    const bool  SHOW_FS   = true; 
    const bool  USE_TEX   = true;
}

DatasetManager * DatasetManager::m_pInstance = NULL;

DatasetManager::DatasetManager(void)
:   m_nextIndex( 1 ),
    m_niftiTransform( 4, 4 ),
    m_countFibers( 0 )
{
}

//////////////////////////////////////////////////////////////////////////

DatasetManager * DatasetManager::getInstance()
{
    if( NULL == m_pInstance )
    {
        m_pInstance = new DatasetManager();
    }

    return m_pInstance;
}

//////////////////////////////////////////////////////////////////////////

std::vector<Anatomy *> DatasetManager::getAnatomies() const
{
    vector<Anatomy *> v;
    for( map<DatasetIndex, Anatomy *>::const_iterator it = m_anatomies.begin(); it != m_anatomies.end(); ++it )
    {
        v.push_back( it->second );
    }
    return v;
}

//////////////////////////////////////////////////////////////////////////

DatasetInfo * DatasetManager::getDataset( DatasetIndex index ) const
{
    map<DatasetIndex, DatasetInfo *>::const_iterator it = m_datasets.find( index );
    if( it != m_datasets.end() )
    {
        return it->second;
    }

    return NULL;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::getDatasetIndex( DatasetInfo * pDatasetInfo ) const
{
    map<DatasetInfo *, DatasetIndex>::const_iterator it = m_reverseDatasets.find( pDatasetInfo );
    if( it != m_reverseDatasets.end() )
    {
        return it->second;
    }

    return BAD_INDEX;
}

//////////////////////////////////////////////////////////////////////////

std::vector<Fibers *> DatasetManager::getFibers() const
{
    vector<Fibers *> v;
    for( map<DatasetIndex, Fibers *>::const_iterator it = m_fibers.begin(); it != m_fibers.end(); ++it )
    {
        v.push_back( it->second );
    }
    return v;
}

//////////////////////////////////////////////////////////////////////////

FibersGroup * DatasetManager::getFibersGroup() const
{
    map<DatasetIndex, FibersGroup *>::const_iterator it = m_fibersGroup.begin();
    if( it != m_fibersGroup.end() )
    {
        return it->second;
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////

std::vector<Mesh *> DatasetManager::getMeshes() const
{
    vector<Mesh *> v;
    for( map<DatasetIndex, Mesh *>::const_iterator it = m_meshes.begin(); it != m_meshes.end(); ++it )
    {
        v.push_back( it->second );
    }
    return v;
}

//////////////////////////////////////////////////////////////////////////

std::vector<ODFs *> DatasetManager::getOdfs() const
{
    vector<ODFs *> v;
    for( map<DatasetIndex, ODFs *>::const_iterator it = m_odfs.begin(); it != m_odfs.end(); ++it )
    {
        v.push_back( it->second );
    }
    return v;
}

//////////////////////////////////////////////////////////////////////////

Fibers * DatasetManager::getSelectedFibers( DatasetIndex index ) const
{
    if( index.isOk() )
    {
        map<DatasetIndex, Fibers *>::const_iterator it = m_fibers.find( index );
        if( it != m_fibers.end() )
        {
            return it->second;
        }
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////

vector<Tensors *> DatasetManager::getTensors() const
{
    vector<Tensors *> v;
    for( map<DatasetIndex, Tensors *>::const_iterator it = m_tensors.begin(); it != m_tensors.end(); ++it )
    {
        v.push_back( it->second );
    }
    return v;
}


//////////////////////////////////////////////////////////////////////////

int DatasetManager::getColumns() const
{
    if( !m_anatomies.empty() )
    {
        map<DatasetIndex, Anatomy *>::const_iterator it = m_anatomies.begin();
        return it->second->getColumns();
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::getFrames() const
{
    if( !m_anatomies.empty() )
    {
        map<DatasetIndex, Anatomy *>::const_iterator it = m_anatomies.begin();
        return it->second->getFrames();
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::getRows() const
{
    if( !m_anatomies.empty() )
    {
        map<DatasetIndex, Anatomy *>::const_iterator it = m_anatomies.begin();
        return it->second->getRows();
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////

float DatasetManager::getVoxelX() const
{
    if( !m_anatomies.empty() )
    {
        map<DatasetIndex, Anatomy *>::const_iterator it = m_anatomies.begin();
        return it->second->getVoxelSizeX();
    }
    return 0.0f;
}

//////////////////////////////////////////////////////////////////////////

float DatasetManager::getVoxelY() const
{
    if( !m_anatomies.empty() )
    {
        map<DatasetIndex, Anatomy *>::const_iterator it = m_anatomies.begin();
        return it->second->getVoxelSizeY();
    }
    return 0.0f;
}

//////////////////////////////////////////////////////////////////////////

float DatasetManager::getVoxelZ() const
{
    if( !m_anatomies.empty() )
    {
        map<DatasetIndex, Anatomy *>::const_iterator it = m_anatomies.begin();
        return it->second->getVoxelSizeZ();
    }
    return 0.0f;
}

//////////////////////////////////////////////////////////////////////////

void DatasetManager::clear()
{
    Logger::getInstance()->print( wxT( "Clearing all datasets..." ), LOGLEVEL_MESSAGE );
    vector<DatasetIndex> indexes;
    indexes.reserve( m_datasets.size() );

    for( map<DatasetIndex, DatasetInfo *>::const_iterator it = m_datasets.begin(); it != m_datasets.end(); ++it )
    {
        indexes.push_back( it->first );
    }

    for( vector<DatasetIndex>::const_iterator it = indexes.begin(); it != indexes.end(); ++it )
    {
        remove( *it );
    }
    Logger::getInstance()->print( wxT( "Datasets cleared." ), LOGLEVEL_MESSAGE );
}


//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::load( const wxString &filename, const wxString &extension )
{
    DatasetIndex result( BAD_INDEX );

    if( !wxFileName::FileExists( filename ) )
    {
        Logger::getInstance()->print( wxString::Format( wxT( "File \"%s\" does not exist!" ), filename.c_str() ), LOGLEVEL_ERROR );
        return result;
    }

    if( wxT( "nii" ) == extension )
    {
        nifti_image *pHeader = nifti_image_read( filename.char_str(), 0 );
        nifti_image *pBody   = nifti_image_read( filename.char_str(), 1 );

        if( NULL == pHeader || NULL == pBody )
        {
            Logger::getInstance()->print( wxT( "nifti file corrupt, cannot create nifti image from header" ), LOGLEVEL_ERROR );
        }
        else if( 16 == pHeader->datatype && 4 == pHeader->ndim && 6 == pHeader->dim[4] )
        {
            if ( m_anatomies.empty() )
            {
                Logger::getInstance()->print( wxT( "No anatomy file loaded" ), LOGLEVEL_ERROR );
            }
            else if ( !m_tensors.empty() )
            {
                Logger::getInstance()->print( wxT( "Tensors already loaded" ), LOGLEVEL_ERROR );
            }
            else
            {
                result = loadTensors( filename, pHeader, pBody );
            }
        }
        else if( 16 == pHeader->datatype && 4 == pHeader->ndim && ( 
            0 == pHeader->dim[4] || 15 == pHeader->dim[4] || 28 == pHeader->dim[4] || 45 == pHeader->dim[4] || 
            66 == pHeader->dim[4] || 91 == pHeader->dim[4] || 120 == pHeader->dim[4] || 153 == pHeader->dim[4] ) )
        {
            if ( m_anatomies.empty() )
            {
                Logger::getInstance()->print( wxT( "No anatomy file loaded" ), LOGLEVEL_ERROR );
            }
            else
            {
                result = loadODF( filename, pHeader, pBody );
            }
        }
        else
        {
            result = loadAnatomy( filename, pHeader, pBody );
        }

        nifti_image_free( pHeader );
        nifti_image_free( pBody );
    }
    else if( wxT("mesh") == extension || wxT( "surf" ) == extension || wxT( "dip" ) == extension )
    {
        if( !isAnatomyLoaded() )
        {
            Logger::getInstance()->print( wxT( "No anatomy file loaded" ), LOGLEVEL_ERROR );
        }
        else
        {
            result = loadMesh( filename, extension );
        }
    }
    else if( wxT( "fib" ) == extension || wxT( "trk" ) == extension || wxT( "bundlesdata" ) == extension || wxT( "Bfloat" ) == extension || wxT( "tck" ) == extension )
    {
        if( !isAnatomyLoaded() )
        {
            Logger::getInstance()->print( wxT( "No anatomy file loaded" ), LOGLEVEL_ERROR );
        }
        else
        {
            result = loadFibers( filename );
        }
    }
    else
    {
        Logger::getInstance()->print( wxString::Format( wxT( "Unsupported file format \"%s\"" ), extension.c_str() ), LOGLEVEL_ERROR );
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////

void DatasetManager::remove( const DatasetIndex index )
{
    map<DatasetIndex, DatasetInfo *>::iterator it = m_datasets.find( index );
    DatasetInfo *pDatasetInfo = it->second;
    
    switch( pDatasetInfo->getType() )
    {
    case HEAD_BYTE:
    case HEAD_SHORT:
    case OVERLAY:
    case RGB:
        Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), static_cast<unsigned int>( index ), wxT( "Anatomy" ) ), LOGLEVEL_DEBUG );
        m_anatomies.erase( m_anatomies.find( index ) );
        break;
//     case TENSOR_FIELD:
//         break;
    case MESH:
        Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), static_cast<unsigned int>( index ), wxT( "Mesh" ) ), LOGLEVEL_DEBUG );
        m_meshes.erase( m_meshes.find( index ) );
        break;
//     case VECTORS:
//         break;
    case TENSORS:
        Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), static_cast<unsigned int>( index ), wxT( "Tensors" ) ), LOGLEVEL_DEBUG );
        m_tensors.erase( m_tensors.find( index ) );
        break;
    case ODFS:
        Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), static_cast<unsigned int>( index ), wxT( "ODFs" ) ), LOGLEVEL_DEBUG );
        m_odfs.erase( m_odfs.find( index ) );
        break;
    case FIBERS:
        Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), static_cast<unsigned int>( index ), wxT( "Fibers" ) ), LOGLEVEL_DEBUG );
        m_fibers.erase( m_fibers.find( index ) );
        break;
//     case SURFACE:
//         Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), index, wxT( "Surface" ) ), LOGLEVEL_DEBUG );
//         m_surfaces.erase( m_surfaces.find( index ) );
//         break;
//     case ISO_SURFACE:
//         break;
    case FIBERSGROUP:
        Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), static_cast<unsigned int>( index ), wxT( "FibersGroup" ) ), LOGLEVEL_DEBUG );
        m_fibersGroup.erase( m_fibersGroup.find( index ) );
        break;
    default:
        Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), static_cast<unsigned int>( index ), wxT( "DatasetInfo" ) ), LOGLEVEL_DEBUG );
        break;
    }

    m_datasets.erase( index );
    m_reverseDatasets.erase( pDatasetInfo );
    delete pDatasetInfo;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( Anatomy * pAnatomy )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]  = pAnatomy;
    m_anatomies[index] = pAnatomy;
    m_reverseDatasets[pAnatomy] = index;

    return index;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( CIsoSurface * pCIsoSurface )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]  = pCIsoSurface;
    m_reverseDatasets[pCIsoSurface] = index;
    // Verify if a new map is needed for this type

    return index;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( Fibers * pFibers )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]  = pFibers;
    m_fibers[index]    = pFibers;
    m_reverseDatasets[pFibers] = index;

    return index;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( FibersGroup * pFibersGroup )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]    = pFibersGroup;
    m_fibersGroup[index] = pFibersGroup;
    m_reverseDatasets[pFibersGroup] = index;

    return index;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( Mesh * pMesh )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]  = pMesh;
    m_meshes[index]    = pMesh;
    m_reverseDatasets[pMesh] = index;

    return index;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( ODFs * pOdfs )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]  = pOdfs;
    m_odfs[index]      = pOdfs;
    m_reverseDatasets[pOdfs] = index;

    return index;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( Tensors * pTensors )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]  = (DatasetInfo *)pTensors;
    m_tensors[index]   = pTensors;
    m_reverseDatasets[pTensors] = index;

    return index;
}

//////////////////////////////////////////////////////////////////////////

// Loads an anatomy. Extension supported: .nii and .nii.gz
DatasetIndex DatasetManager::loadAnatomy( const wxString &filename, nifti_image *pHeader, nifti_image *pBody )
{
    Logger::getInstance()->print( wxT( "Loading anatomy" ), LOGLEVEL_MESSAGE );
    Anatomy *pAnatomy = new Anatomy( filename );
    if( pAnatomy->load( pHeader, pBody ) )
    {
        Logger::getInstance()->print( wxT( "Assigning attributes" ), LOGLEVEL_DEBUG );
        pAnatomy->setThreshold( THRESHOLD );
        pAnatomy->setAlpha( ALPHA );
        pAnatomy->setShow( SHOW );
        pAnatomy->setShowFS( SHOW_FS );
        pAnatomy->setUseTex( USE_TEX );

        DatasetIndex index = insert( pAnatomy );

        SelectionObjList objs = SceneManager::getInstance()->getSelectionObjects();
        for( SelectionObjList::iterator masterIt = objs.begin(); masterIt != objs.end(); ++masterIt )
        {
            for( vector< SelectionObject *>::iterator childIt = masterIt->begin(); childIt != masterIt->end(); ++childIt )
            {
                (*childIt)->update();
            }
        }

        return index;
    }

    delete pAnatomy;
    return BAD_INDEX;
}

//////////////////////////////////////////////////////////////////////////

// Loads a fiber set. Extension supported: .fib, .bundlesdata, .trk and .tck
DatasetIndex DatasetManager::loadFibers( const wxString &filename )
{
    Fibers* l_fibers = new Fibers();

    if( l_fibers->load( filename ) )
    {
        SelectionObjList selectionObjects = SceneManager::getInstance()->getSelectionObjects();
        for( unsigned int i( 0 ); i < selectionObjects.size(); ++i )
        {
            for( unsigned int j( 0 ); j < selectionObjects[i].size(); ++j )
            {
                selectionObjects[i][j]->m_inBox.assign( m_countFibers, false );
                selectionObjects[i][j]->setIsDirty( true );
            }
        }

        l_fibers->setThreshold( THRESHOLD );
        l_fibers->setShow     ( SHOW );
        l_fibers->setShowFS   ( SHOW_FS );
        l_fibers->setUseTex   ( USE_TEX );

        DatasetIndex index = insert( l_fibers );

        l_fibers->updateLinesShown();

        SceneManager::getInstance()->setSelBoxChanged( true );

        return index;
    }

    delete l_fibers;
    return BAD_INDEX;
}

DatasetIndex DatasetManager::createFibers( std::vector<std::vector<Vector> >* RTT )
{
    Fibers* l_fibers = new Fibers();

    l_fibers->convertFromRTT( RTT );
   
    SelectionObjList selectionObjects = SceneManager::getInstance()->getSelectionObjects();
    for( unsigned int i( 0 ); i < selectionObjects.size(); ++i )
    {
        for( unsigned int j( 0 ); j < selectionObjects[i].size(); ++j )
        {
            selectionObjects[i][j]->m_inBox.assign( m_countFibers, false );
            selectionObjects[i][j]->setIsDirty( true );
        }
    }

    l_fibers->setThreshold( THRESHOLD );
    l_fibers->setShow     ( SHOW );
    l_fibers->setShowFS   ( SHOW_FS );
    l_fibers->setUseTex   ( USE_TEX );

    DatasetIndex index = insert( l_fibers );

    l_fibers->updateLinesShown();

    SceneManager::getInstance()->setSelBoxChanged( true );

    delete l_fibers;
	return index;
}

//////////////////////////////////////////////////////////////////////////

// Loads a mesh. Extension supported: .mesh, .surf and .dip
DatasetIndex DatasetManager::loadMesh( const wxString &filename, const wxString &extension )
{
    Mesh *pMesh = new Mesh( filename );

    bool result;

    if( wxT( "mesh" ) == extension )
    {
        result = pMesh->loadMesh( filename );
    }
    else if( wxT( "surf" ) )
    {
        result = pMesh->loadSurf( filename );
    }
    else if( wxT( "dip" ) )
    {
        result = pMesh->loadDip( filename );
    }

    if( result )
    {
        Logger::getInstance()->print( wxT( "Assigning attributes" ), LOGLEVEL_DEBUG );
        pMesh->setThreshold( THRESHOLD );
        pMesh->setShow     ( SHOW );
        pMesh->setShowFS   ( SHOW_FS );
        pMesh->setUseTex   ( USE_TEX );

        DatasetIndex index = insert( pMesh );

        return index;
    }

    delete pMesh;
    return BAD_INDEX;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::loadODF( const wxString &filename, nifti_image *pHeader, nifti_image *pBody )
{
    Logger::getInstance()->print( wxT( "Loading ODF" ), LOGLEVEL_MESSAGE );

    ODFs *pOdfs = new ODFs( filename );
    if( pOdfs->load( pHeader, pBody ) )
    {
        Logger::getInstance()->print( wxT( "Assigning attributes" ), LOGLEVEL_DEBUG );
        pOdfs->setThreshold( THRESHOLD );
        pOdfs->setAlpha( ALPHA );
        pOdfs->setShow( SHOW );
        pOdfs->setShowFS( SHOW_FS );
        pOdfs->setUseTex( USE_TEX );

        DatasetIndex index = insert( pOdfs );

        return index;
    }

    delete pOdfs;
    return BAD_INDEX;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::loadTensors( const wxString &filename, nifti_image *pHeader, nifti_image *pBody )
{
    Logger::getInstance()->print( wxT( "Loading Tensors" ), LOGLEVEL_MESSAGE );

    Tensors *pTensors = new Tensors( filename );
    if( pTensors->load( pHeader, pBody ) )
    {
        Logger::getInstance()->print( wxT( "Assigning attributes" ), LOGLEVEL_DEBUG );
        pTensors->setThreshold( THRESHOLD );
        pTensors->setAlpha( ALPHA );
        pTensors->setShow( SHOW );
        pTensors->setShowFS( SHOW_FS );
        pTensors->setUseTex( USE_TEX );

        DatasetIndex index = insert( pTensors );

        return index;
    }

    delete pTensors;
    return BAD_INDEX;
}

//////////////////////////////////////////////////////////////////////////

DatasetManager::~DatasetManager(void)
{
    Logger::getInstance()->print( wxT( "DatasetManager destruction starting..." ), LOGLEVEL_DEBUG );
    for( map<DatasetIndex, DatasetInfo *>::iterator it = m_datasets.begin(); it != m_datasets.end(); ++it )
    {
        delete it->second;
    }
    m_pInstance = NULL;
    Logger::getInstance()->print( wxT( "DatasetManager destruction done." ), LOGLEVEL_DEBUG );
}
