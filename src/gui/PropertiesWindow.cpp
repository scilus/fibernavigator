#include "ListCtrl.h"
#include "MainFrame.h"
#include "PropertiesWindow.h"
#include "SceneManager.h"
#include "SelectionBox.h"
#include "SelectionEllipsoid.h"
#include "SelectionTree.h"
#include "TrackingWindow.h"
#include "../Logger.h"
#include "../main.h"
#include "../dataset/Anatomy.h"
#include "../dataset/DatasetManager.h"
#include "../dataset/Fibers.h"
#include "../dataset/FibersGroup.h"
#include "../dataset/ODFs.h"
#include "../dataset/RTTrackingHelper.h"
#include "../dataset/Tensors.h"
#include "../dataset/Maximas.h"
#include "../gui/SelectionVOI.h"
#include "../misc/IsoSurface/CIsoSurface.h"
#include "../misc/IsoSurface/TriangleMesh.h"

#include <wx/colordlg.h>
#include <wx/notebook.h>

#include <algorithm>

IMPLEMENT_DYNAMIC_CLASS(PropertiesWindow, wxScrolledWindow)

BEGIN_EVENT_TABLE( PropertiesWindow, wxScrolledWindow )
// EVT_PAINT( PropertiesWindow::OnPaint )
// EVT_SIZE( PropertiesWindow::OnSize )
END_EVENT_TABLE()

PropertiesWindow::PropertiesWindow( wxWindow *pParent, MainFrame *pMainFrame, wxWindowID id, const wxPoint &pos, const wxSize &size, ListCtrl *lstCtrl )
:   wxScrolledWindow( pParent, id, pos, size, wxBORDER_NONE, _T( "Properties Window" ) ),
    m_pNotebook( pParent ),
    m_pMainFrame( pMainFrame ),
    m_pListCtrl( lstCtrl )
{
    SetBackgroundColour( *wxLIGHT_GREY );
//     SetCursor( wxCursor( wxCURSOR_HAND ) );
    SetSizer( new wxBoxSizer( wxVERTICAL ) );
    SetAutoLayout( true );
}

// void PropertiesWindow::OnSize( wxSizeEvent &WXUNUSED(event) )
// {
// }
// 
// void PropertiesWindow::OnPaint( wxPaintEvent &WXUNUSED(event) )
// {
//     wxPaintDC dc( this );
// }

void PropertiesWindow::OnListItemDown( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnListItemDown" ), LOGLEVEL_DEBUG );

    m_pListCtrl->MoveItemDown();
}

void PropertiesWindow::OnListItemUp(wxCommandEvent& WXUNUSED(event))
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnListItemUp" ), LOGLEVEL_DEBUG );

    m_pListCtrl->MoveItemUp();
}

void PropertiesWindow::OnDeleteListItem( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnDeleteListItem" ), LOGLEVEL_DEBUG );

    m_pMainFrame->deleteListItem();
}

void PropertiesWindow::OnToggleIntensityBtn( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleIntensityBtn" ), LOGLEVEL_DEBUG );

    if (m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject);
        if( pDatasetInfo != NULL)
        {
            FibersGroup* pFibersGroup = DatasetManager::getInstance()->getFibersGroup();
            if( pFibersGroup )
            {
                pFibersGroup->OnToggleIntensityBtn();
            }
        }
    }
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnToggleOpacityBtn( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleOpacityBtn" ), LOGLEVEL_DEBUG );

    if (m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject);
        if( pDatasetInfo != NULL)
        {
            FibersGroup* pFibersGroup = DatasetManager::getInstance()->getFibersGroup();
            if( pFibersGroup )
            {
                pFibersGroup->OnToggleOpacityBtn();
            }
        }
    }
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnToggleMinMaxLengthBtn( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleMinMaxLengthBtn" ), LOGLEVEL_DEBUG );

    if (m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject);
        if( pDatasetInfo != NULL)
        {
            FibersGroup* pFibersGroup = DatasetManager::getInstance()->getFibersGroup();
            if( pFibersGroup )
            {
                pFibersGroup->OnToggleMinMaxLengthBtn();
            }
        }
    }
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnToggleSubsamplingBtn( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleSubsamplingBtn" ), LOGLEVEL_DEBUG );

    if (m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject);
        if( pDatasetInfo != NULL)
        {
            FibersGroup* pFibersGroup = DatasetManager::getInstance()->getFibersGroup();
            if( pFibersGroup )
            {
                pFibersGroup->OnToggleSubsamplingBtn();
            }
        }
    }
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnToggleCrossingFibersBtn( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleCrossingFibersBtn" ), LOGLEVEL_DEBUG );

    if (m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject);
        if( pDatasetInfo != NULL)
        {
            FibersGroup* pFibersGroup = DatasetManager::getInstance()->getFibersGroup();
            if(pFibersGroup)
            {
                pFibersGroup->OnToggleCrossingFibersBtn();
            }
        }
    }
}

