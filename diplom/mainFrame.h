#ifndef MAINFRAME_H_
#define MAINFRAME_H_

#include "mainCanvas.h"
#include "myListCtrl.h"
#include "wx/treectrl.h"
#include "wx/laywin.h"
#include "DatasetHelper.h"

class DatasetHelper;
class MyListCtrl;

// Define a new frame
class MainFrame: public wxMDIParentFrame
{
public:
    MainFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long style);
    ~MainFrame();
    void setTSlider(wxSlider *slider) {m_tSlider = slider;};
    void setMStatusBar(wxStatusBar *bar) {m_statusBar = bar;};

    void refreshAllGLWidgets();
    void renewAllGLWidgets();

private:
	/*
	 * Menu Functions
	 */
	// File
	void OnNewIsoSurface(wxCommandEvent& event);
	void OnLoad(wxCommandEvent& event);
	void OnReloadShaders(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnSaveFibers(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	// View
	void OnMenuViewReset(wxCommandEvent& event);
	void OnMenuViewLeft(wxCommandEvent& event);
	void OnMenuViewRight(wxCommandEvent& event);
	void OnMenuViewTop(wxCommandEvent& event);
	void OnMenuViewBottom(wxCommandEvent& event);
	void OnMenuViewFront(wxCommandEvent& event);
	void OnMenuViewBack(wxCommandEvent& event);
	// Voi
    void OnToggleSelBox(wxCommandEvent& event);
    void OnToggleShowBox(wxCommandEvent& event);
    void OnNewSelBox(wxCommandEvent& event);
    void OnHideSelBoxes(wxCommandEvent& event);
	// Spline Surface
	void OnNewSurface(wxCommandEvent& event);
	void OnToggleNormal(wxCommandEvent& event);
	void OnTogglePointMode(wxCommandEvent& event);
	void OnToggleLIC(wxCommandEvent& event);
	// Options
	void OnAssignColor(wxCommandEvent& event);
    void OnToggleLighting(wxCommandEvent& event);
    void OnInvertFibers(wxCommandEvent& event);
    void OnUseFakeTubes(wxCommandEvent& event);
	void OnToggleTextureFiltering(wxCommandEvent& event);
	void OnToggleBlendTexOnMesh(wxCommandEvent& event);
	// Help
	void OnAbout(wxCommandEvent& event);

	/*
	 * Window Functions
	 */
	void OnSize(wxSizeEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    void OnGLEvent(wxCommandEvent &event);

	void OnXSliderMoved(wxCommandEvent& event);
    void OnYSliderMoved(wxCommandEvent& event);
    void OnZSliderMoved(wxCommandEvent& event);
    void OnTSliderMoved(wxCommandEvent& event);
    void OnTSlider2Moved(wxCommandEvent& event);

    /*
     * Button functions
     */
    void OnButtonAxial(wxCommandEvent& event);
    void OnButtonCoronal(wxCommandEvent& event);
    void OnButtonSagittal(wxCommandEvent& event);
    void OnToggleAlpha(wxCommandEvent& event);
    void OnMovePoints1(wxCommandEvent& event);
    void OnMovePoints2(wxCommandEvent& event);

    /*
     * List widget event functions
     */
    void OnActivateListItem(wxListEvent& event);
    void OnSelectListItem(wxListEvent& event);
    void OnListItemUp(wxCommandEvent& event);
    void OnListItemDown(wxCommandEvent& event);

    /*
     * Tree widget event functions
     */
    void OnSelectTreeItem(wxTreeEvent& event);
    void OnUnSelectTreeItem(wxTreeEvent& event);
    void OnActivateTreeItem(wxTreeEvent& event);
    void OnTreeEvent(wxCommandEvent& event);
    void OnTreeLabelEdit(wxTreeEvent& event);
    int treeSelected(wxTreeItemId id);

    /*
     * System functions
     */
    void OnKdTreeThreadFinished(wxCommandEvent& event);
    void updateStatusBar();

    wxSashLayoutWindow* m_leftWindowHolder;
    wxSashLayoutWindow* m_leftWindowTop;
    wxSashLayoutWindow* m_leftWindowBottom;
    wxSashLayoutWindow* m_leftWindowBottom1;
    wxSashLayoutWindow* m_leftWindowBottom2;
    wxSashLayoutWindow* m_leftWindow;
    wxSashLayoutWindow* m_mainWindow;
    wxSashLayoutWindow* m_navWindow;
    wxSashLayoutWindow* m_rightWindowHolder;
    wxSashLayoutWindow* m_rightWindow;
    wxSashLayoutWindow* m_extraRightWindow;
    wxSashLayoutWindow* m_topNavWindow;
    wxSashLayoutWindow* m_middleNavWindow;
    wxSashLayoutWindow* m_bottomNavWindow;
    wxSashLayoutWindow* m_extraNavWindow;

	wxSashLayoutWindow* m_xSliderHolder;
	wxSashLayoutWindow* m_ySliderHolder;
	wxSashLayoutWindow* m_zSliderHolder;

    int NAV_SIZE;
    int NAV_GL_SIZE;

public:
	MainCanvas* m_gl0;
	MainCanvas* m_gl1;
	MainCanvas* m_gl2;
	MainCanvas* m_mainGL;

	wxSlider* m_xSlider;
    wxSlider* m_ySlider;
    wxSlider* m_zSlider;
    wxSlider* m_tSlider;
    wxSlider* m_tSlider2;

	wxStatusBar* m_statusBar;
	MyListCtrl* m_listCtrl;
	wxTreeCtrl* m_treeWidget;
	wxTreeItemId m_tRootId;
	wxTreeItemId m_tPointId;
	wxTreeItemId m_tPlanesId;
	wxTreeItemId m_tAxialId;
	wxTreeItemId m_tCoronalId;
	wxTreeItemId m_tSagittalId;
	wxTreeItemId m_tDatasetId;
	wxTreeItemId m_tHeadId;
	wxTreeItemId m_tRGBId;
	wxTreeItemId m_tOverlayId;
	wxTreeItemId m_tMeshId;
	wxTreeItemId m_tFiberId;
	wxTreeItemId m_tSelBoxId;

	DatasetHelper* m_dh;

DECLARE_EVENT_TABLE()
};

/*
 * Defines for Menu Events
 */
//Menu File
#define MENU_FILE_NEW_ISOSURF						100
#define MENU_FILE_LOAD								101
#define MENU_FILE_RELOAD_SHADER						102
#define MENU_FILE_SAVE								103
#define MENU_FILE_SAVE_FIBERS						104
#define MENU_FILE_QUIT        						wxID_EXIT
// Menu View
#define MENU_VIEW_LEFT								120
#define MENU_VIEW_RIGHT								121
#define MENU_VIEW_FRONT								122
#define MENU_VIEW_BACK								123
#define MENU_VIEW_TOP								124
#define MENU_VIEW_BOTTOM							125
#define MENU_VIEW_RESET								126
// Menu Voi
#define MENU_VOI_NEW_SELBOX							130
#define MENU_VOI_RENDER_SELBOXES					131
#define MENU_VOI_TOGGLE_SELBOX						132
#define MENU_VOI_TOGGLE_SHOWBOX						133
// Menu Spline Surface
#define MENU_SPLINESURF_DRAW_POINTS					140
#define MENU_SPLINESURF_NEW							141
#define MENU_SPLINESURF_TOGGLE_LIC					142
#define MENU_SPLINESURF_TOGGLE_NORMAL				143
// Menu Options
#define MENU_OPTIONS_ASSIGN_COLOR					150
#define MENU_OPTIONS_TOGGLE_LIGHTING				151
#define MENU_OPTIONS_INVERT_FIBERS					152
#define MENU_OPTIONS_TOGGLE_TEXTURE_FILTERING		153
#define MENU_OPTIONS_BLEND_TEX_ON_MESH				154
#define MENU_OPTIONS_USE_FAKE_TUBES					155
// Menu Help
#define MENU_HELP_ABOUT       						wxID_ABOUT
/*
 * Defines for Buttons, will move into Menus eventually
 */
#define BUTTON_AXIAL						 		200
#define BUTTON_CORONAL 								201
#define BUTTON_SAGITTAL 							202
#define BUTTON_TOGGLE_ALPHA 						203
#define BUTTON_MOVE_POINTS1 						204
#define BUTTON_MOVE_POINTS2 						205

/*
 * Defines for interface items and other events
 */
#define KDTREE_EVENT				300

#define ID_GL_NAV_X 				310
#define ID_GL_NAV_Y  				311
#define ID_GL_NAV_Z   				312
#define ID_GL_MAIN					313

#define ID_LIST_CTRL				320
#define ID_TREE_CTRL				321

#define ID_X_SLIDER 				330
#define ID_Y_SLIDER 				331
#define ID_Z_SLIDER 				332
#define ID_T_SLIDER					333
#define ID_T_SLIDER2				334

#define ID_BUTTON_UP 				340
#define ID_BUTTON_DOWN 				341

#endif /*MAINFRAME_H_*/
