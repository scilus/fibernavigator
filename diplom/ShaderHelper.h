/*
 * ShaderHelper.h
 *
 *  Created on: 28.07.2008
 *      Author: ralph
 */

#ifndef SHADERHELPER_H_
#define SHADERHELPER_H_

//#include "GLSL/GLSLShaderProgram.h"
#include "Shader.h"
#include "DatasetHelper.h"

class ShaderHelper {
public:
	ShaderHelper(DatasetHelper*);
	virtual ~ShaderHelper();

	void setTextureShaderVars();
	void setMeshShaderVars();
	void setFiberShaderVars();
	void setSplineSurfaceShaderVars();

	Shader *m_textureShader;
	Shader *m_meshShader;
	Shader *m_fiberShader;
	Shader *m_fakeTubeShader;
	Shader *m_splineSurfShader;

private:
	DatasetHelper* m_dh;
};

#endif /* SHADERHELPER_H_ */
