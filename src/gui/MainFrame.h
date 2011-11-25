/////////////////////////////////////////////////////////////////////////////
// Description: mainFrame class. Contains every elements of the GUI, and frame events
/////////////////////////////////////////////////////////////////////////////

#ifndef MAINFRAME_H_
#define MAINFRAME_H_

#include <wx/checkbox.h>
#include <wx/grid.h>
#include <wx/treectrl.h>

#include "MainCanvas.h"
#include "MyListCtrl.h"

#include "../misc/Algorithms/Helper.h"

class DatasetHelper;
class SelectionObject;
class DatasetInfo;
class ToolBar;
class MenuBar;
class SceneObject;

class MainFrame : public wxFrame
{
    friend class ToolBar;
    friend class MenuBar;
    friend class PropertiesWindow;

public:
    MainFrame( wxWindow *i_parent, const wxWindowID i_id, const wxString &i_title, const wxPoint &i_pos, const wxSize &i_size, const long i_style);
    ~MainFrame();

    void displayPropertiesSheet();
    void deleteSceneObject();
    void deleteListItem();
    void deleteTreeItem();
    void refreshAllGLWidgets();
    void refreshViews();
    void renewAllGLWidgets();
    void screenshot                         ( wxString      i_fileName    );
    void onTreeChange();
    void onMouseEvent                       ( wxMouseEvent&   event );
    void onLoad                             ( wxCommandEvent& event );
	long getCurrentListItem();
    
private:
    // File menu
    void onReloadShaders                    ( wxCommandEvent& event );
    void onSave                             ( wxCommandEvent& event );
    void onSaveFibers                       ( wxCommandEvent& event );
    void onSaveSurface                      ( wxCommandEvent& event );
    void onSaveDataset                      ( wxCommandEvent& event );
    void onQuit                             ( wxCommandEvent& event );
    void onClose                            ( wxCloseEvent&   event );
    // View menu
    void onMenuViewReset                    ( wxCommandEvent& event );
    void onMenuViewLeft                     ( wxCommandEvent& event );
    void onMenuViewRight                    ( wxCommandEvent& event );
    void onMenuViewTop                      ( wxCommandEvent& event );
    void onMenuViewBottom                   ( wxCommandEvent& event );
    void onMenuViewFront                    ( wxCommandEvent& event );
    void onMenuViewBack                     ( wxCommandEvent& event );
    void onMenuViewCrosshair                ( wxCommandEvent& event );
    void onMenuViewAxes                     ( wxCommandEvent& event );
    void onMenuLock                         ( wxCommandEvent& event );
    void onSceneLock                        ( wxCommandEvent& event );
    void onRotateZ                          ( wxCommandEvent& event );
    void onRotateY                          ( wxCommandEvent& event );
    void onRotateX                          ( wxCommandEvent& event );
    void onNavigateSagital                  ( wxCommandEvent& event );
    void onNavigateAxial                    ( wxCommandEvent& event );
    void onNavigateCoronal                  ( wxCommandEvent& event );
    void onToggleShowAxial                  ( wxCommandEvent& event );
    void onToggleShowCoronal                ( wxCommandEvent& event );
    void onToggleShowSagittal               ( wxCommandEvent& event );

    // Voi menu
    void onToggleSelectionObjects           ( wxCommandEvent& event );
    void onNewSelectionBox                  ( wxCommandEvent& event );
    void onNewSelectionEllipsoid            ( wxCommandEvent& event );
    void onHideSelectionObjects             ( wxCommandEvent& event );
    void onActivateSelectionObjects         ( wxCommandEvent& event );
    void onUseMorph                         ( wxCommandEvent& event );    
    // Fibers menu
    void onInvertFibers                     ( wxCommandEvent& event );
    void onUseFakeTubes                     ( wxCommandEvent& event );
    void onResetColor                       ( wxCommandEvent& event );
    void onUseTransparency                  ( wxCommandEvent& event );
    // surface menu
    void onNewSplineSurface                 ( wxCommandEvent& event );
    void onMoveBoundaryPointsLeft           ( wxCommandEvent& event );
    void onMoveBoundaryPointsRight          ( wxCommandEvent& event );
    void moveBoundaryPoints( int i_value);
    // Options menu
    void onToggleLighting                   ( wxCommandEvent& event );
    void onClearToBlack                     ( wxCommandEvent& event );
    void onRulerTool                        ( wxCommandEvent& event );
    void onRulerToolClear                   ( wxCommandEvent& event );
    void onRulerToolAdd                     ( wxCommandEvent& event );
    void onRulerToolDel                     ( wxCommandEvent& event );
    void onToggleTextureFiltering           ( wxCommandEvent& event );
    void onToggleBlendTexOnMesh             ( wxCommandEvent& event );
    void onToggleFilterIso                  ( wxCommandEvent& event );
    void onToggleColorMapLegend             ( wxCommandEvent& event );    
    void onSetCMap0                         ( wxCommandEvent& event );
    void onSetCMap1                         ( wxCommandEvent& event );
    void onSetCMap2                         ( wxCommandEvent& event );
    void onSetCMap3                         ( wxCommandEvent& event );
    void onSetCMap4                         ( wxCommandEvent& event );
    void onSetCMap5                         ( wxCommandEvent& event );
    void onSetCMapNo                        ( wxCommandEvent& event );
    void onToggleNormal                     ( wxCommandEvent& event );
    void onToggleDrawVectors                ( wxCommandEvent& event );
    void onToggleAlpha                      ( wxCommandEvent& event );
    void onToggleDrawPointsMode             ( wxCommandEvent& event );
    // Help menu
    void onAbout                            ( wxCommandEvent& event );
    void onShortcuts                        ( wxCommandEvent& event );
    void onScreenshot                       ( wxCommandEvent& event );
	void onWarningsInformations				( wxCommandEvent& event );
    void onSlizeMovieSag                    ( wxCommandEvent& event );
    void onSlizeMovieCor                    ( wxCommandEvent& event );
    void onSlizeMovieAxi                    ( wxCommandEvent& event );