void PropertiesWindow::OnToggleColorModeBtn( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleColorModeBtn" ), LOGLEVEL_DEBUG );

    if (m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject);
        if( pDatasetInfo != NULL)
        {
            FibersGroup* pFibersGroup = DatasetManager::getInstance()->getFibersGroup();
            if(pFibersGroup)
            {
                pFibersGroup->OnToggleColorModeBtn();
            }
        }
    }
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnToggleLocalColoringBtn( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleLocalColoringBtn" ), LOGLEVEL_DEBUG );

    if (m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject);
        if( pDatasetInfo != NULL)
        {
            FibersGroup* pFibersGroup = DatasetManager::getInstance()->getFibersGroup();
            if( pFibersGroup )
            {
                pFibersGroup->OnToggleLocalColoring();
            }
        }
    }
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnToggleNormalColoringBtn( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleNormalColoringBtn" ), LOGLEVEL_DEBUG );

    if (m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject);
        if( pDatasetInfo != NULL)
        {
            FibersGroup* pFibersGroup = DatasetManager::getInstance()->getFibersGroup();
            if(pFibersGroup)
            {
                pFibersGroup->OnToggleNormalColoring();
            }
        }
    }
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnApplyDifferentColors( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnApplyDifferentColors" ), LOGLEVEL_DEBUG );
    
    if (m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject);
        if( pDatasetInfo != NULL)
        {
            FibersGroup* pFibersGroup = DatasetManager::getInstance()->getFibersGroup();
            if(pFibersGroup)
            {
                pFibersGroup->OnApplyDifferentColors();
            }
        }
    }
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnMergeVisibleFibers( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnMergeVisibleFibers" ), LOGLEVEL_DEBUG );

    vector<Fibers *> bundles;
    vector<long> indicesToRemove;
    
    // Search for currently visible bundles (Fibers objects)
    for( unsigned int index= m_pMainFrame->m_pListCtrl->GetItemCount()-1; index > 0; --index )
    {
        DatasetInfo* pDatasetInfo = DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( index ) );

        // Check if the list item is a currently visible Fibers object
        if( pDatasetInfo->getType() == FIBERS && pDatasetInfo->getShow() )
        {
            bundles.push_back( (Fibers*)pDatasetInfo );
            indicesToRemove.push_back( index );
        }
    }

    if( bundles.empty() )
    {
        return;
    }

    // Create merged bundle (Fibers object)
    Fibers* pFibers = new Fibers();
    pFibers->createFrom( bundles, wxT("Merged") );

    // Remove bundles (Fibers objects) being merged
    for (std::vector<long>::iterator it = indicesToRemove.begin(); it != indicesToRemove.end(); ++it)
    {
        m_pMainFrame->m_pListCtrl->DeleteItem( *it );
    }

    // Insert the merged bundle (Fibers object)
    DatasetIndex index = DatasetManager::getInstance()->addFibers( pFibers );
    m_pMainFrame->m_pListCtrl->InsertItem( index );

    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnClickApplyBtn( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnClickApplyBtn" ), LOGLEVEL_DEBUG );

    if (m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject);
        if( pDatasetInfo != NULL)
        {
            FibersGroup* pFibersGroup = DatasetManager::getInstance()->getFibersGroup();
            if(pFibersGroup)
            {
                pFibersGroup->OnClickApplyBtn();
                m_pListCtrl->UpdateFibers();
            }
        }
    }
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnClickCancelBtn( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnClickCancelBtn" ), LOGLEVEL_DEBUG );

    if (m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject);
        if( pDatasetInfo != NULL)
        {
            FibersGroup* pFibersGroup = DatasetManager::getInstance()->getFibersGroup();
            if(pFibersGroup)
            {
                pFibersGroup->OnClickCancelBtn();
            }
        }
    }
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnClickGenerateFiberVolumeBtn( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnClickGenerateFiberVolumeBtn" ), LOGLEVEL_DEBUG );

    if (m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject);
        if( pDatasetInfo != NULL)
        {
            FibersGroup* pFibersGroup = DatasetManager::getInstance()->getFibersGroup();
            if(pFibersGroup)
            {
                pFibersGroup->OnClickGenerateFiberVolumeBtn();
            }
        }
    }
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnToggleShowFS( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleShowFS" ), LOGLEVEL_DEBUG );

    if( NULL != m_pMainFrame->m_pCurrentSceneObject&& -1 != m_pMainFrame->m_currentListIndex )
    {
        ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject)->toggleShowFS();
        m_pListCtrl->UpdateSelected();
        m_pMainFrame->refreshAllGLWidgets();
    }
}

void PropertiesWindow::OnToggleVisibility( wxCommandEvent&  WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleVisibility" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject == NULL && m_pMainFrame->m_currentListIndex != -1)
        return;

    
    DatasetIndex index = m_pListCtrl->GetItem( m_pMainFrame->m_currentListIndex );
    
    if( index.isOk() )
    {
        DatasetInfo* pInfo = DatasetManager::getInstance()->getDataset( index );
        pInfo->toggleShow();

        if( FIBERSGROUP == pInfo->getType() )
        {
            FibersGroup* pFibersGroup = (FibersGroup*)pInfo;
            if( NULL != pFibersGroup )
            {
                pFibersGroup->OnToggleVisibleBtn();
            }
        }
    }

    m_pListCtrl->UpdateSelected();

    m_pMainFrame->refreshAllGLWidgets();
}


void PropertiesWindow::OnSliderIntensityThresholdMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnSliderIntensityThresholdMoved" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        DatasetInfo* l_current = (DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject;
        float l_threshold = (float)l_current->m_pSliderThresholdIntensity->GetValue() / 100.0f;
        l_current->setThreshold( l_threshold );

        if( l_current->getType() == ISO_SURFACE && ! l_current->m_pSliderThresholdIntensity->leftDown() )
        {
            CIsoSurface* s = (CIsoSurface*)l_current;
            s->GenerateWithThreshold();
			RTTrackingHelper::getInstance()->setRTTDirty( true );
            
            std::vector< Vector > positions = s->m_tMesh->getVerts();
            float shellSeedNb = positions.size();
            m_pMainFrame->m_pTrackingWindow->m_pTxtTotalSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), shellSeedNb) );
            

        }
        else if( l_current->getType() < RGB )
        {
            Anatomy* a = (Anatomy*)l_current;
            if( a->m_pRoi )
                a->m_pRoi->setThreshold( l_threshold );
			
        }

        // This slider will set the Brightness level. Currently only the glyphs uses this value.
        l_current->setBrightness( 1.0f - l_threshold );

        m_pListCtrl->UpdateSelected();

        m_pMainFrame->refreshAllGLWidgets();
    }
}

void PropertiesWindow::OnSliderOpacityThresholdMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnSliderOpacityThresholdMoved" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        DatasetInfo* l_current = (DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject;
        l_current->setAlpha( (float)l_current->m_pSliderOpacity->GetValue() / 100.0f);
        m_pMainFrame->refreshAllGLWidgets();
    }
}


void PropertiesWindow::OnEqualizeDataset( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnEqualizeDataset" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        if( ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject)->getType() < MESH )
        {
            ((Anatomy*)m_pMainFrame->m_pCurrentSceneObject)->toggleEqualization();
        }
    }
}

void PropertiesWindow::OnEqualizationSliderChange( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnEqualizationSliderChange" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        if( ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject)->getType() < MESH )
        {
            ((Anatomy*)m_pMainFrame->m_pCurrentSceneObject)->equalizationSliderChange();
        }
    }
}

void PropertiesWindow::OnRename( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnRename" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        wxTextEntryDialog dialog( this, _T( "Please enter a new name" ) );
        DatasetInfo* pInfo = (DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject;

        dialog.SetValue( pInfo->getName().BeforeFirst( '.' ) );

        wxString ext = pInfo->getName().AfterFirst( '.' );

        if( ( dialog.ShowModal() == wxID_OK ) && ( dialog.GetValue() != _T( "" ) ) )
        {
            pInfo->setName( dialog.GetValue() + wxT( "." ) + ext );
            pInfo->m_pTxtName->SetLabel( pInfo->getName() );

            m_pListCtrl->UpdateSelected();
        }
    }

    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnFlipX( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnFlipX" ), LOGLEVEL_DEBUG );

    ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject)->flipAxis( X_AXIS );
}

void PropertiesWindow::OnFlipY( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnFlipY" ), LOGLEVEL_DEBUG );

    ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject)->flipAxis(Y_AXIS);
}

void PropertiesWindow::OnFlipZ( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnFlipZ" ), LOGLEVEL_DEBUG );

    ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject)->flipAxis(Z_AXIS);
}

void PropertiesWindow::OnDilateDataset( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnDilateDataset" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        if( ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject)->getType() < MESH )
        {
            ((Anatomy*)m_pMainFrame->m_pCurrentSceneObject)->dilate();
        }
    }
}

void PropertiesWindow::OnErodeDataset(wxCommandEvent& WXUNUSED(event))
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnErodeDataset" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        if( ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject)->getType() < MESH )
        {
            ((Anatomy*)m_pMainFrame->m_pCurrentSceneObject)->erode();
        }
    }
}

void PropertiesWindow::OnMinimizeDataset( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnMinimizeDataset" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        if( ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject)->getType() < MESH )
        {
            ((Anatomy*)m_pMainFrame->m_pCurrentSceneObject)->minimize();
        }
    }
}

