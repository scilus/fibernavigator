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

#include <GL/glew.h>
#include "../misc/nifti/nifti1_io.h"
#include "../gui/mainFrame.h"

///////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////
Glyph::Glyph( DatasetHelper* i_datasetHelper,
              float i_minHue     , 
              float i_maxHue     , 
              float i_saturation , 
              float i_luminance ) :
    DatasetInfo             ( i_datasetHelper ),
    m_datasetHelper         ( NULL ),
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
    m_currentLOD            ( LOD_0 )
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
    if( m_datasetHelper.m_useVBO && m_hemisphereBuffer )
    {
        glDeleteBuffers( 1, m_hemisphereBuffer );
        delete m_hemisphereBuffer;
    }
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

    if( DatasetInfo::m_dh-> m_showAxial )
        drawAxial();

    if( DatasetInfo::m_dh-> m_showCoronal )
        drawCoronal();

    if( DatasetInfo::m_dh-> m_showSagittal )
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
    for( int y = 0; y < m_datasetHelper.m_rows; y++ )
        for( int x = 0; x < m_datasetHelper.m_columns; x++ )
            // We only draw the glyphs if the test for our display factor is succesfull.
            if ((x+y)%m_displayFactor==0)
                drawGlyph( m_currentSliderPos[2], y, x, Z_AXIS );
}

///////////////////////////////////////////////////////////////////////////
// This function will display the Coronal slice of the loaded glyphs.
///////////////////////////////////////////////////////////////////////////
void Glyph::drawCoronal()
{  
    for( int z = 0; z < m_datasetHelper.m_frames; z++ )
        for( int x = 0; x < m_datasetHelper.m_columns; x++ )
            // We only draw the glyphs if the test for our display factor is succesfull.
            if ((x+z)%m_displayFactor==0)
                drawGlyph( z, m_currentSliderPos[1], x, Y_AXIS );
}

