#ifndef SCENEMANAGER_H_
#define SCENEMANAGER_H_

#include "ArcBall.h"
#include "../misc/IsoSurface/Vector.h"

#include <GL/glew.h>
#include <wx/string.h>

#include <vector>

class MainFrame;
class MyTreeCtrl;
class SelectionObject;
class ShaderHelper;
class TheScene;

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

    void    updateView( const float x, const float y, const float z );
    void    changeZoom( const int z );
    void    moveScene ( const int x, const int y );
    Vector  mapMouse2World(     const int x, 
                                const int y, 
                                GLdouble projection[16], 
                                GLint viewport[4], 
                                GLdouble modelview[16] );
    Vector  mapMouse2WorldBack( const int x, 
                                const int y, 
                                GLdouble projection[16], 
                                GLint viewport[4], 
                                GLdouble modelview[16] );

    void    doMatrixManipulation();

    SelectionObjList getSelectionObjects();

    void    setMainFrame( MainFrame *pMainFrame )           { m_pMainFrame = pMainFrame; }
    void    setShaderHelper( ShaderHelper *pShaderHelper )  { m_pShaderHelper = pShaderHelper; }
    void    setTreeCtrl ( MyTreeCtrl *pTreeCtrl )           { m_pTreeView  = pTreeCtrl; }
    // TODO: Remove this temporary getter/setter once TheScene is created directly in the constructor of the SceneManager
    void    setTheScene ( TheScene *pTheScene )             { m_pTheScene  = pTheScene; }

    TheScene * getScene() const             { return m_pTheScene; }
    ShaderHelper * getShaderHelper() const  { return m_pShaderHelper; }

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

    Matrix4fT & getTransform() { return m_transform; }
    float getZoom() const   { return m_zoom; }
    float getMoveX() const  { return m_moveX; }
    float getMoveY() const  { return m_moveY; }
    void  setZoom( float zoom ) { m_zoom = zoom; } 
    void  setMoveX( float x )   { m_moveX = x; }
    void  setMoveY( float y )   { m_moveY = y; }

protected:
    SceneManager(void);

private:
    SceneManager( const SceneManager & );
    SceneManager &operator=( const SceneManager & );

private:
    static SceneManager *m_pInstance;

    MyTreeCtrl *m_pTreeView;
    MainFrame  *m_pMainFrame;
    ShaderHelper *m_pShaderHelper;
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
};

#endif SCENEMANAGER_H_