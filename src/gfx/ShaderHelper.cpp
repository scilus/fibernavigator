/*
 * ShaderHelper.cpp
 *
 *  Created on: 28.07.2008
 *      Author: ralph
 */

#include "ShaderHelper.h"
#include "../dataset/DatasetHelper.h"
#include "../dataset/surface.h"

ShaderHelper::ShaderHelper( DatasetHelper* dh ) :
    m_dh( dh )
{
    m_tex.resize( 10 );
    m_type.resize( 10 );
    m_threshold.resize( 10 );
    m_alpha.resize( 10 );

    m_dh->printDebug( _T("initializing texture shader"), 1 );
    m_textureShader = new Shader( wxT("anatomy") );
    m_textureShader->bind();
    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("setup anatomy shader") );

    m_dh->printDebug( _T("initializing mesh shader"), 1 );
    m_meshShader = new Shader( wxT("mesh") );
    m_meshShader->bind();
    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("setup mesh shader") );

    m_dh->printDebug( _T("initializing fiber shader"), 1 );
    m_fiberShader = new Shader( wxT("fibers") );
    m_fiberShader->bind();
    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("setup fiber shader") );

    m_dh->printDebug( _T("initializing fake tube shader"), 1 );
    m_fakeTubeShader = new Shader( wxT("fake-tubes") );
    m_fakeTubeShader->bind();
    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("setup fake tube shader") );

    m_dh->printDebug( _T("initializing spline surface shader"), 1 );
    m_splineSurfShader = new Shader( wxT("splineSurf") );
    m_splineSurfShader->bind();
    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("setup spline surface shader") );
/*
    m_dh->printDebug( _T("initializing vector shader"), 1 );
    m_vectorShader = new Shader( wxT("vectors") );
    m_vectorShader->bind();
    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("setup vectors shader") );
*/
    m_dh->printDebug( _T("initializing legend shader"), 1 );
    m_legendShader = new Shader( wxT("legend") );
    m_legendShader->bind();
    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("setup legend shader") );

    m_dh->printDebug( _T("initializing graph shader"), 1 );
    m_graphShader = new Shader( wxT("graph") );
    m_graphShader->bind();
    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("setup graph shader") );

    m_dh->printDebug( _T("initializing tensors shader"), 1 );
    m_tensorsShader = new Shader( wxT("tensors") );
    m_tensorsShader->bind();
    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("setup tensors shader") );

    m_dh->printDebug( _T("initializing odfs shader"), 1 );
    m_odfsShader = new Shader( wxT("odfs") );
    m_odfsShader->bind();
    if ( m_dh->GLError() )
        m_dh->printGLError( wxT("setup odfs shader") );
}

ShaderHelper::~ShaderHelper()
{
    m_dh->printDebug( _T("execute shader helper destructor\n"), 0 );
    delete m_textureShader;
    delete m_meshShader;
    delete m_fiberShader;
    delete m_splineSurfShader;
    delete m_fakeTubeShader;
    delete m_tensorsShader;
    delete m_odfsShader;

    m_dh->printDebug( _T("shader helper destructor done\n"), 0 );
}

void ShaderHelper::initializeArrays()
{
    m_textureCount = 0;
    m_cutTex = 0;

    m_type.resize( 10, 0 );
    m_threshold.resize( 10, 0 );
    m_alpha.resize( 10, 0 );

    for ( int i = 0; i < m_dh->m_mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* info = (DatasetInfo*) m_dh->m_mainFrame->m_listCtrl->GetItemData( i );
        if ( info->getType() < MESH && info->getShow() )
        {
            m_threshold[m_textureCount] = info->getThreshold();
            m_alpha[m_textureCount] = info->getAlpha();
            m_type[m_textureCount] = info->getType();
            ++m_textureCount;
        }
        if ( m_textureCount == 10 )
            break;
    }
}

