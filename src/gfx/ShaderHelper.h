/*
 * ShaderHelper.h
 *
 *  Created on: 28.07.2008
 *      Author: ralph
 */

#ifndef SHADERHELPER_H_
#define SHADERHELPER_H_

#include "Shader.h"
#include "../dataset/DatasetHelper.h"

#include <vector>

class ShaderHelper {
public:
    ShaderHelper(DatasetHelper* pDh);
    virtual ~ShaderHelper();

    void initializeArrays();

    void setTextureShaderVars();
    void setMeshShaderVars();
    void setFiberShaderVars();
    void setSplineSurfaceShaderVars();

    Shader *m_pTextureShader;
    Shader *m_pMeshShader;
    Shader *m_pFiberShader;
    Shader *m_pFakeTubeShader;
    Shader *m_pSplineSurfShader;
    Shader *m_pVectorShader;
    Shader *m_pLegendShader;
    Shader *m_pGraphShader;
    Shader *m_pTensorsShader;
    Shader *m_pOdfsShader;

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
