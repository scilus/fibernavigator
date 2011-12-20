/*
 *  The Logger class declaration.
 *
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <GL/glew.h>
#include <wx/string.h>

// printDebug levels
enum LogLevel { LOGLEVEL_DEBUG, LOGLEVEL_MESSAGE, LOGLEVEL_WARNING, LOGLEVEL_ERROR, LOGLEVEL_GLERROR };

class Logger
{
public:
    ~Logger();

    static Logger * getInstance();

    void printDebug( const wxString str, const LogLevel level );
    void printGLError( const wxString str, const GLenum errorCode );
    void setMessageLevel( int level );

protected:
    Logger();

private:
    void printTime();
    void printwxT( const wxString str );

    static Logger *m_pInstance;
    int m_level;
};

#endif // LOGGER_H_