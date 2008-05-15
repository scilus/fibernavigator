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

#include "fileopen.xpm"

#include "main.h"
#include "mainFrame.h"



MainFrame *frame = NULL;

IMPLEMENT_APP(MyApp)

// Initialise this in OnInit, not statically
bool MyApp::OnInit(void)
{
  // Create the main frame window

  frame = new MainFrame(NULL, wxID_ANY, _T("Viewer"), wxPoint(0, 0), wxSize(1200, 820),
                      wxDEFAULT_FRAME_STYLE |
                      wxHSCROLL | wxVSCROLL);

  // Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
  frame->SetIcon(wxIcon(_T("sashtest_icn")));
#endif

  frame->SetMinSize(wxSize(800,600));
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
  
  wxToolBar* toolBar = new wxToolBar( frame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL|wxNO_BORDER);
  wxBitmap bmpOpen (fileopen_xpm);
  toolBar->AddTool(VIEWER_LOAD, bmpOpen, wxT("Open"));
  toolBar->Realize();
  frame->SetToolBar(toolBar);
  
  frame->Show(true);
 
  SetTopWindow(frame);

  return true;
}


