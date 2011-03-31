/////////////////////////////////////////////////////////////////////////////
// Name:        main.cpp
// Purpose:     main window
// Author:      Ralph Schurade
// Created:     03/27/08
/////////////////////////////////////////////////////////////////////////////


#ifndef WX_PRECOMP

#include "wx/wx.h"

#endif

#include "wx/wxprec.h"
#include "wx/mdi.h"
#include "wx/filefn.h"
#include "wx/cmdline.h"
#include "wx/filename.h"

#include "icons/fileopen.xpm"
#include "icons/disc.xpm"

#ifdef __WXMSW__

// Icons : 16x16
#include "icons/axialw.xpm"
#include "icons/corw.xpm"
#include "icons/sagw.xpm"

#else

// Icons : 25x25
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
#include "icons/tubes.xpm"
#include "icons/background_color.xpm"
#include "icons/mini_cat.xpm"
#include "icons/alphablend.xpm"
#include "icons/layout.xpm"
#include "icons/rulertool.xpm"

#include "main.h"
#include "gui/mainFrame.h"

#include <exception>

wxString MyApp::respath;
wxString MyApp::shaderPath;

MainFrame* frame = NULL;

const wxString MyApp::APP_NAME   = _T( "main" );
const wxString MyApp::APP_VENDOR = _T( "Ralph S. & Mario H." );

IMPLEMENT_APP(MyApp)

