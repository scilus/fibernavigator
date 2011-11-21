/*
 * ShaderHelper.h
 *
 *  Created on: 28.07.2008
 *      Author: ralph
 */

#ifndef SHADERHELPER_H_
#define SHADERHELPER_H_

#include "Program.h"
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

    Program m_anatomy;
    Program m_mesh;
    Program m_fibers;
    Program m_fakeTubes;
    Program m_splineSurf;
    Program m_vector;
    Program m_legend;
    Program m_graph;
    Program m_tensors;
    Program m_odfs;

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
