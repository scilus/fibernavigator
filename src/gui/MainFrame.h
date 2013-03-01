/////////////////////////////////////////////////////////////////////////////
// Description: mainFrame class. Contains every elements of the GUI, and frame events
/////////////////////////////////////////////////////////////////////////////

#ifndef MAINFRAME_H_
#define MAINFRAME_H_

#include "ListCtrl.h"
#include "MainCanvas.h"
#include "MyListCtrl.h"
#include "../misc/Algorithms/Helper.h"

#include <wx/checkbox.h>
#include <wx/grid.h>
#include <wx/notebook.h>
#include <wx/treectrl.h>

class SelectionObject;
class DatasetInfo;
class ToolBar;
class MenuBar;
class SceneObject;
class SelectionObject;
class TrackingWindow;

enum DrawMode
{
    DRAWMODE_PEN = 0,
    DRAWMODE_ERASER = 1,
    DRAWMODE_INVALID
};

class MainFrame : public wxFrame
{
    friend class ToolBar;
    friend class MenuBar;
    friend class PropertiesWindow;
    friend class TrackingWindow;

public:
    MainFrame( const wxString &title, const wxPoint &pos, const wxSize &size );
    ~MainFrame();

    void createCutDataset();
    void createDistanceMap();
    void createDistanceMapAndIso();
    void createIsoSurface();
    void displayPropertiesSheet();
    void deleteSceneObject();
    void deleteListItem();
    void deleteTreeItem();
    void refreshAllGLWidgets();
    void refreshViews();
    void screenshot                         ( const wxString &path, const wxString &filename );
    void onTreeChange();
    void onLoad                             ( wxCommandEvent& evt );
    void onLoadAsPeaks                      ( wxCommandEvent& evt );
    long getCurrentListIndex() const         { return m_currentListIndex; }
    void createNewAnatomy                   ( DatasetType dataType );
    void updateSliders();

    bool canDraw3D() const          { return m_draw3d; }
    bool canDrawRound() const       { return m_drawRound; }
    bool canUseColorPicker() const  { return m_canUseColorPicker; }
    bool isDrawerToolActive() const { return m_isDrawerToolActive; }

    wxColour  getDrawColor() const  { return m_drawColor; }
    wxImage & getDrawIcon()         { return m_drawColorIcon; }
    DrawMode  getDrawMode() const   { return m_drawMode; }
    int       getDrawSize() const   { return m_drawSize; }

    void      setDrawColor( const wxColour &color )   { m_drawColor = color; }
    void      setDrawSize( const int size ) { m_drawSize = size; }

    void      setThreadsActive( const int nb )      { m_threadsActive = nb; }
    SelectionObject * getLastSelectedObj() const    { return m_pLastSelectionObj; }


public:
    PropertiesWindow    *m_pPropertiesWindow;
    TrackingWindow      *m_pTrackingWindow;
    TrackingWindow      *m_pTrackingWindowHardi;
    MainCanvas          *m_pMainGL;
    MainCanvas          *m_pGL0;
    MainCanvas          *m_pGL1;
    MainCanvas          *m_pGL2;
    ListCtrl            *m_pListCtrl;
    MyTreeCtrl          *m_pTreeWidget;
    wxSlider            *m_pXSlider;
    wxSlider            *m_pYSlider;
    wxSlider            *m_pZSlider;
    wxTreeItemId        m_tSelectionObjectsId;
    wxNotebook          *m_tab;

private:
    void initLayout();

    // File menu
    void onNewAnatomyByte                   ( wxCommandEvent& evt );
    void onNewAnatomyRGB                    ( wxCommandEvent& evt );
    void onSave                             ( wxCommandEvent& evt );
    void onSaveFibers                       ( wxCommandEvent& evt );
    void onSaveSurface                      ( wxCommandEvent& evt );
    void onSaveDataset                      ( wxCommandEvent& evt );
    void onQuit                             ( wxCommandEvent& evt );
    void onClose                            ( wxCloseEvent&   evt );
    void onSize                             ( wxSizeEvent&    evt );
    // View menu
    void onMenuViewReset                    ( wxCommandEvent& evt );
    void onMenuViewLeft                     ( wxCommandEvent& evt );
    void onMenuViewRight                    ( wxCommandEvent& evt );
    void onMenuViewTop                      ( wxCommandEvent& evt );
    void onMenuViewBottom                   ( wxCommandEvent& evt );
    void onMenuViewFront                    ( wxCommandEvent& evt );
    void onMenuViewBack                     ( wxCommandEvent& evt );
    void onMenuViewCrosshair                ( wxCommandEvent& evt );
    void onMenuViewAxes                     ( wxCommandEvent& evt );
    void onMenuLock                         ( wxCommandEvent& evt );
    void onSceneLock                        ( wxCommandEvent& evt );
    void onRotateZ                          ( wxCommandEvent& evt );
    void onRotateY                          ( wxCommandEvent& evt );
    void onRotateX                          ( wxCommandEvent& evt );
    void onNavigateSagital                  ( wxCommandEvent& evt );
    void onNavigateAxial                    ( wxCommandEvent& evt );
    void onNavigateCoronal                  ( wxCommandEvent& evt );
    void onToggleShowAxial                  ( wxCommandEvent& evt );
    void onToggleShowCoronal                ( wxCommandEvent& evt );
    void onToggleShowSagittal               ( wxCommandEvent& evt );

    // Voi menu
    void onNewSelectionBox                  ( wxCommandEvent& evt );
    void onNewSelectionEllipsoid            ( wxCommandEvent& evt );
    void onHideSelectionObjects             ( wxCommandEvent& evt );
    void onActivateSelectionObjects         ( wxCommandEvent& evt );

