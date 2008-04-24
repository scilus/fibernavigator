/////////////////////////////////////////////////////////////////////////////
// Name:        main.cpp
// Purpose:     main window
// Author:      Ralph Schurade
// Modified by:
// Created:     03/27/08
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#include "wx/wx.h"
#include "wx/mdi.h"
#include "wx/laywin.h"

#include "main.h"
#include "mainFrame.h"

MainFrame *frame = NULL;

IMPLEMENT_APP(MyApp)

// Initialise this in OnInit, not statically
bool MyApp::OnInit(void)
{
  // Create the main frame window

  frame = new MainFrame(NULL, wxID_ANY, _T("Viewer"), wxPoint(0, 0), wxSize(800, 600),
                      wxDEFAULT_FRAME_STYLE |
                      wxNO_FULL_REPAINT_ON_RESIZE |
                      wxHSCROLL | wxVSCROLL);

  // Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
  frame->SetIcon(wxIcon(_T("sashtest_icn")));
#endif

  // Make a menubar
  wxMenu *file_menu = new wxMenu;
  file_menu->Append(VIEWER_LOAD, _T("&Load"));
  file_menu->Append(VIEWER_QUIT, _T("&Exit"));

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(VIEWER_ABOUT, _T("&About"));

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(file_menu, _T("&File"));
  menu_bar->Append(help_menu, _T("&Help"));

  // Associate the menu bar with the frame
  frame->SetMenuBar(menu_bar);
    
  frame->CreateStatusBar();

  frame->m_gl1->Init();
  
  frame->Show(true);
  
  wxImage::AddHandler(new wxPNGHandler);
  
  SetTopWindow(frame);

  return true;
}


