/*
 * Shader.cpp
 *
 *  Created on: 29.07.2008
 *      Author: ralph
 */

#include "Shader.h"
#include "../main.h"

Shader::Shader(wxString fileName)
:   m_shaderProgram(0),
    m_vertex(0),
    m_fragment(0)
{
    try
    {
        m_vertex = glCreateShader(GL_VERTEX_SHADER);
        m_fragment = glCreateShader(GL_FRAGMENT_SHADER);
        m_shaderProgram = glCreateProgram();
        if (loadCode(fileName))
        {   
            link();
        }
    }
    catch (...)
    {
        printf("***error while creating shader\n");
        printf("***please check your OpenGL installation\n");
        exit(false);
    }
}

Shader::~Shader() {
    // if shaderProgram is zero, we have nothing allocated
    // and are done here
    if(m_shaderProgram == 0) return;
    unlink();
    m_shaderProgram = 0;
}

bool Shader::link()
{
    glAttachShader(m_shaderProgram, m_vertex);
    glAttachShader(m_shaderProgram, m_fragment);
    glDeleteShader( m_vertex);
    glDeleteShader( m_fragment);
    glLinkProgram(m_shaderProgram);
    GLint linked;
    glGetProgramiv (m_shaderProgram, GL_LINK_STATUS, &linked);
    printProgramLog(m_shaderProgram);
    if (!linked) {
        return false;
    }
    return true;
}

void Shader::unlink()
{
    glDeleteProgram(m_shaderProgram);
}

void Shader::bind()
{
    glUseProgram(m_shaderProgram);
}

void Shader::release()
{
    glUseProgram(0);
}
 
bool Shader::compile(GLuint* shaderId, wxString codeString)
{
    char *temp;
    temp = (char*) malloc(codeString.Length()+1);
    strcpy(temp, (const char*)codeString.mb_str(wxConvUTF8));
    const char* code = temp;

    glShaderSource (*shaderId, 1, &code, NULL);
    glCompileShader(*shaderId);
    GLint compiled;
    glGetShaderiv (*shaderId, GL_COMPILE_STATUS, &compiled);

	free(temp);
    return compiled;
}

bool Shader::loadCode(wxString filename)
{
    wxString codeStringVS;
    wxString codeStringFS;
    if (!loadFromFile(&codeStringVS, filename + wxT(".vs")))
    {
        printf("ERROR: vertex shader file not found!\n");
        return false;
    }
    //codeString = modules + wxT(" ") + codeString;

    if (!loadFromFile(&codeStringFS, filename + wxT(".fs")))
    {
        printf("ERROR: fragment shader file not found!\n");
        return false;
    }
    if (!compile(&m_vertex, codeStringVS))
    {
        printf("ERROR in vertex shader\n");
        printCompilerLog(m_vertex);
        return false;
    }
    if (!compile(&m_fragment, codeStringFS))
    {
        printf("ERROR in fragment shader\n");
        printCompilerLog(m_fragment);
        return false;
    }
    return true;
}

bool Shader::loadFromFile(wxString* code, wxString fileName)
{
    wxTextFile file;
    *code = wxT("");
    if (file.Open(MyApp::shaderPath + fileName))
    {
        size_t i;
        for (i = 0 ; i < file.GetLineCount() ; ++i)
        {
            if (file.GetLine(i).BeforeFirst(' ') == wxT("#include"))
            {
                wxString include = wxT("");
                loadFromFile(&include, file.GetLine(i).AfterFirst(' '));
                *code += include;
            }
            else
            {
                wxString line = file.GetLine(i);
                line.Trim();
                line.Trim(true);
                for (unsigned int j = 0 ; j < line.Length() ; ++j )
                {
                    if (line.GetChar(j) == '/' && line.GetChar(j+1) == '/')
                    {
                        for (unsigned int k = j ; k < line.Length() ; ++k )
                        {
                            if (line.GetChar(k) == '*' && line.GetChar(k+1) == '/')
                                *code += _T("*/");
                        }
                        break;
                    }
                    *code += line.GetChar(j);
                }
            }
        }
        return true;
    }
    return false;
}

