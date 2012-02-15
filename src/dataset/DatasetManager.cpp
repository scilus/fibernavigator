#include "DatasetManager.h"

#include "Anatomy.h"
#include "DatasetHelper.h"
#include "Fibers.h"
#include "Mesh.h"
#include "ODFs.h"
#include "Tensors.h"

#include "../Logger.h"
#include "../misc/nifti/nifti1_io.h"

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
: m_nextIndex( 0 )
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

DatasetInfo * DatasetManager::getDataset( unsigned int index )
{
    if( m_datasets.find( index ) != m_datasets.end() )
    {
        return m_datasets[index];
    }

    return NULL;
}

//////////////////////////////////////////////////////////////////////////

std::vector<Fibers *> DatasetManager::getFibers()
{
    vector<Fibers *> v;
    for( map<unsigned int, Fibers *>::const_iterator it = m_fibers.begin(); it != m_fibers.end(); ++it )
    {
        v.push_back( it->second );
    }
    return v;
}

//////////////////////////////////////////////////////////////////////////

std::vector<ODFs *> DatasetManager::getOdfs()
{
    vector<ODFs *> v;
    for( map<unsigned int, ODFs *>::const_iterator it = m_odfs.begin(); it != m_odfs.end(); ++it )
    {
        v.push_back( it->second );
    }
    return v;
}

//////////////////////////////////////////////////////////////////////////

vector<Tensors *> DatasetManager::getTensors()
{
    vector<Tensors *> v;
    for( map<unsigned int, Tensors *>::const_iterator it = m_tensors.begin(); it != m_tensors.end(); ++it )
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
        map<unsigned int, Anatomy *>::const_iterator it = m_anatomies.begin();
        return it->second->getColumns();
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::getFrames() const
{
    if( !m_anatomies.empty() )
    {
        map<unsigned int, Anatomy *>::const_iterator it = m_anatomies.begin();
        return it->second->getFrames();
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::getRows() const
{
    if( !m_anatomies.empty() )
    {
        map<unsigned int, Anatomy *>::const_iterator it = m_anatomies.begin();
        return it->second->getRows();
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////

float DatasetManager::getVoxelX() const
{
    if( !m_anatomies.empty() )
    {
        map<unsigned int, Anatomy *>::const_iterator it = m_anatomies.begin();
        return it->second->getVoxelSizeX();
    }
    return 0.0f;
}

//////////////////////////////////////////////////////////////////////////

float DatasetManager::getVoxelY() const
{
    if( !m_anatomies.empty() )
    {
        map<unsigned int, Anatomy *>::const_iterator it = m_anatomies.begin();
        return it->second->getVoxelSizeY();
    }
    return 0.0f;
}

//////////////////////////////////////////////////////////////////////////

float DatasetManager::getVoxelZ() const
{
    if( !m_anatomies.empty() )
    {
        map<unsigned int, Anatomy *>::const_iterator it = m_anatomies.begin();
        return it->second->getVoxelSizeZ();
    }
    return 0.0f;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::load( const wxString &filename, const wxString &extension )
{
    int result( -1 );

    if( wxT( "nii" ) == extension )
    {
        nifti_image *pHeader = nifti_image_read( filename.char_str(), 0 );
        nifti_image *pBody   = nifti_image_read( filename.char_str(), 1 );

        if( NULL == pHeader || NULL == pBody )
        {
            Logger::getInstance()->print( wxT( "nifti file corrupt, cannot create nifti image from header" ), LOGLEVEL_ERROR );
        }

        if( 16 == pHeader->datatype && 4 == pHeader->ndim && 6 == pHeader->dim[4] )
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
        Logger::getInstance()->print( wxString::Format( wxT( "Unsupported file format \"%s\"" ), extension ), LOGLEVEL_ERROR );
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////

void DatasetManager::remove( const DatasetIndex index )
{
//     DatasetInfo *pDatasetInfo = m_datasets[index];
//     
//     remove( index, pDatasetInfo );
//     m_datasets.erase( index );

//     unsigned int index( -1 );
//     for( map<unsigned int, DatasetInfo *>::iterator it = m_datasets.begin(); it != m_datasets.end(); ++it )
//     {
//         if( ptr == (long)it->second )
//         {
//             index = it->first;
//             break;
//         }
//     }
// 
//     if( -1 != index )
//     {
        map<unsigned int, DatasetInfo *>::iterator it = m_datasets.find( index );
        
        switch( it->second->getType() )
        {
        case HEAD_BYTE:
        case HEAD_SHORT:
        case OVERLAY:
        case RGB:
            Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), index, wxT( "Anatomy" ) ), LOGLEVEL_DEBUG );
            m_anatomies.erase( m_anatomies.find( index ) );
            break;
//         case TENSOR_FIELD:
//             break;
        case MESH:
            Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), index, wxT( "Mesh" ) ), LOGLEVEL_DEBUG );
            m_meshes.erase( m_meshes.find( index ) );
            break;
//         case VECTORS:
//             break;
        case TENSORS:
            Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), index, wxT( "Tensors" ) ), LOGLEVEL_DEBUG );
            m_tensors.erase( m_tensors.find( index ) );
            break;
        case ODFS:
            Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), index, wxT( "ODFs" ) ), LOGLEVEL_DEBUG );
            m_odfs.erase( m_odfs.find( index ) );
            break;
        case FIBERS:
            Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), index, wxT( "Fibers" ) ), LOGLEVEL_DEBUG );
            m_fibers.erase( m_fibers.find( index ) );
            break;
        case SURFACE:
            Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), index, wxT( "Surface" ) ), LOGLEVEL_DEBUG );
            m_surfaces.erase( m_surfaces.find( index ) );
            break;