void PropertiesWindow::OnListItemCutOut( wxCommandEvent&  WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnListItemCutOut" ), LOGLEVEL_DEBUG );

    m_pMainFrame->createCutDataset();
}

void PropertiesWindow::OnNewIsoSurface( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnNewIsoSurface" ), LOGLEVEL_DEBUG );

    m_pMainFrame->createIsoSurface();
}

void PropertiesWindow::OnNewOffsetSurface( wxCommandEvent& WXUNUSED(event ))
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnNewOffsetSurface" ), LOGLEVEL_DEBUG );

    m_pMainFrame->createDistanceMapAndIso();
}

void PropertiesWindow::OnNewDistanceMap (wxCommandEvent& WXUNUSED(event))
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnNewDistanceMap" ), LOGLEVEL_DEBUG );

    m_pMainFrame->createDistanceMap();
}

void PropertiesWindow::OnNewVoiFromOverlay( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnNewVoiFromOverlay" ), LOGLEVEL_DEBUG );
    
    SelectionObject *pSelectionObject  = NULL;
    Anatomy         *pAnatomy          = NULL;
    
    if(m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        if ( ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject)->getType() < RGB)
        {
            pAnatomy = (Anatomy*)m_pMainFrame->m_pCurrentSceneObject;
            float trs = pAnatomy->getThreshold();
            if( trs == 0.0 )
                trs = 0.01f;
            
            pSelectionObject = new SelectionVOI( pAnatomy, trs, THRESHOLD_GREATER_EQUAL );
        }
        else
        {
            return;
        }
    }
    else
    {
        return;
    }
        
    wxTreeItemId newSelectionObjectId;
    
    SelectionTree &selTree = SceneManager::getInstance()->getSelectionTree();
    
    SelectionObject *pCurObj = m_pMainFrame->getCurrentSelectionObject();
    
    if( selTree.isEmpty() || pCurObj == NULL )
    {
        pSelectionObject->setIsFirstLevel( true );
        int itemId = selTree.addChildrenObject( -1, pSelectionObject );
        
        CustomTreeItem *pTreeItem = new CustomTreeItem( itemId );
        newSelectionObjectId = m_pMainFrame->m_pTreeWidget->AppendItem( m_pMainFrame->m_tSelectionObjectsId, pSelectionObject->getName(), 0, -1, pTreeItem );
        
    }
    else
    {
        pSelectionObject->setIsFirstLevel( false );
        
        int childId = selTree.addChildrenObject( selTree.getId( pCurObj ),  pSelectionObject );
        
        CustomTreeItem *pTreeItem = new CustomTreeItem( childId );
        newSelectionObjectId = m_pMainFrame->m_pTreeWidget->AppendItem( pCurObj->getTreeId(), pSelectionObject->getName(), 0, -1, pTreeItem );
    }
    
    m_pMainFrame->m_pTreeWidget->EnsureVisible( newSelectionObjectId );
    m_pMainFrame->m_pTreeWidget->SetItemImage( newSelectionObjectId, pSelectionObject->getIcon() );
    
    // New items are always set to green.
    m_pMainFrame->m_pTreeWidget->SetItemBackgroundColour( newSelectionObjectId, *wxGREEN );
    m_pMainFrame->m_pTreeWidget->SelectItem(newSelectionObjectId, true);
    
    pSelectionObject->setTreeId( newSelectionObjectId );    
    SceneManager::getInstance()->setSelBoxChanged( true );
}

void PropertiesWindow::OnFloodFill(wxCommandEvent& WXUNUSED(event))
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnFloodFill" ), LOGLEVEL_DEBUG );

    SceneManager::getInstance()->setSegmentMethod( FLOODFILL );

    ((Anatomy*)m_pMainFrame->m_pCurrentSceneObject)->toggleSegment();
}

void PropertiesWindow::OnSliderFloodMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnSliderFloodMoved" ), LOGLEVEL_DEBUG );

    float l_sliderValue = ((Anatomy*)m_pMainFrame->m_pCurrentSceneObject)->m_pSliderFlood->GetValue() / 200.0f;
    ((Anatomy*)m_pMainFrame->m_pCurrentSceneObject)->setFloodThreshold(l_sliderValue);
    ((Anatomy*)m_pMainFrame->m_pCurrentSceneObject)->m_pTxtThres->SetValue(wxString::Format( wxT( "%.2f"), l_sliderValue));
}

void PropertiesWindow::OnFibersFilter( wxCommandEvent& WXUNUSED( event ) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnFibersFilter" ), LOGLEVEL_DEBUG );

    DatasetIndex index = MyApp::frame->m_pListCtrl->GetItem( MyApp::frame->getCurrentListIndex() );

    Fibers* pTmpFib = DatasetManager::getInstance()->getSelectedFibers( index );
    if( pTmpFib != NULL )
    {
        pTmpFib->updateFibersFilters();
    }
}

void PropertiesWindow::OnGenerateFiberVolume( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGenerateFiberVolume" ), LOGLEVEL_DEBUG );

    long index = MyApp::frame->getCurrentListIndex();
    if( -1 != index )
    {
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->m_pListCtrl->GetItem( index ) );
        if( pFibers != NULL )
        {
            pFibers->generateFiberVolume();
        }
    }
}

