#include "wx/wx.h"
#include "wx/laywin.h"

#include "myChild.h"
#include "myCanvas.h"

#include "mainFrame.h"

int winNumber = 1;

BEGIN_EVENT_TABLE(MainFrame, wxMDIParentFrame)
    EVT_MENU(SASHTEST_ABOUT, MainFrame::OnAbout)
    EVT_MENU(SASHTEST_NEW_WINDOW, MainFrame::OnNewWindow)
    EVT_SIZE(MainFrame::OnSize)
    EVT_MENU(SASHTEST_QUIT, MainFrame::OnQuit)
    EVT_MENU(SASHTEST_TOGGLE_WINDOW, MainFrame::OnToggleWindow)
    EVT_SASH_DRAGGED_RANGE(ID_WINDOW_TOP, ID_WINDOW_BOTTOM, MainFrame::OnSashDrag)
END_EVENT_TABLE()


// Define my frame constructor
MainFrame::MainFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size,
    const long style):
  wxMDIParentFrame(parent, id, title, pos, size, style)
{
  // Create some dummy layout windows

  // A window like a toolbar
  wxSashLayoutWindow* win =
      new wxSashLayoutWindow(this, ID_WINDOW_TOP,
                             wxDefaultPosition, wxSize(200, 30),
                             wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);

  win->SetDefaultSize(wxSize(1000, 30));
  win->SetOrientation(wxLAYOUT_HORIZONTAL);
  win->SetAlignment(wxLAYOUT_TOP);
  win->SetBackgroundColour(wxColour(255, 0, 0));
  win->SetSashVisible(wxSASH_BOTTOM, true);

  m_topWindow = win;

  // A window like a statusbar
  win = new wxSashLayoutWindow(this, ID_WINDOW_BOTTOM,
                               wxDefaultPosition, wxSize(200, 30),
                               wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
  win->SetDefaultSize(wxSize(1000, 30));
  win->SetOrientation(wxLAYOUT_HORIZONTAL);
  win->SetAlignment(wxLAYOUT_BOTTOM);
  win->SetBackgroundColour(wxColour(0, 0, 255));
  win->SetSashVisible(wxSASH_TOP, true);

  m_bottomWindow = win;

  // A window to the left of the client window
  win = new wxSashLayoutWindow(this, ID_WINDOW_LEFT1,
                               wxDefaultPosition, wxSize(200, 30),
                               wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
  win->SetDefaultSize(wxSize(120, 1000));
  win->SetOrientation(wxLAYOUT_VERTICAL);
  win->SetAlignment(wxLAYOUT_LEFT);
  win->SetBackgroundColour(wxColour(0, 255, 0));
  win->SetSashVisible(wxSASH_RIGHT, true);
  win->SetExtraBorderSize(10);

  wxTextCtrl* textWindow = new wxTextCtrl(win, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxTE_MULTILINE|wxSUNKEN_BORDER);
//        wxTE_MULTILINE|wxNO_BORDER);
  textWindow->SetValue(_T("A help window"));

  m_leftWindow1 = win;

  // Another window to the left of the client window
  win = new wxSashLayoutWindow(this, ID_WINDOW_LEFT2,
                               wxDefaultPosition, wxSize(200, 30),
                               wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
  win->SetDefaultSize(wxSize(120, 1000));
  win->SetOrientation(wxLAYOUT_VERTICAL);
  win->SetAlignment(wxLAYOUT_LEFT);
  win->SetBackgroundColour(wxColour(0, 255, 255));
  win->SetSashVisible(wxSASH_RIGHT, true);

  m_leftWindow2 = win;
}

void MainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
      Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
      (void)wxMessageBox(_T("wxWidgets 2.0 Sash Demo\nAuthor: Julian Smart (c) 1998"), _T("About Sash Demo"));
}

void MainFrame::OnToggleWindow(wxCommandEvent& WXUNUSED(event))
{
    if (m_leftWindow1->IsShown())
    {
        m_leftWindow1->Show(false);
    }
    else
    {
        m_leftWindow1->Show(true);
    }
#if wxUSE_MDI_ARCHITECTURE
    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
#endif // wxUSE_MDI_ARCHITECTURE
}

void MainFrame::OnSashDrag(wxSashEvent& event)
{
    if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
        return;

    switch (event.GetId())
    {
        case ID_WINDOW_TOP:
        {
            m_topWindow->SetDefaultSize(wxSize(1000, event.GetDragRect().height));
            break;
        }
        case ID_WINDOW_LEFT1:
        {
            m_leftWindow1->SetDefaultSize(wxSize(event.GetDragRect().width, 1000));
            break;
        }
        case ID_WINDOW_LEFT2:
        {
            m_leftWindow2->SetDefaultSize(wxSize(event.GetDragRect().width, 1000));
            break;
        }
        case ID_WINDOW_BOTTOM:
        {
            m_bottomWindow->SetDefaultSize(wxSize(1000, event.GetDragRect().height));
            break;
        }
    }

#if wxUSE_MDI_ARCHITECTURE
    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
#endif // wxUSE_MDI_ARCHITECTURE

    // Leaves bits of itself behind sometimes
    GetClientWindow()->Refresh();
}

void MainFrame::OnNewWindow(wxCommandEvent& WXUNUSED(event))
{
      // Make another frame, containing a canvas
      MyChild *subframe = new MyChild(this, _T("Canvas Frame"),
                                      wxPoint(10, 10), wxSize(300, 300),
                                      wxDEFAULT_FRAME_STYLE |
                                      wxNO_FULL_REPAINT_ON_RESIZE);

      subframe->SetTitle(wxString::Format(_T("Canvas Frame %d"), winNumber));
      winNumber ++;

      // Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
      subframe->SetIcon(wxIcon(_T("sashtest_icn")));
#endif

#if wxUSE_STATUSBAR
      // Give it a status line
      subframe->CreateStatusBar();
#endif // wxUSE_STATUSBAR

      // Make a menubar
      wxMenu *file_menu = new wxMenu;

      file_menu->Append(SASHTEST_NEW_WINDOW, _T("&New window"));
      file_menu->Append(SASHTEST_CHILD_QUIT, _T("&Close child"));
      file_menu->Append(SASHTEST_QUIT, _T("&Exit"));

      wxMenu *option_menu = new wxMenu;

      // Dummy option
      option_menu->Append(SASHTEST_REFRESH, _T("&Refresh picture"));

      wxMenu *help_menu = new wxMenu;
      help_menu->Append(SASHTEST_ABOUT, _T("&About"));

      wxMenuBar *menu_bar = new wxMenuBar;

      menu_bar->Append(file_menu, _T("&File"));
      menu_bar->Append(option_menu, _T("&Options"));
      menu_bar->Append(help_menu, _T("&Help"));

      // Associate the menu bar with the frame
      subframe->SetMenuBar(menu_bar);

      int width, height;
      subframe->GetClientSize(&width, &height);
      MyCanvas *canvas = new MyCanvas(subframe, wxPoint(0, 0), wxSize(width, height));
      canvas->SetCursor(wxCursor(wxCURSOR_PENCIL));
      subframe->canvas = canvas;

      // Give it scrollbars
      canvas->SetScrollbars(20, 20, 50, 50);

      subframe->Show(true);
}

void MainFrame::OnSize(wxSizeEvent& WXUNUSED(event))
{
#if wxUSE_MDI_ARCHITECTURE
    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
#endif // wxUSE_MDI_ARCHITECTURE
}

