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

#include "DatasetManager.h"
#include "../Logger.h"
#include "../gfx/ShaderHelper.h"
#include "../gui/MyListCtrl.h"
#include "../gui/SceneManager.h"
#include "../misc/nifti/nifti1_io.h"
#include "../misc/Fantom/FMatrix.h"

#include <GL/glew.h>
#include <wx/math.h>
#include <wx/xml/xml.h>

#include <algorithm>
#include <complex>
using std::complex;

#include <fstream>
#include <limits>
using std::numeric_limits;

#include <map>
using std::map;
using std::pair;

#include <vector>
using std::vector;

#define DEF_POS   wxDefaultPosition
#define DEF_SIZE  wxDefaultSize

// m_sh_basis
// 0: Original Descoteaux et al RR 5768 basis 
// 1: Descoteaux PhD thesis basis definition (default in dmri)
// 2: Tournier
// 3: PTK
///////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////
// ODFs::ODFs()
// :   Glyph(),
//     m_isMaximasSet   ( false ),
//     m_axisThreshold  ( 0.2f ),
//     m_order          ( 0 ),
//     m_radiusAttribLoc( 0 ),
//     m_radiusBuffer   ( NULL ),
//     m_nbors          ( NULL ),
//     m_sh_basis       ( SH_BASIS_DESCOTEAUX )
// {
//     m_scalingFactor = 5.0f;
// 
//     // Generating hemispheres
//     generateSpherePoints( m_scalingFactor );
// }


ODFs::ODFs( const wxString &filename )
:   Glyph(), 
    m_isMaximasSet   ( false ),
    m_axisThreshold  ( 0.2f ),
    m_order          ( 0 ),
    m_radiusAttribLoc( 0 ),
    m_radiusBuffer   ( NULL ),    
    m_nbors          ( NULL ),
    m_sh_basis       ( SH_BASIS_DESCOTEAUX )
{
    m_scalingFactor = 5.0f;
    m_fullPath = filename;

#ifdef __WXMSW__
    m_name = filename.AfterLast( '\\' );
#else
    m_name = filename.AfterLast( '/' );
#endif

    // Generating hemispheres
    generateSpherePoints( m_scalingFactor );
}

ODFs::~ODFs()
{
    Logger::getInstance()->print( wxT( "Executing ODFs destructor..." ), LOGLEVEL_DEBUG );
    if( m_radiusBuffer )
    {
        glDeleteBuffers( 1, m_radiusBuffer );
        delete [] m_radiusBuffer;
    }

    if( m_nbors != NULL )
    {
        delete m_nbors;
        m_nbors = NULL;
    }
    Logger::getInstance()->print( wxT( "ODFs destructor done." ), LOGLEVEL_DEBUG );
}

bool ODFs::load( nifti_image *pHeader, nifti_image *pBody )
{
    m_columns = pHeader->dim[1]; //80
    m_rows    = pHeader->dim[2]; //1
    m_frames  = pHeader->dim[3]; //72
    m_bands   = pHeader->dim[4];

    m_voxelSizeX = pHeader->dx;
    m_voxelSizeY = pHeader->dy;
    m_voxelSizeZ = pHeader->dz;

    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    if( m_voxelSizeX != voxelX || m_voxelSizeY != voxelY || m_voxelSizeZ != voxelZ )
    {
        Logger::getInstance()->print( wxT( "Voxel size different from anatomy." ), LOGLEVEL_ERROR );
        return false;
    }

    m_type = ODFS;

    // Order has to be between 0 and 16 (0, 2, 4, 6, 8, 10, 12, 14, 16)
//     if( pHeader->datatype != 16  || !( m_bands == 0   || m_bands == 15 || m_bands == 28 || 
//         m_bands == 45  || m_bands == 66 || m_bands == 91 || 
//         m_bands == 120 || m_bands == 153 ) )
//     {
//         DatasetInfo::m_dh->m_lastError = wxT( "Not a valid ODFs file format" );
//         return false;
//     }

    int l_nSize = pHeader->dim[1] * pHeader->dim[2] * pHeader->dim[3];

    float* l_data = (float*)pBody->data;

    std::vector< float > l_fileFloatData( l_nSize * m_bands );

    // We need to do a bit of moving around with the data in order to have it like we want.
    for( int i( 0 ); i < l_nSize; ++i )
    {
        for( int j( 0 ); j < m_bands; ++j )
        {
            l_fileFloatData[i * m_bands + j] = l_data[j * l_nSize + i];
        }
    }

    // Once the file has been read successfully, we need to create the structure 
    // that will contain all the sphere points representing the ODFs.
    createStructure( l_fileFloatData );

    m_isLoaded = true;

    return true;
}

//////////////////////////////////////////////////////////////////////////

