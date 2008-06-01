#ifndef GLSL_SHADERPROGRAM_H
#define GLSL_SHADERPROGRAM_H

#include "GLSLShader.h"

class FGLSLShaderProgram
{
public:
    /**
     * Create an empty shader object
     */
    FGLSLShaderProgram();

    /**
     * Destroy shader object, unlinks it, if needed
     */
    ~FGLSLShaderProgram();

    /**
     * link a vertex and a fragement program to the shader
     */
    bool link( GLSLShader*, GLSLShader*);

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
    GLuint getProgramObject();

private:
    GLuint m_shaderProgram;
    GLSLShader *m_vertex;
    GLSLShader * m_fragment;

private:
  
	void printCompilerLog(GLuint); 
};

#endif

