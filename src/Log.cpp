#include "Log.h"

#include <GL/glew.h>
#include <wx/datetime.h>


Log * Log::m_pInstance = NULL;

Log::Log()
:
#if defined( DEBUG ) || defined( _DEBUG )
    m_level( 0 )
#else
    m_level( 1 )
#endif
{

}

//////////////////////////////////////////////////////////////////////////

Log * Log::getInstance()
{
    if ( NULL == m_pInstance )
    {
        m_pInstance = new Log();
    }

    return m_pInstance;
}

//////////////////////////////////////////////////////////////////////////

void Log::printDebug( const wxString i_string, const LogLevel i_level )
{
    if ( m_level > (int)i_level )
        return;

    wxString prefix;
    switch (i_level)
    {
    case LOGLEVEL_DEBUG:
        prefix = _T( "DEBUG: " );
        break;
    case LOGLEVEL_WARNING:
        prefix = _T( "WARNING: " );
        break;
    case LOGLEVEL_ERROR:
        prefix = _T( "ERROR: " );
        break;
    case LOGLEVEL_GLERROR:
        prefix = _T( "GLERROR: " );
        break;
    case LOGLEVEL_MESSAGE: // same as default
    default:
        prefix = _T( "" );
    }

    printTime();
    printwxT( prefix + i_string + _T( "\n" ) );
}

//////////////////////////////////////////////////////////////////////////

void Log::printGLError( const wxString i_string, const GLenum i_errorCode )
{
    printDebug( i_string, LOGLEVEL_GLERROR );
    printf( "%s\n", gluErrorString( i_errorCode ) );
}

//////////////////////////////////////////////////////////////////////////

void Log::setMessageLevel( int i_level )
{
    m_level = i_level;
}

//////////////////////////////////////////////////////////////////////////

void Log::printTime()
{
    wxDateTime l_dataTime = wxDateTime::Now();
    printf( "[%02d:%02d:%02d] ", l_dataTime.GetHour(), l_dataTime.GetMinute(), l_dataTime.GetSecond() );
}

//////////////////////////////////////////////////////////////////////////

void Log::printwxT( const wxString i_string )
{
    char* l_cstring = (char*)malloc( i_string.length() + 1 );
    strcpy( l_cstring, (const char*)i_string.mb_str( wxConvUTF8 ) );
    printf( "%s", l_cstring );
    free( l_cstring );
}

//////////////////////////////////////////////////////////////////////////

Log::~Log()
{

}

