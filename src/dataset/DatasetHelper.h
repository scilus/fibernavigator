/*
 * DatasetHelper.h
 *
 *  Created on: 27.07.2008
 *      Author: ralph
 */
#ifndef DATASETHELPER_H_
#define DATASETHELPER_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/textfile.h"
#include "wx/file.h"
#include "wx/image.h"
#include "wx/wfstream.h"
#include "wx/datstrm.h"
#include "wx/txtstrm.h"
#include "wx/xml/xml.h"

#include <vector>

#include "DatasetInfo.h"

#include "../gui/SelectionObject.h"

#include "../gui/MainFrame.h"
#include "../gfx/TheScene.h"
#include "SplinePoint.h"

#include "AnatomyHelper.h"
#include "../gfx/ShaderHelper.h"

#include "../misc/lic/TensorField.h"

#include "../misc/Fantom/FMatrix.h"

#include "../Logger.h"

class MainFrame;
class DatasetInfo;
class TheScene;
class AnatomyHelper;
class ShaderHelper;
class SplinePoint;
class SelectionObject;
class Fibers;
class FibersGroup;
class TensorField;
class Surface;

class DatasetHelper 
{
public:
    // Constructor/destructor
    DatasetHelper( MainFrame* i_mainFrame );
    virtual ~DatasetHelper();

    // Functions
    void   deleteAllPoints();
    Vector mapMouse2World( const int i_x, const int i_y,GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16]);
    Vector mapMouse2WorldBack( const int i_x, const int i_y,GLdouble i_projection[16], GLint i_viewport[4], GLdouble i_modelview[16]);   

    /*
     * Called from MainFrame when a kdTree thread signals it's finished
     */
    void treeFinished();

    void changeZoom( const int i_z );
    void moveScene ( int i_x, int i_y );

    void doMatrixManipulation();

	bool getFibersGroupDataset( FibersGroup* &i_fiberGroup );
	bool getSelectedFiberDataset ( Fibers* &i_fiber );
    bool getSurfaceDataset( Surface* &i_surface );
    bool getTextureDataset( std::vector< DatasetInfo* > &o_types ); 
    std::vector< float >* getVectorDataset();
    TensorField* getTensorField();

    void toggleShowAllSelectionObjects() { m_showObjects = ! m_showObjects; };
    void toggleActivateAllSelectionObjects() { m_activateObjects = ! m_activateObjects; };
    bool togglePointMode()        { return m_pointMode = ! m_pointMode; };
    bool getPointMode()           { return m_pointMode; };

    void doLicMovie       ( int i_mode );
    void createLicSliceSag( int i_slize );
    void createLicSliceCor( int i_slize );
    void createLicSliceAxi( int i_slize );
    void licMovieHelper();

    void increaseAnimationStep();

    /////////////////////////////////////////////////////////////////////////////////
    // general info about the datasets
    /////////////////////////////////////////////////////////////////////////////////
    //std::vector< float >* m_floatDataset;
    std::vector<Vector>   m_rulerPts;
    bool                  m_isRulerToolActive;
    double                m_rulerFullLength;
    double                m_rulerPartialLength;
    //int                   m_fibersSamplingFactor;
    bool                  m_isSegmentActive; // SceneManager
    int                   m_SegmentMethod; // SceneManager - Add enum
    //bool                  m_isFloodfillActive;
    //bool                  m_isSelectBckActive;
    //bool                  m_isSelectObjActive;
    //bool                  m_isObjfilled;
    //bool                  m_isBckfilled;
    //bool                  m_isObjCreated;
    //bool                  m_isBckCreated;
    //bool                  m_isBoxCreated;
    //bool                  m_thresSliderMoved;
    //bool graphcutReady()  { return (m_isObjfilled && m_isBckfilled && m_isBoxCreated); };

    FMatrix m_niftiTransform;

    unsigned int m_countFibers;

    //bool m_loadDataset;
    bool m_scnFileLoaded;
    bool m_surfaceIsDirty;

    /////////////////////////////////////////////////////////////////////////////////
    // state variables for rendering
    /////////////////////////////////////////////////////////////////////////////////
    Matrix4fT m_transform;
    bool      m_useVBO;
    int       m_quadrant;
    //int       m_textures;
    //! if set the shaders will be reloaded during next render() call
    bool      m_scheduledReloadShaders;
    // the screenshot button has been pressed, next render pass it will be executed
    bool      m_scheduledScreenshot;

    bool      m_showObjects;
    bool      m_activateObjects;
    bool      m_blendAlpha;
    bool      m_pointMode;
    bool      m_isShowAxes;

    int       m_animationStep;

    //int       m_debugLevel;

    float     m_frustum[6][4]; // Contains the information of the planes forming the frustum.
    /////////////////////////////////////////////////////////////////////////////////
    // state variables for menu entries
    /////////////////////////////////////////////////////////////////////////////////
    //bool m_showSagittal;
    //bool m_showCoronal;
    //bool m_showAxial;

    //bool m_showCrosshair;

    //bool  m_lighting;         // m_lighting for fibers.
    bool  m_blendTexOnMesh;   // Ignore threshold for textures on meshes.
    //bool  m_useLic;           // Show the lic texture on spline surface.
    bool  m_drawVectors;      // Draw vectors as small lines on spline surface.
    float m_normalDirection;  // Normal direction of the spline surface.
    bool  m_geometryShadersSupported;
    bool  m_clearToBlack;
    bool  m_useFibersGeometryShader;
    bool  m_filterIsoSurf;
    int   m_colorMap;
    bool  m_showColorMapLegend;
    //bool  m_displayMinMaxCrossSection;
    //bool  m_displayGlyphOptions;

	bool  m_isDrawerToolActive;
	enum  DrawMode
	{
		DRAWMODE_PEN = 0,
		DRAWMODE_ERASER = 1,
        DRAWMODE_INVALID
	};
	DrawMode m_drawMode;
	int     m_drawSize;
	bool    m_drawRound;
	bool    m_draw3d;
    bool    m_canUseColorPicker;
	wxColor m_drawColor;
	wxImage m_drawColorIcon;

    //bool  m_morphing;

    bool  m_boxLockIsOn;
    //bool  m_semaphore;
    int   m_threadsActive;

   /////////////////////////////////////////////////////////////////////////////////
    // variables for mouse interaction
    /////////////////////////////////////////////////////////////////////////////////
    bool  m_isDragging;
    bool  m_isrDragging;
    bool  m_ismDragging;
    float m_zoom;
    float m_xMove;
    float m_yMove;

    bool  m_texAssigned;
    bool  m_selBoxChanged;

    int   m_geforceLevel;

    wxString m_scenePath;
    wxString m_scnFileName;
    wxString m_screenshotPath;
    wxString m_screenshotName;

    /////////////////////////////////////////////////////////////////////////////////
    // pointers to often used objects
    /////////////////////////////////////////////////////////////////////////////////
    AnatomyHelper*   m_anatomyHelper;
    SelectionObject* m_boxAtCrosshair;
    SplinePoint*     m_lastSelectedPoint;
    SelectionObject* m_lastSelectedObject;
    TheScene*        m_theScene;
    ShaderHelper*    m_shaderHelper;
    
    MainFrame*       m_mainFrame;
};

#define ID_KDTREE_FINISHED    50

#endif /* DATASETHELPER_H_ */
