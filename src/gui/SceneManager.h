#ifndef SCENEMANAGER_H_
#define SCENEMANAGER_H_

#include "../misc/IsoSurface/Vector.h"

#include <GL/glew.h>
#include <wx/string.h>

#include <vector>

class MainFrame;
class MyTreeCtrl;
class SelectionObject;

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

    void    setMainFrame( MainFrame *pMainFrame ) { m_pMainFrame = pMainFrame; }
    void    setTreeCtrl ( MyTreeCtrl *pTreeCtrl ) { m_pTreeView  = pTreeCtrl; }

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

    float m_sliceX;
    float m_sliceY;
    float m_sliceZ;
};

#endif SCENEMANAGER_H_