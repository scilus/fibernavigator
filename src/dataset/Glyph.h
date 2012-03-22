/////////////////////////////////////////////////////////////////////////////
// Name:            Glyph.h
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   11/13/2009
//
// Description: Abstract Glyph class.
//
// Last modifications:
//      by : girardg - 28/12/2010
/////////////////////////////////////////////////////////////////////////////
#ifndef GLYPH_H_
#define GLYPH_H_

#include "DatasetInfo.h"
#include "../misc/Algorithms/Helper.h"

#define TEXTURE_NB_OF_COLOR  64     // Must be a power of 2.
#define HUE_MINIMUM_DISTANCE 0.01f  // The minimum distance between the min and max hue for the glyph color.

enum DisplayShape { NORMAL, SPHERE, AXES, AXIS };

class MainFrame;

class Glyph : public DatasetInfo
{
public:
    // Constructor/Destructor
    Glyph( float i_minHue       = 0.65 , 
           float i_maxHue       = 0.0f, 
           float i_saturation   = 0.75f, 
           float i_luminance    = 0.5f  );
    virtual ~Glyph();

    // From DatasetInfo
    virtual void draw();

    // Functions
    void         flipAxis     ( AxisType i_axisType, bool i_isFlipped );
    bool         isAxisFlipped( AxisType i_axisType );
    
    void         setColor ( GlyphColorModifier i_colorModifier, float i_value );
    float        getColor ( GlyphColorModifier i_colorModifier );
    
    void         setLOD ( LODChoices i_LOD );
    LODChoices   getLOD () { return m_currentLOD; };
    
    void         setDisplayFactor ( int i_displayFactor ) { m_displayFactor     = i_displayFactor; };
    int          getDisplayFactor ()                      { return m_displayFactor;                };
    
    void         setDisplayShape ( DisplayShape i_displayShape );
    DisplayShape getDisplayShape ()                     { return m_displayShape;                };
    bool         isDisplayShape( DisplayShape i_displayShape ) {return m_displayShape == i_displayShape;};
    
    void         setColorWithPosition ( bool i_colorPos ) { m_colorWithPosition = i_colorPos; };
    bool         getColorWithPosition ()                  { return m_colorWithPosition;       };
    
    void         setLighAttenuation  ( float i_attenuation ) { m_lighAttenuation   = i_attenuation; };
    float        getLightAttenuation ()                      { return m_lighAttenuation;            };
    
    void         setLightPosition ( AxisType i_axis, float i_position );
    float        getLightPosition ( AxisType i_axis );
    
    virtual void setScalingFactor ( float i_scalingFactor ) { m_scalingFactor  = i_scalingFactor; };
    float        getScalingFactor ()                        { return m_scalingFactor; };

    void         refreshSlidersValues();
    virtual void createPropertiesSizer(PropertiesWindow *pParent);
    virtual void updatePropertiesSizer();
    int          getGlyphIndex       ( int i_zVoxel, int i_yVoxel, int i_xVoxel );

    virtual void       flipAxis( AxisType i_axe ){};

public:
    // Items related to the glyph options sizer.
    wxSlider           *m_pSliderMinHue;
    wxSlider           *m_pSliderMaxHue;
    wxSlider           *m_pSliderSaturation;
    wxSlider           *m_pSliderLuminance;
    wxSlider           *m_pSliderLOD;
    wxSlider           *m_pSliderLightAttenuation;
    wxSlider           *m_pSliderLightXPosition;
    wxSlider           *m_pSliderLightYPosition;
    wxSlider           *m_pSliderLightZPosition;
    wxSlider           *m_pSliderDisplay;
    wxSlider           *m_pSliderScalingFactor;
    wxToggleButton     *m_pToggleAxisFlipX;
    wxToggleButton     *m_pToggleAxisFlipY;
    wxToggleButton     *m_pToggleAxisFlipZ;
    wxToggleButton     *m_pToggleColorWithPosition; 
    wxRadioButton      *m_pRadNormal;
    wxRadioButton      *m_pRadMapOnSphere;
    wxRadioButton      *m_pRadMainAxis;

protected:
    // From DatasetInfo
    virtual void    activateLIC()      {};
    virtual void    clean()            {};
    virtual void    smooth()           {};
    virtual void    generateGeometry() {};
    virtual GLuint  getGLuint()        { return 0; };
    virtual void    initializeBuffer() {};
    virtual void    generateTexture()  {};

    // Pure virtual functions
    virtual bool    createStructure   ( std::vector< float >& i_fileFloatData ) = 0;
    virtual void    drawGlyph         ( int      i_zVoxel, 
                                        int      i_yVoxel, 
                                        int      i_xVoxel, 
                                        AxisType i_axis )                  = 0;
    
    // Functions
    bool            boxInFrustum        ( Vector i_boxCenter, Vector i_boxSize );
    void            drawAxial           ();
    void            drawCoronal         ();
    void            drawSagittal         ();
    void            fillColorDataset    ( float i_minHueAngle, float i_maxHueAngle, float i_saturationValue, float i_luminanceValue );
    void            generateColorTexture( float i_minHue, float i_maxHue, float i_saturation, float i_luminance );
    void            generateSpherePoints( float i_scalingFactor );
    
    int             getLODNbOfPoints    ( const LODChoices i_LODChoices );
    void            getSpherePoints     ( LODChoices            i_LOD,
                                          float                 i_scalingFactor,
                                          std::vector< float > &o_spherePoints  );    
    void            getSlidersPositions ( int o_slidersPos[3] );
    void            getVoxelOffset      ( int i_zVoxelIndex, int i_yVoxelIndex, int i_xVoxelIndex, float o_offset[3] );
    virtual void    loadBuffer          ();
    virtual void    sliderPosChanged    ( AxisType i_axis ) {};
    void            swap                ( Glyph &g );

protected:
    // Variables
    GLuint*         m_hemisphereBuffer;     // For the buffer containing the points of a hemisphere (stored in video memory) 
    GLuint          m_textureId;
    int             m_nbPointsPerGlyph;      // Nb of points per glyph    
    int             m_nbGlyphs;              // Total nb of glyphs    
    int             m_displayFactor;         // Keep track what is the display factor set to.
    bool            m_axisFlippedToggled;    // True if the flipping option is toggled for one or more of the axes (for the back/front face culling).    
    DisplayShape    m_displayShape;          // Glyph will be mapped on its normal shape, a perfect sphere, three axes or the main axis.
    bool            m_colorWithPosition;     // Indicate if we want this glyph to be colored with the position of the vertex.
    float           m_colorMinHue;           // Current min hue color for this glyph.
    float           m_colorMaxHue;           // Current max hue color for this glyph.
    float           m_colorSaturation;       // Current saturation color for this glyph.
    float           m_colorLuminance;        // Current luminance color for this glyph.
    float           m_lighAttenuation;       // Light attenuation.    
    float           m_scalingFactor;         // Scaling factor for the glpyh.
    LODChoices      m_currentLOD;            // Current LOD




    int   m_currentSliderPos[3];   // Current sliders positions [x, y, z].
    bool  m_flippedAxes[3];        // Are axes flipped or not, true if flipped, false otherwise [x, y, z].
    float m_lightPosition[3];      // Light's position [x, y, z]

    std::vector< float >            m_floatColorDataset;
    std::vector< float >            m_axesPoints;           //the 6 points describing the 3 axes
    std::vector< std::vector < float > > m_LODspheres;      // Stores the hemispheres for all LODs.
    
};

#endif /* GLYPH_H_ */
