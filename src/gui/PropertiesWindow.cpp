#include "ListCtrl.h"
#include "MainFrame.h"
#include "PropertiesWindow.h"
#include "SceneManager.h"
#include "SelectionBox.h"
#include "SelectionEllipsoid.h"
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
#include "../misc/IsoSurface/CIsoSurface.h"
#include "../misc/IsoSurface/TriangleMesh.h"

#include <wx/colordlg.h>
#include <wx/notebook.h>

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

    wxTreeItemId     treeObjectId  = m_pMainFrame->m_tSelectionObjectsId;
    SelectionObject* selectionObj  = NULL;
    Anatomy*         anatomy       = NULL;
    wxColor          color;
    bool             isMaster;

    if( m_pMainFrame->getLastSelectedObj() != NULL )
    {
        treeObjectId = m_pMainFrame->getLastSelectedObj()->GetId();
    }

    if(m_pMainFrame->m_pCurrentSceneObject != NULL && m_pMainFrame->m_currentListIndex != -1)
    {
        if ( ((DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject)->getType() < RGB)
        {
            anatomy = (Anatomy*)m_pMainFrame->m_pCurrentSceneObject;
            selectionObj = new SelectionBox( anatomy );
            float trs = anatomy->getThreshold();
            if( trs == 0.0 )
            {
                trs = 0.01f;
            }
            selectionObj->setThreshold( trs );
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

    if( m_pMainFrame->treeSelected( treeObjectId ) == MASTER_OBJECT)
    {
        color = *wxGREEN;
        isMaster = false;
    }
    else
    {
        treeObjectId = m_pMainFrame->m_tSelectionObjectsId;
        color = *wxCYAN;
        isMaster = true;
    }

    wxTreeItemId l_treeNewObjectId = m_pMainFrame->m_pTreeWidget->AppendItem( treeObjectId, selectionObj->getName(), 0, -1, selectionObj );
    m_pMainFrame->m_pTreeWidget->SetItemBackgroundColour( l_treeNewObjectId, color );
    m_pMainFrame->m_pTreeWidget->EnsureVisible( l_treeNewObjectId );
    m_pMainFrame->m_pTreeWidget->SetItemImage( l_treeNewObjectId, selectionObj->getIcon() );
    selectionObj->setTreeId( l_treeNewObjectId );
    selectionObj->setIsMaster( isMaster );
    anatomy->m_pRoi = selectionObj;

    SceneManager::getInstance()->setSelBoxChanged( true );
    m_pMainFrame->refreshAllGLWidgets();
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
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( index );
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
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( index );
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
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( index );
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
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( index );
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

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the normal coloring radio
// button located in the mean fiber coloring option
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnNormalMeanFiberColoring( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnNormalMeanFiberColoring" ), LOGLEVEL_DEBUG );

   ( (SelectionObject*) m_pMainFrame->m_pCurrentSceneObject )->setMeanFiberColorMode(NORMAL_COLOR); 
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the custom coloring radio
// button located in the mean fiber coloring option
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnCustomMeanFiberColoring( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnCustomMeanFiberColoring" ), LOGLEVEL_DEBUG );

    ( (SelectionObject*) m_pMainFrame->m_pCurrentSceneObject )->setMeanFiberColorMode(CUSTOM_COLOR);
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user move the slider
// button located in the mean fiber coloring option
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnMeanFiberOpacityChange( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnMeanFiberOpacityChange" ), LOGLEVEL_DEBUG );

    ( (SelectionObject*) m_pMainFrame->m_pCurrentSceneObject )->updateMeanFiberOpacity();
}

//////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user clicks on the "Set as distance
// anchor" option.
//////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnDistanceAnchorSet( wxCommandEvent& event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnDistanceAnchorSet" ), LOGLEVEL_DEBUG );

    SelectionObject * pLastSelObj = m_pMainFrame->getLastSelectedObj();
    if( pLastSelObj != NULL )
    {
        pLastSelObj->UseForDistanceColoring( !pLastSelObj->IsUsedForDistanceColoring() );
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
        ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->setScalingFactor( ((Glyph*)m_pMainFrame->m_pCurrentSceneObject)->m_pSliderScalingFactor->GetValue() / 10.0f );
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
#ifndef __WXMSW__
    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->UpdateMeanValueTypeBox();
#endif
    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->SetFiberInfoGridValues();
    m_pMainFrame->refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display mean fiber 
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnDisplayMeanFiber( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnDisplayMeanFiber" ), LOGLEVEL_DEBUG );

    ( (SelectionObject*)m_pMainFrame->m_pCurrentSceneObject )->computeMeanFiber();
    m_pMainFrame->refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display convex hull
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnDisplayConvexHull( wxCommandEvent& WXUNUSED(event) )
{
    ( (SelectionObject*)m_pMainFrame->m_pCurrentSceneObject )->computeConvexHull();
    m_pMainFrame->refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the color button
// beside the display convex hull button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnConvexHullColorChange( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnConvexHullColorChange" ), LOGLEVEL_DEBUG );

    wxColourData l_colorData;

    for( int i = 0; i < 10; ++i )
    {
        wxColour l_color(i * 28, i * 28, i * 28);
        l_colorData.SetCustomColour(i, l_color);
    }

    int i = 10;
    wxColour l_color ( 255, 0, 0 );
    l_colorData.SetCustomColour( i++, l_color );
    wxColour l_color1( 0, 255, 0 );
    l_colorData.SetCustomColour( i++, l_color1 );
    wxColour l_color2( 0, 0, 255 );
    l_colorData.SetCustomColour( i++, l_color2 );
    wxColour l_color3( 255, 255, 0 );
    l_colorData.SetCustomColour( i++, l_color3 );
    wxColour l_color4( 255, 0, 255 );
    l_colorData.SetCustomColour( i++, l_color4 );
    wxColour l_color5( 0, 255, 255 );
    l_colorData.SetCustomColour( i++, l_color5 );

    wxColourDialog dialog( this, &l_colorData );
    wxColour l_col;
    if( dialog.ShowModal() == wxID_OK )
    {
        wxColourData l_retData = dialog.GetColourData();
        l_col = l_retData.GetColour();
    }
    else
    {
        return;
    }

    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->setConvexHullColor( l_col );
  
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnConvexHullOpacityChange( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnConvexHullOpacityChange" ), LOGLEVEL_DEBUG );

    ( (SelectionObject*) m_pMainFrame->m_pCurrentSceneObject )->updateConvexHullOpacity();
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the color palette
// button that is located aside of the Show mean fiber button
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnMeanFiberColorChange( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnMeanFiberColorChange" ), LOGLEVEL_DEBUG );

    wxColourData l_colorData;

    for( int i = 0; i < 10; ++i )
    {
        wxColour l_color(i * 28, i * 28, i * 28);
        l_colorData.SetCustomColour(i, l_color);
    }

    int i = 10;
    wxColour l_color ( 255, 0, 0 );
    l_colorData.SetCustomColour( i++, l_color );
    wxColour l_color1( 0, 255, 0 );
    l_colorData.SetCustomColour( i++, l_color1 );
    wxColour l_color2( 0, 0, 255 );
    l_colorData.SetCustomColour( i++, l_color2 );
    wxColour l_color3( 255, 255, 0 );
    l_colorData.SetCustomColour( i++, l_color3 );
    wxColour l_color4( 255, 0, 255 );
    l_colorData.SetCustomColour( i++, l_color4 );
    wxColour l_color5( 0, 255, 255 );
    l_colorData.SetCustomColour( i++, l_color5 );

    wxColourDialog dialog( this, &l_colorData );
    wxColour l_col;
    if( dialog.ShowModal() == wxID_OK )
    {
        wxColourData l_retData = dialog.GetColourData();
        l_col = l_retData.GetColour();
    }
    else
    {
        return;
    }

    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->setMeanFiberColor( l_col);
    
    m_pMainFrame->refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the rename option is clicked on the right
// click menu of a SelectionObject item in the tree.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnRenameBox( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnRenameBox" ), LOGLEVEL_DEBUG );

    wxTreeItemId l_treeBoxId = m_pMainFrame->m_pTreeWidget->GetSelection();
    if( m_pMainFrame->treeSelected( l_treeBoxId ) == MASTER_OBJECT || m_pMainFrame->treeSelected( l_treeBoxId ) == CHILD_OBJECT )
    {
        SelectionObject* l_box = (SelectionObject*)( m_pMainFrame->m_pTreeWidget->GetItemData( l_treeBoxId ) );

        wxTextEntryDialog dialog(this, _T( "Please enter a new name" ) );
        dialog.SetValue( l_box->getName() );

        if( ( dialog.ShowModal() == wxID_OK ) && ( dialog.GetValue() != _T( "" ) ) )
        {
            l_box->setName( dialog.GetValue() );
        }

        m_pMainFrame->m_pTreeWidget->SetItemText( l_treeBoxId, l_box->getName() );
    }
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnToggleAndNot( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnToggleAndNot" ), LOGLEVEL_DEBUG );

    // Get what selection object is selected.
    wxTreeItemId l_selectionObjectTreeId = m_pMainFrame->m_pTreeWidget->GetSelection();

    if( m_pMainFrame->treeSelected(l_selectionObjectTreeId) == CHILD_OBJECT)
    {
        SelectionObject* l_box = (SelectionObject*)( m_pMainFrame->m_pTreeWidget->GetItemData( l_selectionObjectTreeId ) );
        l_box->toggleIsNOT();

        wxTreeItemId l_parentId = m_pMainFrame->m_pTreeWidget->GetItemParent( l_selectionObjectTreeId );
        ((SelectionObject*)( m_pMainFrame->m_pTreeWidget->GetItemData( l_parentId ) ) )->setIsDirty( true );

        if( ( (SelectionObject*)( m_pMainFrame->m_pTreeWidget->GetItemData( l_selectionObjectTreeId ) ) )->getIsNOT() )
            m_pMainFrame->m_pTreeWidget->SetItemBackgroundColour( l_selectionObjectTreeId, *wxRED   );
        else
            m_pMainFrame->m_pTreeWidget->SetItemBackgroundColour( l_selectionObjectTreeId, *wxGREEN );

        m_pMainFrame->m_pTreeWidget->SetItemImage( l_selectionObjectTreeId, l_box->getIcon() );
        l_box->setIsDirty( true );
    }
    m_pMainFrame->refreshAllGLWidgets();
}


void PropertiesWindow::OnColorRoi( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnColorRoi" ), LOGLEVEL_DEBUG );

    // Get the currently selected object.
    wxTreeItemId l_selectionObjectTreeId = m_pMainFrame->m_pTreeWidget->GetSelection();
    SelectionObject* l_selectionObject = (SelectionObject*)( m_pMainFrame->m_pTreeWidget->GetItemData( l_selectionObjectTreeId ) );

    wxColourData l_colorData;

    for( int i = 0; i < 10; ++i )
    {
        wxColour l_color( i * 28, i * 28, i * 28 );
        l_colorData.SetCustomColour( i, l_color );
    }

    int i = 10;
    wxColour color ( 255, 0,   0   );
    wxColour color1( 0,   255, 0   );
    wxColour color2( 0,   0,   255 );
    wxColour color3( 255, 255, 0   );
    wxColour color4( 255, 0,   255 );
    wxColour color5( 0,   255, 255 );

    l_colorData.SetCustomColour( i++, color  );
    l_colorData.SetCustomColour( i++, color1 );
    l_colorData.SetCustomColour( i++, color2 );
    l_colorData.SetCustomColour( i++, color3 );
    l_colorData.SetCustomColour( i++, color4 );
    l_colorData.SetCustomColour( i++, color5 );
#ifdef __WXMAC__
    wxColourDialog dialog( this);
#else
    wxColourDialog dialog( this, &l_colorData );
#endif
    wxColour l_color;

    if( dialog.ShowModal() == wxID_OK )
    {
        wxColourData retData = dialog.GetColourData();
        l_color = retData.GetColour();
    }
    else
        return;

    l_selectionObject->setColor( l_color );

    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnVoiFlipNormals( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnVoiFlipNormals" ), LOGLEVEL_DEBUG );

    wxTreeItemId l_selectionObjectTreeId = m_pMainFrame->m_pTreeWidget->GetSelection();
    SelectionObject* l_selectionObject = (SelectionObject*)( m_pMainFrame->m_pTreeWidget->GetItemData( l_selectionObjectTreeId ) );

    if(l_selectionObject->getSelectionType() == CISO_SURFACE_TYPE)
    {
        l_selectionObject->FlipNormals();
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

    // Get the selected selection object.
    wxTreeItemId l_selectionObjectTreeId = m_pMainFrame->m_pTreeWidget->GetSelection();

    if( m_pMainFrame->treeSelected( l_selectionObjectTreeId ) == MASTER_OBJECT )
    {
        SelectionObject* l_selecitonObject = (SelectionObject*)( m_pMainFrame->m_pTreeWidget->GetItemData( l_selectionObjectTreeId ) );
        l_selecitonObject->toggleIsVisible();
        m_pMainFrame->m_pTreeWidget->SetItemImage( l_selectionObjectTreeId, l_selecitonObject->getIcon() );
        l_selecitonObject->setIsDirty( true );

        int l_childSelectionObjects = m_pMainFrame->m_pTreeWidget->GetChildrenCount( l_selectionObjectTreeId );
        wxTreeItemIdValue childcookie = 0;
        for( int i = 0; i < l_childSelectionObjects; ++i )
        {
            wxTreeItemId l_childId = m_pMainFrame->m_pTreeWidget->GetNextChild( l_selectionObjectTreeId, childcookie );
            if( l_childId.IsOk() )
            {
                SelectionObject* childBox = ( (SelectionObject*)( m_pMainFrame->m_pTreeWidget->GetItemData( l_childId ) ) );
                childBox->setIsVisible( l_selecitonObject->getIsVisible() );
                m_pMainFrame->m_pTreeWidget->SetItemImage( l_childId, childBox->getIcon() );
                childBox->setIsDirty( true );
            }
        }
    }
    else if( m_pMainFrame->treeSelected( l_selectionObjectTreeId ) == CHILD_OBJECT )
    {
        SelectionObject *l_selectionObject = (SelectionObject*)( m_pMainFrame->m_pTreeWidget->GetItemData( l_selectionObjectTreeId ) );
        l_selectionObject->toggleIsVisible();
        m_pMainFrame->m_pTreeWidget->SetItemImage( l_selectionObjectTreeId, l_selectionObject->getIcon() );
        l_selectionObject->setIsDirty( true );
    }

    SceneManager::getInstance()->setSelBoxChanged( true );
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnAssignColor( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnAssignColor" ), LOGLEVEL_DEBUG );

    wxColourData l_colorData;

    for( int i = 0; i < 10; ++i )
    {
        wxColour l_color(i * 28, i * 28, i * 28);
        l_colorData.SetCustomColour(i, l_color);
    }

    int i = 10;
    wxColour l_color ( 255, 0, 0 );
    l_colorData.SetCustomColour( i++, l_color );
    wxColour l_color1( 0, 255, 0 );
    l_colorData.SetCustomColour( i++, l_color1 );
    wxColour l_color2( 0, 0, 255 );
    l_colorData.SetCustomColour( i++, l_color2 );
    wxColour l_color3( 255, 255, 0 );
    l_colorData.SetCustomColour( i++, l_color3 );
    wxColour l_color4( 255, 0, 255 );
    l_colorData.SetCustomColour( i++, l_color4 );
    wxColour l_color5( 0, 255, 255 );
    l_colorData.SetCustomColour( i++, l_color5 );

    wxColourDialog dialog( this, &l_colorData );
    wxColour l_col;
    if( dialog.ShowModal() == wxID_OK )
    {
        wxColourData l_retData = dialog.GetColourData();
        l_col = l_retData.GetColour();
    }
    else
    {
        return;
    }

    if( m_pMainFrame->m_currentListIndex != -1 )
    {
        DatasetInfo *l_info = (DatasetInfo*)m_pMainFrame->m_pCurrentSceneObject;
        if( l_info->getType() == MESH || l_info->getType() == ISO_SURFACE || l_info->getType() == SURFACE || l_info->getType() == VECTORS)
        {
            l_info->setColor( l_col );
            l_info->setUseTex( false );
            m_pListCtrl->UpdateSelected();
        }
    }
    else if ( m_pMainFrame->getLastSelectedObj() != NULL )
    {
        SelectionObject *l_selObj = (SelectionObject*)m_pMainFrame->m_pCurrentSceneObject;
        if (!l_selObj->getIsMaster())
        {
            wxTreeItemId l_parentId = m_pMainFrame->m_pTreeWidget->GetItemParent( m_pMainFrame->getLastSelectedObj()->GetId() );
            l_selObj = (SelectionObject*)m_pMainFrame->m_pTreeWidget->GetItemData( l_parentId );
        }
        l_selObj->setFiberColor( l_col);
        l_selObj->setIsDirty( true );
        SceneManager::getInstance()->setSelBoxChanged( true );
    }    
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnCreateFibersDensityTexture( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnCreateFibersDensityTexture" ), LOGLEVEL_DEBUG );

    long index = MyApp::frame->getCurrentListIndex();
    if( -1 != index )
    {
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->m_pListCtrl->GetItem( index ) );

        if( pFibers == NULL )
            return ;

        int l_x, l_y, l_z;

        int index = DatasetManager::getInstance()->createAnatomy();
        Anatomy* pNewAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( index );

        int columns = DatasetManager::getInstance()->getColumns();
        int rows    = DatasetManager::getInstance()->getRows();
        int frames  = DatasetManager::getInstance()->getFrames();

        pNewAnatomy->setZero( columns, rows, frames );
        pNewAnatomy->setDataType( 16 );
        pNewAnatomy->setType( OVERLAY );
        float l_max = 0.0f;
        wxTreeItemId l_treeObjectId = m_pMainFrame->m_pTreeWidget->GetSelection();

        if( m_pMainFrame->treeSelected( l_treeObjectId ) == MASTER_OBJECT )
        {
            float voxelX = DatasetManager::getInstance()->getVoxelX();
            float voxelY = DatasetManager::getInstance()->getVoxelY();
            float voxelZ = DatasetManager::getInstance()->getVoxelZ();

            SelectionObject* l_object = (SelectionObject*)( m_pMainFrame->m_pTreeWidget->GetItemData( l_treeObjectId ) );

            std::vector<float>* l_dataset = pNewAnatomy->getFloatDataset();

            for( int l = 0; l < pFibers->getLineCount(); ++l )
            {
                if( l_object->m_inBranch[l] )
                {
                    unsigned int pc = pFibers->getStartIndexForLine(l) * 3;

                    for( int j = 0; j < pFibers->getPointsPerLine(l); ++j )
                    {
                        l_x = (int)( pFibers->getPointValue(pc) / voxelX );
                        ++pc;
                        l_y = (int)( pFibers->getPointValue(pc) / voxelY );
                        ++pc;
                        l_z = (int)( pFibers->getPointValue(pc) / voxelZ );
                        ++pc;

                        int index = l_x + l_y * columns + l_z * columns * rows;
                        l_dataset->at(index) += 1.0;
                        l_max = wxMax( l_max,l_dataset->at(index) );
                    }
                }
            }
            for( int i( 0 ); i < columns * rows * frames; ++i )
            {
                l_dataset->at(i) /= l_max;
            }
        }

        pNewAnatomy->setName( wxT(" (fiber_density)" ) );
        pNewAnatomy->setOldMax( l_max );

        m_pListCtrl->InsertItem( index );

        m_pMainFrame->refreshAllGLWidgets();
    }
}

void PropertiesWindow::OnCreateFibersColorTexture( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnCreateFibersColorTexture" ), LOGLEVEL_DEBUG );

    long index = MyApp::frame->getCurrentListIndex();
    if( -1 != index )
    {
        Fibers* pFibers = DatasetManager::getInstance()->getSelectedFibers( MyApp::frame->m_pListCtrl->GetItem( index ) );

        if( NULL == pFibers )
            return ;

        int l_x, l_y, l_z;
        
        DatasetIndex dsIndex = DatasetManager::getInstance()->createAnatomy();
        Anatomy* pNewAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( dsIndex );

        int columns = DatasetManager::getInstance()->getColumns();
        int rows    = DatasetManager::getInstance()->getRows();
        int frames  = DatasetManager::getInstance()->getFrames();

        pNewAnatomy->setRGBZero( columns, rows, frames );

        wxTreeItemId l_treeObjectId = m_pMainFrame->m_pTreeWidget->GetSelection();
        if(m_pMainFrame-> treeSelected( l_treeObjectId ) == MASTER_OBJECT )
        {
            float voxelX = DatasetManager::getInstance()->getVoxelX();
            float voxelY = DatasetManager::getInstance()->getVoxelY();
            float voxelZ = DatasetManager::getInstance()->getVoxelZ();

            SelectionObject* l_object = (SelectionObject*)( m_pMainFrame->m_pTreeWidget->GetItemData( l_treeObjectId ) );
            wxColour l_color = l_object->getFiberColor();

            std::vector<float>* l_dataset = pNewAnatomy->getFloatDataset();

            for( int l = 0; l < pFibers->getLineCount(); ++l )
            {
                if( l_object->m_inBranch[l] )
                {
                    unsigned int pc = pFibers->getStartIndexForLine( l ) * 3;

                    for( int j = 0; j < pFibers->getPointsPerLine( l ) ; ++j )
                    {
                        l_x = (int)( pFibers->getPointValue( pc ) / voxelX );
                        ++pc;
                        l_y = (int)( pFibers->getPointValue( pc ) / voxelY );
                        ++pc;
                        l_z = (int)( pFibers->getPointValue( pc ) / voxelZ );
                        ++pc;

                        int index = 3 * ( l_x + l_y * columns + l_z * columns * rows );
                        l_dataset->at( index )     = l_color.Red()   / 255.0f;
                        l_dataset->at( index + 1 ) = l_color.Green() / 255.0f;
                        l_dataset->at( index + 2 ) = l_color.Blue()  / 255.0f;
                    }
                }
            }
        }

        pNewAnatomy->setName( wxT( " (fiber_colors)" ) );
        
        m_pListCtrl->InsertItem( dsIndex );
        
        m_pMainFrame->refreshAllGLWidgets();
    }
}

void PropertiesWindow::OnMeanComboBoxSelectionChange( wxCommandEvent& event)
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnMeanComboBoxSelectionChange" ), LOGLEVEL_DEBUG );

    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->SetFiberInfoGridValues();
    m_pMainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnBoxPositionX( wxCommandEvent &event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnBoxPositionX" ), LOGLEVEL_DEBUG );

    double posX = 0;
    Vector currPos;
    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->m_pTxtBoxX->GetValue().ToDouble(&posX);  
    currPos = ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->getCenter();
    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->setCenter(posX,currPos.y,currPos.z);
}

void PropertiesWindow::OnBoxPositionY( wxCommandEvent &event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnBoxPositionY" ), LOGLEVEL_DEBUG );

    double posY = 0;
    Vector currPos;
    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->m_pTxtBoxY->GetValue().ToDouble(&posY);  
    currPos = ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->getCenter();
    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->setCenter(currPos.x,posY,currPos.z);
}

void PropertiesWindow::OnBoxPositionZ( wxCommandEvent &event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnBoxPositionZ" ), LOGLEVEL_DEBUG );

    double posZ = 0;
    Vector currPos;
    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->m_pTxtBoxZ->GetValue().ToDouble(&posZ);  
    currPos = ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->getCenter();
    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->setCenter(currPos.x,currPos.y,posZ);
}

void PropertiesWindow::OnBoxSizeX( wxCommandEvent &event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnBoxSizeX" ), LOGLEVEL_DEBUG );
    double sizeX = 0;
    Vector currSize;
    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->m_pTxtSizeX->GetValue().ToDouble(&sizeX);  
    currSize = ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->getSize();
    currSize.x = sizeX / DatasetManager::getInstance()->getVoxelX();
    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->setSize(currSize);
}

void PropertiesWindow::OnBoxSizeY( wxCommandEvent &event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnBoxSizeY" ), LOGLEVEL_DEBUG );

    double sizeY = 0;
    Vector currSize;
    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->m_pTxtSizeY->GetValue().ToDouble(&sizeY);  
    currSize = ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->getSize();
    currSize.y = sizeY / DatasetManager::getInstance()->getVoxelY();
    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->setSize(currSize);
}

void PropertiesWindow::OnBoxSizeZ( wxCommandEvent &event )
{
    Logger::getInstance()->print( wxT( "Event triggered - PropertiesWindow::OnBoxSizeZ" ), LOGLEVEL_DEBUG );

    double sizeZ = 0;
    Vector currSize;
    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->m_pTxtSizeZ->GetValue().ToDouble(&sizeZ);  
    currSize = ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->getSize();
    currSize.z = sizeZ / DatasetManager::getInstance()->getVoxelZ();
    ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->setSize(currSize);
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