void ShaderHelper::setTextureShaderVars()
{
    m_textureShader->setUniInt( "tex0", 0 );
    m_textureShader->setUniInt( "tex1", 1 );
    m_textureShader->setUniInt( "tex2", 2 );
    m_textureShader->setUniInt( "tex3", 3 );
    m_textureShader->setUniInt( "tex4", 4 );
    m_textureShader->setUniInt( "tex5", 5 );
    m_textureShader->setUniInt( "tex6", 6 );
    m_textureShader->setUniInt( "tex7", 7 );
    m_textureShader->setUniInt( "tex8", 8 );
    m_textureShader->setUniInt( "tex9", 9 );

    m_textureShader->setUniInt( "type0", m_type[0] );
    m_textureShader->setUniInt( "type1", m_type[1] );
    m_textureShader->setUniInt( "type2", m_type[2] );
    m_textureShader->setUniInt( "type3", m_type[3] );
    m_textureShader->setUniInt( "type4", m_type[4] );
    m_textureShader->setUniInt( "type5", m_type[5] );
    m_textureShader->setUniInt( "type6", m_type[6] );
    m_textureShader->setUniInt( "type7", m_type[7] );
    m_textureShader->setUniInt( "type8", m_type[8] );
    m_textureShader->setUniInt( "type9", m_type[9] );

    m_textureShader->setUniFloat( "threshold0", m_threshold[0] );
    m_textureShader->setUniFloat( "threshold1", m_threshold[1] );
    m_textureShader->setUniFloat( "threshold2", m_threshold[2] );
    m_textureShader->setUniFloat( "threshold3", m_threshold[3] );
    m_textureShader->setUniFloat( "threshold4", m_threshold[4] );
    m_textureShader->setUniFloat( "threshold5", m_threshold[5] );
    m_textureShader->setUniFloat( "threshold6", m_threshold[6] );
    m_textureShader->setUniFloat( "threshold7", m_threshold[7] );
    m_textureShader->setUniFloat( "threshold8", m_threshold[8] );
    m_textureShader->setUniFloat( "threshold9", m_threshold[9] );

    m_textureShader->setUniFloat( "alpha0", m_alpha[0] );
    m_textureShader->setUniFloat( "alpha1", m_alpha[1] );
    m_textureShader->setUniFloat( "alpha2", m_alpha[2] );
    m_textureShader->setUniFloat( "alpha3", m_alpha[3] );
    m_textureShader->setUniFloat( "alpha4", m_alpha[4] );
    m_textureShader->setUniFloat( "alpha5", m_alpha[5] );
    m_textureShader->setUniFloat( "alpha6", m_alpha[6] );
    m_textureShader->setUniFloat( "alpha7", m_alpha[7] );
    m_textureShader->setUniFloat( "alpha8", m_alpha[8] );
    m_textureShader->setUniFloat( "alpha9", m_alpha[9] );
}

