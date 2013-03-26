/////////////////////////////////////////////////////////////////////////////
// Name:            SelectionObject.h
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   10/26/2009
//
// Description: SelectionObject class.
//
// Last modifications:
//      by : GGirard - 19/02/2011
/////////////////////////////////////////////////////////////////////////////

#ifndef SELECTIONOBJECT_H_
#define SELECTIONOBJECT_H_

#include "BoundingBox.h"
#include "SceneObject.h"

#include "../misc/Algorithms/Face3D.h"
#include "../misc/Algorithms/Helper.h"
#include "../misc/IsoSurface/Vector.h"

#include <GL/glew.h>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/button.h>
#include <wx/grid.h>
#include <wx/string.h>
#include <wx/tglbtn.h>

#include <list>
#include <map>
#include <vector>
using std::vector;

class Anatomy;
class Fibers;
class CIsoSurface;
class MainCanvas;
class PropertiesWindow;

/****************************************************************************/
// Description : This is the base class for any Selection Object.
/****************************************************************************/

///////////////////////////////////////////////////////////////////////////
// Structure containing the information that are displayed in the fibers info grid window.
///////////////////////////////////////////////////////////////////////////
struct FibersInfoGridParams 
{
    FibersInfoGridParams() : m_count            ( 0    ),
                             m_meanValue        ( 0.0f ),
                             m_meanLength       ( 0.0f ),
                             m_minLength        ( 0.0f ),
                             m_maxLength        ( 0.0f ),
                             m_meanCrossSection ( 0.0f ),
                             m_minCrossSection  ( 0.0f ),
                             m_maxCrossSection  ( 0.0f ),
                             m_meanCurvature    ( 0.0f ),
                             m_meanTorsion      ( 0.0f ),
                             m_dispersion       ( 0.0f )
    {
    }
    
    int   m_count;
    float m_meanValue;
    float m_meanLength;
    float m_minLength;
    float m_maxLength;
    float m_meanCrossSection;
    float m_minCrossSection;
    float m_maxCrossSection;
    float m_meanCurvature;
    float m_meanTorsion;
    float m_dispersion;
};

class SelectionObject : public SceneObject, public wxTreeItemData
{
public :
    SelectionObject( Vector i_center, Vector i_size );
    virtual ~SelectionObject();

    virtual hitResult hitTest( Ray* i_ray ) = 0;

    virtual void objectUpdate();