//         case ISO_SURFACE:
//             break;
        case FIBERSGROUP:
            Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), index, wxT( "FibersGroup" ) ), LOGLEVEL_DEBUG );
            m_fibersGroup.erase( m_fibersGroup.find( index ) );
            break;
        default:
            Logger::getInstance()->print( wxString::Format( wxT( "Removing index: %u type: %s" ), index, wxT( "DatasetInfo" ) ), LOGLEVEL_DEBUG );
            break;
        }

        delete it->second;
        m_datasets.erase( it );
//     }
}

//////////////////////////////////////////////////////////////////////////

void DatasetManager::setDatasetHelper( DatasetHelper * dh )
{
    m_pDatasetHelper = dh;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( Anatomy * pAnatomy )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]  = pAnatomy;
    m_anatomies[index] = pAnatomy;

    return index;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( CIsoSurface * pCIsoSurface )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]  = pCIsoSurface;
    // Verify if a new map is needed for this type

    return index;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( Fibers * pFibers )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]  = pFibers;
    m_fibers[index]    = pFibers;

    return index;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( FibersGroup * pFibersGroup )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]    = pFibersGroup;
    m_fibersGroup[index] = pFibersGroup;

    return index;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( Mesh * pMesh )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]  = pMesh;
    m_meshes[index]    = pMesh;

    return index;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( ODFs * pOdfs )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]  = pOdfs;
    m_odfs[index]      = pOdfs;

    return index;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( Surface * pSurface )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]  = pSurface;
    m_surfaces[index]  = pSurface;

    return index;
}

//////////////////////////////////////////////////////////////////////////

DatasetIndex DatasetManager::insert( Tensors * pTensors )
{
    DatasetIndex index = getNextAvailableIndex();

    m_datasets[index]  = (DatasetInfo *)pTensors;
    m_tensors[index]   = pTensors;

    return index;
}

//////////////////////////////////////////////////////////////////////////

// Loads an anatomy. Extension supported: .nii and .nii.gz
int DatasetManager::loadAnatomy( const wxString &filename, nifti_image *pHeader, nifti_image *pBody )
{
    Logger::getInstance()->print( wxT( "Loading anatomy" ), LOGLEVEL_MESSAGE );
    Anatomy *pAnatomy = new Anatomy( m_pDatasetHelper, filename );
    if( pAnatomy->load( pHeader, pBody ) )
    {
        Logger::getInstance()->print( wxT( "Assigning attributes" ), LOGLEVEL_DEBUG );
        pAnatomy->setThreshold( THRESHOLD );
        pAnatomy->setAlpha( ALPHA );
        pAnatomy->setShow( SHOW );
        pAnatomy->setShowFS( SHOW_FS );
        pAnatomy->setUseTex( USE_TEX );

        DatasetIndex index = insert( pAnatomy );

        m_pDatasetHelper->finishLoading( pAnatomy );

        return index;
    }
    
    delete pAnatomy;
    return -1;
}

//////////////////////////////////////////////////////////////////////////

