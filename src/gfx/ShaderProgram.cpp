#include "ShaderProgram.h"

#include <sstream>
using std::ostringstream;

#include <GL/glew.h>
#include <wx/string.h>

#include "Shader.h"
#include "../misc/Fantom/FMatrix.h"

#include "../Logger.h"

ShaderProgram::ShaderProgram( wxString filename, bool useGeometryShader, bool geometryShadersSupported )
: m_id( NULL ),
  m_name( filename ),
  m_pVertex( NULL ),
  m_pGeometry( NULL ),
  m_pFragment( NULL ),
  m_useGeometry( useGeometryShader ),
  m_geometrySupported( geometryShadersSupported )
{
    if( !useGeometryShader || geometryShadersSupported)
    {
        m_id = glCreateProgram();
        m_pVertex = new Shader( filename + _T( ".vs" ), SHADERTYPE_VERTEX );
        if( m_useGeometry ) 
        {
            m_pGeometry = new Shader( filename + _T( ".gs" ), SHADERTYPE_GEOMETRY );
        }
        m_pFragment = new Shader( filename + _T( ".fs" ), SHADERTYPE_FRAGMENT );
    }
    else
    {
        m_oss << "Shader " << m_name.char_str() << " cannot be created since geometry shaders are not supported.";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );
    }
}

//////////////////////////////////////////////////////////////////////////

// Loads the various shaders from their respective files
bool ShaderProgram::load()
{
    if( !m_useGeometry || m_geometrySupported )
    {
        m_oss << "Shader " << m_name.char_str() << " starting to load...";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );

        if( !m_pVertex->load() )
        {
            m_oss << "ShaderProgram::load() could not load vertex shader " << m_pVertex->getFilename().char_str();
            Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
            m_oss.str( "" );
            return false;
        }
        if( m_useGeometry && !m_pGeometry->load() )
        {
            m_oss << "ShaderProgram::load() could not load geometry shader " << m_pGeometry->getFilename().char_str();
            Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
            m_oss.str( "" );
            return false;
        }
        if( !m_pFragment->load() )
        {
            m_oss << "ShaderProgram::load() could not load fragment shader " << m_pFragment->getFilename().char_str();
            Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
            m_oss.str( "" );
            return false;
        }

        m_oss << "Shader " << m_name.char_str() << " successfully loaded.";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );
        return true;
    }
    else
    {
        m_oss << "Shader " << m_name.char_str() << " cannot be loaded since geometry shaders are not supported.";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );
        return false;
    }
}

//////////////////////////////////////////////////////////////////////////

// Compiles and links the shaders that form the program
bool ShaderProgram::compileAndLink()
{
    if( !m_useGeometry || m_geometrySupported )
    {
        m_oss << "Shader " << m_name.char_str() << " starting to compile...";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );

        if( !m_pVertex->compile() )
        {
            m_oss << "ShaderProgram::compile() could not compile vertex shader " << m_pVertex->getFilename().char_str();
            Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
            m_oss.str( "" );

            printCompilerLog( m_pVertex->getId() );
            return false;
        }
        if( m_useGeometry && !m_pGeometry->compile() )
        {
            m_oss << "ShaderProgram::compile() could not compile geometry shader " << m_pGeometry->getFilename().char_str();
            Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
            m_oss.str( "" );

            printCompilerLog( m_pGeometry->getId() );
            return false;
        }
        if( !m_pFragment->compile() )
        {
            m_oss << "ShaderProgram::compile() could not compile fragment shader " << m_pFragment->getFilename().char_str();
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
            m_oss << "ShaderProgram " << m_name.char_str() << " successfully linked.";
            Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
            m_oss.str( "" );
            
            return true;
        }
        
        m_oss << "ShaderProgram " << m_name.char_str() << " could not link.";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
        return false;

    }
    else
    {
        m_oss << "Shader " << m_name.char_str() << " cannot be compiled nor linked since geometry shaders are not supported.";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );
        return false;
    }
}

//////////////////////////////////////////////////////////////////////////

