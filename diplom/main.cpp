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
#include "wx/filefn.h"
#include "wx/cmdline.h"
#include "wx/filename.h"

#include "icons/fileopen.xpm"
#include "icons/disc.xpm"
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
#include "icons/toggleSurface.xpm"
#include "icons/box.xpm"
#include "icons/box_off.xpm"
#include "icons/box_eye.xpm"
#include "icons/grid.xpm"
#include "icons/grid_spline.xpm"
#include "icons/view1.xpm"
#include "icons/view3.xpm"
#include "icons/colorSelect.xpm"
#include "icons/lightbulb.xpm"
#include "icons/iso_surface.xpm"
#include "icons/mini_cat.xpm"

#include "main.h"
#include "mainFrame.h"

wxString MyApp::respath;
wxString MyApp::shaderPath;

MainFrame *frame = NULL;

const wxString MyApp::APP_NAME = _T( "main" );
const wxString MyApp::APP_VENDOR = _T( "Ralph S. & Mario H." );

IMPLEMENT_APP(MyApp)

static const wxCmdLineEntryDesc desc[] = {
	{wxCMD_LINE_SWITCH, _T("h"), _T("help"), _T("help yourself")},
	{wxCMD_LINE_PARAM, NULL, NULL, _T("scene file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL|wxCMD_LINE_PARAM_MULTIPLE},
	{wxCMD_LINE_NONE}
};

namespace {
  void printwx( const wxString& str )
  {
	char* cstring;
	cstring = (char*) malloc(str.length());
	strcpy(cstring, (const char*) str.mb_str(wxConvUTF8));
	printf("%s\n", cstring);
    free( cstring );
  }

  void printwx( const wxString& str1, const wxString& str2 )
  {
    printwx( str1 ); printwx( str2 );
  }
}

// Initialise this in OnInit, not statically
bool MyApp::OnInit(void) {

	SetAppName(APP_NAME);
	SetVendorName(APP_VENDOR);

	respath = wxFindAppPath(argv[0], wxGetCwd(), _T("FNPATH"), _T("fn"));
#ifdef __WXMSW__
	if (respath.Last() != '\\') respath += '\\';
	shaderPath += _T("GLSL\\");
#elif __WXMAC__
#if 1
    // if we use the above code to get the same on OSX, I get a segfault somewhere
    // therefore I use the OSX native code here:

	// OSX only: Try to find the resource path...
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFURLRef resourcesURL = CFBundleCopyBundleURL( mainBundle );
	CFStringRef str = CFURLCopyFileSystemPath( resourcesURL, kCFURLPOSIXPathStyle );
	CFRelease( resourcesURL );
	char path[ PATH_MAX ];

	CFStringGetCString( str, path, FILENAME_MAX, kCFStringEncodingASCII );
	CFRelease( str );
	fprintf( stderr, path );

	respath = wxString::FromAscii( path );
#endif
    respath += _T( "/Contents/Resources/" );
    shaderPath = respath + _T( "GLSL/" );
#else
	if (respath.Last() != '/') respath += '/';
	shaderPath =  respath + _T("GLSL/");
#endif
#ifdef DEBUG
#ifndef __WXMSW__
	// this crashes on windows
    //printwx( _T( "respath:" ), respath );
    //printwx( _T( "shader:" ), shaderPath );
#endif
#endif
#ifdef __WXMAC__
	//wxImage::AddHandler( new wxJPEGHandler() );
	wxImage::AddHandler( new wxPNGHandler() );
	//wxImage::AddHandler( new wxGIFHandler() );
	//
#endif

	// Create the main frame window
	frame = new MainFrame(NULL, wxID_ANY, _T("Fiber Navigator"), wxPoint(0, 0), wxSize(1200, 820), wxDEFAULT_FRAME_STYLE
			| wxHSCROLL | wxVSCROLL);

	// Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
	frame->SetIcon(wxIcon(_T("sashtest_icn")));
#endif

	frame->SetMinSize(wxSize(800, 600));
	// Make a menubar
	wxMenu *file_menu = new wxMenu;
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
	voi_menu->AppendCheckItem(MENU_VOI_TOGGLE_SELBOX, _T("active"));
	voi_menu->AppendCheckItem(MENU_VOI_TOGGLE_SHOWBOX, _T("visible"));

	wxMenu *surf_menu = new wxMenu;
	surf_menu->Append(MENU_SPLINESURF_NEW, _T("New Spline Surface"));
	surf_menu->Append(MENU_FILE_NEW_ISOSURF, _T("New Iso Surface"));
	surf_menu->AppendSeparator();
	surf_menu->AppendCheckItem(MENU_SPLINESURF_TOGGLE_LIC, _T("Toggle Lic"));
	surf_menu->AppendCheckItem(MENU_SPLINESURF_TOGGLE_NORMAL, _T("Toggle Normal Direction"));
	surf_menu->AppendCheckItem(MENU_SPLINESURF_DRAW_VECTORS, _T("Draw Vectors"));
	surf_menu->AppendSeparator();
	surf_menu->AppendCheckItem(MENU_OPTIONS_TOGGLE_TEXTURE_FILTERING, _T("Toggle Texture Mode"));
	surf_menu->AppendCheckItem(MENU_OPTIONS_BLEND_TEX_ON_MESH, _T("Blend Texture on Mesh"));
	surf_menu->AppendCheckItem(MENU_OPTIONS_FILTER_ISO, _T("Filter Dataset for IsoSurface"));
	surf_menu->Append(MENU_OPTIONS_CLEAN, _T("Clean Artefacts from Surface"));
	surf_menu->Append(MENU_OPTIONS_LOOP, _T("Smooth Surface (Loop SubD)"));


	wxMenu *options_menu = new wxMenu;
	options_menu->Append(MENU_OPTIONS_ASSIGN_COLOR, _T("Assign Color"));
	options_menu->Append(MENU_OPTIONS_RESET_COLOR, _T("Reset Colors on Fibers"));

	wxMenu* cMaps = new wxMenu;
	cMaps->Append(MENU_OPTIONS_CMAP0, wxT("default color map"));
	cMaps->Append(MENU_OPTIONS_CMAP1, wxT("color map 1"));
	cMaps->Append(MENU_OPTIONS_CMAP2, wxT("color map 2"));
	cMaps->Append(MENU_OPTIONS_CMAP3, wxT("color map 3"));
	cMaps->Append(MENU_OPTIONS_CMAP4, wxT("color map 4"));

	options_menu->Append(MENU_OPTIONS_COLOR_MAPS, _T("Set Color Map"), cMaps);
	options_menu->AppendCheckItem(MENU_OPTIONS_CMAP_LEGEND, _T("Show Color Map Legend"));
	options_menu->AppendSeparator();
	options_menu->AppendCheckItem(MENU_OPTIONS_TOGGLE_LIGHTING, _T("Toggle Fiber Lighting"));
	options_menu->AppendCheckItem(MENU_OPTIONS_INVERT_FIBERS, _T("Invert Fiber Selection"));
	options_menu->AppendCheckItem(MENU_OPTIONS_USE_FAKE_TUBES, _T("Use Tubes"));




	wxMenu *help_menu = new wxMenu;
	help_menu->Append(MENU_HELP_ABOUT, _T("About"));

	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(file_menu, _T("&File"));
	menu_bar->Append(view_menu, _T("&View"));
	menu_bar->Append(voi_menu, _T("&VOI"));
	menu_bar->Append(surf_menu, _T("&Surfaces"));
	menu_bar->Append(options_menu, _T("&Options"));
	menu_bar->Append(help_menu, _T("&Help"));

	// Associate the menu bar with the frame
	frame->SetMenuBar(menu_bar);
	frame->setMMenuBar(menu_bar);

	wxToolBar *toolBar =
					new wxToolBar(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL
							| wxNO_BORDER);
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
	wxBitmap bmpView3(view3_xpm);
	wxBitmap bmpMiniCat(mini_cat_xpm);

	wxBitmap bmpNewSurface(toggle_surface_xpm);
	wxBitmap bmpAssignColor(colorSelect_xpm);
	wxBitmap bmpLighting(lightbulb_xpm);
#else
	wxBitmap bmpOpen (wxImage(respath+_T("icons/fileopen.png" ), wxBITMAP_TYPE_PNG));
	wxBitmap bmpSave (wxImage(respath+_T("icons/disc.png" ), wxBITMAP_TYPE_PNG));
	wxBitmap bmpAxial (wxImage(respath+_T("icons/axial.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpCor (wxImage(respath+_T("icons/cor.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpSag (wxImage(respath+_T("icons/sag.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpBox (wxImage(respath+_T("icons/box.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpBoxOff(wxImage(respath+_T("icons/box_off.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpBoxEye(wxImage(respath+_T("icons/box_eye.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpGrid (wxImage(respath+_T("icons/grid.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpGridSpline(wxImage(respath+_T("icons/grid_spline.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpIsoSurface(wxImage(respath+_T("icons/iso_surface.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpView1 (wxImage(respath+_T("icons/view1.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpView2 (wxImage(respath+_T("icons/view2.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpView3 (wxImage(respath+_T("icons/view3.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpMiniCat(wxImage(respath+_T("icons/mini_cat.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpNew (wxImage(respath+_T("icons/new.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpQuit (wxImage(respath+_T("icons/quit.png"), wxBITMAP_TYPE_PNG));

	wxBitmap bmpHideSelbox(wxImage(respath+_T("icons/toggleselbox.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpNewSurface(wxImage(respath+_T("icons/toggleSurface.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpAssignColor(wxImage(respath+_T("icons/colorSelect.png"), wxBITMAP_TYPE_PNG));
	wxBitmap bmpLighting(wxImage(respath+_T("icons/lightbulb.png"), wxBITMAP_TYPE_PNG));

#endif

	//toolBar->AddTool(VIEWER_NEW, bmpNew, wxT("New"));
	toolBar->AddTool(MENU_FILE_LOAD, bmpOpen, wxT("Open"));
	toolBar->AddTool(MENU_FILE_SAVE, bmpSave, wxT("Save Scene"));
	//toolBar->AddTool(VIEWER_QUIT, bmpQuit, wxT("Quit"));
	toolBar->AddSeparator();
	toolBar->AddCheckTool(BUTTON_AXIAL, wxT("Axial"), bmpAxial);
	toolBar->AddCheckTool(BUTTON_CORONAL, wxT("Coronal"), bmpCor);
	toolBar->AddCheckTool(BUTTON_SAGITTAL, wxT("Sagittal"), bmpSag);
	toolBar->AddCheckTool(BUTTON_TOGGLE_ALPHA, wxT("Toggle alpha blending"), bmpNewSurface);
	toolBar->AddSeparator();
	toolBar->AddTool(MENU_VOI_NEW_SELBOX, bmpBox, wxT("New Selection Box"));
	toolBar->AddCheckTool(MENU_VOI_RENDER_SELBOXES, wxT("Toggle Selection Boxes"), bmpBoxEye);
	toolBar->AddCheckTool(MENU_VOI_TOGGLE_SELBOX, wxT("Toggle activation status of selection box"), bmpBoxOff);
	toolBar->AddSeparator();
	toolBar->AddTool(MENU_SPLINESURF_NEW, bmpGridSpline, wxT("New Spline Surface"));
	toolBar->AddTool(MENU_SPLINESURF_DRAW_POINTS, bmpGrid, wxT("Toggle drawing of points"));
	toolBar->AddTool(BUTTON_MOVE_POINTS1, bmpView1, wxT("Move border points of spline surface"));
	toolBar->AddTool(BUTTON_MOVE_POINTS2, bmpView3, wxT("Move border points of spline surface"));
	toolBar->AddSeparator();
	toolBar->AddTool(MENU_OPTIONS_ASSIGN_COLOR, bmpAssignColor, wxT("Assign Color"));
	toolBar->AddSeparator();
	toolBar->AddCheckTool(MENU_OPTIONS_TOGGLE_LIGHTING, wxT("Toggle Lighting"), bmpLighting);
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
	frame->setMToolBar(toolBar);

	wxStatusBar* statusBar = new wxStatusBar(frame, wxID_ANY, wxST_SIZEGRIP);
	frame->SetStatusBar(statusBar);
	int widths[] = { 250, 150, -1 };
	statusBar->SetFieldsCount(WXSIZEOF(widths), widths);
	frame->setMStatusBar(statusBar);

	frame->Show(true);

	SetTopWindow(frame);

	wxString cmdFileName;
	wxCmdLineParser cmdParser(desc, argc, argv);
	cmdParser.Parse(false);

	if (cmdParser.GetParamCount() > 0)
	{
		for (size_t i = 0 ; i < cmdParser.GetParamCount() ; ++i)
		{
			cmdFileName = cmdParser.GetParam(i);
			wxFileName fName(cmdFileName);
			fName.Normalize(wxPATH_NORM_LONG|wxPATH_NORM_DOTS|wxPATH_NORM_TILDE|wxPATH_NORM_ABSOLUTE);
			cmdFileName = fName.GetFullPath();
			frame->m_dh->load(-1, cmdFileName);
		}
	}

	return true;
}

// Find the absolute path where this application has been run from.
// argv0 is wxTheApp->argv[0]
// cwd is the current working directory (at startup)
// appVariableName is the name of a variable containing the directory for this app, e.g.
// MYAPPDIR. This is checked first.
wxString MyApp::wxFindAppPath(const wxString& argv0, const wxString& cwd,
		const wxString& appVariableName, const wxString& appName) {
	wxString str;

#ifndef __WXWINCE__
	// Try appVariableName
	if (!appVariableName.IsEmpty()) {
		str = wxGetenv(appVariableName);
		if (!str.IsEmpty())
			return str;
	}
#endif

#if defined(__WXMAC__) && !defined(__DARWIN__)
	return cwd;
#endif

	if (wxIsAbsolutePath(argv0))
		return wxPathOnly(argv0);
	else {
		// Is it a relative path?
		if (!cwd.IsEmpty()) {
			wxString currentDir(cwd);
			if (currentDir.Last() != wxFILE_SEP_PATH)
				currentDir += wxFILE_SEP_PATH;

			str = currentDir + argv0;
			if (wxFileExists(str))
				return wxPathOnly(str);
#ifdef __WXMAC__
			// The current directory may be above the actual
			// bundle. So if we find the bundle below it,
			// we know where we are.
			if (!appName.IsEmpty())
			{
				wxString p = currentDir + appName + wxT(".app");
				if (wxDirExists(p))
				{
					p += wxFILE_SEP_PATH;
					p += wxT("Content/MacOS");
					return p;
				}
			}
#endif
		}
	}

	// OK, it's neither an absolute path nor a relative path.
	// Search PATH.

	wxPathList pathList;
	pathList.AddEnvList(wxT("PATH"));
	str = pathList.FindAbsoluteValidPath(argv0);
	if (!str.IsEmpty())
		return wxPathOnly(str);

	// Failed
	return wxEmptyString;
}
