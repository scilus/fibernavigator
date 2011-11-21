#include "Program.h"

#include <sstream>
using std::ostringstream;

#include <GL/glew.h>
#include <wx/string.h>

#include "Shader2.h"
#include "../misc/Fantom/FMatrix.h"

#include "../Logger.h"

Program::Program( wxString filename, bool useGeometryShader )
: m_name( filename ),
  m_pVertex( NULL ),
  m_pGeometry( NULL ),
  m_pFragment( NULL ),
  m_useGeometry ( useGeometryShader )
{
    m_id = glCreateProgram();
    m_pVertex = new Shader2( filename + _T( ".vs" ), SHADERTYPE_VERTEX );
    if( m_useGeometry ) 
    {
        m_pGeometry = new Shader2( filename + _T( ".gs" ), SHADERTYPE_GEOMETRY );
    }
    m_pFragment = new Shader2( filename + _T( ".fs" ), SHADERTYPE_FRAGMENT );
}

//////////////////////////////////////////////////////////////////////////

// Loads the various shaders from their respective files
bool Program::load()
{
    m_oss << "Shader " << m_name.char_str() << " starting to load...";
    Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
    m_oss.str( "" );

    if( !m_pVertex->load() )
    {
        m_oss << "Program::load() could not load vertex shader " << m_pVertex->getFilename().char_str();
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
        return false;
    }
    if( m_useGeometry && !m_pGeometry->load() )
    {
        m_oss << "Program::load() could not load geometry shader " << m_pGeometry->getFilename().char_str();
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
        return false;
    }
    if( !m_pFragment->load() )
    {
        m_oss << "Program::load() could not load fragment shader " << m_pFragment->getFilename().char_str();
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
        return false;
    }

    m_oss << "Shader " << m_name.char_str() << " successfully loaded.";
    Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
    m_oss.str( "" );
    return true;
}

//////////////////////////////////////////////////////////////////////////

// Compiles and links the shaders that form the program
bool Program::compileAndLink()
{
    m_oss << "Shader " << m_name.char_str() << " starting to compile...";
    Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
    m_oss.str( "" );

    if( !m_pVertex->compile() )
    {
        m_oss << "Program::compile() could not compile vertex shader " << m_pVertex->getFilename().char_str();
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );

        printCompilerLog( m_pVertex->getId() );
        return false;
    }
    if( m_useGeometry && !m_pGeometry->compile() )
    {
        m_oss << "Program::compile() could not compile geometry shader " << m_pGeometry->getFilename().char_str();
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );

        printCompilerLog( m_pGeometry->getId() );
        return false;
    }
    if( !m_pFragment->compile() )
    {
        m_oss << "Program::compile() could not compile fragment shader " << m_pFragment->getFilename().char_str();
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );

        printCompilerLog( m_pFragment->getId() );
        return false;
    }

    m_oss << "Shader " << m_name.char_str() << " successfully compiled.";
    Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
    m_oss.str( "" );

    m_oss << "Shader " << m_name.char_str() << " starting to link...";
    Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
    m_oss.str( "" );

    glAttachShader( m_id, m_pVertex->getId() );
    glAttachShader( m_id, m_pFragment->getId() );
    if( m_useGeometry )
    {
        glAttachShader( m_id, m_pGeometry->getId() );
    }

    glLinkProgram( m_id );
    
    GLint linked;
    glGetProgramiv( m_id, GL_LINK_STATUS, &linked );
    
    printProgramLog( m_id );

    if( linked )
    {
        m_oss << "Program " << m_name.char_str() << " successfully linked.";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );
        
        return true;
    }
    
    m_oss << "Program " << m_name.char_str() << " could not link.";
    Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
    m_oss.str( "" );
    return false;
}

//////////////////////////////////////////////////////////////////////////

// Binds the program to opengl
void Program::bind()
{
    glUseProgram( m_id );
}

//////////////////////////////////////////////////////////////////////////

void Program::release()
{
    glUseProgram(0);
}

//////////////////////////////////////////////////////////////////////////

GLuint Program::getId()
{
    return m_id;
}

//////////////////////////////////////////////////////////////////////////

void Program::setUniInt(const GLchar* name, const GLint value)
{
    GLint loc = glGetUniformLocation( m_id, name );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << name << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform1i( loc, value );
}

//////////////////////////////////////////////////////////////////////////

void Program::setUniFloat(const GLchar* name, const GLfloat value)
{
    GLint loc = glGetUniformLocation( m_id, name );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << name << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform1f( loc, value );
}

//////////////////////////////////////////////////////////////////////////

