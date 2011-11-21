/*
 *  The Logger class declaration.
 *
 */

#ifndef LOG_H_
#define LOG_H_

#include <GL/glew.h>
#include <wx/string.h>

// printDebug levels
enum LogLevel { LOGLEVEL_DEBUG, LOGLEVEL_MESSAGE, LOGLEVEL_WARNING, LOGLEVEL_ERROR, LOGLEVEL_GLERROR };

class Logger
{
public:
    ~Logger();

    static Logger * getInstance();

    void printDebug( const wxString i_string, const LogLevel i_level );
    void printGLError( const wxString i_string, const GLenum i_errorCode );
    void setMessageLevel( int i_level );

protected:
    Logger();

private:
    void printTime();
    void printwxT( const wxString i_string );

    static Logger *m_pInstance;
    int m_level;
};

#endif LOG_H_