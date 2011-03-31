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
#include "../dataset/DatasetHelper.h"

class ShaderHelper {
public:
	ShaderHelper(DatasetHelper*);
	virtual ~ShaderHelper();

	void initializeArrays();

	void setTextureShaderVars();
	void setMeshShaderVars();
	void setFiberShaderVars();
	void setSplineSurfaceShaderVars();

	Shader *m_textureShader;
	Shader *m_meshShader;
	Shader *m_fiberShader;
	Shader *m_fakeTubeShader;
	Shader *m_splineSurfShader;
	Shader *m_vectorShader;
	Shader *m_legendShader;
	Shader *m_graphShader;
    Shader *m_tensorsShader;
    Shader *m_odfsShader;

private:
	DatasetHelper* m_dh;

	int m_textureCount;
	GLuint m_cutTex;
	std::vector<GLint> m_tex;
	std::vector<GLfloat> m_threshold;
	std::vector<GLfloat> m_alpha;
    std::vector<GLint> m_type;
};

#endif /* SHADERHELPER_H_ */
