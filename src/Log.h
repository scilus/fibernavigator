/*
 *  The Log class declaration.
 *
 */

#ifndef LOG_H_
#define LOG_H_

#include <GL/glew.h>
#include <wx/string.h>

// printDebug levels
enum LogLevel { LOGLEVEL_DEBUG, LOGLEVEL_MESSAGE, LOGLEVEL_WARNING, LOGLEVEL_ERROR, LOGLEVEL_GLERROR };

class Log
{
public:
    ~Log();

    static Log * getInstance();

    void printDebug( const wxString i_string, const LogLevel i_level );
    void printGLError( const wxString i_string, const GLenum i_errorCode );
    void setMessageLevel( int i_level );

protected:
    Log();

private:
    void printTime();
    void printwxT( const wxString i_string );

    static Log *m_pInstance;
    int m_level;
};

#endif LOG_H_