    // List widget event functions     
    void onActivateListItem                 ( wxListEvent&    event );
    void onSelectListItem                   ( wxListEvent&    event );
    void onListMenuName                     ( wxCommandEvent& event );

    // Tree widget event functions
    void onDeleteTreeItem                   ( wxTreeEvent&    event );
    void onSelectTreeItem                   ( wxTreeEvent&    event );
    void onRightClickTreeItem               ( wxTreeEvent&    event );
    void onUnSelectTreeItem                 ( wxTreeEvent&    event );
    void onActivateTreeItem                 ( wxTreeEvent&    event );
    void onTreeLabelEdit                    ( wxTreeEvent&    event );
    int  treeSelected                       ( wxTreeItemId    i_id  ); 
    
    // System functions
    void onSize                             ( wxSizeEvent&    event );
    void doOnSize();
    void onGLEvent                          ( wxCommandEvent& event );    
    void onSliderMoved                      ( wxCommandEvent& event );
    void onKdTreeThreadFinished             ( wxCommandEvent& event );
    void updateStatusBar();
    void updateMenus();
    void onTimerEvent                       ( wxTimerEvent&   event );
    void setTimerSpeed();
    void createNewSelectionObject( ObjectType i_newSelectionObjectType );    
    void onLoadDatasets                     ( wxCommandEvent& event );
    void onLoadMeshes                       ( wxCommandEvent& event );
    void onLoadFibers                       ( wxCommandEvent& event );
    void onLoadTensors                      ( wxCommandEvent& event );
    void onLoadODFs                         ( wxCommandEvent& event );    
    bool loadIndex                          ( int i_index );
    
    
private:
    ToolBar             *m_pToolBar;
    MenuBar             *m_pMenuBar;       
    wxBoxSizer          *m_pCurrentSizer;
    SceneObject         *m_pCurrentSceneObject;
    SceneObject         *m_pLastSelectedSceneObject;
    long                m_currentListItem;
    long                m_lastSelectedListItem;

    wxBoxSizer          *m_pMainSizer;
    wxBoxSizer          *m_pListSizer;
    wxBoxSizer          *m_pObjectSizer;
    wxBoxSizer          *m_pLeftMainSizer;
    wxBoxSizer          *m_pNavSizer;

    wxTimer             *m_pTimer;

public:
    PropertiesWindow    *m_pPropertiesWindow;
    MainCanvas          *m_pMainGL;
    MainCanvas          *m_pGL0;
    MainCanvas          *m_pGL1;
    MainCanvas          *m_pGL2;
    MyListCtrl          *m_pListCtrl;
    MyTreeCtrl          *m_pTreeWidget;
    wxSlider            *m_pXSlider;
    wxSlider            *m_pYSlider;
    wxSlider            *m_pZSlider;
    DatasetHelper       *m_pDatasetHelper;
    wxTreeItemId        m_tRootId;
    wxTreeItemId        m_tPointId;
    wxTreeItemId        m_tSelectionObjectsId;

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

#define ID_LIST_CTRL                                291
#define ID_TREE_CTRL                                300

#define ID_X_SLIDER                                 301
#define ID_Y_SLIDER                                 302
#define ID_Z_SLIDER                                 303
 
#define TREE_CTRL_TOGGLE_ANDNOT                     351
#define TREE_CTRL_DELETE_BOX                        352
#define TREE_CTRL_TOGGLE_BOX_ACTIVE                 353
#define TREE_CTRL_TOGGLE_BOX_SHOW                   370

#endif /*MAINFRAME_H_*/
