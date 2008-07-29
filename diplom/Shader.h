/*
 * Shader.h
 *
 *  Created on: 29.07.2008
 *      Author: ralph
 */

#ifndef SHADER_H_
#define SHADER_H_

#include <GL/glew.h>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/textfile.h"

class Shader {
public:
    /**
     * Create an empty shader object
     */
    Shader(wxString);

    /**
     * Destroy shader object, unlinks it, if needed
     */
    ~Shader();

    /**
     * link a vertex and a fragement program to the shader
     */
    bool link();

    /**
    * unlink
    */
    void unlink();

    /**
     * Bind a program, so it is used for opengl rendering
     */
    void bind();

    /**
     * Release the program, so it is no longer used for rendering
     */
    void release();

    /**
     * get program object
     */
    GLuint getProgramObject() {return m_shaderProgram;};


    void setUniInt(const GLchar*, int);
    void setUniFloat(const GLchar*, float);
    void setUniArrayInt(const GLchar*, int*, int);
    void setUniArrayFloat(const GLchar*, float*, int);
    void setAttribFloat(const GLchar*, float);


private:
    GLuint m_shaderProgram;
    GLuint m_vertex;
    GLuint m_fragment;

    /**
     * compile a vertex or fragment shader program
     */
    bool compile(GLuint*, wxString);

	void printCompilerLog(GLuint);
	void printProgramLog(GLuint);


	bool loadCode (wxString, wxString modules = wxT(""));
	bool loadFromFile(wxString*, wxString);
};

#endif /* SHADER_H_ */