    void draw();
    void drawIsoSurface();
    void lockToCrosshair();
    void moveBack();
    void moveDown();
    void moveForward();
    void moveLeft();
    void moveRight();
    void moveUp();
    void processDrag( wxPoint i_click, wxPoint i_lastPos, GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] );
    void resizeBack();
    void resizeDown();
    void resizeForward();
    void resizeLeft();
    void resizeRight();    
    void resizeUp();
    void select( bool i_flag );
    void update();
    virtual void createPropertiesSizer( PropertiesWindow *pParent );
    virtual void updatePropertiesSizer();
    
    
    // Set/get and there affiliated functions
    bool       toggleIsActive();
    void       setIsActive( bool isActive );
    bool       getIsActive()                          { return m_isActive;                     };

    ObjectType getSelectionType()                     { return m_objectType;                   };    
    bool       isSelectionObject();

    void       setCenter( float i_x, float i_y, float i_z );
    void       setCenter( Vector i_center );
    Vector     getCenter()                            { return m_center;                       };

    void       setColor( wxColour i_color );
    wxColour   getColor()                             { return m_color;                        };

    int        getIcon();

    void       setName( wxString i_name )             { m_name = i_name;                       };
    wxString   getName()                              { return m_name;                         };

    void       setIsMaster( bool i_isMaster );
    bool       getIsMaster()                          { return m_isMaster;                     };
    
    bool       toggleIsNOT();
    void       setIsNOT( bool i_isNOT );
    bool       getIsNOT()                             { return m_isNOT;                        };

    void       setPicked( int i_picked )              { m_hitResult.picked = i_picked;         };

    void       setSize( float sizeX, float sizeY, float sizeZ ) 
                                                      { setSize( Vector( sizeX, sizeY, sizeZ ) ); }
    void       setSize( Vector i_size )               { m_size = i_size; update(); notifyInBoxNeedsUpdating(); };
    Vector     getSize()                              { return m_size;};

    void       setThreshold( float i_threshold );
    float      getThreshold()                         { return m_threshold;                    };

    void       setTreeId( wxTreeItemId i_treeId )     { m_treeId = i_treeId;                   };
    wxTreeItemId getTreeId()                          { return m_treeId;                       };
    
    void       unselect()                             { m_isSelected = false;                  };
    
    bool       toggleIsVisible()                      { setIsVisible( !getIsVisible() ); return getIsVisible(); };
    void       setIsVisible( bool i_isVisible )       { m_isVisible = i_isVisible;             };
    bool       getIsVisible()                         { return m_isVisible;                    };

    void       setConvexHullColor( wxColour i_color ) { m_convexHullColor = i_color;            }; 
    wxColour   getConvexHullColor()                   { return m_convexHullColor;               };

    void       setConvexHullOpacity( float i_opacity) { m_convexHullOpacity = i_opacity;         };
    float      getConvexHullOpacity()                 { return m_convexHullOpacity;              };

    void       setMeanFiberColor( wxColour i_color )  { m_meanFiberColor = i_color;            }; 
    wxColour   getMeanFiberColor()                    { return m_meanFiberColor;               };

    void       setMeanFiberOpacity( float i_opacity) { m_meanFiberOpacity = i_opacity;         };
    float      getMeanFiberOpacity()                 { return m_meanFiberOpacity;              };

    void       setMeanFiberColorMode( FibersColorationMode i_mode ) { m_meanFiberColorationMode = i_mode; };
    FibersColorationMode getMeanFiberColorMode()     { return m_meanFiberColorationMode;        };
    
    // Methods related to the different fiber bundles selection.
    typedef    wxString FiberIdType;
    struct SelectionState
    {
        public: 
            SelectionState()
            : m_inBoxNeedsUpdating( true )
            , m_inBranchNeedsUpdating( true )
            {};
            
            vector< bool > m_inBranch;
            vector< bool > m_inBox;
            bool           m_inBoxNeedsUpdating;
            // TODO selection don't think we need this.
            bool           m_inBranchNeedsUpdating;
    };
    
    bool            addFiberDataset(    const FiberIdType &fiberId, const int fiberCount );
    void            removeFiberDataset( const FiberIdType &fiberId );
    SelectionState& getState(           const FiberIdType &fiberId );
    
    // Methods related to saving and loading.
    // TODO selection tree
    //bool populateXMLNode( wxXmlNode *pCurNode );
    //virtual bool loadFromXMLNode( wxXmlNode *pSelObjNode );
    
    virtual wxString getTypeTag() const;

    //Distance coloring setup
    bool        IsUsedForDistanceColoring() const;
    void        UseForDistanceColoring(bool aUse);

    //Normal flips
    // Do not flip for generic selection objects.
    virtual void flipNormals() {};

    // Variables
    std::vector< bool > m_inBox;
    std::vector< bool > m_inBranch;
    Anatomy *           m_sourceAnatomy;
    bool                m_boxMoved;
    bool                m_boxResized;


protected :
    virtual void drawObject( GLfloat* i_color ) = 0;
    
    void  drag  ( wxPoint i_click, wxPoint i_lastPos, GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] );
    void  resize( wxPoint i_click, wxPoint i_lastPos, GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] );
    void  updateStatusBar();
    float getAxisParallelMovement( int i_x1, int i_y1, int i_x2, int i_y2, Vector i_n, GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] );
    
    Vector          m_center;
    
    std::list< Face3D >  m_hullTriangles;

    wxColour        m_color;         // Used for coloring the isosurface.

    bool            m_mustUpdateConvexHull;
    bool            m_gfxDirty;
    float           m_handleRadius;
    hitResult       m_hitResult;
    bool            m_isActive;
    bool            m_isLockedToCrosshair;
    bool            m_isMaster;
    bool            m_isNOT;
    CIsoSurface*    m_isosurface;
    bool            m_isSelected;
    bool            m_isVisible;
    wxString        m_name;
    ObjectType      m_objectType;
    Vector          m_size;
    int             m_stepSize;
    float           m_threshold;
    wxTreeItemId    m_treeId;
    
    bool            m_statsAreBeingComputed;

    //Distance coloring switch
    bool            m_DistColoring;

    wxColour m_convexHullColor;
    float    m_convexHullOpacity; //Between 0 and 1
    
    //Mean fiber coloring variables
    wxColour m_meanFiberColor; //Custom color chose by the user
    std::vector< Vector > m_meanFiberColorVector; //Vector of colour compute by the program
    float m_meanFiberOpacity; //Between 0 and 1
    FibersColorationMode m_meanFiberColorationMode;

    // Those variables represent the min/max value in pixel of the object.
    float m_minX;
    float m_minY;
    float m_minZ;
    float m_maxX;
    float m_maxY;
    float m_maxZ;
    

    std::map< FiberIdType, SelectionState > m_selectionStates;
    
    void notifyInBoxNeedsUpdating();
    void notifyInBranchNeedsUpdating();

    /******************************************************************************************
    * Functions/variables related to the fiber info calculation.
    ******************************************************************************************/