bool ODFs::save( wxXmlNode *pNode ) const
{
    assert( pNode != NULL );

    pNode->SetName( wxT( "dataset" ) );
    DatasetInfo::save( pNode );

    return true;
}

//////////////////////////////////////////////////////////////////////////

void ODFs::extractMaximas()
{
    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();

    std::cout << "Extracting maximas ... please wait 30sec \n";
    m_nbors = new std::vector<std::pair<float,int> >[m_phiThetaDirection[LOD_3].getDimensionY()]; // Set number of points to maximum details
    m_angle_min = get_min_angle();
    m_nbPointsPerGlyph = getLODNbOfPoints( LOD_3 ); // Set number of points to maximum details for C*B mult
    set_nbors(m_phiThetaDirection[LOD_3]); // Create neighboring system
    m_mainDirections.resize( frames * rows * columns );
    
    int currentIdx;

    for( int z( 0 ); z < frames; ++z )
    {
        for( int y( 0 ); y < rows; ++y )
        {
            for( int x( 0 ); x < columns; ++x )
            {
                currentIdx = getGlyphIndex( z, y, x );

                if( m_coefficients[currentIdx][0] != 0 )
                {
                    m_mainDirections[currentIdx] = getODFmax( m_coefficients[currentIdx], m_shMatrix[LOD_3], m_phiThetaDirection[LOD_3], m_axisThreshold );
                }
            }
        }
    }

    m_nbPointsPerGlyph = getLODNbOfPoints( m_currentLOD ); //Set nb point back to currentLOD
     
}
///////////////////////////////////////////////////////////////////////////
// This function fills up a vector (m_Points) that contains all the point 
// information for all the ODFs. The same is done for the vector containing 
// the color of the tensors (m_tensorsFA).
//
// i_fileFloatData  : The coefficients read from the loaded nifti file.
// Returns true if successful, false otherwise.
///////////////////////////////////////////////////////////////////////////
bool ODFs::createStructure( vector< float >& i_fileFloatData )
{
    m_nbPointsPerGlyph = getLODNbOfPoints( m_currentLOD );
    m_nbGlyphs         = DatasetManager::getInstance()->getColumns() * DatasetManager::getInstance()->getRows() * DatasetManager::getInstance()->getFrames();
    m_order            = (int)(-3.0f / 2.0f + sqrt( 9.0f / 4.0f - 2.0f * ( 1 - m_bands ) ) );

    m_coefficients.resize( m_nbGlyphs );
    vector< float >::iterator it;
    int i = 0;

    // Fetching the coefficients
    for( it = i_fileFloatData.begin(), i = 0; it != i_fileFloatData.end(); it += m_bands, ++i )
    { 
        m_coefficients[i].insert( m_coefficients[i].end(), it, it + m_bands );
    }

    /*cout SH basis name */
    switch( m_sh_basis )
    {
        case 0:
            Logger::getInstance()->print( wxT( "Using RR5768 SH basis (as in DMRI)" ), LOGLEVEL_MESSAGE );
            break;
        case 1:
            Logger::getInstance()->print( wxT( "Using Max's Thesis SH basis" ), LOGLEVEL_MESSAGE );
            break;
        case 2:
            Logger::getInstance()->print( wxT( "Using Tournier's SH basis" ), LOGLEVEL_MESSAGE );
            break;
        case 3:
            Logger::getInstance()->print( wxT( "Using PTK SH basis" ), LOGLEVEL_MESSAGE );
            break;
        default:
            return false; // We do nothing incase the param was not good.
    }

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

float ODFs::get_min_angle()
{ 
    std::vector<std::pair<float,float> > vectUnique;
    std::pair<float,float> res;
    float angle_min = 90.0f;   

    for(unsigned int i=0; i < m_phiThetaDirection[LOD_3].getDimensionY(); i++)
    {   // Remove all recurrent point of phiThetaDir in vectUnique
        bool isUnique = true;
        for(unsigned int j=0; j < vectUnique.size() && isUnique ; j++)
        {
            if(m_phiThetaDirection[LOD_3](i,0) == vectUnique[j].first && m_phiThetaDirection[LOD_3](i,1) == vectUnique[j].second)
            {
                isUnique = false;
            }
        }
        if(isUnique)
        {
            res.first = m_phiThetaDirection[LOD_3](i,0);
            res.second = m_phiThetaDirection[LOD_3](i,1);
            vectUnique.push_back(res); 
        }
    }
    
    //find min angle in mesh
    /* approx angle between two discrete samplings on the sphere */    
    direction d1;
          
    d1.x = std::cos(vectUnique[2].first)*std::sin(vectUnique[2].second);
    d1.y = std::sin(vectUnique[2].first)*std::sin(vectUnique[2].second);
    d1.z = std::cos(vectUnique[2].second);
      
    /* finding minimum angle between samplings */
    for(unsigned int i = 0; i < vectUnique.size(); i++) 
    {
        if(i != 2) 
        {
            direction d2;
            d2.x = std::cos(vectUnique[i].first)*std::sin(vectUnique[i].second);
            d2.y = std::sin(vectUnique[i].first)*std::sin(vectUnique[i].second);
            d2.z = std::cos(vectUnique[i].second);
                   
            float dot = d1.x*d2.x + d1.y*d2.y + d1.z*d2.z;
            dot = 180*std::acos(dot)/M_PI;
              
            if(dot < angle_min)
            {
                angle_min = dot;
            }
        }
    }
    return angle_min;
}

/*
    Set m_nbors for Max Dir
*/
void ODFs::set_nbors(FMatrix o_phiThetaDirection)
{
    // find neighbors to all mesh points 
    direction d,d2; /*current direction*/
    const float max_allowed_angle = 90;
 
    for(unsigned int i = 0; i < m_phiThetaDirection[LOD_3].getDimensionY(); i++) 
    {
    
        d.x = std::cos(m_phiThetaDirection[LOD_3](i,0))*std::sin(m_phiThetaDirection[LOD_3](i,1));
        d.y = std::sin(m_phiThetaDirection[LOD_3](i,0))*std::sin(m_phiThetaDirection[LOD_3](i,1));
        d.z = std::cos(m_phiThetaDirection[LOD_3](i,1));
        
        /* 
           look at other possible direction sampling neighbors
           
           if a sampling directions is within +- 3 degrees from i,
           we consider it and check if i is bigger
        */
        for(unsigned int j=0; j<m_phiThetaDirection[LOD_3].getDimensionY(); j++)
        {
            if(j != i ) 
            {
                d2.x = std::cos(m_phiThetaDirection[LOD_3](j,0))*std::sin(m_phiThetaDirection[LOD_3](j,1));
                d2.y = std::sin(m_phiThetaDirection[LOD_3](j,0))*std::sin(m_phiThetaDirection[LOD_3](j,1));
                d2.z = std::cos(m_phiThetaDirection[LOD_3](j,1));
                        
                float angle_found = 180*std::acos(d.x*d2.x + d.y*d2.y + d.z*d2.z)/M_PI;
                if(angle_found > max_allowed_angle)
                {
                    angle_found = 180 - angle_found;
                }
                if(angle_found >= m_angle_min)
                {
                    /* If not full capacity add another neighbors*/
                    if(m_nbors[i].size() < 15) //Change this value for +/- #directions
                    {
                        bool isDiff = true;
                        if(m_nbors[i].size() !=0)
                        {
                          for(unsigned int n=0; n< m_nbors[i].size() && isDiff ; n++)
                          {
                              if(m_phiThetaDirection[LOD_3](j,0) == m_phiThetaDirection[LOD_3](m_nbors[i][n].second,0) && 
                                  m_phiThetaDirection[LOD_3](j,1) == m_phiThetaDirection[LOD_3](m_nbors[i][n].second,1))
                              {
                                isDiff = false;
                              }
                          }
                          if(isDiff)
                          {
                            std::pair<float,int> element(angle_found,j);
                            m_nbors[i].push_back(element);
                          }
                        }
                        else
                        {
                            std::pair<float,int> element(angle_found,j);
                            m_nbors[i].push_back(element);
                        }
                    }
                    else
                    {   /* If full capacity add another closer neighbors*/
                        float max = -1;
                        int indice = 0;

                        for(unsigned int k=0; k < m_nbors[i].size() ; k++)
                        {
                            if(m_nbors[i][k].first > max)
                            {
                                max = m_nbors[i][k].first;
                                indice = k;
                            }
                        }

                        if(max>=0 && max > angle_found)
                        {
                            bool isDiff = true;
                            for(unsigned int n=0; n< m_nbors[i].size() && isDiff ; n++)
                            {
                                if(m_phiThetaDirection[LOD_3](j,0) == m_phiThetaDirection[LOD_3](m_nbors[i][n].second,0) && 
                                    m_phiThetaDirection[LOD_3](j,1) == m_phiThetaDirection[LOD_3](m_nbors[i][n].second,1))
                                {
                                    isDiff = false;
                                }
                            }
                            if(isDiff)
                            {
                                std::pair<float,int> element(angle_found,j);
                                m_nbors[i][indice] = element;
                            }
                        }
                    }
                }
            }
        } 
    }
}
/*
    Extracts Main Directions of ODFs
*/
std::vector<Vector> ODFs::getODFmax(vector < float > coefs, const FMatrix & SHmatrix, const FMatrix & grad, const float & max_thresh)
{

    vector<Vector>       max_dir;
    const float          epsilon = 0.0f;  //for equality measurement
    float                max     = 0;
    float                min     = numeric_limits<float>::infinity();
    vector< float >      ODF;
    pair< float, float > l_minMax;
    vector<float>        norm_hemisODF;

    computeRadiiArray( SHmatrix, coefs, ODF, l_minMax ); // Projection of spherical harmonics on the sphere

    if(l_minMax.first < 0)
    {   // Eliminate negative values on the sphere if min < 0
        for(unsigned int i = 0; i < ODF.size(); i++) 
        {
            if(ODF[i] < 0) 
            {
                ODF[i]=0;
            }
            if(ODF[i] > max)
            {
                max = ODF[i];
            }
            if(ODF[i] < min)
            {
                min = ODF[i];
            }
        }
    }
    else
    {
        min = l_minMax.first;
        max = l_minMax.second;
    }
    
    /* Min-max normalisation of ODF */
    for(unsigned int i = 0; i < ODF.size(); i++)
    {
        if((max) != 0)
        {
            norm_hemisODF.push_back((ODF[i] - min) /(max - min));
        }
        else
        {
            norm_hemisODF.push_back(0);
        }
    }
    
    bool isCandidate = false;

    /* Find all potential candidate to be a main direction according to the max_threshold */
    for(unsigned int i = 0; i < m_phiThetaDirection[LOD_3].getDimensionY(); i++)
    {
      if(norm_hemisODF[i] > max_thresh)//max_thresh) 
      { 
          //potential maximum
          /* look at other possible direction sampling neighbors
             if a sampling directions is within +- 3 degrees from i,
             we consider it and check if i is bigger */

        isCandidate = true;
        for(unsigned int j=0; j<m_nbors[i].size() && isCandidate ; j++)
        {
            if(norm_hemisODF[i] - norm_hemisODF[m_nbors[i][j].second] < epsilon)
            {
                /* wrong candidate */
                isCandidate = false;
            }
        }
     }

      if(isCandidate)
      {
          float phi   = m_phiThetaDirection[LOD_3](i,0);
          float theta = m_phiThetaDirection[LOD_3](i,1);

          bool isDiff = true;

          Vector dd;
          dd[0] = std::cos(phi)*std::sin(theta);
          dd[1] = std::sin(phi)*std::sin(theta);
          dd[2] = std::cos(theta);
          
          if( max_dir.size() != 0)
          {
              for(unsigned int n=0; n< max_dir.size() && isDiff ; n++)
              {
                  if(dd.x == max_dir[n].x && dd.y == max_dir[n].y && dd.z == max_dir[n].z)
                  {
                      isDiff = false;
                  }
              }
              if(isDiff)
              {
                max_dir.push_back(dd);
              }
          }
          else
          {
                max_dir.push_back(dd);
          }
      }
    }
    m_isMaximasSet = true;
   
    return max_dir;
}

///////////////////////////////////////////////////////////////////////////
// This function will draw the tensors.
///////////////////////////////////////////////////////////////////////////
void ODFs::draw()
{
    // We need VBOs for ODFs, particularly to store the radii
    if( !SceneManager::getInstance()->isUsingVBO() )
        return;

    // Enable the shader.
    ShaderHelper::getInstance()->getOdfsShader()->bind();
    glBindTexture( GL_TEXTURE_1D, m_textureId );

    // This is the color look up table texture.
    ShaderHelper::getInstance()->getOdfsShader()->setUniSampler( "clut", 0 );
    
    // This is the brightness level of the odf.
    ShaderHelper::getInstance()->getOdfsShader()->setUniFloat( "brightness", DatasetInfo::m_brightness );

    // This is the alpha level of the odf.
    ShaderHelper::getInstance()->getOdfsShader()->setUniFloat( "alpha", DatasetInfo::m_alpha );

    // If m_mapOnSphere is true then the color will be set on a sphere instead of a deformed mesh.
    ShaderHelper::getInstance()->getOdfsShader()->setUniInt( "mapOnSphere", ( GLint ) isDisplayShape( SPHERE ) );

    // If m_colorWithPosition is true then the glyph will be colored with the position of the vertex.
    ShaderHelper::getInstance()->getOdfsShader()->setUniInt( "colorWithPos", ( GLint ) m_colorWithPosition );
 
    // Get the radius attribute location
    m_radiusAttribLoc = glGetAttribLocation( ShaderHelper::getInstance()->getOdfsShader()->getId(), "radius" );
    Glyph::draw();

    // Disable the attribute
    glDisableVertexAttribArray( m_radiusAttribLoc);

    // Disable the tensor color shader.
    ShaderHelper::getInstance()->getOdfsShader()->release();
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
    // The first vector represent the voxel center and the second one represent the tensor size.
    if( ! boxInFrustum( Vector( ( i_xVoxel + 0.5f ) * m_voxelSizeX,
                                ( i_yVoxel + 0.5f ) * m_voxelSizeY,
                                ( i_zVoxel + 0.5f ) * m_voxelSizeZ ),
                        Vector( m_voxelSizeX * 0.5f,
                                m_voxelSizeY * 0.5f,
                                m_voxelSizeZ * 0.5f ) ) )
        return;

    // Get the current tensors index in the coeffs's buffer
    int  currentIdx = getGlyphIndex( i_zVoxel, i_yVoxel, i_xVoxel );   
 
    // Odf offset.
    GLfloat l_offset[3];
    getVoxelOffset( i_zVoxel, i_yVoxel, i_xVoxel, l_offset );
    ShaderHelper::getInstance()->getOdfsShader()->setUni3Float( "offset", l_offset );

    // Lets set the min max radii for this odf.
    ShaderHelper::getInstance()->getOdfsShader()->setUni2Float( "radiusMinMax", m_radiiMinMaxMap[currentIdx] );    

    // Enable attribute
    glEnableVertexAttribArray( m_radiusAttribLoc );

    // Radii
    glBindBuffer( GL_ARRAY_BUFFER, m_radiusBuffer[i_axis] );

    // The index of the radii for the current glyph
    int l_radiiIdx = 0;

    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();

    if( i_axis == X_AXIS )
        l_radiiIdx = m_nbPointsPerGlyph * ( i_zVoxel * rows    + i_yVoxel );

    else if( i_axis == Y_AXIS )
        l_radiiIdx = m_nbPointsPerGlyph * ( i_zVoxel * columns + i_xVoxel );

    else if( i_axis == Z_AXIS )
        l_radiiIdx = m_nbPointsPerGlyph * ( i_yVoxel * columns + i_xVoxel );

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
    //if( i_axis == Y_AXIS ) { //Coronal: flip x
    //   l_flippedAxes[0] *= -1.0f;  
    //} 

    // Need a global flip in X on top of that, which is done above

    ShaderHelper::getInstance()->getOdfsShader()->setUni3Float(   "axisFlip",    l_flippedAxes               );
    ShaderHelper::getInstance()->getOdfsShader()->setUniInt( "showAxis", 0 );
    if (isDisplayShape(AXIS))
    {
        ShaderHelper::getInstance()->getOdfsShader()->setUniInt( "showAxis", 1 );
        
        if(m_coefficients[currentIdx][0] != 0)
        {
            for(unsigned int i =0; i < m_mainDirections[currentIdx].size(); i++)
            {
                if(m_mainDirections[currentIdx].size() != 0)
                {  
                    GLfloat l_coloring[3];
                    l_coloring[0] = m_mainDirections[currentIdx][i][0];
                    l_coloring[1] = m_mainDirections[currentIdx][i][1];
                    l_coloring[2] = m_mainDirections[currentIdx][i][2];
                    float halfScale = m_scalingFactor / 5.0f;

                    ShaderHelper::getInstance()->getOdfsShader()->setUni3Float( "coloring", l_coloring );
                    glBegin(GL_LINES);  
                        glVertex3f(-halfScale*m_mainDirections[currentIdx][i][0],-halfScale*m_mainDirections[currentIdx][i][1],-halfScale*m_mainDirections[currentIdx][i][2]);
                        glVertex3f(halfScale*m_mainDirections[currentIdx][i][0],halfScale*m_mainDirections[currentIdx][i][1],halfScale*m_mainDirections[currentIdx][i][2]);       
                    glEnd();
                }
            }
        }
    }
    else
    {
        ShaderHelper::getInstance()->getOdfsShader()->setUniInt( "swapRadius", 0 );
        // Draw the first half of the odfs.
        glDrawArrays( GL_TRIANGLE_STRIP, 0, m_nbPointsPerGlyph );

        // Lets set the radius modifier.
        ShaderHelper::getInstance()->getOdfsShader()->setUniInt( "swapRadius", 1 );
        // Draw the other half of the odfs.
        glDrawArrays( GL_TRIANGLE_STRIP, 0, m_nbPointsPerGlyph );

    }
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

    int rows    = DatasetManager::getInstance()->getRows();
    int frames  = DatasetManager::getInstance()->getFrames();

    for( int z( 0 ); z < frames; ++z )
    {
        for( int y( 0 ); y < rows; ++y )
        {
            l_idx = getGlyphIndex( z, y, m_currentSliderPos[0] );

            // Get radius
            computeRadiiArray( m_shMatrix[m_currentLOD], m_coefficients[l_idx], l_radius, l_minMax );

            m_radiiMinMaxMap[l_idx] = l_minMax;

            m_radius[X_AXIS].insert( m_radius[X_AXIS].end(), l_radius.begin(), l_radius.end() );
        }
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

    int columns = DatasetManager::getInstance()->getColumns();
    int frames  = DatasetManager::getInstance()->getFrames();

    for( int z( 0 ); z < frames; ++z )
    {
        for( int x( 0 ); x < columns; ++x )
        {
            l_idx = getGlyphIndex( z, m_currentSliderPos[1], x );

            // Get radius
            computeRadiiArray( m_shMatrix[m_currentLOD], m_coefficients[l_idx], l_radius, l_minMax );

            m_radiiMinMaxMap[l_idx] = l_minMax;

            m_radius[Y_AXIS].insert( m_radius[Y_AXIS].end(), l_radius.begin(), l_radius.end() );
        }
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

    int columns = DatasetManager::getInstance()->getColumns();
    int rows    = DatasetManager::getInstance()->getRows();

    for( int y( 0 ); y < rows; ++y )
    {
        for( int x( 0 ); x < columns; ++x )
        {
            l_idx = getGlyphIndex( m_currentSliderPos[2], y, x );

            // Get radius
            computeRadiiArray( m_shMatrix[m_currentLOD], m_coefficients[l_idx], l_radius, l_minMax );

            m_radiiMinMaxMap[l_idx] = l_minMax;

            m_radius[Z_AXIS].insert( m_radius[Z_AXIS].end(), l_radius.begin(), l_radius.end() );
        }
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
    if( !SceneManager::getInstance()->isUsingVBO() )
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
    if( Logger::getInstance()->printIfGLError( wxT( "Initialize vbo points for tensors" ) ) )
    {
        SceneManager::getInstance()->setUsingVBO( false );
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

        if(std::abs(i_meshPts[ i * 3     ]) < 1e-5)
            l_cartesianDir[0] = 0;
        else
            l_cartesianDir[0] = i_meshPts[ i * 3     ];
        

        if(std::abs(i_meshPts[ i * 3+1     ]) < 1e-5)
            l_cartesianDir[1] = 0;
        else
            l_cartesianDir[1] = i_meshPts[ i * 3+1     ];


        if(std::abs(i_meshPts[ i * 3+2 ]) < 1e-5)
            l_cartesianDir[2] = 0;
        else
            l_cartesianDir[2] = i_meshPts[ i * 3+2     ];


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

                    o_shMatrix(i, j) = sign*real( cplx_2 );

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

        if(std::abs(i_meshPts[ i * 3     ]) < 1e-5)
            l_cartesianDir[0] = 0;
        else
            l_cartesianDir[0] = i_meshPts[ i * 3     ];


        if(std::abs(i_meshPts[ i * 3+1     ]) < 1e-5)
            l_cartesianDir[1] = 0;
        else
            l_cartesianDir[1] = i_meshPts[ i * 3+1     ];


        if(std::abs(i_meshPts[ i * 3+2 ]) < 1e-5)
            l_cartesianDir[2] = 0;
        else
            l_cartesianDir[2] = i_meshPts[ i * 3+2     ];


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

        if(std::abs(i_meshPts[ i * 3     ]) < 1e-5)
            l_cartesianDir[0] = 0;
        else
            l_cartesianDir[0] = i_meshPts[ i * 3     ];
        

        if(std::abs(i_meshPts[ i * 3+1     ]) < 1e-5)
            l_cartesianDir[1] = 0;
        else
            l_cartesianDir[1] = i_meshPts[ i * 3+1     ];


        if(std::abs(i_meshPts[ i * 3+2 ]) < 1e-5)
            l_cartesianDir[2] = 0;
        else
            l_cartesianDir[2] = i_meshPts[ i * 3+2     ];

        /*l_cartesianDir[0] = i_meshPts[ i * 3     ];
        l_cartesianDir[1] = i_meshPts[ i * 3 + 1 ];
        l_cartesianDir[2] = i_meshPts[ i * 3 + 2 ];*/

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

               if( m >= 0 ) {
                  o_shMatrix(i,j) = real(cplx_1);
               }
               else { // /* negative "m" SH  */
                  o_shMatrix(i,j) = pow(-(float)1, m+1)*imag(cplx_1);
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
      
      getSphericalHarmonicMatrixRR5768(i_meshPts, o_phiThetaDirection, o_shMatrix );
   }
   else if( m_sh_basis == 1 ) {
      
      getSphericalHarmonicMatrixDescoteauxThesis(i_meshPts, o_phiThetaDirection, o_shMatrix );
   }
   else if( m_sh_basis == 2 ) {
      
      getSphericalHarmonicMatrixTournier(i_meshPts, o_phiThetaDirection, o_shMatrix );
   }
   else if( m_sh_basis == 3 ) {
      
      getSphericalHarmonicMatrixPTK(i_meshPts, o_phiThetaDirection, o_shMatrix );
   }
   else
      getSphericalHarmonicMatrixRR5768(i_meshPts, o_phiThetaDirection, o_shMatrix );

   m_isMaximasSet = false;
   
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
/*
    Allows the users to switch between different SH basis (ODFs)
*/
void ODFs::changeShBasis( SH_BASIS basis )
{
    nifti_image *pHeader = nifti_image_read( m_lastODF_path.char_str(), 0 );
    nifti_image *pBody   = nifti_image_read( m_lastODF_path.char_str(), 1 );

    if( NULL == pHeader || NULL == pBody )
    {
        Logger::getInstance()->print( wxT( "nifti file corrupt, cannot create nifti image from header" ), LOGLEVEL_ERROR );
        return;
    }

    ODFs tmp( m_lastODF_path );
    tmp.setShBasis( basis );

    if( tmp.load( pHeader, pBody ) )
    {
        swap( tmp );
        updatePropertiesSizer();
    }

    nifti_image_free( pHeader );
    nifti_image_free( pBody );
}
///////////////////////////////////////////////////////////////////////////
// This function will set a specific scaling factor for the glyph.
//
// i_scalingFactor      : The glyph scaling factor.
///////////////////////////////////////////////////////////////////////////
void ODFs::setScalingFactor( float i_scalingFactor )
{
    m_scalingFactor = i_scalingFactor;
    generateSpherePoints( m_scalingFactor / 5 );
    loadBuffer();
}

void ODFs::createPropertiesSizer( PropertiesWindow *pParent )
{
    Glyph::createPropertiesSizer( pParent );

    setColorWithPosition( true );

    wxBoxSizer *pBoxMain = new wxBoxSizer( wxVERTICAL );

    //////////////////////////////////////////////////////////////////////////

    m_pSliderFlood = new MySlider(     pParent, wxID_ANY, 2, 0, 10,    DEF_POS, wxSize( 100, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS);
    m_pTxtThres    = new wxTextCtrl(   pParent, wxID_ANY, wxT( "0.2"), DEF_POS, wxSize(  40, -1 ), wxTE_READONLY);
    m_pLblThres    = new wxStaticText( pParent, wxID_ANY, wxT( "Threshold" ) );
    m_pBtnMainDir  = new wxButton(     pParent, wxID_ANY, wxT( "Recalculate" ), DEF_POS, wxSize( 140, -1 ) );
    wxRadioButton *pRadDescoteauxBasis = new wxRadioButton( pParent, wxID_ANY, wxT( "Descoteaux" ), DEF_POS, DEF_SIZE, wxRB_GROUP );
    wxRadioButton *pRadTournierBasis   = new wxRadioButton( pParent, wxID_ANY, wxT( "MRtrix" ) );
//     wxRadioButton *pRadOriginalBasis   = new wxRadioButton( pParent, wxID_ANY, wxT( "RR5768" ) );
//     wxRadioButton *pRadPTKBasis        = new wxRadioButton( pParent, wxID_ANY, wxT( "PTK" ) );

    //////////////////////////////////////////////////////////////////////////

    wxBoxSizer *pBoxFlood = new wxBoxSizer( wxHORIZONTAL );
    pBoxFlood->Add( m_pLblThres,   0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxFlood->Add( m_pSliderFlood, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxFlood->Add( m_pTxtThres, 0, wxFIXED_MINSIZE | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxMain->Add( pBoxFlood, 0, wxEXPAND, 0 );

    //////////////////////////////////////////////////////////////////////////

    pBoxMain->Add( m_pBtnMainDir, 0, wxALIGN_CENTER | wxEXPAND | wxRIGHT | wxLEFT, 24 );

    wxBoxSizer *pBoxShBasis = new wxBoxSizer( wxVERTICAL );
    pBoxShBasis->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Sh Basis:" ) ), 0, wxALIGN_LEFT | wxALL, 1 );

    wxBoxSizer *pBoxShBasisRadios = new wxBoxSizer( wxVERTICAL );
//     pBoxShBasisRadios->Add( pRadOriginalBasis,   0, wxALIGN_LEFT | wxALL, 1 );
    pBoxShBasisRadios->Add( pRadDescoteauxBasis, 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxShBasisRadios->Add( pRadTournierBasis,   0, wxALIGN_LEFT | wxALL, 1 );
//     pBoxShBasisRadios->Add( pRadPTKBasis,        0, wxALIGN_LEFT | wxALL, 1 );
    pBoxShBasis->Add( pBoxShBasisRadios, 0, wxALIGN_LEFT | wxLEFT, 32 );

    pBoxMain->Add( pBoxShBasis, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    //////////////////////////////////////////////////////////////////////////

    pParent->Connect( m_pSliderFlood->GetId(),      wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnSliderAxisMoved ) );
    pParent->Connect( m_pBtnMainDir->GetId(),       wxEVT_COMMAND_BUTTON_CLICKED,       wxCommandEventHandler( PropertiesWindow::OnRecalcMainDir ) );
//     pParent->Connect( pRadOriginalBasis->GetId(),   wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnOriginalShBasis ) );
    pParent->Connect( pRadDescoteauxBasis->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnDescoteauxShBasis ) );
    pParent->Connect( pRadTournierBasis->GetId(),   wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnTournierShBasis ) );
//     pParent->Connect( pRadPTKBasis->GetId(),        wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnPTKShBasis ) );

    //////////////////////////////////////////////////////////////////////////

//     pRadOriginalBasis->SetValue(   isShBasis( SH_BASIS_RR5768 ) );
    pRadDescoteauxBasis->SetValue( isShBasis( SH_BASIS_DESCOTEAUX ) );
    pRadTournierBasis->SetValue(   isShBasis( SH_BASIS_TOURNIER ) );
//     pRadPTKBasis->SetValue(        isShBasis( SH_BASIS_PTK ) );

    m_pSliderLightAttenuation->SetValue( m_pSliderLightAttenuation->GetMin() );
    m_pSliderLightXPosition->SetValue( m_pSliderLightXPosition->GetMin() );
    m_pSliderLightYPosition->SetValue( m_pSliderLightYPosition->GetMin() );
    m_pSliderLightZPosition->SetValue( m_pSliderLightZPosition->GetMin() );

    //////////////////////////////////////////////////////////////////////////

    m_pPropertiesSizer->Add( pBoxMain, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );
}

void ODFs::updatePropertiesSizer()
{
//     Glyph::updatePropertiesSizer();
    DatasetInfo::updatePropertiesSizer();

    m_pSliderLightAttenuation->Enable( false );
    m_pSliderLightXPosition->Enable( false );
    m_pSliderLightYPosition->Enable( false );
    m_pSliderLightZPosition->Enable( false );
    m_pBtnFlipX->Enable( false );
    m_pBtnFlipY->Enable( false );
    m_pBtnFlipZ->Enable( false );

    m_pSliderMinHue->SetValue(     getColor( MIN_HUE )    * 100 );
    m_pSliderMaxHue->SetValue(     getColor( MAX_HUE )    * 100 );
    m_pSliderSaturation->SetValue( getColor( SATURATION ) * 100 );
    m_pSliderLuminance->SetValue(  getColor( LUMINANCE )  * 100 );
    m_pSliderLOD->SetValue(        (int)getLOD() );
    m_pSliderDisplay->SetValue(    getDisplayFactor() );
    m_pSliderScalingFactor->SetValue( getScalingFactor() * 10.0f );

    m_pToggleAxisFlipX->SetValue( isAxisFlipped( X_AXIS ) );
    m_pToggleAxisFlipY->SetValue( isAxisFlipped( Y_AXIS ) );
    m_pToggleAxisFlipZ->SetValue( isAxisFlipped( Z_AXIS ) );
    m_pToggleColorWithPosition->SetValue( getColorWithPosition() );

    //m_psliderScalingFactor->SetValue(m_psliderScalingFactor->GetMin());

    if( !isDisplayShape( AXIS ) )
    {
        m_pLblThres->Hide();
        m_pSliderFlood->Hide();
        m_pTxtThres->Hide();
        m_pBtnMainDir->Hide();
    }
    else
    {
        m_pLblThres->Show();
        m_pSliderFlood->Show();
        m_pTxtThres->Show();
        m_pBtnMainDir->Show();
    }
}

void ODFs::swap( ODFs &o )
{
    // Not swaping GUI elements
    Glyph::swap( o );
    std::swap( m_lastODF_path, o.m_lastODF_path );
    std::swap( m_isMaximasSet, o.m_isMaximasSet );
    std::swap( m_axisThreshold, o.m_axisThreshold );
    std::swap( m_order, o.m_order );
    std::swap( m_radiusAttribLoc, o.m_radiusAttribLoc );
    std::swap( m_radiusBuffer, o.m_radiusBuffer );
    std::swap( m_coefficients, o.m_coefficients );
    std::swap( m_radius, o.m_radius );
    std::swap( m_shMatrix, o.m_shMatrix );
    std::swap( m_phiThetaDirection, o.m_phiThetaDirection );
    std::swap( m_meshPts, o.m_meshPts );
    std::swap( m_radiiMinMaxMap, o.m_radiiMinMaxMap );
    std::swap( m_angle_min, o.m_angle_min );
    std::swap( m_nbors, o.m_nbors );
    std::swap( m_mainDirections, o.m_mainDirections );
    std::swap( m_sh_basis, o.m_sh_basis );
}
