/////////////////////////////////////////////////////////////////////////////
// Name:            fibers.h
// Author:          ---
// Creation Date:   ---
//
// Description: fibers class.
//
// Last modifications:
//      by : ggirard - 29/12/2010
/////////////////////////////////////////////////////////////////////////////

#ifndef FIBERS_H_
#define FIBERS_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <vector>

#include "datasetInfo.h"
#include <GL/glew.h>
#include "KdTree.h"
#include "splinePoint.h"
#include "../gui/SelectionObject.h"
#include "../misc/Fantom/FVector.h"

enum FiberFileType 
{
    ASCII_FIBER = 0,
    ASCII_VTK   = 1,
    BINARY_VTK  = 2,
};

using namespace std;

class Fibers : public DatasetInfo , public wxTreeItemData
{
public:
    Fibers( DatasetHelper* );
    virtual ~Fibers();

    void    activateLIC         ()                          {};
    void    clean               ()                          {};
    void    draw                ();
    void    drawVectors         ()                          {};
    void    generateTexture     ()                          {};
    void    generateGeometry    ()                          {};
    bool    getBarycenter       ( SplinePoint*        );
    GLuint  getGLuint           (                     )     { return 0; };
    float   getMaxFibersLength() {return m_maxLength;}
    float   getMinFibersLength() {return m_minLength;}
    int     getLineCount        ();
    int     getLineForPoint     ( int                 );
    int     getPointCount       ();
    int     getPointsPerLine    ( int                 );
    float   getPointValue       ( int      i_index    );
    int     getStartIndexForLine( int                 );
    void    initializeBuffer    ();
    bool    isSelected          ( int      i_fiber    );
    bool    load                ( wxString i_filename );
    bool    loadCamino          ( wxString i_filename );
    bool    loadPTK             ( wxString i_filename );
    void    loadTestFibers      ();
    bool    loadVTK             ( wxString i_filename );
    bool    loadDmri            ( wxString i_fileName ); 
    void    resetColorArray     ();
    void    save                ( wxString i_filename );
    void    saveDMRI            ( wxString i_filename );
    void    smooth              ()                          {};
    void    switchNormals       ( bool     i_positive );
    void    updateFibersColors  ();
    void    updateLinesShown    ();  
    void    updateFibersFilters ();

    void    generateFiberVolume();
    void    setFibersLength();
    virtual void createPropertiesSizer(MainFrame *parent);
    virtual void updatePropertiesSizer();

private:
    wxButton       *m_pGeneratesFibersDensityVolume;
    wxSlider       *m_psliderFibersFilterMin;
    wxSlider       *m_psliderFibersFilterMax;
    wxSlider       *m_psliderFibersSampling;
    wxToggleButton *m_ptoggleLocalColoring;
    wxToggleButton *m_ptoggleNormalColoring;
    wxRadioButton  *m_pradioNormalColoring;
    wxRadioButton  *m_pradioDistanceAnchoring;
    wxRadioButton  *m_pradioMinDistanceAnchoring;
    wxRadioButton  *m_pradioCurvature;
    wxRadioButton  *m_pradioTorsion;
    bool            m_isSpecialFiberDisplay;
    
    void            barycenterTest          ( int, int, int               );
    void            boxTest                 ( int, int, int               );
    void            calculateLinePointers   ();
    void            colorWithCurvature      ( float* i_colorData          );
    void            colorWithTorsion        ( float* i_colorData          );
    void            colorWithDistance       ( float* i_colorData          );
    void            colorWithMinDistance    ( float* i_colorData          );
    void            createColorArray        ( bool i_colorsLoadedFromFile );
    void            drawFakeTubes           ();
    void            drawSortedLines         ();
    void            ellipsoidTest           ( int, int, int               );
    void            freeArrays              ();
    vector< bool >  getLinesShown           ( SelectionObject*            );
    string          intToString             ( int i_number                );
    void            resetLinesShown         ();    
    void            toggleEndianess         ();
    bool            getFiberCoordValues     ( int fiberIndex, vector< Vector > &o_fiberPoints);

    // Variables
    Vector          m_barycenter;
    vector< float > m_boxMax;
    vector< float > m_boxMin;
    vector< float > m_colorArray;
    int             m_count;
    int             m_countLines;
    int				m_countPoints;
    bool            m_isInitialized;
    KdTree*         m_kdTree;
    vector< int >   m_lineArray;
    vector< int >   m_linePointers;
    vector< float > m_pointArray;
    vector< float > m_normalArray;
    bool            m_normalsPositive;
    vector< int >   m_reverse;
    vector< bool >  m_selected;
    vector< bool >  m_filtered;
    vector< float > m_length;
    float           m_maxLength;
    float           m_minLength;
    vector<float  > m_localizedAlpha;

    float           m_cachedThreshold;
};

#endif /* FIBERS_H_ */
