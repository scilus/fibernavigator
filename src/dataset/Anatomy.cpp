/*
 * Anatomy.cpp
 *
 *  Created on: 07.07.2008
 *      Author: ralph
 */

#include "Anatomy.h"
#include "fibers.h"

#include "../gui/mainFrame.h"
#include "../gui/SelectionObject.h"
#include "../misc/nifti/nifti1_io.h"

#include <wx/textfile.h>
#include <GL/glew.h>
#include <cassert>

#define MIN_HEADER_SIZE 348
#define NII_HEADER_SIZE 352

Anatomy::Anatomy( DatasetHelper* i_datasetHelper ) : 
    DatasetInfo ( i_datasetHelper )
{
    m_roi         = 0;
    m_tensorField = 0;
    m_bands       = 1;
    m_dataType    = 2;
}

Anatomy::Anatomy( DatasetHelper* i_datasetHelper, std::vector< float >* i_dataset, int sample) : 
    DatasetInfo( i_datasetHelper )
{
    m_columns       = m_dh->m_columns;
    m_frames        = m_dh->m_frames;
    m_isLoaded      = true;   
    m_roi           = 0;
    m_rows          = m_dh->m_rows;
    m_tensorField   = 0;
    m_type          = HEAD_BYTE;
	
	m_floatDataset.resize( m_columns * m_frames * m_rows);
	for(unsigned int i = 0; i < m_floatDataset.size(); ++i )
        {
			m_floatDataset[i]       = i_dataset->at(i);
        }

}

Anatomy::Anatomy( DatasetHelper* i_datasetHelper, std::vector< float >* i_dataset) : 
    DatasetInfo( i_datasetHelper )
{
    m_columns       = m_dh->m_columns;
    m_frames        = m_dh->m_frames;
    m_bands         = 1;
    m_isLoaded      = true;   
    m_roi           = 0;
    m_rows          = m_dh->m_rows;
    m_tensorField   = 0;
    m_type          = HEAD_BYTE;
    m_dataType      = 2;
    createOffset( i_dataset );
}

Anatomy::Anatomy(DatasetHelper* datasetHelper, int type)
:DatasetInfo(datasetHelper)
{
    if(type == RGB)
    {
        m_columns       = m_dh->m_columns;
        m_frames        = m_dh->m_frames;
        m_isLoaded      = true;   
        m_roi           = 0;
        m_rows          = m_dh->m_rows;
        m_tensorField   = 0;
        m_type          = type;
        m_bands         = 3;
        m_dataType      = 2;

        m_floatDataset.resize( m_columns * m_frames * m_rows * 3 );

        for(unsigned int i = 0; i < m_floatDataset.size(); ++i )
        {
            m_floatDataset[i]       = 0;
        }
    }
    else
    {
        assert(false);
    }
}

Anatomy::~Anatomy()
{
    const GLuint* tex = &m_GLuint;
    glDeleteTextures( 1, tex );

    if( m_roi )
    {
        m_roi->m_sourceAnatomy = NULL;
    }

    m_dh->updateLoadStatus();
}

bool Anatomy::load( wxString i_fileName )
{
    m_fullPath = i_fileName;
#ifdef __WXMSW__
    m_name = i_fileName.AfterLast( '\\' );
#else
    m_name = i_fileName.AfterLast( '/' );
#endif
    // test for nifti
    if ( m_name.AfterLast( '.' ) == _T( "nii" ) )
    {
        //printf( "detected nifti file\n" );
        return loadNifti( i_fileName );
    }
    else if ( m_name.AfterLast( '.' ) == _T( "gz" ) )
    {
        //printf( "checking for compressed nifti file\n" );
        wxString l_tmpName = m_name.BeforeLast( '.' );
        if ( l_tmpName.AfterLast( '.' ) == _T( "nii" ) )
        {
            //printf( "found compressed nifti file\n" );
            return loadNifti( i_fileName );
        }
    }

    return false;
}

