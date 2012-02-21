#include "SceneManager.h"

#include "MainFrame.h"
#include "MyListCtrl.h"
#include "SelectionObject.h"

#include "../Logger.h"
#include "../dataset/DatasetManager.h"
#include "../dataset/ODFs.h"
#include "../dataset/Tensors.h"
#include "../gfx/ShaderHelper.h"
#include "../gfx/TheScene.h"

#include <assert.h>
#include <vector>
using std::vector;

SceneManager * SceneManager::m_pInstance = NULL;

SceneManager::SceneManager(void)
:   m_pMainFrame( NULL ),
    m_pShaderHelper( NULL ),
    m_pTheScene( NULL ),
    m_pTreeView ( NULL ),
    m_geometryShadersSupported( true ),
    m_useFibersGeometryShader( false ),
    m_lighting( true ),
    m_showAxial( true ),
    m_showCoronal( true ),
    m_showSagittal( true ),
    m_showCrosshair( false ),
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

//     long xp, yp, zp;
//     xp = yp = zp = 0;
//     double r00, r10, r20, r01, r11, r21, r02, r12, r22;
//     r10 = r20 = r01 = r21 = r02 = r12 = 0;
//     r00 = r11 = r22 = 1;
// 
//     wxXmlDocument l_xmlDoc;
//     if( !l_xmlDoc.Load( filename ) )
//         return false;
// 	
// 	wxString xmlVersion = l_xmlDoc.GetVersion();
// 	long version;
// 	xmlVersion.ToLong(&version);
// 	
//     wxXmlNode* l_child = l_xmlDoc.GetRoot()->GetChildren();
//     while( l_child )
//     {
//         if( l_child->GetName() == wxT( "anatomy" ) )
//         {
//             wxString srows      = l_child->GetPropVal( wxT( "rows" ),    wxT( "1" ) );
//             wxString scolumns   = l_child->GetPropVal( wxT( "columns" ), wxT( "1" ) );
//             wxString sframes    = l_child->GetPropVal( wxT( "frames" ),  wxT( "1" ) );
// 
//             long l_rows, l_columns, l_frames;
// 
//             srows.ToLong   ( &l_rows );
//             scolumns.ToLong( &l_columns );
//             sframes.ToLong ( &l_frames );
//             if( m_anatomyLoaded )
//             {
//                 if( ( l_rows != m_rows ) || ( l_columns != m_columns ) || ( l_frames != m_frames ) )
//                 {
//                     m_lastError = wxT( "dimensions of loaded files must be the same" );
//                     return false;
//                 }
//             }
//             else
//             {
//                 m_rows            = l_rows;
//                 m_columns         = l_columns;
//                 m_frames          = l_frames;
//                 m_anatomyLoaded   = true;
//             }
//         }
//         else if( l_child->GetName() == wxT( "position" ) )
//         {
//             l_child->GetPropVal( wxT( "x" ), wxT( "1" ) ).ToLong( &xp, 10 );
//             l_child->GetPropVal( wxT( "y" ), wxT( "1" ) ).ToLong( &yp, 10 );
//             l_child->GetPropVal( wxT( "z" ), wxT( "1" ) ).ToLong( &zp, 10 );
//         }
//         else if( l_child->GetName() == wxT( "rotation" ) )
//         {
//             //l_child->GetPropVal( wxT( "rot00" ), wxT( "1" ) ).ToDouble( &r00 );
//             //l_child->GetPropVal( wxT( "rot10" ), wxT( "1" ) ).ToDouble( &r10 );
//             //l_child->GetPropVal( wxT( "rot20" ), wxT( "1" ) ).ToDouble( &r20 );
//             //l_child->GetPropVal( wxT( "rot01" ), wxT( "1" ) ).ToDouble( &r01 );
//             //l_child->GetPropVal( wxT( "rot11" ), wxT( "1" ) ).ToDouble( &r11 );
//             //l_child->GetPropVal( wxT( "rot21" ), wxT( "1" ) ).ToDouble( &r21 );
//             //l_child->GetPropVal( wxT( "rot02" ), wxT( "1" ) ).ToDouble( &r02 );
//             //l_child->GetPropVal( wxT( "rot12" ), wxT( "1" ) ).ToDouble( &r12 );
//             //l_child->GetPropVal( wxT( "rot22" ), wxT( "1" ) ).ToDouble( &r22 );
//         }
// 
//         else if( l_child->GetName() == wxT( "data" ) )
//         {
// 			std::map<DatasetInfo*, long> realPositions;
// 			bool fibersGroupTreated = false;
// 			
//             wxXmlNode *l_dataSetNode = l_child->GetChildren();
//             while( l_dataSetNode )
//             {
//                 wxXmlNode *l_nodes  = l_dataSetNode->GetChildren();
//                 // initialize to mute compiler
// 				bool l_isfiberGroup	= false;
//                 bool l_active       = true;
//                 bool l_useTex       = true;
//                 bool l_showFS       = true;
//                 double l_threshold  = 0.0;
//                 double l_alpha      = 1.0;
// 				long l_pos			= 0;
//                 wxString l_path;
// 				wxString l_name;
// 				
//                 while( l_nodes )
//                 {
//                     if( l_nodes->GetName() == _T( "status" ) )
//                     {
// 						if( version >= 2 )
// 						{
// 							l_isfiberGroup = ( l_nodes->GetPropVal( _T( "isFiberGroup" ), _T( "yes" ) ) == _T( "yes" ) );
// 							l_nodes->GetPropVal( _T( "name" ), &l_name );
// 						}
//                         l_active = ( l_nodes->GetPropVal( _T( "active" ), _T( "yes" ) ) == _T( "yes" ) );
//                         l_useTex = ( l_nodes->GetPropVal( _T( "useTex" ), _T( "yes" ) ) == _T( "yes" ) );
//                         l_showFS = ( l_nodes->GetPropVal( _T( "showFS" ), _T( "yes" ) ) == _T( "yes" ) );
//                         ( l_nodes->GetPropVal( wxT( "threshold"), wxT( "0.0" ) ) ).ToDouble( &l_threshold );
//                         ( l_nodes->GetPropVal( wxT( "alpha"),     wxT( "0.0" ) ) ).ToDouble( &l_alpha );
// 						if( l_nodes->GetPropVal( wxT( "position"), wxT( "0" ) ) )
// 						{
// 							l_nodes->GetPropVal( wxT( "position"), wxT( "0" ) ).ToLong( &l_pos );
// 						}
//                     }
//                     else if( l_nodes->GetName() == _T( "path" ) )
//                     {
//                         l_path = l_nodes->GetNodeContent();
//                     }
// 
//                     l_nodes = l_nodes->GetNext();
//                 }
//                 load( l_path, -1, l_threshold, l_active, l_showFS, l_useTex, l_alpha, l_name, version, l_isfiberGroup, true );
// 				
// 				if( version < 2 ) // in the old version, no fibersgroup were saved
// 				{
// 					long lastItemPos;
// 
// 					#ifdef __WXMAC__
// 						lastItemPos = m_mainFrame->m_pListCtrl2->GetItemCount() - 1;
// 					#else
// 						lastItemPos = 1;
// 					#endif
// 						if(lastItemPos >= 0  && lastItemPos < m_mainFrame->m_pListCtrl2->GetItemCount() && !fibersGroupTreated)
// 						{
// 							DatasetInfo* pDataset = m_mainFrame->m_pListCtrl2->GetItem( lastItemPos );
// 							// if the last inserted item is a fiber, than a fibergroup has been inserted before
// 							if(pDataset->getType() == FIBERS)
// 							{
// 								if(l_pos - 1 >= 0 )
// 								{
// 									DatasetInfo* pDataset = m_mainFrame->m_pListCtrl2->GetItem(lastItemPos - 1);
// 									// insert fibersgroup position
// 									realPositions.insert( pair<DatasetInfo *, long>(pDataset, l_pos - 1) );
// 								}
// 								fibersGroupTreated = true;
// 							}
// 						}
// 					}
// 
// 				DatasetInfo* pDataset;
// 				#ifdef __WXMAC__
// 					pDataset = m_mainFrame->m_pListCtrl2->GetItem(m_mainFrame->m_pListCtrl2->GetItemCount() - 1);
// 				#else
// 					if( m_fibersGroupLoaded && !l_isfiberGroup )
// 					{
// 						pDataset = m_mainFrame->m_pListCtrl2->GetItem(1);
// 						if( pDataset->getType() != FIBERS )
// 						{
// 							pDataset = m_mainFrame->m_pListCtrl2->GetItem(0);
// 						}
// 					}
// 					else
// 					{
// 						pDataset = m_mainFrame->m_pListCtrl2->GetItem(0);
// 					}
// 				#endif
// 				realPositions.insert(pair<DatasetInfo*, long>(pDataset, l_pos));
// 
//                 l_dataSetNode = l_dataSetNode->GetNext();
//             }
// 			
// 			// Reassign dataset to the good position
// 			if( version > 1 ) 
// 			{
// 				for( long i = 0; i < m_mainFrame->m_pListCtrl2->GetItemCount(); i++)
// 				{
// 					DatasetInfo* pDataset = (DatasetInfo*)m_mainFrame->m_pListCtrl->GetItemData(i);
// 					std::map<DatasetInfo*, long>::iterator it = realPositions.find( pDataset );
// 					
// 					long currentPos = pDataset->getListCtrlItemId();
// 					long realPos = it->second;
// 
// 					if( currentPos != realPos)
// 					{
// 						m_mainFrame->m_pListCtrl->swap(currentPos, realPos);
// 					}
// 				}
// 			}
//         }
//         else if( l_child->GetName() == wxT( "points" ) )
//         {
//             wxXmlNode* l_pNode = l_child->GetChildren();
//             while( l_pNode )
//             {
//                 wxString l_sx = l_pNode->GetPropVal( wxT( "x" ), wxT( "0.0" ) );
//                 wxString l_sy = l_pNode->GetPropVal( wxT( "y" ), wxT( "0.0" ) );
//                 wxString l_sz = l_pNode->GetPropVal( wxT( "z" ), wxT( "0.0" ) );
// 
//                 double l_x, l_y, l_z;
// 
//                 l_sx.ToDouble( &l_x );
//                 l_sy.ToDouble( &l_y );
//                 l_sz.ToDouble( &l_z );
// 
//                 SplinePoint* l_point = new SplinePoint( l_x, l_y, l_z, this );
//                 m_mainFrame->m_pTreeWidget->AppendItem( m_mainFrame->m_tPointId, wxT( "point" ), -1, -1, l_point );
//                 l_pNode = l_pNode->GetNext();
//             }
// 
//             if( m_mainFrame->m_pTreeWidget->GetChildrenCount( m_mainFrame->m_tPointId ) > 0 )
//             {
//                 Surface* l_surface = new Surface( this );
// #ifdef __WXMAC__
//                 // insert at zero is a well-known bug on OSX, so we append there...
//                 // http://trac.wxwidgets.org/ticket/4492
//                 long l_id = m_mainFrame->m_pListCtrl->GetItemCount();
// #else
//                 long l_id = 0;
// #endif
//     
//                 m_mainFrame->m_pListCtrl2->InsertItem( l_surface );
// 
//                 m_mainFrame->m_pListCtrl->InsertItem( l_id, wxT( "" ), 0 );
//                 m_mainFrame->m_pListCtrl->SetItem( l_id, 1, _T( "spline surface" ) );
//                 m_mainFrame->m_pListCtrl->SetItem( l_id, 2, wxT( "0.50" ) );
//                 m_mainFrame->m_pListCtrl->SetItem( l_id, 3, wxT( "" ), 1 );
//                 m_mainFrame->m_pListCtrl->SetItemData( l_id, (long)l_surface );
//                 m_mainFrame->m_pListCtrl->SetItemState( l_id, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
//             }
//         }
//         else if( l_child->GetName() == wxT( "selection_objects" ) )
//         {
//             wxXmlNode* l_boxNode = l_child->GetChildren();
//             wxTreeItemId l_currentMasterId;
// 
//             wxString l_name, l_type, l_active, l_visible, l_isBox;
//             double cx, cy, cz, ix, iy, iz;
//             double _cx, _cy, _cz, _ix, _iy, _iz;
//             cx = cy = cz = ix = iy = iz = 0;
//             _cx = _cy = _cz = _ix = _iy = _iz = 0;
// 
//             while( l_boxNode )
//             {
//                 wxXmlNode* l_infoNode = l_boxNode->GetChildren();
//                 while( l_infoNode )
//                 {
//                     if( l_infoNode->GetName() == wxT( "status" ) )
//                     {
//                         l_type    = l_infoNode->GetPropVal( wxT( "type" ),    wxT( "MASTER" ) );
//                         l_active  = l_infoNode->GetPropVal( wxT( "active" ),  wxT( "yes" ) );
//                         l_visible = l_infoNode->GetPropVal( wxT( "visible" ), wxT( "yes" ) );
//                         l_isBox   = l_infoNode->GetPropVal( wxT( "isBox" ), wxT( "yes" ) );
// 
//                     }
//                     if( l_infoNode->GetName() == wxT( "name" ) )
//                     {
//                         l_name = l_infoNode->GetPropVal( wxT( "string" ), wxT( "object" ) );
// 
//                     }
//                     if( l_infoNode->GetName() == wxT( "size" ) )
//                     {
//                         wxString sx = l_infoNode->GetPropVal( wxT( "x" ), wxT( "0.0" ) );
//                         wxString sy = l_infoNode->GetPropVal( wxT( "y" ), wxT( "0.0" ) );
//                         wxString sz = l_infoNode->GetPropVal( wxT( "z" ), wxT( "0.0" ) );
// 
//                         sx.ToDouble( &_ix );
//                         sy.ToDouble( &_iy );
//                         sz.ToDouble( &_iz );
//                     }
//                     if( l_infoNode->GetName() == wxT( "center" ) )
//                     {
//                         wxString sx = l_infoNode->GetPropVal( wxT( "x" ), wxT( "0.0" ) );
//                         wxString sy = l_infoNode->GetPropVal( wxT( "y" ), wxT( "0.0" ) );
//                         wxString sz = l_infoNode->GetPropVal( wxT( "z" ), wxT( "0.0" ) );
// 
//                         sx.ToDouble( &_cx );
//                         sy.ToDouble( &_cy );
//                         sz.ToDouble( &_cz );
//                     }
// 
//                     l_infoNode = l_infoNode->GetNext();
//                 }
// 
//                 Vector l_vc( _cx, _cy, _cz );
//                 Vector l_vs( _ix, _iy, _iz );
// 
//                 // get selected l_anatomy dataset
//                 long l_item = m_mainFrame->m_pListCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
//                 if( l_item == -1 )
//                     return false;
// 
//                 //DatasetInfo* l_info = (DatasetInfo*) m_mainFrame->m_listCtrl->GetItemData( l_item );
//                 //if( l_info->getType() > OVERLAY )
//                 //    return false;
//                 SelectionObject* l_selectionObject;
//                 if( l_isBox == _T( "yes" ) )
//                     l_selectionObject = new SelectionBox( l_vc, l_vs, this );
//                 else
//                     l_selectionObject = new SelectionEllipsoid( l_vc, l_vs, this );
// 
//                 l_selectionObject->setName( l_name );
//                 l_selectionObject->setIsActive ( l_active  == _T( "yes" ) );
//                 l_selectionObject->setIsVisible( l_visible == _T( "yes" ) );
// 
//                 if( l_type == wxT( "MASTER" ) )
//                 {
//                     l_selectionObject->setIsMaster( true );
//                     l_currentMasterId = m_mainFrame->m_pTreeWidget->AppendItem( m_mainFrame->m_tSelectionObjectsId, l_selectionObject->getName(), 0, -1, l_selectionObject );
//                     m_mainFrame->m_pTreeWidget->EnsureVisible( l_currentMasterId );
//                     m_mainFrame->m_pTreeWidget->SetItemImage( l_currentMasterId, l_selectionObject->getIcon() );
//                     m_mainFrame->m_pTreeWidget->SetItemBackgroundColour( l_currentMasterId, *wxCYAN );
//                     l_selectionObject->setTreeId( l_currentMasterId );
//                 }
//                 else
//                 {
//                     l_selectionObject->setIsNOT( l_type == _T( "NOT" ) );
//                     wxTreeItemId boxId = m_mainFrame->m_pTreeWidget->AppendItem( l_currentMasterId, l_selectionObject->getName(), 0, -1, l_selectionObject );
//                     m_mainFrame->m_pTreeWidget->EnsureVisible( boxId );
//                     m_mainFrame->m_pTreeWidget->SetItemImage( boxId, l_selectionObject->getIcon() );
// 
//                     if( l_selectionObject->getIsNOT() )
//                         m_mainFrame->m_pTreeWidget->SetItemBackgroundColour( boxId, *wxRED );
//                     else
//                         m_mainFrame->m_pTreeWidget->SetItemBackgroundColour( boxId, *wxGREEN );
// 
//                     l_selectionObject->setTreeId( boxId );
//                 }
//                 l_boxNode = l_boxNode->GetNext();
//             }
//         }
//         l_child = l_child->GetNext();
//     }
// 
//     m_mainFrame->m_pXSlider->SetValue( xp );
//     m_mainFrame->m_pYSlider->SetValue( yp );
//     m_mainFrame->m_pZSlider->SetValue( zp );
//     updateView( xp, yp, zp );
// 
//     /*m_transform.s.M00 = r00;
//     m_transform.s.M10 = r10;
//     m_transform.s.M20 = r20;
//     m_transform.s.M01 = r01;
//     m_transform.s.M11 = r11;
//     m_transform.s.M21 = r21;
//     m_transform.s.M02 = r02;
//     m_transform.s.M12 = r12;
//     m_transform.s.M22 = r22;
//     m_mainFrame->m_mainGL->setRotation();*/
// 
//     updateLoadStatus();
//     return true;
}