void Shader::printCompilerLog(GLuint shader)
{
    GLint infologLen = 0;
    GLint charsWritten = 0;
    GLchar *infoLog;

    glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &infologLen);
    if (infologLen > 1)
    {
        infoLog = (GLchar*) malloc (infologLen);
        glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
        printf("Compiler Log:\n%s\n===========================================================\n", infoLog);
        free (infoLog);
    }
}

void Shader::printProgramLog(GLuint program)
{
    GLint infologLen = 0;
    GLint charsWritten = 0;
    GLchar *infoLog;

    glGetProgramiv (program, GL_INFO_LOG_LENGTH, &infologLen);
    if (infologLen > 1)
    {
        infoLog = (GLchar*) malloc (infologLen);
        glGetProgramInfoLog(program, infologLen, &charsWritten, infoLog);
        printf("Program Log:\n%s\n===========================================================\n", infoLog);
        free (infoLog);
    }
}

void Shader::printwxT(wxString string)
{
    char* cstring;
    cstring = (char*)malloc(string.length());
    strcpy(cstring, (const char*)string.mb_str(wxConvUTF8));
    printf("%s", cstring);
}

void Shader::setUniInt(const GLchar* name, const GLint value)
{
    GLint texLoc = glGetUniformLocation (m_shaderProgram, name);
    if (texLoc == -1)
    {
        printf("No such uniform named \"%s\"\n", name);
    }
    glUniform1i (texLoc, value);
}

void Shader::setUniFloat(const GLchar* name, const GLfloat value)
{
    GLint texLoc = glGetUniformLocation (m_shaderProgram, name);
    if (texLoc == -1)
    {
        printf("No such uniform named \"%s\"\n", name);
    }
    glUniform1f (texLoc, value);
}

void Shader::setUniArrayInt(const GLchar* name, GLint* value, const GLint size)
{
    GLint texLoc = glGetUniformLocation (m_shaderProgram, name);
    glUniform1iv (texLoc, size, value);
}

void Shader::setUni3Int( const GLchar* name, GLint values[3] )
{
    GLint texLoc = glGetUniformLocation( m_shaderProgram, name );
    glUniform3i( texLoc, values[0], values[1], values[2] );
}

void Shader::setUniArray1Float(const GLchar* name, GLfloat* value, const int size)
{
    GLint texLoc = glGetUniformLocation (m_shaderProgram, name);
    glUniform1fv (texLoc, size, value);
}

void Shader::setUni2Float( const GLchar* name, std::pair< GLfloat, GLfloat > values )
{
    GLint texLoc = glGetUniformLocation( m_shaderProgram, name );
    glUniform2f( texLoc, values.first, values.second );
}

void Shader::setUni3Float( const GLchar* name, GLfloat values[3] )
{
    GLint texLoc = glGetUniformLocation( m_shaderProgram, name );
    glUniform3f( texLoc, values[0], values[1], values[2] );
}

void Shader::setUniMatrix3f( const GLchar* name, const FMatrix &values )
{
    float* l_valueCast = new float[9];

    l_valueCast[0] = values(0,0);
    l_valueCast[1] = values(0,1);
    l_valueCast[2] = values(0,2);
    l_valueCast[3] = values(1,0);
    l_valueCast[4] = values(1,1);
    l_valueCast[5] = values(1,2);
    l_valueCast[6] = values(2,0);
    l_valueCast[7] = values(2,1);
    l_valueCast[8] = values(2,2);
    
    GLint texLoc = glGetUniformLocation( m_shaderProgram, name );
    glUniformMatrix3fv( texLoc, 1, GL_TRUE, l_valueCast );

    delete[] l_valueCast;
}

void Shader::setAttribFloat(const GLchar* name, const GLfloat value)
{
    GLint attrib = glGetAttribLocation (m_shaderProgram, name);
    //if (attrib == -1)
    //    printf("No such attribute named \"%s\"\n", name);
    glVertexAttrib1f (attrib, value);
}

void Shader::setUniSampler( const GLchar* name, const GLint value )
{
    GLint location = glGetUniformLocation( m_shaderProgram, name );
    glUniform1i( location, value );
}