    // Fibers menu
    void onInvertFibers                     ( wxCommandEvent& evt );
    void onUseFakeTubes                     ( wxCommandEvent& evt );
    void onResetColor                       ( wxCommandEvent& evt );
    void onUseTransparency                  ( wxCommandEvent& evt );
    void onUseGeometryShader                ( wxCommandEvent& evt );

    // Options menu
    void onToggleLighting                   ( wxCommandEvent& evt );
    void onClearToBlack                     ( wxCommandEvent& evt );
    void onSelectNormalPointer              ( wxCommandEvent& evt );
    void onSelectRuler                      ( wxCommandEvent& evt );
    void onRulerToolClear                   ( wxCommandEvent& evt );
    void onRulerToolAdd                     ( wxCommandEvent& evt );
    void onRulerToolDel                     ( wxCommandEvent& evt );
    void onToggleTextureFiltering           ( wxCommandEvent& evt );
    void onToggleBlendTexOnMesh             ( wxCommandEvent& evt );
    void onToggleFilterIso                  ( wxCommandEvent& evt );    
    void onSetCMap0                         ( wxCommandEvent& evt );
    void onSetCMap1                         ( wxCommandEvent& evt );
    void onSetCMap2                         ( wxCommandEvent& evt );
    void onSetCMap3                         ( wxCommandEvent& evt );
    void onSetCMap4                         ( wxCommandEvent& evt );
    void onSetCMap5                         ( wxCommandEvent& evt );
    void onSetCMapNo                        ( wxCommandEvent& evt );
    void onToggleNormal                     ( wxCommandEvent& evt );
    void onToggleDrawVectors                ( wxCommandEvent& evt );
    void onToggleAlpha                      ( wxCommandEvent& evt );
    void onToggleDrawPointsMode             ( wxCommandEvent& evt );
    // Help menu
    void onAbout                            ( wxCommandEvent& evt );
    void onShortcuts                        ( wxCommandEvent& evt );
    void onScreenshot                       ( wxCommandEvent& evt );
    void onWarningsInformations             ( wxCommandEvent& evt );

    // List widget event functions     
    void onActivateListItem                 ( wxListEvent&    evt );
    void onSelectListItem                   ( wxListEvent&    evt );

    void onDeleteAllListItems               ( wxListEvent&    evt );
    void onDeleteListItem                   ( wxListEvent&    evt );
    void onDeselectListItem                 ( wxListEvent&    evt );

    // Tree widget event functions
    void onDeleteTreeItem                   ( wxTreeEvent&    evt );
    void onSelectTreeItem                   ( wxTreeEvent&    evt );
    void onRightClickTreeItem               ( wxTreeEvent&    evt );
    void onUnSelectTreeItem                 ( wxTreeEvent&    evt );
    void onActivateTreeItem                 ( wxTreeEvent&    evt );
    void onTreeLabelEdit                    ( wxTreeEvent&    evt );
    int  treeSelected                       ( wxTreeItemId    id  ); 

    // System functions
    void onGLEvent                          ( wxCommandEvent& evt );    
    void onSliderMoved                      ( wxCommandEvent& evt );
    void onKdTreeThreadFinished             ( wxCommandEvent& evt );
    void updateStatusBar();
    void updateMenus();
    void onTimerEvent                       ( wxTimerEvent&   evt );
    void setTimerSpeed();
    void createNewSelectionObject( ObjectType i_newSelectionObjectType );

    void onSelectDrawer                     ( wxCommandEvent& evt );
    void onSwitchDrawer                     ( wxCommandEvent& evt );
    void onToggleDrawRound                  ( wxCommandEvent& evt );
    void onToggleDraw3d                     ( wxCommandEvent& evt );
    void onSelectColorPicker                ( wxCommandEvent& evt );
    void onSelectStroke1                    ( wxCommandEvent& evt );
    void onSelectStroke2                    ( wxCommandEvent& evt );
    void onSelectStroke3                    ( wxCommandEvent& evt );
    void onSelectStroke4                    ( wxCommandEvent& evt );
    void onSelectStroke5                    ( wxCommandEvent& evt );
    void onSelectStroke7                    ( wxCommandEvent& evt );
    void onSelectStroke10                   ( wxCommandEvent& evt );
    void onSelectPen                        ( wxCommandEvent& evt );
    void onSelectEraser                     ( wxCommandEvent& evt );

    // Utility
    void updateDrawerToolbar();

    void changePropertiesSizer( SceneObject * pSceneObj, int index );

private:
    ToolBar             *m_pToolBar;
    MenuBar             *m_pMenuBar;       
    wxSizer             *m_pCurrentSizer;
    SceneObject         *m_pCurrentSceneObject;
//     SceneObject         *m_pLastSelectedSceneObject;
    long                m_currentListIndex;
//     long                m_lastSelectedListItem;

    wxString            m_lastPath;

    wxTimer             *m_pTimer;

    bool     m_isDrawerToolActive;
    DrawMode m_drawMode;
    int      m_drawSize;
    bool     m_drawRound;
    bool     m_draw3d;
    bool     m_canUseColorPicker;
    wxColor  m_drawColor;
    wxImage  m_drawColorIcon;

    int  m_threadsActive;
    SelectionObject* m_pLastSelectionObj;

DECLARE_EVENT_TABLE()
};

/*
 * Defines for interface items and other events
 */
#define KDTREE_EVENT                                270

#define ID_GL_NAV_X                                 271
#define ID_GL_NAV_Y                                 272
#define ID_GL_NAV_Z                                 273
#define ID_GL_MAIN                                  290

//#define ID_LIST_CTRL                                291
#define ID_TREE_CTRL                                300

#define ID_X_SLIDER                                 301
#define ID_Y_SLIDER                                 302
#define ID_Z_SLIDER                                 303

#endif /*MAINFRAME_H_*/