void ShaderHelper::setMeshShaderVars()
{
    m_meshShader->setUniInt( "blendTex", m_dh->m_blendTexOnMesh );

    m_meshShader->setUniInt( "cutAtSurface", m_dh->m_surfaceLoaded );
    m_meshShader->setUniInt( "lightOn", m_dh->m_lighting );
    //m_meshShader->setUniInt( "useColorMap", m_dh->m_colorMap );

    m_meshShader->setUniInt( "dimX", m_dh->m_columns );
    m_meshShader->setUniInt( "dimY", m_dh->m_rows );
    m_meshShader->setUniInt( "dimZ", m_dh->m_frames );
    m_meshShader->setUniFloat( "voxX", m_dh->m_xVoxel );
    m_meshShader->setUniFloat( "voxY", m_dh->m_yVoxel );
    m_meshShader->setUniFloat( "voxZ", m_dh->m_zVoxel );

    m_meshShader->setUniInt( "sector", m_dh->m_quadrant );
    m_meshShader->setUniFloat( "cutX", m_dh->m_xSlize + 0.5f );
    m_meshShader->setUniFloat( "cutY", m_dh->m_ySlize + 0.5f );
    m_meshShader->setUniFloat( "cutZ", m_dh->m_zSlize + 0.5f );


    for ( int i = 0; i < m_dh->m_mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* info = (DatasetInfo*) m_dh->m_mainFrame->m_listCtrl->GetItemData( i );

        if ( info->getType() == SURFACE )
        {
            Surface* s = (Surface*) m_dh->m_mainFrame->m_listCtrl->GetItemData( i );
            m_cutTex = s->getCutTex();

            glActiveTexture( GL_TEXTURE0 + 9 );
            glBindTexture( GL_TEXTURE_2D, m_cutTex );
            m_tex[9] = 9;
            m_threshold[9] = 0;
            m_type[9] = 5;
        }
    }
    m_meshShader->setUniInt( "cutTex", 9 );

    m_meshShader->setUniInt( "tex0", 0 );
    m_meshShader->setUniInt( "tex1", 1 );
    m_meshShader->setUniInt( "tex2", 2 );
    m_meshShader->setUniInt( "tex3", 3 );
    m_meshShader->setUniInt( "tex4", 4 );
    m_meshShader->setUniInt( "tex5", 5 );
    /*m_meshShader->setUniInt( "tex6", 6 );
    m_meshShader->setUniInt( "tex7", 7 );
    m_meshShader->setUniInt( "tex8", 8 );
    m_meshShader->setUniInt( "tex9", 9 );*/

    m_meshShader->setUniInt( "type0", m_type[0] );
    m_meshShader->setUniInt( "type1", m_type[1] );
    m_meshShader->setUniInt( "type2", m_type[2] );
    m_meshShader->setUniInt( "type3", m_type[3] );
    m_meshShader->setUniInt( "type4", m_type[4] );
    m_meshShader->setUniInt( "type5", m_type[5] );
    /*m_meshShader->setUniInt( "type6", m_type[6] );
    m_meshShader->setUniInt( "type7", m_type[7] );
    m_meshShader->setUniInt( "type8", m_type[8] );
    m_meshShader->setUniInt( "type9", m_type[9] );*/

    m_meshShader->setUniFloat( "threshold0", m_threshold[0] );
    m_meshShader->setUniFloat( "threshold1", m_threshold[1] );
    m_meshShader->setUniFloat( "threshold2", m_threshold[2] );
    m_meshShader->setUniFloat( "threshold3", m_threshold[3] );
    m_meshShader->setUniFloat( "threshold4", m_threshold[4] );
    m_meshShader->setUniFloat( "threshold5", m_threshold[5] );
    /*m_meshShader->setUniFloat( "threshold6", m_threshold[6] );
    m_meshShader->setUniFloat( "threshold7", m_threshold[7] );
    m_meshShader->setUniFloat( "threshold8", m_threshold[8] );
    m_meshShader->setUniFloat( "threshold9", m_threshold[9] );*/

    m_meshShader->setUniFloat( "alpha0", m_alpha[0] );
    m_meshShader->setUniFloat( "alpha1", m_alpha[1] );
    m_meshShader->setUniFloat( "alpha2", m_alpha[2] );
    m_meshShader->setUniFloat( "alpha3", m_alpha[3] );
    m_meshShader->setUniFloat( "alpha4", m_alpha[4] );
    m_meshShader->setUniFloat( "alpha5", m_alpha[5] );
    /*m_meshShader->setUniFloat( "alpha6", m_alpha[6] );
    m_meshShader->setUniFloat( "alpha7", m_alpha[7] );
    m_meshShader->setUniFloat( "alpha8", m_alpha[8] );
    m_meshShader->setUniFloat( "alpha9", m_alpha[9] );*/

}

void ShaderHelper::setFiberShaderVars()
{
    m_fiberShader->setUniInt( "dimX", m_dh->m_columns );
    m_fiberShader->setUniInt( "dimY", m_dh->m_rows );
    m_fiberShader->setUniInt( "dimZ", m_dh->m_frames );
    m_fiberShader->setUniFloat( "voxX", m_dh->m_xVoxel );
    m_fiberShader->setUniFloat( "voxY", m_dh->m_yVoxel );
    m_fiberShader->setUniFloat( "voxZ", m_dh->m_zVoxel );

    int tex = 0;
    int show = 0;
    float threshold = 0;
    int type = 0;

    int c = 0;
    for ( int i = 0; i < m_dh->m_mainFrame->m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* info = (DatasetInfo*) m_dh->m_mainFrame->m_listCtrl->GetItemData( i );
        if ( info->getType() < MESH )
        {
            if ( ( info->getType() == OVERLAY ) && info->getShow() )
            {
                tex = c;
                show = info->getShow();
                threshold = info->getThreshold();
                type = info->getType();
                break;
            }
            ++c;
        }
        if ( c == 10 )
            break;
    }

    m_fiberShader->setUniInt( "tex", tex );
    m_fiberShader->setUniInt( "type", type );
    m_fiberShader->setUniFloat( "threshold", threshold );
}

