/////////////////////////////////////////////////////////////////////////////
// Name:            Glyph.cpp
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   11/13/2009
//
// Description: This is the implementation file for the abstract Glyph class.
//
// Last modifications:
//      by : girardg - 28/12/2010
/////////////////////////////////////////////////////////////////////////////

#include "Glyph.h"

#include "DatasetManager.h"
#include "../Logger.h"
#include "../gui/MainFrame.h"
#include "../gui/SceneManager.h"
#include "../misc/nifti/nifti1_io.h"

#include <GL/glew.h>
#include <wx/tglbtn.h>

#include <algorithm>
#include <vector>
using std::vector;

#define DEF_POS wxDefaultPosition
#define DEF_SIZE     wxDefaultSize

///////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////
Glyph::Glyph( float i_minHue     , 
              float i_maxHue     , 
              float i_saturation , 
              float i_luminance )
:   DatasetInfo             (),
    m_hemisphereBuffer      ( NULL ),
    m_textureId             ( 0 ),    
    m_nbPointsPerGlyph      ( 0 ),    
    m_nbGlyphs              ( 0 ),
    m_displayFactor         ( 1 ),     // By default we display all the glyphs.
    m_axisFlippedToggled    ( false ),    
    m_displayShape          ( NORMAL ), // By default we want the glyph to be deformed.
    m_colorWithPosition     ( false ),
    m_colorMinHue           ( i_minHue ),
    m_colorMaxHue           ( i_maxHue ),
    m_colorSaturation       ( i_saturation ),
    m_colorLuminance        ( i_luminance ),
    m_lighAttenuation       ( 0.5f ),
    m_currentLOD            ( LOD_2 )
{
    generateColorTexture( m_colorMinHue, m_colorMaxHue, m_colorSaturation, m_colorLuminance );

    m_currentSliderPos[0] = m_currentSliderPos[1] = m_currentSliderPos[2] = 0.0f;
    m_flippedAxes[0]      = m_flippedAxes[1]      = m_flippedAxes[2]      = false;
    m_lightPosition[0]    = m_lightPosition[1]    = m_lightPosition[2]    = 1.0f;  

}

///////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////
Glyph::~Glyph()
{
    Logger::getInstance()->print( wxT( "Executing Glyph destructor..." ), LOGLEVEL_DEBUG );
    if( SceneManager::getInstance()->isUsingVBO() && m_hemisphereBuffer )
    {
        glDeleteBuffers( 1, m_hemisphereBuffer );
        delete m_hemisphereBuffer;
    }
    Logger::getInstance()->print( wxT( "Glyph destructor done." ), LOGLEVEL_DEBUG );
}

///////////////////////////////////////////////////////////////////////////
// This function will draw the glyphs.
///////////////////////////////////////////////////////////////////////////
void Glyph::draw()
{      
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_CULL_FACE );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    if( SceneManager::getInstance()->isAxialDisplayed() )
        drawAxial();

    if( SceneManager::getInstance()->isCoronalDisplayed() )
        drawCoronal();

    if( SceneManager::getInstance()->isSagittalDisplayed() )
        drawSagittal();

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_CULL_FACE );
    glDisable( GL_BLEND );
}

