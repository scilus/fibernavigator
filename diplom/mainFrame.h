#ifndef MAINFRAME_H_
#define MAINFRAME_H_

#include "navigationCanvas.h"
#include "theDataset.h"

// Define a new frame
class MainFrame: public wxMDIParentFrame
{
	
private:
	TheDataset *dataset;

	
public:

    MainFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long style);

    void OnSize(wxSizeEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnNewWindow(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnLoad(wxCommandEvent& event);
    void OnToggleWindow(wxCommandEvent& event);
    void OnSashDrag(wxSashEvent& event);

public:
    wxSashLayoutWindow* m_leftWindow;
    wxSashLayoutWindow* m_mainWindow;
    wxSashLayoutWindow* m_navWindow;
    wxSashLayoutWindow* m_rightWindow;
    wxSashLayoutWindow* m_navWindow1;
    wxSashLayoutWindow* m_navWindow2;
    
    wxSashLayoutWindow* m_topNavWindow;
    wxSashLayoutWindow* m_middleNavWindow;
    wxSashLayoutWindow* m_bottomNavWindow;
    wxSashLayoutWindow* m_rightWindow2;
    
    
    NavigationCanvas* m_gl1;
    NavigationCanvas* m_gl2;
    NavigationCanvas* m_gl3;

    
    wxTextCtrl* m_textWindow;
    wxPanel* m_panel1;
    wxStaticBitmap* m_statBitmap1;

DECLARE_EVENT_TABLE()
};

#define VIEWER_QUIT        wxID_EXIT
#define VIEWER_NEW_WINDOW  2
#define VIEWER_REFRESH     3
#define VIEWER_ABOUT       wxID_ABOUT
#define VIEWER_LOAD		4

#define ID_WINDOW_LEFT    	100
#define ID_WINDOW_RIGHT  	101
#define ID_WINDOW_LEFT1   	102
#define ID_WINDOW_RIGHT1  	103

#define ID_WINDOW_LEFT_TOP   		104
#define ID_WINDOW_LEFT_MIDDLE  	105
#define ID_WINDOW_LEFT_BOTTOM   	106
#define ID_WINDOW_RIGHT_BOTTOM  	107


#endif /*MAINFRAME_H_*/
