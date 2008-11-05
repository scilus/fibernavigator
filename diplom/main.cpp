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

#include "icons/fileopen.xpm"
#include "icons/disc.xpm"
#include "icons/view1.xpm"
#include "icons/view2.xpm"
#include "icons/view3.xpm"
#ifdef __WXMSW__
// icons 16x16
#include "icons/axialw.xpm"
#include "icons/corw.xpm"
#include "icons/sagw.xpm"
#else
//icons 25x25
#include "icons/axial.xpm"
#include "icons/cor.xpm"
#include "icons/sag.xpm"
#endif
#include "icons/box.xpm"
#include "icons/box_off.xpm"
#include "icons/box_eye.xpm"

#include "icons/grid.xpm"
#include "icons/grid_spline.xpm"

#include "icons/iso_surface.xpm"

#include "icons/mini_cat.xpm"
#include "icons/new.xpm"
#include "icons/lightbulb.xpm"
#include "icons/quit.xpm"
#include "icons/toggleselbox.xpm"
#include "icons/toggleSurface.xpm"
#include "icons/colorSelect.xpm"

#include "main.h"
#include "mainFrame.h"

MainFrame *frame = NULL;

const wxString MyApp::APP_NAME = _T(  "main" );
const wxString MyApp::APP_VENDOR = _T(  "Ralph S. & Mario H." );

IMPLEMENT_APP(MyApp)

