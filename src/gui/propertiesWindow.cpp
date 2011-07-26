#include <wx/colordlg.h>

#include "propertiesWindow.h"
#include "SelectionBox.h"
#include "SelectionEllipsoid.h"
#include "../dataset/Anatomy.h"
#include "../dataset/fibers.h"
#include "../dataset/ODFs.h"
#include "../dataset/splinePoint.h"
#include "../dataset/surface.h"
#include "../dataset/Tensors.h"
#include "../misc/IsoSurface/CIsoSurface.h"

IMPLEMENT_DYNAMIC_CLASS(PropertiesWindow, wxScrolledWindow)

BEGIN_EVENT_TABLE(PropertiesWindow, wxScrolledWindow)
EVT_PAINT(      PropertiesWindow::OnPaint)
EVT_SIZE(       PropertiesWindow::OnSize)
END_EVENT_TABLE()


PropertiesWindow::PropertiesWindow( MainFrame *parent, wxWindowID id,
                    const wxPoint &pos, const wxSize &size )
        : wxScrolledWindow( parent, id, pos, size, wxBORDER_NONE, _T("test canvas") )
{
    m_mainFrame = parent;
    SetBackgroundColour( *wxLIGHT_GREY );
    SetCursor( wxCursor( wxCURSOR_HAND ) );
    propertiesSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( propertiesSizer );
    SetAutoLayout(true);
}

void PropertiesWindow::OnSize( wxSizeEvent &WXUNUSED(event) )
{

}

void PropertiesWindow::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
    wxPaintDC dc( this );
}

void PropertiesWindow::OnListItemUp(wxCommandEvent& WXUNUSED(event))
{
    m_mainFrame->m_listCtrl->moveItemUp(m_mainFrame->m_currentListItem);
    m_mainFrame->m_listCtrl->EnsureVisible(m_mainFrame->m_currentListItem);   
    m_mainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnListItemDown( wxCommandEvent& WXUNUSED(event) )
{
    m_mainFrame->m_listCtrl->moveItemDown(m_mainFrame->m_currentListItem);
    m_mainFrame->m_listCtrl->EnsureVisible(m_mainFrame->m_currentListItem);
    m_mainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnDeleteListItem( wxEvent& WXUNUSED(event) )
{
    m_mainFrame->deleteListItem();
}

void PropertiesWindow::OnToggleShowFS( wxEvent& WXUNUSED(event) )
{
    if (m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1)
    {
        if( ! ((DatasetInfo*)m_mainFrame->m_currentSceneObject)->toggleShowFS())
            m_mainFrame->m_listCtrl->SetItem( m_mainFrame->m_currentListItem, 1, ((DatasetInfo*)m_mainFrame->m_currentSceneObject)->getName().BeforeFirst( '.' ) + wxT( "*" ) );
        else
            m_mainFrame->m_listCtrl->SetItem( m_mainFrame->m_currentListItem, 1, ((DatasetInfo*)m_mainFrame->m_currentSceneObject)->getName().BeforeFirst( '.' ) );
        m_mainFrame->refreshAllGLWidgets();
    }
}

void PropertiesWindow::OnListItemShow( wxCommandEvent&  WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject == NULL && m_mainFrame->m_currentListItem != -1)
        return;

    DatasetInfo* l_info = (DatasetInfo*)m_mainFrame->m_listCtrl->GetItemData( m_mainFrame->m_currentListItem );
    if( l_info->toggleShow() )
        m_mainFrame->m_listCtrl->SetItem( m_mainFrame->m_currentListItem, 0, wxT( "" ), 0 );
    else
        m_mainFrame->m_listCtrl->SetItem( m_mainFrame->m_currentListItem, 0, wxT( "" ), 1 );

    m_mainFrame->refreshAllGLWidgets();
}


void PropertiesWindow::OnSliderIntensityThresholdMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        DatasetInfo* l_current = (DatasetInfo*)m_mainFrame->m_currentSceneObject;
        float l_threshold = (float)l_current->m_psliderThresholdIntensity->GetValue() / 100.0f;

        if( l_current->getUseTex() )
            m_mainFrame->m_listCtrl->SetItem( m_mainFrame->m_currentListItem, 2, wxString::Format( wxT( "%.2f" ),   l_threshold * l_current->getOldMax() ) );
        else
            m_mainFrame->m_listCtrl->SetItem( m_mainFrame->m_currentListItem, 2, wxString::Format( wxT( "(%.2f)" ), l_threshold * l_current->getOldMax() ) );

        l_current->setThreshold( l_threshold );
        if( l_current->getType() == SURFACE )
        {
            Surface* s = (Surface*)l_current;
            s->movePoints();
        }
        if( l_current->getType() == ISO_SURFACE && ! l_current->m_psliderThresholdIntensity->leftDown() )
        {
            CIsoSurface* s = (CIsoSurface*)l_current;
            s->GenerateWithThreshold();
        }
        if( l_current->getType() < RGB )
        {
            Anatomy* a = (Anatomy*)l_current;
            if( a->m_roi )
                a->m_roi->setThreshold( l_threshold );
        }
        // This slider will set the Brightness level. Currently only the glyphs uses this value.
        l_current->setBrightness( 1.0f - l_threshold );
        m_mainFrame->refreshAllGLWidgets();
    }
}

void PropertiesWindow::OnSliderOpacityThresholdMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        DatasetInfo* l_current = (DatasetInfo*)m_mainFrame->m_currentSceneObject;
        l_current->setAlpha( (float)l_current->m_psliderOpacity->GetValue() / 100.0f);
        m_mainFrame->refreshAllGLWidgets();
    }
}

void PropertiesWindow::OnDilateDataset( wxCommandEvent& WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        if( ((DatasetInfo*)m_mainFrame->m_currentSceneObject)->getType() < MESH )
        {
            ((Anatomy*)m_mainFrame->m_currentSceneObject)->dilate();
        }
    }
}

