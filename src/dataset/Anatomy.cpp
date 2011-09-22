/*
 *  The Anatomy class implementation.
 *
 */

#include "Anatomy.h"
#include "Fibers.h"

#include "../gui/MainFrame.h"
#include "../gui/SelectionObject.h"
#include "../misc/nifti/nifti1_io.h"

#include <wx/textfile.h>
#include <GL/glew.h>
#include <cassert>

#define MIN_HEADER_SIZE 348
#define NII_HEADER_SIZE 352

Anatomy::Anatomy( DatasetHelper* pDatasetHelper ) 
: DatasetInfo ( pDatasetHelper ),
  m_isSegmentOn( false ),  
  m_pRoi( NULL ),
  m_dataType( 2 ),
  m_pTensorField( NULL )
{
    m_bands = 1;
}

Anatomy::Anatomy( DatasetHelper* pDatasetHelper, 
                 std::vector< float >* pDataset)
                 : DatasetInfo( pDatasetHelper ),
                 m_isSegmentOn( false ),
                 m_pRoi( NULL ),
                 m_dataType( 2 ),
                 m_pTensorField( NULL )
{
    m_columns       = m_dh->m_columns;
    m_frames        = m_dh->m_frames;
    m_rows          = m_dh->m_rows;
    m_bands         = 1;
    m_isLoaded      = true;   
    m_type          = HEAD_BYTE;

    createOffset( *pDataset );
    equalizeHistogram();
}

Anatomy::Anatomy( DatasetHelper* pDatasetHelper, 
                  std::vector< float >* pDataset, 
                  const int sample ) 
: DatasetInfo( pDatasetHelper ),
  m_isSegmentOn( false ),
  m_pRoi( NULL ),
  m_dataType( 2 ),
  m_pTensorField( NULL )
{
    m_columns = m_dh->m_columns;
    m_frames  = m_dh->m_frames;
    m_rows    = m_dh->m_rows;
    m_bands   = 1;
    
    m_type    = HEAD_BYTE;

    m_isLoaded = true;   
    
    m_floatDataset.resize( m_columns * m_frames * m_rows );
    
    for( unsigned int i(0); i < m_floatDataset.size(); ++i )
    {
        m_floatDataset[i] = pDataset->at(i);
    }
    equalizeHistogram();
}

Anatomy::Anatomy( DatasetHelper* pDatasetHelper, 
                  const int type )
: DatasetInfo( pDatasetHelper ),
  m_isSegmentOn( false ),
  m_pRoi( NULL ),
  m_dataType( 2 ),
  m_pTensorField( NULL )
{
    if(type == RGB)
    {
        m_columns       = m_dh->m_columns;
        m_frames        = m_dh->m_frames;
        m_rows          = m_dh->m_rows;
        m_bands         = 3;
        m_isLoaded      = true;   
        m_type          = type;

        m_floatDataset.resize( m_columns * m_frames * m_rows * 3, 0.0f );

//         for(unsigned int i = 0; i < m_floatDataset.size(); ++i )
//         {
//             m_floatDataset[i] = 0;
//         }
        equalizeHistogram();
    }
    else
    {
        assert(false);
    }
}

//////////////////////////////////////////////////////////////////////////

float Anatomy::at( const int pos )
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
    if( ! m_GLuint )
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

    /*for( int i(0); i < datasetSize; ++i )
    {
        m_floatDataset[i] = 0.0f;
    }*/
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