// Loads a fiber set. Extension supported: .fib, .bundlesdata, .trk and .tck
int DatasetManager::loadFibers( const wxString &filename )
{
    Fibers* l_fibers = new Fibers( m_pDatasetHelper );

    if( l_fibers->load( filename ) )
    {
        std::vector< std::vector< SelectionObject* > > l_selectionObjects = m_pDatasetHelper->getSelectionObjects();
        for( DatasetIndex i( 0 ); i < l_selectionObjects.size(); ++i )
        {
            for( DatasetIndex j( 0 ); j < l_selectionObjects[i].size(); ++j )
            {
                l_selectionObjects[i][j]->m_inBox.resize( m_pDatasetHelper->m_countFibers, sizeof(bool) );
                for( DatasetIndex k( 0 ); k < m_pDatasetHelper->m_countFibers; ++k )
                {
                    l_selectionObjects[i][j]->m_inBox[k] = 0;
                }

                l_selectionObjects[i][j]->setIsDirty( true );
            }
        }

        l_fibers->setThreshold( THRESHOLD );
        l_fibers->setShow     ( SHOW );
        l_fibers->setShowFS   ( SHOW_FS );
        l_fibers->setUseTex   ( USE_TEX );

        DatasetIndex index = insert( l_fibers );

        m_pDatasetHelper->finishLoading( l_fibers );

        l_fibers->updateLinesShown();

        m_pDatasetHelper->m_selBoxChanged = true;

        return index;
    }

    delete l_fibers;
    return -1;
}

//////////////////////////////////////////////////////////////////////////

// Loads a mesh. Extension supported: .mesh, .surf and .dip
int DatasetManager::loadMesh( const wxString &filename, const wxString &extension )
{
    Mesh *pMesh = new Mesh( m_pDatasetHelper, filename );

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

        m_pDatasetHelper->finishLoading( pMesh );

        return index;
    }

    delete pMesh;
    return -1;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::loadODF( const wxString &filename, nifti_image *pHeader, nifti_image *pBody )
{
    Logger::getInstance()->print( wxT( "Loading ODF" ), LOGLEVEL_MESSAGE );

    ODFs *pOdfs = new ODFs( m_pDatasetHelper, filename );
    if( pOdfs->load( pHeader, pBody ) )
    {
        Logger::getInstance()->print( wxT( "Assigning attributes" ), LOGLEVEL_DEBUG );
        pOdfs->setThreshold( THRESHOLD );
        pOdfs->setAlpha( ALPHA );
        pOdfs->setShow( SHOW );
        pOdfs->setShowFS( SHOW_FS );
        pOdfs->setUseTex( USE_TEX );

        DatasetIndex index = insert( pOdfs );

        m_pDatasetHelper->finishLoading( pOdfs );

        return index;
    }

    delete pOdfs;
    return -1;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::loadTensors( const wxString &filename, nifti_image *pHeader, nifti_image *pBody )
{
    Logger::getInstance()->print( wxT( "Loading Tensors" ), LOGLEVEL_MESSAGE );

    Tensors *pTensors = new Tensors( m_pDatasetHelper, filename );
    if( pTensors->load( pHeader, pBody ) )
    {
        Logger::getInstance()->print( wxT( "Assigning attributes" ), LOGLEVEL_DEBUG );
        pTensors->setThreshold( THRESHOLD );
        pTensors->setAlpha( ALPHA );
        pTensors->setShow( SHOW );
        pTensors->setShowFS( SHOW_FS );
        pTensors->setUseTex( USE_TEX );

        DatasetIndex index = insert( pTensors );

        m_pDatasetHelper->finishLoading( pTensors );

        return index;
    }

    delete pTensors;
    return -1;
}

//////////////////////////////////////////////////////////////////////////

DatasetManager::~DatasetManager(void)
{
    Logger::getInstance()->print( wxT( "DatasetManager destruction starting..." ), LOGLEVEL_DEBUG );
    Logger::getInstance()->print( wxT( "Cleaning ressources..." ), LOGLEVEL_MESSAGE );
    for( map<unsigned int, DatasetInfo *>::iterator it = m_datasets.begin(); it != m_datasets.end(); ++it )
    {
        delete it->second;
    }
    Logger::getInstance()->print( wxT( "Ressources cleaned." ), LOGLEVEL_MESSAGE );
    m_pInstance = NULL;
    Logger::getInstance()->print( wxT( "DatasetManager destruction done." ), LOGLEVEL_DEBUG );
}
