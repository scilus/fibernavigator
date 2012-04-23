#include "Shader.h"

#include "../main.h"
#include "../Logger.h"

#include <GL/glew.h>
#include <wx/filename.h>
#include <wx/string.h>
#include <wx/textfile.h>

#include <fstream>
using std::ifstream;

#include <sstream>
using std::ostringstream;


Shader::Shader( wxString filename, SHADERTYPE type )
: m_id( NULL ),
  m_filename( filename )  
{
    switch ( type )
    {
    case SHADERTYPE_VERTEX:
        m_id = glCreateShader( GL_VERTEX_SHADER );
        break;
    case SHADERTYPE_GEOMETRY:
        m_id = glCreateShader( GL_GEOMETRY_SHADER );
        break;
    case SHADERTYPE_FRAGMENT:
        m_id = glCreateShader( GL_FRAGMENT_SHADER );
        break;
    default:
        Logger::getInstance()->print( _T( "Shader type not supported." ), LOGLEVEL_ERROR );
    }

    if( 0 == m_id )
    {
        m_oss << "Failed to create shader " << m_filename.char_str() << ".";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_GLERROR );
        m_oss.str( "" );
    }
}

//////////////////////////////////////////////////////////////////////////

bool Shader::load()
{
    m_oss << "Shader " << m_filename.char_str() << " starting to load...";
    Logger::getInstance()->print(wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
    m_oss.str( "" );

    if( 0 != m_id )
    {
        if( fileExists() )
        {
            if( loadFromFile( &m_code, m_filename ) )
            {
                m_oss << "Shader " << m_filename.char_str() << " finished loading.";
                Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
                m_oss.str( "" );
                return true;
            }

            m_oss << "Could not load file " << m_filename.char_str();
            Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
            m_oss.str( "" );
            return false;
        }

        m_oss << "File " << m_filename.char_str() << " not found.";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_WARNING );
        m_oss.str( "" );
        return false;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

bool Shader::compile()
{
    if( 0 != m_id )
    {
        GLuint *pId = &m_id;

        m_oss << "Shader " << m_filename.char_str() << " starting to compile...";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );

        char *pTemp;
        pTemp = ( char* ) malloc( m_code.Length() + 1 );
        strcpy( pTemp, ( const char* ) m_code.mb_str( wxConvUTF8 ) );
        const char* pCode = pTemp;

        glShaderSource( *pId, 1, &pCode, NULL );
        glCompileShader( *pId );
        GLint compiled;
        glGetShaderiv( *pId, GL_COMPILE_STATUS, &compiled );

        m_oss << "Shader " << m_filename.char_str() << " finished compiling.";
        Logger::getInstance()->print( wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
        m_oss.str( "" );

        free( pTemp );
        return 0 != compiled; // removes casting warning
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

const wxString & Shader::getFilename()
{
    return m_filename;
}

//////////////////////////////////////////////////////////////////////////

const GLuint & Shader::getId()
{
    return m_id;
}

//////////////////////////////////////////////////////////////////////////

bool Shader::loadFromFile( wxString *pCode, const wxString &filename )
{
    wxTextFile file;
    *pCode = wxT( "" );

    if( file.Open( MyApp::shaderPath + filename ) )
    {
        for( size_t i( 0 ); i < file.GetLineCount(); ++i )
        {
            if( file.GetLine( i ).BeforeFirst( ' ' ) == wxT( "#include" ) )
            {
                wxString include = wxT( "" );
                loadFromFile( &include, file.GetLine( i ).AfterFirst( ' ' ) );
                *pCode += include;
            }
            else
            {
                wxString line = file.GetLine( i );
                line.Trim();
                line.Trim( true );

                *pCode += line + wxT( '\n' );
            }
        }
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

bool Shader::fileExists()
{
    return wxFileName::FileExists( MyApp::shaderPath + m_filename );
}

//////////////////////////////////////////////////////////////////////////

Shader::~Shader()
{
    glDeleteShader( m_id );
    Logger::getInstance()->printIfGLError( wxString::Format( wxT( "Deleting Shader (%s) failed." ), m_filename.c_str() ) );
    m_id = 0;
}
