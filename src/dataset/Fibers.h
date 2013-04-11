#ifndef FIBERS_H_
#define FIBERS_H_

#include "DatasetInfo.h"
#include "KdTree.h"
#include "Octree.h"
#include "../gui/SelectionObject.h"
#include "../misc/Fantom/FVector.h"

#include <GL/glew.h>
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <fstream>
#include <string>
#include <vector>

enum FiberFileType
{
    ASCII_FIBER = 0,
    ASCII_VTK   = 1,
    BINARY_VTK  = 2,
};

/**
 * This class represents a set of fibers.
 * It supports loading different fibers file types.
 * It holds the fibers data, and can also be used to
 * compute statistics and selected fibers data.
 */
class Fibers : public DatasetInfo
{
public:
    Fibers();
    virtual ~Fibers();

    // Fibers loading methods
    bool    load( const wxString &filename );

    void    updateFibersColors();

    Anatomy* generateFiberVolume();

    void    getFibersInfoToSave( std::vector<float> &pointsToSave, std::vector<int> &linesToSave, std::vector<int> &colorsToSave, int &countLines );
    void    getNbLines( int &nbLines );
    void    loadDMRIFibersInFile( std::ofstream &myfile );

    void    save( wxString filename );
    bool    save( wxXmlNode *pNode ) const;
    void    saveDMRI( wxString filename );

    int     getPointsPerLine(     const int lineId );
    int     getStartIndexForLine( const int lineId );

    int     getLineForPoint( const int pointIdx );

    void    resetColorArray();
    
    void     setFiberColor( const int fiberIdx, const wxColour& col );
    wxColour getFiberPointColor( const int fiberIdx, const int ptIdx );

    void    updateLinesShown();

    void    generateKdTree();

    void    initializeBuffer();

    void    draw();
    void    switchNormals( bool positive );

    float   getPointValue( int  ptIndex );
    int     getLineCount();
    int     getPointCount();
    bool    isSelected( int  fiberId );

    float    getLocalizedAlpha( int index );

    void    setFibersLength();
    
    float   getFiberLength( const int fiberId ) const
    {
        if( fiberId < 0 || static_cast< unsigned int >( fiberId ) >= m_length.size() )
        {
            return 0.0f;
        }
        
        return m_length[ fiberId ];
    }
    
    bool    getFiberCoordValues( int fiberIndex, std::vector< Vector > &fiberPoints );

    void    updateFibersFilters();
    void    updateFibersFilters(int minLength, int maxLength, int minSubsampling, int maxSubsampling);
    std::vector< bool >  getFilteredFibers();

    void    flipAxis( AxisType i_axe );
    
    int     getFibersCount() const { return m_countLines; }
    
    // TODO check if we can set const
    Octree* getOctree() const { return m_pOctree; }
    
    vector< int > getReverseIdx() const { return m_reverse; }

    virtual void createPropertiesSizer( PropertiesWindow *pParent );
    virtual void updatePropertiesSizer();

    bool isUsingFakeTubes()    const       { return m_useFakeTubes; }
    bool isUsingTransparency() const       { return m_useTransparency; }
    bool isFibersInverted()    const       { return m_fibersInverted; }

    void updateColorationMode()              { m_isColorationUpdated = true; }
    FibersColorationMode getColorationMode() { return m_fiberColorationMode; }
    void setColorationMode(FibersColorationMode val) { m_fiberColorationMode = val; }
    
    void setConstantColor( const wxColor &col ) { m_constantColor = col; }

    void useFakeTubes();
    void useTransparency();
    bool invertFibers()        { return m_fibersInverted = !m_fibersInverted; }

    void findCrossingFibers();

    GLuint  getGLuint() { return 0; }

    float   getMaxFibersLength() { return m_maxLength; }
    float   getMinFibersLength() { return m_minLength; }
    void    updateSliderMinLength( int val )        { m_pSliderFibersFilterMin->SetValue( val ); }
    void    updateSliderMaxLength( int val )        { m_pSliderFibersFilterMax->SetValue( val ); }
    void    updateSliderSubsampling( int val )      { m_pSliderFibersSampling->SetValue( val ); }
    void    updateToggleLocalColoring( bool val )   { m_pToggleLocalColoring->SetValue( val ); }
    void    updateToggleNormalColoring( bool val )  { m_pToggleNormalColoring->SetValue( val ); }
    void    updateSliderThickness( int val )        { m_pSliderInterFibersThickness->SetValue( val ); }