static const wxCmdLineEntryDesc desc[] =
{
    { wxCMD_LINE_SWITCH, _T("h"), _T("help"), _T("help yourself") },
    { wxCMD_LINE_SWITCH, _T("p"), _T("screenshot"), _T("screenshot") },
    { wxCMD_LINE_SWITCH, _T("d"), _T("dmap"), _T("create a distance map on the first loaded dataset") },
    { wxCMD_LINE_SWITCH, _T("e"), _T("exit"), _T("exit after executing the command line") },
    { wxCMD_LINE_PARAM, NULL, NULL, _T("scene file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },
    { wxCMD_LINE_NONE } 
};

namespace
{

/////////////////////////////////////////////////////////////////////////////

void printwx( const wxString& str )
{
    char* cstring = NULL;
    cstring = (char*) malloc( str.length() + 1 );
    strcpy( cstring, (const char*) str.mb_str( wxConvUTF8 ) );
    printf( "%s\n", cstring );
    free( cstring );
}

/////////////////////////////////////////////////////////////////////////////

void printwx( const wxString& str1, const wxString& str2 )
{
    printwx( str1 );
    printwx( str2 );
}

} // namespace

/////////////////////////////////////////////////////////////////////////////
// Initialise this in OnInit, not statically

bool MyApp::OnInit( void )
{

#ifdef __WXMSW__

    // UNCOMMENT FOR CONSOLE OUTPUT
    // AllocConsole();
    // freopen("CONOUT$", "wb", stdout);

#endif

    try
    {
        SetAppName( APP_NAME );
        SetVendorName( APP_VENDOR );

        respath = wxFindAppPath( argv[0], wxGetCwd(), _T("FNPATH"), _T("fn") );

#ifdef __WXMSW__

        if (respath.Last() != '\\') respath += '\\';
        shaderPath = respath + _T("GLSL\\");

#elif __WXMAC__

#if 1

        // If we use the above code to get the same on OSX, I get a segfault somewhere
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

        if ( respath.Last() != '/' )
            respath += '/';
        shaderPath = respath + _T("GLSL/");

#endif

#ifdef DEBUG

#ifndef __WXMSW__

        // this crashes on windows
        printwx( _T( "Warning: This version of Fibernavigator is debug compiled."), _T( "For better performance please compile a Release version.") );
        printwx( _T( "respath:" ), respath );
        printwx( _T( "shader:" ), shaderPath );

#endif

#endif

#ifdef __WXMAC__

        //wxImage::AddHandler( new wxJPEGHandler() );
        wxImage::AddHandler( new wxPNGHandler() );
        //wxImage::AddHandler( new wxGIFHandler() );

#endif

        // Create the main frame window
        frame = new MainFrame( NULL, wxID_ANY, _T("Neuro Planning Navigator"), wxPoint( 0, 0 ), wxSize( 1200, 820 ), wxDEFAULT_FRAME_STYLE );
        // Give it an icon (this is ignored in MDI mode: uses resources)

#ifdef __WXMSW__

        frame->SetIcon( wxIcon( _T( "sashtest_icn" ) ) );

#endif

        frame->SetMinSize( wxSize( 945, 730 ) );
		frame->SetSize( wxSize( 945, 730 ) );

        // Make a menubar
        wxMenu* file_menu = new wxMenu;
        file_menu->Append( MENU_FILE_LOAD,          _T( "Load\tCtrl-L" ) );
        file_menu->Append( MENU_FILE_RELOAD_SHADER, _T( "Reload Shader" ) );
        file_menu->AppendSeparator();
        file_menu->Append( MENU_FILE_SAVE,          _T( "Save Current Scene\tCtrl-S" ) );
        file_menu->Append( MENU_FILE_SAVE_FIBERS,   _T( "Save Selected Fibers" ) );
        file_menu->Append( MENU_FILE_SAVE_FIBERS_DMRI,   _T( "Save Selected Fibers DMRI FORMAT" ) );
        file_menu->Append( MENU_FILE_SAVE_SURFACE,  _T( "Save Selected Surface" ) );
        file_menu->AppendSeparator();
        file_menu->Append( MENU_FILE_QUIT,          _T( "Exit" ) );

        wxMenu* view_menu = new wxMenu;
        view_menu->Append( MENU_VIEW_RESET,         _T( "reset" ) );
        view_menu->AppendSeparator();
        view_menu->Append( MENU_VIEW_LEFT,          _T( "left\tL" ) );
        view_menu->Append( MENU_VIEW_RIGHT,         _T( "right\tR" ) );
        view_menu->Append( MENU_VIEW_TOP,           _T( "superior\tS" ) );
        view_menu->Append( MENU_VIEW_BOTTOM,        _T( "inferior\tI" ) );
        view_menu->Append( MENU_VIEW_BACK,          _T( "anterior\tA" ) );
        view_menu->Append( MENU_VIEW_FRONT,         _T( "posterior\tP" ) );
        view_menu->AppendSeparator();
        view_menu->AppendCheckItem( MENU_VIEW_SHOW_CROSSHAIR, _T( "show crosshair\tC" ) );

        wxMenu* selectionObjectMenu = new wxMenu;
        selectionObjectMenu->Append( MENU_VOI_NEW_SELECTION_OBJECT_BOX ,       _T( "Box" )       );
        selectionObjectMenu->Append( MENU_VOI_NEW_SELECTION_OBJECT_ELLIPSOID , _T( "Ellipsoid" ) );

        wxMenu* voi_menu = new wxMenu;
        voi_menu->Append( MENU_VOI_NEW_SELECTION_OBJECT, _T( "New Selection-Object" ), selectionObjectMenu );
        voi_menu->Append( MENU_VOI_NEW_FROM_OVERLAY,     _T( "New VOI from Overlay" ) );
        voi_menu->AppendSeparator();
        voi_menu->AppendCheckItem( MENU_VOI_USE_MORPH, _T( "morphing" ) );
        voi_menu->AppendSeparator();
        voi_menu->AppendCheckItem( MENU_VOI_TOGGLE_SELECTION_OBJECTS,        _T( "active\tCtrl-A" )  );
        voi_menu->AppendCheckItem( MENU_VOI_TOGGLE_VISIBLE_SELECTION_OBJECT, _T( "visible\tCtrl-V" ) );

        wxMenu *surf_menu = new wxMenu;
        surf_menu->Append( MENU_SPLINESURF_NEW,     _T( "New Spline Surface" ) );
        surf_menu->Append( MENU_FILE_NEW_ISOSURF,   _T( "New Iso Surface" )    );
        surf_menu->Append( MENU_SURFACE_NEW_OFFSET, _T( "New Distance Map + Surface" )   );
        surf_menu->Append( MENU_SURFACE_DIST_MAP,   _T( "New Distance Map"));
        surf_menu->AppendSeparator();
        surf_menu->AppendCheckItem( MENU_SPLINESURF_TOGGLE_LIC,    _T( "Toggle Lic" )              );
        surf_menu->Append( MENU_SPLINESURF_TOGGLE_NORMAL, _T( "Flip Normal Direction" ) );

#ifdef __DRAW_STREAMLINES__

        surf_menu->AppendCheckItem(MENU_SPLINESURF_DRAW_VECTORS, _T("Draw Vectors"));

#endif

        surf_menu->AppendSeparator();
        surf_menu->AppendCheckItem( MENU_OPTIONS_TOGGLE_TEXTURE_FILTERING, _T( "Toggle Texture Mode" )           );
        surf_menu->AppendCheckItem( MENU_OPTIONS_BLEND_TEX_ON_MESH,        _T( "Blend Texture on Mesh" )         );
        surf_menu->AppendCheckItem( MENU_OPTIONS_FILTER_ISO,               _T( "Filter Dataset for IsoSurface" ) );
        surf_menu->Append( MENU_OPTIONS_CLEAN, _T( "Clean Artefacts from Surface" ) );
        surf_menu->Append( MENU_OPTIONS_LOOP,  _T( "Smooth Surface (Loop SubD)" )   );

        wxMenu *options_menu = new wxMenu;
        options_menu->Append( MENU_OPTIONS_ASSIGN_COLOR, _T( "Assign Color\tCtrl-C" )           );
        options_menu->Append( MENU_OPTIONS_RESET_COLOR,  _T( "Reset Colors on Fibers\tCtrl-F" ) );

        wxMenu* cMaps = new wxMenu;
        cMaps->Append( MENU_OPTIONS_CMAPNO, wxT( "Gray" )              );
        cMaps->Append( MENU_OPTIONS_CMAP0,  wxT( "Blue-Green-Purple" ) );
        cMaps->Append( MENU_OPTIONS_CMAP1,  wxT( "Rainbow" )           );
        cMaps->Append( MENU_OPTIONS_CMAP2,  wxT( "Hotiron" )           );
        cMaps->Append( MENU_OPTIONS_CMAP3,  wxT( "Red-Yellow" )        );
        cMaps->Append( MENU_OPTIONS_CMAP4,  wxT( "Blue-Lightblue" )    );
		options_menu->Append( MENU_OPTIONS_COLOR_MAPS, _T( "Set Color Map" ), cMaps );

		wxMenu* sampling = new wxMenu;
        sampling->Append( MENU_OPTIONS_SAMPLING_OPTNO, wxT( "100%" )            );
        sampling->Append( MENU_OPTIONS_SAMPLING_OPT1,  wxT( "50%" )				);
        sampling->Append( MENU_OPTIONS_SAMPLING_OPT2,  wxT( "25%" )				);
        sampling->Append( MENU_OPTIONS_SAMPLING_OPT3,  wxT( "10%" )				);
        sampling->Append( MENU_OPTIONS_SAMPLING_OPT4,  wxT( "5%" )				);
        sampling->Append( MENU_OPTIONS_SAMPLING_OPT5,  wxT( "1%" )				);
        options_menu->Append( MENU_OPTIONS_SAMPLING, _T( "Fibers Subsampling" ), sampling );

        options_menu->AppendCheckItem( MENU_OPTIONS_CMAP_LEGEND, _T( "Show Color Map\tCtrl-M" ) );
        options_menu->AppendSeparator();
        options_menu->AppendCheckItem( MENU_OPTIONS_TOGGLE_LIGHTING,  _T( "Toggle Fiber Lighting\tCtrl-I" ) );
        options_menu->AppendCheckItem( MENU_OPTIONS_INVERT_FIBERS,    _T( "Invert Fiber Selection" )        );
        options_menu->AppendCheckItem( MENU_OPTIONS_USE_FAKE_TUBES,   _T( "Use Tubes\tCtrl-T" )             );
        options_menu->AppendCheckItem( MENU_OPTIONS_CLEAR_TO_BLACK,   _T( "Clear Background to Black" )     );
        options_menu->AppendCheckItem( MENU_OPTIONS_USE_TRANSPARENCY, _T( "Use Transparent Fibers" )        );

		options_menu->AppendSeparator();
		options_menu->AppendCheckItem( MENU_OPTIONS_RULER_TOOL,       _T( "Toggle Ruler Tool\tCtrl-R" )     );
		options_menu->Append( MENU_OPTIONS_RULER_TOOL_CLEAR,          _T( "Ruler Tool, Clear Points\tEnd" )     ); 
		options_menu->Append( MENU_OPTIONS_RULER_TOOL_ADD,            _T( "Ruler Tool, Add Point\tIns" )     ); 
		options_menu->Append( MENU_OPTIONS_RULER_TOOL_DEL,            _T( "Ruler Tool, Delete Point\tDel" )     ); 

        wxMenu* licMovs = new wxMenu;
        licMovs->Append( MENU_HELP_SLIZEMOVIESAG, wxT( "sagittal" ) );
        licMovs->Append( MENU_HELP_SLIZEMOVIECOR, wxT( "coronal" )  );
        licMovs->Append( MENU_HELP_SLIZEMOVIEAXI, wxT( "axial" )    );

        wxMenu *help_menu = new wxMenu;
        help_menu->Append( MENU_HELP_ABOUT, _T( "About" ) );

#ifdef __WXMAC__

        // we need this in order to allow the about menu relocation, since ABOUT is
        // not the default id of the about menu
        wxApp::s_macAboutMenuItemId = MENU_HELP_ABOUT;

#endif

        help_menu->Append( MENU_HELP_SHORTCUTS,  _T( "Keyboard shortcuts" )       );
        help_menu->Append( MENU_HELP_SCREENSHOT, _T( "Screenshot" )               );
        help_menu->Append( MENU_HELP_SLIZEMOVIE, _T( "LIC slize movie" ), licMovs );

        wxMenuBar *menu_bar = new wxMenuBar;
        menu_bar->Append( file_menu,    _T( "&File" )     );
        menu_bar->Append( view_menu,    _T( "&View" )     );
        menu_bar->Append( voi_menu,     _T( "&VOI" )      );
        menu_bar->Append( surf_menu,    _T( "&Surfaces" ) );
        menu_bar->Append( options_menu, _T( "&Options" )  );
        menu_bar->Append( help_menu,    _T( "&Help" )     );

        // Associate the menu bar with the frame
        frame->SetMenuBar( menu_bar );

        wxToolBar *toolBar = new wxToolBar( frame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxNO_BORDER );

#ifndef __WXMAC__

        wxBitmap bmpOpen( fileopen_xpm );
        wxBitmap bmpSave( disc_xpm );
        wxBitmap bmpAxial( axial_xpm );
        wxBitmap bmpCor( cor_xpm );
        wxBitmap bmpSag( sag_xpm );
        wxBitmap bmpBox( box_xpm );
        wxBitmap bmpBoxOff( box_off_xpm );
        wxBitmap bmpBoxEye( box_eye_xpm );
        wxBitmap bmpGrid( grid_xpm );
        wxBitmap bmpGridSpline( grid_spline_xpm );
        wxBitmap bmpIsoSurface( iso_surface_xpm );
        wxBitmap bmpView1( view1_xpm );
        wxBitmap bmpView3( view3_xpm );
        wxBitmap bmpMiniCat( mini_cat_xpm );

        wxBitmap bmpAlphaBlend( alphablend_xpm );
        wxBitmap bmpToggleLayout( layout_xpm );
        wxBitmap bmpNewSurface( toggle_surface_xpm );
        wxBitmap bmpAssignColor( colorSelect_xpm );
        wxBitmap bmpLighting( lightbulb_xpm );
        wxBitmap bmpTubes( tubes_xpm );
        wxBitmap bmpClearColor( backgroud_color_xpm );
		wxBitmap bmpRulerTool( rulertool_xpm );
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
        wxBitmap bmpAlphaBlend(wxImage(respath+_T("icons/alphablend.png"), wxBITMAP_TYPE_PNG));
        wxBitmap bmpToggleLayout(wxImage(respath+_T("icons/layout.png"), wxBITMAP_TYPE_PNG));
        wxBitmap bmpNewSurface(wxImage(respath+_T("icons/toggleSurface.png"), wxBITMAP_TYPE_PNG));
        wxBitmap bmpAssignColor(wxImage(respath+_T("icons/colorSelect.png"), wxBITMAP_TYPE_PNG));
        wxBitmap bmpLighting(wxImage(respath+_T("icons/lightbulb.png"), wxBITMAP_TYPE_PNG));
        wxBitmap bmpTubes (wxImage(respath+_T("icons/tubes.png"), wxBITMAP_TYPE_PNG));
        wxBitmap bmpClearColor (wxImage(respath+_T("icons/background_color.png"), wxBITMAP_TYPE_PNG));
		  wxBitmap bmpRulerTool (wxImage(respath+_T("icons/rulertool.png"), wxBITMAP_TYPE_PNG));
		//wxBitmap bmpRulerTool (wxImage(respath+_T("icons/background_color.png"), wxBITMAP_TYPE_PNG));
		//wxBitmap bmpRulerToolClear (wxImage(respath+_T("icons/rulertool_clear.png"), wxBITMAP_TYPE_PNG));
		//wxBitmap bmpRulerToolAdd (wxImage(respath+_T("icons/rulertool_add.png"), wxBITMAP_TYPE_PNG));
		//wxBitmap bmpRulerToolDel (wxImage(respath+_T("icons/rulertool_del.png"), wxBITMAP_TYPE_PNG));

#endif

        toolBar->AddTool( MENU_FILE_LOAD,       bmpOpen,         wxT( "Open" )                 );
        toolBar->AddTool( MENU_FILE_SAVE,       bmpSave,         wxT( "Save Scene" )           );
        toolBar->AddTool( BUTTON_TOGGLE_LAYOUT, bmpToggleLayout, wxT( "Toggle window layout" ) );
        toolBar->AddSeparator();

        toolBar->AddCheckTool( BUTTON_AXIAL,        wxT( "Axial" ),                 bmpAxial,      wxNullBitmap, wxT( "Axial" )                 );
        toolBar->AddCheckTool( BUTTON_CORONAL,      wxT( "Coronal" ),               bmpCor,        wxNullBitmap, wxT( "Coronal" )               );
        toolBar->AddCheckTool( BUTTON_SAGITTAL,     wxT( "Sagittal" ),              bmpSag,        wxNullBitmap, wxT( "Sagittal" )              );
        toolBar->AddCheckTool( BUTTON_TOGGLE_ALPHA, wxT( "Toggle alpha blending" ), bmpAlphaBlend, wxNullBitmap, wxT( "Toggle alpha blending" ) );
        toolBar->AddSeparator();

        toolBar->AddTool( MENU_VOI_NEW_SELECTION_OBJECT_BOX, bmpBox, wxT( "New Selection Box" ) );
        toolBar->AddCheckTool( MENU_VOI_RENDER_SELECTION_OBJECTS, wxT( "Toggle Selection Objects" ),                                bmpBoxEye, wxNullBitmap, wxT( "Toggle Selection Objects" )                     );
        toolBar->AddCheckTool( MENU_VOI_TOGGLE_SELECTION_OBJECTS,            wxT( "Toggle activation status of selection object" ), bmpBoxOff, wxNullBitmap, wxT( "Toggle activation status of selection object" ) );
        toolBar->AddSeparator();

        toolBar->AddTool( MENU_SPLINESURF_NEW, bmpGridSpline, wxT( "New Spline Surface" ) );
        toolBar->AddCheckTool( MENU_SPLINESURF_DRAW_POINTS, wxT( "Toggle drawing of points" ), bmpGrid, wxNullBitmap, wxT( "Toggle drawing of points" ) );
        toolBar->AddTool( BUTTON_MOVE_POINTS1, bmpView1, wxT( "Move boundary points of spline surface" ) );
        toolBar->AddTool( BUTTON_MOVE_POINTS2, bmpView3, wxT( "Move boundary points of spline surface" ) );
        toolBar->AddSeparator();

        toolBar->AddTool( MENU_OPTIONS_ASSIGN_COLOR, bmpAssignColor, wxT( "Assign Color" ) );
        toolBar->AddSeparator();

        toolBar->AddCheckTool( MENU_OPTIONS_TOGGLE_LIGHTING, wxT( "Toggle Lighting" ), bmpLighting, wxNullBitmap, wxT( "Toggle Lighting" ) );
        toolBar->AddSeparator();

        toolBar->AddTool( MENU_FILE_NEW_ISOSURF, bmpIsoSurface, wxT( "New Iso Surface" ) );
        toolBar->AddSeparator();

        toolBar->AddCheckTool( MENU_OPTIONS_USE_FAKE_TUBES, wxT( "Toggle Tubes" ), bmpTubes, wxNullBitmap, wxT( "Toggle Tubes" ) );
        toolBar->AddSeparator();

        toolBar->AddCheckTool( MENU_OPTIONS_CLEAR_TO_BLACK, wxT( "Clear To Black" ), bmpClearColor, wxNullBitmap, wxT( "Clear To Black" ) );
        toolBar->AddSeparator();
		toolBar->AddCheckTool( MENU_OPTIONS_RULER_TOOL, wxT( "Toggle Ruler Tool" ), bmpRulerTool, wxNullBitmap, wxT( "Toggle Ruler Tool" ) );
		toolBar->AddSeparator();		

#ifdef DEBUG

        toolBar->AddTool( MENU_OPTIONS_INVERT_FIBERS, bmpMiniCat, wxT( "Invert Fibers" ) );
        toolBar->AddSeparator();
        toolBar->AddTool( MENU_FILE_RELOAD_SHADER, bmpMiniCat, wxT( "Reload Shaders" ) );
        toolBar->AddSeparator();

#endif
        toolBar->Realize();

        frame->SetToolBar( toolBar );

        frame->CreateStatusBar( 2 );
        wxStatusBar* statusBar = frame->GetStatusBar(); /* new wxStatusBar(frame, wxID_ANY, wxST_SIZEGRIP); */
		
        int widths[] = { 250, 250, -1 };
        statusBar->SetFieldsCount( WXSIZEOF(widths), widths );
        statusBar->Show();

        frame->Show( true );

        SetTopWindow( frame );

        wxString cmd;
        wxString cmdFileName;
        wxCmdLineParser cmdParser( desc, argc, argv );
        cmdParser.Parse( false );

        if ( cmdParser.GetParamCount() > 0 )
        {
            for ( size_t i = 0; i < cmdParser.GetParamCount(); ++i )
            {
                cmd = cmdParser.GetParam( i );
                wxFileName fName( cmd );
                fName.Normalize( wxPATH_NORM_LONG | wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_ABSOLUTE );
                cmdFileName = fName.GetFullPath();

                if ( cmdParser.Found(_T("d")) &&  ( i == 0 ) )
                {
                    // We pass -1 in the load function because there is no index because this is call from the cmd line.
                    frame->m_datasetHelper->load( cmdFileName, -1 );
                    frame->m_listCtrl->SetItemState(0,wxLIST_STATE_SELECTED, wxALL);
                    frame->m_datasetHelper->updateLoadStatus();
                    frame->m_datasetHelper->createDistanceMapAndIso();
                }
                else if ( cmdParser.Found( _T( "p" ) ) &&  ( i == cmdParser.GetParamCount() -1 ) )
                {
                    frame->Screenshot( cmdFileName );
                }
                else
                {
                    // We pass -1 in the load function because there is no index because this is call from the cmd line.
                    frame->m_datasetHelper->load( cmdFileName, -1 );
                }
            }
        }

        if ( cmdParser.Found( _T( "e" ) ) )
        {
            exit( 0 );
        }

        return true;

    } 
    catch ( ... )
    {
        printf( "something went wrong, terribly wrong\n" );
        return false;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Find the absolute path where this application has been run from.
//      argv0 is wxTheApp->argv[0]
//      cwd is the current working directory (at startup)
//      appVariableName is the name of a variable containing the directory for this app, e.g.
//      MYAPPDIR. This is checked first.

wxString MyApp::wxFindAppPath( const wxString& argv0, const wxString& cwd, const wxString& appVariableName, const wxString& appName )
{    
    wxString str;

#ifndef __WXWINCE__

    // Try appVariableName
    if ( !appVariableName.IsEmpty() )
    {
        str = wxGetenv( appVariableName );

        if ( !str.IsEmpty() )
            return str;
    }

#endif

#if defined(__WXMAC__) && !defined(__DARWIN__)

    return cwd;

#endif

    if ( wxIsAbsolutePath( argv0 ) )
        return wxPathOnly( argv0 );
    else
    {
        // Is it a relative path?
        if ( !cwd.IsEmpty() )
        {
            wxString currentDir( cwd );

            if ( currentDir.Last() != wxFILE_SEP_PATH )
                currentDir += wxFILE_SEP_PATH;

            str = currentDir + argv0;

            if ( wxFileExists( str ) )
                return wxPathOnly( str );
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
    pathList.AddEnvList( wxT("PATH") );
    str = pathList.FindAbsoluteValidPath( argv0 );

    if ( !str.IsEmpty() )
        return wxPathOnly( str );

    // Failed
    return wxEmptyString;
}
