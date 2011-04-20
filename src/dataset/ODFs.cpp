/////////////////////////////////////////////////////////////////////////////
// Name:            ODFs.cpp
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   10/07/2009
//
// Description: This is the implementation file for ODFs class.
//
// Last modifications:
//      by : GGirard - 03/12/2010
/////////////////////////////////////////////////////////////////////////////

#include "ODFs.h"

#include <algorithm>
#include <GL/glew.h>
#include <math.h>

#include "../misc/nifti/nifti1_io.h"
#include "../misc/Fantom/FMatrix.h"

// m_sh_basis
// 0: Original Descoteaux et al RR 5768 basis (default in dmri)
// 1: Descoteaux PhD thesis basis definition
// 2: Tournier
///////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////
ODFs::ODFs( DatasetHelper* i_datasetHelper ) :
    Glyph            ( i_datasetHelper ), 
    m_order          ( 0    ),
    m_radiusAttribLoc( 0    ),
    m_radiusBuffer   ( NULL ),
    m_sh_basis       ( 0 )
{
    m_scalingFactor = 0.0f;

    // Generating hemispheres
    generateSpherePoints( m_scalingFactor );
}

ODFs::~ODFs()
{
    if( m_radiusBuffer )
    {
        glDeleteBuffers( 1, m_radiusBuffer );
        delete [] m_radiusBuffer;        
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will load a ODFs type Nifty file.
//
// i_fileName       : The name of the file to load.
///////////////////////////////////////////////////////////////////////////
bool ODFs::load( wxString i_fileName )
{
    m_fullPath = i_fileName;

#ifdef __WXMSW__
    m_name = i_fileName.AfterLast( '\\' );
#else
    m_name = i_fileName.AfterLast( '/' );
#endif

    return loadNifti( i_fileName );
}

///////////////////////////////////////////////////////////////////////////
// This function will load a ODFs type Nifty file.
//
// i_fileName       : The name of the file to load.
// Returns true if succesfull, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool ODFs::loadNifti( wxString i_fileName )
{
    char* l_hdrFile;
    l_hdrFile = (char*)malloc( i_fileName.length() + 1 );
    strcpy( l_hdrFile, (const char*)i_fileName.mb_str( wxConvUTF8 ) );

    nifti_image* l_image = nifti_image_read( l_hdrFile, 0 );

    if( ! l_image )
    {
        DatasetInfo::m_dh->m_lastError = wxT( "nifti file corrupt, cannot create nifti image from header" );
        return false;
    }

    m_datasetHelper.m_columns = l_image->dim[1]; //80
    m_datasetHelper.m_rows    = l_image->dim[2]; //1
    m_datasetHelper.m_frames  = l_image->dim[3]; //72
    m_bands                   = l_image->dim[4];

    m_datasetHelper.m_xVoxel = l_image->dx;
    m_datasetHelper.m_yVoxel = l_image->dy;
    m_datasetHelper.m_zVoxel = l_image->dz;

    m_type = ODFS;

    // Order has to be between 0 and 16 (0, 2, 4, 6, 8, 10, 12, 14, 16)
    if( l_image->datatype != 16  || !( m_bands == 0   || m_bands == 15 || m_bands == 28 || 
                                       m_bands == 45  || m_bands == 66 || m_bands == 91 || 
                                       m_bands == 120 || m_bands == 153 ) )
    {
        DatasetInfo::m_dh->m_lastError = wxT( "not a valid ODFs file format" );
        return false;
    }    

    nifti_image* l_fileData = nifti_image_read( l_hdrFile, 1 );
    if( ! l_fileData )
    {
        DatasetInfo::m_dh->m_lastError = wxT( "nifti file corrupt" );
        return false;
    }

    int l_nSize = l_image->dim[1] * l_image->dim[2] * l_image->dim[3];

    float* l_data = (float*)l_fileData->data;
    
    std::vector< float > l_fileFloatData;
    l_fileFloatData.resize( l_nSize * m_bands );

    // We need to do a bit of moving around with the data in order to have it like we want.
    for( int i = 0; i < l_nSize; ++i )
        for( int j = 0; j < m_bands; ++j )
            l_fileFloatData[i * m_bands + j] = l_data[(j * l_nSize) + i];


    // Once the file has been read succesfully, we need to create the structure 
    // that will contain all the sphere points representing the ODFs.
    createStructure( l_fileFloatData );

    m_isLoaded = true;

    return true;
}

///////////////////////////////////////////////////////////////////////////
// This function fills up a vector (m_Points) that contains all the point 
// information for all the ODFs. The same is done for the vector containing 
// the color of the tensors (m_tensorsFA).
//
// i_fileFloatData  : The coefficients read from the loaded nifti file.
// Returns true if succesfull, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool ODFs::createStructure( vector< float >& i_fileFloatData )
{
    m_nbPointsPerGlyph = getLODNbOfPoints( m_currentLOD );
    m_nbGlyphs         = m_datasetHelper.m_columns * m_datasetHelper.m_rows * m_datasetHelper.m_frames;
    m_order            = (int)(-3.0f / 2.0f + sqrt( 9.0f / 4.0f - 2.0f * ( 1 - m_bands ) ) );

    m_coefficients.resize( m_nbGlyphs );
    vector< float >::iterator it;
    int i = 0;

    // Fetching the coefficients
    for( it = i_fileFloatData.begin(), i = 0; it != i_fileFloatData.end(); it += m_bands, ++i )
        m_coefficients[i].insert( m_coefficients[i].end(), it, it + m_bands );

    for( unsigned int i = 0; i < NB_OF_LOD; ++i )
    {
        // Creating phi / theta directions matrices with for all LODs
        m_phiThetaDirection.push_back( FMatrix( getLODNbOfPoints((LODChoices)i), 2) );

        // Creating spherical harmonics matrices for all LODs
        m_shMatrix.push_back( FMatrix( getLODNbOfPoints((LODChoices)i), m_bands ) );

        // Filling up the matrices
        getSphericalHarmonicMatrix( m_LODspheres[i], m_phiThetaDirection.back(), m_shMatrix.back() );

        // Fetching our odfs points
        getODFpoints( m_phiThetaDirection.back(), m_LODspheres[i] );
    }

    // Fetching initial sliders positions
    getSlidersPositions( m_currentSliderPos );

    // Axis X, Y, and Z
    m_radius.resize( 3 );

    // Set the number of points per glyph.
    m_nbPointsPerGlyph = getLODNbOfPoints( m_currentLOD );
    
    // We need to reload the buffer in video memory.
    loadBuffer();

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This function will fill up o_sphere with points representing the tensor at a certin LOD.
//
// i_phiThetaDirection  : Matrix containing the phi and theta directions
// o_deformedMeshPts    : Points of the deformed mesh. These points will be rendered
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ODFs::getODFpoints(   FMatrix                       &i_phiThetaDirection,
                           vector < float >              &o_deformedMeshPts  )
{
    int l_idx = 0;

    bool flipx = false;

    float x,y,z;
    // Deformiong the mesh
    for( unsigned int i = 0; i < o_deformedMeshPts.size(); i+=3 )
    {
        l_idx = i / 3;

        // phi    = i_phiThetaDirection( l_idx, 0 );
        // theta  = i_phiThetaDirection( l_idx, 1 );

        o_deformedMeshPts[i  ] = sin( i_phiThetaDirection( l_idx, 1 ) ) * cos( i_phiThetaDirection( l_idx, 0 ) );
        o_deformedMeshPts[i+1] = sin( i_phiThetaDirection( l_idx, 1 ) ) * sin( i_phiThetaDirection( l_idx, 0 ) );
        o_deformedMeshPts[i+2] = cos( i_phiThetaDirection( l_idx, 1 ) );
        
        x = o_deformedMeshPts[i  ];
        y = o_deformedMeshPts[i+1];
        z = o_deformedMeshPts[i+2];

        if ( flipx ) {

           o_deformedMeshPts[i  ] = -x;
           o_deformedMeshPts[i+1] = y;
           o_deformedMeshPts[i+2] = z;
           
        }

    }
}


/////////////////////////////////////////////////////////////////////////// 
// Computes a the C * B multiplication, where
//
// i_C : Odf coefficients vector
// i_B : Spherical harmonics matrix
//
// Returns a vector resulting form C * B
///////////////////////////////////////////////////////////////////////////
void ODFs::computeRadiiArray( const FMatrix &i_B, vector< float > &i_C, vector< float >& o_radius, pair< float, float > &o_minMax )
{
    o_radius.clear();
    o_radius.resize( m_nbPointsPerGlyph, 0.0f );

    if( (int)i_C.size() != m_bands )
        return;

    // Matrix Multiplication
    for( int i = 0; i < m_nbPointsPerGlyph; ++i )
        for( int j = 0; j < m_bands; j++ )
            o_radius[i] += i_C[j] * i_B(i, j);

    // Minimum
    o_minMax.first = *min_element( o_radius.begin(), o_radius.end() );

    // Maximum
    o_minMax.second = *max_element( o_radius.begin(), o_radius.end() ); 
}

///////////////////////////////////////////////////////////////////////////
// This function will draw the tensors.
///////////////////////////////////////////////////////////////////////////
void ODFs::draw()
{
    // We need VBOs for ODFs, particularly to store the radii
    if( ! m_datasetHelper.m_useVBO  )
        return;

    // Enable the shader.
    DatasetInfo::m_dh->m_shaderHelper->m_odfsShader->bind();
    glBindTexture( GL_TEXTURE_1D, m_textureId );

    // This is the color look up table texture.
    DatasetInfo::m_dh->m_shaderHelper->m_odfsShader->setUniSampler( "clut",        0                           );
    
    // This is the brightness level of the odf.
    DatasetInfo::m_dh->m_shaderHelper->m_odfsShader->setUniFloat(   "brightness",  DatasetInfo::m_brightness   );

    // This is the alpha level of the odf.
    DatasetInfo::m_dh->m_shaderHelper->m_odfsShader->setUniFloat(   "alpha",       DatasetInfo::m_alpha        );

    // If m_mapOnSphere is true then the color will be set on a sphere instead of a deformed mesh.
    DatasetInfo::m_dh->m_shaderHelper->m_odfsShader->setUniInt(      "mapOnSphere", (GLint)isDisplayShape(SPHERE)      );

    // If m_colorWithPosition is true then the glyph will be colored with the position of the vertex.
    DatasetInfo::m_dh->m_shaderHelper->m_odfsShader->setUniInt(      "colorWithPos", (GLint)m_colorWithPosition );
 
    // Get the radius attribute location
    m_radiusAttribLoc = glGetAttribLocation( DatasetInfo::m_dh->m_shaderHelper->m_odfsShader->getProgramObject(),
                        "radius" );
    Glyph::draw();

    // Disable the attribute
    glDisableVertexAttribArray( m_radiusAttribLoc);

    // Disable the tensor color shader.
    DatasetInfo::m_dh->m_shaderHelper->m_odfsShader->release();
}

///////////////////////////////////////////////////////////////////////////
// This function will draw a tensor at a specified voxel position.
//
// i_zIndex         : The z voxel of the tensor.
// i_yIndex         : The y voxel of the tensor.
// i_xIndex         : The x voxel of the tensor
///////////////////////////////////////////////////////////////////////////
void ODFs::drawGlyph( int i_zVoxel, int i_yVoxel, int i_xVoxel, AxisType i_axis )
{
    // Before we start calculating everything, lets make sure that the glyph is visible on the screen (inside the frustum).
    // To make things faster and easier, we use the glyph voxel as its bounding box.
    // The first vector represent the voxel center and the second one represend the tensor size.
    if( ! boxInFrustum( Vector( ( i_xVoxel + 0.5f ) * m_datasetHelper.m_xVoxel,
                                ( i_yVoxel + 0.5f ) * m_datasetHelper.m_yVoxel,
                                ( i_zVoxel + 0.5f ) * m_datasetHelper.m_zVoxel ),
                        Vector( m_datasetHelper.m_xVoxel / 2.0f,
                                m_datasetHelper.m_yVoxel / 2.0f,
                                m_datasetHelper.m_zVoxel / 2.0f ) ) )
        return;

    // Get the current tensors index in the coeffs's buffer
    int  currentIdx = getGlyphIndex( i_zVoxel, i_yVoxel, i_xVoxel );   
 
    // Odf offset.
    GLfloat l_offset[3];
    getVoxelOffset( i_zVoxel, i_yVoxel, i_xVoxel, l_offset );
    DatasetInfo::m_dh->m_shaderHelper->m_odfsShader->setUni3Float( "offset", l_offset );

    // Lets set the min max radii for this odf.
    DatasetInfo::m_dh->m_shaderHelper->m_odfsShader->setUni2Float( "radiusMinMax", m_radiiMinMaxMap[currentIdx] );    

    // Enable attribute
    glEnableVertexAttribArray( m_radiusAttribLoc );

    // Radii
    glBindBuffer( GL_ARRAY_BUFFER, m_radiusBuffer[i_axis] );

    // The index of the radii for the current glyph
    int l_radiiIdx = 0;

    if( i_axis == X_AXIS )
        l_radiiIdx = m_nbPointsPerGlyph * ( i_zVoxel * m_datasetHelper.m_rows    + i_yVoxel );

    else if( i_axis == Y_AXIS )
        l_radiiIdx = m_nbPointsPerGlyph * ( i_zVoxel * m_datasetHelper.m_columns + i_xVoxel );

    else if( i_axis == Z_AXIS )
        l_radiiIdx = m_nbPointsPerGlyph * ( i_yVoxel * m_datasetHelper.m_columns + i_xVoxel );

    // One radius per vertex
    glVertexAttribPointer( m_radiusAttribLoc, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*) (l_radiiIdx * sizeof( float )) );

    // Vertices
    glBindBuffer( GL_ARRAY_BUFFER, *m_hemisphereBuffer );
    glVertexPointer( 3, GL_FLOAT, 0, 0 );

    // The culling does not work for the odfs, so we simply disable it.
    glDisable( GL_CULL_FACE );

    // Set axis flip.
    GLfloat l_flippedAxes[3];
    m_flippedAxes[0] ? l_flippedAxes[0] = -1.0f : l_flippedAxes[0] = 1.0f;
    m_flippedAxes[1] ? l_flippedAxes[1] = -1.0f : l_flippedAxes[1] = 1.0f;
    m_flippedAxes[2] ? l_flippedAxes[2] = -1.0f : l_flippedAxes[2] = 1.0f;
    
    //For VisContest with sh basis computed from ptk, Max Thesis
    if( i_axis == Y_AXIS ) { //Coronal: flip x
       l_flippedAxes[0] *= -1.0f;  
    } 
    // Need a global flip in X on top of that, which is done above


    DatasetInfo::m_dh->m_shaderHelper->m_odfsShader->setUni3Float(   "axisFlip",    l_flippedAxes               );

    // Lets set the radius modifier.
    DatasetInfo::m_dh->m_shaderHelper->m_odfsShader->setUniInt( "swapRadius", 0 );
    // Draw the first half of the odfs.
    glDrawArrays( GL_TRIANGLE_STRIP, 0, m_nbPointsPerGlyph );

    // Lets set the radius modifier.
    DatasetInfo::m_dh->m_shaderHelper->m_odfsShader->setUniInt( "swapRadius", 1 );
    // Draw the other half of the odfs.
    glDrawArrays( GL_TRIANGLE_STRIP, 0, m_nbPointsPerGlyph );
}

///////////////////////////////////////////////////////////////////////////
// This function will reload the appropriate radius buffer, depending on
// which slider was moved
//
// i_axis       : The axis that needs to be refreshed.
//////////////////////////////////////////////////////////////////////////
void ODFs::sliderPosChanged( AxisType i_axis )
{
    switch( i_axis )
    {
        case X_AXIS : computeXRadiusSlice(); break;
        case Y_AXIS : computeYRadiusSlice(); break;
        case Z_AXIS : computeZRadiusSlice(); break;
        default     :  return;
    }   
    
    reloadRadiusBuffer( i_axis );
}

///////////////////////////////////////////////////////////////////////////
// This function will compute the current radius's for the X slice.
///////////////////////////////////////////////////////////////////////////
void ODFs::computeXRadiusSlice()
{
    int l_idx = 0;
    vector< float > l_radius;
    pair< float, float > l_minMax;

    m_radius[X_AXIS].clear();

    for( int z = 0; z <  m_datasetHelper.m_frames; ++z )
        for( int y = 0; y <  m_datasetHelper.m_rows; ++y )
        {
            l_idx = getGlyphIndex( z, y, m_currentSliderPos[0] );

            // Get radius
            computeRadiiArray( m_shMatrix[m_currentLOD], m_coefficients[l_idx], l_radius, l_minMax );

            m_radiiMinMaxMap[l_idx] = l_minMax;

            m_radius[X_AXIS].insert( m_radius[X_AXIS].end(), l_radius.begin(), l_radius.end() );
        }
}

///////////////////////////////////////////////////////////////////////////
// This function will compute the current radius's for the Y slice.
///////////////////////////////////////////////////////////////////////////
void ODFs::computeYRadiusSlice()
{
    int l_idx = 0;
    vector< float > l_radius;
    pair< float, float > l_minMax;

    m_radius[Y_AXIS].clear();

    for( int z = 0; z <  m_datasetHelper.m_frames; ++z )
        for( int x = 0; x <  m_datasetHelper.m_columns; ++x )
        {
            l_idx = getGlyphIndex( z, m_currentSliderPos[1], x );

            // Get radius
            computeRadiiArray( m_shMatrix[m_currentLOD], m_coefficients[l_idx], l_radius, l_minMax );

            m_radiiMinMaxMap[l_idx] = l_minMax;

            m_radius[Y_AXIS].insert( m_radius[Y_AXIS].end(), l_radius.begin(), l_radius.end() );
        }
}

///////////////////////////////////////////////////////////////////////////
// This function will compute the current radius's for the Z slice.
///////////////////////////////////////////////////////////////////////////
void ODFs::computeZRadiusSlice()
{
    int l_idx = 0;
    vector< float > l_radius;
    pair< float, float > l_minMax;

    m_radius[Z_AXIS].clear();

    for( int y = 0; y <  m_datasetHelper.m_rows; ++y )
        for( int x = 0; x <  m_datasetHelper.m_columns; ++x )
        {
            l_idx = getGlyphIndex( m_currentSliderPos[2], y, x );

            // Get radius
            computeRadiiArray( m_shMatrix[m_currentLOD], m_coefficients[l_idx], l_radius, l_minMax );

            m_radiiMinMaxMap[l_idx] = l_minMax;
            
            m_radius[Z_AXIS].insert( m_radius[Z_AXIS].end(), l_radius.begin(), l_radius.end() );
        }
}

///////////////////////////////////////////////////////////////////////////
// This function initializes the array buffer containing the hemishere points 
// that will be load in a buffer in video memory. If an error occur while 
// performing this operation then we will set m_useVBO to false, to indicate
// that we will not use VBO. 
//////////////////////////////////////////////////////////////////////////
void ODFs::loadBuffer()
{
    // We need to (re)load the buffer in video memory only if we are using VBO.
    if( !m_datasetHelper.m_useVBO )
        return;        

    computeXRadiusSlice();
    computeYRadiusSlice();
    computeZRadiusSlice();

    Glyph::loadBuffer();

    // Radius Buffers
    loadRadiusBuffer( X_AXIS );
    loadRadiusBuffer( Y_AXIS );
    loadRadiusBuffer( Z_AXIS );
}

///////////////////////////////////////////////////////////////////////////
// This function loads the radius buffers into video memory
//
// i_axis : the axis for which we wish to load the buffer
//////////////////////////////////////////////////////////////////////////
void ODFs::loadRadiusBuffer( AxisType i_axis )
{
    // Generating buffer name.
    if( m_radiusBuffer == NULL )
    {
        m_radiusBuffer = new GLuint[3];
        glGenBuffers( 3, m_radiusBuffer );
    }

    // Current slice
    glBindBuffer( GL_ARRAY_BUFFER, m_radiusBuffer[i_axis] );

    glBufferData( GL_ARRAY_BUFFER, 
                  m_radius[i_axis].size() * sizeof( GLfloat ), 
                  &m_radius[i_axis][0], 
                  GL_DYNAMIC_DRAW );

    // There was a problem loading this buffer into video memory!
    if( DatasetInfo::m_dh->GLError() )
    {
        DatasetInfo::m_dh->printGLError( wxT( "initialize vbo points for tensors" ) );
        m_datasetHelper.m_useVBO = false;
        delete [] m_radiusBuffer;
    }
}

///////////////////////////////////////////////////////////////////////////
// This function re-loads the radius buffers into video memory
//
// i_axis : the axis for which we wish to re-load the buffer
//////////////////////////////////////////////////////////////////////////
void ODFs::reloadRadiusBuffer( AxisType i_axis )
{
    if( m_radiusBuffer == NULL )
        return;

    float * l_bufferData = NULL;
       
    glBindBuffer( GL_ARRAY_BUFFER, m_radiusBuffer[i_axis] );
    l_bufferData = (float*) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );

    if( l_bufferData == NULL )
        return;
        
    memcpy( l_bufferData,
            &m_radius[i_axis][0],
            sizeof( float ) * m_radius[i_axis].size() );

    glUnmapBuffer( GL_ARRAY_BUFFER );
}

///////////////////////////////////////////////////////////////////////////
// This functions computes the Spherical harmonics matrix as well as the
// matrix containing the phi and theta directions. The basis implementation
// corresponds to Descoteaux's et al Research Report 5768
//
// i_meshPts            : Points of the undeformed mesh.
// o_phiThetaDirection  : Matrix containing the phi and theta directions
// o_shMatrix           : Spherical harmonics matrix
///////////////////////////////////////////////////////////////////////////
void ODFs::getSphericalHarmonicMatrixRR5768( const vector< float > &i_meshPts, 
                                             FMatrix &o_phiThetaDirection, FMatrix &o_shMatrix )
{
    int nbMeshPts = i_meshPts.size() / 3;
    
    complex< float > cplx, cplx_1, cplx_2;

    float l_cartesianDir[3]; // Stored in x, y z
    float l_sphericalDir[3]; // Stored in radius, phi, theta

    for( int i = 0; i < nbMeshPts; i++) 
    {
        int j = 0;   // Counter for the j dimension of o_shMatrix

        l_cartesianDir[0] = i_meshPts[ i * 3     ];
        l_cartesianDir[1] = i_meshPts[ i * 3 + 1 ];
        l_cartesianDir[2] = i_meshPts[ i * 3 + 2 ];

        Helper::cartesianToSpherical( l_cartesianDir, l_sphericalDir );

        o_phiThetaDirection( i, 0 ) = l_sphericalDir[1]; // Phi
        o_phiThetaDirection( i, 1 ) = l_sphericalDir[2]; // Theta

        int sign = 0;

        for( int l = 0; l <= m_order; l+=2 )
            for( int m = 0; m <= l; ++m ) 
            {                   
                // Positive "m" spherical harmonic
                if( m == 0 ) 
                {
                    cplx_1 = getSphericalHarmonic( l, m, l_sphericalDir[2], l_sphericalDir[1] );

                    if( fabs( imag( cplx_1 ) ) > 0.0001 )                  
                        return; // Modified spherical harmonic basis must be REAL !

                    o_shMatrix(i, j) = real( cplx_1 );

                }

                // Negative "m" spherical harmonic
                else
                {
                    // Get the corresponding spherical harmonic
                    cplx_1 = getSphericalHarmonic( l,  m, l_sphericalDir[2], l_sphericalDir[1] );
                    cplx_2 = getSphericalHarmonic( l, -m, l_sphericalDir[2], l_sphericalDir[1] );

                    // (-1)^m
                    ( m % 2 == 1 ) ? sign = -1 : sign = 1;
                    
                    {
                        complex< float > s( sign, 0.0 );
                        complex< float > n( sqrt( ( float )2 ) / 2, 0.0 );
                        cplx = n * ( s * cplx_1 + cplx_2 );
                    }

                    // (-1)^(m+1) 
                    ( m % 2 == 1 ) ? sign = 1 : sign = -1;

                    {
                        complex< float > n(0.0, ( float )sqrt( ( float ) 2 ) / 2 );
                        complex< float > s(sign, 0.0);
                        cplx_2 = n*(s*cplx_1 + cplx_2);
                    }                

                    if( fabs(imag(cplx)) > 0.0001 || fabs(imag(cplx_2)) > 0.0001 ) 
                        return; // Modified spherical harmonic basis must be REAL!

                    o_shMatrix(i, j) = real( cplx_1 );
                    
                    ++j;

                    o_shMatrix(i, j) = real( cplx_2 );

                } // else

                ++j;

            } // for
    } // for
}


///////////////////////////////////////////////////////////////////////////
// This functions computes the Spherical harmonics matrix as well as the
// matrix containing the phi and theta directions. The basis implementation
// corresponds to Descoteaux's PhD thesis
//
// i_meshPts            : Points of the undeformed mesh.
// o_phiThetaDirection  : Matrix containing the phi and theta directions
// o_shMatrix           : Spherical harmonics matrix
///////////////////////////////////////////////////////////////////////////
void ODFs::getSphericalHarmonicMatrixDescoteauxThesis( const vector< float > &i_meshPts, 
                                                       FMatrix &o_phiThetaDirection, 
                                                       FMatrix &o_shMatrix )
{
    int nbMeshPts = i_meshPts.size() / 3;
    
    complex< float > cplx, cplx_1, cplx_2;

    float l_cartesianDir[3]; // Stored in x, y z
    float l_sphericalDir[3]; // Stored in radius, phi, theta

    for( int i = 0; i < nbMeshPts; i++) 
    {
        int j = 0;   // Counter for the j dimension of o_shMatrix

        l_cartesianDir[0] = i_meshPts[ i * 3     ];
        l_cartesianDir[1] = i_meshPts[ i * 3 + 1 ];
        l_cartesianDir[2] = i_meshPts[ i * 3 + 2 ];

        Helper::cartesianToSpherical( l_cartesianDir, l_sphericalDir );

        o_phiThetaDirection( i, 0 ) = l_sphericalDir[1]; // Phi
        o_phiThetaDirection( i, 1 ) = l_sphericalDir[2]; // Theta

        for( int l = 0; l <= m_order; l+=2 )
            for( int m = -l; m <= l; ++m ) 
            {                   
               cplx_1 = getSphericalHarmonic( l,  m, l_sphericalDir[2], l_sphericalDir[1] );
               cplx_2 = getSphericalHarmonic( l, abs(m), l_sphericalDir[2], l_sphericalDir[1] );

               if(m > 0)
                  o_shMatrix(i,j) = std::sqrt(2.0)*imag(cplx_1);
               else if( m == 0 )
                  o_shMatrix(i,j) = real(cplx_1);
               else
                  o_shMatrix(i,j) = std::sqrt(2.0)*real(cplx_2);

               ++j;
               
            } // for
    } // for
}


///////////////////////////////////////////////////////////////////////////
// This functions computes the Spherical harmonics matrix as well as the
// matrix containing the phi and theta directions. The basis implementation
// corresponds to the implementation in PTK (Poupon ToolKit)
//
// i_meshPts            : Points of the undeformed mesh.
// o_phiThetaDirection  : Matrix containing the phi and theta directions
// o_shMatrix           : Spherical harmonics matrix
///////////////////////////////////////////////////////////////////////////
void ODFs::getSphericalHarmonicMatrixPTK( const vector< float > &i_meshPts, 
                                          FMatrix &o_phiThetaDirection, 
                                          FMatrix &o_shMatrix )
{
    int nbMeshPts = i_meshPts.size() / 3;
    
    complex< float > cplx, cplx_1, cplx_2;

    float l_cartesianDir[3]; // Stored in x, y z
    float l_sphericalDir[3]; // Stored in radius, phi, theta

    for( int i = 0; i < nbMeshPts; i++) 
    {
        int j = 0;   // Counter for the j dimension of o_shMatrix

        l_cartesianDir[0] = i_meshPts[ i * 3     ];
        l_cartesianDir[1] = i_meshPts[ i * 3 + 1 ];
        l_cartesianDir[2] = i_meshPts[ i * 3 + 2 ];

        Helper::cartesianToSpherical( l_cartesianDir, l_sphericalDir );

        o_phiThetaDirection( i, 0 ) = l_sphericalDir[1]; // Phi
        o_phiThetaDirection( i, 1 ) = l_sphericalDir[2]; // Theta

        for( int l = 0; l <= m_order; l+=2 )
            for( int m = -l; m <= l; ++m ) 
            {                   
               cplx_1 = getSphericalHarmonic( l,  m, l_sphericalDir[2], l_sphericalDir[1] );
               cplx_2 = getSphericalHarmonic( l, abs(m), l_sphericalDir[2], l_sphericalDir[1] );

               int sign = 1;
               if( m % 2 )
                  sign *= -1;

               if(m > 0)
                  o_shMatrix(i,j) = std::sqrt(2.0)*sign*imag(cplx_2);
               else if( m == 0 )
                  o_shMatrix(i,j) = real(cplx_1);
               else
                  o_shMatrix(i,j) = std::sqrt(2.0)*real(cplx_1);

               ++j;
               
            } // for
    } // for
}


///////////////////////////////////////////////////////////////////////////
// This functions computes the Spherical harmonics matrix as well as the
// matrix containing the phi and theta directions. The basis implementation
// corresponds to Tournier et al 2004 and 2007 bases
//
// i_meshPts            : Points of the undeformed mesh.
// o_phiThetaDirection  : Matrix containing the phi and theta directions
// o_shMatrix           : Spherical harmonics matrix
///////////////////////////////////////////////////////////////////////////
void ODFs::getSphericalHarmonicMatrixTournier( const vector< float > &i_meshPts, 
                                               FMatrix &o_phiThetaDirection, 
                                               FMatrix &o_shMatrix )
{
    int nbMeshPts = i_meshPts.size() / 3;
    
    complex< float > cplx, cplx_1, cplx_2;

    float l_cartesianDir[3]; // Stored in x, y z
    float l_sphericalDir[3]; // Stored in radius, phi, theta

    for( int i = 0; i < nbMeshPts; i++) 
    {
        int j = 0;   // Counter for the j dimension of o_shMatrix

        l_cartesianDir[0] = i_meshPts[ i * 3     ];
        l_cartesianDir[1] = i_meshPts[ i * 3 + 1 ];
        l_cartesianDir[2] = i_meshPts[ i * 3 + 2 ];

        Helper::cartesianToSpherical( l_cartesianDir, l_sphericalDir );

        o_phiThetaDirection( i, 0 ) = l_sphericalDir[1]; // Phi
        o_phiThetaDirection( i, 1 ) = l_sphericalDir[2]; // Theta

        for( int l = 0; l <= m_order; l+=2 )
        {
            for( int m = -l; m <= l; ++m ) 
            {                   

               cplx_1 = getSphericalHarmonic( l,  m, l_sphericalDir[2], l_sphericalDir[1] );

               if(m >= 0) {
                  o_shMatrix(i,j) = real(cplx_1);
               }
               else { // /* negative "m" SH  */
                  o_shMatrix(i,j) = imag(cplx_1);
               }
               ++j;
            } 
        }
    }
}

void ODFs::getSphericalHarmonicMatrix( const vector< float > &i_meshPts, 
                                       FMatrix &o_phiThetaDirection, FMatrix &o_shMatrix )
{

   if( m_sh_basis == 0 ) {
      cout << "Using RR5768 SH basis (as in DMRI)\n";
      getSphericalHarmonicMatrixRR5768(i_meshPts, o_phiThetaDirection, o_shMatrix );
   }
   else if( m_sh_basis == 1 ) {
      cout << "Using Max's Thesis SH basis\n";
      getSphericalHarmonicMatrixDescoteauxThesis(i_meshPts, o_phiThetaDirection, o_shMatrix );
   }
   else if( m_sh_basis == 2 ) {
      cout << "Using Tournier's SH basis\n";
      getSphericalHarmonicMatrixTournier(i_meshPts, o_phiThetaDirection, o_shMatrix );
   }
   else if( m_sh_basis == 3 ) {
      cout << "Using PTK SH basis\n";
      getSphericalHarmonicMatrixPTK(i_meshPts, o_phiThetaDirection, o_shMatrix );
   }
   else
      getSphericalHarmonicMatrixRR5768(i_meshPts, o_phiThetaDirection, o_shMatrix );
   
}

/////////////////////////////////////////////////////////////////////////// 
// Computes a specific spherical harmonic
//
// i_l      : order of the spherical harmonic
// i_m      : phase factor of the spherical harmonic
// i_theta  : theta angle
// i_phi    : phi angle
//
// Returns the spherical harmonic
///////////////////////////////////////////////////////////////////////////
complex< float > ODFs::getSphericalHarmonic( int i_l, int i_m, float i_theta, float i_phi ) 
{
    int l_absm = std::abs( i_m );
    float l_sign = 0.0f;

    ( l_absm % 2 == 1 ) ? l_sign = -1.0f : l_sign = 1.0f;

    complex< float > l_retval(0.0f, (float)( l_absm * i_phi ) );
    l_retval = std::exp( l_retval );

    double l_legendrePoly = 0.0;

    Helper::getAssociatedLegendrePlm( i_l, l_absm, cos( i_theta ), l_legendrePoly );

    float l_factor = sqrt( ( (float)( 2 * i_l + 1 ) / ( 4.0f * M_PI ) ) * 
                           ( Helper::getFactorial( i_l - l_absm ) /
                        Helper::getFactorial( i_l + l_absm ) ) ) * l_legendrePoly;

    l_retval *= l_factor;

    if ( i_m < 0 ) 
    {
        l_retval  = std::conj( l_retval );
        l_retval *= l_sign;
    }

    return l_retval;
}

void ODFs::createPropertiesSizer(MainFrame *parent)
{
    Glyph::createPropertiesSizer(parent);
}

void ODFs::updatePropertiesSizer()
{
    Glyph::updatePropertiesSizer();
    //set to min.
    m_pradiobtnMainAxis->Enable(false);
    //m_psliderLightAttenuation->SetValue(m_psliderLightAttenuation->GetMin());
    m_psliderLightAttenuation->Enable(false);
    //m_psliderLightXPosition->SetValue(m_psliderLightXPosition->GetMin());
    m_psliderLightXPosition->Enable(false);
    //m_psliderLightYPosition->SetValue(m_psliderLightYPosition->GetMin());
    m_psliderLightYPosition->Enable(false);
    //m_psliderLightZPosition->SetValue(m_psliderLightZPosition->GetMin());
    m_psliderLightZPosition->Enable(false);
    //m_psliderScalingFactor->SetValue(m_psliderScalingFactor->GetMin());
    m_psliderScalingFactor->Enable (false);
}



