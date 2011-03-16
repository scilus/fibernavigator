/////////////////////////////////////////////////////////////////////////////
// Name:            menuBar.cpp
// Author:          GGirard
// Creation Date:   03/12/2010
//
// Description: MainFrame MenuBar
//
// Last modifications:
//      
/////////////////////////////////////////////////////////////////////////////

#include "menuBar.h"
#include "mainFrame.h"

MenuBar::MenuBar()
{
    m_menuFile = new wxMenu();
    m_itemLoad = m_menuFile->Append(wxID_ANY, wxT("Load\tCtrl-L"));
    //m_itemReloadShader = m_menuFile->Append(wxID_ANY, wxT("Reload Shader"));
    m_itemSaveSCN = m_menuFile->Append(wxID_ANY, wxT("Save Current Scene\tCtrl-S"));
    m_itemSaveSelectedFibers = m_menuFile->Append(wxID_ANY, wxT("Save Selected Fibers"));
    m_itemSaveSelectedSurface = m_menuFile->Append(wxID_ANY, wxT("Save Selected Surface"));
    m_itemSaveSelectedDataset = m_menuFile->Append(wxID_ANY, wxT("Save Selected Dataset"));
    m_menuFile->AppendSeparator();
    m_itemQuit = m_menuFile->Append(wxID_EXIT, wxT("Quit\tCtrl-Q"));

    m_menuView = new wxMenu();
    m_itemReset = m_menuView->Append(wxID_ANY, wxT("Reset"));
    m_menuView->AppendSeparator();
    m_itemLeft = m_menuView->Append(wxID_ANY, wxT("Left\tL"));
    m_itemRight = m_menuView->Append(wxID_ANY, wxT("Right\tR"));
    m_itemSuperior = m_menuView->Append(wxID_ANY, wxT("Superior\tS"));
    m_itemInferior = m_menuView->Append(wxID_ANY, wxT("Inferior\tI"));
    m_itemAnterior = m_menuView->Append(wxID_ANY, wxT("Anterior\tA"));
    m_itemPosterior = m_menuView->Append(wxID_ANY, wxT("Posterior\tP"));
    m_menuView->AppendSeparator();
    m_menuShowSlices = new wxMenu();
    m_menuView->AppendSubMenu(m_menuShowSlices, wxT("Show Slice"));
    m_itemToggleShowAxial = m_menuShowSlices->AppendCheckItem(wxID_ANY, wxT("Show Axial Slice"));
    m_itemToggleShowCoronal = m_menuShowSlices->AppendCheckItem(wxID_ANY, wxT("Show Coronal Slice"));
    m_itemToggleShowSagittal = m_menuShowSlices->AppendCheckItem(wxID_ANY, wxT("Show Sagittal Slice"));
    m_menuView->AppendSeparator();
    m_itemToggleShowCrosshair = m_menuView->AppendCheckItem(wxID_ANY, wxT("Show Crosshair"));
    m_itemToggleShowAxes = m_menuView->AppendCheckItem(wxID_ANY, wxT("Show Axes"));
    m_itemToggleShowProperties = m_menuView->AppendCheckItem(wxID_ANY, wxT("Show Properties Sheet"));

    m_menuVoi = new wxMenu();
    m_itemNewSelectionBox = m_menuVoi->Append(wxID_ANY, wxT("New Selection Box"));
    m_itemNewSelectionEllipsoid = m_menuVoi->Append(wxID_ANY, wxT("New Selection Ellipsoid"));
    m_menuVoi->AppendSeparator();
    m_itemToggleUseMorphing = m_menuVoi->AppendCheckItem(wxID_ANY, wxT("Morphing"));

    m_menuFibers = new wxMenu();
    m_itemResetFibersColors = m_menuFibers->Append(wxID_ANY, wxT("Reset Color on Fibers"));
    m_itemToogleInvertFibersSelection = m_menuFibers->AppendCheckItem(wxID_ANY, wxT("Inverse Fibers Selection"));
    m_itemToggleUseFakeTubes = m_menuFibers->AppendCheckItem(wxID_ANY, wxT("Use Fake Tubes"));
    m_itemToggleUseTransparency = m_menuFibers->AppendCheckItem(wxID_ANY, wxT("Use Transparent Fibers"));
 
    m_menuSurface = new wxMenu();
    m_itemNewSplineSurface = m_menuSurface->Append(wxID_ANY, wxT("New Spline Surface"));
    m_itemMoveBoundaryPointLeft = m_menuSurface->Append(wxID_ANY, wxT("Move Left"));
    m_itemMoveBoundaryPointRight = m_menuSurface->Append(wxID_ANY, wxT("Move Right"));
    
    m_menuOptions = new wxMenu();
    m_menuRuler = new wxMenu();
    m_itemToggleRuler = m_menuRuler->AppendCheckItem(wxID_ANY, wxT("Ruler"));
    m_itemRulerClear = m_menuRuler->Append(wxID_ANY, wxT("Clear Points"));
    m_itemRulerAdd = m_menuRuler->Append(wxID_ANY, wxT("Add New Point"));
    m_itemRulerRemove = m_menuRuler->Append(wxID_ANY, wxT("Remove Last Point"));
    m_menuOptions->AppendSubMenu(m_menuRuler, wxT("Ruler"));

    m_menuColorMaps = new wxMenu();
    m_itemGray = m_menuColorMaps->Append(wxID_ANY, wxT("Gray"));
    m_itemBlueGreenPurple = m_menuColorMaps->Append(wxID_ANY, wxT("Blue-Green-Purple"));
    m_itemRainbow = m_menuColorMaps->Append(wxID_ANY, wxT("Rainbow"));
    m_itemHotIron = m_menuColorMaps->Append(wxID_ANY, wxT("Hotiron"));
    m_itemRedYellow = m_menuColorMaps->Append(wxID_ANY, wxT("Red-Yellow"));
    m_itemBlueLightblue = m_menuColorMaps->Append(wxID_ANY, wxT("Blue-Lightblue"));
    m_menuOptions->AppendSubMenu(m_menuColorMaps,wxT("Color Maps"));  

    m_itemToggleDrawPoints = m_menuOptions->AppendCheckItem(wxID_ANY, wxT("Draw Points"));
    m_itemToggleDrawVectors = m_menuOptions->AppendCheckItem(wxID_ANY, wxT("Draw Vectors"));
    m_itemToggleLighting = m_menuOptions->AppendCheckItem(wxID_ANY, wxT("Lighting"));
    m_itemToggleClearToBlack = m_menuOptions->AppendCheckItem(wxID_ANY, wxT("Clear to Black"));
    m_itemToggleBlendTextureOnMesh = m_menuOptions->AppendCheckItem(wxID_ANY, wxT("Blend Tex. on Mesh"));
    m_itemToggleFilterISO = m_menuOptions->AppendCheckItem(wxID_ANY, wxT("Filter Iso"));
    m_itemToggleNormal = m_menuOptions->AppendCheckItem(wxID_ANY, wxT("Flip Normal"));
    //m_itemToggleColorMapLegend = m_menuOptions->AppendCheckItem(wxID_ANY, wxT("Show Color Map"));


    m_menuHelp = new wxMenu();
    m_itemKeyboardShortcuts = m_menuHelp->Append(wxID_ANY, wxT("Keyboard Shortcut"));
    //m_itemScreenShot = m_menuHelp->Append(wxID_ANY, wxT("ScreenShot"));
    //m_menuMovie = new  wxMenu();
    //m_itemAxialMovie = m_menuMovie->Append(wxID_ANY, wxT("Axial"));
    //m_itemCoronalMovie = m_menuMovie->Append(wxID_ANY, wxT("Coronal"));
    //m_itemSagittalMovie = m_menuMovie->Append(wxID_ANY, wxT("Sagittal"));
    //m_menuHelp->AppendSubMenu(m_menuMovie, wxT("Movie"));
    m_menuHelp->AppendSeparator();
    m_itemAbout = m_menuHelp->Append(wxID_ABOUT, wxT("About"));

    this->Append(m_menuFile, wxT("&File"));
    this->Append(m_menuView, wxT("&View"));
    this->Append(m_menuVoi, wxT("&VOI"));
    this->Append(m_menuFibers, wxT("&Fibers"));
    this->Append(m_menuSurface, wxT("Surface"));
    this->Append(m_menuOptions, wxT("&Option"));
    this->Append(m_menuHelp, wxT("&Help"));

#ifdef __WXMAC__

    // we need this in order to allow the about menu relocation, since ABOUT is
    // not the default id of the about menu
    wxApp::s_macAboutMenuItemId = m_itemAbout->GetId();

#endif     
}