void ShaderHelper::setSplineSurfaceShaderVars()
{
    m_splineSurfShader->setUniInt( "dimX", m_dh->m_columns );
    m_splineSurfShader->setUniInt( "dimY", m_dh->m_rows );
    m_splineSurfShader->setUniInt( "dimZ", m_dh->m_frames );
    m_splineSurfShader->setUniFloat( "voxX", m_dh->m_xVoxel );
    m_splineSurfShader->setUniFloat( "voxY", m_dh->m_yVoxel );
    m_splineSurfShader->setUniFloat( "voxZ", m_dh->m_zVoxel );

    m_splineSurfShader->setUniInt( "tex0", 0 );
    m_splineSurfShader->setUniInt( "tex1", 1 );
    m_splineSurfShader->setUniInt( "tex2", 2 );
    m_splineSurfShader->setUniInt( "tex3", 3 );
    m_splineSurfShader->setUniInt( "tex4", 4 );
    m_splineSurfShader->setUniInt( "tex5", 5 );
    m_splineSurfShader->setUniInt( "tex6", 6 );
    m_splineSurfShader->setUniInt( "tex7", 7 );
    m_splineSurfShader->setUniInt( "tex8", 8 );
    m_splineSurfShader->setUniInt( "tex9", 9 );

    m_splineSurfShader->setUniInt( "type0", m_type[0] );
    m_splineSurfShader->setUniInt( "type1", m_type[1] );
    m_splineSurfShader->setUniInt( "type2", m_type[2] );
    m_splineSurfShader->setUniInt( "type3", m_type[3] );
    m_splineSurfShader->setUniInt( "type4", m_type[4] );
    m_splineSurfShader->setUniInt( "type5", m_type[5] );
    m_splineSurfShader->setUniInt( "type6", m_type[6] );
    m_splineSurfShader->setUniInt( "type7", m_type[7] );
    m_splineSurfShader->setUniInt( "type8", m_type[8] );
    m_splineSurfShader->setUniInt( "type9", m_type[9] );

    m_splineSurfShader->setUniFloat( "threshold0", m_threshold[0] );
    m_splineSurfShader->setUniFloat( "threshold1", m_threshold[1] );
    m_splineSurfShader->setUniFloat( "threshold2", m_threshold[2] );
    m_splineSurfShader->setUniFloat( "threshold3", m_threshold[3] );
    m_splineSurfShader->setUniFloat( "threshold4", m_threshold[4] );
    m_splineSurfShader->setUniFloat( "threshold5", m_threshold[5] );
    m_splineSurfShader->setUniFloat( "threshold6", m_threshold[6] );
    m_splineSurfShader->setUniFloat( "threshold7", m_threshold[7] );
    m_splineSurfShader->setUniFloat( "threshold8", m_threshold[8] );
    m_splineSurfShader->setUniFloat( "threshold9", m_threshold[9] );

    m_splineSurfShader->setUniFloat( "alpha0", m_alpha[0] );
    m_splineSurfShader->setUniFloat( "alpha1", m_alpha[1] );
    m_splineSurfShader->setUniFloat( "alpha2", m_alpha[2] );
    m_splineSurfShader->setUniFloat( "alpha3", m_alpha[3] );
    m_splineSurfShader->setUniFloat( "alpha4", m_alpha[4] );
    m_splineSurfShader->setUniFloat( "alpha5", m_alpha[5] );
    m_splineSurfShader->setUniFloat( "alpha6", m_alpha[6] );
    m_splineSurfShader->setUniFloat( "alpha7", m_alpha[7] );
    m_splineSurfShader->setUniFloat( "alpha8", m_alpha[8] );
    m_splineSurfShader->setUniFloat( "alpha9", m_alpha[9] );
}

