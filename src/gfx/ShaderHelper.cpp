/*
 * ShaderHelper.cpp
 *
 *  Created on: 28.07.2008
 *      Author: ralph
 */

#include "ShaderHelper.h"
#include "../dataset/DatasetHelper.h"
#include "../dataset/Surface.h"

ShaderHelper::ShaderHelper( DatasetHelper* pDh ) :
    m_pDh( pDh )
{
    m_tex.resize( 10 );
    m_type.resize( 10 );
    m_threshold.resize( 10 );
    m_alpha.resize( 10 );

    m_pDh->printDebug( _T("initializing texture shader"), 1 );
    m_pTextureShader = new Shader( wxT("anatomy") );
    m_pTextureShader->bind();
    if ( m_pDh->GLError() )
        m_pDh->printGLError( wxT("setup anatomy shader") );

    m_pDh->printDebug( _T("initializing mesh shader"), 1 );
    m_pMeshShader = new Shader( wxT("mesh") );
    m_pMeshShader->bind();
    if ( m_pDh->GLError() )
        m_pDh->printGLError( wxT("setup mesh shader") );

    m_pDh->printDebug( _T("initializing fiber shader"), 1 );
    m_pFiberShader = new Shader( wxT("fibers") );
    m_pFiberShader->bind();
    if ( m_pDh->GLError() )
        m_pDh->printGLError( wxT("setup fiber shader") );

    m_pDh->printDebug( _T("initializing fake tube shader"), 1 );
    m_pFakeTubeShader = new Shader( wxT("fake-tubes") );
    m_pFakeTubeShader->bind();
    if ( m_pDh->GLError() )
        m_pDh->printGLError( wxT("setup fake tube shader") );

    m_pDh->printDebug( _T("initializing spline surface shader"), 1 );
    m_pSplineSurfShader = new Shader( wxT("splineSurf") );
    m_pSplineSurfShader->bind();
    if ( m_pDh->GLError() )
        m_pDh->printGLError( wxT("setup spline surface shader") );
/*
    m_pDh->printDebug( _T("initializing vector shader"), 1 );
    m_pVectorShader = new Shader( wxT("vectors") );
    m_pVectorShader->bind();
    if ( m_pDh->GLError() )
        m_pDh->printGLError( wxT("setup vectors shader") );
*/
    m_pDh->printDebug( _T("initializing legend shader"), 1 );
    m_pLegendShader = new Shader( wxT("legend") );
    m_pLegendShader->bind();
    if ( m_pDh->GLError() )
        m_pDh->printGLError( wxT("setup legend shader") );

    m_pDh->printDebug( _T("initializing graph shader"), 1 );
    m_pGraphShader = new Shader( wxT("graph") );
    m_pGraphShader->bind();
    if ( m_pDh->GLError() )
        m_pDh->printGLError( wxT("setup graph shader") );

    m_pDh->printDebug( _T("initializing tensors shader"), 1 );
    m_pTensorsShader = new Shader( wxT("tensors") );
    m_pTensorsShader->bind();
    if ( m_pDh->GLError() )
        m_pDh->printGLError( wxT("setup tensors shader") );

    m_pDh->printDebug( _T("initializing odfs shader"), 1 );
    m_pOdfsShader = new Shader( wxT("odfs") );
    m_pOdfsShader->bind();
    if ( m_pDh->GLError() )
        m_pDh->printGLError( wxT("setup odfs shader") );
}

ShaderHelper::~ShaderHelper()
{
    m_pDh->printDebug( _T("execute shader helper destructor\n"), 0 );
    delete m_pTextureShader;
    delete m_pMeshShader;
    delete m_pFiberShader;
    delete m_pSplineSurfShader;
    delete m_pFakeTubeShader;
    delete m_pTensorsShader;
    delete m_pOdfsShader;
	delete m_pGraphShader;
	delete m_pLegendShader;

    m_pDh->printDebug( _T("shader helper destructor done\n"), 0 );
}

void ShaderHelper::initializeArrays()
{
    m_textureCount = 0;
    m_cutTex = 0;

    m_type.resize( 10, 0 );
    m_threshold.resize( 10, 0 );
    m_alpha.resize( 10, 0 );

    for ( int i = 0; i < m_pDh->m_mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* pInfo = (DatasetInfo*) m_pDh->m_mainFrame->m_listCtrl->GetItemData( i );
        if ( pInfo->getType() < MESH && pInfo->getShow() )
        {
            m_threshold[m_textureCount] = pInfo->getThreshold();
            m_alpha[m_textureCount] = pInfo->getAlpha();
            m_type[m_textureCount] = pInfo->getType();
            ++m_textureCount;
        }
        if ( m_textureCount == 10 )
            break;
    }
}

