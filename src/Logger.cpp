#include "Logger.h"

#include <GL/glew.h>
#include <wx/datetime.h>

Logger * Logger::m_pInstance = NULL;

Logger::Logger()
:
#if defined( DEBUG ) || defined( _DEBUG )
    m_level( 0 )
#else
    m_level( 1 )
#endif
{

}

//////////////////////////////////////////////////////////////////////////

Logger * Logger::getInstance()
{
    if ( NULL == m_pInstance )
    {
        m_pInstance = new Logger();
    }

    return m_pInstance;
}

//////////////////////////////////////////////////////////////////////////

void Logger::printDebug( const wxString str, const LogLevel level )
{
    if ( m_level > (int)level )
        return;

    wxString prefix;
    switch (level)
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
    printwxT( prefix + str + _T( "\n" ) );
}

//////////////////////////////////////////////////////////////////////////

void Logger::printGLError( const wxString str, const GLenum errorCode )
{
    printDebug( str, LOGLEVEL_GLERROR );
    printf( "%s\n", gluErrorString( errorCode ) );
}

//////////////////////////////////////////////////////////////////////////

void Logger::setMessageLevel( int level )
{
    m_level = level;
}

//////////////////////////////////////////////////////////////////////////

void Logger::printTime()
{
    wxDateTime time = wxDateTime::Now();
    printf( "[%02d:%02d:%02d] ", time.GetHour(), time.GetMinute(), time.GetSecond() );
}

//////////////////////////////////////////////////////////////////////////

void Logger::printwxT( const wxString str )
{
    char* pStr = (char*)malloc( str.length() + 1 );
    strcpy( pStr, (const char*)str.mb_str( wxConvUTF8 ) );
    printf( "%s", pStr );
    free( pStr );
}

//////////////////////////////////////////////////////////////////////////

Logger::~Logger()
{

}