bool Anatomy::loadNifti( wxString i_fileName )
{
    char* l_hdrFile;
    l_hdrFile = (char*)malloc( i_fileName.length() + 1 );
    strcpy( l_hdrFile, (const char*)i_fileName.mb_str( wxConvUTF8 ) );

    nifti_image* l_image = nifti_image_read( l_hdrFile, 0 );
    if( ! l_image )
    {
        m_dh->m_lastError = wxT( "nifti file corrupt, cannot create nifti image from header" );
        return false;
    }
#ifdef DEBUG
    //nifti_1_header *l_tmphdr = nifti_read_header( l_hdrFile, 0, 0 );
    //disp_nifti_1_header( "", l_tmphdr );
#endif
    m_columns   = l_image->dim[1]; 
    m_rows      = l_image->dim[2]; 
    m_frames    = l_image->dim[3]; 
    m_bands     = l_image->dim[4];
    m_dataType = l_image->datatype;

    if( m_dh->m_anatomyLoaded )
    {
        if( m_rows != m_dh->m_rows || m_columns != m_dh->m_columns || m_frames != m_dh->m_frames )
        {
            m_dh->m_lastError = wxT( "dimensions of loaded files must be the same" );
            return false;
        }
    }

    m_dh->m_xVoxel = l_image->dx;
    m_dh->m_yVoxel = l_image->dy;
    m_dh->m_zVoxel = l_image->dz;
    if( l_image->datatype == 2 )
    {
        if( l_image->dim[4] == 1 )
        {
            m_type = HEAD_BYTE;
        }
        else if( l_image->dim[4] == 3 )
        {
            m_type = RGB;
        }
        else
            m_type = BOT_INITIALIZED;
    }
    else if( l_image->datatype == 4 )
        m_type = HEAD_SHORT;

    else if( l_image->datatype == 16 )
    {
        if( l_image->dim[4] == 3 )
        {
            m_type = VECTORS;
        }
        else
            m_type = OVERLAY;
    }
    else
        m_type = BOT_INITIALIZED;

    nifti_image* l_fileData = nifti_image_read( l_hdrFile, 1 );
    if( ! l_fileData )
    {
        m_dh->m_lastError = wxT( "nifti file corrupt" );
        return false;
    }
    int l_nSize = l_image->dim[1] * l_image->dim[2] * l_image->dim[3];

    bool l_flag = false;

    switch( m_type )
    {
        case HEAD_BYTE:
        {
            unsigned char* l_data = (unsigned char*)l_fileData->data;
            m_floatDataset.resize( l_nSize );

            for( int i = 0; i < l_nSize; ++i )
            {
                m_floatDataset[i] = (float)l_data[i] / 255.0;
            }

            l_flag = true;
            m_oldMax = 255;            
        }
            break;

        case HEAD_SHORT:
        {
            short int* l_data = (short int*)l_fileData->data;
            int l_max = 0;
            std::vector<int> l_histo( 65536, 0 );

            for( int i = 0; i < l_nSize; ++i )
            {
                l_max = wxMax(l_max, l_data[i]);
                ++l_histo[l_data[i]];
            }

            int l_fivePercent   = (int)( l_nSize * 0.001 );
            int l_newMax        = 65535;
            int l_adder         = 0;

            for( int i = 65535; i > 0; --i )
            {
                l_adder += l_histo[i];
                l_newMax = i;

                if( l_adder > l_fivePercent )
                    break;
            }
            for( int i = 0; i < l_nSize; ++i )
            {
                if ( l_data[i] > l_newMax )
                    l_data[i] = l_newMax;
            }

            m_floatDataset.resize ( l_nSize );

            for( int i = 0; i < l_nSize; ++i )
            {
                m_floatDataset[i] = (float)l_data[i] / (float)l_newMax;
            }

            m_oldMax    = l_max;
            m_newMax    = l_newMax;
            l_flag      = true;
        }
            break;

        case OVERLAY:
        {
            float* l_data = (float*)l_fileData->data;

            m_floatDataset.resize( l_nSize );
            for( int i = 0; i < l_nSize; ++i )
            {
                m_floatDataset[i] = (float)l_data[i];
            }

            float l_max = 0.0;
            for( int i = 0; i < l_nSize; ++i )
            {
                if (m_floatDataset[i] > l_max)
                    l_max = m_floatDataset[i];
            }

            for( int i = 0; i < l_nSize; ++i )
            {
                m_floatDataset[i] = m_floatDataset[i] / l_max;
            }

            m_oldMax    = l_max;
            m_newMax    = 1.0;
            l_flag      = true;
        }
            break;

        case RGB:
        {
            unsigned char* l_data = (unsigned char*)l_fileData->data;

            m_floatDataset.resize( l_nSize * 3 );

            for( int i = 0; i < l_nSize; ++i )
            {
                m_floatDataset[i * 3]       = (float)l_data[i]  / 255.0f;
                m_floatDataset[i * 3 + 1]   = (float)l_data[l_nSize + i] / 255.0f;
                m_floatDataset[i * 3 + 2]   = (float)l_data[(2 * l_nSize) + i] / 255.0f;
            }

            l_flag = true;
        }
            break;

        case VECTORS:
        {
            float* l_data = (float*)l_fileData->data;
            m_floatDataset.resize( l_nSize * 3 );

            for( int i = 0; i < l_nSize; ++i )
            {
                m_floatDataset[i * 3]       = l_data[i];
                m_floatDataset[i * 3 + 1]   = l_data[l_nSize + i];
                m_floatDataset[i * 3 + 2]   = l_data[(2 * l_nSize) + i];
            }

            m_tensorField              = new TensorField( m_dh, &m_floatDataset, 1, 3 );
            m_dh->m_tensorsFieldLoaded = true;
            m_dh->m_vectorsLoaded      = true;
            m_dh->m_surfaceIsDirty     = true;
            l_flag                     = true;
        }
            break;

        default:
            m_dh->m_lastError = wxT( "unsuported file format" );
            return false;
            break;
    }

    if( l_flag )
    {
        m_dh->m_rows            = m_rows;
        m_dh->m_columns         = m_columns;
        m_dh->m_frames          = m_frames;
        m_dh->m_anatomyLoaded   = true;
    } 

    m_isLoaded = l_flag;

    return l_flag;
}