void PropertiesWindow::OnToggleUseTex( wxCommandEvent&  WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleUseTex" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject == NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        return;
    }

    DatasetInfo* pInfo = (DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject;
    if( pInfo && pInfo->getType() >= MESH )
    {
        pInfo->toggleUseTex();
        m_pListCtrl->UpdateSelected();
    }
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when the distance coloring option is
// selected when right-clicking on a fiber.
//////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnListMenuDistance( wxCommandEvent& WXUNUSED(event))
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnListMenuDistance" ), LOGLEVEL_DEBUG );

    long index = MyApp::frame->getCurrentListIndex();
    if( -1 != index )
    {
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->m_pListCtrl->GetItem( index ) );
        if( pFibers != NULL )
        {
            Logger::getInstance()->print( _T( "Event triggered - PropertiesWindow::OnListMenuDistance" ), LOGLEVEL_DEBUG );

            if( pFibers->getColorationMode() != DISTANCE_COLOR )
            {
                pFibers->setColorationMode( DISTANCE_COLOR );
                pFibers->updateFibersColors();  
                pFibers->updateColorationMode();
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when the minimum distance coloring option is
// selected when right-clicking on a fiber.
//////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnListMenuMinDistance( wxCommandEvent& WXUNUSED(event))
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnListMenuMinDistance" ), LOGLEVEL_DEBUG );

    long index = MyApp::frame->getCurrentListIndex();
    if( -1 != index )
    {
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->m_pListCtrl->GetItem( index ) );
        if( pFibers != NULL )
        {
            if( pFibers->getColorationMode() != MINDISTANCE_COLOR )
            {
                pFibers->setColorationMode( MINDISTANCE_COLOR );
                pFibers->updateFibersColors();
                pFibers->updateColorationMode();
            }
        }
    }
    else
    {
        Logger::getInstance()->print( wxT( "PropertiesWindow::OnListMenuMinDistance - Current index is -1" ), LOGLEVEL_ERROR );
    }
}


///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the color with curvature button
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnColorWithCurvature( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnColorWithCurvature" ), LOGLEVEL_DEBUG );

    long index = MyApp::frame->getCurrentListIndex();
    if( -1 != index )
    {
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->m_pListCtrl->GetItem( index ) );
        if( pFibers != NULL )
        {
            if( pFibers->getColorationMode() != CURVATURE_COLOR )
            {
                pFibers->setColorationMode( CURVATURE_COLOR );
                pFibers->updateFibersColors();
                pFibers->updateColorationMode();
            }
        }
    }
    else
    {
        Logger::getInstance()->print( wxT( "PropertiesWindow::OnColorWithCurvature - Current index is -1" ), LOGLEVEL_ERROR );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display min/max cross section
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnColorWithTorsion( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnColorWithTorsion" ), LOGLEVEL_DEBUG );

    long index = MyApp::frame->getCurrentListIndex();
    if( -1 != index )
    {
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->m_pListCtrl->GetItem( index ) );
        if( pFibers != NULL )
        {
            if( pFibers->getColorationMode() != TORSION_COLOR )
            {
                pFibers->setColorationMode( TORSION_COLOR );
                pFibers->updateFibersColors();
                pFibers->updateColorationMode();
            }
        }
    }
    else
    {
        Logger::getInstance()->print( wxT( "PropertiesWindow::OnColorWithTorsion - Current index is -1" ), LOGLEVEL_ERROR );
    }
}

void PropertiesWindow::OnNormalColoring( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnNormalColoring" ), LOGLEVEL_DEBUG );

    long index = MyApp::frame->getCurrentListIndex();
    if( -1 != index )
    {
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->m_pListCtrl->GetItem( index ) );
        if( pFibers != NULL )
        {
            if( pFibers->getColorationMode() != NORMAL_COLOR )
            {
                pFibers->setColorationMode( NORMAL_COLOR );
                pFibers->updateFibersColors();
                pFibers->updateColorationMode();
            }
        }
    }
    else
    {
        Logger::getInstance()->print( wxT( "PropertiesWindow::OnNormalColoring - Current index is -1" ), LOGLEVEL_ERROR );
    }
}

void PropertiesWindow::OnColorWithConstantColor( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnColorWithConstantColor" ), LOGLEVEL_DEBUG );
    
    long index = MyApp::frame->getCurrentListIndex();
    if( -1 != index )
    {
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->m_pListCtrl->GetItem( index ) );
        if( pFibers != NULL )
        {
            if( pFibers->getColorationMode() != CONSTANT_COLOR )
            {
                pFibers->setColorationMode( CONSTANT_COLOR );
                pFibers->updateFibersColors();
                pFibers->updateColorationMode();
            }
        }
    }
    else
    {
        Logger::getInstance()->print( wxT( "PropertiesWindow::OnColorWithConstantColor - Current index is -1" ), LOGLEVEL_ERROR );
    }
}