///////////////////////////////////////////////////////////////////////////
// This function will display the Sagittal slice of the loaded glyphs.
///////////////////////////////////////////////////////////////////////////
void Glyph::drawSagittal()
{
    for( int z = 0; z < m_datasetHelper.m_frames; z++ )
        for( int y = 0; y < m_datasetHelper.m_rows; y++ )
            // We only draw the glyphs if the test for our display factor is succesfull.
            if ((y+z)%m_displayFactor==0)
                drawGlyph( z, y, m_currentSliderPos[0], X_AXIS );
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
        if( m_currentSliderPos[i] !=  l_currentPos[i] )
        {
            m_currentSliderPos[i] = l_currentPos[i];
            sliderPosChanged( (AxisType)i );
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function computes the current sliders postionss (all 3 axes)
///////////////////////////////////////////////////////////////////////////
void Glyph::getSlidersPositions( int o_slidersPos[3] )
{
    // For the X axis.
    float l_xSliderRatio  = (float)DatasetInfo::m_dh->m_xSlize / ( (float)DatasetInfo::m_dh->m_columns - 1 );
    o_slidersPos[0]       = ( l_xSliderRatio * ( (float)m_datasetHelper.m_columns - 1 ) );

    // For the Y axis.
    float l_ySliderRatio  = (float)DatasetInfo::m_dh->m_ySlize / ( (float)DatasetInfo::m_dh->m_rows - 1 );
    o_slidersPos[1]       = ( l_ySliderRatio * ( (float)m_datasetHelper.m_rows - 1 ) );

    // For the Z axis.
    float l_zSliderRatio  = (float)DatasetInfo::m_dh->m_zSlize / ( (float)DatasetInfo::m_dh->m_frames - 1 );    
    o_slidersPos[2]       = ( l_zSliderRatio * ( (float)m_datasetHelper.m_frames - 1 ) );
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
    // Get the current tensors index in the coeffs's buffer
    return( i_zVoxel * m_datasetHelper.m_columns * m_datasetHelper.m_rows + 
            i_yVoxel * m_datasetHelper.m_columns +
            i_xVoxel );    
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
      if( DatasetInfo::m_dh->m_frustum[p][0] * ( i_boxCenter.x - i_boxSize.x ) + 
          DatasetInfo::m_dh->m_frustum[p][1] * ( i_boxCenter.y - i_boxSize.y ) + 
          DatasetInfo::m_dh->m_frustum[p][2] * ( i_boxCenter.z - i_boxSize.z ) + 
          DatasetInfo::m_dh->m_frustum[p][3] > 0 )
         continue;

      if( DatasetInfo::m_dh->m_frustum[p][0] * ( i_boxCenter.x + i_boxSize.x ) + 
          DatasetInfo::m_dh->m_frustum[p][1] * ( i_boxCenter.y - i_boxSize.y ) + 
          DatasetInfo::m_dh->m_frustum[p][2] * ( i_boxCenter.z - i_boxSize.z ) + 
          DatasetInfo::m_dh->m_frustum[p][3] > 0 )
         continue;

      if( DatasetInfo::m_dh->m_frustum[p][0] * ( i_boxCenter.x - i_boxSize.x ) + 
          DatasetInfo::m_dh->m_frustum[p][1] * ( i_boxCenter.y + i_boxSize.y ) + 
          DatasetInfo::m_dh->m_frustum[p][2] * ( i_boxCenter.z - i_boxSize.z ) + 
          DatasetInfo::m_dh->m_frustum[p][3] > 0 )
         continue;

      if( DatasetInfo::m_dh->m_frustum[p][0] * ( i_boxCenter.x + i_boxSize.x ) + 
          DatasetInfo::m_dh->m_frustum[p][1] * ( i_boxCenter.y + i_boxSize.y ) + 
          DatasetInfo::m_dh->m_frustum[p][2] * ( i_boxCenter.z - i_boxSize.z ) + 
          DatasetInfo::m_dh->m_frustum[p][3] > 0 )
         continue;

      if( DatasetInfo::m_dh->m_frustum[p][0] * ( i_boxCenter.x - i_boxSize.x ) + 
          DatasetInfo::m_dh->m_frustum[p][1] * ( i_boxCenter.y - i_boxSize.y ) + 
          DatasetInfo::m_dh->m_frustum[p][2] * ( i_boxCenter.z + i_boxSize.z ) + 
          DatasetInfo::m_dh->m_frustum[p][3] > 0 )
         continue;

      if( DatasetInfo::m_dh->m_frustum[p][0] * ( i_boxCenter.x + i_boxSize.x ) + 
          DatasetInfo::m_dh->m_frustum[p][1] * ( i_boxCenter.y - i_boxSize.y ) + 
          DatasetInfo::m_dh->m_frustum[p][2] * ( i_boxCenter.z + i_boxSize.z ) + 
          DatasetInfo::m_dh->m_frustum[p][3] > 0 )
         continue;

      if( DatasetInfo::m_dh->m_frustum[p][0] * ( i_boxCenter.x - i_boxSize.x ) + 
          DatasetInfo::m_dh->m_frustum[p][1] * ( i_boxCenter.y + i_boxSize.y ) + 
          DatasetInfo::m_dh->m_frustum[p][2] * ( i_boxCenter.z + i_boxSize.z ) + 
          DatasetInfo::m_dh->m_frustum[p][3] > 0 )
         continue;

      if( DatasetInfo::m_dh->m_frustum[p][0] * ( i_boxCenter.x + i_boxSize.x ) + 
          DatasetInfo::m_dh->m_frustum[p][1] * ( i_boxCenter.y + i_boxSize.y ) + 
          DatasetInfo::m_dh->m_frustum[p][2] * ( i_boxCenter.z + i_boxSize.z ) + 
          DatasetInfo::m_dh->m_frustum[p][3] > 0 )
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
    o_offset[2] = ( i_zVoxelIndex + 0.5f ) * m_datasetHelper.m_zVoxel;
    o_offset[1] = ( i_yVoxelIndex + 0.5f ) * m_datasetHelper.m_yVoxel;
    o_offset[0] = ( i_xVoxelIndex + 0.5f ) * m_datasetHelper.m_xVoxel;
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
    // Make sure that there is enought distance between the i_minHue and the i_maxHue.
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
// i_LOD                : A LOD in wich we will generate our perfect sphere.
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
    if( !m_datasetHelper.m_useVBO )
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
    if( DatasetInfo::m_dh->GLError() )
    {
        DatasetInfo::m_dh->printGLError( wxT( "initialize vbo points for tensors" ) );
        m_datasetHelper.m_useVBO = false;
        delete m_hemisphereBuffer;
    }
}


void Glyph::setDisplayShape ( DisplayShape i_displayShape ) 
{
    m_displayShape = i_displayShape;    
}

void Glyph::createPropertiesSizer(MainFrame *parent)
{
    DatasetInfo::createPropertiesSizer(parent);
    wxSizer *l_sizer;
    
    m_psliderMinHueValue  = new wxSlider( parent, wxID_ANY,   0, 0, 100, wxDefaultPosition, wxSize( 140, -1 ));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText( parent, wxID_ANY, _T( "Min Hue" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(m_psliderMinHueValue,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_psliderMinHueValue->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(MainFrame::OnGlyphMinHueSliderMoved)); 

    m_psliderMaxHueValue  = new wxSlider( parent, wxID_ANY,   0, 0, 100, wxDefaultPosition, wxSize( 140, -1 ));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText( parent, wxID_ANY, _T( "Max Hue" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(m_psliderMaxHueValue,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);    
    parent->Connect(m_psliderMaxHueValue->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(MainFrame::OnGlyphMaxHueSliderMoved)); 

    m_psliderSaturationValue  = new wxSlider( parent, wxID_ANY,   0, 0, 100, wxDefaultPosition, wxSize( 140, -1 ));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText( parent, wxID_ANY, _T( "Saturation" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(m_psliderSaturationValue,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);  
    parent->Connect(m_psliderSaturationValue->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(MainFrame::OnGlyphSaturationSliderMoved)); 

    m_psliderLuminanceValue  = new wxSlider( parent, wxID_ANY,   0, 0, 100, wxDefaultPosition, wxSize( 140, -1 ));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText( parent, wxID_ANY, _T( "Luminace" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(m_psliderLuminanceValue,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER); 
    parent->Connect(m_psliderLuminanceValue->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(MainFrame::OnGlyphLuminanceSliderMoved)); 

    m_psliderLightAttenuation  = new wxSlider( parent, wxID_ANY,   0, 0, 100, wxDefaultPosition, wxSize( 140, -1 ));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText( parent, wxID_ANY, _T( "Light Attenuation" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(m_psliderLightAttenuation,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER); 
    parent->Connect(m_psliderLightAttenuation->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(MainFrame::OnGlyphLightAttenuationSliderMoved)); 

    m_psliderLightXPosition  = new wxSlider( parent, wxID_ANY,   0, -100, 100, wxDefaultPosition, wxSize( 140, -1 ));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText( parent, wxID_ANY, _T( "Light x Position" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(m_psliderLightXPosition,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER); 
    parent->Connect(m_psliderLightXPosition->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(MainFrame::OnGlyphLightXDirectionSliderMoved)); 

    m_psliderLightYPosition  = new wxSlider( parent, wxID_ANY,   0, -100, 100, wxDefaultPosition, wxSize( 140, -1 ));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText( parent, wxID_ANY, _T( "Light y Position" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(m_psliderLightYPosition,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER); 
    parent->Connect(m_psliderLightYPosition->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(MainFrame::OnGlyphLightYDirectionSliderMoved));

    m_psliderLightZPosition  = new wxSlider( parent, wxID_ANY,   0, -100, 100, wxDefaultPosition, wxSize( 140, -1 ));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText( parent, wxID_ANY, _T( "Light z Position" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(m_psliderLightZPosition,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER); 
    parent->Connect(m_psliderLightZPosition->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(MainFrame::OnGlyphLightZDirectionSliderMoved));

    m_psliderDisplayValue  = new wxSlider( parent, wxID_ANY,   0, 1, 20, wxDefaultPosition, wxSize( 140, -1 ));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText( parent, wxID_ANY, _T( "Display" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(m_psliderDisplayValue,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER); 
    parent->Connect(m_psliderDisplayValue->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(MainFrame::OnGlyphDisplaySliderMoved));

    m_psliderScalingFactor  = new wxSlider( parent, wxID_ANY,   0, 1, 20, wxDefaultPosition, wxSize( 140, -1 ));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText( parent, wxID_ANY, _T( "Scaling Factor" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(m_psliderScalingFactor,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER); 
    parent->Connect(m_psliderScalingFactor->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(MainFrame::OnGlyphScalingFactorSliderMoved));

    m_psliderLODValue  = new wxSlider( parent, wxID_ANY, 0, 0, NB_OF_LOD - 1, wxDefaultPosition, wxSize( 140, -1 ));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText( parent, wxID_ANY, _T( "Details" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(m_psliderLODValue,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER); 
    parent->Connect(m_psliderLODValue->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(MainFrame::OnGlyphLODSliderMoved));

    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText( parent, wxID_ANY, _T( "Flips" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->AddSpacer(8);
    m_ptoggleAxisFlipX = new wxToggleButton(parent, wxID_ANY, wxT("X"),wxDefaultPosition, wxSize(42,-1));    
    l_sizer->Add(m_ptoggleAxisFlipX,0,wxALIGN_CENTER);
    m_ptoggleAxisFlipY = new wxToggleButton(parent, wxID_ANY, wxT("Y"),wxDefaultPosition, wxSize(42,-1));
    l_sizer->Add(m_ptoggleAxisFlipY,0,wxALIGN_CENTER);
    m_ptoggleAxisFlipZ = new wxToggleButton(parent, wxID_ANY, wxT("Z"),wxDefaultPosition, wxSize(42,-1));
    l_sizer->Add(m_ptoggleAxisFlipZ,0,wxALIGN_CENTER);
    l_sizer->AddSpacer(8);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER); 
    parent->Connect(m_ptoggleAxisFlipX->GetId(),wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnGlyphXAxisFlipChecked));
    parent->Connect(m_ptoggleAxisFlipY->GetId(),wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnGlyphYAxisFlipChecked));
    parent->Connect(m_ptoggleAxisFlipZ->GetId(),wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnGlyphZAxisFlipChecked));
    m_ptoggleColorWithPosition = new wxToggleButton(parent, wxID_ANY, wxT("Color with Position"),wxDefaultPosition, wxSize(140,-1));
    m_propertiesSizer->AddSpacer(8);
    m_propertiesSizer->Add(m_ptoggleColorWithPosition,0,wxALIGN_CENTER);
    parent->Connect(m_ptoggleColorWithPosition->GetId(),wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnGlyphColorWithPosition));
    
    m_propertiesSizer->AddSpacer(8);
    m_psizerDisplay = new wxBoxSizer(wxVERTICAL);
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText( parent, wxID_ANY, _T( "Display" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(8,1,0);
    m_pradiobtnNormal = new wxRadioButton(parent, wxID_ANY, _T( "Normal" ), wxDefaultPosition, wxSize(132,-1));
    l_sizer->Add(m_pradiobtnNormal);
    m_psizerDisplay->Add(l_sizer,0,wxALIGN_CENTER);
    m_pradiobtnMapOnSphere  = new wxRadioButton(parent, wxID_ANY, _T( "Map On Sphere" ), wxDefaultPosition, wxSize(132,-1));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(68,1,0);
    l_sizer->Add(m_pradiobtnMapOnSphere);
    m_psizerDisplay->Add(l_sizer,0,wxALIGN_CENTER);
    m_pradiobtnMainAxis = new wxRadioButton(parent, wxID_ANY, _T( "Main Diffusion Axis" ), wxDefaultPosition, wxSize(132,-1));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(68,1,0);
    l_sizer->Add(m_pradiobtnMainAxis);
    m_psizerDisplay->Add(l_sizer,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(m_psizerDisplay,0,wxCENTER);
    parent->Connect(m_pradiobtnNormal->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(MainFrame::OnGlyphNormalSelected));
    parent->Connect(m_pradiobtnMapOnSphere->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(MainFrame::OnGlyphMapOnSphereSelected));
    parent->Connect(m_pradiobtnMainAxis->GetId(),wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(MainFrame::OnGlyphMainAxisSelected));
    m_pradiobtnNormal->SetValue        (isDisplayShape(NORMAL));
    m_pradiobtnMapOnSphere->SetValue   (isDisplayShape(SPHERE));
    m_pradiobtnMainAxis->SetValue      (isDisplayShape(AXIS));
}

void Glyph::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();

    m_psliderMinHueValue->SetValue     (getColor( MIN_HUE ) * 100);
    m_psliderMaxHueValue->SetValue     (getColor( MAX_HUE ) * 100);
    m_psliderSaturationValue->SetValue (getColor( SATURATION ) * 100);
    m_psliderLuminanceValue->SetValue  (getColor( LUMINANCE ) * 100);
    m_psliderLODValue->SetValue        ((int)getLOD());
    m_psliderLightAttenuation->SetValue(getLightAttenuation() * 100);
    m_psliderLightXPosition->SetValue  (getLightPosition( X_AXIS ) * 100);
    m_psliderLightYPosition->SetValue  (getLightPosition( Y_AXIS ) * 100);
    m_psliderLightZPosition->SetValue  (getLightPosition( Z_AXIS ) * 100 );
    m_psliderDisplayValue->SetValue    (getDisplayFactor());
    m_psliderScalingFactor->SetValue   (getScalingFactor());
    m_ptoggleAxisFlipX->SetValue       (isAxisFlipped( X_AXIS ));
    m_ptoggleAxisFlipY->SetValue       (isAxisFlipped( Y_AXIS ));
    m_ptoggleAxisFlipZ->SetValue       (isAxisFlipped( Z_AXIS ));
    m_ptoggleColorWithPosition->SetValue(getColorWithPosition());

}

