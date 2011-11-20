#include "Shader2.h"

#include <fstream>
using std::ifstream;

#include <sstream>
using std::ostringstream;

#include <GL/glew.h>
#include <wx/wxprec.h>
#include <wx/string.h>
#include <wx/textfile.h>

#include "../main.h"
#include "../Log.h"

Shader2::Shader2( wxString filename, SHADERTYPE type )
: m_filename( filename )
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
        Log::getInstance()->printDebug( _T( "Shader type not supported."), LOGLEVEL_ERROR );
    }
}

//////////////////////////////////////////////////////////////////////////

bool Shader2::load()
{
    m_oss << "Shader " << m_filename.char_str() << " starting to load...";
    Log::getInstance()->printDebug(wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
    m_oss.str( "" );

    if( fileExists() )
    {
        if ( loadFromFile( &m_code, m_filename ) )
        {
            m_oss << "Shader " << m_filename.char_str() << " finished loading.";
            Log::getInstance()->printDebug(wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
            m_oss.str( "" );
            return true;
        }

        m_oss << "Could not load file " << m_filename.char_str();
        Log::getInstance()->printDebug(wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_ERROR );
        m_oss.str( "" );
        return false;
    }

    m_oss << "File " << m_filename.char_str() << " not found.";
    Log::getInstance()->printDebug(wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_WARNING );
    m_oss.str( "" );
    return false;
}

//////////////////////////////////////////////////////////////////////////

bool Shader2::compile()
{
    GLuint *id = &m_id;

    m_oss << "Shader " << m_filename.char_str() << " starting to compile...";
    Log::getInstance()->printDebug(wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
    m_oss.str( "" );

    char *temp;
    temp = ( char* ) malloc( m_code.Length() + 1 );
    strcpy( temp, ( const char* ) m_code.mb_str( wxConvUTF8 ) );
    const char* code = temp;

    glShaderSource( *id, 1, &code, NULL );
    glCompileShader( *id );
    GLint compiled;
    glGetShaderiv( *id, GL_COMPILE_STATUS, &compiled );

    m_oss << "Shader " << m_filename.char_str() << " finished compiling.";
    Log::getInstance()->printDebug(wxString( m_oss.str().c_str(), wxConvUTF8 ), LOGLEVEL_DEBUG );
    m_oss.str( "" );

    free( temp );
    return 0 != compiled; // removes casting warning
}

//////////////////////////////////////////////////////////////////////////

const wxString & Shader2::getFilename()
{
    return m_filename;
}

//////////////////////////////////////////////////////////////////////////

const GLuint & Shader2::getId()
{
    return m_id;
}

//////////////////////////////////////////////////////////////////////////

bool Shader2::loadFromFile(wxString *code, const wxString &filename)
{
    wxTextFile file;
    *code = wxT("");

    if (file.Open(MyApp::shaderPath + filename))
    {
        for( size_t i( 0 ); i < file.GetLineCount(); ++i )
        {
            if( file.GetLine( i ).BeforeFirst( ' ' ) == wxT( "#include" ) )
            {
                wxString include = wxT( "" );
                loadFromFile(&include, file.GetLine( i ).AfterFirst( ' ' ));
                *code += include;
            }
            else
            {
                wxString line = file.GetLine(i);
                line.Trim();
                line.Trim( true );

                for( unsigned int j( 0 ); j < line.Length(); ++j )
                {
                    if( line.GetChar( j ) == '/' && line.GetChar( j+1 ) == '/' )
                    {
                        for( unsigned int k( j ); k < line.Length(); ++k )
                        {
                            if( line.GetChar( k ) == '*' && line.GetChar( k+1 ) == '/' )
                            {
                                *code += _T( "*/" );
                            }
                        }
                        break;
                    }
                    *code += line.GetChar( j );
                }
            }
        }
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

bool Shader2::fileExists()
{
    bool result;
    ifstream file;
    file.open(MyApp::shaderPath + m_filename); 
    result = file.fail() ? false : true;
    file.close();
    return result;
}

//////////////////////////////////////////////////////////////////////////

Shader2::~Shader2()
{
    if( 0 != m_id )
    {
        glDeleteShader( m_id );
        m_id = 0;
    }
}




