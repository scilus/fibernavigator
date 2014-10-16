#include "RenderManager.h"

#include "../Logger.h"

RenderManager * RenderManager::m_pInstance = NULL;

RenderManager::RenderManager()
:   m_maxTextureNb(0)
{
}

RenderManager * RenderManager::getInstance()
{
    if( !m_pInstance )
    {
        m_pInstance = new RenderManager();
    }
    
    return m_pInstance;
}

void RenderManager::queryGPUCapabilities()
{
    glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &m_maxTextureNb );
}

int RenderManager::getNbMaxTextures()
{
    return m_maxTextureNb;
}

RenderManager::~RenderManager()
{
    Logger::getInstance()->print( wxT( "Executing RenderManager destructor" ), LOGLEVEL_DEBUG );

    m_pInstance = NULL;

    Logger::getInstance()->print( wxT( "RenderManager destructor done" ), LOGLEVEL_DEBUG );
}
