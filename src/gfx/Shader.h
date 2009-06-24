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


    void setUniInt(const GLchar* name, const GLint value);
    void setUniFloat(const GLchar* name, const GLfloat value);
    void setUniArrayInt(const GLchar* name, GLint* array, const int size);
    void setUniArrayFloat(const GLchar* name, GLfloat* array, const int size);
    void setAttribFloat(const GLchar* name , const GLfloat value);


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
	void printwxT(wxString);


	bool loadCode (wxString);
	bool loadFromFile(wxString*, wxString);
};

#endif /* SHADER_H_ */
