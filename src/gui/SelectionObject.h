/////////////////////////////////////////////////////////////////////////////
// Name:            SelectionObject.h
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   10/26/2009
//
// Description: SelectionObject class.
//
// Last modifications:
//      by : Imagicien - 12/11/2009
/////////////////////////////////////////////////////////////////////////////

#ifndef SELECTIONOBJECT_H_
#define SELECTIONOBJECT_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "boundingBox.h"
#include "GL/glew.h"
#include <vector>
#include "../dataset/DatasetHelper.h"
#include "../misc/Algorithms/Helper.h"
#include "../misc/IsoSurface/Vector.h"

class Anatomy;
class CIsoSurface;
class DatasetHelper;
class MainCanvas;

using namespace std;

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
        //nothing
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

class SelectionObject : public wxTreeItemData
{
public :
    // Constructor / Destructor
    SelectionObject ( Vector i_center, Vector i_size, DatasetHelper* i_datasetHelper );
    ~SelectionObject();

    // Virtual Pure functions
    virtual hitResult hitTest( Ray* i_ray ) = 0;

    // Virtual functions
    virtual void objectUpdate();

    // Functions
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
	
	// Set/get and there affiliated functions
	bool	   toggleIsActive();
	void	   setIsActive( bool i_isActive )         { m_isActive = i_isActive;               };
	bool	   getIsActive()                          { return m_isActive;                     };

	ObjectType getSelectionType()                     { return m_objectType;                   };	
    bool       isSelectionObject();

    void	   setCenter( float i_x, float i_y, float i_z );
	void	   setCenter( Vector i_center );
	Vector	   getCenter()                            { return m_center;                       };

	void       setColor( wxColour i_color );
	wxColour   getColor()                             { return m_color;                        };

	void	   setColorChanged( bool i_colorChanged ) { m_colorChanged = i_colorChanged;       };
	bool	   isColorChanged()                       { return m_colorChanged;                 };

	void	   setIsDirty( bool i_isDirty );
	bool	   getIsDirty()                           { return m_isDirty;                      };

	void       setFiberColor( wxColour i_color );
	wxColour   getFiberColor()                        { return m_fiberColor;                   };

	int		   getIcon();

	void       setName( wxString i_name )             { m_name = i_name;                       };
	wxString   getName()                              { return m_name;                         };

	void	   setIsMaster( bool i_isMaster );
	bool	   getIsMaster()                          { return m_isMaster;                     };
	
	bool	   toggleIsNOT()                          { return m_isNOT = ! m_isNOT;            };
	void	   setIsNOT( bool i_isNOT )               { m_isNOT = i_isNOT;                     };
	bool	   getIsNOT()                             { return m_isNOT;                        };

	void	   setPicked( int i_picked )              { m_hitResult.picked = i_picked;         };

	void	   setSize( Vector i_size )               { m_size = i_size; m_isDirty = true;     };
	Vector	   getSize()                              { return m_size;};

	void	   setThreshold( float i_threshold );
	float	   getThreshold()                         { return m_threshold;                    };

	void	   setTreeId( wxTreeItemId i_treeId )     { m_treeId = i_treeId;                   };
	
	void	   unselect()                             { m_isSelected = false;                  };
	
	bool	   toggleIsVisible()                      { return m_isVisible = ! m_isVisible;    };
    void	   setIsVisible( bool i_isVisible )       { m_isVisible = i_isVisible;             };
	bool	   getIsVisible()                         { return m_isVisible;                    };

    //Distance coloring setup
    bool        IsUsedForDistanceColoring() const;
    void        UseForDistanceColoring(bool aUse);

    //Normal flips
    void        FlipNormals();


    // Variables
    DatasetHelper* m_datasetHelper;
	vector< bool > m_inBox;
	vector< bool > m_inBranch;
	Anatomy*       m_sourceAnatomy;

protected :
	// Virtual Pure functions
    virtual void drawObject( GLfloat* i_color ) = 0;
    
    // Functions
	void  drag  ( wxPoint i_click, wxPoint i_lastPos, GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] );
	void  resize( wxPoint i_click, wxPoint i_lastPos, GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] );
	void  updateStatusBar();
	float getAxisParallelMovement( int i_x1, int i_y1, int i_x2, int i_y2, Vector i_n, GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16] );
	
	// Variables
	Vector			m_center;
	wxColour		m_color;		// Used for coloring the isosurface.
	bool			m_colorChanged;
	wxColour		m_fiberColor;	// Used for the selected fibers.
	bool			m_gfxDirty;
	float			m_handleRadius;
	hitResult		m_hitResult;
	bool			m_isActive;
	bool			m_isDirty;
	bool			m_isLockedToCrosshair;
	bool			m_isMaster;
	bool			m_isNOT;
	CIsoSurface*	m_isosurface;
	bool			m_isSelected;
	bool			m_isVisible;
	wxString		m_name;
    ObjectType      m_objectType;
	Vector			m_size;
	int				m_stepSize;
	float			m_threshold;
	wxTreeItemId	m_treeId;

    //Distance coloring switch
    bool            m_DistColoring;
	
    // Those variables represent the min/max value in pixel of the object.
    float m_minX;
    float m_minY;
    float m_minZ;
    float m_maxX;
    float m_maxY;
    float m_maxZ;

    /******************************************************************************************
    * Functions/variables related to the fiber info calculation.
    ******************************************************************************************/