    // Empty derived methods
    void    drawVectors()      {};
    void    generateTexture()  {};
    void    generateGeometry() {};
    void    smooth()           {};

    void    toggleCrossingFibers() { m_useIntersectedFibers = !m_useIntersectedFibers; }
    void    updateCrossingFibersThickness();

	void    convertFromRTT( std::vector<std::vector<Vector> >* RTT );

    // Inherited from DatasetInfo
    bool    toggleShow();

private:
    Fibers( const Fibers & );
    Fibers &operator=( const Fibers & );

private:
    bool            loadTRK(    const wxString &filename );
    bool            loadCamino( const wxString &filename );
    bool            loadMRtrix( const wxString &filename );
    bool            loadPTK(    const wxString &filename );
    bool            loadVTK(    const wxString &filename );
    bool            loadDmri(   const wxString &filename );
    void            loadTestFibers();

    void            colorWithTorsion(       float *pColorData );
    void            colorWithCurvature(     float *pColorData );
    void            colorWithDistance(      float *pColorData );
    void            colorWithMinDistance(   float *pColorData );
    void            colorWithConstantColor( float *pColorData );

    void            toggleEndianess();
    std::string     intToString( const int number );

    void            calculateLinePointers();
    void            createColorArray( const bool colorsLoadedFromFile );

    void            resetLinesShown();

    void            drawFakeTubes();
    void            drawSortedLines();
    void            drawCrossingFibers();

    void            freeArrays();

    void            setShader();
    void            releaseShader();

private:
    // Variables
    bool                  m_isSpecialFiberDisplay;
    Vector                m_barycenter;
    std::vector< float >  m_boxMax;
    std::vector< float >  m_boxMin;
    std::vector< float >  m_colorArray;
    int                   m_count;
    int                   m_countLines;
    int                   m_countPoints;
    bool                  m_isInitialized;
    std::vector< int >    m_lineArray;
    std::vector< int >    m_linePointers;
    std::vector< float >  m_pointArray;
    std::vector< float >  m_normalArray;
    bool                  m_normalsPositive;
    std::vector< int >    m_reverse;
    std::vector< bool >   m_selected;
    std::vector< bool >   m_filtered;
    std::vector< float >  m_length;
    float                 m_maxLength;
    float                 m_minLength;
    std::vector< float  > m_localizedAlpha;
    float                 m_cachedThreshold;
    bool                  m_fibersInverted;
    bool                  m_useFakeTubes;
    bool                  m_useTransparency;

    bool                  m_isColorationUpdated;
    FibersColorationMode  m_fiberColorationMode;

    KdTree                *m_pKdTree;
    Octree                *m_pOctree;

    bool            m_cfDrawDirty;
    bool            m_axialShown;
    bool            m_coronalShown;
    bool            m_sagittalShown;
    bool            m_useIntersectedFibers;
    float           m_thickness;
    float           m_xDrawn;
    float           m_yDrawn;
    float           m_zDrawn;
    std::vector< unsigned int > m_cfStartOfLine;
    std::vector< unsigned int > m_cfPointsPerLine;
    
    wxColor         m_constantColor;

    // GUI members
    wxSlider       *m_pSliderFibersFilterMin;
    wxSlider       *m_pSliderFibersFilterMax;
    wxSlider       *m_pSliderFibersSampling;
    wxSlider       *m_pSliderInterFibersThickness;
    wxToggleButton *m_pToggleLocalColoring;
    wxToggleButton *m_pToggleNormalColoring;
    wxButton       *m_pSelectConstantFibersColor;
    wxToggleButton *m_pToggleCrossingFibers;
    wxRadioButton  *m_pRadNormalColoring;
    wxRadioButton  *m_pRadDistanceAnchoring;
    wxRadioButton  *m_pRadMinDistanceAnchoring;
    wxRadioButton  *m_pRadCurvature;
    wxRadioButton  *m_pRadTorsion;
    wxRadioButton  *m_pRadConstant;
};

#endif /* FIBERS_H_ */
