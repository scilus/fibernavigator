#ifndef SHADER_H_
#define SHADER_H_

#include <sstream>

#include <GL/glew.h>
#include <wx/string.h>

enum SHADERTYPE { SHADERTYPE_VERTEX, SHADERTYPE_GEOMETRY, SHADERTYPE_FRAGMENT };

class Shader2
{
public:
    Shader2( wxString filename, SHADERTYPE type );
    ~Shader2();

    bool load();
    bool compile();

    const wxString & getFilename();
    const GLuint & getId();

private:
    bool loadFromFile( wxString *code, const wxString &filename );
    bool fileExists();
    
    GLuint m_id;
    wxString m_filename;
    wxString m_code;
    std::ostringstream m_oss;
};

#endif // SHADER_H_