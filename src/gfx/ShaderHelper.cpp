/*
 * ShaderHelper.cpp
 *
 *  Created on: 28.07.2008
 *      Author: ralph
 */

#include "ShaderHelper.h"

#include "ShaderProgram.h"
#include "../Logger.h"
#include "../main.h"
#include "../dataset/DatasetManager.h"
#include "../gui/MainFrame.h"
#include "../gui/SceneManager.h"

ShaderHelper * ShaderHelper::m_pInstance = NULL;

ShaderHelper::ShaderHelper()
:   m_tex( 10 ),
    m_threshold( 10 ),
    m_alpha( 10 ),
    m_type( 10 ),
    m_pAnatomyShader( NULL ),
    m_pMeshShader( NULL ),
    m_pFibersShader( NULL ),
    m_pFakeTubesShader( NULL ),
    m_pCrossingFibersShader( NULL ),
    m_pSplineSurfShader( NULL ),
    m_pVectorShader( NULL ),
    m_pLegendShader( NULL ),
    m_pGraphShader( NULL ),
    m_pTensorsShader( NULL ),
    m_pOdfsShader( NULL ),
    m_pRTTShader( NULL )    
{
}

ShaderHelper * ShaderHelper::getInstance()
{
    if( !m_pInstance )
    {
        m_pInstance = new ShaderHelper();
    }
    return m_pInstance;
}