//     for ( int i(0); i < datasetSize * m_bands; ++i )
//     {
//         m_floatDataset[i] = 0.0f;
//     }

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
                if( m_floatDataset[curIndex] == 1.0 )
                {
                    dilateInternal( tmp, curIndex );
                }
            }
        }
    }

    for( int i(0); i < datasetSize; ++i )
    {
        if ( tmp[i] )
            m_floatDataset[i] = 1.0;
    }

    const GLuint* pTexId = &m_GLuint;
    glDeleteTextures( 1, pTexId );
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
                if( m_floatDataset[curIndex] == 1.0 )
                    erodeInternal( tmp, curIndex );
            }
        }
    }

    for( int i(0); i < datasetSize; ++i )
    {
        if ( !tmp[i] )
            m_floatDataset[i] = 0.0;
    }

    const GLuint* pTexId = &m_GLuint;
    glDeleteTextures( 1, pTexId );
    generateTexture();
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::minimize()
{
    if( ! m_dh->m_fibersLoaded )
    {
        return;
    }

    std::vector<bool> workData( m_columns * m_rows * m_frames, false );
    Fibers* pFibers( NULL );
    m_dh->getFiberDataset( pFibers );

    int curX, curY, curZ, index;

    for( int i(0); i < pFibers->getLineCount(); ++i )
    {
        if( pFibers->isSelected( i ) )
        {
            for( int j = pFibers->getStartIndexForLine( i ); 
                j < ( pFibers->getStartIndexForLine( i ) + ( pFibers->getPointsPerLine( i )) ); )
            {
                curX = wxMin( m_dh->m_columns - 1, wxMax( 0, (int) pFibers->getPointValue( j * 3 ) / m_dh->m_xVoxel ) );
                curY = wxMin( m_dh->m_rows    - 1, wxMax( 0, (int) pFibers->getPointValue( j * 3 + 1) / m_dh->m_yVoxel ) );
                curZ = wxMin( m_dh->m_frames  - 1, wxMax( 0, (int) pFibers->getPointValue( j * 3 + 2) / m_dh->m_zVoxel) );

                index = curX + curY * m_dh->m_columns + curZ * m_dh->m_rows * m_dh->m_columns;
                workData[index] = true;
                j += 3;
            }
        }
    }

    Anatomy* pNewAnatomy = new Anatomy( m_dh );
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

    m_dh->m_mainFrame->m_pListCtrl->InsertItem( 0, wxT( "" ), 0 );
    m_dh->m_mainFrame->m_pListCtrl->SetItem( 0, 1, pNewAnatomy->getName() );
    m_dh->m_mainFrame->m_pListCtrl->SetItem( 0, 2, wxT( "0.00") );
    m_dh->m_mainFrame->m_pListCtrl->SetItem( 0, 3, wxT( ""), 1 );
    m_dh->m_mainFrame->m_pListCtrl->SetItemData( 0, (long)pNewAnatomy );
    m_dh->m_mainFrame->m_pListCtrl->SetItemState( 0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
}

//////////////////////////////////////////////////////////////////////////

bool Anatomy::load( wxString fileName )
{
    m_fullPath = fileName;
#ifdef __WXMSW__
    m_name = fileName.AfterLast( '\\' );
#else
    m_name = fileName.AfterLast( '/' );
#endif
    
    // test for nifti
    if ( m_name.AfterLast( '.' ) == _T( "nii" ) )
    {
        //printf( "detected nifti file\n" );
        return loadNifti( fileName );
    }
    else if ( m_name.AfterLast( '.' ) == _T( "gz" ) )
    {
        //printf( "checking for compressed nifti file\n" );
        wxString tmpName = m_name.BeforeLast( '.' );
        if ( tmpName.AfterLast( '.' ) == _T( "nii" ) )
        {
            //printf( "found compressed nifti file\n" );
            return loadNifti( fileName );
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////

bool Anatomy::loadNifti( wxString fileName )
{
    char *pHdrFile;
    pHdrFile = (char*)malloc( fileName.length() + 1 );
    strcpy( pHdrFile, (const char*)fileName.mb_str( wxConvUTF8 ) );

    nifti_image *pImage = nifti_image_read( pHdrFile, 0 );
    if( ! pImage )
    {
        m_dh->m_lastError = wxT( "nifti file corrupt, cannot create nifti image from header" );
        return false;
    }
#ifdef DEBUG
    //nifti_1_header *l_tmphdr = nifti_read_header( l_hdrFile, 0, 0 );
    //disp_nifti_1_header( "", l_tmphdr );
#endif
    m_columns   = pImage->dim[1]; 
    m_rows      = pImage->dim[2]; 
    m_frames    = pImage->dim[3]; 
    m_bands     = pImage->dim[4];
    m_dataType  = pImage->datatype;

    if( m_dh->m_anatomyLoaded )
    {
        if( m_rows != m_dh->m_rows || m_columns != m_dh->m_columns || m_frames != m_dh->m_frames )
        {
            m_dh->m_lastError = wxT( "dimensions of loaded files must be the same" );
            return false;
        }
    }

    m_dh->m_xVoxel = pImage->dx;
    m_dh->m_yVoxel = pImage->dy;
    m_dh->m_zVoxel = pImage->dz;
    
    if( pImage->datatype == 2 )
    {
        if( pImage->dim[4] == 1 )
        {
            m_type = HEAD_BYTE;
        }
        else if( pImage->dim[4] == 3 )
        {
            m_type = RGB;
        }
        else
        {
            m_type = BOT_INITIALIZED;
        }
    }
    else if( pImage->datatype == 4 )
    {
        m_type = HEAD_SHORT;
    }

    else if( pImage->datatype == 16 )
    {
        if( pImage->dim[4] == 3 )
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

    nifti_image *pFileData = nifti_image_read( pHdrFile, 1 );
    if( !pFileData )
    {
        m_dh->m_lastError = wxT( "nifti file corrupt" );
        return false;
    }
    
    int datasetSize = pImage->dim[1] * pImage->dim[2] * pImage->dim[3];

    bool flag = false;

    switch( m_type )
    {
        case HEAD_BYTE:
        {
            unsigned char* pData = (unsigned char*)pFileData->data;
            m_floatDataset.resize( datasetSize );

            for( int i(0); i < datasetSize; ++i )
            {
                m_floatDataset[i] = (float)pData[i] / 255.0;
            }

            flag = true;
            m_oldMax = 255;            
        }
        break;

        case HEAD_SHORT:
        {
            short int* pData = (short int*)pFileData->data;
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

            m_floatDataset.resize ( datasetSize );

            for( int i(0); i < datasetSize; ++i )
            {
                m_floatDataset[i] = (float)pData[i] / (float)newMax;
            }

            m_oldMax    = dataMax;
            m_newMax    = newMax;
            flag        = true;
        }
        break;

        case OVERLAY:
        {
            float* pData = (float*)pFileData->data;

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
        }
        break;

        case RGB:
        {
            unsigned char* pData = (unsigned char*)pFileData->data;

            m_floatDataset.resize( datasetSize * 3 );

            for( int i(0); i < datasetSize; ++i )
            {
                m_floatDataset[i * 3]       = (float)pData[i]  / 255.0f;
                m_floatDataset[i * 3 + 1]   = (float)pData[datasetSize + i] / 255.0f;
                m_floatDataset[i * 3 + 2]   = (float)pData[(2 * datasetSize) + i] / 255.0f;
            }

            flag = true;
        }
        break;

        case VECTORS:
        {
            float* pData = (float*)pFileData->data;
            m_floatDataset.resize( datasetSize * 3 );

            for( int i(0); i < datasetSize; ++i )
            {
                m_floatDataset[i * 3]       = pData[i];
                m_floatDataset[i * 3 + 1]   = pData[datasetSize + i];
                m_floatDataset[i * 3 + 2]   = pData[(2 * datasetSize) + i];
            }

            m_pTensorField             = new TensorField( m_dh, &m_floatDataset, 1, 3 );
            m_dh->m_tensorsFieldLoaded = true;
            m_dh->m_vectorsLoaded      = true;
            m_dh->m_surfaceIsDirty     = true;
            flag                       = true;
        }
        break;

        default:
        {
            m_dh->m_lastError = wxT( "unsuported file format" );
            flag = false;
            // Will not return now to make sure the pHdrFile pointer is freed.
        }
    }

    if( flag )
    {
        m_dh->m_rows            = m_rows;
        m_dh->m_columns         = m_columns;
        m_dh->m_frames          = m_frames;
        m_dh->m_anatomyLoaded   = true;

        equalizeHistogram();
    }
    
    free(pHdrFile);
    pHdrFile = NULL;

    m_isLoaded = flag;

    return flag;
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::saveNifti( wxString fileName )
{    
    int dims[] = { 4, m_columns, m_rows, m_frames, m_bands, 0, 0, 0 };
    nifti_image* pImage(NULL);
    pImage = nifti_make_new_nim( dims, m_dataType, 1 );
    
    char fn[1024];
    strcpy( fn, (const char*)fileName.mb_str( wxConvUTF8 ) );

    pImage->qform_code = 1;    
    pImage->datatype   = m_dataType;
    pImage->fname = fn;
    pImage->dx = m_dh->m_xVoxel;
    pImage->dy = m_dh->m_yVoxel;
    pImage->dz = m_dh->m_zVoxel;

    if( m_type == HEAD_BYTE )
    {
        vector<unsigned char> tmp( m_floatDataset.size() );
        for(unsigned int i(0); i < m_floatDataset.size(); ++i )
        {
            tmp[i] = m_floatDataset[i] * 255;
        }
        
        // Do not move the call to nifti_image_write out of the 
        // if, because it will crash, since the temp vector will
        // not exist anymore, and pImage->data will point to garbage.
        pImage->data = &tmp[0];
        nifti_image_write( pImage );
    }
    else if( m_type == HEAD_SHORT )
    {
        vector<short> tmp( m_floatDataset.size() );
        for(unsigned int i(0); i < m_floatDataset.size(); ++i )
        {
            tmp[i] = (short)(m_floatDataset[i] * m_newMax);
        }
        
        // Do not move the call to nifti_image_write out of the 
        // if, because it will crash, since the temp vector will
        // not exist anymore, and pImage->data will point to garbage.
        pImage->data = &tmp[0];
        nifti_image_write( pImage );
    }
    else if( m_type == RGB )
    {
        vector<unsigned char> tmp( m_floatDataset.size() );
        int datasetSize = m_floatDataset.size()/3;
        for( int i(0); i < datasetSize; ++i )
        {
            tmp[i]                   = m_floatDataset[i * 3]     * 255.0f;
            tmp[datasetSize + i]     = m_floatDataset[i * 3 + 1] * 255.0f;
            tmp[2 * datasetSize + i] = m_floatDataset[i * 3 + 2] * 255.0f;
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
        pImage->data = &m_floatDataset[0];
        nifti_image_write( pImage );
    }
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::createPropertiesSizer( PropertiesWindow *pParentWindow )
{
    DatasetInfo::createPropertiesSizer(pParentWindow);  
    
    m_pBtnDilate = new wxButton(pParentWindow, wxID_ANY, wxT("Dilate"),wxDefaultPosition, wxSize(85,-1));
    m_pBtnErode  = new wxButton(pParentWindow, wxID_ANY, wxT("Erode"),wxDefaultPosition, wxSize(85,-1));
    
    wxSizer *pSizer;
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( m_pBtnDilate, 0, wxALIGN_CENTER );
    pSizer->Add( m_pBtnErode,  0, wxALIGN_CENTER );
    
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );

    pParentWindow->Connect( m_pBtnDilate->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnDilateDataset ) );
    pParentWindow->Connect( m_pBtnErode->GetId(),  wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnErodeDataset ) );

    m_pBtnCut =      new wxButton( pParentWindow, wxID_ANY, wxT("Cut (boxes)"), wxDefaultPosition, wxSize(85, -1) );
    m_pBtnMinimize = new wxButton( pParentWindow, wxID_ANY, wxT("Minimize (fibers)"), wxDefaultPosition, wxSize(85, -1) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( m_pBtnCut,      0, wxALIGN_CENTER );
    pSizer->Add( m_pBtnMinimize, 0, wxALIGN_CENTER );

    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );

    pParentWindow->Connect( m_pBtnMinimize->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnMinimizeDataset ) );
    pParentWindow->Connect( m_pBtnCut->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnListItemCutOut ) );

    m_pBtnNewDistanceMap =   new wxButton( pParentWindow, wxID_ANY, wxT("New Distance Map"),   wxDefaultPosition, wxSize(140, -1) );
    m_pBtnNewIsoSurface  =   new wxButton( pParentWindow, wxID_ANY, wxT("New Iso Surface"),    wxDefaultPosition, wxSize(140, -1) );
    m_pBtnNewOffsetSurface = new wxButton( pParentWindow, wxID_ANY, wxT("New Offset Surface"), wxDefaultPosition, wxSize(140, -1) );
    m_pBtnNewVOI =           new wxButton( pParentWindow, wxID_ANY, wxT("New VOI"),            wxDefaultPosition, wxSize(140, -1) );

    m_propertiesSizer->Add( m_pBtnNewDistanceMap,   0, wxALIGN_CENTER );
    m_propertiesSizer->Add( m_pBtnNewIsoSurface,    0, wxALIGN_CENTER );
    m_propertiesSizer->Add( m_pBtnNewOffsetSurface, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( m_pBtnNewVOI,           0, wxALIGN_CENTER );

    pParentWindow->Connect( m_pBtnNewIsoSurface->GetId(),    wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnNewIsoSurface) );
    pParentWindow->Connect( m_pBtnNewDistanceMap->GetId(),   wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnNewDistanceMap) );
    pParentWindow->Connect( m_pBtnNewOffsetSurface->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnNewOffsetSurface) );
    pParentWindow->Connect( m_pBtnNewVOI->GetId(),           wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnNewVoiFromOverlay) );
    
    m_pToggleSegment = new wxToggleButton( pParentWindow, wxID_ANY,wxT("Floodfill"), wxDefaultPosition, wxSize(140, -1) );

    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( m_pToggleSegment, 0, wxALIGN_CENTER );

    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );

    pParentWindow->Connect( m_pToggleSegment->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnFloodFill) );

    m_pSliderFlood = new MySlider( pParentWindow, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxSize(100, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderFlood->SetValue( 40 );
    setFloodThreshold( 0.2f );

    m_pTxtThresBox = new wxTextCtrl( pParentWindow, wxID_ANY, wxT("0.20"),       wxDefaultPosition, wxSize(40, -1), wxTE_CENTRE | wxTE_READONLY );
    m_pTextThres = new wxStaticText( pParentWindow, wxID_ANY, wxT("Threshold "), wxDefaultPosition, wxSize(60, -1), wxALIGN_RIGHT );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( m_pTextThres,   0, wxALIGN_CENTER );
    pSizer->Add( m_pSliderFlood, 0, wxALIGN_CENTER );
    pSizer->Add( m_pTxtThresBox, 0, wxALIGN_CENTER );

    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );

    pParentWindow->Connect( m_pSliderFlood->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(PropertiesWindow::OnSliderFloodMoved) );
    
    // The following interface objects are related to flood fill and graph cuts.
    // They are kept here temporiraly, but will need to be implemented or removed.
    // Please also note that the coding standard has not been applied to these lines, 
    // so please apply it if you re enable them.
    /*pSizer = new wxBoxSizer(wxHORIZONTAL);
    m_pRadioBtnFlood = new wxRadioButton(pParentWindow, wxID_ANY, _T( "Click region" ), wxDefaultPosition, wxSize(80,-1));
    pSizer->Add(new wxStaticText(pParentWindow, wxID_ANY, wxT("Floodfill   "),wxDefaultPosition, wxSize(50,-1), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    pSizer->Add(m_pRadioBtnFlood);
    m_propertiesSizer->Add(pSizer,0,wxALIGN_CENTER);
    pParentWindow->Connect(m_pRadioBtnFlood->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(PropertiesWindow::OnFloodFill));*/

    /*pSizer = new wxBoxSizer(wxHORIZONTAL);
    m_pRadioBtnObj = new wxRadioButton(pParentWindow, wxID_ANY, _T( "Select Class 1" ), wxDefaultPosition, wxSize(85,-1));
    pSizer->Add(new wxStaticText(pParentWindow, wxID_ANY, wxT("Graphcut   "),wxDefaultPosition, wxSize(55,-1), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    pSizer->Add(m_pRadioBtnObj);
    m_propertiesSizer->Add(pSizer,0,wxALIGN_CENTER);
    pParentWindow->Connect(m_pRadioBtnObj->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(PropertiesWindow::OnSelectObj));
    
    pSizer = new wxBoxSizer(wxHORIZONTAL);
    m_pRadioBtnBck = new wxRadioButton(pParentWindow, wxID_ANY, _T( "Select Class 2" ), wxDefaultPosition, wxSize(85,-1));
    pSizer->Add(new wxStaticText(pParentWindow, wxID_ANY, wxT("Graphcut   "),wxDefaultPosition, wxSize(55,-1), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    pSizer->Add(m_pRadioBtnBck);
    m_propertiesSizer->Add(pSizer,0,wxALIGN_CENTER);
    pParentWindow->Connect(m_pRadioBtnBck->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(PropertiesWindow::OnSelectBck));

    m_pSliderGraphSigma = new MySlider(pParentWindow, wxID_ANY,0,0,500, wxDefaultPosition, wxSize(80,-1), wxSL_HORIZONTAL | wxSL_AUTOTICKS);
    m_pSliderGraphSigma->SetValue(200);
    setGraphSigma(200.0f);
    pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(new wxStaticText(pParentWindow, wxID_ANY, wxT("Sigma "),wxDefaultPosition, wxSize(60,-1), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    pSizer->Add(m_pSliderGraphSigma,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(pSizer,0,wxALIGN_CENTER);
    pParentWindow->Connect(m_pSliderGraphSigma->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(PropertiesWindow::OnSliderGraphSigmaMoved));

    m_pBtnGraphCut = new wxButton(pParentWindow, wxID_ANY, wxT("Generate Graphcut"), wxDefaultPosition, wxSize(120,-1));
    pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_pBtnGraphCut,0,wxALIGN_CENTER);
    m_pBtnGraphCut->Enable(m_dh->graphcutReady());
    m_propertiesSizer->Add(pSizer,0,wxALIGN_CENTER);
    pParentWindow->Connect(m_pBtnGraphCut->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnbtnGraphCut));*/

    /*m_pBtnKmeans = new wxButton(pParentWindow, wxID_ANY, wxT("K-Means"), wxDefaultPosition, wxSize(132,-1));
    pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_pBtnKmeans,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(pSizer,0,wxALIGN_CENTER);
    pParentWindow->Connect(m_pBtnKmeans->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnKmeans));*/
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();
    
    m_pBtnMinimize->Enable( m_dh->m_fibersLoaded );
    m_pBtnCut->Enable(      m_dh->getSelectionObjects().size() > 0 );

    m_pBtnNewIsoSurface->Enable(    getType() <= OVERLAY );
    m_pBtnNewDistanceMap->Enable(   getType() <= OVERLAY );
    m_pBtnNewOffsetSurface->Enable( getType() <= OVERLAY );

    m_pBtnNewVOI->Enable(   getType() <= OVERLAY );
 
    //m_pBtnGraphCut->Enable( m_dh->graphcutReady() );
    
    if(!m_isSegmentOn)
    {
        m_pTextThres->Hide();
        m_pSliderFlood->Hide();
        m_pTxtThresBox->Hide();
    }
    else
    {
        m_pTextThres->Show();
        m_pSliderFlood->Show();
        m_pTxtThresBox->Show();
    }
    
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::createOffset( const std::vector<float> &sourceDataset )
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
        if( sourceDataset.at(i) < 0.01 )
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
    float acc  = m_floatDataset[curIndex - 1] + m_floatDataset[curIndex] + m_floatDataset[curIndex + 1]
    + m_floatDataset[curIndex - m_columns - 1] + m_floatDataset[curIndex - m_columns]
    + m_floatDataset[curIndex - m_columns + 1] + m_floatDataset[curIndex + m_columns - 1]
    + m_floatDataset[curIndex + m_columns] + m_floatDataset[curIndex + m_columns + 1]
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
        workData.at( curIndex ) = 1.0;
    }
}