void PropertiesWindow::OnErodeDataset(wxCommandEvent& WXUNUSED(event))
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        if( ((DatasetInfo*)m_mainFrame->m_currentSceneObject)->getType() < MESH )
        {
            ((Anatomy*)m_mainFrame->m_currentSceneObject)->erode();
        }
    }
}

void PropertiesWindow::OnMinimizeDataset( wxCommandEvent& WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        if( ((DatasetInfo*)m_mainFrame->m_currentSceneObject)->getType() < MESH )
        {
            ((Anatomy*)m_mainFrame->m_currentSceneObject)->minimize();
        }
    }
}

void PropertiesWindow::OnListItemCutOut( wxCommandEvent&  WXUNUSED(event) )
{
    m_mainFrame->m_datasetHelper->createCutDataset();
    m_mainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnNewIsoSurface( wxCommandEvent& WXUNUSED(event) )
{
    m_mainFrame->m_datasetHelper->createIsoSurface();
}

void PropertiesWindow::OnNewOffsetSurface( wxCommandEvent& WXUNUSED(event ))
{
    m_mainFrame->m_datasetHelper->createDistanceMapAndIso();
}

void PropertiesWindow::OnNewDistanceMap (wxCommandEvent& WXUNUSED(event))
{
    m_mainFrame->m_datasetHelper->createDistanceMap();
}

void PropertiesWindow::OnNewVoiFromOverlay( wxCommandEvent& WXUNUSED(event) )
{
    wxTreeItemId     l_treeObjectId     = m_mainFrame->m_tSelectionObjectsId;
    SelectionObject* l_selectionObject  = NULL;
    Anatomy*         l_anatomy          = NULL;

    if (m_mainFrame->m_datasetHelper->m_lastSelectedObject !=NULL)
    {
        l_treeObjectId = m_mainFrame->m_datasetHelper->m_lastSelectedObject->GetId();
    }

    if(m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1)
    {
        if ( ((DatasetInfo*)m_mainFrame->m_currentSceneObject)->getType() < RGB)
        {
            l_anatomy = (Anatomy*)m_mainFrame->m_currentSceneObject;
            l_selectionObject = new SelectionBox( m_mainFrame->m_datasetHelper, l_anatomy );
            float trs = l_anatomy->getThreshold();
            if( trs == 0.0 )
                trs = 0.01f;
            l_selectionObject->setThreshold( trs );
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

    if( m_mainFrame->treeSelected( l_treeObjectId ) == MASTER_OBJECT)
    {        
        wxTreeItemId l_treeNewObjectId  = m_mainFrame->m_treeWidget->AppendItem( l_treeObjectId, l_selectionObject->getName(), 0, -1,l_selectionObject);
        m_mainFrame->m_treeWidget->SetItemBackgroundColour( l_treeNewObjectId, *wxGREEN );
        m_mainFrame->m_treeWidget->EnsureVisible( l_treeNewObjectId );
        m_mainFrame->m_treeWidget->SetItemImage( l_treeNewObjectId, l_selectionObject->getIcon() );
        l_selectionObject->setTreeId( l_treeNewObjectId );
        l_selectionObject->setIsMaster( false );        
    }
    else
    {
        wxTreeItemId l_treeNewObjectId = m_mainFrame->m_treeWidget->AppendItem(m_mainFrame-> m_tSelectionObjectsId, l_selectionObject->getName(), 0, -1, l_selectionObject );
        m_mainFrame->m_treeWidget->SetItemBackgroundColour( l_treeNewObjectId, *wxCYAN );
        m_mainFrame->m_treeWidget->EnsureVisible( l_treeNewObjectId );
        m_mainFrame->m_treeWidget->SetItemImage( l_treeNewObjectId, l_selectionObject->getIcon() );
        l_selectionObject->setTreeId( l_treeNewObjectId );
        l_selectionObject->setIsMaster( true );        
    }
    l_anatomy->m_roi = l_selectionObject;

    m_mainFrame->m_datasetHelper->m_selBoxChanged = true;
    m_mainFrame->refreshAllGLWidgets();
}


void PropertiesWindow::OnSegment(wxCommandEvent& WXUNUSED(event))
{
    m_mainFrame->m_datasetHelper->m_isSegmentActive = !m_mainFrame->m_datasetHelper->m_isSegmentActive;

    if(!m_mainFrame->m_mainGL->object.empty())
        m_mainFrame->m_mainGL->object.clear();
    if(!m_mainFrame->m_mainGL->background.empty())
        m_mainFrame->m_mainGL->background.clear();

    m_mainFrame->m_datasetHelper->m_isObjfilled = false;
    m_mainFrame->m_datasetHelper->m_isBckfilled = false;
    m_mainFrame->m_datasetHelper->m_isObjCreated = false;
    m_mainFrame->m_datasetHelper->m_isBckCreated = false;
}

void PropertiesWindow::OnFloodFill(wxCommandEvent& WXUNUSED(event))
{
    m_mainFrame->m_datasetHelper->m_SegmentMethod = 0;
    m_mainFrame->m_datasetHelper->m_isFloodfillActive = true;
    m_mainFrame->m_datasetHelper->m_isSelectBckActive = false;
    m_mainFrame->m_datasetHelper->m_isSelectObjActive = false;
    ((Anatomy*)m_mainFrame->m_currentSceneObject)->toggleSegment();
}

void PropertiesWindow::OnSliderFloodMoved( wxCommandEvent& WXUNUSED(event) )
{
    float l_sliderValue = ((Anatomy*)m_mainFrame->m_currentSceneObject)->m_psliderFlood->GetValue() / 200.0f;
    ((Anatomy*)m_mainFrame->m_currentSceneObject)->setFloodThreshold(l_sliderValue);
    ((Anatomy*)m_mainFrame->m_currentSceneObject)->m_ptxtThresBox->SetValue(wxString::Format( wxT( "%.2f"), l_sliderValue));
}

void PropertiesWindow::OnSliderGraphSigmaMoved( wxCommandEvent& WXUNUSED(event) )
{
    ((Anatomy*)m_mainFrame->m_currentSceneObject)->setGraphSigma(((Anatomy*)m_mainFrame->m_currentSceneObject)->m_psliderGraphSigma->GetValue());
    std::cout << (((Anatomy*)m_mainFrame->m_currentSceneObject)->m_psliderGraphSigma->GetValue()) << endl;
}

void PropertiesWindow::OnKmeans( wxCommandEvent& WXUNUSED(event) )
{
    m_mainFrame->m_datasetHelper->m_SegmentMethod = 2;
    m_mainFrame->m_mainGL->segmentTumor();
}

void PropertiesWindow::OnSelectObj(wxCommandEvent& WXUNUSED(event))
{
    m_mainFrame->m_datasetHelper->m_SegmentMethod = 1;
    m_mainFrame->m_datasetHelper->m_isSelectBckActive = false;
    m_mainFrame->m_datasetHelper->m_isFloodfillActive = false;
    m_mainFrame->m_datasetHelper->m_isSelectObjActive = true;
}

void PropertiesWindow::OnSelectBck(wxCommandEvent& WXUNUSED(event))
{
    m_mainFrame->m_datasetHelper->m_SegmentMethod = 1;
    m_mainFrame->m_datasetHelper->m_isFloodfillActive = false;
    m_mainFrame->m_datasetHelper->m_isSelectBckActive = true;
    m_mainFrame->m_datasetHelper->m_isSelectObjActive = false;    
}

void PropertiesWindow::OnbtnGraphCut(wxCommandEvent& WXUNUSED(event))
{
    m_mainFrame->m_datasetHelper->m_SegmentMethod = 1;
    m_mainFrame->m_datasetHelper->m_isFloodfillActive = false;
    m_mainFrame->m_mainGL->segmentTumor();    
}


void PropertiesWindow::OnClean( wxCommandEvent& WXUNUSED(event) )
{
    if(m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_mainFrame->m_currentSceneObject;
        if( l_info->getType() == MESH || l_info->getType() == ISO_SURFACE)
            l_info->clean();
    }
    m_mainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnLoop( wxCommandEvent& WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        ((DatasetInfo*)m_mainFrame->m_currentSceneObject)->smooth();
    }
    m_mainFrame->refreshAllGLWidgets();
}


void PropertiesWindow::OnToggleLIC( wxCommandEvent& WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 && m_mainFrame->m_datasetHelper->m_vectorsLoaded )
    {
        ((DatasetInfo*) m_mainFrame->m_currentSceneObject)->activateLIC();
    }
    m_mainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnToggleDrawPointsMode( wxCommandEvent& event )
{
   m_mainFrame->OnToggleDrawPointsMode(event);
}

void PropertiesWindow::OnMoveBoundaryPointsLeft( wxCommandEvent& event )
{
    m_mainFrame->OnMoveBoundaryPointsLeft(event);
}

void PropertiesWindow::OnMoveBoundaryPointsRight(wxCommandEvent& event)
{
    m_mainFrame->OnMoveBoundaryPointsRight(event);
}

void PropertiesWindow::OnFibersFilter( wxCommandEvent& event)
{
    Fibers* pTmpFib = NULL;
    m_mainFrame->m_datasetHelper->getFiberDataset(pTmpFib);    
    if(pTmpFib != NULL)
    {
        pTmpFib->updateFibersFilters();
    }
}

void PropertiesWindow::OnGenerateFiberVolume( wxCommandEvent& WXUNUSED(event) )
{
    Fibers* pTmpFib = NULL;
    m_mainFrame->m_datasetHelper->getFiberDataset(pTmpFib);
    if(pTmpFib != NULL)
    {
        pTmpFib->generateFiberVolume();
    }
}

void PropertiesWindow::OnListMenuThreshold( wxCommandEvent&  WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject == NULL && m_mainFrame->m_currentListItem != -1)
        return;
    DatasetInfo* l_info = (DatasetInfo*)m_mainFrame->m_currentSceneObject;
    if( l_info->getType() >= MESH )
    {
        if( ! l_info->toggleUseTex() )
            m_mainFrame->m_listCtrl->SetItem( m_mainFrame->m_currentListItem, 2, wxT( "(" ) + wxString::Format( wxT( "%.2f" ), l_info->getThreshold() * l_info->getOldMax()) + wxT( ")" ) );
        else
            m_mainFrame->m_listCtrl->SetItem( m_mainFrame->m_currentListItem, 2, wxString::Format( wxT( "%.2f" ), l_info->getThreshold() * l_info->getOldMax() ) );
    }
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when the distance coloring option is
// selected when right-clicking on a fiber.
//////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnListMenuDistance( wxCommandEvent& WXUNUSED(event))
{
    if(m_mainFrame->m_datasetHelper->m_fiberColorationMode != DISTANCE_COLOR)
    {
        m_mainFrame->m_datasetHelper->m_fiberColorationMode = DISTANCE_COLOR;
        ColorFibers();
    }
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when the minimum distance coloring option is
// selected when right-clicking on a fiber.
//////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnListMenuMinDistance( wxCommandEvent& WXUNUSED(event))
{
    if(m_mainFrame->m_datasetHelper->m_fiberColorationMode != MINDISTANCE_COLOR)
    {
        m_mainFrame->m_datasetHelper->m_fiberColorationMode = MINDISTANCE_COLOR;
        ColorFibers();
    }
}


///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the color with curvature button
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnColorWithCurvature( wxCommandEvent& WXUNUSED(event) )
{
    if(m_mainFrame->m_datasetHelper->m_fiberColorationMode != CURVATURE_COLOR)
    {
        m_mainFrame->m_datasetHelper->m_fiberColorationMode = CURVATURE_COLOR;
        ColorFibers();
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display min/max cross section
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnColorWithTorsion( wxCommandEvent& WXUNUSED(event) )
{
    if(m_mainFrame->m_datasetHelper->m_fiberColorationMode != TORSION_COLOR)
    {
        m_mainFrame->m_datasetHelper->m_fiberColorationMode = TORSION_COLOR;
        ColorFibers();
    }
}

void PropertiesWindow::OnNormalColoring( wxCommandEvent& WXUNUSED(event) )
{
    if(m_mainFrame->m_datasetHelper->m_fiberColorationMode != NORMAL_COLOR)
    {
        m_mainFrame->m_datasetHelper->m_fiberColorationMode = NORMAL_COLOR;
        ColorFibers();
    }
}

//////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user clicks on the "Set as distance
// anchor" option.
//////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnDistanceAnchorSet( wxCommandEvent& event )
{
    if (m_mainFrame->m_datasetHelper->m_lastSelectedObject!=NULL)
    {
        m_mainFrame->m_datasetHelper->m_lastSelectedObject->UseForDistanceColoring(!m_mainFrame->m_datasetHelper->m_lastSelectedObject->IsUsedForDistanceColoring());
        ColorFibers();
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will call the updateFibersColors function on the currently loaded fiber set.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::ColorFibers()
{   
    if (m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1)
    {
        ((Fibers*)m_mainFrame->m_currentSceneObject)->updateFibersColors();  
    }  
}
void PropertiesWindow::OnOriginalShBasis( wxCommandEvent& WXUNUSED(event) )
{
    ODFs* l_dataset = new ODFs( m_mainFrame->m_datasetHelper );
    ((ODFs*)m_mainFrame->m_currentSceneObject)->changeShBasis(l_dataset, m_mainFrame->m_datasetHelper, 0);

}

void PropertiesWindow::OnDescoteauxShBasis( wxCommandEvent& WXUNUSED(event) )
{
    ODFs* l_dataset = new ODFs( m_mainFrame->m_datasetHelper );
    ((ODFs*)m_mainFrame->m_currentSceneObject)->changeShBasis(l_dataset, m_mainFrame->m_datasetHelper, 1);

}

void PropertiesWindow::OnTournierShBasis( wxCommandEvent& WXUNUSED(event) )
{
    ODFs* l_dataset = new ODFs( m_mainFrame->m_datasetHelper );
    ((ODFs*)m_mainFrame->m_currentSceneObject)->changeShBasis(l_dataset, m_mainFrame->m_datasetHelper, 2);

}

void PropertiesWindow::OnPTKShBasis( wxCommandEvent& WXUNUSED(event) )
{
    ODFs* l_dataset = new ODFs( m_mainFrame->m_datasetHelper );
    ((ODFs*)m_mainFrame->m_currentSceneObject)->changeShBasis(l_dataset, m_mainFrame->m_datasetHelper, 3);
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the min hue value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphMinHueSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    updateGlyphColoration( MIN_HUE, ((Glyph*)m_mainFrame->m_currentSceneObject)->m_psliderMinHueValue->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the max hue value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphMaxHueSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    updateGlyphColoration( MAX_HUE, ((Glyph*)m_mainFrame->m_currentSceneObject)->m_psliderMaxHueValue->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the saturation value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphSaturationSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    updateGlyphColoration( SATURATION,((Glyph*)m_mainFrame->m_currentSceneObject)->m_psliderSaturationValue->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the luminance value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphLuminanceSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    updateGlyphColoration( LUMINANCE, ((Glyph*)m_mainFrame->m_currentSceneObject)->m_psliderLuminanceValue->GetValue() / 100.0f );
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
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {            
        DatasetInfo* l_info = (DatasetInfo*)m_mainFrame->m_currentSceneObject;    
        if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
            ( (Glyph*)l_info )->setColor( i_modifier, i_value );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the LOD of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphLODSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {            
        DatasetInfo* l_info = (DatasetInfo*)m_mainFrame->m_currentSceneObject;
        if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
        {
            ( (Glyph*)l_info )->setLOD( (LODChoices)((Glyph*)m_mainFrame->m_currentSceneObject)->m_psliderLODValue->GetValue() );
            //((ODFs*)l_info)->isMaximasSet = false;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the light attenuation of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphLightAttenuationSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {            
        DatasetInfo* l_info = (DatasetInfo*)m_mainFrame->m_currentSceneObject;
        if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
            ( (Glyph*)l_info )->setLighAttenuation( ((Glyph*)m_mainFrame->m_currentSceneObject)->m_psliderLightAttenuation->GetValue() / 100.0f );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the light x position slider moved.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphLightXDirectionSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        OnGlyphLightPositionChanged( X_AXIS, ((Glyph*)m_mainFrame->m_currentSceneObject)->m_psliderLightXPosition->GetValue() / 100.0f  );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the light y position slider moved.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphLightYDirectionSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        OnGlyphLightPositionChanged( Y_AXIS, ((Glyph*)m_mainFrame->m_currentSceneObject)->m_psliderLightYPosition->GetValue() / 100.0f  );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the light z position slider moved.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphLightZDirectionSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        OnGlyphLightPositionChanged( Z_AXIS, ((Glyph*)m_mainFrame->m_currentSceneObject)->m_psliderLightZPosition->GetValue() / 100.0f  );
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
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        ((Glyph*)m_mainFrame->m_currentSceneObject)->setLightPosition( i_axisType, i_position);
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the display value of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphDisplaySliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        ((Glyph*)m_mainFrame->m_currentSceneObject)->setDisplayFactor( ((Glyph*)m_mainFrame->m_currentSceneObject)->m_psliderDisplayValue->GetValue());
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the scaling factor of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphScalingFactorSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        ((Glyph*)m_mainFrame->m_currentSceneObject)->setScalingFactor( ((Glyph*)m_mainFrame->m_currentSceneObject)->m_psliderScalingFactor->GetValue());
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the x flip check box in the 
// glyph options is checked/unchecked.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphXAxisFlipChecked( wxCommandEvent& event )
{
    OnGlyphFlip( X_AXIS, event.IsChecked() );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the y flip check box in the 
// glyph options is checked/unchecked.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphYAxisFlipChecked( wxCommandEvent& event )
{
    OnGlyphFlip( Y_AXIS, event.IsChecked() );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the z flip check box in the 
// glyph options is checked/unchecked.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphZAxisFlipChecked( wxCommandEvent& event )
{
    OnGlyphFlip( Z_AXIS, event.IsChecked() );
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
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        ((Glyph*)m_mainFrame->m_currentSceneObject)->flipAxis( i_axisType, i_isChecked );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the map on sphere radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphMapOnSphereSelected( wxCommandEvent& event )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        ((Glyph*)m_mainFrame->m_currentSceneObject)->setDisplayShape( SPHERE );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the normal display radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphNormalSelected( wxCommandEvent& event )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        ((Glyph*)m_mainFrame->m_currentSceneObject)->setDisplayShape( NORMAL );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the axes display radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphAxesSelected( wxCommandEvent& event )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        ((Glyph*)m_mainFrame->m_currentSceneObject)->setDisplayShape( AXES );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the main axis display radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphMainAxisSelected( wxCommandEvent& event )
{

    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        if(((DatasetInfo*)m_mainFrame->m_currentSceneObject)->getType() == ODFS && !((ODFs*)m_mainFrame->m_currentSceneObject)->isMaximasSet)
        {
            ((ODFs*)m_mainFrame->m_currentSceneObject)->extractMaximas();
        }
        ((Glyph*)m_mainFrame->m_currentSceneObject)->setDisplayShape( AXIS );
        ((Glyph*)m_mainFrame->m_currentSceneObject)->updatePropertiesSizer();
      
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the color with position check box in the 
// glyph options is checked/unchecked.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnGlyphColorWithPosition( wxCommandEvent& event )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {
        ((Glyph*)m_mainFrame->m_currentSceneObject)->setColorWithPosition( event.IsChecked() );
    }
}

void PropertiesWindow::OnNormalizeTensors( wxCommandEvent& event )
{
    if( m_mainFrame->m_currentSceneObject != NULL && m_mainFrame->m_currentListItem != -1 )
    {        
        ((Tensors*)m_mainFrame->m_currentSceneObject)->normalize();        
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display fibers 
// info after a right click in the tree on a selectio object.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnDisplayFibersInfo( wxCommandEvent& WXUNUSED(event) )
{
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->SetFiberInfoGridValues();
    m_mainFrame->refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display mean fiber 
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnDisplayMeanFiber( wxCommandEvent& WXUNUSED(event) )
{

}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display cross sections
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnDisplayCrossSections( wxCommandEvent& WXUNUSED(event) )
{
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->m_displayCrossSections = (CrossSectionsDisplay)( ( (int)((SelectionObject*)m_mainFrame->m_currentSceneObject)->m_displayCrossSections ) + 1 );
    if( ((SelectionObject*)m_mainFrame->m_currentSceneObject)->m_displayCrossSections == CS_NB_OF_CHOICES )
        ((SelectionObject*)m_mainFrame->m_currentSceneObject)->m_displayCrossSections = CS_NOTHING;
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display dispersion tube
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnDisplayDispersionTube( wxCommandEvent& WXUNUSED(event) )
{
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->m_displayDispersionCone = (DispersionConeDisplay)( ( (int)((SelectionObject*)m_mainFrame->m_currentSceneObject)->m_displayDispersionCone ) + 1 );
    if( ((SelectionObject*)m_mainFrame->m_currentSceneObject)->m_displayDispersionCone == DC_NB_OF_CHOICES )
        ((SelectionObject*)m_mainFrame->m_currentSceneObject)->m_displayDispersionCone = DC_NOTHING;
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the rename option is clicked on the right
// click menu of a SelectionObject item in the tree.
///////////////////////////////////////////////////////////////////////////
void PropertiesWindow::OnRenameBox( wxCommandEvent& WXUNUSED(event) )
{
    wxTreeItemId l_treeBoxId = m_mainFrame->m_treeWidget->GetSelection();
    if( m_mainFrame->treeSelected( l_treeBoxId ) == MASTER_OBJECT || m_mainFrame->treeSelected( l_treeBoxId ) == CHILD_OBJECT )
    {
        SelectionObject* l_box = (SelectionObject*)( m_mainFrame->m_treeWidget->GetItemData( l_treeBoxId ) );

        wxTextEntryDialog dialog(this, _T( "Please enter a new name" ) );
        dialog.SetValue( l_box->getName() );

        if( ( dialog.ShowModal() == wxID_OK ) && ( dialog.GetValue() != _T( "" ) ) )
            l_box->setName( dialog.GetValue() );

        m_mainFrame->m_treeWidget->SetItemText( l_treeBoxId, l_box->getName() );
    }
    m_mainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnToggleAndNot( wxCommandEvent& WXUNUSED(event) )
{
    if( !m_mainFrame->m_datasetHelper->m_theScene)
        return;

    // Get what selection object is selected.
    wxTreeItemId l_selectionObjectTreeId = m_mainFrame->m_treeWidget->GetSelection();

    if( m_mainFrame->treeSelected(l_selectionObjectTreeId) == CHILD_OBJECT)
    {
        SelectionObject* l_box = (SelectionObject*)( m_mainFrame->m_treeWidget->GetItemData( l_selectionObjectTreeId ) );
        l_box->toggleIsNOT();

        wxTreeItemId l_parentId = m_mainFrame->m_treeWidget->GetItemParent( l_selectionObjectTreeId );
        ((SelectionObject*)( m_mainFrame->m_treeWidget->GetItemData( l_parentId ) ) )->setIsDirty( true );

        if( ( (SelectionObject*)( m_mainFrame->m_treeWidget->GetItemData( l_selectionObjectTreeId ) ) )->getIsNOT() )
            m_mainFrame->m_treeWidget->SetItemBackgroundColour( l_selectionObjectTreeId, *wxRED   );
        else
            m_mainFrame->m_treeWidget->SetItemBackgroundColour( l_selectionObjectTreeId, *wxGREEN );

        m_mainFrame->m_treeWidget->SetItemImage( l_selectionObjectTreeId, l_box->getIcon() );
        l_box->setIsDirty( true );
    }
    m_mainFrame->refreshAllGLWidgets();
}


void PropertiesWindow::OnColorRoi( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_mainFrame->m_datasetHelper->m_theScene )
        return;

    // Get the currently selected object.
    wxTreeItemId l_selectionObjectTreeId = m_mainFrame->m_treeWidget->GetSelection();
    SelectionObject* l_selectionObject = (SelectionObject*)( m_mainFrame->m_treeWidget->GetItemData( l_selectionObjectTreeId ) );

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

    m_mainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnVoiFlipNormals( wxCommandEvent& WXUNUSED(event) )
{
    wxTreeItemId l_selectionObjectTreeId = m_mainFrame->m_treeWidget->GetSelection();
    SelectionObject* l_selectionObject = (SelectionObject*)( m_mainFrame->m_treeWidget->GetItemData( l_selectionObjectTreeId ) );

    if(l_selectionObject->getSelectionType() == CISO_SURFACE_TYPE)
    {
        l_selectionObject->FlipNormals();        
    }
}

void PropertiesWindow::OnDeleteTreeItem( wxTreeEvent&    event )
{
    m_mainFrame->OnDeleteTreeItem(event);
}

void PropertiesWindow::OnActivateTreeItem ( wxTreeEvent&    event )
{
    m_mainFrame->OnActivateTreeItem(event);
}

void PropertiesWindow::OnToggleShowSelectionObject( wxCommandEvent& WXUNUSED(event) )
{
    if( !m_mainFrame->m_datasetHelper->m_theScene)
        return;

    // Get the selected selection object.
    wxTreeItemId l_selectionObjectTreeId = m_mainFrame->m_treeWidget->GetSelection();

    if( m_mainFrame->treeSelected( l_selectionObjectTreeId ) == MASTER_OBJECT )
    {
        SelectionObject* l_selecitonObject = (SelectionObject*)( m_mainFrame->m_treeWidget->GetItemData( l_selectionObjectTreeId ) );
        l_selecitonObject->toggleIsVisible();
        m_mainFrame->m_treeWidget->SetItemImage( l_selectionObjectTreeId, l_selecitonObject->getIcon() );
        l_selecitonObject->setIsDirty( true );

        int l_childSelectionObjects = m_mainFrame->m_treeWidget->GetChildrenCount( l_selectionObjectTreeId );
        wxTreeItemIdValue childcookie = 0;
        for( int i = 0; i < l_childSelectionObjects; ++i )
        {
            wxTreeItemId l_childId = m_mainFrame->m_treeWidget->GetNextChild( l_selectionObjectTreeId, childcookie );
            if( l_childId.IsOk() )
            {
                SelectionObject* childBox = ( (SelectionObject*)( m_mainFrame->m_treeWidget->GetItemData( l_childId ) ) );
                childBox->setIsVisible( l_selecitonObject->getIsVisible() );
                m_mainFrame->m_treeWidget->SetItemImage( l_childId, childBox->getIcon() );
                childBox->setIsDirty( true );
            }
        }
    }
    else if( m_mainFrame->treeSelected( l_selectionObjectTreeId ) == CHILD_OBJECT )
    {
        SelectionObject *l_selectionObject = (SelectionObject*)( m_mainFrame->m_treeWidget->GetItemData( l_selectionObjectTreeId ) );
        l_selectionObject->toggleIsVisible();
        m_mainFrame->m_treeWidget->SetItemImage( l_selectionObjectTreeId, l_selectionObject->getIcon() );
        l_selectionObject->setIsDirty( true );
    }

    m_mainFrame->m_datasetHelper->m_selBoxChanged = true;
    m_mainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnAssignColor( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_mainFrame->m_datasetHelper->m_theScene )
        return;

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


    if( m_mainFrame->m_currentListItem != -1 )
    {
        DatasetInfo *l_info = (DatasetInfo*)m_mainFrame->m_currentSceneObject;
        if( l_info->getType() == MESH || l_info->getType() == ISO_SURFACE || l_info->getType() == SURFACE || l_info->getType() == VECTORS)
        {
            l_info->setColor( l_col );
            l_info->setuseTex( false );
            m_mainFrame->m_listCtrl->SetItem( m_mainFrame->m_currentListItem, 2, wxT( "(") + wxString::Format( wxT( "%.2f" ), l_info->getThreshold() ) + wxT( ")" ) );           
        }
    }
    else if ( m_mainFrame->m_datasetHelper->m_lastSelectedObject != NULL )
    {
        SelectionObject *l_selObj = (SelectionObject*)m_mainFrame->m_currentSceneObject;
        if (!l_selObj->getIsMaster())
        {
            wxTreeItemId l_parentId = m_mainFrame->m_treeWidget->GetItemParent( m_mainFrame->m_datasetHelper->m_lastSelectedObject->GetId());
            l_selObj = (SelectionObject*)m_mainFrame->m_treeWidget->GetItemData(l_parentId);
        }
        l_selObj->setFiberColor( l_col);
        l_selObj->setIsDirty( true );
        m_mainFrame->m_datasetHelper->m_selBoxChanged = true;
    }    
    m_mainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnCreateFibersDensityTexture( wxCommandEvent& WXUNUSED(event) )
{
    Fibers* l_fibers = NULL;

    if( ! m_mainFrame->m_datasetHelper->getFiberDataset(l_fibers) )
        return ;

    int l_x,l_y,l_z;

    Anatomy* l_newAnatomy = new Anatomy( m_mainFrame->m_datasetHelper );
    l_newAnatomy->setZero( m_mainFrame->m_datasetHelper->m_columns, m_mainFrame->m_datasetHelper->m_rows, m_mainFrame->m_datasetHelper->m_frames );
    l_newAnatomy->setDataType( 16 );
    l_newAnatomy->setType( OVERLAY );
    float l_max = 0.0f;
    wxTreeItemId l_treeObjectId = m_mainFrame->m_treeWidget->GetSelection();
    if( m_mainFrame->treeSelected( l_treeObjectId ) == MASTER_OBJECT )
    {
        SelectionObject* l_object = (SelectionObject*)( m_mainFrame->m_treeWidget->GetItemData( l_treeObjectId ) );

        std::vector<float>* l_dataset = l_newAnatomy->getFloatDataset();

        for( int l = 0; l < l_fibers->getLineCount(); ++l )
        {
            if( l_object->m_inBranch[l] )
            {
                unsigned int pc = l_fibers->getStartIndexForLine(l)*3;

                for( int j = 0; j < l_fibers->getPointsPerLine(l) ; ++j )
                {
                    l_x = (int)( l_fibers->getPointValue(pc) / m_mainFrame->m_datasetHelper->m_xVoxel );
                    ++pc;
                    l_y = (int)( l_fibers->getPointValue(pc) / m_mainFrame->m_datasetHelper->m_yVoxel );
                    ++pc;
                    l_z = (int)( l_fibers->getPointValue(pc) / m_mainFrame->m_datasetHelper->m_zVoxel );
                    ++pc;

                    int index =( l_x + l_y * m_mainFrame->m_datasetHelper->m_columns + l_z * m_mainFrame->m_datasetHelper->m_columns * m_mainFrame->m_datasetHelper->m_rows );
                    l_dataset->at(index) += 1.0;
                    l_max = wxMax( l_max,l_dataset->at(index) );
                }
            }
        }
        for( int i = 0 ; i < m_mainFrame->m_datasetHelper->m_columns * m_mainFrame->m_datasetHelper->m_rows * m_mainFrame->m_datasetHelper->m_frames ; ++i )
        {
            l_dataset->at(i) /= l_max;
        }
    }

    l_newAnatomy->setName( wxT(" (fiber_density)" ) );
    l_newAnatomy->setOldMax( l_max );
    m_mainFrame->m_listCtrl->InsertItem( 0, wxT( "" ), 0 );
    m_mainFrame->m_listCtrl->SetItem( 0, 1, l_newAnatomy->getName() );
    m_mainFrame->m_listCtrl->SetItem( 0, 2, wxT( "0.00" ) );
    m_mainFrame->m_listCtrl->SetItem( 0, 3, wxT( "" ), 1 );
    m_mainFrame->m_listCtrl->SetItemData( 0, (long) l_newAnatomy );
    m_mainFrame->m_listCtrl->SetItemState( 0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    m_mainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnCreateFibersColorTexture( wxCommandEvent& WXUNUSED(event) )
{
    Fibers* l_fibers = NULL;

    if( !m_mainFrame->m_datasetHelper->getFiberDataset(l_fibers) )
        return ;

    int l_x,l_y,l_z;
    Anatomy* l_newAnatomy = new Anatomy( m_mainFrame->m_datasetHelper );
    l_newAnatomy->setRGBZero( m_mainFrame->m_datasetHelper->m_columns, m_mainFrame->m_datasetHelper->m_rows, m_mainFrame->m_datasetHelper->m_frames );

    wxTreeItemId l_treeObjectId = m_mainFrame->m_treeWidget->GetSelection();
    if(m_mainFrame-> treeSelected( l_treeObjectId ) == MASTER_OBJECT )
    {
        SelectionObject* l_object = (SelectionObject*)( m_mainFrame->m_treeWidget->GetItemData( l_treeObjectId ) );
        wxColour l_color = l_object->getFiberColor();

        std::vector<float>* l_dataset = l_newAnatomy->getFloatDataset();

        for( int l = 0; l < l_fibers->getLineCount(); ++l )
        {
            if( l_object->m_inBranch[l] )
            {
                unsigned int pc = l_fibers->getStartIndexForLine( l ) * 3;

                for( int j = 0; j < l_fibers->getPointsPerLine( l ) ; ++j )
                {
                    l_x = (int)( l_fibers->getPointValue( pc ) / m_mainFrame->m_datasetHelper->m_xVoxel );
                    ++pc;
                    l_y = (int)( l_fibers->getPointValue( pc ) / m_mainFrame->m_datasetHelper->m_yVoxel );
                    ++pc;
                    l_z = (int)( l_fibers->getPointValue( pc ) / m_mainFrame->m_datasetHelper->m_zVoxel );
                    ++pc;

                    int index = ( l_x + l_y * m_mainFrame->m_datasetHelper->m_columns + l_z *m_mainFrame-> m_datasetHelper->m_columns * m_mainFrame->m_datasetHelper->m_rows ) * 3;
                    l_dataset->at( index )     = l_color.Red()   / 255.0f;
                    l_dataset->at( index + 1 ) = l_color.Green() / 255.0f;
                    l_dataset->at( index + 2 ) = l_color.Blue()  / 255.0f;
                }
            }
        }
    }
    l_newAnatomy->setName( wxT( " (fiber_colors)" ) );
    m_mainFrame->m_listCtrl->InsertItem( 0, wxT( "" ), 0 );
    m_mainFrame->m_listCtrl->SetItem( 0, 1, l_newAnatomy->getName() );
    m_mainFrame->m_listCtrl->SetItem( 0, 2, wxT( "0.00" ) );
    m_mainFrame->m_listCtrl->SetItem( 0, 3, wxT( "" ), 1 );
    m_mainFrame->m_listCtrl->SetItemData( 0, (long)l_newAnatomy );
    m_mainFrame->m_listCtrl->SetItemState( 0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    m_mainFrame->refreshAllGLWidgets();
}

void PropertiesWindow::OnBoxPositionX( wxCommandEvent &event )
{    
    double posX = 0;
    Vector currPos;
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->m_ctrlBoxX->GetValue().ToDouble(&posX);  
    currPos = ((SelectionObject*)m_mainFrame->m_currentSceneObject)->getCenter();
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->setCenter(posX,currPos.y,currPos.z);
}

void PropertiesWindow::OnBoxPositionY( wxCommandEvent &event )
{    
    double posY = 0;
    Vector currPos;
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->m_ctrlBoxY->GetValue().ToDouble(&posY);  
    currPos = ((SelectionObject*)m_mainFrame->m_currentSceneObject)->getCenter();
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->setCenter(currPos.x,posY,currPos.z);
}

void PropertiesWindow::OnBoxPositionZ( wxCommandEvent &event )
{    
    double posZ = 0;
    Vector currPos;
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->m_ctrlBoxZ->GetValue().ToDouble(&posZ);  
    currPos = ((SelectionObject*)m_mainFrame->m_currentSceneObject)->getCenter();
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->setCenter(currPos.x,currPos.y,posZ);
}

void PropertiesWindow::OnBoxSizeX( wxCommandEvent &event )
{    
    double sizeX = 0;
    Vector currSize;
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->m_ctrlBoxSizeX->GetValue().ToDouble(&sizeX);  
    currSize = ((SelectionObject*)m_mainFrame->m_currentSceneObject)->getSize();
    currSize.x = sizeX/m_mainFrame->m_datasetHelper->m_xVoxel;
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->setSize(currSize);
}

void PropertiesWindow::OnBoxSizeY( wxCommandEvent &event )
{    
    double sizeY = 0;
    Vector currSize;
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->m_ctrlBoxSizeY->GetValue().ToDouble(&sizeY);  
    currSize = ((SelectionObject*)m_mainFrame->m_currentSceneObject)->getSize();
    currSize.y = sizeY/m_mainFrame->m_datasetHelper->m_yVoxel;
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->setSize(currSize);
}

void PropertiesWindow::OnBoxSizeZ( wxCommandEvent &event )
{    
    double sizeZ = 0;
    Vector currSize;
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->m_ctrlBoxSizeZ->GetValue().ToDouble(&sizeZ);  
    currSize = ((SelectionObject*)m_mainFrame->m_currentSceneObject)->getSize();
    currSize.z = sizeZ/m_mainFrame->m_datasetHelper->m_zVoxel;
    ((SelectionObject*)m_mainFrame->m_currentSceneObject)->setSize(currSize);
}

void PropertiesWindow::OnSliderAxisMoved( wxCommandEvent& WXUNUSED(event) )
{
    float l_sliderValue = ((ODFs*)m_mainFrame->m_currentSceneObject)->m_psliderFlood->GetValue() / 10.0f;
    ((ODFs*)m_mainFrame->m_currentSceneObject)->m_axisThreshold = l_sliderValue;
    ((ODFs*)m_mainFrame->m_currentSceneObject)->m_ptxtThresBox->SetValue(wxString::Format( wxT( "%.1f"), l_sliderValue));

    std::cout << ((ODFs*)m_mainFrame->m_currentSceneObject)->m_axisThreshold << std::endl;
}

void PropertiesWindow::OnRecalcMainDir( wxCommandEvent& WXUNUSED(event) )
{   
    ((ODFs*)m_mainFrame->m_currentSceneObject)->extractMaximas();

    /*for( int z = 0; z < m_mainFrame->m_datasetHelper->m_frames; z++ )
        for( int y = 0; y < m_mainFrame->m_datasetHelper->m_rows; y++ )
            for( int x = 0; x < m_mainFrame->m_datasetHelper->m_columns; x++ )
            {
                int  currentIdx = ((Glyph*)m_mainFrame->m_currentSceneObject)->getGlyphIndex( z, y, x );

                if(((ODFs*)m_mainFrame->m_currentSceneObject)->getCoeffs().at(currentIdx)[0] != 0)
                    ((ODFs*)m_mainFrame->m_currentSceneObject)->mainDirections[currentIdx] = ((ODFs*)m_mainFrame->m_currentSceneObject)->getODFmaxNotNorm(((ODFs*)m_mainFrame->m_currentSceneObject)->getCoeffs().at(currentIdx),
                    ((ODFs*)m_mainFrame->m_currentSceneObject)->getShMatrix()[NB_OF_LOD - 1], ((ODFs*)m_mainFrame->m_currentSceneObject)->getPhiTheta()[NB_OF_LOD - 1],((ODFs*)m_mainFrame->m_currentSceneObject)->m_axisThreshold,((ODFs*)m_mainFrame->m_currentSceneObject)->angle,((ODFs*)m_mainFrame->m_currentSceneObject)->Nbors);
            }*/
}