void ShaderHelper::setTextureShaderVars()
{
    m_pTextureShader->setUniInt( "tex0", 0 );
    m_pTextureShader->setUniInt( "tex1", 1 );
    m_pTextureShader->setUniInt( "tex2", 2 );
    m_pTextureShader->setUniInt( "tex3", 3 );
    m_pTextureShader->setUniInt( "tex4", 4 );
    m_pTextureShader->setUniInt( "tex5", 5 );
    m_pTextureShader->setUniInt( "tex6", 6 );
    m_pTextureShader->setUniInt( "tex7", 7 );
    m_pTextureShader->setUniInt( "tex8", 8 );
    m_pTextureShader->setUniInt( "tex9", 9 );

    m_pTextureShader->setUniInt( "type0", m_type[0] );
    m_pTextureShader->setUniInt( "type1", m_type[1] );
    m_pTextureShader->setUniInt( "type2", m_type[2] );
    m_pTextureShader->setUniInt( "type3", m_type[3] );
    m_pTextureShader->setUniInt( "type4", m_type[4] );
    m_pTextureShader->setUniInt( "type5", m_type[5] );
    m_pTextureShader->setUniInt( "type6", m_type[6] );
    m_pTextureShader->setUniInt( "type7", m_type[7] );
    m_pTextureShader->setUniInt( "type8", m_type[8] );
    m_pTextureShader->setUniInt( "type9", m_type[9] );

    m_pTextureShader->setUniFloat( "threshold0", m_threshold[0] );
    m_pTextureShader->setUniFloat( "threshold1", m_threshold[1] );
    m_pTextureShader->setUniFloat( "threshold2", m_threshold[2] );
    m_pTextureShader->setUniFloat( "threshold3", m_threshold[3] );
    m_pTextureShader->setUniFloat( "threshold4", m_threshold[4] );
    m_pTextureShader->setUniFloat( "threshold5", m_threshold[5] );
    m_pTextureShader->setUniFloat( "threshold6", m_threshold[6] );
    m_pTextureShader->setUniFloat( "threshold7", m_threshold[7] );
    m_pTextureShader->setUniFloat( "threshold8", m_threshold[8] );
    m_pTextureShader->setUniFloat( "threshold9", m_threshold[9] );

    m_pTextureShader->setUniFloat( "alpha0", m_alpha[0] );
    m_pTextureShader->setUniFloat( "alpha1", m_alpha[1] );
    m_pTextureShader->setUniFloat( "alpha2", m_alpha[2] );
    m_pTextureShader->setUniFloat( "alpha3", m_alpha[3] );
    m_pTextureShader->setUniFloat( "alpha4", m_alpha[4] );
    m_pTextureShader->setUniFloat( "alpha5", m_alpha[5] );
    m_pTextureShader->setUniFloat( "alpha6", m_alpha[6] );
    m_pTextureShader->setUniFloat( "alpha7", m_alpha[7] );
    m_pTextureShader->setUniFloat( "alpha8", m_alpha[8] );
    m_pTextureShader->setUniFloat( "alpha9", m_alpha[9] );
}

