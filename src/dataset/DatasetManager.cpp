#include "DatasetManager.h"

#include "Anatomy.h"
#include "DatasetHelper.h"

#include "../Logger.h"
#include "../misc/nifti/nifti1_io.h"

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
            result = loadTensors( filename );
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
            result = loadODF( filename );
        }
        else
        {
            result = loadAnatomy( filename, pHeader, pBody );
        }

        delete pHeader;
        delete pBody;

        return result;
    }
    else if( wxT("mesh") == extension || wxT( "surf" ) == extension || wxT( "dip" ) == extension )
    {
        loadMesh( filename );
    }
    else if( wxT( "fib" ) == extension || wxT( "trk" ) == extension || wxT( "bundlesdata" ) == extension || wxT( "Bfloat" ) == extension || wxT( "tck" ) == extension )
    {
        loadFibers( filename );
    }
    else
    {
        Logger::getInstance()->print( wxString::Format( wxT( "Unsupported file format \"%s\"" ), extension ), LOGLEVEL_ERROR );
    }


//     else if( l_ext == _T( "mesh" ) || l_ext == _T( "surf" ) || l_ext == _T( "dip" ) )
//     {
//         if( ! m_anatomyLoaded )
//         {
//             m_lastError = wxT( "no anatomy file loaded" );
//             return false;
//         }
// 
//         Mesh *l_mesh = new Mesh( this );
// 
//         if( l_mesh->load( i_fileName ) )
//         {
//             l_mesh->setThreshold( i_threshold );
//             l_mesh->setShow     ( i_active );
//             l_mesh->setShowFS   ( i_showFS );
//             l_mesh->setuseTex   ( i_useTex );
//             finishLoading       ( l_mesh );
//             return true;
//         }
//         return false;
//     }
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
//             l_fibers->setuseTex   ( i_useTex );            
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
        pAnatomy->setuseTex( USE_TEX );

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
    return -1;
}

//////////////////////////////////////////////////////////////////////////

// Loads a mesh. Extension supported: .mesh, .surf and .dip
int DatasetManager::loadMesh( const wxString &filename )
{
    return -1;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::loadODF( const wxString &filename )
{
    return -1;
}

//////////////////////////////////////////////////////////////////////////

int DatasetManager::loadTensors( const wxString &filename )
{
    return -1;
}


//////////////////////////////////////////////////////////////////////////

DatasetManager::~DatasetManager(void)
{
    // TODO: Free all space used by the different datasets
}