void MenuBar::initMenuBar( MainFrame *mf )
{
    mf->Connect(m_itemLoad->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnLoad));
    //mf->Connect(m_itemReloadShader->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnReloadShaders));
    mf->Connect(m_itemSaveSCN->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSave));
    mf->Connect(m_itemSaveSelectedFibers->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSaveFibers));
    mf->Connect(m_itemSaveSelectedSurface->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSaveSurface));
    mf->Connect(m_itemSaveSelectedDataset->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSaveDataset));
    mf->Connect(m_itemQuit->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnQuit));    
    mf->Connect(m_itemToggleShowAxial->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnToggleShowAxial));
    mf->Connect(m_itemToggleShowCoronal->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnToggleShowCoronal));
    mf->Connect(m_itemToggleShowSagittal->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnToggleShowSagittal));
    mf->Connect(m_itemReset->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnMenuViewReset));
    mf->Connect(m_itemLeft->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnMenuViewLeft));
    mf->Connect(m_itemRight->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnMenuViewRight));
    mf->Connect(m_itemSuperior->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnMenuViewTop));
    mf->Connect(m_itemInferior->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnMenuViewBottom));
    mf->Connect(m_itemAnterior->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnMenuViewBack));
    mf->Connect(m_itemPosterior->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnMenuViewFront));
    mf->Connect(m_itemToggleShowCrosshair->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnMenuViewCrosshair));
    mf->Connect(m_itemToggleShowAxes->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnMenuViewAxes));
    mf->Connect(m_itemNewSelectionBox->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnNewSelectionBox));
    mf->Connect(m_itemNewSelectionEllipsoid->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnNewSelectionEllipsoid));
    mf->Connect(m_itemToggleUseMorphing->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnUseMorph));
    mf->Connect(m_itemResetFibersColors->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnResetColor));
    mf->Connect(m_itemToggleLighting->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnToggleLighting));    
    mf->Connect(m_itemToogleInvertFibersSelection->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnInvertFibers));
    mf->Connect(m_itemToggleUseFakeTubes->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnUseFakeTubes));
    mf->Connect(m_itemToggleClearToBlack->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnClearToBlack));
    mf->Connect(m_itemToggleBlendTextureOnMesh->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnToggleBlendTexOnMesh));
    mf->Connect(m_itemToggleFilterISO->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnToggleFilterIso));
    mf->Connect(m_itemToggleNormal->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnToggleNormal));
    //mf->Connect(m_itemToggleColorMapLegend->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnToggleColorMapLegend));
    mf->Connect(m_itemToggleRuler->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnRulerTool));
    mf->Connect(m_itemRulerClear->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnRulerToolClear));
    mf->Connect(m_itemRulerAdd->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnRulerToolAdd));
    mf->Connect(m_itemRulerRemove->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnRulerToolDel));
    mf->Connect(m_itemNewSplineSurface->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnNewSplineSurface));
    mf->Connect(m_itemToggleDrawPoints->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnToggleDrawPointsMode));
    mf->Connect(m_itemToggleDrawVectors->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnToggleDrawVectors));
    mf->Connect(m_itemMoveBoundaryPointLeft->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnMoveBoundaryPointsLeft));
    mf->Connect(m_itemMoveBoundaryPointRight->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnMoveBoundaryPointsRight));
    mf->Connect(m_itemToggleUseTransparency->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnUseTransparency));
    mf->Connect(m_itemGray->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSetCMapNo));
    mf->Connect(m_itemBlueGreenPurple->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSetCMap0));
    mf->Connect(m_itemRainbow->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSetCMap1));
    mf->Connect(m_itemHotIron->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSetCMap2));
    mf->Connect(m_itemRedYellow->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSetCMap3));
    mf->Connect(m_itemBlueLightblue->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSetCMap4));
    mf->Connect(m_itemAbout->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnAbout));
    mf->Connect(m_itemKeyboardShortcuts->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnShortcuts));
    //mf->Connect(m_itemScreenShot->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnScreenshot));
    //mf->Connect(m_itemAxialMovie->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSlizeMovieAxi));
    //mf->Connect(m_itemCoronalMovie->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSlizeMovieCor));
    //mf->Connect(m_itemSagittalMovie->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSlizeMovieSag));
    mf->Connect(m_itemToggleShowProperties->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnToggleShowProperties));
}