//////////////////////////////////////////////////////////////////////////

bool SceneManager::save( const wxString &filename )
{
    return false;

    //     wxXmlNode* l_root                 = new wxXmlNode( NULL,   wxXML_ELEMENT_NODE, wxT( "theScene" ) );
    //     wxXmlNode* l_nodeSelectionObjects = new wxXmlNode( l_root, wxXML_ELEMENT_NODE, wxT( "selection_objects" ) );
    //     wxXmlNode* l_nodePoints           = new wxXmlNode( l_root, wxXML_ELEMENT_NODE, wxT( "points" ) );
    //     wxXmlNode* l_data                 = new wxXmlNode( l_root, wxXML_ELEMENT_NODE, wxT( "data" ) );
    //     wxXmlNode* l_anatomy              = new wxXmlNode( l_root, wxXML_ELEMENT_NODE, wxT( "anatomy" ) );
    //     wxXmlNode* l_rotation             = new wxXmlNode( l_root, wxXML_ELEMENT_NODE, wxT( "rotation" ) );
    //     wxXmlNode* l_anatomyPos           = new wxXmlNode( l_root, wxXML_ELEMENT_NODE, wxT( "position" ) );
    // 
    //     wxXmlProperty* l_prop1 = new wxXmlProperty( wxT( "rows" ),    wxString::Format( wxT( "%d" ), m_rows ) );
    //     wxXmlProperty* l_prop2 = new wxXmlProperty( wxT( "columns" ), wxString::Format( wxT( "%d" ), m_columns ), l_prop1 );
    //     wxXmlProperty* l_prop3 = new wxXmlProperty( wxT( "frames" ),  wxString::Format( wxT( "%d" ), m_frames ), l_prop2 );
    //     l_anatomy->AddProperty( l_prop3 );
    // 
    //     wxXmlProperty* l_rot00 = new wxXmlProperty( wxT( "rot00" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M00 ) );
    //     wxXmlProperty* l_rot10 = new wxXmlProperty( wxT( "rot10" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M10 ), l_rot00 );
    //     wxXmlProperty* l_rot20 = new wxXmlProperty( wxT( "rot20" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M20 ), l_rot10 );
    //     wxXmlProperty* l_rot01 = new wxXmlProperty( wxT( "rot01" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M01 ), l_rot20 );
    //     wxXmlProperty* l_rot11 = new wxXmlProperty( wxT( "rot11" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M11 ), l_rot01 );
    //     wxXmlProperty* l_rot21 = new wxXmlProperty( wxT( "rot21" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M21 ), l_rot11 );
    //     wxXmlProperty* l_rot02 = new wxXmlProperty( wxT( "rot02" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M02 ), l_rot21 );
    //     wxXmlProperty* l_rot12 = new wxXmlProperty( wxT( "rot12" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M12 ), l_rot02 );
    //     wxXmlProperty* l_rot22 = new wxXmlProperty( wxT( "rot22" ), wxString::Format( wxT( "%.8f" ), m_transform.s.M22 ), l_rot12 );
    //     l_rotation->AddProperty( l_rot22 );
    // 
    //     int l_countPoints = m_mainFrame->m_pTreeWidget->GetChildrenCount( m_mainFrame->m_tPointId, true );
    //     wxTreeItemId l_id, l_childId;
    //     wxTreeItemIdValue l_cookie = 0;
    // 
    //     for( int i = 0; i < l_countPoints; ++i )
    //     {
    //         l_id = m_mainFrame->m_pTreeWidget->GetNextChild( m_mainFrame->m_tPointId, l_cookie );
    //         SplinePoint* l_point = (SplinePoint*)( m_mainFrame->m_pTreeWidget->GetItemData( l_id ) );
    //         wxXmlNode* l_pointNode = new wxXmlNode( l_nodePoints, wxXML_ELEMENT_NODE, wxT( "point" ) );
    // 
    //         wxXmlProperty* l_propZ = new wxXmlProperty( wxT( "z" ), wxString::Format( wxT( "%f" ), l_point->getCenter().z ) );
    //         wxXmlProperty* l_propY = new wxXmlProperty( wxT( "y" ), wxString::Format( wxT( "%f" ), l_point->getCenter().y ), l_propZ );
    //         wxXmlProperty* l_propX = new wxXmlProperty( wxT( "x" ), wxString::Format( wxT( "%f" ), l_point->getCenter().x ), l_propY );
    //         l_pointNode->AddProperty( l_propX );
    //     }
    // 
    //     SelectionObject* l_currentSelectionObject;
    //     std::vector< std::vector< SelectionObject* > > l_selectionObjects = getSelectionObjects();
    // 
    //     for( unsigned int i = l_selectionObjects.size(); i > 0; --i )
    //     {
    //         for( unsigned int j = l_selectionObjects[i - 1].size(); j > 0; --j )
    //         {
    //             wxXmlNode* l_selectionObject = new wxXmlNode( l_nodeSelectionObjects, wxXML_ELEMENT_NODE, wxT( "object" ) );
    //             l_currentSelectionObject = l_selectionObjects[i - 1][j - 1];
    // 
    //             if( ! l_currentSelectionObject->isSelectionObject() )
    //                 continue;
    // 
    //             wxXmlNode* l_center    = new wxXmlNode( l_selectionObject, wxXML_ELEMENT_NODE, wxT( "center" ) );
    //             wxXmlProperty *l_propZ = new wxXmlProperty( wxT( "z" ), wxString::Format( wxT( "%f" ), l_currentSelectionObject->getCenter().z ) );
    //             wxXmlProperty *l_propY = new wxXmlProperty( wxT( "y" ), wxString::Format( wxT( "%f" ), l_currentSelectionObject->getCenter().y ), l_propZ );
    //             wxXmlProperty *l_propX = new wxXmlProperty( wxT( "x" ), wxString::Format( wxT( "%f" ), l_currentSelectionObject->getCenter().x ), l_propY );
    //             l_center->AddProperty( l_propX );
    // 
    //             wxXmlNode* l_size = new wxXmlNode( l_selectionObject, wxXML_ELEMENT_NODE, wxT( "size" ) );
    //             l_propZ = new wxXmlProperty( wxT( "z" ), wxString::Format( wxT( "%f" ), l_currentSelectionObject->getSize().z ) );
    //             l_propY = new wxXmlProperty( wxT( "y" ), wxString::Format( wxT( "%f" ), l_currentSelectionObject->getSize().y ), l_propZ );
    //             l_propX = new wxXmlProperty( wxT( "x" ), wxString::Format( wxT( "%f" ), l_currentSelectionObject->getSize().x ), l_propY );
    //             l_size->AddProperty( l_propX );
    // 
    //             wxXmlNode* l_name = new wxXmlNode( l_selectionObject, wxXML_ELEMENT_NODE, wxT( "name" ) );
    //             wxXmlProperty* l_propName = new wxXmlProperty( wxT( "string" ), l_currentSelectionObject->getName() );
    //             l_name->AddProperty( l_propName );
    // 
    //             wxXmlNode *status = new wxXmlNode( l_selectionObject, wxXML_ELEMENT_NODE, wxT( "status" ) );
    //             wxXmlProperty *l_propType;
    // 
    //             if( j - 1 == 0 )
    //                 l_propType = new wxXmlProperty( wxT( "type" ), wxT( "MASTER" ) );
    //             else
    //                 l_propType = new wxXmlProperty( wxT( "type" ), l_currentSelectionObject->getIsNOT() ? wxT( "NOT" ) : wxT( "AND" ) );
    // 
    //             wxXmlProperty* l_propActive  = new wxXmlProperty( wxT( "active" ),  l_currentSelectionObject->getIsActive()       ? wxT( "yes" ) : wxT( "no" ), l_propType );
    //             wxXmlProperty* l_propVisible = new wxXmlProperty( wxT( "visible" ), l_currentSelectionObject->getIsVisible()      ? wxT( "yes" ) : wxT( "no" ), l_propActive );
    //             wxXmlProperty* l_propIsBox   = new wxXmlProperty( wxT( "isBox" ),   (l_currentSelectionObject->getSelectionType() == BOX_TYPE) ? wxT( "yes" ) : wxT( "no" ), l_propVisible );
    //             
    //             status->AddProperty( l_propIsBox );
    //         }
    //     }
    // 
    //     int l_countTextures = m_mainFrame->m_pListCtrl2->GetItemCount();
    // 
    //     if( l_countTextures == 0 )
    //         return;
    // 
    // 	
    // 	std::vector<int> anatomyPositions;
    // 	
    //     for( int i( l_countTextures - 1 ); i >= 0; --i )
    //     {
    // 		DatasetInfo* l_info = m_mainFrame->m_pListCtrl2->GetItem( i );
    // 		
    // 		if(l_info->getType() < MESH)
    // 		{
    // 			anatomyPositions.push_back(i);
    // 			continue;
    // 		}
    // 		
    //         if( l_info->getType() < SURFACE)
    //         {
    //             wxXmlNode* l_dataSetNode = new wxXmlNode( l_data, wxXML_ELEMENT_NODE, wxT( "dataset" ) );
    // 
    //             wxXmlNode* l_pathNode = new wxXmlNode( l_dataSetNode, wxXML_ELEMENT_NODE, wxT( "path" ) );
    //             new wxXmlNode( l_pathNode, wxXML_TEXT_NODE, wxT( "path" ), l_info->getPath() );
    // 
    //             wxXmlNode* l_statusNode = new wxXmlNode( l_dataSetNode, wxXML_ELEMENT_NODE, wxT( "status" ) );
    // 			
    // 			wxXmlProperty* l_propP  = new wxXmlProperty( wxT( "position" ), wxString::Format( wxT( "%ld" ), l_info->getListCtrlItemId() ) );
    //             wxXmlProperty* l_propT  = new wxXmlProperty( wxT( "threshold" ), wxString::Format( wxT( "%.2f" ), l_info->getThreshold() ), l_propP );
    //             wxXmlProperty* l_propTA = new wxXmlProperty( wxT( "alpha" ), wxString::Format( wxT( "%.2f" ), l_info->getAlpha() ), l_propT );
    //             wxXmlProperty* l_propA  = new wxXmlProperty( wxT( "active" ), l_info->getShow()   ? _T( "yes" ) : _T( "no" ), l_propTA );
    //             wxXmlProperty* l_propF  = new wxXmlProperty( wxT( "showFS" ), l_info->getShowFS() ? _T( "yes" ) : _T( "no" ), l_propA );
    //             wxXmlProperty* l_propU  = new wxXmlProperty( wxT( "useTex" ), l_info->getUseTex() ? _T( "yes" ) : _T( "no" ), l_propF );
    // 			wxXmlProperty* l_propN	= new wxXmlProperty( wxT( "name" ), l_info->getName().BeforeFirst( '.' ), l_propU );
    // 			wxXmlProperty* l_propI	= new wxXmlProperty( wxT( "isFiberGroup" ), _T( "no" ), l_propN );
    //             l_statusNode->AddProperty( l_propI );
    //         }
    // 		if(l_info->getType() == FIBERSGROUP)
    // 		{
    // 			wxXmlNode* l_dataSetNode = new wxXmlNode( l_data, wxXML_ELEMENT_NODE, wxT( "dataset" ) );
    // 			
    //             wxXmlNode* l_statusNode = new wxXmlNode( l_dataSetNode, wxXML_ELEMENT_NODE, wxT( "status" ) );
    // 
    // 			wxXmlProperty* l_propP  = new wxXmlProperty( wxT( "position" ), wxString::Format( wxT( "%ld" ), l_info->getListCtrlItemId() ) );
    //             wxXmlProperty* l_propA  = new wxXmlProperty( wxT( "active" ), l_info->getShow()   ? _T( "yes" ) : _T( "no" ), l_propP );
    //             wxXmlProperty* l_propF  = new wxXmlProperty( wxT( "showFS" ), l_info->getShowFS() ? _T( "yes" ) : _T( "no" ), l_propA );
    //             wxXmlProperty* l_propU  = new wxXmlProperty( wxT( "useTex" ), l_info->getUseTex() ? _T( "yes" ) : _T( "no" ), l_propF );
    // 			wxXmlProperty* l_propN	= new wxXmlProperty( wxT( "name" ), l_info->getName().BeforeFirst( '.' ), l_propU );
    //             wxXmlProperty* l_propI	= new wxXmlProperty( wxT( "isFiberGroup" ), _T( "yes" ), l_propN );
    // 			l_statusNode->AddProperty( l_propI );
    // 		}
    //     }
    // 	
    // 	// Save anatomies at the end so they would always be at the beginning of data
    // 	for( int i = 0; i < (int)anatomyPositions.size(); i++ )
    //     {
    // 		DatasetInfo* l_info = (DatasetInfo*) m_mainFrame->m_pListCtrl->GetItemData( anatomyPositions[i] );
    // 		
    // 		wxXmlNode* l_dataSetNode = new wxXmlNode( l_data, wxXML_ELEMENT_NODE, wxT( "dataset" ) );
    // 		
    // 		wxXmlNode* l_pathNode = new wxXmlNode( l_dataSetNode, wxXML_ELEMENT_NODE, wxT( "path" ) );
    // 		new wxXmlNode( l_pathNode, wxXML_TEXT_NODE, wxT( "path" ), l_info->getPath() );
    // 		
    // 		wxXmlNode* l_statusNode = new wxXmlNode( l_dataSetNode, wxXML_ELEMENT_NODE, wxT( "status" ) );
    // 		
    // 		wxXmlProperty* l_propId  = new wxXmlProperty( wxT( "position" ), wxString::Format( wxT( "%ld" ), l_info->getListCtrlItemId() ) );
    // 		wxXmlProperty* l_propT  = new wxXmlProperty( wxT( "threshold" ), wxString::Format( wxT( "%.2f" ), l_info->getThreshold() ), l_propId );
    // 		wxXmlProperty* l_propTA = new wxXmlProperty( wxT( "alpha" ), wxString::Format( wxT( "%.2f" ), l_info->getAlpha() ), l_propT );
    // 		wxXmlProperty* l_propA  = new wxXmlProperty( wxT( "active" ), l_info->getShow()   ? _T( "yes" ) : _T( "no" ), l_propTA );
    // 		wxXmlProperty* l_propF  = new wxXmlProperty( wxT( "showFS" ), l_info->getShowFS() ? _T( "yes" ) : _T( "no" ), l_propA );
    // 		wxXmlProperty* l_propU  = new wxXmlProperty( wxT( "useTex" ), l_info->getUseTex() ? _T( "yes" ) : _T( "no" ), l_propF );
    // 		wxXmlProperty* l_propN	= new wxXmlProperty( wxT( "name" ), l_info->getName().BeforeFirst( '.' ), l_propU );
    // 		wxXmlProperty* l_propI	= new wxXmlProperty( wxT( "isFiberGroup" ), _T( "no" ), l_propN );
    // 		l_statusNode->AddProperty( l_propI );
    // 	}
    // 
    //     wxXmlProperty* l_propPosX = new wxXmlProperty( wxT( "x" ), wxString::Format( wxT( "%d" ), m_mainFrame->m_pXSlider->GetValue() ) );
    //     wxXmlProperty* l_propPosY = new wxXmlProperty( wxT( "y" ), wxString::Format( wxT( "%d" ), m_mainFrame->m_pYSlider->GetValue() ), l_propPosX );
    //     wxXmlProperty* l_propPosZ = new wxXmlProperty( wxT( "z" ), wxString::Format( wxT( "%d" ), m_mainFrame->m_pZSlider->GetValue() ), l_propPosY );
    //     l_anatomyPos->AddProperty( l_propPosZ );
    // 
    //     wxXmlDocument l_xmlDoc;
    //     l_xmlDoc.SetRoot( l_root );
    // 	
    // 	l_xmlDoc.SetVersion( _T("2.0") );
    // 
    //     if ( i_fileName.AfterLast( '.' ) != _T( "scn" ) )
    //         l_xmlDoc.Save( i_fileName + _T( ".scn" ), 2 );
    //     else
    //         l_xmlDoc.Save( i_fileName, 2 );
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
    // Semaphore was removed from datasethelper. Don't know its use. If necessary,
    // add a default bool at the end of this method to act as the semaphore.
    // Should be sufficient
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
    Logger::getInstance()->print( wxT( "Executing SceneManager destructor" ), LOGLEVEL_DEBUG );
    if( m_pTheScene )
    {
        delete m_pTheScene;
        m_pTheScene = NULL;
    }

    if( m_pShaderHelper )
    {
        delete m_pShaderHelper;
        m_pShaderHelper = NULL;
    }

    m_pInstance = NULL;
    Logger::getInstance()->print( wxT( "SceneManager destructor done" ), LOGLEVEL_DEBUG );
}
