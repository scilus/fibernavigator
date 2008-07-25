#include "GLSLShader.h"

GLSLShader::GLSLShader(GLenum target)
{
  m_target = target;
  m_shaderID = 0;
  m_shaderID = glCreateShader(target);

  if (glewIsSupported("GL_VERTEX_SHADER"))
  {
	  //m_shaderID = glCreateShader(target);
  }
  else
  {
	  //printf("shader not supported\n");
  }

}

GLSLShader::~GLSLShader()
{
  destroy();
}

void GLSLShader::bind() const
{

}

void GLSLShader::release() const
{

}

void GLSLShader::destroy()
{

}

bool GLSLShader::loadCode(const GLchar** source)
{
	glShaderSource (m_shaderID, 1, source, NULL);
	glCompileShader(m_shaderID);
	GLint compiled;
	glGetShaderiv (m_shaderID, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		printCompilerLog(m_shaderID);
		return false;
	}
	return true;
}

bool GLSLShader::loadCode(wxString filename, wxString modules)
{
	wxString codeString;
	if (!loadFromFile(&codeString, filename))
	{
		printf("ERROR: Shader file not found!\n");
		return false;
	}
	codeString = modules + wxT(" ") + codeString;


	char temp[codeString.Length()];
	strcpy(temp, (const char*)codeString.mb_str(wxConvUTF8));
	const char* code = temp;

	glShaderSource (m_shaderID, 1, &code, NULL);
	glCompileShader(m_shaderID);
	GLint compiled;
	glGetShaderiv (m_shaderID, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		printCompilerLog(m_shaderID);
		return false;
	}
	return true;
}

GLuint GLSLShader::getShaderID() const
{
  return m_shaderID;
}

void GLSLShader::setShaderID(GLuint shaderID)
{
  m_shaderID = shaderID;
}


void GLSLShader::printCompilerLog(GLuint shader)
{
	int infologLen = 0;
	int charsWritten = 0;
	GLchar *infoLog;

	glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &infologLen);
	if (infologLen > 0)
	{
		infoLog = (GLchar*) malloc (infologLen);
		glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
		printf("InfoLog:\n%s\n\n", infoLog);
		free (infoLog);
	}
}

void GLSLShader::printCode() const
{

}

bool GLSLShader::loadFromFile(wxString* code, wxString fileName)
{
	wxTextFile file;
	*code = wxT("");
	if (file.Open(fileName))
	{
		size_t i;
		for (i = 0 ; i < file.GetLineCount() ; ++i)
		{
			*code += file.GetLine(i);
		}
		return true;
	}
	return false;
}