void ShaderHelper::loadShaders( bool geometryShadersSupported )
{
    delete m_pAnatomyShader;
    delete m_pCrossingFibersShader;
    delete m_pFakeTubesShader;
    delete m_pFibersShader;
    delete m_pGraphShader;
    delete m_pLegendShader;
    delete m_pMeshShader;
    delete m_pOdfsShader;
    //delete m_pSplineSurfShader;
    delete m_pTensorsShader;
    //delete m_pVectorShader;

    m_pAnatomyShader = new ShaderProgram( wxT( "anatomy" ) );
    m_pMeshShader = new ShaderProgram( wxT( "mesh" ) );
    m_pFibersShader = new ShaderProgram( wxT( "fibers" ) );
    m_pFakeTubesShader = new ShaderProgram( wxT( "fake-tubes") );
    m_pCrossingFibersShader = new ShaderProgram( wxT( "crossing_fibers" ), true, geometryShadersSupported );
    //m_pSplineSurfShader = new ShaderProgram( wxT( "splineSurf" ) );
    //m_pVectorShader = new ShaderProgram( wxT( "vectors" ) );
    m_pLegendShader = new ShaderProgram( wxT( "legend" ) );
    m_pGraphShader = new ShaderProgram( wxT( "graph" ) );
    m_pTensorsShader = new ShaderProgram( wxT( "tensors" ) );
    m_pOdfsShader = new ShaderProgram( wxT( "odfs" ) );
    m_pRTTShader = new ShaderProgram( wxT( "RTT" ) );

    Logger::getInstance()->print( _T( "Initializing anatomy shader..." ), LOGLEVEL_MESSAGE );
    if( m_pAnatomyShader->load() && m_pAnatomyShader->compileAndLink() )
    {
        m_pAnatomyShader->bind();
        Logger::getInstance()->print( _T( "Anatomy shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->print( _T( "Could not initialize anatomy shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->print( _T( "Initializing mesh shader..." ), LOGLEVEL_MESSAGE );
    if( m_pMeshShader->load() && m_pMeshShader->compileAndLink() )
    {
        m_pMeshShader->bind();
        Logger::getInstance()->print( _T( "Mesh shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->print( _T( "Could not initialize mesh shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->print( _T( "Initializing fibers shader..." ), LOGLEVEL_MESSAGE );
    if( m_pFibersShader->load() && m_pFibersShader->compileAndLink() )
    {
        m_pFibersShader->bind();
        Logger::getInstance()->print( _T( "Fibers shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->print( _T( "Could not initialize fibers shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->print( _T( "Initializing fake tubes shader..." ), LOGLEVEL_MESSAGE );
    if( m_pFakeTubesShader->load() && m_pFakeTubesShader->compileAndLink() )
    {
        m_pFakeTubesShader->bind();
        Logger::getInstance()->print( _T( "Fake Tubes shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->print( _T( "Could not initialize fake Tubes shader." ), LOGLEVEL_ERROR );
    }

    if ( geometryShadersSupported )
    {
        Logger::getInstance()->print( _T( "Initializing crossing fibers shader..." ), LOGLEVEL_MESSAGE );

        glProgramParameteriEXT( m_pCrossingFibersShader->getId(), GL_GEOMETRY_INPUT_TYPE_EXT, GL_LINES );
        glProgramParameteriEXT( m_pCrossingFibersShader->getId(), GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_LINE_STRIP );
        glProgramParameteriEXT( m_pCrossingFibersShader->getId(), GL_GEOMETRY_VERTICES_OUT_EXT, 50 );

        if( m_pCrossingFibersShader->load() && m_pCrossingFibersShader->compileAndLink() )
        {
            m_pCrossingFibersShader->bind();
            Logger::getInstance()->print( _T( "Crossing fibers shader initialized." ), LOGLEVEL_MESSAGE );
        }
        else
        {
            Logger::getInstance()->print( _T( "Could not initialize crossing fibers shader." ), LOGLEVEL_ERROR );
        }
    }
    else
    {
        Logger::getInstance()->print( _T( "Geometry shaders are not supported. Cannot load crossing fibers shader." ), LOGLEVEL_WARNING );
    }

//     Logger::getInstance()->print( _T( "Initializing spline surf shader..." ), LOGLEVEL_MESSAGE );
//     if( m_pSplineSurfShader->load() && m_pSplineSurfShader->compileAndLink() )
//     {
//         m_pSplineSurfShader->bind();
//         Logger::getInstance()->print( _T( "Spline surf shader initialized." ), LOGLEVEL_MESSAGE );
//     }
//     else
//     {
//         Logger::getInstance()->print( _T( "Could not initialize spline surf shader." ), LOGLEVEL_ERROR );
//     }

//     Logger::getInstance()->print( _T( "Initializing vector shader..." ), LOGLEVEL_MESSAGE );
//     if( m_pVectorShader->load() && m_pVectorShader->compileAndLink() )
//     {
//        m_pVectorShader->bind();
//        Logger::getInstance()->print( _T( "Vector shader initialized." ), LOGLEVEL_MESSAGE );
//     }
//     else
//     {
//        Logger::getInstance()->print( _T( "Could not initialize vector shader." ), LOGLEVEL_ERROR );
//     }

    Logger::getInstance()->print( _T( "Initializing legend shader..." ), LOGLEVEL_MESSAGE );
    if( m_pLegendShader->load() && m_pLegendShader->compileAndLink() )
    {
        m_pLegendShader->bind();
        Logger::getInstance()->print( _T( "Legend shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->print( _T( "Could not initialize legend shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->print( _T( "Initializing graph shader..." ), LOGLEVEL_MESSAGE );
    if( m_pGraphShader->load() && m_pGraphShader->compileAndLink() )
    {
        m_pGraphShader->bind();
        Logger::getInstance()->print( _T( "Graph shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->print( _T( "Could not initialize graph shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->print( _T( "Initializing tensors shader..." ), LOGLEVEL_MESSAGE );
    if( m_pTensorsShader->load() && m_pTensorsShader->compileAndLink() )
    {
        m_pTensorsShader->bind();
        Logger::getInstance()->print( _T( "Tensors shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->print( _T( "Could not initialize tensors shader." ), LOGLEVEL_ERROR );
    }

    Logger::getInstance()->print( _T( "Initializing odfs shader..." ), LOGLEVEL_MESSAGE );
    if( m_pOdfsShader->load() && m_pOdfsShader->compileAndLink() )
    {
        m_pOdfsShader->bind();
        Logger::getInstance()->print( _T( "Odfs shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->print( _T( "Could not initialize odfs shader." ), LOGLEVEL_ERROR );
    }
    Logger::getInstance()->print( _T( "Initializing RTT shader..." ), LOGLEVEL_MESSAGE );
    if( m_pRTTShader->load() && m_pRTTShader->compileAndLink() )
    {
        m_pRTTShader->bind();
        Logger::getInstance()->print( _T( "RTT shader initialized." ), LOGLEVEL_MESSAGE );
    }
    else
    {
        Logger::getInstance()->print( _T( "Could not initialize RTT shader." ), LOGLEVEL_ERROR );
    }
}

void ShaderHelper::initializeArrays()
{
    m_textureCount = 0;
    m_cutTex = 0;

    m_type.resize( 10, 0 );
    m_threshold.resize( 10, 0 );
    m_alpha.resize( 10, 0 );

    for ( int i = 0; i < MyApp::frame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* pInfo = DatasetManager::getInstance()->getDataset( MyApp::frame->m_pListCtrl->GetItem( i ) );
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
    m_pAnatomyShader->setUniInt( "tex0", 0 );
    m_pAnatomyShader->setUniInt( "tex1", 1 );
    m_pAnatomyShader->setUniInt( "tex2", 2 );
    m_pAnatomyShader->setUniInt( "tex3", 3 );
    m_pAnatomyShader->setUniInt( "tex4", 4 );
    m_pAnatomyShader->setUniInt( "tex5", 5 );
    m_pAnatomyShader->setUniInt( "tex6", 6 );
    m_pAnatomyShader->setUniInt( "tex7", 7 );
    m_pAnatomyShader->setUniInt( "tex8", 8 );
    m_pAnatomyShader->setUniInt( "tex9", 9 );

    m_pAnatomyShader->setUniInt( "type0", m_type[0] );
    m_pAnatomyShader->setUniInt( "type1", m_type[1] );
    m_pAnatomyShader->setUniInt( "type2", m_type[2] );
    m_pAnatomyShader->setUniInt( "type3", m_type[3] );
    m_pAnatomyShader->setUniInt( "type4", m_type[4] );
    m_pAnatomyShader->setUniInt( "type5", m_type[5] );
    m_pAnatomyShader->setUniInt( "type6", m_type[6] );
    m_pAnatomyShader->setUniInt( "type7", m_type[7] );
    m_pAnatomyShader->setUniInt( "type8", m_type[8] );
    m_pAnatomyShader->setUniInt( "type9", m_type[9] );

    m_pAnatomyShader->setUniFloat( "threshold0", m_threshold[0] );
    m_pAnatomyShader->setUniFloat( "threshold1", m_threshold[1] );
    m_pAnatomyShader->setUniFloat( "threshold2", m_threshold[2] );
    m_pAnatomyShader->setUniFloat( "threshold3", m_threshold[3] );
    m_pAnatomyShader->setUniFloat( "threshold4", m_threshold[4] );
    m_pAnatomyShader->setUniFloat( "threshold5", m_threshold[5] );
    m_pAnatomyShader->setUniFloat( "threshold6", m_threshold[6] );
    m_pAnatomyShader->setUniFloat( "threshold7", m_threshold[7] );
    m_pAnatomyShader->setUniFloat( "threshold8", m_threshold[8] );
    m_pAnatomyShader->setUniFloat( "threshold9", m_threshold[9] );

    m_pAnatomyShader->setUniFloat( "alpha0", m_alpha[0] );
    m_pAnatomyShader->setUniFloat( "alpha1", m_alpha[1] );
    m_pAnatomyShader->setUniFloat( "alpha2", m_alpha[2] );
    m_pAnatomyShader->setUniFloat( "alpha3", m_alpha[3] );
    m_pAnatomyShader->setUniFloat( "alpha4", m_alpha[4] );
    m_pAnatomyShader->setUniFloat( "alpha5", m_alpha[5] );
    m_pAnatomyShader->setUniFloat( "alpha6", m_alpha[6] );
    m_pAnatomyShader->setUniFloat( "alpha7", m_alpha[7] );
    m_pAnatomyShader->setUniFloat( "alpha8", m_alpha[8] );
    m_pAnatomyShader->setUniFloat( "alpha9", m_alpha[9] );
}

void ShaderHelper::setMeshShaderVars()
{
    m_pMeshShader->setUniInt( "blendTex", SceneManager::getInstance()->isTexBlendOnMesh() );

    m_pMeshShader->setUniInt( "lightOn",      SceneManager::getInstance()->isLightingActive() );

    m_pMeshShader->setUniInt( "dimX", DatasetManager::getInstance()->getColumns() );
    m_pMeshShader->setUniInt( "dimY", DatasetManager::getInstance()->getRows() );
    m_pMeshShader->setUniInt( "dimZ", DatasetManager::getInstance()->getFrames() );

    m_pMeshShader->setUniFloat( "voxX", DatasetManager::getInstance()->getVoxelX() );
    m_pMeshShader->setUniFloat( "voxY", DatasetManager::getInstance()->getVoxelY() );
    m_pMeshShader->setUniFloat( "voxZ", DatasetManager::getInstance()->getVoxelZ() );

    m_pMeshShader->setUniInt( "sector", SceneManager::getInstance()->getQuadrant() );
    m_pMeshShader->setUniFloat( "cutX", SceneManager::getInstance()->getSliceX() + 0.5f );
    m_pMeshShader->setUniFloat( "cutY", SceneManager::getInstance()->getSliceY() + 0.5f );
    m_pMeshShader->setUniFloat( "cutZ", SceneManager::getInstance()->getSliceZ() + 0.5f );

    m_pMeshShader->setUniInt( "tex0", 0 );
    m_pMeshShader->setUniInt( "tex1", 1 );
    m_pMeshShader->setUniInt( "tex2", 2 );
    m_pMeshShader->setUniInt( "tex3", 3 );
    m_pMeshShader->setUniInt( "tex4", 4 );
    m_pMeshShader->setUniInt( "tex5", 5 );

    m_pMeshShader->setUniInt( "type0", m_type[0] );
    m_pMeshShader->setUniInt( "type1", m_type[1] );
    m_pMeshShader->setUniInt( "type2", m_type[2] );
    m_pMeshShader->setUniInt( "type3", m_type[3] );
    m_pMeshShader->setUniInt( "type4", m_type[4] );
    m_pMeshShader->setUniInt( "type5", m_type[5] );

    m_pMeshShader->setUniFloat( "threshold0", m_threshold[0] );
    m_pMeshShader->setUniFloat( "threshold1", m_threshold[1] );
    m_pMeshShader->setUniFloat( "threshold2", m_threshold[2] );
    m_pMeshShader->setUniFloat( "threshold3", m_threshold[3] );
    m_pMeshShader->setUniFloat( "threshold4", m_threshold[4] );
    m_pMeshShader->setUniFloat( "threshold5", m_threshold[5] );

    m_pMeshShader->setUniFloat( "alpha0", m_alpha[0] );
    m_pMeshShader->setUniFloat( "alpha1", m_alpha[1] );
    m_pMeshShader->setUniFloat( "alpha2", m_alpha[2] );
    m_pMeshShader->setUniFloat( "alpha3", m_alpha[3] );
    m_pMeshShader->setUniFloat( "alpha4", m_alpha[4] );
    m_pMeshShader->setUniFloat( "alpha5", m_alpha[5] );
}

void ShaderHelper::setFiberShaderVars()
{
    m_pFibersShader->setUniInt( "dimX", DatasetManager::getInstance()->getColumns() );
    m_pFibersShader->setUniInt( "dimY", DatasetManager::getInstance()->getRows() );
    m_pFibersShader->setUniInt( "dimZ", DatasetManager::getInstance()->getFrames() );
    m_pFibersShader->setUniFloat( "voxX", DatasetManager::getInstance()->getVoxelX() );
    m_pFibersShader->setUniFloat( "voxY", DatasetManager::getInstance()->getVoxelY() );
    m_pFibersShader->setUniFloat( "voxZ", DatasetManager::getInstance()->getVoxelZ() );

    int tex = 0;
    int show = 0;
    float threshold = 0;
    int type = 0;

    int c = 0;
    for ( int i = 0; i < MyApp::frame->m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* pInfo = DatasetManager::getInstance()->getDataset( MyApp::frame->m_pListCtrl->GetItem( i ) );
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

    m_pFibersShader->setUniInt( "tex", tex );
    m_pFibersShader->setUniInt( "type", type );
    m_pFibersShader->setUniFloat( "threshold", threshold );
}

ShaderHelper::~ShaderHelper()
{
    Logger::getInstance()->print( wxT( "Executing ShaderHelper destructor" ), LOGLEVEL_DEBUG );
    delete m_pAnatomyShader;
//    delete m_pCrossingFibersShader;
    delete m_pFakeTubesShader;
    delete m_pFibersShader;
    delete m_pGraphShader;
    delete m_pLegendShader;
//    delete m_pMeshShader;
    delete m_pOdfsShader;
    delete m_pSplineSurfShader;
    delete m_pTensorsShader;
    delete m_pVectorShader;

    m_pInstance = NULL;
    Logger::getInstance()->print( wxT( "ShaderHelper destructor done" ), LOGLEVEL_DEBUG );
}
