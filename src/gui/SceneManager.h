#ifndef SCENEMANAGER_H_
#define SCENEMANAGER_H_

#include "ArcBall.h"
#include "../misc/IsoSurface/Vector.h"

#include <GL/glew.h>
#include <wx/string.h>

#include <vector>

class AnatomyHelper;
class MainFrame;
class MyTreeCtrl;
class SelectionObject;
class ShaderHelper;
class TheScene;
class wxXmlNode;

typedef std::vector< std::vector< SelectionObject * > > SelectionObjList;

enum SEGMETHOD { FLOODFILL = 0, GRAPHCUT, KMEANS };

class SceneManager
{
public:
    ~SceneManager(void);
    
    static SceneManager * getInstance();

    bool    load( const wxString &filename );
    bool    save( const wxString &filename );

    void    deleteAllSelectionObjects();
    void    updateAllSelectionObjects();

    void    updateView( const float x, const float y, const float z, bool semaphore = false );
    void    changeZoom( const int z );
    void    moveScene ( const int x, const int y );

    void    doMatrixManipulation();

    SelectionObjList getSelectionObjects();

    void    setMainFrame( MainFrame *pMainFrame )           { m_pMainFrame = pMainFrame; }
    void    setTreeCtrl ( MyTreeCtrl *pTreeCtrl )           { m_pTreeView  = pTreeCtrl; }

    AnatomyHelper* getAnatomyHelper() const { return m_pAnatomyHelper; }
    TheScene *     getScene() const         { return m_pTheScene; }

    bool areGeometryShadersSupported() const    { return m_geometryShadersSupported; }
    bool isFibersGeomShaderActive() const       { return m_useFibersGeometryShader; }

    void setGeometryShaderSupported( bool supported )   { m_geometryShadersSupported = supported; }
    bool toggleFibersGeomShaderActive()     { return m_useFibersGeometryShader = !m_useFibersGeometryShader; }

    bool isLightingActive() const           { return m_lighting; }
    bool isAlphaBlend() const               { return m_blendAlpha; }
    bool isTexBlendOnMesh() const           { return m_blendTexOnMesh; }
    bool isAxialDisplayed() const           { return m_showAxial; }
    bool isCoronalDisplayed() const         { return m_showCoronal; }
    bool isSagittalDisplayed() const        { return m_showSagittal; }
    bool isCrosshairDisplayed() const       { return m_showCrosshair; }
    bool areAxesDisplayed() const           { return m_showAxes; }

    bool toggleLightingDisplay()        { return m_lighting       = !m_lighting; }
    bool toggleAlphaBlend()             { return m_blendAlpha     = !m_blendAlpha; }
    bool toggleTexBlendOnMesh()         { return m_blendTexOnMesh = !m_blendTexOnMesh; }
    bool toggleAxialDisplay()           { return m_showAxial      = !m_showAxial; }
    bool toggleCoronalDisplay()         { return m_showCoronal    = !m_showCoronal; }
    bool toggleSagittalDisplay()        { return m_showSagittal   = !m_showSagittal; }
    bool toggleCrosshairDisplay()       { return m_showCrosshair  = !m_showCrosshair; }
    bool toggleAxesDisplay()            { return m_showAxes       = !m_showAxes; }
    
    float getSliceX() const         { return m_sliceX; }
    float getSliceY() const         { return m_sliceY; }
    float getSliceZ() const         { return m_sliceZ; }

    void  setSliceX( float x )      { m_sliceX = x; }
    void  setSliceY( float y )      { m_sliceY = y; }
    void  setSliceZ( float z )      { m_sliceZ = z; }

    bool  isUsingVBO() const        { return m_useVBO; }
    void  setUsingVBO( bool state ) { m_useVBO = state; }

    int   getQuadrant() const       { return m_quadrant; }
    void  setQuadrant( int quad )   { m_quadrant = quad; }

    bool  isSegmentActive() const           { return m_segmentActive; }
    void  setSegmentActive( bool active )   { m_segmentActive = active; }
    bool  toggleSegmentActive()             { return m_segmentActive = !m_segmentActive; }

    SEGMETHOD   getSegmentMethod() const                { return m_segmentMethod; }
    void        setSegmentMethod( SEGMETHOD method )    { m_segmentMethod = method; }

    int   getAnimationStep() const   { return m_animationStep; }
    void  increaseAnimStep()         { m_animationStep = ( m_animationStep + 1 ) % 1000000; }
    void  setAnimationStep(int step) { m_animationStep = step; }
    bool  isPointMode() const        { return m_pointMode; }
    bool  togglePointMode()          { return m_pointMode = !m_pointMode; }

    bool  areVectorsDisplayed() const   { return m_drawVectors; }
    bool  toggleVectorsDisplay()        { return m_drawVectors = !m_drawVectors; }
    float getNormalDirection() const    { return m_normalDirection; }
    void  setNormalDirection( float dir )   { m_normalDirection = dir; }

