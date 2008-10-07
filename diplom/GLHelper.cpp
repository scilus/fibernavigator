/*
 * GLHelper.cpp
 *
 *  Created on: 07.10.2008
 *      Author: ralph
 */

#include "GLHelper.h"

GLHelper::GLHelper() {
	// TODO Auto-generated constructor stub

}

GLHelper::~GLHelper() {
	// TODO Auto-generated destructor stub
}

bool GLHelper::GLError()
{
	lastGLError = glGetError();
	if (lastGLError == GL_NO_ERROR) return false;
	return true;
}

void GLHelper::printGLError(wxString function)
{
	printwxT(function);
	printf(" : ERROR: %s\n", gluErrorString(lastGLError));
}

void GLHelper::printTime()
{
	wxDateTime dt = wxDateTime::Now();
	printf("[%02d:%02d:%02d] ",dt.GetHour(), dt.GetMinute(), dt.GetSecond());
}

void GLHelper::printwxT(wxString string)
{
	char* cstring;
	cstring = (char*)malloc(string.length());
	strcpy(cstring, (const char*)string.mb_str(wxConvUTF8));
	printf("%s", cstring);
}
