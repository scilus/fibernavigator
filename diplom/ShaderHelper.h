/*
 * ShaderHelper.h
 *
 *  Created on: 28.07.2008
 *      Author: ralph
 */

#ifndef SHADERHELPER_H_
#define SHADERHELPER_H_

#include "GLSL/GLSLShaderProgram.h"
#include "DatasetHelper.h"

class ShaderHelper {
public:
	ShaderHelper(DatasetHelper*);
	virtual ~ShaderHelper();

	void setTextureShaderVars();
	void setMeshShaderVars();

	FGLSLShaderProgram *m_textureShader;
	FGLSLShaderProgram *m_meshShader;
	FGLSLShaderProgram *m_curveShader;

private:
	DatasetHelper* m_dh;
};

#endif /* SHADERHELPER_H_ */