void Anatomy::saveNifti( wxString i_fileName )
{    
    int dims[] = { 4, m_columns, m_rows, m_frames, m_bands, 0, 0, 0 };
    nifti_image* l_image;
    l_image = nifti_make_new_nim( dims, m_dataType, 1 );
    
    char l_fn[1024];
    strcpy( l_fn, (const char*)i_fileName.mb_str( wxConvUTF8 ) );

    l_image->qform_code  = 1;    
    l_image->datatype = m_dataType;
    l_image->fname  = l_fn;
    l_image->dx = m_dh->m_xVoxel;
    l_image->dy = m_dh->m_yVoxel;
    l_image->dz = m_dh->m_zVoxel;

    if(m_type==HEAD_BYTE)
    {
        vector<unsigned char> tmp(m_floatDataset.size());
        for(unsigned int i=0;i<m_floatDataset.size();i++)
            tmp[i] = m_floatDataset[i]*255;
        l_image->data   = &tmp[0];
        nifti_image_write( l_image );
    }
    else if (m_type==HEAD_SHORT)
    {
        vector<short> tmp(m_floatDataset.size());
        for(unsigned int i=0;i<m_floatDataset.size();i++)
            tmp[i] = (short)(m_floatDataset[i]*m_newMax);
        l_image->data   = &tmp[0];
        nifti_image_write( l_image );
    }
    else if (m_type==RGB)
    {
        vector<unsigned char> tmp(m_floatDataset.size());
        int l_nSize = m_floatDataset.size()/3;
        for( int i = 0; i < l_nSize; ++i )
        {
            tmp[i]              = m_floatDataset[i * 3]     * 255.0f;
            tmp[l_nSize + i]    = m_floatDataset[i * 3 + 1] * 255.0f;
            tmp[2*l_nSize + i]  = m_floatDataset[i * 3 + 2] * 255.0f;
        }
        l_image->data   = &tmp[0];
        nifti_image_write( l_image );
    }
    else
    {
        l_image->data   = &m_floatDataset[0];
        nifti_image_write( l_image );
    }

}

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

GLuint Anatomy::getGLuint()
{
    if( ! m_GLuint )
        generateTexture();

    return m_GLuint;
}