    Matrix4fT getTransform() const { return m_transform; }
    void  setTransform( Matrix4fT mat ) { m_transform = mat; }
    void  setZoom( float zoom ) { m_zoom = zoom; } 
    void  setMoveX( float x )   { m_moveX = x; }
    void  setMoveY( float y )   { m_moveY = y; }

    bool  toggleShowAllSelObj()         { return m_showObjects = !m_showObjects; }
    bool  toggleActivateAllSelObj()     { return m_activateObjects = !m_activateObjects; }

    bool  getShowAllSelObj() const      { return m_showObjects; }
    bool  getActivateAllSelObj() const  { return m_activateObjects;}

    bool  isSceneFileLoaded() const     { return m_scnFileLoaded; }
    wxString getSceneFilename() const   { return m_scnFilename; }
    wxString getScenePath() const       { return m_scnPath; }
    void  setScenePath( const wxString &path )  { m_scnPath = path; }

    bool isScreenshotScheduled() const  { return m_scheduledScreenshot; }
    void setScreenshotScheduled( const bool b ) { m_scheduledScreenshot = b; }
    wxString getScreenshotName() const { return m_screenshotName; }
    wxString getScreenshotPath() const { return m_screenshotPath; }
    void setScreenshotName( const wxString &name )  { m_screenshotName = name; }
    void setScreenshotPath( const wxString &path )  { m_screenshotPath = path; }

    bool getClearToBlack() const { return m_clearToBlack; }
    bool toggleClearToBlack()    { return m_clearToBlack = !m_clearToBlack; }

    int  getColorMap() const                { return m_colorMap; }
    void setColorMap( const int color )     { m_colorMap = color; }

    bool isIsoSurfaceFiltered() const       { return m_filterIsoSurface; }
    bool toggleIsoSurfaceFiltered()         { return m_filterIsoSurface = !m_filterIsoSurface; }

    void setBoxLock( const bool lock )      { m_isBoxLocked = lock; }

    void setBoxAtCrosshair( SelectionObject * pBox ) { m_pBoxAtCrosshair = pBox; }

    bool isSelBoxChanged() const            { return m_selBoxChanged; }
    void setSelBoxChanged( bool changed )   { m_selBoxChanged = changed; }

    std::vector< Vector > & getRulerPts()   { return m_rulerPts; }
    bool   isRulerActive() const            { return m_isRulerActive; }
    double getRulerFullLength() const       { return m_rulerFullLength; }
    double getRulerPartialLenth() const     { return m_rulerPartialLength; }

    void   setRulerActive( const bool active )          { m_isRulerActive = active; }
    void   setRulerFullLength( const double length )    { m_rulerFullLength = length; }
    void   setRulerPartialLength( const double length ) { m_rulerPartialLength = length; }

public:
    float m_frustum[6][4];

protected:
    SceneManager(void);

private:
    SceneManager( const SceneManager & );
    SceneManager &operator=( const SceneManager & );

    bool loadOldVersion( wxXmlNode * pRoot );

private:
    static SceneManager *m_pInstance;

    AnatomyHelper *m_pAnatomyHelper;
    MyTreeCtrl *m_pTreeView;
    MainFrame  *m_pMainFrame;
    TheScene   *m_pTheScene;

    bool  m_geometryShadersSupported;
    bool  m_useFibersGeometryShader;

    bool  m_lighting;
    bool  m_blendAlpha;
    bool  m_blendTexOnMesh;

    bool  m_showAxial;
    bool  m_showCoronal;
    bool  m_showSagittal;

    bool  m_showCrosshair;
    bool  m_showAxes;

    float m_sliceX;
    float m_sliceY;
    float m_sliceZ;

    bool  m_useVBO;
    int   m_quadrant;

    bool  m_segmentActive;
    SEGMETHOD m_segmentMethod;

    int   m_animationStep;
    bool  m_pointMode;

    bool  m_drawVectors;
    float m_normalDirection;

    Matrix4fT m_transform;
    float     m_zoom;
    float     m_moveX;
    float     m_moveY;

    bool  m_showObjects;
    bool  m_activateObjects;

    bool     m_scnLoading;
    bool     m_scnFileLoaded;
    wxString m_scnFilename;
    wxString m_scnPath;

    bool  m_scheduledScreenshot;
    wxString m_screenshotName;
    wxString m_screenshotPath;

    bool  m_clearToBlack;
    int   m_colorMap;

    bool  m_filterIsoSurface;
    bool  m_isBoxLocked;

    SelectionObject *m_pBoxAtCrosshair;

    bool  m_selBoxChanged;

    // Ruler
    std::vector< Vector >   m_rulerPts;
    bool                    m_isRulerActive;
    double                  m_rulerFullLength;
    double                  m_rulerPartialLength;
};

#endif //SCENEMANAGER_H_