public:
    // Functions
    void   updateStats                       ();
    // TODO selection make private?
    void   notifyStatsNeedUpdating           ();
    bool   statsAreBeingComputed             () { return m_statsAreBeingComputed; };

    void   calculateGridParams               (       FibersInfoGridParams       &io_gridInfo               );
    void   computeMeanFiber                  ();
    void   computeConvexHull                 ();
    void   getProgressionCurvature           ( const Vector                     &i_point0, 
                                               const Vector                     &i_point1, 
                                               const Vector                     &i_point2, 
                                               const Vector                     &i_point3, 
                                               const Vector                     &i_point4,
                                                     double                      i_progression,
                                                     double                     &o_curvature               );
    void   getProgressionTorsion             ( const Vector                     &i_point0, 
                                               const Vector                     &i_point1, 
                                               const Vector                     &i_point2, 
                                               const Vector                     &i_point3, 
                                               const Vector                     &i_point4,
                                                     double                      i_progression,
                                                     double                     &o_torsion                 );
    void   SetFiberInfoGridValues             ();
    void   updateMeanFiberOpacity             ();
    void   UpdateMeanValueTypeBox             ();
    void   updateConvexHullOpacity            ();
protected:
    void   drawCrossSections                 ();
    void   drawCrossSectionsPolygons         ();
    void   drawDispersionCone                ();
    void   drawFibersInfo                    ();
    void   setNormalColorArray               ( const std::vector< Vector > &i_fiberPoints);
    void   setShowMeanFiberOption            ( bool i_val );
    void   drawPolygon                       ( const std::vector< Vector >           &i_crossSectionPoints      );
    void   drawSimpleCircles                 ( const std::vector< std::vector< Vector > > &i_allCirclesPoints        );
    void   drawThickFiber                    ( const std::vector< Vector >           &i_fiberPoints,
                                                     float                           i_thickness, 
                                                     int                             i_nmTubeEdge               );
    void   drawConvexHull                    ();
    void   setShowConvexHullOption           (bool i_val);
    void   drawTube                          ( const std::vector< std::vector< Vector > > &i_allCirclesPoints,
                                                     GLenum                          i_tubeType               );
    void   getCrossSectionAreaColor          (       unsigned int                    i_index                   );
    void   getDispersionCircle               ( const std::vector< Vector >           &i_crossSectionPoints, 
                                               const Vector                          &i_crossSectionNormal, 
                                                     std::vector< Vector >           &o_circlePoints            );
    bool   getFiberCoordValues               (       int                             fiberIndex, 
                                                     std::vector< Vector >           &o_fiberPoints             );
    bool   getFiberLength                    ( const std::vector< Vector >           &i_fiberPoints,
                                                     float                           &o_length                  );
    bool   getFiberMeanCurvatureAndTorsion   ( const std::vector< Vector >           &i_fibersPoints, 
                                                     float                           &o_curvature,
                                                     float                           &o_torsion                 );
    bool   getFiberPlaneIntersectionPoint    ( const std::vector< Vector >           &i_fiberPoints, 
                                               const Vector                          &i_pointOnPlane,
                                               const Vector                          &i_planeNormal,
                                                     std::vector< Vector >           &o_intersectionPoints      );
    bool   getFibersCount                    (       int                             &o_count                   );
    bool   getFiberDispersion                (       float                           &o_dispersion              );
    bool   getFibersMeanCurvatureAndTorsion  ( const std::vector< std::vector< Vector > > &i_fibersPoints, 
                                                     float                           &o_meanCurvature, 
                                                     float                           &o_meanTorsion             );  
    float  getMaxDistanceBetweenPoints       ( const std::vector< Vector >           &i_points, 
                                                     int*                            o_firstPointIndex = NULL, 
                                                     int*                            o_secondPointIndex = NULL );
    bool   getMeanFiber                      ( const std::vector< std::vector< Vector > > &i_fibersPoints,
                                                     unsigned int                    i_nbPoints,
                                                     std::vector< Vector >           &o_meanFiber               );
    bool   getMeanFiberValue                 ( const std::vector< std::vector< Vector > > &fibersPoints, 
                                                     float                           &computedMeanValue         );
    
    bool   getMeanMaxMinFiberCrossSection    ( const std::vector< std::vector< Vector > > &i_fibersPoints,
                                               const std::vector< Vector >           &i_meanFiberPoints,
                                                     float                           &o_meanCrossSection,
                                                     float                           &o_maxCrossSection,
                                                     float                           &o_minCrossSection         );
    // TODO TBR selection tree
    bool   getMeanMaxMinFiberLength          ( const std::vector< std::vector< Vector > > &i_fibersPoints,
                                                     float                           &o_meanLength,
                                                     float                           &o_maxLength,
                                                     float                           &o_minLength               );
    bool   getMeanMaxMinFiberLengthNew(       const vector< int > &selectedFibersIndexes,
                                       Fibers        *pCurFibers,
                                       float                      &o_meanLength,
                                       float                      &o_maxLength,
                                       float                      &o_minLength                );

    void   getProgressionCurvatureAndTorsion ( const Vector                          &i_point0, 
                                               const Vector                          &i_point1, 
                                               const Vector                          &i_point2, 
                                               const Vector                          &i_point3, 
                                               const Vector                          &i_point4,
                                                     double                          i_progression,
                                                     double                          &o_curvature,
                                                     double                          &o_torsion                 );

    bool   getShowFibers                      ();

    std::vector< std::vector< Vector > >   getSelectedFibersPoints ();
    
    vector< int > getSelectedFibersIndexes( Fibers *pFibers );
    bool          getSelectedFibersInfo( const vector< int > &selectedFibersIdx, 
                                        Fibers *pFibers,
                                        vector< int > &pointsCount, 
                                        vector< vector< Vector > > &fibersPoints );

    
    std::vector< float >        m_crossSectionsAreas;   // All the cross sections areas value.
    std::vector< Vector >       m_crossSectionsNormals; // All the cross sections normals value.
    std::vector< std::vector < Vector > > m_crossSectionsPoints;  // All the cross sections hull points in 3D.
    unsigned int                m_maxCrossSectionIndex; // Index of the max cross section of m_crossSectionsPoints.
    std::vector< Vector >       m_meanFiberPoints;      // The points representing the mean fiber.
    unsigned int                m_minCrossSectionIndex; // Index of the min cross section of m_crossSectionsPoints.
    
    FibersInfoGridParams        m_stats;                // The stats for this box.
    bool                        m_statsNeedUpdating;    // Will be used to check if the stats
    /******************************************************************************************
    * END of the functions/variables related to the fiber info calculation.
    *****************************************************************************************/

