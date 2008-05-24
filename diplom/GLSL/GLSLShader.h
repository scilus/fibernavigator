#ifndef _GLSLSHADER_H_
#define _GLSLSHADER_H_

#include <GL/gl.h>
#include <GL/glu.h>

#include "wx/wx.h"
#include "wx/textfile.h"

class GLSLShader
{
  public:
     GLSLShader(GLenum target);
    ~GLSLShader();

    void bind() const;
    void release() const;

    void destroy();
   
    bool loadCode (const GLchar**);
    bool loadCode (wxString);
    bool loadFromFile(wxString);

    //bool loadProgram(const std::string &programString) const;

    GLuint getShaderID() const;
    void setShaderID(GLuint programID);

    
  private:
    GLenum	m_target;
    GLuint	m_shaderID;
    wxString m_codeString;
    const char* m_code;

    void printCompilerLog(GLuint) ;
    void printCode() const;
};

#endif /* _SHADER_H_ */

