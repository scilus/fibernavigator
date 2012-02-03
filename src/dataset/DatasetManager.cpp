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
: m_maxIndex( 8 )
{
    for (unsigned int i( 0 ); i < m_maxIndex + 1; ++i )
    {
        m_freeIndexes.insert( i );
    }    
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

DatasetInfo * DatasetManager::getDataset( int index )
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

int DatasetManager::load( const wxString &filename, const wxString &extension )
{
    if( wxT( "nii" ) == extension )
    {
        int result( -1 );

        nifti_image *pHeader = nifti_image_read( filename.char_str(), 0 );
        nifti_image *pBody   = nifti_image_read( filename.char_str(), 1 );

        if( NULL == pHeader || NULL == pBody )
        {
            Logger::getInstance()->print( wxT( "nifti file corrupt, cannot create nifti image from header" ), LOGLEVEL_ERROR );
            return -1;
        }

        if( 16 == pHeader->datatype && 4 == pHeader->ndim && 6 == pHeader->dim[4] )
        {
            if ( m_anatomies.empty() )
            {
                Logger::getInstance()->print( wxT( "No anatomy file loaded" ), LOGLEVEL_ERROR );
                return -1;
            }
            else if ( !m_tensors.empty() )
            {
                Logger::getInstance()->print( wxT( "Tensors already loaded" ), LOGLEVEL_ERROR );
                return -1;
            }
            result = loadTensors( filename, pHeader, pBody );
        }
        else if( 16 == pHeader->datatype && 4 == pHeader->ndim && ( 
            0 == pHeader->dim[4] || 15 == pHeader->dim[4] || 28 == pHeader->dim[4] || 45 == pHeader->dim[4] || 
            66 == pHeader->dim[4] || 91 == pHeader->dim[4] || 120 == pHeader->dim[4] || 153 == pHeader->dim[4] ) )
        {
            if ( m_anatomies.empty() )
            {
                Logger::getInstance()->print( wxT( "No anatomy file loaded" ), LOGLEVEL_ERROR );
                return -1;
            }
            result = loadODF( filename, pHeader, pBody );
        }
        else
        {
            result = loadAnatomy( filename, pHeader, pBody );
        }

        nifti_image_free( pHeader );
        nifti_image_free( pBody );

        return result;
    }
    else if( wxT("mesh") == extension || wxT( "surf" ) == extension || wxT( "dip" ) == extension )
    {
        if( !isAnatomyLoaded() )
        {
            Logger::getInstance()->print( wxT( "No anatomy file loaded" ), LOGLEVEL_ERROR );
            return -1;
        }

        return loadMesh( filename, extension );
    }
    else if( wxT( "fib" ) == extension || wxT( "trk" ) == extension || wxT( "bundlesdata" ) == extension || wxT( "Bfloat" ) == extension || wxT( "tck" ) == extension )
    {
        if( !isAnatomyLoaded() )
        {
            Logger::getInstance()->print( wxT( "No anatomy file loaded" ), LOGLEVEL_ERROR );
            return -1;
        }
        
        return loadFibers( filename );
    }
    else
    {
        Logger::getInstance()->print( wxString::Format( wxT( "Unsupported file format \"%s\"" ), extension ), LOGLEVEL_ERROR );
    }

//     else if( l_ext == _T( "fib" ) || l_ext == _T( "trk" ) || l_ext == _T( "bundlesdata" ) || l_ext == _T( "Bfloat" ) || l_ext == _T("tck") )
//     {
//         if( ! m_anatomyLoaded )
//         {
//             m_lastError = wxT( "no anatomy file loaded" );
//             return false;
//         }
//         if( m_fibersLoaded )
//         {
//             m_lastError = wxT( "fibers already loaded" );
//             return false;
//         }
// 
//         Fibers* l_fibers = new Fibers( this );
// 
//         if( l_fibers->load( i_fileName ) )
//         {
//             m_fibersLoaded = true;
// 
//             std::vector< std::vector< SelectionObject* > > l_selectionObjects = getSelectionObjects();
//             for( unsigned int i = 0; i < l_selectionObjects.size(); ++i )
//             {
//                 for( unsigned int j = 0; j < l_selectionObjects[i].size(); ++j )
//                 {
//                     l_selectionObjects[i][j]->m_inBox.resize( m_countFibers, sizeof(bool) );
//                     for( unsigned int k = 0; k < m_countFibers; ++k )
//                     {
//                         l_selectionObjects[i][j]->m_inBox[k] = 0;
//                     }
// 
//                     l_selectionObjects[i][j]->setIsDirty( true );
//                 }
//             }
// 
//             l_fibers->setThreshold( i_threshold );
//             l_fibers->setShow     ( i_active );
//             l_fibers->setShowFS   ( i_showFS );
//             l_fibers->setUseTex   ( i_useTex );            
//             finishLoading         ( l_fibers );
// 
//             return true;
//         }
//         return false;
//     }
// 
//     m_lastError = wxT( "unsupported file format" );

    return -1;
}

//////////////////////////////////////////////////////////////////////////

void DatasetManager::setDatasetHelper( DatasetHelper * dh )
{
    m_pDatasetHelper = dh;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::getNextAvailableIndex()
{
    if( m_freeIndexes.empty() )
    {
        for( unsigned int i( m_maxIndex + 1 ); i < 2 * m_maxIndex + 1; ++i )
        {
            m_freeIndexes.insert( i );
        }
        m_maxIndex *= 2;
    }

    int result = *m_freeIndexes.begin();
    m_freeIndexes.erase(m_freeIndexes.begin());
    return result;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::insert( Anatomy * pAnatomy )
{
    int index = getNextAvailableIndex();

    m_datasets[index]  = pAnatomy;
    m_anatomies[index] = pAnatomy;

    return index;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::insert( Fibers * pFibers )
{
    int index = getNextAvailableIndex();

    m_datasets[index]  = pFibers;
    m_fibers[index] = pFibers;

    return index;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::insert( Mesh * pMesh )
{
    int index = getNextAvailableIndex();

    m_datasets[index]  = pMesh;
    m_meshes[index]    = pMesh;

    return index;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::insert( ODFs * pOdfs )
{
    int index = getNextAvailableIndex();

    m_datasets[index]  = pOdfs;
    m_odfs[index]      = pOdfs;

    return index;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::insert( Tensors * pTensors )
{
    int index = getNextAvailableIndex();

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

        int index = insert( pAnatomy );

        m_pDatasetHelper->finishLoading( pAnatomy );

        return index;
    }
    
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
        for( unsigned int i = 0; i < l_selectionObjects.size(); ++i )
        {
            for( unsigned int j = 0; j < l_selectionObjects[i].size(); ++j )
            {
                l_selectionObjects[i][j]->m_inBox.resize( m_pDatasetHelper->m_countFibers, sizeof(bool) );
                for( unsigned int k = 0; k < m_pDatasetHelper->m_countFibers; ++k )
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

        int index = insert( l_fibers );

        m_pDatasetHelper->finishLoading( l_fibers );

        l_fibers->updateLinesShown();

        m_pDatasetHelper->m_selBoxChanged = true;

        return index;
    }

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

        int index = insert( pMesh );

        m_pDatasetHelper->finishLoading( pMesh );

        return index;
    }

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

        int index = insert( pOdfs );

        m_pDatasetHelper->finishLoading( pOdfs );

        return index;
    }

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

        int index = insert( pTensors );

        m_pDatasetHelper->finishLoading( pTensors );

        return index;
    }

    return -1;
}


//////////////////////////////////////////////////////////////////////////

DatasetManager::~DatasetManager(void)
{
    for( map<unsigned int, DatasetInfo *>::iterator it = m_datasets.begin(); it != m_datasets.end(); ++it )
    {
        delete it->second;
    }
}
