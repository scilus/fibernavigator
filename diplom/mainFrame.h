#ifndef MAINFRAME_H_
#define MAINFRAME_H_

// Define a new frame
class MainFrame: public wxMDIParentFrame
{
  public:

    MainFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long style);

    void OnSize(wxSizeEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnNewWindow(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnToggleWindow(wxCommandEvent& event);
    void OnSashDrag(wxSashEvent& event);

protected:
    wxSashLayoutWindow* m_topWindow;
    wxSashLayoutWindow* m_leftWindow1;
    wxSashLayoutWindow* m_leftWindow2;
    wxSashLayoutWindow* m_bottomWindow;

DECLARE_EVENT_TABLE()
};

#define SASHTEST_QUIT        wxID_EXIT
#define SASHTEST_NEW_WINDOW  2
#define SASHTEST_REFRESH     3
#define SASHTEST_ABOUT       wxID_ABOUT
#define SASHTEST_TOGGLE_WINDOW 6

#define ID_WINDOW_TOP       100
#define ID_WINDOW_LEFT1     101
#define ID_WINDOW_LEFT2     102
#define ID_WINDOW_BOTTOM    103


#endif /*MAINFRAME_H_*/