// Initialise this in OnInit, not statically
bool MyApp::OnInit(void) {

    SetAppName (  APP_NAME );
    SetVendorName (  APP_VENDOR );

#ifdef __WXMAC__
    //wxImage::AddHandler( new wxJPEGHandler() );
    wxImage::AddHandler( new wxPNGHandler() );
    //wxImage::AddHandler( new wxGIFHandler() );
#endif

	// Create the main frame window
	frame = new MainFrame(NULL, wxID_ANY, _T("Viewer"), wxPoint(0, 0), wxSize(1200, 820), wxDEFAULT_FRAME_STYLE
			| wxHSCROLL | wxVSCROLL);


	// Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
	frame->SetIcon(wxIcon(_T("sashtest_icn")));
#endif

	frame->SetMinSize(wxSize(800, 600));
	// Make a menubar
	wxMenu *file_menu = new wxMenu;
	file_menu->Append(MENU_FILE_NEW_ISOSURF, _T("New IsoSurface"));
	file_menu->AppendSeparator();
	file_menu->Append(MENU_FILE_LOAD, _T("Load"));
	file_menu->Append(MENU_FILE_RELOAD_SHADER, _T("Reload Shader"));
	file_menu->AppendSeparator();
	file_menu->Append(MENU_FILE_SAVE, _T("Save Current Scene"));
	file_menu->Append(MENU_FILE_SAVE_FIBERS, _T("Save Selected Fibers"));
	file_menu->AppendSeparator();
	file_menu->Append(MENU_FILE_QUIT, _T("Exit"));

	wxMenu *view_menu = new wxMenu;
	view_menu->Append(MENU_VIEW_RESET, _T("reset"));
	view_menu->AppendSeparator();
	view_menu->Append(MENU_VIEW_LEFT, _T("left"));
	view_menu->Append(MENU_VIEW_RIGHT, _T("right"));
	view_menu->Append(MENU_VIEW_FRONT, _T("posterior"));
	view_menu->Append(MENU_VIEW_BACK, _T("anterior"));
	view_menu->Append(MENU_VIEW_TOP, _T("superior"));
	view_menu->Append(MENU_VIEW_BOTTOM, _T("inferior"));

	wxMenu *voi_menu = new wxMenu;
	voi_menu->Append(MENU_VOI_NEW_SELBOX, _T("New"));
	voi_menu->AppendSeparator();
	voi_menu->Append(MENU_VOI_TOGGLE_SELBOX, _T("toggle activation"));
	voi_menu->Append(MENU_VOI_TOGGLE_SHOWBOX, _T("toggle visibility"));

	wxMenu *surf_menu = new wxMenu;
	surf_menu->Append(MENU_SPLINESURF_NEW, _T("New"));
	surf_menu->AppendSeparator();
	surf_menu->AppendCheckItem(MENU_SPLINESURF_TOGGLE_LIC, _T("Toggle Lic"));
	surf_menu->AppendCheckItem(MENU_SPLINESURF_TOGGLE_NORMAL, _T("Toggle Normal Direction"));

	wxMenu *options_menu = new wxMenu;
	options_menu->Append(MENU_OPTIONS_ASSIGN_COLOR, _T("Assign Color"));
	options_menu->AppendSeparator();
	options_menu->AppendCheckItem(MENU_OPTIONS_TOGGLE_LIGHTING, _T("Toggle Fiber Lighting"));
	options_menu->AppendCheckItem(MENU_OPTIONS_INVERT_FIBERS, _T("Invert Fiber Selection"));
	options_menu->AppendCheckItem(MENU_OPTIONS_USE_FAKE_TUBES, _T("Use Fake Tubes"));
	options_menu->AppendSeparator();
	options_menu->AppendCheckItem(MENU_OPTIONS_TOGGLE_TEXTURE_FILTERING, _T("Toggle Texture Mode"));
	options_menu->AppendCheckItem(MENU_OPTIONS_BLEND_TEX_ON_MESH, _T("Blend Texture on Mesh"));

	wxMenu *help_menu = new wxMenu;
	help_menu->Append(MENU_HELP_ABOUT, _T("About"));

	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(file_menu, _T("&File"));
	menu_bar->Append(view_menu, _T("&View"));
	menu_bar->Append(voi_menu, _T("&VOI"));
	menu_bar->Append(surf_menu, _T("&Spline Surface"));
	menu_bar->Append(options_menu, _T("&Options"));
	menu_bar->Append(help_menu, _T("&Help"));

	// Associate the menu bar with the frame
	frame->SetMenuBar(menu_bar);

	wxToolBar* toolBar = new wxToolBar(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxNO_BORDER);
#ifndef __WXMAC__
	wxBitmap bmpOpen(fileopen_xpm);
	wxBitmap bmpSave(disc_xpm);
	wxBitmap bmpAxial(axial_xpm);
	wxBitmap bmpCor(cor_xpm);
	wxBitmap bmpSag(sag_xpm);
	wxBitmap bmpBox(box_xpm);
	wxBitmap bmpBoxOff(box_off_xpm);
	wxBitmap bmpBoxEye(box_eye_xpm);
	wxBitmap bmpGrid(grid_xpm);
	wxBitmap bmpGridSpline(grid_spline_xpm);
	wxBitmap bmpIsoSurface(iso_surface_xpm);
	wxBitmap bmpView1(view1_xpm);
	wxBitmap bmpView2(view2_xpm);
	wxBitmap bmpView3(view3_xpm);
	wxBitmap bmpMiniCat(mini_cat_xpm);
	wxBitmap bmpNew(new_xpm);
	wxBitmap bmpQuit(quit_xpm);

    wxBitmap bmpHideSelbox(toggleselbox_xpm);
	wxBitmap bmpNewSurface(toggle_surface_xpm);
	wxBitmap bmpAssignColor(colorSelect_xpm);
	wxBitmap bmpLighting(lightbulb_xpm);
#else

	wxBitmap bmpOpen  (wxImage(_T("icons/fileopen.png" ), wxBITMAP_TYPE_PNG));
	wxBitmap bmpSave  (wxImage(_T("icons/disc.png" ), wxBITMAP_TYPE_PNG));
	wxBitmap bmpAxial (wxImage(_T("icons/axial.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpCor   (wxImage(_T("icons/cor.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpSag   (wxImage(_T("icons/sag.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpBox   (wxImage(_T("icons/box.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpBoxOff(wxImage(_T("icons/box_off.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpBoxEye(wxImage(_T("icons/box_eye.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpGrid  (wxImage(_T("icons/grid.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpGridSpline(wxImage(_T("icons/grid_spline.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpIsoSurface(wxImage(_T("icons/iso_surface.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpView1 (wxImage(_T("icons/view1.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpView2 (wxImage(_T("icons/view2.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpView3 (wxImage(_T("icons/view3.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpMiniCat(wxImage(_T("icons/mini_cat.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpNew   (wxImage(_T("icons/new.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpQuit  (wxImage(_T("icons/quit.png"), wxBITMAP_TYPE_PNG));

    wxBitmap bmpHideSelbox(wxImage(_T("icons/toggleselbox.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpNewSurface(wxImage(_T("icons/toggleSurface.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpAssignColor(wxImage(_T("icons/colorSelect.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpLighting(wxImage(_T("icons/lightbulb.png"), wxBITMAP_TYPE_PNG));

#endif

	//toolBar->AddTool(VIEWER_NEW, bmpNew, wxT("New"));
	toolBar->AddTool(MENU_FILE_LOAD, bmpOpen, wxT("Open"));
	toolBar->AddTool(MENU_FILE_SAVE, bmpSave, wxT("Save Scene"));
	//toolBar->AddTool(VIEWER_QUIT, bmpQuit, wxT("Quit"));
	toolBar->AddSeparator();
	toolBar->AddTool(BUTTON_AXIAL, bmpAxial, wxT("Axial"));
	toolBar->AddTool(BUTTON_CORONAL, bmpCor, wxT("Coronal"));
	toolBar->AddTool(BUTTON_SAGITTAL, bmpSag, wxT("Sagittal"));
	toolBar->AddTool(BUTTON_TOGGLE_ALPHA, bmpNewSurface, wxT("Toggle alpha blending"));
	toolBar->AddSeparator();
	toolBar->AddTool(MENU_VOI_NEW_SELBOX, bmpBox, wxT("New Selection Box"));
	toolBar->AddTool(MENU_VOI_RENDER_SELBOXES, bmpBoxEye, wxT("Toggle Selection Boxes"));
	toolBar->AddTool(MENU_VOI_TOGGLE_SELBOX, bmpBoxOff, wxT("Toggle activation status of selection box"));
	toolBar->AddSeparator();
	toolBar->AddTool(MENU_SPLINESURF_NEW, bmpGridSpline, wxT("New Spline Surface"));
	toolBar->AddTool(MENU_SPLINESURF_DRAW_POINTS, bmpGrid, wxT("Toggle drawing of points"));
	toolBar->AddTool(BUTTON_MOVE_POINTS1, bmpView1, wxT("Move border points of spline surface"));
	toolBar->AddTool(BUTTON_MOVE_POINTS2, bmpView3, wxT("Move border points of spline surface"));
	toolBar->AddSeparator();
	toolBar->AddTool(MENU_OPTIONS_ASSIGN_COLOR, bmpAssignColor, wxT("Assign Color"));
	toolBar->AddSeparator();
	toolBar->AddTool(MENU_OPTIONS_TOGGLE_LIGHTING, bmpLighting, wxT("Toggle Lighting"));
	toolBar->AddSeparator();
	toolBar->AddTool(MENU_FILE_NEW_ISOSURF, bmpIsoSurface, wxT("New Iso Surface "));
	toolBar->AddSeparator();
#ifdef DEBUG
	toolBar->AddTool(MENU_OPTIONS_INVERT_FIBERS, bmpMiniCat, wxT("Invert Fibers"));
	toolBar->AddSeparator();
	toolBar->AddTool(MENU_FILE_RELOAD_SHADER, bmpMiniCat, wxT("Reload Shaders"));
	toolBar->AddSeparator();
#endif
	toolBar->Realize();
	frame->SetToolBar(toolBar);

	wxStatusBar* statusBar = new wxStatusBar(frame, wxID_ANY, wxST_SIZEGRIP);
	frame->SetStatusBar(statusBar);
	int widths[] = { 250, 150, -1 };
	statusBar->SetFieldsCount(WXSIZEOF(widths), widths);
	frame->setMStatusBar(statusBar);

	frame->Show(true);

	SetTopWindow(frame);

	return true;
}

