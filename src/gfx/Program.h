#ifndef PROGRAM_H_
#define PROGRAM_H_

#include <sstream>

#include <GL/glew.h>
#include <wx/string.h>

#include "Shader2.h"
#include "../misc/Fantom/FMatrix.h"

/*
 * Class that represents a shader program.
 */
class Program
{
public:
    Program(wxString name, bool useGeometryShader = false );
    ~Program();

    // Loads the various shaders from their respective files
    bool load();
    // Compiles and links the shaders that form the program
    bool compileAndLink();
    // Binds the program to opengl
    void bind();


    void setUniInt        ( const GLchar* name, const GLint value );
    void setUniFloat      ( const GLchar* name, const GLfloat value );
    void setUniArrayInt   ( const GLchar* name, GLint* array, const int size );
    void setUni3Int       ( const GLchar* name, GLint values[3] );
    void setUniArray1Float( const GLchar* name, GLfloat* array, const int size );
    void setUni2Float     ( const GLchar* name, std::pair< GLfloat, GLfloat > values );
    void setUni3Float     ( const GLchar* name, GLfloat values[3] );
    void setAttribFloat   ( const GLchar* name, const GLfloat value );
    void setUniMatrix3f   ( const GLchar* name, const FMatrix &values );
    void setUniSampler    ( const GLchar* name, const GLint value );

    
private:
    void printCompilerLog( GLuint shaderId );
    void printProgramLog( GLuint id );

    GLuint m_id;
    bool m_useGeometry;
    wxString  m_name;
    Shader2 *m_pVertex;
    Shader2 *m_pGeometry;
    Shader2 *m_pFragment;
    std::ostringstream m_oss;
    
};

#endif PROGRAM_H_