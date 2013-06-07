#ifndef SHADER_H_
#define SHADER_H_

#include <sstream>
#include <string>

#include <GL/glew.h>
#include <wx/string.h>

enum SHADERTYPE { SHADERTYPE_VERTEX, SHADERTYPE_GEOMETRY, SHADERTYPE_FRAGMENT };

class Shader
{
public:
    Shader( wxString filename, SHADERTYPE type );
    ~Shader();

    bool load();
    bool compile();

    const wxString & getFilename();
    const GLuint & getId();
    
    std::string getStdStringFilename();

private:
    bool loadFromFile( wxString *code, const wxString &filename );
    bool fileExists();

private:
    GLuint m_id;
    wxString m_filename;
    wxString m_code;
    std::ostringstream m_oss;
};

#endif // SHADER_H_