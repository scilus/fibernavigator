#ifndef SHADERPROGRAM_H_
#define SHADERPROGRAM_H_

#include <sstream>
#include <string>

#include <GL/glew.h>
#include <wx/string.h>

#include "Shader.h"
#include "../misc/Fantom/FMatrix.h"

/*
 * Class that represents a shader program.
 */
class ShaderProgram
{
public:
    ShaderProgram( wxString name, bool useGeometryShader = false, bool geometryShadersSupported = false );
    ~ShaderProgram();

    // Loads the various shaders from their respective files
    bool load();
    // Compiles and links the shaders that form the program
    bool compileAndLink();
    // Binds the program to opengl
    void bind();
    // Releases the program from opengl
    void release();

    GLuint getId();

    void setUniInt        ( const GLchar* pName, const GLint value );
    void setUniFloat      ( const GLchar* pName, const GLfloat value );
    void setUniArrayInt   ( const GLchar* pName, GLint* pArray, const int size );
    void setUni3Int       ( const GLchar* pName, GLint values[3] );
    void setUniArray1Float( const GLchar* pName, GLfloat* pArray, const int size );
    void setUni2Float     ( const GLchar* pName, std::pair< GLfloat, GLfloat > values );
    void setUni3Float     ( const GLchar* pName, GLfloat values[3] );
    void setAttribFloat   ( const GLchar* pName, const GLfloat value );
    void setUniMatrix3f   ( const GLchar* pName, const FMatrix &values );
    void setUniSampler    ( const GLchar* pName, const GLint value );
    void setUniBool       ( const GLchar* pName, const bool value );
    
private:
    ShaderProgram(const ShaderProgram &);
    ShaderProgram &operator=(const ShaderProgram &);

    void printCompilerLog( GLuint shaderId );
    void printProgramLog( GLuint programId );
    
    std::string getStdStringName();

    GLuint m_id;
    wxString  m_name;
    Shader *m_pVertex;
#if _COMPILE_GEO_SHADERS
    Shader *m_pGeometry;
#endif
    Shader *m_pFragment;
    bool m_useGeometry;
    bool m_geometrySupported;
    std::ostringstream m_oss;
};

#endif // SHADERPROGRAM_H_