public:
    // Functions
    void   calculateGridParams               (       FibersInfoGridParams       &io_gridInfo               );
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
  
protected:
    // Functions
    void   drawCrossSections                 ();
    void   drawCrossSectionsPolygons         ();
    void   drawDispersionCone                ();
    void   drawFibersInfo                    ();
    void   drawPolygon                       ( const vector< Vector >           &i_crossSectionPoints      );
    void   drawSimpleCircles                 ( const vector< vector< Vector > > &i_allCirclesPoints        );
    void   drawThickFiber                    ( const vector< Vector >           &i_fiberPoints,
                                                     float                      i_thickness, 
                                                     int                        i_nmTubeEdge               );
    void   drawTube                          ( const vector< vector< Vector > > &i_allCirclesPoints,
                                                      GLenum                      i_tubeType               );
    void   getCrossSectionAreaColor          (       unsigned int                i_index                   );
    void   getDispersionCircle               ( const vector< Vector >           &i_crossSectionPoints, 
                                               const Vector                     &i_crossSectionNormal, 
                                                     vector< Vector >           &o_circlePoints            );
    bool   getFiberCoordValues               (       int                         fiberIndex, 
                                                     vector< Vector >           &o_fiberPoints             );
    bool   getFiberLength                    ( const vector< Vector >           &i_fiberPoints,
                                                     float                      &o_length                  );
    bool   getFiberMeanCurvatureAndTorsion   ( const vector< Vector >           &i_fibersPoints, 
                                                     float                      &o_curvature,
                                                     float                      &o_torsion                 );
    bool   getFiberPlaneIntersectionPoint    ( const vector< Vector >           &i_fiberPoints, 
                                               const Vector                     &i_pointOnPlane,
                                               const Vector                     &i_planeNormal,
                                                     vector< Vector >           &o_intersectionPoints      );
    bool   getFibersCount                    (       int                        &o_count                   );
    bool   getFiberDispersion                (       float                      &o_dispersion              );
    bool   getFibersMeanCurvatureAndTorsion  ( const vector< vector< Vector > > &i_fibersPoints, 
                                                     float                      &o_meanCurvature, 
                                                     float                      &o_meanTorsion             );  
    float  getMaxDistanceBetweenPoints       ( const vector< Vector >           &i_points, 
                                                     int*                        o_firstPointIndex = NULL, 
                                                     int*                        o_secondPointIndex = NULL );
    bool   getMeanFiber                      ( const vector< vector< Vector > > &i_fibersPoints,
                                                     unsigned int                i_nbPoints,
                                                     vector< Vector >           &o_meanFiber               );
    bool   getMeanFiberValue                 ( const vector< vector< Vector > > &i_fibersPoints, 
                                                     float                      &o_meanValue               );
    
    bool   getMeanMaxMinFiberCrossSection    ( const vector< vector< Vector > > &i_fibersPoints,
                                               const vector< Vector >           &i_meanFiberPoints,
                                                     float                      &o_meanCrossSection,
                                                     float                      &o_maxCrossSection,
                                                     float                      &o_minCrossSection         );
    bool   getMeanMaxMinFiberLength          ( const vector< vector< Vector > > &i_fibersPoints,
                                                     float                      &o_meanLength,
                                                     float                      &o_maxLength,
                                                     float                      &o_minLength               );
    void   getProgressionCurvatureAndTorsion ( const Vector                     &i_point0, 
                                               const Vector                     &i_point1, 
                                               const Vector                     &i_point2, 
                                               const Vector                     &i_point3, 
                                               const Vector                     &i_point4,
                                                     double                      i_progression,
                                                     double                     &o_curvature,
                                                     double                     &o_torsion                 );
    
    // Variables
    vector< float >             m_crossSectionsAreas;   // All the cross sections areas value.
    vector< Vector >            m_crossSectionsNormals; // All the cross sections normals value.
    vector< vector < Vector > > m_crossSectionsPoints;  // All the cross sections hull points in 3D.
    unsigned int                m_maxCrossSectionIndex; // Index of the max cross section of m_crossSectionsPoints.
    vector< Vector >            m_meanFiberPoints;      // The points representing the mean fiber.
    unsigned int                m_minCrossSectionIndex; // Index of the min cross section of m_crossSectionsPoints.
    /******************************************************************************************
    * END of the functions/variables related to the fiber info calculation.
    *****************************************************************************************/
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