protected:
    wxBitmapButton  *m_pbtnSelectColor;

private:
    wxTextCtrl      *m_pTxtName;
    wxToggleButton  *m_pToggleVisibility;
    wxToggleButton  *m_pToggleActivate;
    wxToggleButton  *m_pToggleCalculatesFibersInfo;
    wxGrid          *m_pGridFibersInfo;
    wxToggleButton  *m_pToggleDisplayMeanFiber;
//     wxToggleButton  *m_pToggleDisplayConvexHull;
//     wxBitmapButton  *m_pBtnSelectConvexHullColor;
//     wxStaticText    *m_pLblConvexHullOpacity;
//     wxSlider        *m_pSliderConvexHullOpacity;
    wxBitmapButton  *m_pBtnSelectMeanFiberColor;
    wxStaticText    *m_pLblColoring;
    wxRadioButton   *m_pRadCustomColoring;
    wxRadioButton   *m_pRadNormalColoring;
    wxStaticText    *m_pLblMeanFiberOpacity;
    wxSlider        *m_pSliderMeanFiberOpacity;
    wxButton        *m_pbtnDisplayCrossSections;
    wxButton        *m_pbtnDisplayDispersionTube;
    wxStaticText    *m_pLabelAnatomy;
    wxChoice        *m_pCBSelectDataSet;

public:
    wxTextCtrl      *m_pTxtBoxX;
    wxTextCtrl      *m_pTxtBoxY;
    wxTextCtrl      *m_pTxtBoxZ;
    wxTextCtrl      *m_pTxtSizeX;
    wxTextCtrl      *m_pTxtSizeY;
    wxTextCtrl      *m_pTxtSizeZ;
    
    static const int    DISPERSION_CONE_NB_TUBE_EDGE=25; // This value represent the number of edge the dispersion cone will have.
    static const int    MEAN_FIBER_NB_POINTS=50;         // This value represent the number of points we want the mean fiber to have.
    static const int    THICK_FIBER_NB_TUBE_EDGE=10;     // This value represent the number of edge the tube of the thick fiber will have.
    static const int    THICK_FIBER_THICKNESS=33;        // This value represent the size of the tube the thick fiber will have (*1/100).

public:
    CrossSectionsDisplay   m_displayCrossSections;
    DispersionConeDisplay  m_displayDispersionCone;
};

//////////////////////////////////////////////////////////////////////////

inline bool SelectionObject::IsUsedForDistanceColoring() const
{
    return m_DistColoring;
}

//////////////////////////////////////////////////////////////////////////

inline void SelectionObject::UseForDistanceColoring(bool aUse)
{
    m_DistColoring = aUse;
}

//////////////////////////////////////////////////////////////////////////


#endif /*SELECTIONOBJECT_H_*/
