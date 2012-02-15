#include "SceneManager.h"

#include "MainFrame.h"
#include "MyListCtrl.h"
#include "SelectionObject.h"

#include "../Logger.h"
#include "../dataset/DatasetManager.h"
#include "../dataset/ODFs.h"
#include "../dataset/Tensors.h"

#include <assert.h>
#include <vector>
using std::vector;

SceneManager * SceneManager::m_pInstance = NULL;

SceneManager::SceneManager(void)
:   m_pMainFrame( NULL ),
    m_pTreeView ( NULL ),
    m_sliceX( 0.0f ),
    m_sliceY( 0.0f ),
    m_sliceZ( 0.0f )
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
    return false;
}

//////////////////////////////////////////////////////////////////////////

bool SceneManager::save( const wxString &filename )
{
    return false;
}

//////////////////////////////////////////////////////////////////////////

void SceneManager::deleteAllSelectionObjects()
{
    Logger::getInstance()->print( wxT( "SceneManager::deleteAllSelectionObjects" ), LOGLEVEL_DEBUG );

    assert( m_pTreeView != NULL );

    SelectionObjList selectionObjs = getSelectionObjects();
    for( SelectionObjList::iterator it = selectionObjs.begin(); it != selectionObjs.end(); ++it)
    {
        for( vector<SelectionObject *>::iterator childIt = it->begin(); childIt != it->end(); ++childIt )
        {
            m_pTreeView->Delete( (*childIt)->GetId() );
        }
    }
}

//////////////////////////////////////////////////////////////////////////

void SceneManager::updateAllSelectionObjects()
{
    Logger::getInstance()->print( wxT( "SceneManager::updateAllSelectionObjects" ), LOGLEVEL_DEBUG );

    SelectionObjList selectionObjs = getSelectionObjects();
    for( SelectionObjList::iterator it = selectionObjs.begin(); it != selectionObjs.end(); ++it)
    {
        for( vector<SelectionObject *>::iterator childIt = it->begin(); childIt != it->end(); ++childIt )
        {
            (*childIt)->setIsDirty( true );
        }
    }
}

//////////////////////////////////////////////////////////////////////////

void SceneManager::updateView( const float x, const float y, const float z )
{
    m_sliceX = x;
    m_sliceY = y;
    m_sliceZ = z;

    // TODO: Implement this
//     if( m_boxLockIsOn && !m_semaphore )
//         m_boxAtCrosshair->setCenter( i_x, i_y, i_z );

    vector<ODFs *> odfs = DatasetManager::getInstance()->getOdfs();
    for( vector<ODFs *>::iterator it = odfs.begin(); it != odfs.end(); ++it )
    {
        (*it)->refreshSlidersValues();
    }

    vector<Tensors *> tensors = DatasetManager::getInstance()->getTensors();
    for( vector<Tensors *>::iterator it = tensors.begin(); it != tensors.end(); ++it )
    {
        (*it)->refreshSlidersValues();
    }
}

//////////////////////////////////////////////////////////////////////////

SelectionObjList SceneManager::getSelectionObjects()
{
    assert( m_pMainFrame != NULL );
    assert( m_pTreeView  != NULL );

    SelectionObjList selectionObjects;

    wxTreeItemId id, childId;
    wxTreeItemIdValue cookie = 0;

    id = m_pTreeView->GetFirstChild( m_pMainFrame->m_tSelectionObjectsId, cookie );

    while( id.IsOk() )
    {
        std::vector< SelectionObject * > vect;
        vect.push_back( (SelectionObject *)m_pTreeView->GetItemData( id ) );
        
        childId = m_pTreeView->GetFirstChild( id, cookie );

        while( childId.IsOk() )
        {
            vect.push_back( (SelectionObject*) m_pTreeView->GetItemData( childId ) );
            childId = m_pTreeView->GetNextChild( id, cookie );
        }

        id = m_pTreeView->GetNextChild( m_pMainFrame->m_tSelectionObjectsId, cookie );
        selectionObjects.push_back( vect );
    }

    return selectionObjects;
}

//////////////////////////////////////////////////////////////////////////

SceneManager::~SceneManager(void)
{
    m_pInstance = NULL;
}