void ShaderHelper::setMeshShaderVars()
{
    m_pMeshShader->setUniInt( "blendTex", m_pDh->m_blendTexOnMesh );

    m_pMeshShader->setUniInt( "cutAtSurface", m_pDh->m_surfaceLoaded );
    m_pMeshShader->setUniInt( "lightOn", m_pDh->m_lighting );
    //m_pMeshShader->setUniInt( "useColorMap", m_pDh->m_colorMap );

    m_pMeshShader->setUniInt( "dimX", m_pDh->m_columns );
    m_pMeshShader->setUniInt( "dimY", m_pDh->m_rows );
    m_pMeshShader->setUniInt( "dimZ", m_pDh->m_frames );
    m_pMeshShader->setUniFloat( "voxX", m_pDh->m_xVoxel );
    m_pMeshShader->setUniFloat( "voxY", m_pDh->m_yVoxel );
    m_pMeshShader->setUniFloat( "voxZ", m_pDh->m_zVoxel );

    m_pMeshShader->setUniInt( "sector", m_pDh->m_quadrant );
    m_pMeshShader->setUniFloat( "cutX", m_pDh->m_xSlize + 0.5f );
    m_pMeshShader->setUniFloat( "cutY", m_pDh->m_ySlize + 0.5f );
    m_pMeshShader->setUniFloat( "cutZ", m_pDh->m_zSlize + 0.5f );


    for ( int i = 0; i < m_pDh->m_mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* pInfo = (DatasetInfo*) m_pDh->m_mainFrame->m_listCtrl->GetItemData( i );

        if ( pInfo->getType() == SURFACE )
        {
            Surface* pS = (Surface*) m_pDh->m_mainFrame->m_listCtrl->GetItemData( i );
            m_cutTex = pS->getCutTex();

            glActiveTexture( GL_TEXTURE0 + 9 );
            glBindTexture( GL_TEXTURE_2D, m_cutTex );
            m_tex[9] = 9;
            m_threshold[9] = 0;
            m_type[9] = 5;
        }
    }
    m_pMeshShader->setUniInt( "cutTex", 9 );

    m_pMeshShader->setUniInt( "tex0", 0 );
    m_pMeshShader->setUniInt( "tex1", 1 );
    m_pMeshShader->setUniInt( "tex2", 2 );
    m_pMeshShader->setUniInt( "tex3", 3 );
    m_pMeshShader->setUniInt( "tex4", 4 );
    m_pMeshShader->setUniInt( "tex5", 5 );
    /*m_pMeshShader->setUniInt( "tex6", 6 );
    m_pMeshShader->setUniInt( "tex7", 7 );
    m_pMeshShader->setUniInt( "tex8", 8 );
    m_pMeshShader->setUniInt( "tex9", 9 );*/

    m_pMeshShader->setUniInt( "type0", m_type[0] );
    m_pMeshShader->setUniInt( "type1", m_type[1] );
    m_pMeshShader->setUniInt( "type2", m_type[2] );
    m_pMeshShader->setUniInt( "type3", m_type[3] );
    m_pMeshShader->setUniInt( "type4", m_type[4] );
    m_pMeshShader->setUniInt( "type5", m_type[5] );
    /*m_pMeshShader->setUniInt( "type6", m_type[6] );
    m_pMeshShader->setUniInt( "type7", m_type[7] );
    m_pMeshShader->setUniInt( "type8", m_type[8] );
    m_pMeshShader->setUniInt( "type9", m_type[9] );*/

    m_pMeshShader->setUniFloat( "threshold0", m_threshold[0] );
    m_pMeshShader->setUniFloat( "threshold1", m_threshold[1] );
    m_pMeshShader->setUniFloat( "threshold2", m_threshold[2] );
    m_pMeshShader->setUniFloat( "threshold3", m_threshold[3] );
    m_pMeshShader->setUniFloat( "threshold4", m_threshold[4] );
    m_pMeshShader->setUniFloat( "threshold5", m_threshold[5] );
    /*m_pMeshShader->setUniFloat( "threshold6", m_threshold[6] );
    m_pMeshShader->setUniFloat( "threshold7", m_threshold[7] );
    m_pMeshShader->setUniFloat( "threshold8", m_threshold[8] );
    m_pMeshShader->setUniFloat( "threshold9", m_threshold[9] );*/

    m_pMeshShader->setUniFloat( "alpha0", m_alpha[0] );
    m_pMeshShader->setUniFloat( "alpha1", m_alpha[1] );
    m_pMeshShader->setUniFloat( "alpha2", m_alpha[2] );
    m_pMeshShader->setUniFloat( "alpha3", m_alpha[3] );
    m_pMeshShader->setUniFloat( "alpha4", m_alpha[4] );
    m_pMeshShader->setUniFloat( "alpha5", m_alpha[5] );
    /*m_pMeshShader->setUniFloat( "alpha6", m_alpha[6] );
    m_pMeshShader->setUniFloat( "alpha7", m_alpha[7] );
    m_pMeshShader->setUniFloat( "alpha8", m_alpha[8] );
    m_pMeshShader->setUniFloat( "alpha9", m_alpha[9] );*/

}

void ShaderHelper::setFiberShaderVars()
{
    m_pFiberShader->setUniInt( "dimX", m_pDh->m_columns );
    m_pFiberShader->setUniInt( "dimY", m_pDh->m_rows );
    m_pFiberShader->setUniInt( "dimZ", m_pDh->m_frames );
    m_pFiberShader->setUniFloat( "voxX", m_pDh->m_xVoxel );
    m_pFiberShader->setUniFloat( "voxY", m_pDh->m_yVoxel );
    m_pFiberShader->setUniFloat( "voxZ", m_pDh->m_zVoxel );

    int tex = 0;
    int show = 0;
    float threshold = 0;
    int type = 0;

    int c = 0;
    for ( int i = 0; i < m_pDh->m_mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* pInfo = (DatasetInfo*) m_pDh->m_mainFrame->m_listCtrl->GetItemData( i );
        if ( pInfo->getType() < MESH )
        {
            if ( ( pInfo->getType() == OVERLAY ) && pInfo->getShow() )
            {
                tex = c;
                show = pInfo->getShow();
                threshold = pInfo->getThreshold();
                type = pInfo->getType();
                break;
            }
            ++c;
        }
        if ( c == 10 )
            break;
    }

    m_pFiberShader->setUniInt( "tex", tex );
    m_pFiberShader->setUniInt( "type", type );
    m_pFiberShader->setUniFloat( "threshold", threshold );
}