void Anatomy::createOffset( std::vector<float>* i_source )
{
    int b, r, c, bb, rr, r0, b0, c0;
    int i, istart, iend;
    int l_nbBands, l_nbRows, l_nbCols, l_nbPixels;
    int d, d1, d2, cc1, cc2;
    float u, dmin, dmax;
    bool* l_srcpix;
    double g, *array;

    l_nbBands  = m_frames;
    l_nbRows   = m_rows;
    l_nbCols   = m_columns;

    l_nbPixels = wxMax( l_nbBands, l_nbRows );
    array = new double[l_nbPixels];

    l_nbPixels = l_nbBands * l_nbRows * l_nbCols;

    m_floatDataset.resize( l_nbPixels );
    for( int i = 0; i < l_nbPixels; ++i )
    {
        m_floatDataset[i] = 0.0;
    }

    bool* l_bitMask = new bool[l_nbPixels];
    for( int i = 0; i < l_nbPixels; ++i )
    {
        if( i_source->at(i) < 0.01 )
            l_bitMask[i] = true;
        else
            l_bitMask[i] = false;
    }

    dmax = 999999999.0f;

    // first pass
    for( b = 0; b < l_nbBands; ++b )
    {
        for( r = 0; r < l_nbRows; ++r )
        {
            for( c = 0; c < l_nbCols; ++c )
            {
                //if (VPixel(src,b,r,c,VBit) == 1)
                if ( l_bitMask[b * l_nbRows * l_nbCols + r * l_nbCols + c] )
                {
                    //m_floatDataset[b * l_nbRows * l_nbCols + r * l_nbCols + c] = 0;
                    continue;
                }

                l_srcpix = l_bitMask + b * l_nbRows * l_nbCols + r * l_nbCols + c;
                cc1 = c;

                while( cc1 < l_nbCols && *l_srcpix++ == 0 )
                    cc1++;

                d1 = ( cc1 >= l_nbCols ? l_nbCols : ( cc1 - c ) );

                l_srcpix = l_bitMask + b * l_nbRows * l_nbCols + r * l_nbCols + c;
                cc2 = c;

                while( cc2 >= 0 && *l_srcpix-- == 0 )
                    cc2--;

                d2 = ( cc2 <= 0 ? l_nbCols : ( c - cc2 ) );

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
                m_floatDataset[b * l_nbRows * l_nbCols + r * l_nbCols + c] = (float)( d * d );
            }
        }
    }

    // second pass
    for( b = 0; b < l_nbBands; b++ )
    {
        for( c = 0; c < l_nbCols; c++ )
        {
            for( r = 0; r < l_nbRows; r++ )
                array[r] = (double)m_floatDataset[b * l_nbRows * l_nbCols + r * l_nbCols + c];

            for( r = 0; r < l_nbRows; r++ )
            {
                if( l_bitMask[b * l_nbRows * l_nbCols + r * l_nbCols + c] == 1 )
                    continue;

                dmin    = dmax;
                r0      = r;
                g       = sqrt(array[r]);
                istart  = r - (int) g;

                if( istart < 0 )
                    istart = 0;

                iend    = r + (int) g + 1;

                if ( iend >= l_nbRows )
                    iend = l_nbRows;

                for( rr = istart; rr < iend; rr++ )
                {
                    u = array[rr] + (r - rr) * (r - rr);
                    if( u < dmin )
                    {
                        dmin = u;
                        r0   = rr;
                    }
                }
                m_floatDataset[b * l_nbRows * l_nbCols + r * l_nbCols + c] = dmin;
            }
        }
    }

    // third pass

    for( r = 0; r < l_nbRows; r++ )
    {
        for( c = 0; c < l_nbCols; c++ )
        {
            for( b = 0; b < l_nbBands; b++ )
                array[b] = (double) m_floatDataset[b * l_nbRows * l_nbCols + r * l_nbCols + c];

            for( b = 0; b < l_nbBands; b++ )
            {
                if( l_bitMask[b * l_nbRows * l_nbCols + r * l_nbCols + c] == 1 )
                    continue;

                dmin   = dmax;
                b0     = b;
                g      = sqrt(array[b]);
                istart = b - (int) g - 1;

                if( istart < 0 )
                    istart = 0;

                iend   = b + (int) g + 1;

                if( iend >= l_nbBands )
                    iend = l_nbBands;

                for( bb = istart; bb < iend; bb++ )
                {
                    u = array[bb] + (b - bb) * (b - bb);

                    if( u < dmin )
                    {
                        dmin = u;
                        b0   = bb;
                    }
                }
                m_floatDataset[b * l_nbRows * l_nbCols + r * l_nbCols + c] = dmin;
            }
        }
    }

    //delete[] array;

    float max = 0;
    for( i = 0; i < l_nbPixels; ++i )
    {
        m_floatDataset[i] = sqrt( (double)m_floatDataset[i] );
        if( m_floatDataset[i] > max )
            max = m_floatDataset[i];
    }
    for( i = 0; i < l_nbPixels; ++i )
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
    std::vector<float> tmp( l_nbPixels );
    int c1, cc;

    for( int i = 0; i < l_nbPixels; ++i )
    {
        tmp[i] = 0.0;
    }

    for( b = 0; b < l_nbBands; ++b )
    {
        for( r = 0; r < l_nbRows; ++r )
        {
            for( c = d; c < l_nbCols - d; ++c )
            {
                float_pp = kernel;
                sum      = 0;
                c0       = c - d;
                c1       = c + d;

                for( cc = c0; cc <= c1; cc++ )
                {
                    x = m_floatDataset[b * l_nbRows * l_nbCols + r * l_nbCols + cc];
                    sum += x * (*float_pp++);
                }
                tmp[b * l_nbRows * l_nbCols + r * l_nbCols + c] = sum;
            }
        }
    }
    int r1;
    for( b = 0; b < l_nbBands; ++b )
    {
        for( r = d; r < l_nbRows - d; ++r )
        {
            for( c = 0; c < l_nbCols; ++c )
            {
                float_pp = kernel;
                sum      = 0;
                r0       = r - d;
                r1       = r + d;

                for( rr = r0; rr <= r1; rr++ )
                {
                    x = tmp[b * l_nbRows * l_nbCols + rr * l_nbCols + c];
                    sum += x * (*float_pp++);
                }

                m_floatDataset[b * l_nbRows * l_nbCols + r * l_nbCols + c] = sum;
            }
        }
    }
    int b1;
    for( b = d; b < l_nbBands - d; ++b )
    {
        for( r = 0; r < l_nbRows; ++r )
        {
            for( c = 0; c < l_nbCols; ++c )
            {
                float_pp = kernel;
                sum      = 0;
                b0       = b - d;
                b1       = b + d;

                for( bb = b0; bb <= b1; bb++ )
                {
                    x   = m_floatDataset[bb * l_nbRows * l_nbCols + r * l_nbCols + c];
                    sum += x * (*float_pp++);
                }

                tmp[b * l_nbRows * l_nbCols + r * l_nbCols + c] = sum;
            }
        }
    }

    delete[] l_bitMask;
    delete[] kernel;

    m_floatDataset = tmp;
}