void PropertiesWindow::OnSelectConstantColor( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnSelectConstantColor" ), LOGLEVEL_DEBUG );
    
    long index = MyApp::frame->getCurrentListIndex();
    if( -1 == index )
    {
        Logger::getInstance()->print( wxT( "PropertiesWindow::OnSelectConstantColor - Current index is -1" ), LOGLEVEL_ERROR );
        return;
    }
    
    wxColour newCol;
    
    bool success = SelectColor( newCol );
    
    if( !success )
    {
        return;
    }
    
    Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->m_pListCtrl->GetItem( index ) );
    if( pFibers != NULL )
    {
        pFibers->setConstantColor( newCol );
        pFibers->updateFibersColors();
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the normal coloring radio
// button located in the mean fiber coloring option
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnNormalMeanFiberColoring( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnNormalMeanFiberColoring" ), LOGLEVEL_DEBUG );

    SelectionObject *pSelObj = m_pMainFrame->getCurrentSelectionObject();
    if( pSelObj != NULL )
    {
        pSelObj->setMeanFiberColorMode( NORMAL_COLOR );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the custom coloring radio
// button located in the mean fiber coloring option
///////////////////////////////////////////////////////////////////////////
// TODO selection can we remove m_pCurrentSceneObject
void PropertiesWindow::OnCustomMeanFiberColoring( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnCustomMeanFiberColoring" ), LOGLEVEL_DEBUG );

    SelectionObject *pSelObj = m_pMainFrame->getCurrentSelectionObject();
    if( pSelObj != NULL )
    {
        pSelObj->setMeanFiberColorMode( CUSTOM_COLOR );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user move the slider
// button located in the mean fiber coloring option
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnMeanFiberOpacityChange( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnMeanFiberOpacityChange" ), LOGLEVEL_DEBUG );
    
    SelectionObject *pSelObj = m_pMainFrame->getCurrentSelectionObject();
    if( pSelObj != NULL )
    {
        pSelObj->updateMeanFiberOpacity();
    }
}

//////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user clicks on the "Set as distance
// anchor" option.
//////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnDistanceAnchorSet( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnDistanceAnchorSet" ), LOGLEVEL_DEBUG );

    SelectionObject *pCurSelObj = m_pMainFrame->getCurrentSelectionObject();
    if( pCurSelObj != NULL )
    {
        pCurSelObj->UseForDistanceColoring( !pCurSelObj->IsUsedForDistanceColoring() );
        ColorFibers();
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will call the updateFibersColors function on the currently loaded fiber set.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::ColorFibers()
{   
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::ColorFibers" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        ((Fibers*)m_pMainFrame->m_pCurrentSceneObject)->updateFibersColors();  
    }  
}
void PropertiesWindow::OnOriginalShBasis( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnOriginalShBasis" ), LOGLEVEL_DEBUG );

    long index = m_pMainFrame->getCurrentListIndex();
    if( -1 != index )
    {
        ODFs *pOdfs = (ODFs *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( index ) );
        pOdfs->changeShBasis( SH_BASIS_RR5768 );
    }
}

void PropertiesWindow::OnDescoteauxShBasis( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnDescoteauxShBasis" ), LOGLEVEL_DEBUG );

    long index = m_pMainFrame->getCurrentListIndex();
    if( -1 != index )
    {
        ODFs *pOdfs = (ODFs *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( index ) );
        pOdfs->changeShBasis( SH_BASIS_DESCOTEAUX );
    }
}

void PropertiesWindow::OnMaximasDisplaySlice( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnMaximasDisplaySlice" ), LOGLEVEL_DEBUG );

    long index = m_pMainFrame->getCurrentListIndex();
    if( -1 != index )
    {
        Maximas *pMaximas = (Maximas *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( index ) );
        pMaximas->changeDisplay( SLICES );
    }
}


void PropertiesWindow::OnMaximasDisplayWhole( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnMaximasDisplaySlice" ), LOGLEVEL_DEBUG );

    long index = m_pMainFrame->getCurrentListIndex();
    if( -1 != index )
    {
        Maximas *pMaximas = (Maximas *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( index ) );
        pMaximas->changeDisplay( WHOLE );
    }
}

void PropertiesWindow::OnTournierShBasis( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnTournierShBasis" ), LOGLEVEL_DEBUG );

    long index = m_pMainFrame->getCurrentListIndex();
    if( -1 != index )
    {
        ODFs *pOdfs = (ODFs *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( index ) );
        pOdfs->changeShBasis( SH_BASIS_TOURNIER );
    }
}

void PropertiesWindow::OnDipyShBasis( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnDipyShBasis" ), LOGLEVEL_DEBUG );

    long index = m_pMainFrame->getCurrentListIndex();
    if( -1 != index )
    {
        ODFs *pOdfs = (ODFs *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( index ) );
        pOdfs->changeShBasis( SH_BASIS_DIPY );
    }
}

void PropertiesWindow::OnPTKShBasis( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnPTKShBasis" ), LOGLEVEL_DEBUG );

    long index = m_pMainFrame->getCurrentListIndex();
    if( -1 != index )
    {
        ODFs *pOdfs = (ODFs *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( index ) );
        pOdfs->changeShBasis( SH_BASIS_PTK );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the min hue value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphMinHueSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphMinHueSliderMoved" ), LOGLEVEL_DEBUG );

    updateGlyphColoration( MIN_HUE, ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pSliderMinHue->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the max hue value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphMaxHueSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphMaxHueSliderMoved" ), LOGLEVEL_DEBUG );

    updateGlyphColoration( MAX_HUE, ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pSliderMaxHue->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the saturation value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphSaturationSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphSaturationSliderMoved" ), LOGLEVEL_DEBUG );

    updateGlyphColoration( SATURATION,((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pSliderSaturation->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the luminance value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphLuminanceSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphLuminanceSliderMoved" ), LOGLEVEL_DEBUG );

    updateGlyphColoration( LUMINANCE, ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pSliderLuminance->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will set the value of a glyph color modifier by the value
// on its corresponding slider.
//
// i_modifier       : The modifier indicating what GlyphColorModifier needs to be updated.
// i_value          : The value of the modifier to set.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::updateGlyphColoration( GlyphColorModifier i_modifier, float i_value )
{

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {            
        DatasetInfo* l_info = (DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject;    
        if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
            ( (Glyph*)l_info )->setColor( i_modifier, i_value );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the LOD of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphLODSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphLODSliderMoved" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {            
        DatasetInfo* l_info = (DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject;
        if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
        {
            ( (Glyph*)l_info )->setLOD( (LODChoices)((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pSliderLOD->GetValue() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the light attenuation of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphLightAttenuationSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphLightAttenuationSliderMoved" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {            
        DatasetInfo* l_info = (DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject;
        if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
            ( (Glyph*)l_info )->setLighAttenuation( ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pSliderLightAttenuation->GetValue() / 100.0f );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the light x position slider moved.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphLightXDirectionSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphLightXDirectionSliderMoved" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        OnGlyphLightPositionChanged( X_AXIS, ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pSliderLightXPosition->GetValue() / 100.0f  );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the light y position slider moved.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphLightYDirectionSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphLightYDirectionSliderMoved" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        OnGlyphLightPositionChanged( Y_AXIS, ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pSliderLightYPosition->GetValue() / 100.0f  );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the light z position slider moved.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphLightZDirectionSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphLightZDirectionSliderMoved" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        OnGlyphLightPositionChanged( Z_AXIS, ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pSliderLightZPosition->GetValue() / 100.0f  );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the light position for the proper axis.
//
// i_axisType       : The axis that we want to set the lght position for.
// i_position       : The value of the position.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphLightPositionChanged( AxisType i_axisType, float i_position )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphLightPositionChanged" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->setLightPosition( i_axisType, i_position);
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the display value of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphDisplaySliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphDisplaySliderMoved" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->setDisplayFactor( ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pSliderDisplay->GetValue());
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the scaling factor of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphScalingFactorSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphScalingFactorSliderMoved" ), LOGLEVEL_DEBUG );
   
    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        float sliderValue = ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pSliderScalingFactor->GetValue();
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pTxtBoxScalingFactor->SetValue( wxString::Format( wxT( "%.2f"), sliderValue / 10.0f ));
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->setScalingFactor( sliderValue / 10.0f );
    }
}

void PropertiesWindow::OnBoxScalingFactor( wxCommandEvent &event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnBoxScalingFactor" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        double boxValue = 0;
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pTxtBoxScalingFactor->GetValue().ToDouble(&boxValue);
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pSliderScalingFactor->SetValue( boxValue * 10.0f );
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->setScalingFactor( boxValue );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the x flip check box in the 
// glyph options is checked/unchecked.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphXAxisFlipChecked( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphXAxisFlipChecked" ), LOGLEVEL_DEBUG );

    OnGlyphFlip( X_AXIS, event.IsChecked() );
    RTTrackingHelper::getInstance()->setRTTDirty( true );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the y flip check box in the 
// glyph options is checked/unchecked.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphYAxisFlipChecked( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphYAxisFlipChecked" ), LOGLEVEL_DEBUG );

    OnGlyphFlip( Y_AXIS, event.IsChecked() );
    RTTrackingHelper::getInstance()->setRTTDirty( true );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the z flip check box in the 
// glyph options is checked/unchecked.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphZAxisFlipChecked( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::" ), LOGLEVEL_DEBUG );

    OnGlyphFlip( Z_AXIS, event.IsChecked() );
    RTTrackingHelper::getInstance()->setRTTDirty( true );
}

///////////////////////////////////////////////////////////////////////////
// This function will simply find the currently displayed glyph and call 
// the flipAxis function with the proper parameter.
//
// i_axisType               : Determines on what axis we want to do the flip.
// i_isChecked              : Determines if the item is checked or not.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphFlip( AxisType i_axisType, bool i_isChecked )
{
    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->flipAxis( i_axisType, i_isChecked );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the map on sphere radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphMapOnSphereSelected( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphMapOnSphereSelected" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->setDisplayShape( SPHERE );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the normal display radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphNormalSelected( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphNormalSelected" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->setDisplayShape( NORMAL );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the axes display radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphAxesSelected( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphAxesSelected" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->setDisplayShape( AXES );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the main axis display radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphMainAxisSelected( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphMainAxisSelected" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        if(((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject)->getType() == ODFS && !((ODFs*)m_pMainFrame->m_pCurrentSceneObject)->m_isMaximasSet)
        {
            ((ODFs*)m_pMainFrame->m_pCurrentSceneObject)->extractMaximas();
            ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->setDisplayShape( AXIS );
            ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->updatePropertiesSizer();
            
            int indx = DatasetManager::getInstance()->createMaximas( wxT( "Extracted Maximas" ) );
            Maximas* pMaximas = (Maximas *)DatasetManager::getInstance()->getDataset( indx );
            if( pMaximas->createMaximas( *(((ODFs*)m_pMainFrame->m_pCurrentSceneObject)->getMainDirs())) )
            {
                Logger::getInstance()->print( wxT( "Assigning attributes" ), LOGLEVEL_DEBUG );
                pMaximas->setThreshold( 0.0f );
                pMaximas->setAlpha( 1.0f );
                pMaximas->setShow( true );
                pMaximas->setShowFS( true );
                pMaximas->setUseTex( true );

                m_pListCtrl->InsertItem( indx );             
            }
        }
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->setDisplayShape( AXIS );
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->updatePropertiesSizer();
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the color with position check box in the 
// glyph options is checked/unchecked.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphColorWithPosition( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnGlyphColorWithPosition" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->setColorWithPosition( event.IsChecked() );
    }
}

void PropertiesWindow::OnNormalizeTensors( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnNormalizeTensors" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {        
        ((Tensors*)m_pMainFrame->m_pCurrentSceneObject)->normalize();        
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display fibers 
// info after a right click in the tree on a selection object.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnDisplayFibersInfo( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnDisplayFibersInfo" ), LOGLEVEL_DEBUG );

    // TODO remove when the bug with the wxChoice in Windows is fixed.
    SelectionObject* pSelObj = m_pMainFrame->getCurrentSelectionObject();
    if( pSelObj != NULL )
    {
        pSelObj->notifyStatsNeedUpdating();
#ifndef __WXMSW__
        pSelObj->UpdateMeanValueTypeBox();
#endif
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display mean fiber 
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnDisplayMeanFiber( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnDisplayMeanFiber" ), LOGLEVEL_DEBUG );

    SelectionObject *pSelObj = m_pMainFrame->getCurrentSelectionObject();
    
    pSelObj->notifyStatsNeedUpdating();
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display convex hull
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
// TODO selection convex hull test
void PropertiesWindow::OnDisplayConvexHull( wxCommandEvent& WXUNUSED(event) )
{
    SelectionObject *pSelObj = m_pMainFrame->getCurrentSelectionObject();
    if( pSelObj != NULL )
    {
        pSelObj->computeConvexHull();
        m_pMainFrame->refreshAllGLWidgets();
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the color button
// beside the display convex hull button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
// TODO selection convex hull test
void PropertiesWindow::OnConvexHullColorChange( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnConvexHullColorChange" ), LOGLEVEL_DEBUG );

    SelectionObject *pSelObj = m_pMainFrame->getCurrentSelectionObject();
    
    if( pSelObj != NULL )
    {
        wxColour newCol;
        
        bool success = SelectColor( newCol );
        
        if( !success )
        {
            return;
        }

        pSelObj->setConvexHullColor( newCol );
        
        // TODO is this mandatory
        m_pMainFrame->refreshAllGLWidgets();
    }
}

// TODO selection convex hull test
void PropertiesWindow::OnConvexHullOpacityChange( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnConvexHullOpacityChange" ), LOGLEVEL_DEBUG );

    SelectionObject *pSelObj = m_pMainFrame->getCurrentSelectionObject();
    if( pSelObj != NULL )
    {
        pSelObj->updateConvexHullOpacity();
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the color palette
// button that is located aside of the Show mean fiber button
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnMeanFiberColorChange( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnMeanFiberColorChange" ), LOGLEVEL_DEBUG );

    wxColour newCol;
    
    bool success = SelectColor( newCol );
    
    if( !success )
    {
        return;
    }

    SelectionObject *pSelObj = m_pMainFrame->getCurrentSelectionObject();
    if( pSelObj != NULL )
    {
        pSelObj->setMeanFiberColor( newCol );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the rename option is clicked on the right
// click menu of a SelectionObject item in the tree.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnRenameBox( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnRenameBox" ), LOGLEVEL_DEBUG );
    
    SelectionObject *pSelObj = m_pMainFrame->getCurrentSelectionObject();
    
    if( pSelObj != NULL )
    {
        wxTextEntryDialog dialog(this, _T( "Please enter a new name" ) );
        dialog.SetValue( pSelObj->getName() );
        
        if( ( dialog.ShowModal() == wxID_OK ) && ( dialog.GetValue() != _T( "" ) ) )
		{
            pSelObj->setName( dialog.GetValue() );
		}

        m_pMainFrame->m_pTreeWidget->SetItemText( pSelObj->getTreeId(), pSelObj->getName() );
    }
}

void PropertiesWindow::OnToggleAndNot( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleAndNot" ), LOGLEVEL_DEBUG );
    
    SelectionObject *pSelObj = m_pMainFrame->getCurrentSelectionObject();
    
    if( pSelObj != NULL )
    {
        pSelObj->toggleIsNOT();
        
        if( pSelObj->getIsNOT() )
        {
            m_pMainFrame->m_pTreeWidget->SetItemBackgroundColour( pSelObj->getTreeId(), *wxRED   );
        }
        else
        {
            m_pMainFrame->m_pTreeWidget->SetItemBackgroundColour( pSelObj->getTreeId(), *wxGREEN   );
        }
    }
}


void PropertiesWindow::OnColorRoi( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnColorRoi" ), LOGLEVEL_DEBUG );
    
    wxColour newCol;
    
    bool success = SelectColor( newCol );
    
    if( !success )
    {
        return;
    }
    
    SelectionObject *pSelObject = m_pMainFrame->getCurrentSelectionObject();
    
    pSelObject->setColor( newCol );
    
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnVoiFlipNormals( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnVoiFlipNormals" ), LOGLEVEL_DEBUG );

    SelectionObject *pSelObj = m_pMainFrame->getCurrentSelectionObject();
    if( pSelObj != NULL )
    {
        pSelObj->flipNormals();
    }
}

void PropertiesWindow::OnDeleteTreeItem( wxTreeEvent& evt )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnDeleteTreeItem" ), LOGLEVEL_DEBUG );

    m_pMainFrame->onDeleteTreeItem( evt );
    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->clearFibersRTT();
    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->clearColorsRTT();
    RTTrackingHelper::getInstance()->setRTTDirty( false );
    RTTrackingHelper::getInstance()->setRTTReady( false );
    m_pMainFrame->m_pTrackingWindow->m_pBtnStart->Enable( false );
}

void PropertiesWindow::OnActivateTreeItem ( wxTreeEvent& evt )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnActivateTreeItem" ), LOGLEVEL_DEBUG );

    m_pMainFrame->onActivateTreeItem( evt );
}

void PropertiesWindow::OnToggleShowSelectionObject( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleShowSelectionObject" ), LOGLEVEL_DEBUG );
    
    m_pMainFrame->toggleTreeItemVisibility();
}

void PropertiesWindow::OnAssignColor( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnAssignColor" ), LOGLEVEL_DEBUG );

    wxColour newCol;
    
    bool success = SelectColor( newCol );
    
    if( !success )
    {
        return;
    }
    
    SelectionObject* pSelObj = m_pMainFrame->getCurrentSelectionObject();
    
    if( pSelObj != NULL )
    {
        // Iterate over all fibers.
        vector<Fibers*> allFibs(DatasetManager::getInstance()->getFibers());
        
        for (vector<Fibers*>::iterator curFib(allFibs.begin()); curFib < allFibs.end(); ++curFib)
        {
            vector<bool> selFibers = SceneManager::getInstance()->getSelectionTree().getSelectedFibers( *curFib );
            
            vector<bool> filteredFibs = (*curFib)->getFilteredFibers();

            vector<bool>::iterator filteredIt(filteredFibs.begin());

            int fibItIdx(0);
            
            for( vector<bool>::iterator fibIt(selFibers.begin()); fibIt != selFibers.end(); ++fibIt, ++filteredIt, ++fibItIdx )
            {
                if( *fibIt && !*filteredIt)
                {
                    (*curFib)->setFiberColor( fibItIdx, newCol );
                }
            }
        }
        // TODO is this mandatory
        SceneManager::getInstance()->setSelBoxChanged( true );
    }
    
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnAssignColorDataset( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnAssignColorDataset" ), LOGLEVEL_DEBUG );
    
    wxColour newCol;
    
    bool success = SelectColor( newCol );
    
    if( !success )
    {
        return;
    }
    
    if( m_pMainFrame->m_currentListIndex != -1 )
    {
        DatasetInfo *pInfo = (DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject;
        if( pInfo->getType() == MESH || pInfo->getType() == ISO_SURFACE || pInfo->getType() == SURFACE || pInfo->getType() == VECTORS)
        {
            pInfo->setColor( newCol );
            pInfo->setUseTex( false );
            m_pListCtrl->UpdateSelected();
        }
    }
    
    m_pMainFrame->refreshAllGLWidgets();
}

// TODO error management: add error messages, disable button when no fibers loaded.
void PropertiesWindow::OnCreateFibersDensityTexture( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnCreateFibersDensityTexture" ), LOGLEVEL_DEBUG );
    
    SelectionObject* pSelObj = m_pMainFrame->getCurrentSelectionObject();
    
    if( pSelObj == NULL )
    {
        return;
    }
    
    if( DatasetManager::getInstance()->getFibersCount() == 0 )
    {
        return;
    }
    
    DatasetIndex dsIndex = DatasetManager::getInstance()->createAnatomy();
    Anatomy* pNewAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( dsIndex );
    
    int columns = DatasetManager::getInstance()->getColumns();
    int rows    = DatasetManager::getInstance()->getRows();
    int frames  = DatasetManager::getInstance()->getFrames();
    
    pNewAnatomy->setZero( columns, rows, frames );
    pNewAnatomy->setDataType( 16 );
    pNewAnatomy->setType( OVERLAY );
    
    std::vector<float>* pDataset = pNewAnatomy->getFloatDataset();
    
    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();
    
    // Iterate over all fibers.
    vector<Fibers*> allFibs(DatasetManager::getInstance()->getFibers());
    
    for (vector<Fibers*>::iterator curFib(allFibs.begin()); curFib < allFibs.end(); ++curFib)
    {
        vector<bool> selFibers = SceneManager::getInstance()->getSelectionTree().getSelectedFibers( *curFib );
        
        vector<bool> filteredFibs = (*curFib)->getFilteredFibers();
        
        vector<bool>::iterator filteredIt(filteredFibs.begin());
        
        int fibItIdx(0);
        
        for( vector<bool>::iterator fibIt(selFibers.begin()); fibIt != selFibers.end(); ++fibIt, ++filteredIt, ++fibItIdx )
        {
            if( *fibIt && !*filteredIt)
            {
                unsigned int pc = (*curFib)->getStartIndexForLine( fibItIdx ) * 3;
                
                for( int j = 0; j < (*curFib)->getPointsPerLine( fibItIdx ) ; ++j, pc += 3 )
                {
                    int curX = static_cast<int>( (*curFib)->getPointValue( pc ) / voxelX );
                    int curY = static_cast<int>( (*curFib)->getPointValue( pc + 1 ) / voxelY );
                    int curZ = static_cast<int>( (*curFib)->getPointValue( pc + 2 ) / voxelZ );
                    
                    int index = curX + curY * columns + curZ * columns * rows;
                    
                    pDataset->at( index )     += 1.0f;
                }
            }
        }
    }
    
    // Normalize all values.
    float largestCount(*std::max_element( pDataset->begin(), pDataset->end() ) );
    
    if( largestCount > 0 )
    {
        for( vector<float>::iterator voxIt(pDataset->begin()); voxIt != pDataset->end();
            ++voxIt )
        {
            (*voxIt) /= largestCount;
        }
    }
    
    pNewAnatomy->setName( wxT(" (fiber_density)" ) );
    pNewAnatomy->setOldMax( largestCount );
    
    m_pListCtrl->InsertItem( dsIndex );
    
    m_pMainFrame->refreshAllGLWidgets();
}

// TODO error management: add error messages, disable button when no fibers loaded.
void PropertiesWindow::OnCreateFibersColorTexture( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnCreateFibersColorTexture" ), LOGLEVEL_DEBUG );

    SelectionObject* pSelObj = m_pMainFrame->getCurrentSelectionObject();
    
    if( pSelObj == NULL )
    {
        return;
    }
    
    if( DatasetManager::getInstance()->getFibersCount() == 0 )
    {
        return;
    }
    
        
    DatasetIndex dsIndex = DatasetManager::getInstance()->createAnatomy();
    Anatomy* pNewAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( dsIndex );

    int columns = DatasetManager::getInstance()->getColumns();
    int rows    = DatasetManager::getInstance()->getRows();
    int frames  = DatasetManager::getInstance()->getFrames();

    pNewAnatomy->setRGBZero( columns, rows, frames );
    std::vector<float>* pDataset = pNewAnatomy->getFloatDataset();
    
    std::vector<int> voxHitCount( pDataset->size() / 3, 0 );

    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();
    
    // Iterate over all fibers.
    vector<Fibers*> allFibs(DatasetManager::getInstance()->getFibers());
    
    for (vector<Fibers*>::iterator curFib(allFibs.begin()); curFib < allFibs.end(); ++curFib)
    {
        vector<bool> selFibers = SceneManager::getInstance()->getSelectionTree().getSelectedFibers( *curFib );
        
        vector<bool> filteredFibs = (*curFib)->getFilteredFibers();
        
        vector<bool>::iterator filteredIt(filteredFibs.begin());
        
        int fibItIdx(0);
        
        for( vector<bool>::iterator fibIt(selFibers.begin()); fibIt != selFibers.end(); ++fibIt, ++filteredIt, ++fibItIdx )
        {
            if( *fibIt && !*filteredIt)
            {
                unsigned int pc = (*curFib)->getStartIndexForLine( fibItIdx ) * 3;
                
                for( int j = 0; j < (*curFib)->getPointsPerLine( fibItIdx ) ; ++j, pc += 3 )
                {
                    wxColour ptCol = (*curFib)->getFiberPointColor( fibItIdx, j );
                    
                    int curX = static_cast<int>( (*curFib)->getPointValue( pc ) / voxelX );
                    int curY = static_cast<int>( (*curFib)->getPointValue( pc + 1 ) / voxelY );
                    int curZ = static_cast<int>( (*curFib)->getPointValue( pc + 2 ) / voxelZ );
                    
                    int index = 3 * ( curX + curY * columns + curZ * columns * rows );
                    
                    pDataset->at( index )     += ptCol.Red()   / 255.0f;
                    pDataset->at( index + 1 ) += ptCol.Green() / 255.0f;
                    pDataset->at( index + 2 ) += ptCol.Blue()  / 255.0f;
                    
                    voxHitCount.at( index / 3 ) += 1;
                }
            }
        }
    }
    
    // Normalize all values.
    int largestCount(*std::max_element( voxHitCount.begin(), voxHitCount.end() ) );
    
    if( largestCount > 0 )
    {
        for( vector<float>::iterator voxIt(pDataset->begin()); voxIt != pDataset->end();
             ++voxIt )
        {
            (*voxIt) /= largestCount;
        }
    }

    pNewAnatomy->setName( wxT( " (fiber_colors)" ) );
    
    m_pListCtrl->InsertItem( dsIndex );
    
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnMeanComboBoxSelectionChange( wxCommandEvent& event)
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnMeanComboBoxSelectionChange" ), LOGLEVEL_DEBUG );

    SceneManager::getInstance()->getSelectionTree().notifyAllObjectsNeedUpdating();
}

void PropertiesWindow::OnBoxPositionX( wxCommandEvent &event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnBoxPositionX" ), LOGLEVEL_DEBUG );

    double posX = 0;
    Vector currPos;
    
    SelectionObject *pSelObj = m_pMainFrame->getCurrentSelectionObject();
    if( pSelObj != NULL )
    {
        pSelObj->m_pTxtBoxX->GetValue().ToDouble(&posX);  
        currPos = pSelObj->getCenter();
        pSelObj->setCenter(posX,currPos.y,currPos.z);
    }
}

void PropertiesWindow::OnBoxPositionY( wxCommandEvent &event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnBoxPositionY" ), LOGLEVEL_DEBUG );

    double posY = 0;
    Vector currPos;
    
    SelectionObject *pSelObj = m_pMainFrame->getCurrentSelectionObject();
    if( pSelObj != NULL )
    {
        pSelObj->m_pTxtBoxY->GetValue().ToDouble(&posY);  
        currPos = pSelObj->getCenter();
        pSelObj->setCenter(currPos.x,posY,currPos.z);
    }
}

void PropertiesWindow::OnBoxPositionZ( wxCommandEvent &event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnBoxPositionZ" ), LOGLEVEL_DEBUG );

    double posZ = 0;
    Vector currPos;
    
    SelectionObject *pSelObj = m_pMainFrame->getCurrentSelectionObject();
    if( pSelObj != NULL )
    {
        pSelObj->m_pTxtBoxZ->GetValue().ToDouble(&posZ);  
        currPos = pSelObj->getCenter();
        pSelObj->setCenter(currPos.x,currPos.y,posZ);
    }
}

void PropertiesWindow::OnBoxSizeX( wxCommandEvent &event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnBoxSizeX" ), LOGLEVEL_DEBUG );
    double sizeX = 0;
    Vector currSize;
    
    SelectionObject* pSelObj = m_pMainFrame->getCurrentSelectionObject();
    
    if( pSelObj != NULL )
    {
        pSelObj->m_pTxtSizeX->GetValue().ToDouble( &sizeX );
        currSize = pSelObj->getSize();
        currSize.x = sizeX / DatasetManager::getInstance()->getVoxelX();
        pSelObj->setSize( currSize );
    }
}

void PropertiesWindow::OnBoxSizeY( wxCommandEvent &event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnBoxSizeY" ), LOGLEVEL_DEBUG );

    double sizeY = 0;
    Vector currSize;
    
    SelectionObject* pSelObj = m_pMainFrame->getCurrentSelectionObject();
    
    if( pSelObj != NULL )
    {
        pSelObj->m_pTxtSizeY->GetValue().ToDouble( &sizeY );
        currSize = pSelObj->getSize();
        currSize.y = sizeY / DatasetManager::getInstance()->getVoxelY();
        pSelObj->setSize( currSize );
    }
}

void PropertiesWindow::OnBoxSizeZ( wxCommandEvent &event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnBoxSizeZ" ), LOGLEVEL_DEBUG );

    double sizeZ = 0;
    Vector currSize;
    
    SelectionObject* pSelObj = m_pMainFrame->getCurrentSelectionObject();
    
    if( pSelObj != NULL )
    {
        pSelObj->m_pTxtSizeZ->GetValue().ToDouble( &sizeZ );
        currSize = pSelObj->getSize();
        currSize.z = sizeZ / DatasetManager::getInstance()->getVoxelZ();
        pSelObj->setSize( currSize );
    }
}

void PropertiesWindow::OnSliderAxisMoved( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnSliderAxisMoved" ), LOGLEVEL_DEBUG );

    float l_sliderValue = ((ODFs*)m_pMainFrame->m_pCurrentSceneObject)->getSliderFlood()->GetValue() / 10.0f;
    ((ODFs*)m_pMainFrame->m_pCurrentSceneObject)->m_axisThreshold = l_sliderValue;
    ((ODFs*)m_pMainFrame->m_pCurrentSceneObject)->getTxtThresBox()->SetValue(wxString::Format( wxT( "%.1f"), l_sliderValue));

    std::cout << ((ODFs*)m_pMainFrame->m_pCurrentSceneObject)->m_axisThreshold << std::endl;
}

void PropertiesWindow::OnRecalcMainDir( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnRecalcMainDir" ), LOGLEVEL_DEBUG );

    ((ODFs*)m_pMainFrame->m_pCurrentSceneObject)->extractMaximas();
}

void PropertiesWindow::OnToggleCrossingFibers( wxEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleCrossingFibers" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        if( ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject)->getType() == FIBERS )
        {
            ((Fibers*)m_pMainFrame->m_pCurrentSceneObject)->toggleCrossingFibers();
        }
    }
}

void PropertiesWindow::OnCrossingFibersThicknessChange( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnCrossingFibersThicknessChange" ), LOGLEVEL_DEBUG );

    if( m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1 )
    {
        if( ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject)->getType() == FIBERS )
        {
            ((Fibers*)m_pMainFrame->m_pCurrentSceneObject)->updateCrossingFibersThickness();
        }
    }
}

bool PropertiesWindow::SelectColor( wxColour &col )
{
    wxColourData colorData;
    
    for( int i = 0; i < 10; ++i )
    {
        wxColour colorTemp(i * 28, i * 28, i * 28);
        colorData.SetCustomColour(i, colorTemp);
    }
    
    int i = 10;
    wxColour colorTemp ( 255, 0, 0 );
    colorData.SetCustomColour( i++, colorTemp );
    wxColour colorTemp1( 0, 255, 0 );
    colorData.SetCustomColour( i++, colorTemp1 );
    wxColour colorTemp2( 0, 0, 255 );
    colorData.SetCustomColour( i++, colorTemp2 );
    wxColour colorTemp3( 255, 255, 0 );
    colorData.SetCustomColour( i++, colorTemp3 );
    wxColour colorTemp4( 255, 0, 255 );
    colorData.SetCustomColour( i++, colorTemp4 );
    wxColour colorTemp5( 0, 255, 255 );
    colorData.SetCustomColour( i++, colorTemp5 );
    
    wxColourDialog dialog( this, &colorData );

    if( dialog.ShowModal() == wxID_OK )
    {
        wxColourData retData = dialog.GetColourData();
        col = retData.GetColour();
        return true;
    }

    return false;
}
