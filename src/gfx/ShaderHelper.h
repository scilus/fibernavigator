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
    ~ShaderHelper();

    static ShaderHelper * getInstance();

    void loadShaders( bool useGeometryShaders );

    void initializeArrays();

    ShaderProgram * getAnatomyShader()        { return m_pAnatomyShader; }
    ShaderProgram * getMeshShader()           { return m_pMeshShader; }
    ShaderProgram * getFibersShader()         { return m_pFibersShader; }
    ShaderProgram * getFakeTubesShader()      { return m_pFakeTubesShader; }
    ShaderProgram * getCrossingFibersShader() { return m_pCrossingFibersShader; }
    ShaderProgram * getSplineSurfShader()     { return m_pSplineSurfShader; }
    ShaderProgram * getVectorShader()         { return m_pVectorShader; }
    ShaderProgram * getLegendShader()         { return m_pLegendShader; }
    ShaderProgram * getGraphShader()          { return m_pGraphShader; }
    ShaderProgram * getTensorsShader()        { return m_pTensorsShader; }
    ShaderProgram * getOdfsShader()           { return m_pOdfsShader; }
    ShaderProgram * getRTTShader()            { return m_pRTTShader; }

    void setTextureShaderVars();
    void setMeshShaderVars();
    void setFiberShaderVars();
    void setSplineSurfaceShaderVars();

protected:
    ShaderHelper(void);

private:
    ShaderHelper(const ShaderHelper &);
    ShaderHelper &operator=(const ShaderHelper &);

private:
    static ShaderHelper *m_pInstance;

    int m_textureCount;
    GLuint m_cutTex;
    std::vector<GLint>   m_tex;
    std::vector<GLfloat> m_threshold;
    std::vector<GLfloat> m_alpha;
    std::vector<GLint>   m_type;

    ShaderProgram *m_pAnatomyShader;
    ShaderProgram *m_pMeshShader;
    ShaderProgram *m_pFibersShader;
    ShaderProgram *m_pFakeTubesShader;
    ShaderProgram *m_pCrossingFibersShader;
    ShaderProgram *m_pSplineSurfShader;
    ShaderProgram *m_pVectorShader;
    ShaderProgram *m_pLegendShader;
    ShaderProgram *m_pGraphShader;
    ShaderProgram *m_pTensorsShader;
    ShaderProgram *m_pOdfsShader;
    ShaderProgram *m_pRTTShader;
};

#endif /* SHADERHELPER_H_ */
