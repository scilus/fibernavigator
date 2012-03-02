/*
 * ShaderHelper.h
 *
 *  Created on: 28.07.2008
 *      Author: ralph
 */

#ifndef SHADERHELPER_H_
#define SHADERHELPER_H_

#include "ShaderProgram.h"
#include "../dataset/DatasetHelper.h"

#include <vector>

class ShaderHelper {
public:
    ShaderHelper( DatasetHelper* pDh, bool useGeometryShaders );
    virtual ~ShaderHelper();

    void initializeArrays();

    void setTextureShaderVars();
    void setMeshShaderVars();
    void setFiberShaderVars();
    void setSplineSurfaceShaderVars();

    ShaderProgram m_anatomyShader;
    ShaderProgram m_meshShader;
    ShaderProgram m_fibersShader;
    ShaderProgram m_fakeTubesShader;
    ShaderProgram m_crossingFibersShader;
    ShaderProgram m_splineSurfShader;
    ShaderProgram m_vectorShader;
    ShaderProgram m_legendShader;
    ShaderProgram m_graphShader;
    ShaderProgram m_tensorsShader;
    ShaderProgram m_odfsShader;
	ShaderProgram m_RTTShader;

private:
    DatasetHelper* m_pDh;

    int m_textureCount;
    GLuint m_cutTex;
    std::vector<GLint>   m_tex;
    std::vector<GLfloat> m_threshold;
    std::vector<GLfloat> m_alpha;
    std::vector<GLint>   m_type;
};

#endif /* SHADERHELPER_H_ */
