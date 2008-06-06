#include "GLSLShaderProgram.h"
#include "GLSLShader.h"

FGLSLShaderProgram::FGLSLShaderProgram()
{
  m_shaderProgram = glCreateProgram();
  m_vertex = 0;
  m_fragment = 0;
}

FGLSLShaderProgram::~FGLSLShaderProgram()
{
  // if shaderProgram is zero, we have nothing allocated
  // and are done here
  if(m_shaderProgram == 0) return;
  unlink();
  m_shaderProgram = 0;
}

bool FGLSLShaderProgram::link( GLSLShader *vertex, GLSLShader *fragment)
{
	m_vertex = vertex;
	m_fragment = fragment;
	glAttachShader(m_shaderProgram, m_vertex->getShaderID());
	glAttachShader(m_shaderProgram, m_fragment->getShaderID());
	glDeleteShader( m_vertex->getShaderID());
	glDeleteShader( m_fragment->getShaderID());
	glLinkProgram(m_shaderProgram);
	GLint linked;
	glGetProgramiv (m_shaderProgram, GL_LINK_STATUS, &linked);
	if (!linked) {
		printCompilerLog(m_shaderProgram);
		return false;
	}
	return true;
}

void FGLSLShaderProgram::unlink()
{
	glDeleteProgram(m_shaderProgram);
}

void FGLSLShaderProgram::bind()
{
	glUseProgram(m_shaderProgram);
	//printf("using shader: %d\n", m_shaderProgram);
}

void FGLSLShaderProgram::release()
{
	glUseProgram(0);
}

void FGLSLShaderProgram::printCompilerLog(GLuint program)
{
	int infologLen = 0;
	int charsWritten = 0;
	GLchar *infoLog;
	
	glGetProgramiv (program, GL_INFO_LOG_LENGTH, &infologLen);
	if (infologLen > 0)
	{
		infoLog = (GLchar*) malloc (infologLen);
		glGetProgramInfoLog(program, infologLen, &charsWritten, infoLog);
		printf("Program Log:\n%s\n\n", infoLog);
		free (infoLog);
	}
}

GLuint FGLSLShaderProgram::getProgramObject()
{
	return m_shaderProgram;
}
