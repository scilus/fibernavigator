/////////////////////////////////////////////////////////////////////////////
// Name:        main.cpp
// Purpose:     main window
// Author:      Ralph Schurade
// Created:     03/27/08
/////////////////////////////////////////////////////////////////////////////

#include "main.h"

#include "Logger.h"
#include "dataset/DatasetManager.h"
#include "dataset/Loader.h"
#include "gfx/ShaderHelper.h"
#include "gui/MainFrame.h"
#include "gui/MenuBar.h"
#include "gui/SceneManager.h"
#include "gui/ToolBar.h"

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/cmdline.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/mdi.h>
#include <wx/wxprec.h>

#include <exception>

wxString    MyApp::respath;
wxString    MyApp::shaderPath;
wxString    MyApp::iconsPath;
MainFrame * MyApp::frame = NULL;

const wxString MyApp::APP_NAME   = _T( "main" );
const wxString MyApp::APP_VENDOR = _T( "Ralph S. & Mario H." );

IMPLEMENT_APP( MyApp )

static const wxCmdLineEntryDesc desc[] =
{
    { wxCMD_LINE_SWITCH, _T("h"), _T("help"), _T("help yourself") },
    { wxCMD_LINE_SWITCH, _T("p"), _T("screenshot"), _T("screenshot") },
    { wxCMD_LINE_SWITCH, _T("d"), _T("dmap"), _T("create a distance map on the first loaded dataset") },
    { wxCMD_LINE_SWITCH, _T("e"), _T("exit"), _T("exit after executing the command line") },
    { wxCMD_LINE_PARAM, NULL, NULL, _T("scene file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },
    { wxCMD_LINE_NONE } 
};

MyApp::MyApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// Initialize this in OnInit, not statically

bool MyApp::OnInit( void )
{
    try
    {
        SetAppName( APP_NAME );
        SetVendorName( APP_VENDOR );

        respath = wxFindAppPath( argv[0], wxGetCwd(), _T("FNPATH"), _T("fn") );

#ifdef __WXMSW__

        if (respath.Last() != '\\') respath += '\\';
        shaderPath = respath + _T("GLSL\\");
        iconsPath  = respath + _T("icons\\");
        
        int fd;
        FILE *fp;
        AllocConsole();
        fd = _open_osfhandle( (long)GetStdHandle( STD_OUTPUT_HANDLE ), 0);
        fp = _fdopen( fd, "w" );
        *stdout = *fp;
        setvbuf( stdout, NULL, _IONBF, 0 );        

#elif __WXMAC__

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
        fprintf( stderr, "%s", path );

        respath = wxString::FromAscii( path );

        respath += _T( "/Contents/Resources/" );
        shaderPath = respath + _T( "GLSL/" );
        iconsPath = respath + _T( "icons/" );

#else
        if ( respath.Last() != '/' )
            respath += '/';
        shaderPath = respath + _T("GLSL/");
        iconsPath = respath + _T("icons/");

#endif

        Logger::getInstance()->print( wxT( "Warning: This version of Fibernavigator is debug compiled." ), LOGLEVEL_DEBUG );
        Logger::getInstance()->print( wxT( "For better performance please compile a Release version."), LOGLEVEL_DEBUG );
        Logger::getInstance()->print( wxString::Format( wxT( "respath: %s" ), respath.c_str() ), LOGLEVEL_DEBUG );
        Logger::getInstance()->print( wxString::Format( wxT( "shader: %s" ), shaderPath.c_str() ), LOGLEVEL_DEBUG );

        // Create the main frame window
        frame = new MainFrame( NULL, wxID_ANY, _T("Fiber Navigator 1219"), wxPoint( 50, 50 ), wxSize( 800, 600 ), wxDEFAULT_FRAME_STYLE );
        SceneManager::getInstance()->setMainFrame( frame );
        SceneManager::getInstance()->setTreeCtrl( frame->m_pTreeWidget );

#ifdef __WXMSW__
        // Give it an icon (this is ignored in MDI mode: uses resources)
        frame->SetIcon( wxIcon( _T( "sashtest_icn" ) ) );
#endif
        
        frame->SetMinSize( wxSize( 800, 600 ) );
        frame->SetSize( wxSize( 1024, 768 ) );
                
        frame->Show( true );
        SetTopWindow( frame );

        wxString cmd;
        wxString cmdFileName;
        wxCmdLineParser cmdParser( desc, argc, argv );
        cmdParser.Parse( false );
        
        if ( cmdParser.GetParamCount() > 0 )
        {
            Loader loader = Loader(frame, frame->m_pListCtrl );
            for ( size_t i = 0; i < cmdParser.GetParamCount(); ++i )
            {
                cmd = cmdParser.GetParam( i );
                wxFileName fName( cmd );
                fName.Normalize( wxPATH_NORM_LONG | wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_ABSOLUTE );
                cmdFileName = fName.GetFullPath();

                if ( cmdParser.Found(_T("d")) &&  ( i == 0 ) )
                {
                    loader( cmdFileName );
                    frame->createDistanceMapAndIso();
                }
                else if ( cmdParser.Found( _T( "p" ) ) &&  ( i == cmdParser.GetParamCount() -1 ) )
                {
                    frame->screenshot( wxT( "" ), cmdFileName );
                }
                else
                {
                    loader( cmdFileName );
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
        Logger::getInstance()->print( wxT( "Something went wrong, terribly wrong" ), LOGLEVEL_ERROR );
        return false;
    }
    Logger::getInstance()->print( wxT( "End on init main" ), LOGLEVEL_DEBUG );
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

int MyApp::OnExit()
{
    Logger::getInstance()->print( wxT( "Exiting..." ), LOGLEVEL_MESSAGE );
    Logger::getInstance()->print( wxT( "Cleaning ressources..." ), LOGLEVEL_MESSAGE );
    frame = NULL;

    // Deleting singletons
    delete ShaderHelper::getInstance();
    delete DatasetManager::getInstance();
	delete SceneManager::getInstance();
    delete Logger::getInstance();
    return 0;
}