void MenuBar::updateMenuBar( MainFrame *mf )
{
    //m_itemSaveSelectedDataset->Enable(mf->m_currentListItem != -1 && (((DatasetInfo*)mf->m_currentFNObject)->getType()==HEAD_BYTE || ((DatasetInfo*)mf->m_currentFNObject)->getType()==HEAD_SHORT));
    
    m_itemToggleLighting->Check(mf->m_datasetHelper->m_lighting);
    m_itemToggleRuler->Check(mf->m_datasetHelper->m_isRulerToolActive);
    m_itemToogleInvertFibersSelection->Check(mf->m_datasetHelper->m_fibersInverted);
    m_itemToggleUseFakeTubes->Check(mf->m_datasetHelper->m_useFakeTubes);
    m_itemToggleUseTransparency->Check(mf->m_datasetHelper->m_useTransparency);
    m_itemToggleUseFakeTubes->Check(mf->m_datasetHelper->m_useFakeTubes);
    m_itemToggleUseMorphing->Check(mf->m_datasetHelper->m_morphing);
    m_itemToggleShowCrosshair->Check(mf->m_datasetHelper->m_showCrosshair);
    m_itemToggleShowAxial->Check(mf->m_datasetHelper->m_showAxial);
    m_itemToggleShowCoronal->Check(mf->m_datasetHelper->m_showCoronal);
    m_itemToggleShowSagittal->Check(mf->m_datasetHelper->m_showSagittal);
    m_itemToggleClearToBlack->Check(mf->m_datasetHelper->m_clearToBlack);
    m_itemToggleBlendTextureOnMesh->Check(mf->m_datasetHelper->m_blendTexOnMesh);
    m_itemToggleFilterISO->Check(mf->m_datasetHelper->m_filterIsoSurf);
    m_itemToggleShowAxes->Check(mf->m_datasetHelper->m_isShowAxes);
    //m_itemToggleColorMapLegend->Check(mf->m_datasetHelper->m_colorMap);
    m_itemToggleShowProperties->Check(mf->m_isDisplayProperties);     
    m_itemMoveBoundaryPointLeft->Enable(mf->m_datasetHelper->m_surfaceLoaded);
    m_itemMoveBoundaryPointRight->Enable(mf->m_datasetHelper->m_surfaceLoaded);
    m_itemToggleDrawPoints->Check(mf->m_datasetHelper->m_pointMode);
    m_itemToggleDrawVectors->Check(mf->m_datasetHelper->m_drawVectors);
    m_itemNewSplineSurface->Enable(mf->m_datasetHelper->m_anatomyLoaded && !mf->m_datasetHelper->m_surfaceLoaded);
    
}
