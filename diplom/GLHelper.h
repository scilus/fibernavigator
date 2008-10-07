/*
 * GLHelper.h
 *
 *  Created on: 07.10.2008
 *      Author: ralph
 */

#ifndef GLHELPER_H_
#define GLHELPER_H_

#include <GL/glew.h>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class GLHelper {
public:
	GLHelper();
	virtual ~GLHelper();

	/*
	 * Helper functions
	 */
	void printTime();
	void printwxT(wxString);
	/*
	 * Check for GL error
	 */
	bool GLError();
	void printGLError(wxString function = wxT(""));

private:
	GLenum lastGLError;
};

#endif /* GLHELPER_H_ */