void ShaderHelper::setSplineSurfaceShaderVars()
{
    m_pSplineSurfShader->setUniInt( "dimX", m_pDh->m_columns );
    m_pSplineSurfShader->setUniInt( "dimY", m_pDh->m_rows );
    m_pSplineSurfShader->setUniInt( "dimZ", m_pDh->m_frames );
    m_pSplineSurfShader->setUniFloat( "voxX", m_pDh->m_xVoxel );
    m_pSplineSurfShader->setUniFloat( "voxY", m_pDh->m_yVoxel );
    m_pSplineSurfShader->setUniFloat( "voxZ", m_pDh->m_zVoxel );

    m_pSplineSurfShader->setUniInt( "tex0", 0 );
    m_pSplineSurfShader->setUniInt( "tex1", 1 );
    m_pSplineSurfShader->setUniInt( "tex2", 2 );
    m_pSplineSurfShader->setUniInt( "tex3", 3 );
    m_pSplineSurfShader->setUniInt( "tex4", 4 );
    m_pSplineSurfShader->setUniInt( "tex5", 5 );
    /*m_pSplineSurfShader->setUniInt( "tex6", 6 );
    m_pSplineSurfShader->setUniInt( "tex7", 7 );
    m_pSplineSurfShader->setUniInt( "tex8", 8 );
    m_pSplineSurfShader->setUniInt( "tex9", 9 );*/

    m_pSplineSurfShader->setUniInt( "type0", m_type[0] );
    m_pSplineSurfShader->setUniInt( "type1", m_type[1] );
    m_pSplineSurfShader->setUniInt( "type2", m_type[2] );
    m_pSplineSurfShader->setUniInt( "type3", m_type[3] );
    m_pSplineSurfShader->setUniInt( "type4", m_type[4] );
    m_pSplineSurfShader->setUniInt( "type5", m_type[5] );
   /* m_pSplineSurfShader->setUniInt( "type6", m_type[6] );
    m_pSplineSurfShader->setUniInt( "type7", m_type[7] );
    m_pSplineSurfShader->setUniInt( "type8", m_type[8] );
    m_pSplineSurfShader->setUniInt( "type9", m_type[9] );*/

    m_pSplineSurfShader->setUniFloat( "threshold0", m_threshold[0] );
    m_pSplineSurfShader->setUniFloat( "threshold1", m_threshold[1] );
    m_pSplineSurfShader->setUniFloat( "threshold2", m_threshold[2] );
    m_pSplineSurfShader->setUniFloat( "threshold3", m_threshold[3] );
    m_pSplineSurfShader->setUniFloat( "threshold4", m_threshold[4] );
    m_pSplineSurfShader->setUniFloat( "threshold5", m_threshold[5] );
   /* m_pSplineSurfShader->setUniFloat( "threshold6", m_threshold[6] );
    m_pSplineSurfShader->setUniFloat( "threshold7", m_threshold[7] );
    m_pSplineSurfShader->setUniFloat( "threshold8", m_threshold[8] );
    m_pSplineSurfShader->setUniFloat( "threshold9", m_threshold[9] );*/

    m_pSplineSurfShader->setUniFloat( "alpha0", m_alpha[0] );
    m_pSplineSurfShader->setUniFloat( "alpha1", m_alpha[1] );
    m_pSplineSurfShader->setUniFloat( "alpha2", m_alpha[2] );
    m_pSplineSurfShader->setUniFloat( "alpha3", m_alpha[3] );
    m_pSplineSurfShader->setUniFloat( "alpha4", m_alpha[4] );
    m_pSplineSurfShader->setUniFloat( "alpha5", m_alpha[5] );
    /*m_pSplineSurfShader->setUniFloat( "alpha6", m_alpha[6] );
    m_pSplineSurfShader->setUniFloat( "alpha7", m_alpha[7] );
    m_pSplineSurfShader->setUniFloat( "alpha8", m_alpha[8] );
    m_pSplineSurfShader->setUniFloat( "alpha9", m_alpha[9] );*/
}

