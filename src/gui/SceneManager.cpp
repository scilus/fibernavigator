#include "SceneManager.h"

SceneManager * SceneManager::m_pInstance = NULL;

SceneManager::SceneManager(void)
{
}

//////////////////////////////////////////////////////////////////////////

SceneManager * SceneManager::getInstance()
{
    if( NULL == m_pInstance )
    {
        m_pInstance = new SceneManager();
    }

    return m_pInstance;
}

//////////////////////////////////////////////////////////////////////////

bool SceneManager::load(const wxString &filename)
{
//     if( l_ext == wxT( "scn" ) )
//     {
//         if( ! loadScene( i_fileName ) )
//         {
//             return false;
//         }
// 
//         m_selBoxChanged = true;
//         m_mainFrame->refreshAllGLWidgets();
// 
// #ifdef __WXMSW__
//         m_scnFileName = i_fileName.AfterLast ( '\\' );
//         m_scenePath   = i_fileName.BeforeLast( '\\' );
// #else
//         m_scnFileName = i_fileName.AfterLast ( '/' );
//         m_scenePath   = i_fileName.BeforeLast( '/' );
// #endif
//         m_scnFileLoaded = true;
//         return true;
//     } 
    return false;
}


//////////////////////////////////////////////////////////////////////////

SceneManager::~SceneManager(void)
{
}
