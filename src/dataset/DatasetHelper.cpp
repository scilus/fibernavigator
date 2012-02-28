/*
 * DatasetHelper.cpp
 *
 *  Created on: 27.07.2008
 *      Author: ralph
 */
#include "DatasetHelper.h"

#include "SplinePoint.h"
#include "../Logger.h"
#include "../main.h"
#include "../gui/MainFrame.h"
#include "../gui/MyListCtrl.h"
#include "../gui/SceneManager.h"


///////////////////////////////////////////////////////////////////////////
// Constructor
DatasetHelper::DatasetHelper() 
:   m_selBoxChanged( true ),
    m_boxAtCrosshair    ( 0 ),
    m_lastSelectedPoint ( NULL ),
    m_lastSelectedObject( NULL )
{
}

DatasetHelper::~DatasetHelper()
{
    Logger::getInstance()->print( wxT( "Execute DatasetHelper destructor" ), LOGLEVEL_DEBUG );

    Logger::getInstance()->print( wxT( "DatasetHelper destructor done" ), LOGLEVEL_DEBUG );
}

void DatasetHelper::deleteAllPoints()
{
    std::vector< std::vector< SplinePoint* > > l_points;

    wxTreeItemId l_id, l_childId;
    wxTreeItemIdValue l_cookie = 0;

    l_id = MyApp::frame->m_pTreeWidget->GetFirstChild( MyApp::frame->m_tPointId, l_cookie );

    while( l_id.IsOk() )
    {
        std::vector< SplinePoint* > l_b;
        l_b.push_back( (SplinePoint*)( MyApp::frame->m_pTreeWidget->GetItemData( l_id ) ) );
        wxTreeItemIdValue childcookie = 0;
        l_childId = MyApp::frame->m_pTreeWidget->GetFirstChild( l_id, childcookie );

        while( l_childId.IsOk() )
        {
            l_b.push_back( (SplinePoint*)( MyApp::frame->m_pTreeWidget->GetItemData( l_childId ) ) );
            l_childId = MyApp::frame->m_pTreeWidget->GetNextChild( l_id, childcookie );
        }

        l_id = MyApp::frame->m_pTreeWidget->GetNextChild( MyApp::frame->m_tPointId, l_cookie );
        l_points.push_back( l_b );
    }

    for( unsigned int i = 0; i < l_points.size(); ++i )
    {
        for( unsigned int j = 0; j < l_points[i].size(); ++j )
        {
            MyApp::frame->m_pTreeWidget->Delete(l_points[i][j]->GetId());      
        }
    }
}
