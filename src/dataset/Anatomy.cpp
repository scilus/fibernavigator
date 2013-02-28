/*
 *  The Anatomy class implementation.
 *
 */

#include "Anatomy.h"
#include "Fibers.h"

#include "../Logger.h"
#include "../main.h"
#include "../dataset/DatasetManager.h"
#include "../gui/MainFrame.h"
#include "../gui/SceneManager.h"
#include "../gui/SelectionObject.h"
#include "../misc/lic/TensorField.h"
#include "../misc/nifti/nifti1_io.h"

#include <GL/glew.h>
#include <wx/textfile.h>
#include <wx/tglbtn.h>
#include <wx/xml/xml.h>

#include <algorithm>
using std::fill;

#include <cassert>
#include <sstream>
using std::ostringstream;

#include <stack>
using std::stack;

#include <vector>
using std::vector;

#define MIN_HEADER_SIZE 348
#define NII_HEADER_SIZE 352

#define LOWER_EQ_THRES  20
#define UPPER_EQ_THRES  255

Anatomy::Anatomy( ) 
: DatasetInfo(),
  m_isSegmentOn( false ),  
  m_pRoi( NULL ),
  m_dataType( 2 ),
  m_pTensorField( NULL ),
  m_useEqualizedDataset( false ),
  m_lowerEqThreshold( LOWER_EQ_THRES ),
  m_upperEqThreshold( UPPER_EQ_THRES ),
  m_currentLowerEqThreshold( -1 ),
  m_currentUpperEqThreshold( -1 ),
  m_originalAxialOrientation( ORIENTATION_UNDEFINED )
{
    m_bands = 1;
}

Anatomy::Anatomy( const wxString &filename ) 
: DatasetInfo(),
  m_isSegmentOn( false ),  
  m_pRoi( NULL ),
  m_dataType( 2 ),
  m_pTensorField( NULL ),
  m_useEqualizedDataset( false ),
  m_lowerEqThreshold( LOWER_EQ_THRES ),
  m_upperEqThreshold( UPPER_EQ_THRES ),
  m_currentLowerEqThreshold( -1 ),
  m_currentUpperEqThreshold( -1 ),
  m_originalAxialOrientation( ORIENTATION_UNDEFINED )
{
    m_bands = 1;

    m_fullPath = filename;

#ifdef __WXMSW__
    m_name = filename.AfterLast( '\\' );
#else
    m_name = filename.AfterLast( '/' );
#endif
}

// Seems to be used for the create a Distance Map
Anatomy::Anatomy( const Anatomy * const pAnatomy )
: DatasetInfo(),
  m_isSegmentOn( false ),
  m_pRoi( NULL ),
  m_dataType( 2 ),
  m_pTensorField( NULL ),
  m_useEqualizedDataset( false ),
  m_lowerEqThreshold( LOWER_EQ_THRES ),
  m_upperEqThreshold( UPPER_EQ_THRES ),
  m_currentLowerEqThreshold( -1 ),
  m_currentUpperEqThreshold( -1 ),
  m_originalAxialOrientation( ORIENTATION_UNDEFINED )
{
    m_columns = pAnatomy->m_columns;
    m_rows    = pAnatomy->m_rows;
    m_frames  = pAnatomy->m_frames;
    m_bands         = 1;
    m_isLoaded      = true;
    m_type          = HEAD_BYTE;

    createOffset( pAnatomy );
}

Anatomy::Anatomy( std::vector< float >* pDataset, 
                  const int sample ) 
: DatasetInfo(),
  m_isSegmentOn( false ),
  m_pRoi( NULL ),
  m_dataType( 2 ),
  m_pTensorField( NULL ),
  m_useEqualizedDataset( false ),
  m_lowerEqThreshold( LOWER_EQ_THRES ),
  m_upperEqThreshold( UPPER_EQ_THRES ),
  m_currentLowerEqThreshold( -1 ),
  m_currentUpperEqThreshold( -1 ),
  m_originalAxialOrientation( ORIENTATION_UNDEFINED )
{
    m_columns = DatasetManager::getInstance()->getColumns();
    m_rows    = DatasetManager::getInstance()->getRows();
    m_frames  = DatasetManager::getInstance()->getFrames();
    m_bands   = 1;

    m_type    = HEAD_BYTE;

    m_isLoaded = true;

    m_floatDataset.resize( m_columns * m_frames * m_rows );
    std::copy( pDataset->begin(), pDataset->end(), m_floatDataset.begin() );
}

Anatomy::Anatomy( const int type )
: DatasetInfo(),
  m_isSegmentOn( false ),
  m_pRoi( NULL ),
  m_dataType( 2 ),
  m_pTensorField( NULL ),
  m_useEqualizedDataset( false ),
  m_lowerEqThreshold( LOWER_EQ_THRES ),
  m_upperEqThreshold( UPPER_EQ_THRES ),
  m_currentLowerEqThreshold( -1 ),
  m_currentUpperEqThreshold( -1 ),
  m_originalAxialOrientation( ORIENTATION_UNDEFINED )
{
    m_columns = DatasetManager::getInstance()->getColumns();
    m_rows    = DatasetManager::getInstance()->getRows();
    m_frames  = DatasetManager::getInstance()->getFrames();

    if(type == RGB)
    {
        m_bands         = 3;
        m_isLoaded      = true;   
        m_type          = type;

        m_floatDataset.resize( m_columns * m_frames * m_rows * 3, 0.0f );
    }
    else if(type == HEAD_BYTE)
    {
        m_bands         = 1;
        m_isLoaded      = true;   
        m_type          = type;

        m_floatDataset.resize( m_columns * m_frames * m_rows, 0.0f );
    }
    else
    {
        // Only compiled and runned in debug
        assert(false);
    }
}

void Anatomy::add( Anatomy* pAnatomy )
{
    for( unsigned int i = 0; i < m_floatDataset.size(); ++i )
    {
        m_floatDataset[i] += pAnatomy->m_floatDataset[i];
    }
}

//////////////////////////////////////////////////////////////////////////

float Anatomy::at( const int pos ) const
{
    return m_floatDataset[pos];
}

//////////////////////////////////////////////////////////////////////////

std::vector< float >* Anatomy::getFloatDataset()
{
    return &m_floatDataset;
}

//////////////////////////////////////////////////////////////////////////

std::vector< float >* Anatomy::getEqualizedDataset()
{
    return &m_equalizedDataset;
}

//////////////////////////////////////////////////////////////////////////

