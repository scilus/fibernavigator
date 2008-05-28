#ifndef MAINFRAME_H_
#define MAINFRAME_H_

#include "theScene.h"
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

    void setTSlider(wxSlider *slider) {m_tSlider = slider;};
    void setMStatusBar(wxStatusBar *bar) {m_statusBar = bar;};
    void loadStandard();

private:
	void OnSize(wxSizeEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnNewWindow(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    
	void OnLoad(wxCommandEvent& event);
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
    void OnReloadShaders(wxCommandEvent& event);
    
    void refreshAllGLWidgets();
    void updateInfoString();
    void updateStatusBar();
	
    wxSashLayoutWindow* m_leftWindowHolder;
    wxSashLayoutWindow* m_leftWindowTop;
    wxSashLayoutWindow* m_leftWindowBottom;
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
    
    wxStatusBar* m_statusBar;

    MainCanvas* m_gl0;
    MainCanvas* m_gl1;
    MainCanvas* m_gl2;
    MainCanvas* m_mainGL;
    
    wxTextCtrl* m_textWindow;
    wxPanel* m_panel1;
    
    int NAV_SIZE;
    int NAV_GL_SIZE;

DECLARE_EVENT_TABLE()
};

#define VIEWER_QUIT        	wxID_EXIT
#define VIEWER_ABOUT       	wxID_ABOUT
#define VIEWER_LOAD				4
#define VIEWER_NEW					5
#define VIEWER_TOGGLEVIEW1 		11
#define VIEWER_TOGGLEVIEW2 		12
#define VIEWER_TOGGLEVIEW3 		13
#define VIEWER_RELOAD_SHADER		16

#define ID_GL_NAV_X 	120
#define ID_GL_NAV_Y  	121
#define ID_GL_NAV_Z   	122
#define ID_GL_MAIN		123

#define ID_X_SLIDER 201
#define ID_Y_SLIDER 202
#define ID_Z_SLIDER 203
#define ID_T_SLIDER 204

#endif /*MAINFRAME_H_*/