//////////////////////////////////////////////////////////////////////////

/************************************************************************/
/* Formula:
   h(i) = round[ (cdf(i) - cdfMin) / (M * N - cdfMin) * (L - 1) ]
   where   cdf is the cumulative distribution function
           cdfMin is the lowest cdf value other than 0
           M is the number of rows
           N is the number of columns
           L is the total number of gray levels (typically 256)
*/
/************************************************************************/
void Anatomy::equalizeHistogram()
{
    //TODO: Check format for gray scale value
    //TODO: Add support for RGB
    static const unsigned int GRAY_SCALE(256);
    unsigned int size(m_rows * m_columns);

    m_equalizedDataset.resize(m_floatDataset.size(), 0.0f);

    if(0 == size)
    {
        return;
    }

    for(unsigned int frame(0); frame < static_cast<unsigned int>(m_frames); ++frame)
    {
        bool isCdfMinFound(false);
        unsigned int currentCdf(0);
        unsigned int prevCdf(0);
        unsigned int cdfMin(0);
        unsigned int pixelCount[GRAY_SCALE]  = { 0 };
        float equalizedHistogram[GRAY_SCALE] = { 0 }; // Index is the original dataset gray value

        for(unsigned int i(frame * size); i < (frame + 1) * size; ++i)
        {
            unsigned int pixelValue(static_cast<unsigned int>(m_floatDataset.at(i) * (GRAY_SCALE - 1)));

            if(pixelValue >= GRAY_SCALE)
            {
                // log error?
                return;
            }

            // TODO: Check if this is valid
            pixelCount[pixelValue]++;
        }

        // Calculate cdf and equalized histogram
        for(unsigned int i(0); i < GRAY_SCALE; ++i)
        {
            currentCdf = prevCdf + pixelCount[i];
            
            if(!isCdfMinFound && 0 != currentCdf)
            {
                cdfMin = currentCdf;
                isCdfMinFound = true;
   
                if (0 == size - cdfMin)
                {
                    // Division by zero, cancel calculation
                    // Log error
                    return;
                }
            }

            // Calculate the lookup table for equalized values
            if (isCdfMinFound)
            {
                // Since our dataset is normalized, we can strip the round and the * (L - 1)
                float result = static_cast<double>(currentCdf - cdfMin) / (size - cdfMin);
                equalizedHistogram[i] = result;
            }

            prevCdf = currentCdf;
        }

        // Calculate the equalized frame
        for(unsigned int i(frame * size); i < (frame + 1) * size; ++i)
        {
            m_equalizedDataset[i] = equalizedHistogram[static_cast<unsigned int>(m_floatDataset.at(i) * (GRAY_SCALE - 1))];
        }
    }
}

//////////////////////////////////////////////////////////////////////////

void Anatomy::generateTexture()
{
    glPixelStorei  ( GL_UNPACK_ALIGNMENT, 1 );
    glGenTextures  ( 1, &m_GLuint );
    glBindTexture  ( GL_TEXTURE_3D, m_GLuint );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP );

    switch( m_type )
    {
        case HEAD_BYTE:
        case HEAD_SHORT:
        case OVERLAY:
            glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, m_columns, m_rows, m_frames, 0, GL_LUMINANCE, GL_FLOAT, &m_floatDataset[0] );
            break;

        case RGB:
            glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, m_columns, m_rows, m_frames, 0, GL_RGB, GL_FLOAT, &m_floatDataset[0] );
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
}

//////////////////////////////////////////////////////////////////////////

Anatomy::~Anatomy()
{
    const GLuint* tex = &m_GLuint;
    glDeleteTextures( 1, tex );

    if( m_pRoi )
    {
        m_pRoi->m_sourceAnatomy = NULL;
    }

    m_dh->updateLoadStatus();
}