// Binds the program to opengl
void ShaderProgram::bind()
{
    glUseProgram( m_id );
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::release()
{
    glUseProgram( 0 );
}

//////////////////////////////////////////////////////////////////////////

GLuint ShaderProgram::getId()
{
    return m_id;
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::setUniInt( const GLchar* pName, const GLint value )
{
    GLint loc = glGetUniformLocation( m_id, pName );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << pName << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform1i( loc, value );
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::setUniFloat( const GLchar* pName, const GLfloat value )
{
    GLint loc = glGetUniformLocation( m_id, pName );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << pName << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform1f( loc, value );
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::setUniArrayInt( const GLchar* pName, GLint* pValue, const GLint size )
{
    GLint loc = glGetUniformLocation( m_id, pName );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << pName << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform1iv( loc, size, pValue );
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::setUni3Int( const GLchar* pName, GLint values[3] )
{
    GLint loc = glGetUniformLocation( m_id, pName );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << pName << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform3i( loc, values[0], values[1], values[2] );
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::setUniArray1Float( const GLchar* pName, GLfloat* pValue, const int size )
{
    GLint loc = glGetUniformLocation( m_id, pName );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << pName << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform1fv( loc, size, pValue );
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::setUni2Float( const GLchar* pName, std::pair< GLfloat, GLfloat > values )
{
    GLint loc = glGetUniformLocation( m_id, pName );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << pName << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform2f( loc, values.first, values.second );
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::setUni3Float( const GLchar* pName, GLfloat values[3] )
{
    GLint loc = glGetUniformLocation( m_id, pName );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << pName << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform3f( loc, values[0], values[1], values[2] );
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::setAttribFloat( const GLchar* pName, const GLfloat value )
{
    GLint attrib = glGetAttribLocation( m_id, pName );
    if( -1 == attrib )
    {
        m_oss << "No such attribute named \"" << pName << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glVertexAttrib1f( attrib, value );
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::setUniMatrix3f( const GLchar* pName, const FMatrix &values )
{
    float* pValueCast = new float[9];

    pValueCast[0] = values(0,0);
    pValueCast[1] = values(0,1);
    pValueCast[2] = values(0,2);
    pValueCast[3] = values(1,0);
    pValueCast[4] = values(1,1);
    pValueCast[5] = values(1,2);
    pValueCast[6] = values(2,0);
    pValueCast[7] = values(2,1);
    pValueCast[8] = values(2,2);

    GLint loc = glGetUniformLocation( m_id, pName );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << pName << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniformMatrix3fv( loc, 1, GL_TRUE, pValueCast );

    delete[] pValueCast;
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::setUniSampler( const GLchar* pName, const GLint value )
{
    GLint loc = glGetUniformLocation( m_id, pName );
    if( -1 == loc )
    {
        m_oss << "No such uniform named \"" << pName << "\"";
        Logger::getInstance()->printDebug( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
    }
    glUniform1i( loc, value );
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::setUniBool( const GLchar* pName, const bool value )
{
    setUniInt( pName, value );
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::printCompilerLog( GLuint shaderId )
{
    GLint infoLogLen = 0;
    GLint charsWritten = 0;
    GLchar *pInfoLog;

    glGetShaderiv( shaderId, GL_INFO_LOG_LENGTH, &infoLogLen );

    if( 1 < infoLogLen )
    {
        pInfoLog = ( GLchar* ) malloc( infoLogLen );
        glGetShaderInfoLog( shaderId, infoLogLen, &charsWritten, pInfoLog );
        
        m_oss << "Compiler Log:\n";
        m_oss << "===========================================================\n";
        m_oss << pInfoLog << "\n";
        m_oss << "===========================================================";
        
        Logger::getInstance()->printDebug( wxString ( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_GLERROR );
        m_oss.str( "" );

        free( pInfoLog );
    }
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::printProgramLog( GLuint programId )
{
    GLint infoLogLen = 0;
    GLint charsWritten = 0;
    GLchar *pInfoLog;

    glGetProgramiv( programId, GL_INFO_LOG_LENGTH, &infoLogLen );
    
    if( 1 < infoLogLen )
    {
        pInfoLog = ( GLchar* ) malloc( infoLogLen );
        glGetProgramInfoLog( programId, infoLogLen, &charsWritten, pInfoLog );
        
        m_oss << "ShaderProgram Log:\n";
        m_oss << "===========================================================\n";
        m_oss << pInfoLog << "\n";
        m_oss << "===========================================================";

        Logger::getInstance()->printDebug( wxString ( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_GLERROR );
        m_oss.str( "" );

        free( pInfoLog );
    }
}

//////////////////////////////////////////////////////////////////////////

ShaderProgram::~ShaderProgram()
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
		delete m_pFragment;
    }
}


