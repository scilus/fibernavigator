#ifndef MAINFRAME_H_
#define MAINFRAME_H_

#include "theScene.h"
#include "mainCanvas.h"
#include "theDataset.h"
#include "myListCtrl.h"
#include "wx/treectrl.h"
#include "point.h"

// Define a new frame
class MainFrame: public wxMDIParentFrame
{
public:
    MainFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long style);

    void setTSlider(wxSlider *slider) {m_tSlider = slider;};
    void setMStatusBar(wxStatusBar *bar) {m_statusBar = bar;};
    void loadStandard();
    TheScene* getScene() {return m_scene;};

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

    void OnToggleView1(wxCommandEvent& event);
    void OnToggleView2(wxCommandEvent& event);
    void OnToggleView3(wxCommandEvent& event);
    void OnToggleAlpha(wxCommandEvent& event);
    void OnToggleSelBox(wxCommandEvent& event);
    void OnNewSelBox(wxCommandEvent& event);
    void OnHideSelBoxes(wxCommandEvent& event);

    void OnTogglePointMode(wxCommandEvent& event);
    void OnNewSurface(wxCommandEvent& event);

    void OnReloadShaders(wxCommandEvent& event);

    void OnActivateListItem(wxListEvent& event);
    void OnSelectListItem(wxListEvent& event);
    void OnListItemUp(wxCommandEvent& event);
    void OnListItemDown(wxCommandEvent& event);

    void OnSelectTreeItem(wxTreeEvent& event);
    void OnUnSelectTreeItem(wxTreeEvent& event);
    void OnActivateTreeItem(wxTreeEvent& event);
    void OnTreeEvent(wxCommandEvent& event);

    void load(wxString);
    void load();
    void load(int);
    void updateTreeDims();
    void updateTreeDS(int);
    void refreshAllGLWidgets();
    void renewAllGLWidgets();
    void updateStatusBar();

    TheScene *m_scene;

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

    wxSlider* m_xSlider;
    wxSlider* m_ySlider;
    wxSlider* m_zSlider;
    wxSlider* m_tSlider;

    MyListCtrl* m_datasetListCtrl;

    wxStatusBar* m_statusBar;

    MainCanvas* m_gl0;
    MainCanvas* m_gl1;
    MainCanvas* m_gl2;
    MainCanvas* m_mainGL;

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

    int NAV_SIZE;
    int NAV_GL_SIZE;

    Point* m_lastSelectedPoint;

DECLARE_EVENT_TABLE()
};

#define VIEWER_QUIT        	wxID_EXIT
#define VIEWER_ABOUT       	wxID_ABOUT
#define VIEWER_NEW					4
#define VIEWER_LOAD					5
#define VIEWER_SAVE					6
#define VIEWER_TOGGLE_AXIAL 		11
#define VIEWER_TOGGLE_CORONAL 		12
#define VIEWER_TOGGLE_SAGITTAL 		13
#define VIEWER_TOGGLE_ALPHA 		14
#define VIEWER_RELOAD_SHADER		16
#define VIEWER_NEW_SELBOX			17
#define VIEWER_RENDER_SELBOXES		19
#define VIEWER_TOGGLE_SELBOX		20
#define VIEWER_DRAW_POINTS			21
#define VIEWER_NEW_SURFACE			22

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