double Anatomy::xxgauss( double i_x, double i_sigma )
{
    double l_y, l_z, l_a = 2.506628273;

    l_z = i_x / i_sigma;
    l_y = exp( (double) -l_z * l_z * 0.5 ) / ( i_sigma * l_a );

    return l_y;
}

void Anatomy::setZero( int i_x, int i_y, int i_z )
{
    m_columns = i_x;
    m_rows    = i_y;
    m_frames  = i_z;
    m_bands   = 1;

    int l_nSize = m_rows * m_columns * m_frames;

    m_floatDataset.clear();
    m_floatDataset.resize( l_nSize );

    for( int i = 0; i < l_nSize; ++i )
    {
        m_floatDataset[i] = 0.0;
    }
}

void Anatomy::setRGBZero( int i_x, int i_y, int i_z )
{
    m_columns = i_x;
    m_rows    = i_y;
    m_frames  = i_z;
    m_bands   = 3;

    int l_nSize = m_rows * m_columns * m_frames;

    m_floatDataset.clear();
    m_floatDataset.resize( l_nSize * m_bands);

    for (int i = 0; i < l_nSize * m_bands; ++i)
    {
        m_floatDataset[i] = 0.0;
    }
    m_dataType = 2;
    m_type = RGB;
}

void Anatomy::minimize()
{
    if( ! m_dh->m_fibersLoaded )
        return;

    std::vector<bool> l_tmp( m_columns * m_rows * m_frames, false );
    Fibers* l_fiber = NULL;
    m_dh->getFiberDataset( l_fiber );

    int l_x, l_y, l_z, l_index;

    for( int i = 0; i < l_fiber->getLineCount(); ++i )
    {
        if( l_fiber->isSelected( i ) )
        {
            for( int j = l_fiber->getStartIndexForLine(i); j < ( l_fiber->getStartIndexForLine( i ) + ( l_fiber->getPointsPerLine( i )) ); )
            {
                l_x = wxMin( m_dh->m_columns - 1, wxMax( 0, (int) l_fiber->getPointValue( j*3 )/m_dh->m_xVoxel ) );
                l_y = wxMin( m_dh->m_rows    - 1, wxMax( 0, (int) l_fiber->getPointValue( j*3 + 1)/m_dh->m_yVoxel ) );
                l_z = wxMin( m_dh->m_frames  - 1, wxMax( 0, (int) l_fiber->getPointValue( j*3 + 2) /m_dh->m_zVoxel) );

                l_index = l_x + l_y * m_dh->m_columns + l_z * m_dh->m_rows * m_dh->m_columns;
                l_tmp[l_index] = true;
                j += 3;
            }
        }
    }

    Anatomy* l_newAnatomy = new Anatomy( m_dh );
    l_newAnatomy->setZero( m_columns, m_rows, m_frames );

    std::vector<float>* l_dst = l_newAnatomy->getFloatDataset();

    for( int i = 0; i < m_columns * m_rows * m_frames; ++i )
    {
        if( l_tmp[i] && m_floatDataset[i] > 0 )
            l_dst->at( i ) = 1.0;
    }

    l_newAnatomy->setName( getName() + _T( "(minimal)" ) );
    l_newAnatomy->setType( HEAD_BYTE );
    l_newAnatomy->setDataType( 2 );

    m_dh->m_mainFrame->m_listCtrl->InsertItem( 0, wxT( "" ), 0 );
    m_dh->m_mainFrame->m_listCtrl->SetItem( 0, 1, l_newAnatomy->getName() );
    m_dh->m_mainFrame->m_listCtrl->SetItem( 0, 2, wxT( "0.00") );
    m_dh->m_mainFrame->m_listCtrl->SetItem( 0, 3, wxT( ""), 1 );
    m_dh->m_mainFrame->m_listCtrl->SetItemData( 0, (long)l_newAnatomy );
    m_dh->m_mainFrame->m_listCtrl->SetItemState( 0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
}

void Anatomy::dilate()
{
    int l_nSize = m_columns * m_rows * m_frames;
    std::vector<bool> l_tmp( l_nSize, false );
    int l_index;

    for( int c = 1; c < m_columns - 1; ++c )
    {
        for( int r = 1; r < m_rows - 1; ++r )
        {
            for( int f = 1; f < m_frames - 1; ++f )
            {
                l_index = c + r * m_columns + f * m_columns * m_rows;
                if( m_floatDataset[l_index] == 1.0 )
                    dilate1( &l_tmp, l_index );
            }
        }
    }
    for( int i = 0; i < l_nSize; ++i )
    {
        if ( l_tmp[i] )
            m_floatDataset[i] = 1.0;
    }

    const GLuint* l_tex = &m_GLuint;
    glDeleteTextures( 1, l_tex );
    generateTexture();
}

void Anatomy::dilate1( std::vector<bool>* i_input, int i_index )
{
    i_input->at( i_index - 1 )                              = true;
    i_input->at( i_index )                                  = true;
    i_input->at( i_index + 1 )                              = true;
    i_input->at( i_index - m_columns - 1 )                  = true;
    i_input->at( i_index - m_columns )                      = true;
    i_input->at( i_index - m_columns + 1 )                  = true;
    i_input->at( i_index + m_columns - 1 )                  = true;
    i_input->at( i_index + m_columns )                      = true;
    i_input->at( i_index + m_columns + 1 )                  = true;
    i_input->at( i_index - m_columns * m_rows - 1 )         = true;
    i_input->at( i_index - m_columns * m_rows )             = true;
    i_input->at( i_index - m_columns * m_rows + 1 )         = true;
    i_input->at( i_index + m_columns * m_rows - 1 )         = true;
    i_input->at( i_index + m_columns * m_rows )             = true;
    i_input->at( i_index + m_columns * m_rows + 1 )         = true;
    i_input->at( i_index - m_columns * m_rows - m_columns ) = true;
    i_input->at( i_index - m_columns * m_rows + m_columns ) = true;
    i_input->at( i_index + m_columns * m_rows - m_columns ) = true;
    i_input->at( i_index + m_columns * m_rows + m_columns ) = true;
}

void Anatomy::erode()
{
    int l_nsize = m_columns * m_rows * m_frames;
    std::vector<bool> l_tmp( l_nsize, false );
    int l_index;

    for( int c = 1; c < m_columns - 1; ++c )
    {
        for( int r = 1; r < m_rows - 1; ++r )
        {
            for( int f = 1; f < m_frames - 1; ++f )
            {
                l_index = c + r * m_columns + f * m_columns * m_rows;
                if( m_floatDataset[l_index] == 1.0 )
                    erode1( &l_tmp, l_index );
            }
        }
    }
    for( int i = 0; i < l_nsize; ++i )
    {
        if ( ! l_tmp[i] )
            m_floatDataset[i] = 0.0;
    }
    const GLuint* l_tex = &m_GLuint;
    glDeleteTextures( 1, l_tex );
    generateTexture();
}

void Anatomy::erode1( std::vector< bool >* i_tmp, int i_index )
{
    float test = m_floatDataset[i_index - 1] + m_floatDataset[i_index] + m_floatDataset[i_index + 1]
               + m_floatDataset[i_index - m_columns - 1] + m_floatDataset[i_index - m_columns]
               + m_floatDataset[i_index - m_columns + 1] + m_floatDataset[i_index + m_columns - 1]
               + m_floatDataset[i_index + m_columns] + m_floatDataset[i_index + m_columns + 1]
               + m_floatDataset[i_index - m_columns * m_rows - 1] + m_floatDataset[i_index - m_columns * m_rows]
               + m_floatDataset[i_index - m_columns * m_rows + 1] + m_floatDataset[i_index + m_columns * m_rows - 1]
               + m_floatDataset[i_index + m_columns * m_rows] + m_floatDataset[i_index + m_columns * m_rows + 1]
               + m_floatDataset[i_index - m_columns * m_rows - m_columns] + m_floatDataset[i_index - m_columns
               * m_rows + m_columns] + m_floatDataset[i_index + m_columns * m_rows - m_columns]
               + m_floatDataset[i_index + m_columns * m_rows + m_columns];

    if( test == 19.0 )
        i_tmp->at( i_index ) = 1.0;
}

std::vector< float >* Anatomy::getFloatDataset()
{
    return &m_floatDataset;
}


float Anatomy::at( int l_i )
{
    return m_floatDataset[l_i];
}

TensorField* Anatomy::getTensorField()
{
    return m_tensorField;
}

void Anatomy::createPropertiesSizer(MainFrame *parent)
{
    DatasetInfo::createPropertiesSizer(parent);  
    wxSizer *l_sizer;
    m_pbtnDilate = new wxButton(parent, wxID_ANY, wxT("Dilate"),wxDefaultPosition, wxSize(85,-1));
    m_pbtnErode  = new wxButton(parent, wxID_ANY, wxT("Erode"),wxDefaultPosition, wxSize(85,-1));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(m_pbtnDilate,0,wxALIGN_CENTER);
    l_sizer->Add(m_pbtnErode,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_pbtnDilate->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnDilateDataset));
    parent->Connect(m_pbtnErode->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnErodeDataset));

    m_pbtnCut = new wxButton(parent, wxID_ANY, wxT("Cut (boxes)"), wxDefaultPosition, wxSize(85,-1));
    m_pbtnMinimize = new wxButton(parent, wxID_ANY, wxT("Minimize (fibers)"), wxDefaultPosition, wxSize(85,-1));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(m_pbtnCut,0,wxALIGN_CENTER);
    l_sizer->Add(m_pbtnMinimize,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_pbtnMinimize->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnMinimizeDataset));
    parent->Connect(m_pbtnCut->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnListMenuCutOut));

    m_pbtnNewDistanceMap = new wxButton(parent, wxID_ANY, wxT("New Distance Map"), wxDefaultPosition, wxSize(140,-1));
    m_pbtnNewIsoSurface  = new wxButton(parent, wxID_ANY, wxT("New Iso Surface"), wxDefaultPosition, wxSize(140,-1));
    m_pbtnNewOffsetSurface = new wxButton(parent, wxID_ANY, wxT("New Offset Surface"), wxDefaultPosition, wxSize(140,-1));
    m_pbtnNewVOI = new wxButton(parent, wxID_ANY, wxT("New VOI"), wxDefaultPosition, wxSize(140,-1));
    m_propertiesSizer->Add(m_pbtnNewDistanceMap,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(m_pbtnNewIsoSurface,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(m_pbtnNewOffsetSurface,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(m_pbtnNewVOI,0,wxALIGN_CENTER);
    parent->Connect(m_pbtnNewIsoSurface->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnNewIsoSurface));
    parent->Connect(m_pbtnNewDistanceMap->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnNewDistanceMap));
    parent->Connect(m_pbtnNewOffsetSurface->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnNewOffsetSurface));
    parent->Connect(m_pbtnNewVOI->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnNewVoiFromOverlay));
	
	m_ptoggleSegment = new wxToggleButton(parent, wxID_ANY,wxT("Segment mode"),wxDefaultPosition, wxSize(140,-1));
	l_sizer = new wxBoxSizer(wxHORIZONTAL);
	l_sizer->Add(m_ptoggleSegment,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
	parent->Connect(m_ptoggleSegment->GetId(),wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnSegment));
	
	l_sizer = new wxBoxSizer(wxHORIZONTAL);
	m_pradiobtnFlood = new wxRadioButton(parent, wxID_ANY, _T( "Click region" ), wxDefaultPosition, wxSize(80,-1));
	l_sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Floodfill   "),wxDefaultPosition, wxSize(50,-1), wxALIGN_RIGHT),0,wxALIGN_CENTER);
	l_sizer->Add(m_pradiobtnFlood);
	m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
	parent->Connect(m_pradiobtnFlood->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(MainFrame::OnFloodFill));

	m_psliderFlood = new MySlider(parent, wxID_ANY,0,0,100, wxDefaultPosition, wxSize(80,-1), wxSL_HORIZONTAL | wxSL_AUTOTICKS);
    m_psliderFlood->SetValue(40);
	setFloodThreshold(0.2f);
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
	l_sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Threshold "),wxDefaultPosition, wxSize(60,-1), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(m_psliderFlood,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_psliderFlood->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(MainFrame::OnSliderFloodMoved));

	l_sizer = new wxBoxSizer(wxHORIZONTAL);
	m_pradiobtnObj = new wxRadioButton(parent, wxID_ANY, _T( "Select Class 1" ), wxDefaultPosition, wxSize(85,-1));
	l_sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Graphcut   "),wxDefaultPosition, wxSize(55,-1), wxALIGN_RIGHT),0,wxALIGN_CENTER);
	l_sizer->Add(m_pradiobtnObj);
	m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
	parent->Connect(m_pradiobtnObj->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(MainFrame::OnSelectObj));
	
	l_sizer = new wxBoxSizer(wxHORIZONTAL);
	m_pradiobtnBck = new wxRadioButton(parent, wxID_ANY, _T( "Select Class 2" ), wxDefaultPosition, wxSize(85,-1));
	l_sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Graphcut   "),wxDefaultPosition, wxSize(55,-1), wxALIGN_RIGHT),0,wxALIGN_CENTER);
	l_sizer->Add(m_pradiobtnBck);
	m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
	parent->Connect(m_pradiobtnBck->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(MainFrame::OnSelectBck));

	m_pbtnGraphCut = new wxButton(parent, wxID_ANY, wxT("Generate Graphcut"), wxDefaultPosition, wxSize(120,-1));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(m_pbtnGraphCut,0,wxALIGN_CENTER);
	m_pbtnGraphCut->Enable(m_dh->graphcutReady());
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_pbtnGraphCut->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnbtnGraphCut));

	m_pbtnKmeans = new wxButton(parent, wxID_ANY, wxT("K-Means"), wxDefaultPosition, wxSize(132,-1));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(m_pbtnKmeans,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_pbtnKmeans->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnKmeans));

	

}

void Anatomy::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();
    m_pbtnMinimize->Enable(m_dh->m_fibersLoaded);   
    m_pbtnCut->Enable(m_dh->getSelectionObjects().size()>0);
    m_pbtnNewIsoSurface->Enable(getType() <= OVERLAY);
    m_pbtnNewDistanceMap->Enable(getType() <= OVERLAY);
    m_pbtnNewOffsetSurface->Enable(getType() <= OVERLAY);
    m_pbtnNewVOI->Enable(getType() <= OVERLAY);
    m_pbtnMinimize->Enable(getType() <= OVERLAY);
    m_pbtnCut->Enable(getType() <= OVERLAY);
	m_ptoggleSegment->SetValue(m_dh->m_isSegmentActive);
	m_pbtnGraphCut->Enable(m_dh->graphcutReady());
}

