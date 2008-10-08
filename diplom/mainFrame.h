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
	void OnSize(wxSizeEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnNewWindow(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);

	void OnLoad(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnNew(wxCommandEvent& event);
    void OnToggleWindow(wxCommandEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    void OnGLEvent(wxCommandEvent &event);

	void OnXSliderMoved(wxCommandEvent& event);
    void OnYSliderMoved(wxCommandEvent& event);
    void OnZSliderMoved(wxCommandEvent& event);
    void OnTSliderMoved(wxCommandEvent& event);

    void OnButtonAxial(wxCommandEvent& event);
    void OnButtonCoronal(wxCommandEvent& event);
    void OnButtonSagittal(wxCommandEvent& event);
    void OnToggleAlpha(wxCommandEvent& event);
    void OnToggleSelBox(wxCommandEvent& event);
    void OnNewSelBox(wxCommandEvent& event);
    void OnHideSelBoxes(wxCommandEvent& event);

    void OnTogglePointMode(wxCommandEvent& event);
    void OnNewSurface(wxCommandEvent& event);
    void OnNewSurface2(wxCommandEvent& event);

    void OnReloadShaders(wxCommandEvent& event);

    void OnActivateListItem(wxListEvent& event);
    void OnSelectListItem(wxListEvent& event);
    void OnListItemUp(wxCommandEvent& event);
    void OnListItemDown(wxCommandEvent& event);

    void OnSelectTreeItem(wxTreeEvent& event);
    void OnUnSelectTreeItem(wxTreeEvent& event);
    void OnActivateTreeItem(wxTreeEvent& event);
    void OnTreeEvent(wxCommandEvent& event);
    void OnTreeLabelEdit(wxTreeEvent& event);

    void OnAssignColor(wxCommandEvent& event);
    void OnToggleLighting(wxCommandEvent& event);

	void OnInvertFibers(wxCommandEvent& event);
	void OnCutAnatomy(wxCommandEvent& event);

	void OnMovePoints1(wxCommandEvent& event);
	void OnMovePoints2(wxCommandEvent& event);

	void OnToggleTextureFiltering(wxCommandEvent& event);

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

    MainCanvas* m_gl0;
    MainCanvas* m_gl1;
    MainCanvas* m_gl2;

    int NAV_SIZE;
    int NAV_GL_SIZE;

public:
	MainCanvas* m_mainGL;

	wxSlider* m_xSlider;
    wxSlider* m_ySlider;
    wxSlider* m_zSlider;
    wxSlider* m_tSlider;

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

#define VIEWER_QUIT        	wxID_EXIT
#define VIEWER_ABOUT       	wxID_ABOUT
#define VIEWER_NEW					4
#define VIEWER_LOAD					5
#define VIEWER_SAVE					6
#define VIEWER_BUTTON_AXIAL 		11
#define VIEWER_BUTTON_CORONAL 		12
#define VIEWER_BUTTON_SAGITTAL 		13
#define VIEWER_TOGGLE_ALPHA 		14
#define VIEWER_RELOAD_SHADER		16
#define VIEWER_NEW_SELBOX			17
#define VIEWER_RENDER_SELBOXES		19
#define VIEWER_TOGGLE_SELBOX		20
#define VIEWER_DRAW_POINTS			21
#define VIEWER_NEW_SURFACE			22
#define VIEWER_NEW_SURFACE2			23
#define VIEWER_ASSIGN_COLOR			24
#define VIEWER_TOGGLE_LIGHTING		25
#define VIEWER_INVERT_FIBERS		26
#define VIEWER_CUT_ANATOMY			27
#define VIEWER_MOVE_POINTS1 		28
#define VIEWER_MOVE_POINTS2 		29
#define VIEWER_TOGGLE_TEXTURE_FILTERING		30

#define VIEWER_VIEW_LEFT		200
#define VIEWER_VIEW_RIGHT		201
#define VIEWER_VIEW_FRONT		202
#define VIEWER_VIEW_BACK		203
#define VIEWER_VIEW_TOP			204
#define VIEWER_VIEW_DOWN		205



#define KDTREE_EVENT			50

#define ID_GL_NAV_X 	120
#define ID_GL_NAV_Y  	121
#define ID_GL_NAV_Z   	122
#define ID_GL_MAIN		123

#define LIST_CTRL		130
#define TREE_CTRL		131

#define ID_X_SLIDER 201
#define ID_Y_SLIDER 202
#define ID_Z_SLIDER 203
#define ID_T_SLIDER 204

#define ID_BUTTON_UP 	211
#define ID_BUTTON_DOWN 212

#endif /*MAINFRAME_H_*/
