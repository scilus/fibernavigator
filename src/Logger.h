/*
 *  The Logger class declaration.
 *
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <GL/glew.h>
#include <wx/string.h>
#include <sstream>

// print levels
enum LogLevel { LOGLEVEL_DEBUG, LOGLEVEL_MESSAGE, LOGLEVEL_WARNING, LOGLEVEL_ERROR, LOGLEVEL_GLERROR };

class Logger
{
public:
    ~Logger();

    static Logger * getInstance();

    void print( const wxString &str, const LogLevel level );
    bool printIfGLError( wxString str );

    wxString getLastError() const { return m_lastError; }
    void setMessageLevel( int level );

protected:
    Logger();

private:
    static Logger *m_pInstance;
    int m_level;
    std::ostringstream m_oss;
    wxString m_lastError;
};

#endif // LOGGER_H_