GLuint Anatomy::getGLuint()
{
    if(0 == m_GLuint)
    {
        generateTexture();
    }

    return m_GLuint;
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::setZero( const int sizeX, 
                       const int sizeY, 
                       const int sizeZ )
{
    m_columns = sizeX;
    m_rows    = sizeY;
    m_frames  = sizeZ;
    m_bands   = 1;

    int datasetSize = m_rows * m_columns * m_frames;

    m_floatDataset.clear();
    m_floatDataset.resize( datasetSize, 0.0f );
    m_equalizedDataset.clear();
    m_equalizedDataset.resize( datasetSize, 0.0f );
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::setRGBZero( const int sizeX, 
                          const int sizeY, 
                          const int sizeZ )
{
    m_columns = sizeX;
    m_rows    = sizeY;
    m_frames  = sizeZ;
    m_bands   = 3;

    int datasetSize = m_rows * m_columns * m_frames;

    m_floatDataset.clear();
    m_floatDataset.resize( datasetSize * m_bands, 0.0f );
    m_equalizedDataset.clear();
    m_equalizedDataset.resize( datasetSize * m_bands, 0.0f );

    m_dataType = 2;
    m_type = RGB;
}

//////////////////////////////////////////////////////////////////////////

TensorField* Anatomy::getTensorField()
{
    return m_pTensorField;
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::dilate()
{
    int datasetSize(m_columns * m_rows * m_frames);
    std::vector<bool> tmp( datasetSize, false );
    int curIndex;

    for( int c(1); c < m_columns - 1; ++c )
    {
        for( int r(1); r < m_rows - 1; ++r )
        {
            for( int f(1); f < m_frames - 1; ++f )
            {
                curIndex = c + r * m_columns + f * m_columns * m_rows;
                if( m_floatDataset[curIndex] == 1.0f )
                {
                    dilateInternal( tmp, curIndex );
                }
            }
        }
    }

    if( m_equalizedDataset.size() != m_floatDataset.size() )
    {
        m_equalizedDataset.resize( m_floatDataset.size() );
    }

    for( int i(0); i < datasetSize; ++i )
    {
        if ( tmp[i] )
        {
            m_floatDataset[i] = 1.0f;
            m_equalizedDataset[i] = 1.0f;
        }
    }

    const GLuint* pTexId = &m_GLuint;
    glDeleteTextures( 1, pTexId );
    Logger::getInstance()->printIfGLError( wxT( "Anatomy::dilate - glDeleteTextures") );

    generateTexture();
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::erode()
{
    int datasetSize = m_columns * m_rows * m_frames;
    std::vector<bool> tmp( datasetSize, false );
    int curIndex;

    for( int c(1); c < m_columns - 1; ++c )
    {
        for( int r(1); r < m_rows - 1; ++r )
        {
            for( int f(1); f < m_frames - 1; ++f )
            {
                curIndex = c + r * m_columns + f * m_columns * m_rows;
                if( m_floatDataset[curIndex] == 1.0f )
                {
                    erodeInternal(tmp, curIndex );
                }
            }
        }
    }

    if( m_equalizedDataset.size() != m_floatDataset.size() )
    {
        m_equalizedDataset.resize( m_floatDataset.size() );
    }

    for( int i(0); i < datasetSize; ++i )
    {
        if( !tmp[i] )
        {
            m_floatDataset[i] = 0.0f;
            m_equalizedDataset[i] = 0.0f;
        }
    }

    const GLuint* pTexId = &m_GLuint;
    glDeleteTextures( 1, pTexId );
    Logger::getInstance()->printIfGLError( wxT( "Anatomy::erode - glDeleteTextures") );
    generateTexture();
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::minimize()
{
    if( !DatasetManager::getInstance()->isFibersLoaded() )
    {
        return;
    }

    std::vector<bool> workData( m_columns * m_rows * m_frames, false );

    long index = MyApp::frame->getCurrentListIndex();
    if( -1 != index )
    {
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->m_pListCtrl->GetItem( index ) );

        int curX, curY, curZ, index;

        for( int i(0); i < pFibers->getLineCount(); ++i )
        {
            if( pFibers->isSelected( i ) )
            {
                for( int j = pFibers->getStartIndexForLine( i ); 
                    j < ( pFibers->getStartIndexForLine( i ) + ( pFibers->getPointsPerLine( i )) ); j += 3 )
                {
                    // TODO: Verify that changing from dh to current obj m_rows & al. is ok
                    curX = std::min( m_columns - 1, std::max( 0, (int)( pFibers->getPointValue( j * 3 )    / m_voxelSizeX ) ) ); // m_dh->m_xVoxel ) );
                    curY = std::min( m_rows    - 1, std::max( 0, (int)( pFibers->getPointValue( j * 3 + 1) / m_voxelSizeY ) ) ); // m_dh->m_yVoxel ) );
                    curZ = std::min( m_frames  - 1, std::max( 0, (int)( pFibers->getPointValue( j * 3 + 2) / m_voxelSizeZ ) ) ); // m_dh->m_zVoxel ) );

                    index = curX + curY * m_columns + curZ * m_rows * m_columns;
                    workData[index] = true;
                }
            }
        }

        int indx = DatasetManager::getInstance()->createAnatomy();
        Anatomy* pNewAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( indx );
        pNewAnatomy->setZero( m_columns, m_rows, m_frames );

        std::vector<float> *pNewAnatDataset = pNewAnatomy->getFloatDataset();

        for( int i(0); i < m_columns * m_rows * m_frames; ++i )
        {
            if( workData[i] && m_floatDataset[i] > 0.0f )
            {
                pNewAnatDataset->at( i ) = 1.0;
            }
        }

        pNewAnatomy->setName( getName() + _T( "(minimal)" ) );
        pNewAnatomy->setType( HEAD_BYTE );
        pNewAnatomy->setDataType( 2 );

        MyApp::frame->m_pListCtrl->InsertItem( indx );
    }
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::flipAxis( AxisType axe )
{
    flipAxisInternal( axe, true );
}

void Anatomy::flipAxisInternal( AxisType axe, const bool regenerateDisplayObjects )
{
    float tmp;
    int curIndex;
    int flipIndex;

    int row(m_rows);
    int col(m_columns);
    int frames(m_frames);

    switch (axe)
    {
        case X_AXIS:
            col /= 2;
            break;
        case Y_AXIS:
            row /= 2;
            break;
        case Z_AXIS:
            frames /= 2;
            break;
        default:
            Logger::getInstance()->print( wxT( "Cannot flip axis. The given axis is undefined." ), LOGLEVEL_ERROR );
            return;
    }

    for( int f(0); f < frames; ++f )
    {
        for( int r(0); r < row; ++r )
        {
            for( int c(0); c < col; ++c )
            {
                curIndex = (c + r * m_columns + f * m_columns * m_rows) * m_bands;

                //Compute the index of the value that will be replaced by the one defined by our current index
                switch (axe)
                {
                    case X_AXIS:
                        flipIndex = ((m_columns - 1 - c) + r * m_columns + f * m_columns * m_rows) * m_bands;
                        break;
                    case Y_AXIS:
                        flipIndex = (c + (m_rows - 1 - r) * m_columns + f * m_columns * m_rows) * m_bands;
                        break;
                    case Z_AXIS:
                        flipIndex = (c + r * m_columns + (m_frames - 1 - f) * m_columns * m_rows) * m_bands;
                        break;
                    default:
                        break;
                }

                for ( int i(0); i < m_bands; ++i )
                { 
                    tmp = m_floatDataset[curIndex + i];
                    m_floatDataset[curIndex + i] = m_floatDataset[flipIndex + i];
                    m_floatDataset[flipIndex + i] = tmp;
                }
            }
        }
    }

    if( 0 != m_equalizedDataset.size() )
    {
        equalizeHistogram();
    }

    if( regenerateDisplayObjects )
    {
        const GLuint* pTexId = &m_GLuint;
        glDeleteTextures( 1, pTexId );
        Logger::getInstance()->printIfGLError( wxT( "Anatomy::flipAxis - glDeleteTextures") );
        generateTexture();
    }
}

//////////////////////////////////////////////////////////////////////////

bool Anatomy::load( nifti_image *pHeader, nifti_image *pBody )
{
    m_columns   = pHeader->dim[1]; 
    m_rows      = pHeader->dim[2]; 
    m_frames    = pHeader->dim[3]; 
    m_bands     = pHeader->dim[4];
    m_dataType  = pHeader->datatype;
    
    // Fix the case where some nifti files have a value of 0 for the fourth
    // dimension. It is a valid case, but some of the code use the number of bands
    // to process the data indenpendantly from its dimension.
    if( m_bands == 0 )
    {
        m_bands = 1;
    }

    m_voxelSizeX = pHeader->dx;
    m_voxelSizeY = pHeader->dy;
    m_voxelSizeZ = pHeader->dz;

    if( DatasetManager::getInstance()->isAnatomyLoaded() )
    {
        int   columns = DatasetManager::getInstance()->getColumns();
        int   rows    = DatasetManager::getInstance()->getRows();
        int   frames  = DatasetManager::getInstance()->getFrames();
        float voxelX  = DatasetManager::getInstance()->getVoxelX();
        float voxelY  = DatasetManager::getInstance()->getVoxelY();
        float voxelZ  = DatasetManager::getInstance()->getVoxelZ();
        
        const float VOXEL_SIZE_EPSILON(0.0001f);

        if( m_rows != rows || m_columns != columns || m_frames != frames )
        {
            Logger::getInstance()->print( wxT( "Dimensions of loaded files must be the same" ), LOGLEVEL_ERROR );
            return false;
        }
        
        if( m_voxelSizeX != voxelX || m_voxelSizeY != voxelY || m_voxelSizeZ != voxelZ )
        {
            // NOTE TO THE TEAM: THIS IS NOT A VERY GOOD THING TO DO. We do it to support
            // different software that save the metadata with incorrect rounding / conversion.
            if( std::abs(m_voxelSizeX - voxelX) < VOXEL_SIZE_EPSILON &&
                std::abs(m_voxelSizeY - voxelY) < VOXEL_SIZE_EPSILON &&
                std::abs(m_voxelSizeZ - voxelZ) < VOXEL_SIZE_EPSILON )
            {
                // In this case, we are in the expected error range between something coming from an integer
                // and something coming from a float. We accept it, make sure thesizes fit for the new anatomy, 
                // and still display a debug message for developers.
                m_voxelSizeX = voxelX;
                m_voxelSizeY = voxelY;
                m_voxelSizeZ = voxelZ;
                
                Logger::getInstance()->print( wxT( "Voxel sizes did not exactly fit. In expected float range error. Using the already loaded voxel sizes." ),
                                              LOGLEVEL_DEBUG );
            }
            else
            {
                Logger::getInstance()->print( wxT( "Voxel size different from anatomy" ), LOGLEVEL_ERROR );
                return false;
            }
        }
    }

    // Get the transformation to put the anatomy file in world space.
    // The transformation used depends on the one used in the nifti image.
    // We currently only use it when loading Mrtrix fibers.
    if( pHeader->sform_code > 0 )
    {
        FMatrix &transform = DatasetManager::getInstance()->getNiftiTransform();

        transform( 0, 0 ) = pHeader->sto_xyz.m[0][0];
        transform( 0, 1 ) = pHeader->sto_xyz.m[0][1];
        transform( 0, 2 ) = pHeader->sto_xyz.m[0][2];
        transform( 0, 3 ) = pHeader->sto_xyz.m[0][3];
        transform( 1, 0 ) = pHeader->sto_xyz.m[1][0];
        transform( 1, 1 ) = pHeader->sto_xyz.m[1][1];
        transform( 1, 2 ) = pHeader->sto_xyz.m[1][2];
        transform( 1, 3 ) = pHeader->sto_xyz.m[1][3];
        transform( 2, 0 ) = pHeader->sto_xyz.m[2][0];
        transform( 2, 1 ) = pHeader->sto_xyz.m[2][1];
        transform( 2, 2 ) = pHeader->sto_xyz.m[2][2];
        transform( 2, 3 ) = pHeader->sto_xyz.m[2][3];
        transform( 3, 0 ) = pHeader->sto_xyz.m[3][0];
        transform( 3, 1 ) = pHeader->sto_xyz.m[3][1];
        transform( 3, 2 ) = pHeader->sto_xyz.m[3][2];
        transform( 3, 3 ) = pHeader->sto_xyz.m[3][3];
    }
    else if( pHeader->qform_code > 0 )
    {
        FMatrix &transform = DatasetManager::getInstance()->getNiftiTransform();

        transform( 0, 0 ) = pHeader->qto_xyz.m[0][0];
        transform( 0, 1 ) = pHeader->qto_xyz.m[0][1];
        transform( 0, 2 ) = pHeader->qto_xyz.m[0][2];
        transform( 0, 3 ) = pHeader->qto_xyz.m[0][3];
        transform( 1, 0 ) = pHeader->qto_xyz.m[1][0];
        transform( 1, 1 ) = pHeader->qto_xyz.m[1][1];
        transform( 1, 2 ) = pHeader->qto_xyz.m[1][2];
        transform( 1, 3 ) = pHeader->qto_xyz.m[1][3];
        transform( 2, 0 ) = pHeader->qto_xyz.m[2][0];
        transform( 2, 1 ) = pHeader->qto_xyz.m[2][1];
        transform( 2, 2 ) = pHeader->qto_xyz.m[2][2];
        transform( 2, 3 ) = pHeader->qto_xyz.m[2][3];
        transform( 3, 0 ) = pHeader->qto_xyz.m[3][0];
        transform( 3, 1 ) = pHeader->qto_xyz.m[3][1];
        transform( 3, 2 ) = pHeader->qto_xyz.m[3][2];
        transform( 3, 3 ) = pHeader->qto_xyz.m[3][3];
    }
    else
    {
        Logger::getInstance()->print( wxT( "No transformation encoded in the nifti file. Using identity transform." ), LOGLEVEL_WARNING );

        // This is not a typo, the method is called makeIdendity in FMatrix.
        DatasetManager::getInstance()->getNiftiTransform().makeIdendity();
    }
    
    // Guess the original data orientation from the transformation matrix.
    if( pHeader->sform_code > 0 )
    {
        if( pHeader->sto_xyz.m[0][0] < 0.0 )
        {
            m_originalAxialOrientation = ORIENTATION_RIGHT_TO_LEFT;
        }
        else
        {
            m_originalAxialOrientation = ORIENTATION_LEFT_TO_RIGHT;
        }
    }
    else if( pHeader->qform_code > 0 )
    {
        if( pHeader->qto_xyz.m[0][0] < 0.0 )
        {
            m_originalAxialOrientation = ORIENTATION_RIGHT_TO_LEFT;
        }
        else
        {
            m_originalAxialOrientation = ORIENTATION_LEFT_TO_RIGHT;
        }
    }
    
    // Check the data type.
    if( pHeader->datatype == 2 )
    {
        if( pHeader->dim[4] == 1 )
        {
            m_type = HEAD_BYTE;
        }
        else if( pHeader->dim[4] == 3 )
        {
            m_type = RGB;
        }
        else
        {
            m_type = BOT_INITIALIZED;
        }
    }
    else if( pHeader->datatype == 4 )
    {
        m_type = HEAD_SHORT;
    }

    else if( pHeader->datatype == 16 )
    {
        if( pHeader->dim[4] == 3 )
        {
            m_type = VECTORS;
        }
        else
        {
            m_type = OVERLAY;
        }
    }
    else
    {
        m_type = BOT_INITIALIZED;
    }

//     nifti_image *pFileData = nifti_image_read( pHdrFile, 1 );
//     if( !pFileData )
//     {
//         m_dh->m_lastError = wxT( "nifti file corrupt" );
//         return false;
//     }
    
    int datasetSize = pHeader->dim[1] * pHeader->dim[2] * pHeader->dim[3];

    bool flag = false;

    switch( m_type )
    {
        case HEAD_BYTE:
        {
            unsigned char* pData = (unsigned char*)pBody->data;
            m_floatDataset.resize( datasetSize );

            for( int i(0); i < datasetSize; ++i )
            {
                m_floatDataset[i] = (float)pData[i] / 255.0;
            }

            flag = true;
            m_oldMax = 255;
            break;
        }

        case HEAD_SHORT:
        {
            short int* pData = (short int*)pBody->data;
            int dataMax = 0;
            std::vector<int> histo( 65536, 0 );

            for( int i(0); i < datasetSize; ++i )
            {
                dataMax = wxMax(dataMax, pData[i]);
                ++histo[pData[i]];
            }

            int fivePercent   = (int)( datasetSize * 0.001 );
            int newMax        = 65535;
            int adder         = 0;

            for( int i(65535); i > 0; --i )
            {
                adder += histo[i];
                newMax = i;

                if( adder > fivePercent )
                {
                    break;
                }
            }
            
            for( int i(0); i < datasetSize; ++i )
            {
                if ( pData[i] > newMax )
                {
                    pData[i] = newMax;
                }
            }

            m_floatDataset.resize( datasetSize );

            for( int i(0); i < datasetSize; ++i )
            {
                m_floatDataset[i] = (float)pData[i] / (float)newMax;
            }

            m_oldMax    = dataMax;
            m_newMax    = newMax;
            flag        = true;
            break;
        }

        case OVERLAY:
        {
            float* pData = (float*)pBody->data;

            m_floatDataset.resize( datasetSize );
            
            for( int i(0); i < datasetSize; ++i )
            {
                m_floatDataset[i] = (float)pData[i];
            }

            float dataMax = 0.0f;
            for( int i(0); i < datasetSize; ++i )
            {
                if (m_floatDataset[i] > dataMax)
                {
                    dataMax = m_floatDataset[i];
                }
            }

            for( int i(0); i < datasetSize; ++i )
            {
                m_floatDataset[i] = m_floatDataset[i] / dataMax;
            }

            m_oldMax    = dataMax;
            m_newMax    = 1.0;
            flag        = true;
            break;
        }

        case RGB:
        {
            unsigned char* pData = (unsigned char*)pBody->data;

            m_floatDataset.resize( datasetSize * 3 );

            for( int i(0); i < datasetSize; ++i )
            {
                m_floatDataset[i * 3]       = (float)pData[i]  / 255.0f;
                m_floatDataset[i * 3 + 1]   = (float)pData[datasetSize + i] / 255.0f;
                m_floatDataset[i * 3 + 2]   = (float)pData[(2 * datasetSize) + i] / 255.0f;
            }

            flag = true;
            break;
        }

        case VECTORS:
        {
            float* pData = (float*)pBody->data;
            m_floatDataset.resize( datasetSize * 3 );

            for( int i(0); i < datasetSize; ++i )
            {
                m_floatDataset[i * 3]       = pData[i];
                m_floatDataset[i * 3 + 1]   = pData[datasetSize + i];
                m_floatDataset[i * 3 + 2]   = pData[2 * datasetSize + i];
            }

            flag = true;
            break;
        }

        default:
        {
            Logger::getInstance()->print( wxT( "Unsupported file format" ), LOGLEVEL_ERROR );
            flag = false;
            // Will not return now to make sure the pHdrFile pointer is freed.
        }
    }

    m_isLoaded = flag;
    
    // Flip the data if needed.
    if( m_originalAxialOrientation == ORIENTATION_RIGHT_TO_LEFT )
    {
        flipAxisInternal( X_AXIS, false );
    }
    
    if( m_isLoaded && m_type == VECTORS )
    {
        m_pTensorField = new TensorField( m_columns, m_rows, m_frames, &m_floatDataset, 1, 3 );
    }

    return flag;
}

//////////////////////////////////////////////////////////////////////////

bool Anatomy::save( wxXmlNode *pNode ) const
{
    assert( pNode != NULL );

    pNode->SetName( wxT( "dataset" ) );
    DatasetInfo::save( pNode );

    return true;
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::saveNifti( wxString fileName )
{
    // Prevents copying the whole vector
    vector<float> *pDataset = m_useEqualizedDataset ? &m_equalizedDataset : &m_floatDataset;

    int dims[] = { 4, m_columns, m_rows, m_frames, m_bands, 0, 0, 0 };
    nifti_image* pImage(NULL);
    pImage = nifti_make_new_nim( dims, m_dataType, 1 );
    
    if( !fileName.EndsWith( _T( ".nii" ) ) && !fileName.EndsWith( _T( ".nii.gz" ) ) )
    {
        fileName += _T( ".nii.gz" );
    }   

    char fn[1024];
    strcpy( fn, (const char*)fileName.mb_str( wxConvUTF8 ) );

    pImage->qform_code = 1;    
    pImage->datatype   = m_dataType;
    pImage->fname = fn;
    pImage->dx = m_voxelSizeX;
    pImage->dy = m_voxelSizeY;
    pImage->dz = m_voxelSizeZ;

    if( m_type == HEAD_BYTE )
    {
        vector<unsigned char> tmp( pDataset->size() );
        for(unsigned int i(0); i < pDataset->size(); ++i )
        {
            tmp[i] = (*pDataset)[i] * 255;
        }
        
        // Do not move the call to nifti_image_write out of the 
        // if, because it will crash, since the temp vector will
        // not exist anymore, and pImage->data will point to garbage.
        pImage->data = &tmp[0];
        nifti_image_write( pImage );
    }
    else if( m_type == HEAD_SHORT )
    {
        vector<short> tmp( pDataset->size() );
        for(unsigned int i(0); i < pDataset->size(); ++i )
        {
            tmp[i] = (short)( (*pDataset)[i] * m_newMax );
        }
        
        // Do not move the call to nifti_image_write out of the 
        // if, because it will crash, since the temp vector will
        // not exist anymore, and pImage->data will point to garbage.
        pImage->data = &tmp[0];
        nifti_image_write( pImage );
    }
    else if( m_type == RGB )
    {
        vector<unsigned char> tmp( pDataset->size() );
        int datasetSize = pDataset->size()/3;
        for( int i(0); i < datasetSize; ++i )
        {
            tmp[i]                   = (*pDataset)[i * 3]     * 255.0f;
            tmp[datasetSize + i]     = (*pDataset)[i * 3 + 1] * 255.0f;
            tmp[2 * datasetSize + i] = (*pDataset)[i * 3 + 2] * 255.0f;
        }
        
        // Do not move the call to nifti_image_write out of the 
        // if, because it will crash, since the temp vector will
        // not exist anymore, and pImage->data will point to garbage.
        pImage->data = &tmp[0];
        nifti_image_write( pImage );
    }
    else
    {
        // Do not move the call to nifti_image_write out of the 
        // if, because it will crash, since the temp vector will
        // not exist anymore, and pImage->data will point to garbage.
        pImage->data = &(*pDataset)[0];
        nifti_image_write( pImage );
    }
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::createPropertiesSizer( PropertiesWindow *pParent )
{
    DatasetInfo::createPropertiesSizer( pParent );

    wxBoxSizer *pBoxMain = new wxBoxSizer( wxVERTICAL );

    //////////////////////////////////////////////////////////////////////////

    // Init widgets
    m_pLowerEqSlider =       new wxSlider( pParent, wxID_ANY, m_lowerEqThreshold * .2f, 0, 51, wxDefaultPosition, wxSize( 120, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pUpperEqSlider =       new wxSlider( pParent, wxID_ANY, m_upperEqThreshold * .2f, 0, 51, wxDefaultPosition, wxSize( 120, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pEqualize      = new wxToggleButton( pParent, wxID_ANY, wxT( "Equalize" ),               wxDefaultPosition, wxSize( 140, -1 ) );
    m_pBtnDilate =           new wxButton( pParent, wxID_ANY, wxT( "Dilate" ),                 wxDefaultPosition, wxSize( 85,  -1 ) );
    m_pBtnErode  =           new wxButton( pParent, wxID_ANY, wxT( "Erode" ),                  wxDefaultPosition, wxSize( 85,  -1 ) );
    m_pBtnCut =              new wxButton( pParent, wxID_ANY, wxT( "Cut (boxes)" ),            wxDefaultPosition, wxSize( 85,  -1 ) );
    m_pBtnMinimize =         new wxButton( pParent, wxID_ANY, wxT( "Minimize (fibers)" ),      wxDefaultPosition, wxSize( 85,  -1 ) );
    m_pBtnNewDistanceMap =   new wxButton( pParent, wxID_ANY, wxT( "New Distance Map" ),       wxDefaultPosition, wxSize( 140, -1 ) );
    m_pBtnNewIsoSurface  =   new wxButton( pParent, wxID_ANY, wxT( "New Iso Surface" ),        wxDefaultPosition, wxSize( 140, -1 ) );
    m_pBtnNewOffsetSurface = new wxButton( pParent, wxID_ANY, wxT( "New Offset Surface" ),     wxDefaultPosition, wxSize( 140, -1 ) );
    m_pBtnNewVOI =           new wxButton( pParent, wxID_ANY, wxT( "New VOI" ),                wxDefaultPosition, wxSize( 140, -1 ) );
    m_pToggleSegment = new wxToggleButton( pParent, wxID_ANY, wxT( "Floodfill" ),              wxDefaultPosition, wxSize( 140, -1 ) );  

    m_pSliderFlood = new MySlider( pParent, wxID_ANY, 40, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    setFloodThreshold( 0.2f );

    m_pTxtThres = new wxTextCtrl( pParent, wxID_ANY, wxT( "0.20" ), wxDefaultPosition, wxSize( 40, -1 ), wxTE_READONLY );
    m_pLblThres = new wxStaticText( pParent, wxID_ANY, wxT( "Threshold" ) );

    //////////////////////////////////////////////////////////////////////////

    wxFlexGridSizer *pGridSliders = new wxFlexGridSizer( 2 );
    pGridSliders->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Lower Threshold" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pLowerEqSlider, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL, 1 );
    pGridSliders->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Upper Threshold" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pUpperEqSlider, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL, 1 );
    pBoxMain->Add( pGridSliders, 0, wxEXPAND | wxALL, 2 );

    //////////////////////////////////////////////////////////////////////////

    pBoxMain->Add( m_pEqualize, 0, wxALIGN_CENTER | wxEXPAND | wxRIGHT | wxLEFT, 24 );

    wxGridSizer *pGridButtons = new wxGridSizer( 2 );
    pGridButtons->Add( m_pBtnDilate,   0, wxEXPAND | wxALL, 1 );
    pGridButtons->Add( m_pBtnErode,    0, wxEXPAND | wxALL, 1 );
    pGridButtons->Add( m_pBtnCut,      0, wxEXPAND | wxALL, 1 );
    pGridButtons->Add( m_pBtnMinimize, 0, wxEXPAND | wxALL, 1 );
    pBoxMain->Add( pGridButtons, 0, wxEXPAND | wxALL | wxALIGN_CENTER, 2 );

    pBoxMain->Add( m_pBtnNewDistanceMap,   0, wxALIGN_CENTER | wxEXPAND | wxRIGHT | wxLEFT, 24 );
    pBoxMain->Add( m_pBtnNewIsoSurface,    0, wxALIGN_CENTER | wxEXPAND | wxRIGHT | wxLEFT, 24 );
    pBoxMain->Add( m_pBtnNewOffsetSurface, 0, wxALIGN_CENTER | wxEXPAND | wxRIGHT | wxLEFT, 24 );
    pBoxMain->Add( m_pBtnNewVOI,           0, wxALIGN_CENTER | wxEXPAND | wxRIGHT | wxLEFT, 24 );
    pBoxMain->Add( m_pToggleSegment,       0, wxALIGN_CENTER | wxEXPAND | wxRIGHT | wxLEFT, 24 );

    //////////////////////////////////////////////////////////////////////////

    wxBoxSizer *pBoxFlood = new wxBoxSizer( wxHORIZONTAL );
    pBoxFlood->Add( m_pLblThres,   0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxFlood->Add( m_pSliderFlood, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxFlood->Add( m_pTxtThres, 0, wxFIXED_MINSIZE | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxMain->Add( pBoxFlood, 0, wxEXPAND, 0 );

    //////////////////////////////////////////////////////////////////////////

    m_pPropertiesSizer->Add( pBoxMain, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );


    // Connect widgets with callback function
    pParent->Connect( m_pLowerEqSlider->GetId(),       wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnEqualizationSliderChange ) );
    pParent->Connect( m_pUpperEqSlider->GetId(),       wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnEqualizationSliderChange ) );
    pParent->Connect( m_pEqualize->GetId(),            wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler       ( PropertiesWindow::OnEqualizeDataset ) );
    pParent->Connect( m_pBtnDilate->GetId(),           wxEVT_COMMAND_BUTTON_CLICKED,       wxCommandEventHandler( PropertiesWindow::OnDilateDataset ) );
    pParent->Connect( m_pBtnErode->GetId(),            wxEVT_COMMAND_BUTTON_CLICKED,       wxCommandEventHandler( PropertiesWindow::OnErodeDataset ) );
    pParent->Connect( m_pBtnCut->GetId(),              wxEVT_COMMAND_BUTTON_CLICKED,       wxCommandEventHandler( PropertiesWindow::OnListItemCutOut ) );
    pParent->Connect( m_pBtnMinimize->GetId(),         wxEVT_COMMAND_BUTTON_CLICKED,       wxCommandEventHandler( PropertiesWindow::OnMinimizeDataset ) );
    pParent->Connect( m_pBtnNewDistanceMap->GetId(),   wxEVT_COMMAND_BUTTON_CLICKED,       wxCommandEventHandler( PropertiesWindow::OnNewDistanceMap ) );
    pParent->Connect( m_pBtnNewIsoSurface->GetId(),    wxEVT_COMMAND_BUTTON_CLICKED,       wxCommandEventHandler( PropertiesWindow::OnNewIsoSurface ) );
    pParent->Connect( m_pBtnNewOffsetSurface->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,       wxCommandEventHandler( PropertiesWindow::OnNewOffsetSurface ) );
    pParent->Connect( m_pBtnNewVOI->GetId(),           wxEVT_COMMAND_BUTTON_CLICKED,       wxCommandEventHandler( PropertiesWindow::OnNewVoiFromOverlay ) );
    pParent->Connect( m_pToggleSegment->GetId(),       wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnFloodFill ) );
    pParent->Connect( m_pSliderFlood->GetId(),         wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnSliderFloodMoved ) );



    // The following interface objects are related to flood fill and graph cuts.
    // They are kept here temporarily, but will need to be implemented or removed.
    // Please also note that the coding standard has not been applied to these lines, 
    // so please apply it if you re enable them.
//     pSizer = new wxBoxSizer(wxHORIZONTAL);
//     m_pRadioBtnFlood = new wxRadioButton(pParentWindow, wxID_ANY, _T( "Click region" ), wxDefaultPosition, wxSize(80,-1));
//     pSizer->Add(new wxStaticText(pParentWindow, wxID_ANY, wxT("Floodfill   "),wxDefaultPosition, wxSize(50,-1), wxALIGN_RIGHT),0,wxALIGN_CENTER);
//     pSizer->Add(m_pRadioBtnFlood);
//     m_propertiesSizer->Add(pSizer,0,wxALIGN_CENTER);
//     pParentWindow->Connect(m_pRadioBtnFlood->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(PropertiesWindow::OnFloodFill));
// 
//     pSizer = new wxBoxSizer(wxHORIZONTAL);
//     m_pRadioBtnObj = new wxRadioButton(pParentWindow, wxID_ANY, _T( "Select Class 1" ), wxDefaultPosition, wxSize(85,-1));
//     pSizer->Add(new wxStaticText(pParentWindow, wxID_ANY, wxT("Graphcut   "),wxDefaultPosition, wxSize(55,-1), wxALIGN_RIGHT),0,wxALIGN_CENTER);
//     pSizer->Add(m_pRadioBtnObj);
//     m_propertiesSizer->Add(pSizer,0,wxALIGN_CENTER);
//     pParentWindow->Connect(m_pRadioBtnObj->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(PropertiesWindow::OnSelectObj));
//     
//     pSizer = new wxBoxSizer(wxHORIZONTAL);
//     m_pRadioBtnBck = new wxRadioButton(pParentWindow, wxID_ANY, _T( "Select Class 2" ), wxDefaultPosition, wxSize(85,-1));
//     pSizer->Add(new wxStaticText(pParentWindow, wxID_ANY, wxT("Graphcut   "),wxDefaultPosition, wxSize(55,-1), wxALIGN_RIGHT),0,wxALIGN_CENTER);
//     pSizer->Add(m_pRadioBtnBck);
//     m_propertiesSizer->Add(pSizer,0,wxALIGN_CENTER);
//     pParentWindow->Connect(m_pRadioBtnBck->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(PropertiesWindow::OnSelectBck));
// 
//     m_pSliderGraphSigma = new MySlider(pParentWindow, wxID_ANY,0,0,500, wxDefaultPosition, wxSize(80,-1), wxSL_HORIZONTAL | wxSL_AUTOTICKS);
//     m_pSliderGraphSigma->SetValue(200);
//     setGraphSigma(200.0f);
//     pSizer = new wxBoxSizer(wxHORIZONTAL);
//     pSizer->Add(new wxStaticText(pParentWindow, wxID_ANY, wxT("Sigma "),wxDefaultPosition, wxSize(60,-1), wxALIGN_RIGHT),0,wxALIGN_CENTER);
//     pSizer->Add(m_pSliderGraphSigma,0,wxALIGN_CENTER);
//     m_propertiesSizer->Add(pSizer,0,wxALIGN_CENTER);
//     pParentWindow->Connect(m_pSliderGraphSigma->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(PropertiesWindow::OnSliderGraphSigmaMoved));
// 
//     m_pBtnGraphCut = new wxButton(pParentWindow, wxID_ANY, wxT("Generate Graphcut"), wxDefaultPosition, wxSize(120,-1));
//     pSizer = new wxBoxSizer(wxHORIZONTAL);
//     pSizer->Add(m_pBtnGraphCut,0,wxALIGN_CENTER);
//     m_pBtnGraphCut->Enable(m_dh->graphcutReady());
//     m_propertiesSizer->Add(pSizer,0,wxALIGN_CENTER);
//     pParentWindow->Connect(m_pBtnGraphCut->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnbtnGraphCut));
// 
//     m_pBtnKmeans = new wxButton(pParentWindow, wxID_ANY, wxT("K-Means"), wxDefaultPosition, wxSize(132,-1));
//     pSizer = new wxBoxSizer(wxHORIZONTAL);
//     pSizer->Add(m_pBtnKmeans,0,wxALIGN_CENTER);
//     m_propertiesSizer->Add(pSizer,0,wxALIGN_CENTER);
//     pParentWindow->Connect(m_pBtnKmeans->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnKmeans));
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();
    
    m_pLowerEqSlider->Enable( 1 == m_bands );
    m_pUpperEqSlider->Enable( 1 == m_bands );
    m_pEqualize->Enable(      1 == m_bands );
    m_pBtnMinimize->Enable( DatasetManager::getInstance()->isFibersLoaded() );
    m_pBtnCut->Enable(      SceneManager::getInstance()->getSelectionObjects().size() > 0 );

    m_pBtnNewIsoSurface->Enable(    getType() <= OVERLAY );
    m_pBtnNewDistanceMap->Enable(   getType() <= OVERLAY );
    m_pBtnNewOffsetSurface->Enable( getType() <= OVERLAY );

    m_pBtnNewVOI->Enable(   getType() <= OVERLAY );
 
    //m_pBtnGraphCut->Enable( m_dh->graphcutReady() );
    
    if(!m_isSegmentOn)
    {
        m_pLblThres->Hide();
        m_pSliderFlood->Hide();
        m_pTxtThres->Hide();
    }
    else
    {
        m_pLblThres->Show();
        m_pSliderFlood->Show();
        m_pTxtThres->Show();
    }
}

void Anatomy::updateTexture( SubTextureBox drawZone, const bool isRound, float color ) 
{
    drawZone.datasize = drawZone.width * drawZone.height * drawZone.depth;
    //save this zone on top of history before we change anything
    fillHistory(drawZone, false);

    //create the modified region's vector in the right color
    std::vector<float> subData( drawZone.datasize, color );

    for( int f = 0; f < drawZone.depth; ++f )
    {
        for( int r = 0; r < drawZone.height; ++r )
        {
            for( int c = 0; c < drawZone.width; ++c )
            {
                int sourceIndex = (c+drawZone.x) + (r+drawZone.y) * m_columns + (f+drawZone.z) * m_columns * m_rows;
                int subIndex = c + r * drawZone.width + f * drawZone.width * drawZone.height;

                //save a backup in the history data
                //drawZone.data[subIndex] = m_floatDataset[sourceIndex];

                //update values
                if(isRound)
                {
                    //we might have one direction without proper size (flat), but never 2
                    double radius = (double)( std::max(drawZone.width, drawZone.height) - 1 ) / 2.0;

                    //inside sphere: put color in the source
                    if(( Vector(double(drawZone.width)/2.0, double(drawZone.height)/2.0, double(drawZone.depth)/2.0) - Vector(double(c), double(r), double(f)) ).getLength() <= radius)
                    {
                        m_floatDataset[sourceIndex] = color;
                    }
                    else //outside sphere: copy source values in the subImage
                    {
                        subData[subIndex] = m_floatDataset[sourceIndex];
                    }
                }
                else
                {
                    m_floatDataset[sourceIndex] = color;
                }
            }
        }
    }

    glBindTexture(GL_TEXTURE_3D, m_GLuint);    //The texture we created already
    Logger::getInstance()->printIfGLError( wxT( "Anatomy::updateTexture - glBindTexture") );
    glTexSubImage3D( GL_TEXTURE_3D, 0, drawZone.x, drawZone.y, drawZone.z, drawZone.width, drawZone.height, drawZone.depth, GL_LUMINANCE, GL_FLOAT, &subData[0] );
    Logger::getInstance()->printIfGLError( wxT( "Anatomy::updateTexture - glTexSubImage3D") );
}

void Anatomy::updateTexture( SubTextureBox drawZone, const bool isRound, wxColor colorRGB ) 
{
    drawZone.datasize = drawZone.width * drawZone.height * drawZone.depth * 3;
    //save this zone on top of history before we change anything
    fillHistory(drawZone, true);
    
    //create the modified region's vector and put the right color
    std::vector<float> subData( drawZone.datasize, colorRGB.Red() );
    for( int i=0; i < drawZone.datasize; i+=3 )
    {
        //subData[i] = colorRGB.Red(); //done at declaration
        subData[i+1] = colorRGB.Green();
        subData[i+2] = colorRGB.Blue();
    }
    
    for( int f = 0; f < drawZone.depth; ++f )
    {
        for( int r = 0; r < drawZone.height; ++r )
        {
            for( int c = 0; c < drawZone.width; ++c )
            {
                int sourceIndex = (c+drawZone.x) + (r+drawZone.y) * m_columns + (f+drawZone.z) * m_columns * m_rows;
                int subIndex = c + r * drawZone.width + f * drawZone.width * drawZone.height;

                if(isRound)
                {
                    //we might have one direction without proper size (flat), but never 2
                    double radius = (double)(std::max( drawZone.width, drawZone.height ) - 1) / 2.0;

                    //inside sphere: put color in the source
                    if(( Vector(double(drawZone.width)/2.0, double(drawZone.height)/2.0, double(drawZone.depth)/2.0) - Vector(double(c), double(r), double(f)) ).getLength() <= radius)
                    {
                        m_floatDataset[sourceIndex*3] = colorRGB.Red();
                        m_floatDataset[sourceIndex*3 + 1] = colorRGB.Green();
                        m_floatDataset[sourceIndex*3 + 2] = colorRGB.Blue();
                    }
                    else //outside sphere: copy source values in the subImage
                    {
                        subData[subIndex*3] = m_floatDataset[sourceIndex*3];
                        subData[subIndex*3 + 1] = m_floatDataset[sourceIndex*3 + 1];
                        subData[subIndex*3 + 2] = m_floatDataset[sourceIndex*3 + 2];
                    }
                }
                else 
                {
                    m_floatDataset[sourceIndex*3] = colorRGB.Red();
                    m_floatDataset[sourceIndex*3 + 1] = colorRGB.Green();
                    m_floatDataset[sourceIndex*3 + 2] = colorRGB.Blue();
                }
            }
        }
    }

    glBindTexture(GL_TEXTURE_3D, m_GLuint);    //The texture we created already
    Logger::getInstance()->printIfGLError( wxT( "Anatomy::updateTexture - glBindTexture") );
    glTexSubImage3D( GL_TEXTURE_3D, 0, drawZone.x, drawZone.y, drawZone.z, drawZone.width, drawZone.height, drawZone.depth, GL_RGB, GL_FLOAT, &subData[0] );
    Logger::getInstance()->printIfGLError( wxT( "Anatomy::updateTexture - glTexSubImage3D") );
}

//////////////////////////////////////////////////////////////////////////

bool Anatomy::toggleEqualization()
{
    m_useEqualizedDataset = !m_useEqualizedDataset;

    if ( m_useEqualizedDataset && ( m_lowerEqThreshold != m_currentLowerEqThreshold || m_upperEqThreshold != m_currentUpperEqThreshold ) )
    {
        equalizeHistogram();
    }

    const GLuint* pTexId = &m_GLuint;
    glDeleteTextures( 1, pTexId );
    Logger::getInstance()->printIfGLError( wxT( "Anatomy::toggleEqualization - glDeleteTextures") );
    generateTexture();

    return m_useEqualizedDataset;
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::equalizationSliderChange()
{
    m_lowerEqThreshold = m_pLowerEqSlider->GetValue() * 5;
    m_upperEqThreshold = m_pUpperEqSlider->GetValue() * 5;
    if ( m_useEqualizedDataset && ( m_lowerEqThreshold != m_currentLowerEqThreshold || m_upperEqThreshold != m_currentUpperEqThreshold ) )
    {
        Logger::getInstance()->print( wxT( "Calling equalizeHistogram" ), LOGLEVEL_DEBUG );
        equalizeHistogram();

        const GLuint* pTexId = &m_GLuint;
        glDeleteTextures( 1, pTexId );
        Logger::getInstance()->printIfGLError( wxT( "Anatomy::equalizationSliderChange - glDeleteTextures") );
        generateTexture();
    }
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::createOffset( const Anatomy * const pAnatomy )
{
    int b, r, c, bb, rr, r0, b0, c0;
    int i, istart, iend;
    int nbPixels;
    int d, d1, d2, cc1, cc2;
    float u, dmin;
    bool *pSrcPix;
    double g;

    const int nbBands( m_frames );
    const int nbRows( m_rows );
    const int nbCols( m_columns );

    nbPixels = wxMax( nbBands, nbRows );
    double *array = new double[nbPixels];

    nbPixels = nbBands * nbRows * nbCols;

    m_floatDataset.assign( nbPixels, 0.0f );

    bool *pBitMask = new bool[nbPixels];
    for( int i(0); i < nbPixels; ++i )
    {
        if( pAnatomy->at(i) < 0.01 )
        {
            pBitMask[i] = true;
        }
        else
        {
            pBitMask[i] = false;
        }
    }

    float dmax( 999999999.0f );

    // first pass
    for( b = 0; b < nbBands; ++b )
    {
        for( r = 0; r < nbRows; ++r )
        {
            for( c = 0; c < nbCols; ++c )
            {
                if ( pBitMask[b * nbRows * nbCols + r * nbCols + c] )
                {
                    continue;
                }

                pSrcPix = pBitMask + b * nbRows * nbCols + r * nbCols + c;
                cc1 = c;

                while( cc1 < nbCols && *pSrcPix++ == 0 )
                {
                    cc1++;
                }

                d1 = ( cc1 >= nbCols ? nbCols : ( cc1 - c ) );

                pSrcPix = pBitMask + b * nbRows * nbCols + r * nbCols + c;
                cc2 = c;

                while( cc2 >= 0 && *pSrcPix-- == 0 )
                {
                    cc2--;
                }

                d2 = ( cc2 <= 0 ? nbCols : ( c - cc2 ) );

                if( d1 <= d2 )
                {
                    d  = d1;
                    c0 = cc1;
                }
                else
                {
                    d  = d2;
                    c0 = cc2;
                }

                m_floatDataset[b * nbRows * nbCols + r * nbCols + c] = (float)( d * d );
            }
        }
    }

    // second pass
    for( b = 0; b < nbBands; b++ )
    {
        for( c = 0; c < nbCols; c++ )
        {
            for( r = 0; r < nbRows; r++ )
            {
                array[r] = (double)m_floatDataset[b * nbRows * nbCols + r * nbCols + c];
            }

            for( r = 0; r < nbRows; r++ )
            {
                if( pBitMask[b * nbRows * nbCols + r * nbCols + c] == 1 )
                    continue;

                dmin    = dmax;
                r0      = r;
                g       = sqrt(array[r]);
                istart  = r - (int) g;

                if( istart < 0 )
                    istart = 0;

                iend    = r + (int) g + 1;

                if ( iend >= nbRows )
                    iend = nbRows;

                for( rr = istart; rr < iend; rr++ )
                {
                    u = array[rr] + (r - rr) * (r - rr);
                    if( u < dmin )
                    {
                        dmin = u;
                        r0   = rr;
                    }
                }

                m_floatDataset[b * nbRows * nbCols + r * nbCols + c] = dmin;
            }
        }
    }

    // third pass

    for( r = 0; r < nbRows; r++ )
    {
        for( c = 0; c < nbCols; c++ )
        {
            for( b = 0; b < nbBands; b++ )
            {
                array[b] = (double) m_floatDataset[b * nbRows * nbCols + r * nbCols + c];
            }

            for( b = 0; b < nbBands; b++ )
            {
                if( pBitMask[b * nbRows * nbCols + r * nbCols + c] == 1 )
                    continue;

                dmin   = dmax;
                b0     = b;
                g      = sqrt(array[b]);
                istart = b - (int) g - 1;

                if( istart < 0 )
                    istart = 0;

                iend   = b + (int) g + 1;

                if( iend >= nbBands )
                    iend = nbBands;

                for( bb = istart; bb < iend; bb++ )
                {
                    u = array[bb] + (b - bb) * (b - bb);

                    if( u < dmin )
                    {
                        dmin = u;
                        b0   = bb;
                    }
                }
                m_floatDataset[b * nbRows * nbCols + r * nbCols + c] = dmin;
            }
        }
    }

    float max = 0;
    for( i = 0; i < nbPixels; ++i )
    {
        m_floatDataset[i] = sqrt( (double)m_floatDataset[i] );
        if( m_floatDataset[i] > max )
            max = m_floatDataset[i];
    }
    for( i = 0; i < nbPixels; ++i )
    {
        m_floatDataset[i] = m_floatDataset[i] / max;
    }

    // filter with gauss
    // create the filter kernel
    double sigma  = 4;

    int dim       = (int)( 3.0 * sigma + 1 );
    int n         = 2* dim + 1;
    double step   = 1;

    float* kernel = new float[n];

    double sum    = 0;
    double x      = -(float)dim;

    double uu;
    for( int i = 0; i < n; ++i )
    {
        uu        = xxgauss( x, sigma );
        sum       += uu;
        kernel[i] = uu;
        x         += step;
    }

    /* normalize */
    for( int i = 0; i < n; ++i )
    {
        uu        = kernel[i];
        uu        /= sum;
        kernel[i] = uu;
    }

    d = n / 2;
    float* float_pp;
    std::vector<float> tmp( nbPixels, 0.0f );
    int c1, cc;

    for( b = 0; b < nbBands; ++b )
    {
        for( r = 0; r < nbRows; ++r )
        {
            for( c = d; c < nbCols - d; ++c )
            {
                float_pp = kernel;
                sum      = 0;
                c0       = c - d;
                c1       = c + d;

                for( cc = c0; cc <= c1; cc++ )
                {
                    x = m_floatDataset[b * nbRows * nbCols + r * nbCols + cc];
                    sum += x * (*float_pp++);
                }
                tmp[b * nbRows * nbCols + r * nbCols + c] = sum;
            }
        }
    }
    int r1;
    for( b = 0; b < nbBands; ++b )
    {
        for( r = d; r < nbRows - d; ++r )
        {
            for( c = 0; c < nbCols; ++c )
            {
                float_pp = kernel;
                sum      = 0;
                r0       = r - d;
                r1       = r + d;

                for( rr = r0; rr <= r1; rr++ )
                {
                    x = tmp[b * nbRows * nbCols + rr * nbCols + c];
                    sum += x * (*float_pp++);
                }

                m_floatDataset[b * nbRows * nbCols + r * nbCols + c] = sum;
            }
        }
    }
    int b1;
    for( b = d; b < nbBands - d; ++b )
    {
        for( r = 0; r < nbRows; ++r )
        {
            for( c = 0; c < nbCols; ++c )
            {
                float_pp = kernel;
                sum      = 0;
                b0       = b - d;
                b1       = b + d;

                for( bb = b0; bb <= b1; bb++ )
                {
                    x   = m_floatDataset[bb * nbRows * nbCols + r * nbCols + c];
                    sum += x * (*float_pp++);
                }

                tmp[b * nbRows * nbCols + r * nbCols + c] = sum;
            }
        }
    }

    delete[] pBitMask;
    delete[] kernel;

    m_floatDataset = tmp;
}

//////////////////////////////////////////////////////////////////////////

double Anatomy::xxgauss( const double x, const double sigma )
{
    double y, z, a = 2.506628273;

    z = x / sigma;
    y = exp( (double) -z * z * 0.5 ) / ( sigma * a );

    return y;
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::dilateInternal( std::vector< bool > &workData, int curIndex )
{
    workData.at( curIndex - 1 )                              = true;
    workData.at( curIndex )                                  = true;
    workData.at( curIndex + 1 )                              = true;
    workData.at( curIndex - m_columns - 1 )                  = true;
    workData.at( curIndex - m_columns )                      = true;
    workData.at( curIndex - m_columns + 1 )                  = true;
    workData.at( curIndex + m_columns - 1 )                  = true;
    workData.at( curIndex + m_columns )                      = true;
    workData.at( curIndex + m_columns + 1 )                  = true;
    workData.at( curIndex - m_columns * m_rows - 1 )         = true;
    workData.at( curIndex - m_columns * m_rows )             = true;
    workData.at( curIndex - m_columns * m_rows + 1 )         = true;
    workData.at( curIndex + m_columns * m_rows - 1 )         = true;
    workData.at( curIndex + m_columns * m_rows )             = true;
    workData.at( curIndex + m_columns * m_rows + 1 )         = true;
    workData.at( curIndex - m_columns * m_rows - m_columns ) = true;
    workData.at( curIndex - m_columns * m_rows + m_columns ) = true;
    workData.at( curIndex + m_columns * m_rows - m_columns ) = true;
    workData.at( curIndex + m_columns * m_rows + m_columns ) = true;
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::erodeInternal( std::vector< bool > &workData, int curIndex )
{
    float acc  = m_floatDataset[curIndex - 1]
    + m_floatDataset[curIndex] + m_floatDataset[curIndex + 1]
    + m_floatDataset[curIndex - m_columns - 1]
    + m_floatDataset[curIndex - m_columns]
    + m_floatDataset[curIndex - m_columns + 1]
    + m_floatDataset[curIndex + m_columns - 1]
    + m_floatDataset[curIndex + m_columns]
    + m_floatDataset[curIndex + m_columns + 1]
    + m_floatDataset[curIndex - m_columns * m_rows - 1]
    + m_floatDataset[curIndex - m_columns * m_rows]
    + m_floatDataset[curIndex - m_columns * m_rows + 1]
    + m_floatDataset[curIndex + m_columns * m_rows - 1]
    + m_floatDataset[curIndex + m_columns * m_rows]
    + m_floatDataset[curIndex + m_columns * m_rows + 1]
    + m_floatDataset[curIndex - m_columns * m_rows - m_columns]
    + m_floatDataset[curIndex - m_columns * m_rows + m_columns]
    + m_floatDataset[curIndex + m_columns * m_rows - m_columns]
    + m_floatDataset[curIndex + m_columns * m_rows + m_columns];

    if( acc == 19.0 )
    {
        workData.at( curIndex ) = true;
    }
}

//////////////////////////////////////////////////////////////////////////

/************************************************************************/
/* Formula:
   h(i) =  (cdf(i) - cdfMin) / (R * C * F - n - cdfMin)
   where   - cdf is the cumulative distribution function
           - cdfMin is the lowest cdf value other than 0, with i >= m_lowerEqThreshold
           - R is the number of rows
           - C is the number of columns
           - F is the number of frames
           - n is the number of pixels ignored
*/
/************************************************************************/
void Anatomy::equalizeHistogram()
{
    Logger::getInstance()->print( wxT( "Anatomy::equalizeHistogram() Starting equalization..." ), LOGLEVEL_DEBUG );
    clock_t startTime( clock() );

    //TODO: Add support for RGB
    static const unsigned int GRAY_SCALE( 256 );
    unsigned int size( m_frames * m_rows * m_columns );

    if( 0 == size || 1 != m_bands )
    {
        Logger::getInstance()->print( wxT( "Anatomy::equalizeHistogram() Anatomy not supported" ), LOGLEVEL_WARNING );
        return;
    }

    m_currentLowerEqThreshold = m_lowerEqThreshold;
    m_currentUpperEqThreshold = m_upperEqThreshold;

    if( m_equalizedDataset.size() != size )
    {
        m_equalizedDataset.clear();
        m_equalizedDataset.resize( size, 0.0f );

        unsigned int pixelCount[GRAY_SCALE] = { 0 };

        for( unsigned int i( 0 ); i < size; ++i )
        {
            unsigned int pixelValue( static_cast< unsigned int >( m_floatDataset.at( i ) * ( GRAY_SCALE - 1 ) ) );

            if( pixelValue < GRAY_SCALE )
            {
                ++pixelCount[pixelValue];
            }
            else
            {
                Logger::getInstance()->print( wxT( "Anatomy::equalizeHistogram() pixel value out of range" ), LOGLEVEL_ERROR );
            }
        }

        unsigned int cdfCount( 0 );
        for( unsigned int i( 0 ); i < GRAY_SCALE; ++i )
        {
            cdfCount += pixelCount[i];
            m_cdf[i] = cdfCount;
        }
    }

    unsigned int currentCdf( 0 );
    unsigned int cdfMin( 0 );
    bool isCdfMinFound( false );
    float equalizedHistogram[GRAY_SCALE] = { 0 };

    // Eliminate background noise
    for ( unsigned int i( 0 ); i < m_lowerEqThreshold; ++i )
    {
        equalizedHistogram[i] = 1.0f / ( GRAY_SCALE - 1 );
    }

    unsigned int nbPixelsEliminated( m_cdf[m_lowerEqThreshold] + m_cdf[GRAY_SCALE - 1] - m_cdf[m_upperEqThreshold] );

    for( unsigned int i( m_lowerEqThreshold + 1 ); i <= m_upperEqThreshold; ++i )
    {
        currentCdf = m_cdf[i] - m_cdf[m_lowerEqThreshold];

        if( !isCdfMinFound && 0 != currentCdf )
        {
            cdfMin = currentCdf;
            isCdfMinFound = true;

            if( 0 == size - nbPixelsEliminated - cdfMin )
            {
                // Division by zero, cancel calculation
                Logger::getInstance()->print( wxT( "Anatomy::equalizeHistogram() division by zero" ), LOGLEVEL_ERROR );
                return;
            }
        }

        // Calculate the lookup table for equalized values
        if( isCdfMinFound )
        {
            float result = static_cast<double>( currentCdf - cdfMin ) / ( size - nbPixelsEliminated - cdfMin );
            equalizedHistogram[i] = result;
        }
    }

    for ( unsigned int i( m_upperEqThreshold + 1 ); i < GRAY_SCALE; ++i )
    {
        equalizedHistogram[i] = 1.0f;
    }

    // Calculate the equalized frame
    for( unsigned int i( 0 ); i < size; ++i )
    {
        m_equalizedDataset[i] = equalizedHistogram[static_cast< unsigned int >( m_floatDataset.at( i ) * ( GRAY_SCALE - 1 ) )];
    }

    clock_t endTime( clock() );

    ostringstream oss;
    oss << "Anatomy::equalizeHistogram() took ";
    oss << static_cast<float>(endTime - startTime) / CLOCKS_PER_SEC;
    oss << " seconds."; 
    Logger::getInstance()->print( wxString( oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
}

//////////////////////////////////////////////////////////////////////////
void Anatomy::writeVoxel( const int x, const int y, const int z, const int layer, const int size, const bool isRound, const bool draw3d, wxColor colorRGB )
{
    SubTextureBox l_stb = getStrokeBox(x, y, z, layer, size, draw3d);

    switch( m_type )
    {
        case HEAD_BYTE:
        case HEAD_SHORT:
        case OVERLAY:
        {
            if(colorRGB == wxColor(0,0,0)) // erase
            {
                float transparent = 0.0f;
                updateTexture(l_stb, isRound, transparent);
            }
            else // draw, always white (or always purple for an overlay)
            {
                float white = 1.0f;
                updateTexture(l_stb, isRound, white);
            }
            break;
        }
        case RGB: //draw in color
        {
            updateTexture(l_stb, isRound, colorRGB);
            break;
        }
        case VECTORS:
        {
            break;
        }
    }
}


SubTextureBox Anatomy::getStrokeBox( const int x, const int y, const int z, const int layer, const int size, const bool draw3d )
{
    SubTextureBox box;

    //set dimensions of the box
    box.width = size+1;
    box.height = size+1;
    box.depth = size+1;
    if(!draw3d)
    {
        switch(layer)
        {
        case AXIAL:
            box.depth = 1;
            break;
        case CORONAL:
            box.height = 1;
            break;
        case SAGITTAL:
            box.width = 1;
            break;
        default:
            break;
        }
    }

    //set position of the box
    box.x = std::min( std::max( x-box.width / 2, 0 ),  m_columns-box.width );
    box.y = std::min( std::max( y-box.height / 2, 0 ), m_rows-box.height );
    box.z = std::min( std::max( z-box.depth / 2, 0 ),  m_frames-box.depth );

    return box;
}

void Anatomy::generateTexture()
{
    glPixelStorei  ( GL_UNPACK_ALIGNMENT, 1 );
    Logger::getInstance()->printIfGLError( wxT( "Anatomy::generateTexture - glPixelStorei") );
    glGenTextures  ( 1, &m_GLuint );
    Logger::getInstance()->printIfGLError( wxT( "Anatomy::generateTexture - glGenTextures") );
    glBindTexture  ( GL_TEXTURE_3D, m_GLuint );
    Logger::getInstance()->printIfGLError( wxT( "Anatomy::generateTexture - glBindTexture") );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP );
    Logger::getInstance()->printIfGLError( wxT( "Anatomy::generateTexture - glTexParameteri") );

    switch( m_type )
    {
        case HEAD_BYTE:
        case HEAD_SHORT:
        case OVERLAY:
            glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, m_columns, m_rows, m_frames, 0, GL_LUMINANCE, GL_FLOAT, m_useEqualizedDataset ? &m_equalizedDataset[0] : &m_floatDataset[0] );
            break;

        case RGB:
            glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, m_columns, m_rows, m_frames, 0, GL_RGB, GL_FLOAT, m_useEqualizedDataset ? &m_equalizedDataset[0] : &m_floatDataset[0] );
            break;

        case VECTORS:
            break;

        // The code to generate a texture from Tensors is not implemented yet, basicly this means that 
        // the data inside the m_floatDataset is not set properly.
        case TENSOR_FIELD:
            break;

        default:
            break;
    }
    Logger::getInstance()->printIfGLError( wxT( "Anatomy::generateTexture - glTexImage3D") );
}

//////////////////////////////////////////////////////////////////////////

Anatomy::~Anatomy()
{
    Logger::getInstance()->print( wxT( "Executing Anatomy destructor..." ), LOGLEVEL_DEBUG );
    const GLuint* tex = &m_GLuint;
    glDeleteTextures( 1, tex );
    Logger::getInstance()->printIfGLError( wxT( "Anatomy::~Anatomy - glDeleteTextures") );

    if( m_pRoi )
    {
        m_pRoi->m_sourceAnatomy = NULL;
    }

    Logger::getInstance()->print( wxT( "Anatomy destructor done." ), LOGLEVEL_DEBUG );
}

void Anatomy::pushHistory()
{
    m_drawHistory.push(stack<SubTextureBox>());
}

void Anatomy::fillHistory( const SubTextureBox drawZone, bool isRGB) 
{
    //push current subtexture's properties
    m_drawHistory.top().push(drawZone);
    //init buffer size
    m_drawHistory.top().top().data.resize(drawZone.datasize);

    //fill with texture values
    for( int z = 0; z < drawZone.depth; ++z )
    {
        for( int y = 0; y < drawZone.height; ++y )
        {
            for( int x = 0; x < drawZone.width; ++x )
            {
                int sourceIndex = (x +drawZone.x) + (y + drawZone.y) * m_columns + (z + drawZone.z) * m_columns * m_rows;
                int subIndex = x + y * drawZone.width + z * drawZone.width * drawZone.height;

                if(isRGB)
                {
                    m_drawHistory.top().top().data[subIndex*3] = m_floatDataset[sourceIndex*3];
                    m_drawHistory.top().top().data[subIndex*3 + 1] = m_floatDataset[sourceIndex*3 + 1];
                    m_drawHistory.top().top().data[subIndex*3 + 2] = m_floatDataset[sourceIndex*3 + 2];
                }
                else
                {
                    m_drawHistory.top().top().data[subIndex] = m_floatDataset[sourceIndex];
                }
            }
        }
    }
}

void Anatomy::popHistory(bool isRGB)
{
    if(m_drawHistory.empty())
    {
        return;
    }

    //restore the data from top of history
    while( !m_drawHistory.top().empty() )
    {
        //the top contains a list of every subtextures made in one click,
        //we need to get them back from top to bottom
        SubTextureBox* topPtr = &(m_drawHistory.top().top());

        for( int z = 0; z < topPtr->depth; ++z )
        {
            for( int y = 0; y < topPtr->height; ++y )
            {
                for( int x = 0; x < topPtr->width; ++x )
                {
                    int sourceIndex = (x+topPtr->x) + (y+topPtr->y) * m_columns + (z+topPtr->z) * m_columns * m_rows;
                    int subIndex = x + y * topPtr->width + z * topPtr->width * topPtr->height;

                    if(isRGB)
                    {
                        m_floatDataset[sourceIndex*3] = topPtr->data[subIndex*3];
                        m_floatDataset[sourceIndex*3 + 1] = topPtr->data[subIndex*3 + 1];
                        m_floatDataset[sourceIndex*3 + 2] = topPtr->data[subIndex*3 + 2];
                    }
                    else
                    {
                        m_floatDataset[sourceIndex] = topPtr->data[subIndex];
                    }
                }
            }
        }
        //restore texture with the data
        glBindTexture(GL_TEXTURE_3D, m_GLuint);    //The texture we created already
        Logger::getInstance()->printIfGLError( wxT( "Anatomy::popHistory - glBindTexture") );
        if(isRGB)
        {
            glTexSubImage3D( GL_TEXTURE_3D, 0, topPtr->x, topPtr->y, topPtr->z, topPtr->width, topPtr->height, topPtr->depth, GL_RGB, GL_FLOAT, &(topPtr->data[0]) );
        }
        else
        {
            glTexSubImage3D( GL_TEXTURE_3D, 0, topPtr->x, topPtr->y, topPtr->z, topPtr->width, topPtr->height, topPtr->depth, GL_LUMINANCE, GL_FLOAT, &(topPtr->data[0]) );
        }
        Logger::getInstance()->printIfGLError( wxT( "Anatomy::popHistory - glTexSubImage3D") );
        //discard this subtexture
        topPtr = NULL;
        m_drawHistory.top().pop();
    }

    //discard top of history
    m_drawHistory.pop();
}