///////////////////////////////////////////////////////////////////////////
// This function will display the Axial slice of the loaded glyphs.
///////////////////////////////////////////////////////////////////////////
void Glyph::drawAxial()
{
    for( int y( 0 ); y < m_rows; ++y )
    {
        for( int x( 0 ); x < m_columns; ++x )
        {
            // We only draw the glyphs if the test for our display factor is successful.
            if( ( x + y ) % m_displayFactor == 0 )
            {
                drawGlyph( m_currentSliderPos[2], y, x, Z_AXIS );
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will display the Coronal slice of the loaded glyphs.
///////////////////////////////////////////////////////////////////////////
void Glyph::drawCoronal()
{
    for( int z( 0 ); z < m_frames; ++z )
    {
        for( int x( 0 ); x < m_columns; ++x )
        {
            // We only draw the glyphs if the test for our display factor is successful.
            if( ( x + z ) % m_displayFactor == 0 )
            {
                drawGlyph( z, m_currentSliderPos[1], x, Y_AXIS );
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will display the Sagittal slice of the loaded glyphs.
///////////////////////////////////////////////////////////////////////////
void Glyph::drawSagittal()
{
    for( int z( 0 ); z < m_frames; ++z )
    {
        for( int y( 0 ); y < m_rows; ++y )
        {
            // We only draw the glyphs if the test for our display factor is successful.
            if( ( y + z ) % m_displayFactor == 0 )
            {
                drawGlyph( z, y, m_currentSliderPos[0], X_AXIS );
            }
        }
    }
}

void Glyph::drawSemiAll()
{
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_CULL_FACE );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    int disp = 4/m_voxelSizeX;

    for( int z( 0 ); z < m_frames; z+=disp )
    {
        for( int y( 0 ); y < m_rows; y+=disp )
        {
            for( int x( 0 ); x < m_columns; x+=disp )
            {
                drawGlyph( z, y, x, X_AXIS );
            }
        }
    }

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_CULL_FACE );
    glDisable( GL_BLEND );
}

///////////////////////////////////////////////////////////////////////////
// This function will set the current sliders values.
///////////////////////////////////////////////////////////////////////////
void Glyph::refreshSlidersValues()
{
    // Fetching the current sliders positions
    int l_currentPos[3];
    getSlidersPositions( l_currentPos );

    // For all 3 axes
    for( int i = 0; i < 3; ++i )
    {        
        if( m_currentSliderPos[i] != l_currentPos[i] )
        {
            m_currentSliderPos[i] = l_currentPos[i];
            sliderPosChanged( (AxisType)i );
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function computes the current sliders positions (all 3 axes)
///////////////////////////////////////////////////////////////////////////
void Glyph::getSlidersPositions( int o_slidersPos[3] )
{
    // For the X axis.
    float l_xSliderRatio  = SceneManager::getInstance()->getSliceX() / ( DatasetManager::getInstance()->getColumns() - 1 );
    o_slidersPos[0]       = l_xSliderRatio * ( m_columns - 1 );

    // For the Y axis.
    float l_ySliderRatio  = SceneManager::getInstance()->getSliceY() / ( DatasetManager::getInstance()->getRows() - 1 );
    o_slidersPos[1]       = l_ySliderRatio * ( m_rows - 1 );

    // For the Z axis.
    float l_zSliderRatio  = SceneManager::getInstance()->getSliceZ() / ( DatasetManager::getInstance()->getFrames() - 1 );    
    o_slidersPos[2]       = l_zSliderRatio * ( m_frames - 1 );
}

///////////////////////////////////////////////////////////////////////////
// This function will get the index of a glyph at a specified position.
//
// i_zVoxel         : The z voxel of the glyph.
// i_yVoxel         : The y voxel of the glyph.
// i_xVoxel         : The x voxel of the glyph.
///////////////////////////////////////////////////////////////////////////
int Glyph::getGlyphIndex( int i_zVoxel, int i_yVoxel, int i_xVoxel )
{
    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();

    // Get the current tensors index in the coeffs's buffer
    return i_zVoxel * columns * rows + i_yVoxel * columns + i_xVoxel;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// This function will generate a 1D texture from the elements in m_floatColorDataset.
// m_floatColorDataset will be filled filled by the call to fillColorDataset().
// This texture will be used by the shader to do a color lookup table to set the glyphs color.
//
// i_minHue        : The min Hue,    must be [0,1[.
// i_maxHue        : The max Hue,    must be [0,1[.
// i_saturation    : The saturation, must be [0,1].
// i_luminance     : The luminance,  must be [0,1].
////////////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::generateColorTexture( float i_minHue, float i_maxHue, float i_saturation, float i_luminance )
{    
    fillColorDataset( i_minHue, i_maxHue, i_saturation, i_luminance );
    
    glGenTextures( 1, &m_textureId );
    glBindTexture( GL_TEXTURE_1D, m_textureId );

    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    glTexImage1D( GL_TEXTURE_1D, 0, 3, TEXTURE_NB_OF_COLOR, 0, GL_RGB, GL_FLOAT, &m_floatColorDataset[0] );

    // Once the texture is generated, we don't need the information inside m_floatColorDataset anymore. 
    m_floatColorDataset.clear();
}

///////////////////////////////////////////////////////////////////////////
// This function will test if a box is inside the frustum or outside.
//
// i_boxCenter      : The center of the box.
// i_boxSize        : The size of the box (this is the distance from the center
//                    to the side, so its like a radius for a sphere).
//
// Returns TRUE if the box is inside the frustum culling, FALSE otherwise.
// See the tutorial here to understand : http://www.crownandcutlass.com/features/technicaldetails/frustum.html
///////////////////////////////////////////////////////////////////////////
bool Glyph::boxInFrustum( Vector i_boxCenter, Vector i_boxSize )
{
   for( int p = 0; p < 6; ++p )
   {
      if( SceneManager::getInstance()->m_frustum[p][0] * ( i_boxCenter.x - i_boxSize.x ) + 
          SceneManager::getInstance()->m_frustum[p][1] * ( i_boxCenter.y - i_boxSize.y ) + 
          SceneManager::getInstance()->m_frustum[p][2] * ( i_boxCenter.z - i_boxSize.z ) + 
          SceneManager::getInstance()->m_frustum[p][3] > 0 )
         continue;

      if( SceneManager::getInstance()->m_frustum[p][0] * ( i_boxCenter.x + i_boxSize.x ) + 
          SceneManager::getInstance()->m_frustum[p][1] * ( i_boxCenter.y - i_boxSize.y ) + 
          SceneManager::getInstance()->m_frustum[p][2] * ( i_boxCenter.z - i_boxSize.z ) + 
          SceneManager::getInstance()->m_frustum[p][3] > 0 )
         continue;

      if( SceneManager::getInstance()->m_frustum[p][0] * ( i_boxCenter.x - i_boxSize.x ) + 
          SceneManager::getInstance()->m_frustum[p][1] * ( i_boxCenter.y + i_boxSize.y ) + 
          SceneManager::getInstance()->m_frustum[p][2] * ( i_boxCenter.z - i_boxSize.z ) + 
          SceneManager::getInstance()->m_frustum[p][3] > 0 )
         continue;

      if( SceneManager::getInstance()->m_frustum[p][0] * ( i_boxCenter.x + i_boxSize.x ) + 
          SceneManager::getInstance()->m_frustum[p][1] * ( i_boxCenter.y + i_boxSize.y ) + 
          SceneManager::getInstance()->m_frustum[p][2] * ( i_boxCenter.z - i_boxSize.z ) + 
          SceneManager::getInstance()->m_frustum[p][3] > 0 )
         continue;

      if( SceneManager::getInstance()->m_frustum[p][0] * ( i_boxCenter.x - i_boxSize.x ) + 
          SceneManager::getInstance()->m_frustum[p][1] * ( i_boxCenter.y - i_boxSize.y ) + 
          SceneManager::getInstance()->m_frustum[p][2] * ( i_boxCenter.z + i_boxSize.z ) + 
          SceneManager::getInstance()->m_frustum[p][3] > 0 )
         continue;

      if( SceneManager::getInstance()->m_frustum[p][0] * ( i_boxCenter.x + i_boxSize.x ) + 
          SceneManager::getInstance()->m_frustum[p][1] * ( i_boxCenter.y - i_boxSize.y ) + 
          SceneManager::getInstance()->m_frustum[p][2] * ( i_boxCenter.z + i_boxSize.z ) + 
          SceneManager::getInstance()->m_frustum[p][3] > 0 )
         continue;

      if( SceneManager::getInstance()->m_frustum[p][0] * ( i_boxCenter.x - i_boxSize.x ) + 
          SceneManager::getInstance()->m_frustum[p][1] * ( i_boxCenter.y + i_boxSize.y ) + 
          SceneManager::getInstance()->m_frustum[p][2] * ( i_boxCenter.z + i_boxSize.z ) + 
          SceneManager::getInstance()->m_frustum[p][3] > 0 )
         continue;

      if( SceneManager::getInstance()->m_frustum[p][0] * ( i_boxCenter.x + i_boxSize.x ) + 
          SceneManager::getInstance()->m_frustum[p][1] * ( i_boxCenter.y + i_boxSize.y ) + 
          SceneManager::getInstance()->m_frustum[p][2] * ( i_boxCenter.z + i_boxSize.z ) + 
          SceneManager::getInstance()->m_frustum[p][3] > 0 )
         continue;

      return false;
   }
   return true;
}

///////////////////////////////////////////////////////////////////////////
// This function will flip the sign of all the values for 1 axis for the all 
// the glyphs points.
//
// i_axisIdentifier         : Determines on what axis we wat to do the flip.
// i_isFlipped              : Determines if the item is checked or not.
///////////////////////////////////////////////////////////////////////////
void Glyph::flipAxis( AxisType i_axisType, bool i_isFlipped )
{
    // This will be use to know if we need to do some front or back face culling.
    m_axisFlippedToggled = !m_axisFlippedToggled;

    // Update what axis is flipped or not.
    switch( i_axisType )
    {
        case X_AXIS : m_flippedAxes[0] = i_isFlipped; break;
        case Y_AXIS : m_flippedAxes[1] = i_isFlipped; break;
        case Z_AXIS : m_flippedAxes[2] = i_isFlipped; break;
        default     : break;
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will let anyone who wants to know if the axis specified by
// the parameter is currently flipped or not.
//
// i_axisType           : The axis type we want to get the info for.
//
// Returns TRUE if the axis specified by i_axisType is flipped, FALSE otherwise.
///////////////////////////////////////////////////////////////////////////
bool Glyph::isAxisFlipped( AxisType i_axisType )
{
    switch( i_axisType )
    {
        case X_AXIS : return m_flippedAxes[0];
        case Y_AXIS : return m_flippedAxes[1];
        case Z_AXIS : return m_flippedAxes[2];
        default     : return false;
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will return an offset in x,y,z that will indicate the center
// of this voxel in world pixel coordinate.
//
// i_zVoxelIndex        : The Z index of the voxel.
// i_yVoxelIndex        : The Y index of the voxel.
// i_xVoxelIndex        : The X index of the voxel.
// o_offset             : The output vector containing the offset values.
///////////////////////////////////////////////////////////////////////////
void Glyph::getVoxelOffset( int i_zVoxelIndex, int i_yVoxelIndex, int i_xVoxelIndex, float o_offset[3] )
{
    // The offset values is to return the points in the pixel world and not in the voxel world.
    // The + 0.5f is because we want to place the glyph in the middle of its voxel.
    o_offset[2] = ( i_zVoxelIndex + 0.5f ) * m_voxelSizeZ;
    o_offset[1] = ( i_yVoxelIndex + 0.5f ) * m_voxelSizeY;
    o_offset[0] = ( i_xVoxelIndex + 0.5f ) * m_voxelSizeX;
}

///////////////////////////////////////////////////////////////////////////
// This function will fill the m_floatColorDataset vectot with a raybow type or colors.
// The colors are created from a fixed saturation value (i_saturation) and a fix luminance
// value (i_luminance) but with a varying hue value (varying from [i_minHue, i_maxHue].
//
// i_minHue        : The min Hue.
// i_maxHue        : The max Hue.
// i_saturation    : The saturation.
// i_luminance     : The luminance.
///////////////////////////////////////////////////////////////////////////
void Glyph::fillColorDataset( float i_minHue, float i_maxHue, float i_saturation, float i_luminance )
{  
    // This is to make sure that we will generate a texture no matter
    // how messed up the i_minHue and i_maxHue values are.
    // Make sure that there is enough distance between the i_minHue and the i_maxHue.
    if( fabs( i_maxHue - i_minHue ) < HUE_MINIMUM_DISTANCE )
    {
        if( i_maxHue >= i_minHue )
        {
            if( i_minHue < HUE_MINIMUM_DISTANCE )
                i_maxHue += HUE_MINIMUM_DISTANCE;
            else 
                i_minHue -= HUE_MINIMUM_DISTANCE;
        }
        else
        {
            if( i_maxHue < HUE_MINIMUM_DISTANCE )
                i_minHue += HUE_MINIMUM_DISTANCE;
            else 
                i_maxHue -= HUE_MINIMUM_DISTANCE;
        }        
    }

    float l_hueIncrement = ( i_maxHue - i_minHue ) / TEXTURE_NB_OF_COLOR;

    m_floatColorDataset.resize( TEXTURE_NB_OF_COLOR * 3 );

    float l_hue = i_minHue;
    float r, g, b;

    for( int i = 0; i < TEXTURE_NB_OF_COLOR; ++i, l_hue += l_hueIncrement )
    {
        r = g = b = 0;
        Helper::HSLtoRGB( l_hue, i_saturation, i_luminance, r, g, b );
        m_floatColorDataset[i*3]     = r;
        m_floatColorDataset[i*3 + 1] = g;
        m_floatColorDataset[i*3 + 2] = b;
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the light direction for a specific axis.
//
// i_axis           : The axis we want to set the light position for.
// i_position       : The position of the light.
///////////////////////////////////////////////////////////////////////////
void Glyph::setLightPosition( AxisType i_axis, float i_position )
{
    switch( i_axis )
    {
        case X_AXIS : m_lightPosition[0] = i_position; break;
        case Y_AXIS : m_lightPosition[1] = i_position; break;
        case Z_AXIS : m_lightPosition[2] = i_position; break;
        default     : break;
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will get the light direction for a specific axis.
//
// i_axis           : The axis we want to get the light position for.
//
// Returns the light position for a specific axis.
///////////////////////////////////////////////////////////////////////////
float Glyph::getLightPosition( AxisType i_axis )
{
    switch( i_axis )
    {
        case X_AXIS : return m_lightPosition[0];
        case Y_AXIS : return m_lightPosition[1];
        case Z_AXIS : return m_lightPosition[2];
        default     : return 1.0f;
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set a specific glyph color param.
//
// i_colorModifier      : The glyph color param we want to modify.
// i_value              : The value of the param we want to modify.
///////////////////////////////////////////////////////////////////////////
void Glyph::setColor( GlyphColorModifier i_colorModifier, float i_value )
{
    switch( i_colorModifier )
    {
        case MIN_HUE    : m_colorMinHue     = i_value; break;
        case MAX_HUE    : m_colorMaxHue     = i_value; break;
        case SATURATION : m_colorSaturation = i_value; break;
        case LUMINANCE  : m_colorLuminance  = i_value; break;
        default         : return; // Invalid param: no point in calling generateColorTexture()
    }

    // As soon as a parameter for the texture color is changed, we want to regenerate the 
    // color texture to update the glyph color.
    generateColorTexture( m_colorMinHue, m_colorMaxHue, m_colorSaturation, m_colorLuminance );
}

///////////////////////////////////////////////////////////////////////////
// This function will get a specific glyph color param.
//
// i_colorModifier      : The glyph color param we want to get.
//
// Returns the value of the glyph color modifier we wanted to get or -1 if 
// the param was not good.
///////////////////////////////////////////////////////////////////////////
float Glyph::getColor( GlyphColorModifier i_colorModifier )
{
     switch( i_colorModifier )
    {
        case MIN_HUE    : return m_colorMinHue;
        case MAX_HUE    : return m_colorMaxHue;
        case SATURATION : return m_colorSaturation;
        case LUMINANCE  : return m_colorLuminance;
        default         : return -1;
    }
}

///////////////////////////////////////////////////////////////////////////
// Generate the spheres for all the LODs and saves them inside m_LODspheres.
//
// i_scalingFactor      : A scaling factor.
///////////////////////////////////////////////////////////////////////////
void Glyph::generateSpherePoints( float i_scalingFactor )
{
    vector< float > l_spherePts;

    m_LODspheres.clear();
    m_LODspheres.reserve( NB_OF_LOD );

    for( unsigned int i = 0; i < NB_OF_LOD; ++i )
    {
        // Fetching the sphere points
        getSpherePoints( (LODChoices)i, i_scalingFactor, l_spherePts );

        // Current LOD sphere
        m_LODspheres.push_back( l_spherePts );
        l_spherePts.clear();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This function will fill up o_spherePoints with points representing a perfect
// sphere at a certain LOD and with a certain scaling factor.
//
// i_LOD                : A LOD in which we will generate our perfect sphere.
//                        LOD_0 is the lowest LOD = less points, LOD_3 is the highest LOD = more points.
// i_scalingFactor      : This values is a simple way to make our tensor a bit bigger that they would be by default.
// o_spherePoints       : The output vector containing the sphere points. in this order [x1,y1,z1,x2,y2,z2,x3,y3,z3...]
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::getSpherePoints( LODChoices       i_LOD,
                             float            i_scalingFactor,
                             vector< float > &o_spherePoints  )
{
    int l_lats  = 0;
    int l_longs = 0;

    switch( i_LOD )
    {
        case LOD_0 : l_lats  = 4;  l_longs = 4;  break; // For 30   points.
        case LOD_1 : l_lats  = 6;  l_longs = 6;  break; // For 56   points.
        case LOD_2 : l_lats  = 10; l_longs = 10; break; // For 132  points.
        case LOD_3 : l_lats  = 14; l_longs = 14; break; // For 240  points.
        case LOD_4 : l_lats  = 20; l_longs = 20; break; // For 462  points.
        case LOD_5 : l_lats  = 28; l_longs = 28; break; // For 870  points.
        case LOD_6 : l_lats  = 32; l_longs = 32; break; // For 1122 points.
        default: return; // We do nothing incase the param was not good.
    } 

    // We do not pass the same number of lats and longs because we want to form a half sphere.
    Helper::createSphere( l_lats, l_longs, l_lats / 2, l_longs, o_spherePoints );

    if( i_scalingFactor != 0 )
        // We scale our points.
        for( unsigned i = 0; i < o_spherePoints.size(); ++i )
            o_spherePoints[i] *= i_scalingFactor;
}

///////////////////////////////////////////////////////////////////////////
// This function will return the number of points representing a tensor at a specific LOD.
//
// i_LODChoices            : The specific LOD we want to get the number of points info of.
//
// Returns the number of points a tensor with i_LODChoices will have 
// or -1 if i_LODChoices is not a valid LOD choice.
///////////////////////////////////////////////////////////////////////////
int Glyph::getLODNbOfPoints( const LODChoices i_LODChoices )
{
    switch( i_LODChoices )
    {
        case LOD_0 : return 30;   // For 4, 4.
        case LOD_1 : return 56;   // For 6, 6.
        case LOD_2 : return 132;  // For 10, 10.
        case LOD_3 : return 240;  // For 14, 14.
        case LOD_4 : return 462;  // For 20, 20.
        case LOD_5 : return 870;  // For 28, 28.
        case LOD_6 : return 1122; // For 32, 32.
        default    : return -1;
    }
}

///////////////////////////////////////////////////////////////////////////
// Sets a new current LOD value
//
// i_LOD : new LOD value
///////////////////////////////////////////////////////////////////////////
void Glyph::setLOD( LODChoices i_LOD ) 
{ 
    if( m_currentLOD != i_LOD )
    {
        m_currentLOD = i_LOD;
        
        // Set the number of points per glyph.
        m_nbPointsPerGlyph = getLODNbOfPoints( i_LOD );

        loadBuffer();
    }
}

///////////////////////////////////////////////////////////////////////////
// This function initializes the array buffer containing the hemishere points 
// that will be load in a buffer in video memory. If an error occur while 
// performing this operation then we will set m_useVBO to false, to indicate
// that we will not use VBO. 
//////////////////////////////////////////////////////////////////////////
void Glyph::loadBuffer()
{
    // We need to (re)load the buffer in video memory only if we are using VBO.
    if( !SceneManager::getInstance()->isUsingVBO() )
        return;        

    // Sphere buffers
    // Generating buffer name.
    if( m_hemisphereBuffer == NULL )
    {
        m_hemisphereBuffer = new GLuint();
        glGenBuffers( 1, m_hemisphereBuffer );
    }

    glBindBuffer( GL_ARRAY_BUFFER, *m_hemisphereBuffer );

    glBufferData( GL_ARRAY_BUFFER, 
        m_LODspheres[m_currentLOD].size() * sizeof( GLfloat ), 
        &m_LODspheres[m_currentLOD][0], 
        GL_STATIC_DRAW );    

    // There was a problem loading this buffer into video memory!
    if( Logger::getInstance()->printIfGLError( wxT( "Initialize vbo points for tensors" ) ) )
    {
        SceneManager::getInstance()->setUsingVBO( false );
        delete m_hemisphereBuffer;
    }
}

//////////////////////////////////////////////////////////////////////////

void Glyph::swap( Glyph &g )
{
    // Not swapping GUI elements
    DatasetInfo::swap( g );
    std::swap( m_hemisphereBuffer, g.m_hemisphereBuffer );
    std::swap( m_textureId, g.m_textureId );
    std::swap( m_nbPointsPerGlyph, g.m_nbPointsPerGlyph );
    std::swap( m_nbGlyphs, g.m_nbGlyphs );
    std::swap( m_displayFactor, g.m_displayFactor );
    std::swap( m_axisFlippedToggled, g.m_axisFlippedToggled );
    std::swap( m_displayShape, g.m_displayShape );
    std::swap( m_colorWithPosition, g.m_colorWithPosition );
    std::swap( m_colorMinHue, g.m_colorMinHue );
    std::swap( m_colorMaxHue, g.m_colorMaxHue );
    std::swap( m_colorSaturation, g.m_colorSaturation );
    std::swap( m_colorLuminance, g.m_colorLuminance );
    std::swap( m_lighAttenuation, g.m_lighAttenuation );
    std::swap( m_scalingFactor, g.m_scalingFactor );
    std::swap( m_currentLOD, g.m_currentLOD );
    std::swap_ranges( m_currentSliderPos, m_currentSliderPos + 3, g.m_currentSliderPos );
    std::swap_ranges( m_flippedAxes, m_flippedAxes + 3, g.m_flippedAxes );
    std::swap_ranges( m_lightPosition, m_lightPosition + 3, g.m_lightPosition );
    std::swap( m_floatColorDataset, g.m_floatColorDataset );
    std::swap( m_axesPoints, g.m_axesPoints );
    std::swap( m_LODspheres, g.m_LODspheres );
}

//////////////////////////////////////////////////////////////////////////

void Glyph::setDisplayShape ( DisplayShape i_displayShape ) 
{
    m_displayShape = i_displayShape;
}

void Glyph::createPropertiesSizer( PropertiesWindow *pParent )
{
    DatasetInfo::createPropertiesSizer( pParent );

    wxBoxSizer *pBoxMain = new wxBoxSizer( wxVERTICAL );

    //////////////////////////////////////////////////////////////////////////

#if !_USE_LIGHT_GUI
    m_pSliderMinHue       = new wxSlider( pParent, wxID_ANY,  0,    0, 100, DEF_POS, wxSize( 150, -1 ) );
    m_pSliderMaxHue       = new wxSlider( pParent, wxID_ANY,  0,    0, 100 );
    m_pSliderSaturation   = new wxSlider( pParent, wxID_ANY,  0,    0, 100 );
    m_pSliderLuminance    = new wxSlider( pParent, wxID_ANY,  0,    0, 100 );
    m_pSliderLightAttenuation  = new wxSlider( pParent, wxID_ANY,  0,    0, 100 );
    m_pSliderLightXPosition    = new wxSlider( pParent, wxID_ANY,  0, -100, 100 );
    m_pSliderLightYPosition    = new wxSlider( pParent, wxID_ANY,  0, -100, 100 );
    m_pSliderLightZPosition    = new wxSlider( pParent, wxID_ANY,  0, -100, 100 );
#endif
    
    // Need to track it to be able to hide it for Maximas.
    m_pLabelDisplay        = new wxStaticText( pParent, wxID_ANY, wxT( "Display" ) );
    m_pSliderDisplay       = new wxSlider( pParent, wxID_ANY,  0,    1,  20 );
    m_pSliderScalingFactor = new wxSlider( pParent, wxID_ANY, 50,    1, 200 );
    
    // Need to track it to be able to hide it for Maximas.
    m_pLabelLOD           = new wxStaticText( pParent, wxID_ANY, wxT( "Details" ) );
    m_pSliderLOD          = new wxSlider( pParent, wxID_ANY,  0,    0, NB_OF_LOD - 1 );
    m_pRadNormal          = new wxRadioButton( pParent,  wxID_ANY, wxT( "Normal" ), DEF_POS, DEF_SIZE, wxRB_GROUP );
    m_pRadMapOnSphere     = new wxRadioButton( pParent,  wxID_ANY, wxT( "Map On Sphere" ) );
    m_pRadMainAxis        = new wxRadioButton( pParent,  wxID_ANY, wxT( "Maximas" ) );
    m_pToggleAxisFlipX    = new wxToggleButton( pParent, wxID_ANY, wxT( "X" ), DEF_POS, wxSize( 15, -1 ) );
    m_pToggleAxisFlipY    = new wxToggleButton( pParent, wxID_ANY, wxT( "Y" ), DEF_POS, wxSize( 15, -1 ) );
    m_pToggleAxisFlipZ    = new wxToggleButton( pParent, wxID_ANY, wxT( "Z" ), DEF_POS, wxSize( 15, -1 ) );
    
#if !_USE_LIGHT_GUI
    m_pToggleColorWithPosition = new wxToggleButton( pParent, wxID_ANY, wxT( "Color with Position" ), DEF_POS, wxSize( 80, -1 ) );
#endif
    
    m_pTxtBoxScalingFactor  = new wxTextCtrl( pParent, wxID_ANY, wxString::Format( wxT( "%.1f" ), 5.0f ), DEF_POS, wxSize( 50, -1 ) );

    m_pRadNormal->SetValue(      isDisplayShape( NORMAL ) );
    m_pRadMapOnSphere->SetValue( isDisplayShape( SPHERE ) );
    m_pRadMainAxis->SetValue(    isDisplayShape( AXIS ) );

    //////////////////////////////////////////////////////////////////////////

    wxFlexGridSizer *pGridSliders = new wxFlexGridSizer( 2 );

#if !_USE_LIGHT_GUI
    pGridSliders->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Min Hue" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderMinHue, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pGridSliders->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Max Hue" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderMaxHue, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pGridSliders->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Saturation" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderSaturation, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pGridSliders->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Luminance" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderLuminance, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pGridSliders->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Light Att." ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderLightAttenuation, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pGridSliders->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Light X" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderLightXPosition, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pGridSliders->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Light Y" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderLightYPosition, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pGridSliders->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Light Z" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderLightZPosition, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );
#endif

    pGridSliders->Add( m_pLabelDisplay, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderDisplay, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pGridSliders->Add( m_pLabelLOD, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderLOD, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );
    
    pGridSliders->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Scaling" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderScalingFactor, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );
    pGridSliders->Add( new wxStaticText( pParent, wxID_ANY, wxT( " " ) ), 0, wxALIGN_LEFT | wxALL );
    pGridSliders->Add( m_pTxtBoxScalingFactor, 0, wxALIGN_CENTER | wxEXPAND | wxALL, 1 );

    wxBoxSizer *pBoxFlips = new wxBoxSizer( wxHORIZONTAL );
    pBoxFlips->Add( m_pToggleAxisFlipX, 1, wxALIGN_CENTER | wxALL, 1 );
    pBoxFlips->Add( m_pToggleAxisFlipY, 1, wxALIGN_CENTER | wxALL, 1 );
    pBoxFlips->Add( m_pToggleAxisFlipZ, 1, wxALIGN_CENTER | wxALL, 1 );

    pGridSliders->Add( new wxStaticText( pParent, wxID_ANY, wxT( "Local Flips" ) ), 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( pBoxFlips, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );
    
#if !_USE_LIGHT_GUI
    // Cheating to align the button.
    pGridSliders->Add( new wxStaticText( pParent, wxID_ANY, wxT( " " ) ), 0, wxALIGN_LEFT | wxALL );
    pGridSliders->Add( m_pToggleColorWithPosition, 1, wxEXPAND | wxALIGN_CENTER | wxALL, 1 );
#endif

    pBoxMain->Add( pGridSliders, 0, wxEXPAND | wxALL, 2 );

    //////////////////////////////////////////////////////////////////////////

    wxBoxSizer *pBoxDisplay = new wxBoxSizer( wxVERTICAL );
    m_pLabelDisplayRadioType = new wxStaticText( pParent, wxID_ANY, wxT( "Display:" ) );
    pBoxDisplay->Add( m_pLabelDisplayRadioType, 0, wxALIGN_LEFT | wxALL, 1 );

    m_pBoxDisplayRadios = new wxBoxSizer( wxVERTICAL );
    m_pBoxDisplayRadios->Add( m_pRadNormal,      0, wxALIGN_LEFT | wxALL, 1 );
    m_pBoxDisplayRadios->Add( m_pRadMapOnSphere, 0, wxALIGN_LEFT | wxALL, 1 );
    m_pBoxDisplayRadios->Add( m_pRadMainAxis,    0, wxALIGN_LEFT | wxALL, 1 );
    pBoxDisplay->Add( m_pBoxDisplayRadios, 0, wxALIGN_LEFT | wxLEFT, 32 );

    pBoxMain->Add( pBoxDisplay, 0, wxFIXED_MINSIZE | wxEXPAND | wxTOP | wxBOTTOM, 8 );

    //////////////////////////////////////////////////////////////////////////

    m_pPropertiesSizer->Add( pBoxMain, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    //////////////////////////////////////////////////////////////////////////
    // Connect widgets with callback function
    pParent->Connect( m_pToggleAxisFlipX->GetId(),         wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnGlyphXAxisFlipChecked ) );
    pParent->Connect( m_pToggleAxisFlipY->GetId(),         wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnGlyphYAxisFlipChecked ) );
    pParent->Connect( m_pToggleAxisFlipZ->GetId(),         wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnGlyphZAxisFlipChecked ) );
    pParent->Connect( m_pRadNormal->GetId(),          wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnGlyphNormalSelected ) );
    pParent->Connect( m_pRadMapOnSphere->GetId(),     wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnGlyphMapOnSphereSelected ) );
    pParent->Connect( m_pRadMainAxis->GetId(),        wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnGlyphMainAxisSelected ) );
    
#if !_USE_LIGHT_GUI
    pParent->Connect( m_pSliderMinHue->GetId(),       wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnGlyphMinHueSliderMoved ) ); 
    pParent->Connect( m_pSliderMaxHue->GetId(),       wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnGlyphMaxHueSliderMoved ) ); 
    pParent->Connect( m_pSliderSaturation->GetId(),   wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnGlyphSaturationSliderMoved ) ); 
    pParent->Connect( m_pSliderLuminance->GetId(),    wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnGlyphLuminanceSliderMoved ) ); 
    pParent->Connect( m_pSliderLightAttenuation->GetId(),  wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnGlyphLightAttenuationSliderMoved ) ); 
    pParent->Connect( m_pSliderLightXPosition->GetId(),    wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnGlyphLightXDirectionSliderMoved ) ); 
    pParent->Connect( m_pSliderLightYPosition->GetId(),    wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnGlyphLightYDirectionSliderMoved ) );
    pParent->Connect( m_pSliderLightZPosition->GetId(),    wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnGlyphLightZDirectionSliderMoved ) );
#endif
    
    pParent->Connect( m_pSliderDisplay->GetId(),      wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnGlyphDisplaySliderMoved ) );
    pParent->Connect( m_pSliderScalingFactor->GetId(),     wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnGlyphScalingFactorSliderMoved ) );
    pParent->Connect( m_pSliderLOD->GetId(),          wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnGlyphLODSliderMoved ) );

#if !_USE_LIGHT_GUI
    pParent->Connect( m_pToggleColorWithPosition->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnGlyphColorWithPosition ) );
#endif
    
    pParent->Connect( m_pTxtBoxScalingFactor->GetId(),  wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( PropertiesWindow::OnBoxScalingFactor ) );
}

void Glyph::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();
    
    // TODO: issue 117
    // Interpolation has no effect for glyphs for the moment.
    m_pToggleFiltering->Hide();

#if !_USE_LIGHT_GUI
    m_pSliderMinHue->SetValue    ( getColor( MIN_HUE )    * 100 );
    m_pSliderMaxHue->SetValue    ( getColor( MAX_HUE )    * 100 );
    m_pSliderSaturation->SetValue( getColor( SATURATION ) * 100 );
    m_pSliderLuminance->SetValue ( getColor( LUMINANCE )  * 100 );
    m_pSliderLOD->SetValue       ( (int)getLOD() );
    m_pSliderLightAttenuation->SetValue( getLightAttenuation()      * 100 );
    m_pSliderLightXPosition->SetValue  ( getLightPosition( X_AXIS ) * 100 );
    m_pSliderLightYPosition->SetValue  ( getLightPosition( Y_AXIS ) * 100 );
    m_pSliderLightZPosition->SetValue  ( getLightPosition( Z_AXIS ) * 100 );
    m_pToggleColorWithPosition->SetValue( getColorWithPosition() );
#endif
    
    m_pSliderDisplay->SetValue( getDisplayFactor() );
    m_pSliderScalingFactor->SetValue( getScalingFactor() * 10.0f );

    m_pToggleAxisFlipX->SetValue    ( isAxisFlipped( X_AXIS ) );
    m_pToggleAxisFlipY->SetValue    ( isAxisFlipped( Y_AXIS ) );
    m_pToggleAxisFlipZ->SetValue    ( isAxisFlipped( Z_AXIS ) );
    
    // TODO issue 118.
    // Hidden for the moment, not implemented.
    m_pBtnFlipX->Hide();
    m_pBtnFlipY->Hide();
    m_pBtnFlipZ->Hide();
}

