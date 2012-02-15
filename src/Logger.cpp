#include "Logger.h"

#include <GL/glew.h>
#include <wx/datetime.h>
#include <iomanip>
using std::setw;
using std::setfill;

Logger * Logger::m_pInstance = NULL;

Logger::Logger()
:
#if defined( DEBUG ) || defined( _DEBUG )
    m_level( 0 )
#else
    m_level( 1 )
#endif
{
    m_lastError = wxT( "No error to report" );
}

//////////////////////////////////////////////////////////////////////////

Logger * Logger::getInstance()
{
    if ( NULL == m_pInstance )
    {
        m_pInstance = new Logger();
        m_pInstance->m_oss << setfill('0');
    }

    return m_pInstance;
}

//////////////////////////////////////////////////////////////////////////

void Logger::print( const wxString &str, const LogLevel level )
{
    if ( m_level > (int)level )
        return;

    wxString prefix;
    switch( level )
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

    wxDateTime time = wxDateTime::Now();
    m_oss << "[" << setw(2) << time.GetHour() << ":" << setw(2) << time.GetMinute() << ":" << setw(2) << time.GetSecond() << "]" << " " << prefix.char_str() << str.char_str() << "\n";
    
    if( LOGLEVEL_ERROR == level || LOGLEVEL_GLERROR == level )
    {
        m_lastError = prefix + str;
    }

    printf( m_oss.str().c_str() );
    m_oss.str( "" );
}

//////////////////////////////////////////////////////////////////////////

bool Logger::printIfGLError( wxString str )
{
    GLenum error = glGetError();
    if( GL_NO_ERROR != error )
    {
        str.Append( wxT( "\n" ) );
        m_oss << gluErrorString( error );
        str.Append( wxString( m_oss.str().c_str(), wxConvUTF8 ) );
        m_oss.str( "" );
        print( str, LOGLEVEL_GLERROR );
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

void Logger::setMessageLevel( int level )
{
    m_level = level;
}

//////////////////////////////////////////////////////////////////////////

Logger::~Logger()
{
    m_pInstance = NULL;
}

