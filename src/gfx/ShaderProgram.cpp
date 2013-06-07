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
#if _COMPILE_GEO_SHADERS
  m_pGeometry( NULL ),
#endif
  m_pFragment( NULL ),
  m_useGeometry( useGeometryShader ),
  m_geometrySupported( geometryShadersSupported )
{
    if( !useGeometryShader || geometryShadersSupported)
    {
        m_id = glCreateProgram();
        m_pVertex = new Shader( filename + _T( ".vs" ), SHADERTYPE_VERTEX );
#if _COMPILE_GEO_SHADERS
        if( m_useGeometry ) 
        {
            m_pGeometry = new Shader( filename + _T( ".gs" ), SHADERTYPE_GEOMETRY );
        }
#endif
        m_pFragment = new Shader( filename + _T( ".fs" ), SHADERTYPE_FRAGMENT );
    }
    else
    {
        m_oss << "Shader " << getStdStringName() << " cannot be created since geometry shaders are not supported.";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );
    }
}

//////////////////////////////////////////////////////////////////////////

// Loads the various shaders from their respective files
bool ShaderProgram::load()
{
    if( !m_useGeometry || m_geometrySupported )
    {
        m_oss << "Shader " << getStdStringName() << " starting to load...";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );

        if( !m_pVertex->load() )
        {
            m_oss << "ShaderProgram::load() could not load vertex shader " << m_pVertex->getStdStringFilename();
            Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
            m_oss.str( "" );
            return false;
        }
#if _COMPILE_GEO_SHADERS
        if( m_useGeometry && !m_pGeometry->load() )
        {
            m_oss << "ShaderProgram::load() could not load geometry shader " << m_pGeometry->getStdStringFilename();
            Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
            m_oss.str( "" );
            return false;
        }
#endif
        if( !m_pFragment->load() )
        {
            m_oss << "ShaderProgram::load() could not load fragment shader " << m_pFragment->getStdStringFilename();
            Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
            m_oss.str( "" );
            return false;
        }

        m_oss << "Shader " << getStdStringName() << " successfully loaded.";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );
        return true;
    }
    else
    {
        m_oss << "Shader " << getStdStringName() << " cannot be loaded since geometry shaders are not supported.";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
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
        m_oss << "Shader " << getStdStringName() << " starting to compile...";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );

        if( !m_pVertex->compile() )
        {
            m_oss << "ShaderProgram::compile() could not compile vertex shader " << m_pVertex->getStdStringFilename();
            Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
            m_oss.str( "" );

            printCompilerLog( m_pVertex->getId() );
            return false;
        }
#if _COMPILE_GEO_SHADERS
        if( m_useGeometry && !m_pGeometry->compile() )
        {
            m_oss << "ShaderProgram::compile() could not compile geometry shader " << m_pGeometry->getStdStringFilename();
            Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
            m_oss.str( "" );

            printCompilerLog( m_pGeometry->getId() );
            return false;
        }
#endif
        if( !m_pFragment->compile() )
        {
            m_oss << "ShaderProgram::compile() could not compile fragment shader " << m_pFragment->getStdStringFilename();
            Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
            m_oss.str( "" );

            printCompilerLog( m_pFragment->getId() );
            return false;
        }

        m_oss << "Shader " << getStdStringName() << " successfully compiled.";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );

        m_oss << "Shader " << getStdStringName() << " starting to link...";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );

        glAttachShader( m_id, m_pVertex->getId() );
        glAttachShader( m_id, m_pFragment->getId() );

#if _COMPILE_GEO_SHADERS
        if( m_useGeometry )
        {
            glAttachShader( m_id, m_pGeometry->getId() );
        }
#endif

        glLinkProgram( m_id );
        
        GLint linked;
        glGetProgramiv( m_id, GL_LINK_STATUS, &linked );
        
        printProgramLog( m_id );

        if( linked )
        {
            m_oss << "ShaderProgram " << getStdStringName() << " successfully linked.";
            Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
            m_oss.str( "" );
            
            return true;
        }
        
        m_oss << "ShaderProgram " << getStdStringName() << " could not link.";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
        return false;
    }
    else
    {
        m_oss << "Shader " << getStdStringName() << " cannot be compiled nor linked since geometry shaders are not supported.";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );
        return false;
    }
}

