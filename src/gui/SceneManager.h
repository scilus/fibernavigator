#ifndef SCENEMANAGER_H_
#define SCENEMANAGER_H_

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

    void    increaseAnimationStep();
    void    doMatrixManipulation();

    SelectionObjList getSelectionObjects();

    void    setMainFrame( MainFrame *pMainFrame )           { m_pMainFrame = pMainFrame; }
    void    setShaderHelper( ShaderHelper *pShaderHelper )  { m_pShaderHelper = pShaderHelper; }
    void    setTreeCtrl ( MyTreeCtrl *pTreeCtrl )           { m_pTreeView  = pTreeCtrl; }
    // TODO: Remove this temporary getter/setter once TheScene is created directly in the constructor of the SceneManager
    void    setTheScene ( TheScene *pTheScene )             { m_pTheScene  = pTheScene; }

    TheScene * getScene() const             { return m_pTheScene; }
    ShaderHelper * getShaderHelper() const  { return m_pShaderHelper; }

//////////////////////////////////////////////////////////////////////////
//  GETTERS / SETTERS
//////////////////////////////////////////////////////////////////////////

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

    bool toggleLightingDisplay()        { return m_lighting       = !m_lighting; }
    bool toggleAlphaBlend()             { return m_blendAlpha     = !m_blendAlpha; }
    bool toggleTexBlendOnMesh()         { return m_blendTexOnMesh = !m_blendTexOnMesh; }
    bool toggleAxialDisplay()           { return m_showAxial      = !m_showAxial; }
    bool toggleCoronalDisplay()         { return m_showCoronal    = !m_showCoronal; }
    bool toggleSagittalDisplay()        { return m_showSagittal   = !m_showSagittal; }
    bool toggleCrosshairDisplay()       { return m_showCrosshair  = !m_showCrosshair; }
    
    float getSliceX() const     { return m_sliceX; }
    float getSliceY() const     { return m_sliceY; }
    float getSliceZ() const     { return m_sliceZ; }

    void  setSliceX( float x )  { m_sliceX = x; }
    void  setSliceY( float y )  { m_sliceY = y; }
    void  setSliceZ( float z )  { m_sliceZ = z; }

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

    float m_sliceX;
    float m_sliceY;
    float m_sliceZ;
};

#endif SCENEMANAGER_H_