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

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/mdi.h"
#include "wx/laywin.h"

#include "icons/fileopen.xpm"
#include "icons/view1.xpm"
#include "icons/view2.xpm"
#include "icons/view3.xpm"
#include "icons/toggleOverlay.xpm"
#include "icons/toggleRGB.xpm"
#include "icons/mini_cat.xpm"

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
  file_menu->Append(VIEWER_LOAD_DATA1, _T("&Load Overlay"));
  file_menu->Append(VIEWER_LOAD_DATARGB, _T("&Load Data RGB"));
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
  wxBitmap bmpView1 (view1_xpm);
  wxBitmap bmpView2 (view2_xpm);
  wxBitmap bmpView3 (view3_xpm);
  wxBitmap bmpToggleOverlay (toggle_overlay_xpm);
  wxBitmap bmpToggleRGB (toggle_rgb_xpm);
  wxBitmap bmpMiniCat (mini_cat_xpm);
  toolBar->AddTool(VIEWER_LOAD, bmpOpen, wxT("Open"));
  toolBar->AddSeparator();
  toolBar->AddTool(VIEWER_TOGGLEVIEW1, bmpView1, wxT("Toggle View 1"));
  toolBar->AddTool(VIEWER_TOGGLEVIEW2, bmpView2, wxT("Toggle View 2"));
  toolBar->AddTool(VIEWER_TOGGLEVIEW3, bmpView3, wxT("Toggle View 3"));
  toolBar->AddSeparator();
  toolBar->AddTool(VIEWER_TOGGLE_OVERLAY, bmpToggleOverlay, wxT("Toggle Overlay"));
  toolBar->AddTool(VIEWER_TOGGLE_RGB, bmpToggleRGB, wxT("Toggle RGB"));
  toolBar->AddSeparator();
  toolBar->AddTool(VIEWER_RELOAD_SHADER, bmpMiniCat, wxT("Reload Shaders"));
  toolBar->Realize();
  frame->SetToolBar(toolBar);
  
  frame->Show(true);
 
  SetTopWindow(frame);
  
  frame->loadStandard();

  return true;
}