void Program::setUniArrayInt(const GLchar* name, GLint* value, const GLint size)
{
    GLint loc = glGetUniformLocation( m_id, name );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << name << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform1iv( loc, size, value );
}

//////////////////////////////////////////////////////////////////////////

void Program::setUni3Int(const GLchar* name, GLint values[3])
{
    GLint loc = glGetUniformLocation( m_id, name );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << name << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform3i( loc, values[0], values[1], values[2] );
}

//////////////////////////////////////////////////////////////////////////

void Program::setUniArray1Float(const GLchar* name, GLfloat* value, const int size)
{
    GLint loc = glGetUniformLocation( m_id, name );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << name << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform1fv( loc, size, value );
}

//////////////////////////////////////////////////////////////////////////

void Program::setUni2Float(const GLchar* name, std::pair< GLfloat, GLfloat > values)
{
    GLint loc = glGetUniformLocation( m_id, name );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << name << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform2f( loc, values.first, values.second );
}

//////////////////////////////////////////////////////////////////////////

void Program::setUni3Float(const GLchar* name, GLfloat values[3])
{
    GLint loc = glGetUniformLocation( m_id, name );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << name << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform3f( loc, values[0], values[1], values[2] );
}

//////////////////////////////////////////////////////////////////////////

void Program::setAttribFloat(const GLchar* name, const GLfloat value)
{
    GLint attrib = glGetAttribLocation( m_id, name );
    if( -1 == attrib )
    {
        m_oss << "No such attribute named \"" << name << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glVertexAttrib1f( attrib, value );
}

//////////////////////////////////////////////////////////////////////////

void Program::setUniMatrix3f(const GLchar* name, const FMatrix &values)
{
    float* l_valueCast = new float[9];

    l_valueCast[0] = values(0,0);
    l_valueCast[1] = values(0,1);
    l_valueCast[2] = values(0,2);
    l_valueCast[3] = values(1,0);
    l_valueCast[4] = values(1,1);
    l_valueCast[5] = values(1,2);
    l_valueCast[6] = values(2,0);
    l_valueCast[7] = values(2,1);
    l_valueCast[8] = values(2,2);

    GLint loc = glGetUniformLocation( m_id, name );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << name << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniformMatrix3fv( loc, 1, GL_TRUE, l_valueCast );

    delete[] l_valueCast;
}

//////////////////////////////////////////////////////////////////////////

void Program::setUniSampler(const GLchar* name, const GLint value)
{
    GLint loc = glGetUniformLocation( m_id, name );
    if (loc == -1)
    {
        m_oss << "No such uniform named \"" << name << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform1i( loc, value );
}

//////////////////////////////////////////////////////////////////////////

void Program::printCompilerLog( GLuint shaderId )
{
    GLint infologLen = 0;
    GLint charsWritten = 0;
    GLchar *infoLog;

    glGetShaderiv( shaderId, GL_INFO_LOG_LENGTH, &infologLen );

    if( infologLen > 1 )
    {
        infoLog = ( GLchar* ) malloc( infologLen );
        glGetShaderInfoLog( shaderId, infologLen, &charsWritten, infoLog );
        
        m_oss << "Compiler Logger:\n";
        m_oss << "===========================================================\n";
        m_oss << infoLog << "\n";
        m_oss << "===========================================================";
        
        Logger::getInstance()->printDebug( wxString ( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_GLERROR );
        m_oss.str( "" );

        free( infoLog );
    }
}

//////////////////////////////////////////////////////////////////////////

void Program::printProgramLog( GLuint id )
{
    GLint infologLen = 0;
    GLint charsWritten = 0;
    GLchar *infoLog;

    glGetProgramiv( id, GL_INFO_LOG_LENGTH, &infologLen );
    
    if( infologLen > 1 )
    {
        infoLog = ( GLchar* ) malloc( infologLen );
        glGetProgramInfoLog( id, infologLen, &charsWritten, infoLog );
        
        m_oss << "Program Logger:\n";
        m_oss << "===========================================================\n";
        m_oss << infoLog << "\n";
        m_oss << "===========================================================";

        Logger::getInstance()->printDebug( wxString ( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_GLERROR );
        m_oss.str( "" );

        free (infoLog);
    }
}

//////////////////////////////////////////////////////////////////////////

Program::~Program()
{
    if( NULL != m_pVertex )
    {
        delete m_pVertex;
    }
    if( NULL != m_pGeometry )
    {
        delete m_pGeometry;
    }
    if( NULL != m_pFragment )
    {
        delete m_pGeometry;
    }
}