//////////////////////////////////////////////////////////////////////////

// Binds the program to opengl
void ShaderProgram::bind()
{
    glUseProgram( m_id );
    Logger::getInstance()->printIfGLError( wxString::Format( wxT( "ShaderProgram::bind - Binding %s" ), m_name.wx_str() ) );
}

//////////////////////////////////////////////////////////////////////////

void ShaderProgram::release()
{
    glUseProgram( 0 );
    Logger::getInstance()->printIfGLError( wxString::Format( wxT( "ShaderProgram::release - Releasing %s" ), m_name.wx_str() ) );
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
        m_oss << "No such uniform named \"" << pName << "\" in \"" << getStdStringName() << "\"";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
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
        m_oss << "No such uniform named \"" << pName << "\" in \"" << getStdStringName() << "\"";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
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
        m_oss << "No such uniform named \"" << pName << "\" in \"" << getStdStringName() << "\"";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
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
        m_oss << "No such uniform named \"" << pName << "\" in \"" << getStdStringName() << "\"";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
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
        m_oss << "No such uniform named \"" << pName << "\" in \"" << getStdStringName() << "\"";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
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
        m_oss << "No such uniform named \"" << pName << "\" in \"" << getStdStringName() << "\"";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
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
        m_oss << "No such uniform named \"" << pName << "\" in \"" << getStdStringName() << "\"";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
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
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
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
        m_oss << "No such uniform named \"" << pName << "\" in \"" << getStdStringName() << "\"";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
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
        m_oss << "No such uniform named \"" << pName << "\" in \"" << getStdStringName() << "\"";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
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
        
        Logger::getInstance()->print( wxString ( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_GLERROR );
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

        Logger::getInstance()->print( wxString ( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_GLERROR );
        m_oss.str( "" );

        free( pInfoLog );
    }
}

std::string ShaderProgram::getStdStringName()
{
    // Get std::string from wxString.
    // This avoids problems between different compiler versions.
    return std::string(m_name.mb_str());
}

//////////////////////////////////////////////////////////////////////////

ShaderProgram::~ShaderProgram()
{
    Logger::getInstance()->print( wxString::Format( wxT ( "Executing ShaderProgram (%s) destructor" ), m_name.c_str() ), LOGLEVEL_DEBUG );

    release();

    glDetachShader( m_id, m_pVertex->getId() );
    Logger::getInstance()->printIfGLError( wxString::Format( wxT( "Detaching Shader (%s) failed." ), m_pVertex->getFilename().c_str() ) );
#if _COMPILE_GEO_SHADERS
    if( m_pGeometry )
    {
        glDetachShader( m_id, m_pGeometry->getId() );
        Logger::getInstance()->printIfGLError( wxString::Format( wxT( "Detaching Shader (%s) failed." ), m_pGeometry->getFilename().c_str() ) );
    }
#endif
    glDetachShader( m_id, m_pFragment->getId() );
    Logger::getInstance()->printIfGLError( wxString::Format( wxT( "Detaching Shader (%s) failed." ), m_pFragment->getFilename().c_str() ) );

    glDeleteProgram( m_id );
    Logger::getInstance()->printIfGLError( wxString::Format( wxT( "Deleting ShaderProgram (%s) failed." ), m_name.c_str() ) );

    delete m_pVertex;
    m_pVertex = NULL;

#if _COMPILE_GEO_SHADERS
    delete m_pGeometry;
    m_pGeometry = NULL;
#endif

    delete m_pFragment;
    m_pFragment = NULL;

    Logger::getInstance()->print( wxString::Format( wxT ( "ShaderProgram (%s) destructor done" ), m_name.c_str() ), LOGLEVEL_DEBUG );
}
