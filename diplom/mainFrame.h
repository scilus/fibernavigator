#ifndef MAINFRAME_H_
#define MAINFRAME_H_

#include "theScene.h"
#include "navigationCanvas.h"
#include "mainCanvas.h"
#include "theDataset.h"


// Define a new frame
class MainFrame: public wxMDIParentFrame
{
	
private:
	TheDataset *m_dataset;
	TheScene *m_scene;
	
public:
    MainFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long style);

    void loadStandard();

private:
	void OnSize(wxSizeEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnNewWindow(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    
	void OnLoad(wxCommandEvent& event);
    void OnLoadData1(wxCommandEvent& event);
    void OnLoadDataRGB(wxCommandEvent& event);
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
    void OnToggleOverlay(wxCommandEvent& event);
    void OnToggleRGB(wxCommandEvent& event);
    void OnReloadShaders(wxCommandEvent& event);
    
    void refreshAllGLWidgets();
    void updateInfoString();
	
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

    NavigationCanvas* m_gl0;
    NavigationCanvas* m_gl1;
    NavigationCanvas* m_gl2;
    MainCanvas* m_mainGL;
    
    wxTextCtrl* m_textWindow;
    wxPanel* m_panel1;
    wxStaticBitmap* m_statBitmap1;
    
    int NAV_SIZE;
    int NAV_GL_SIZE;

DECLARE_EVENT_TABLE()
};

#define VIEWER_QUIT        	wxID_EXIT
#define VIEWER_ABOUT       	wxID_ABOUT
#define VIEWER_LOAD				4
#define VIEWER_LOAD_DATA1		5
#define VIEWER_LOAD_DATARGB		6
#define VIEWER_TOGGLEVIEW1 		11
#define VIEWER_TOGGLEVIEW2 		12
#define VIEWER_TOGGLEVIEW3 		13
#define VIEWER_TOGGLE_OVERLAY	14
#define VIEWER_TOGGLE_RGB		15
#define VIEWER_RELOAD_SHADER	16


#define ID_WINDOW_LEFT    		100
#define ID_WINDOW_RIGHT  		101
#define ID_WINDOW_NAV_MAIN 		102
#define ID_WINDOW_EXTRA_RIGHT  	103
#define ID_WINDOW_RIGHT_HOLDER  104

#define ID_WINDOW_NAV_X 	105
#define ID_WINDOW_NAV_Y  	106
#define ID_WINDOW_NAV_Z   	107
#define ID_WINDOW_NAV3  	108

#define ID_GL_NAV_X 	110
#define ID_GL_NAV_Y  	111
#define ID_GL_NAV_Z   	112
#define ID_GL_MAIN		113

#define ID_X_SLIDER 115
#define ID_Y_SLIDER 116
#define ID_Z_SLIDER 117
#define ID_T_SLIDER 118

#endif /*MAINFRAME_H_*/
