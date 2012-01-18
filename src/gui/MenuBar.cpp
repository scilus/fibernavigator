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

#include "MenuBar.h"
#include "MainFrame.h"
#include "../dataset/Fibers.h"
#include "../dataset/FibersGroup.h"

MenuBar::MenuBar()
{
    m_menuFile = new wxMenu();
	m_itemLoad = m_menuFile->Append(wxID_ANY, wxT("Open\tCtrl-O"));
    m_menuNewAnatomy = new wxMenu();
	m_itemNewAnatomyByte = m_menuNewAnatomy->Append(wxID_ANY, wxT("New White"));
	m_itemNewAnatomyRGB = m_menuNewAnatomy->Append(wxID_ANY, wxT("New RGB"));
    m_menuFile->AppendSubMenu(m_menuNewAnatomy, wxT("New Empty Anatomy"));
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

    m_menuNavigate = new wxMenu();
    m_menuView->AppendSubMenu(m_menuNavigate, wxT("Navigate"));
    m_itemNavigateSlizeX = m_menuNavigate->AppendCheckItem(wxID_ANY, wxT("Sagittal"));
    m_itemNavigateSlizeY = m_menuNavigate->AppendCheckItem(wxID_ANY, wxT("Axial"));
    m_itemNavigateSlizeZ = m_menuNavigate->AppendCheckItem(wxID_ANY, wxT("Coronal"));
    m_menuView->AppendSeparator();
    
    m_menuRotate = new wxMenu();
    m_menuView->AppendSubMenu(m_menuRotate, wxT("Rotate"));
    m_itemRotateX = m_menuRotate->AppendCheckItem(wxID_ANY, wxT("X axis"));
    m_itemRotateY = m_menuRotate->AppendCheckItem(wxID_ANY, wxT("Y axis"));
    m_itemRotateZ = m_menuRotate->AppendCheckItem(wxID_ANY, wxT("Z axis"));
    m_menuView->AppendSeparator();

    m_itemToggleShowCrosshair = m_menuView->AppendCheckItem(wxID_ANY, wxT("Show Crosshair"));
    m_itemToggleShowAxes = m_menuView->AppendCheckItem(wxID_ANY, wxT("Show Axes"));
    m_itemLockSlizes = m_menuView->AppendCheckItem(wxID_ANY, wxT("Lock Slices\tF"));
    m_itemLockScene = m_menuView->AppendCheckItem(wxID_ANY, wxT("Lock Scene\tG"));

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
	m_itemToggleUseGeometryShader = m_menuFibers->AppendCheckItem(wxID_ANY, wxT("Use Geometry Shader"));
 
    m_menuSurface = new wxMenu();
    m_itemNewSplineSurface = m_menuSurface->Append(wxID_ANY, wxT("New Spline Surface"));
    m_itemMoveBoundaryPointLeft = m_menuSurface->Append(wxID_ANY, wxT("Move Left"));
    m_itemMoveBoundaryPointRight = m_menuSurface->Append(wxID_ANY, wxT("Move Right"));
    
    m_menuOptions = new wxMenu();
    m_menuRuler = new wxMenu();
    m_itemToggleRuler = m_menuRuler->AppendCheckItem(wxID_ANY, wxT("Activate Ruler"));
    m_itemRulerClear = m_menuRuler->Append(wxID_ANY, wxT("Clear Points"));
    m_itemRulerAdd = m_menuRuler->Append(wxID_ANY, wxT("Add New Point"));
    m_itemRulerRemove = m_menuRuler->Append(wxID_ANY, wxT("Remove Last Point"));
    m_menuOptions->AppendSubMenu(m_menuRuler, wxT("Ruler"));

	m_menuDrawer = new wxMenu();
	m_itemToggleDrawer = m_menuDrawer->AppendCheckItem(wxID_ANY, wxT("Drawer Activated"));
	m_itemToggleDrawRound = m_menuDrawer->AppendCheckItem(wxID_ANY, wxT("Use Round Shape"));
	m_itemToggleDraw3d = m_menuDrawer->AppendCheckItem(wxID_ANY, wxT("Draw 3d"));
	m_itemDrawColorPicker = m_menuDrawer->Append(wxID_ANY, wxT("Color Picker"));
	m_menuStrokeSize = new wxMenu();
		m_itemDrawS1 = m_menuStrokeSize->AppendRadioItem(wxID_ANY, wxT("Size : 1"));
		m_itemDrawS2 = m_menuStrokeSize->AppendRadioItem(wxID_ANY, wxT("Size : 2"));
		m_itemDrawS3 = m_menuStrokeSize->AppendRadioItem(wxID_ANY, wxT("Size : 3"));
		m_itemDrawS4 = m_menuStrokeSize->AppendRadioItem(wxID_ANY, wxT("Size : 4"));
		m_itemDrawS5 = m_menuStrokeSize->AppendRadioItem(wxID_ANY, wxT("Size : 5"));
		m_itemDrawS7 = m_menuStrokeSize->AppendRadioItem(wxID_ANY, wxT("Size : 7"));
		m_itemDrawS10 = m_menuStrokeSize->AppendRadioItem(wxID_ANY, wxT("Size : 10"));
		m_itemDrawS2->Check();
		m_menuOptions->AppendSubMenu(m_menuStrokeSize, wxT("Stroke Size"));
	m_itemDrawPen = m_menuDrawer->AppendCheckItem(wxID_ANY, wxT("Use Pen"));
	m_itemDrawEraser = m_menuDrawer->AppendCheckItem(wxID_ANY, wxT("Use Eraser"));
	m_menuOptions->AppendSubMenu(m_menuDrawer,wxT("Drawer"));  

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
    m_itemScreenShot = m_menuHelp->Append(wxID_ANY, wxT("ScreenShot"));
	m_itemWarningsInfo = m_menuHelp->Append(wxID_ANY, wxT("Warnings Informations"));
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
    mf->Connect(m_itemLoad->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onLoad));
	mf->Connect(m_itemNewAnatomyByte->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onNewAnatomyByte));
	mf->Connect(m_itemNewAnatomyRGB->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onNewAnatomyRGB));
    //mf->Connect(m_itemReloadShader->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnReloadShaders));
    mf->Connect(m_itemSaveSCN->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSave));
    mf->Connect(m_itemSaveSelectedFibers->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSaveFibers));
    mf->Connect(m_itemSaveSelectedSurface->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSaveSurface));
    mf->Connect(m_itemSaveSelectedDataset->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSaveDataset));
    mf->Connect(m_itemQuit->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onQuit));    
    mf->Connect(m_itemToggleShowAxial->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleShowAxial));
    mf->Connect(m_itemToggleShowCoronal->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleShowCoronal));
    mf->Connect(m_itemToggleShowSagittal->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleShowSagittal));
    
    mf->Connect(m_itemNavigateSlizeX->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onNavigateSagital));
    mf->Connect(m_itemNavigateSlizeY->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onNavigateAxial));
    mf->Connect(m_itemNavigateSlizeZ->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onNavigateCoronal));
   
    mf->Connect(m_itemReset->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onMenuViewReset));
    mf->Connect(m_itemLeft->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onMenuViewLeft));
    mf->Connect(m_itemRight->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onMenuViewRight));
    mf->Connect(m_itemSuperior->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onMenuViewTop));
    mf->Connect(m_itemInferior->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onMenuViewBottom));
    mf->Connect(m_itemAnterior->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onMenuViewBack));
    mf->Connect(m_itemPosterior->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onMenuViewFront));
    mf->Connect(m_itemLockSlizes->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onMenuLock));
    mf->Connect(m_itemLockScene->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSceneLock));
    mf->Connect(m_itemToggleShowCrosshair->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onMenuViewCrosshair));
    mf->Connect(m_itemToggleShowAxes->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onMenuViewAxes));
    mf->Connect(m_itemNewSelectionBox->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onNewSelectionBox));
    mf->Connect(m_itemNewSelectionEllipsoid->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onNewSelectionEllipsoid));
    mf->Connect(m_itemToggleUseMorphing->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onUseMorph));
    mf->Connect(m_itemResetFibersColors->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onResetColor));
    mf->Connect(m_itemToggleLighting->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleLighting));    
    mf->Connect(m_itemToogleInvertFibersSelection->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onInvertFibers));
    mf->Connect(m_itemToggleUseFakeTubes->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onUseFakeTubes));
    mf->Connect(m_itemToggleClearToBlack->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onClearToBlack));
    mf->Connect(m_itemToggleBlendTextureOnMesh->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleBlendTexOnMesh));
    mf->Connect(m_itemToggleFilterISO->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleFilterIso));
    mf->Connect(m_itemToggleNormal->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleNormal));
    //mf->Connect(m_itemToggleColorMapLegend->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnToggleColorMapLegend));
    mf->Connect(m_itemToggleRuler->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectRuler));
    mf->Connect(m_itemRulerClear->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onRulerToolClear));
    mf->Connect(m_itemRulerAdd->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onRulerToolAdd));
    mf->Connect(m_itemRulerRemove->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onRulerToolDel));
    mf->Connect(m_itemNewSplineSurface->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onNewSplineSurface));
    mf->Connect(m_itemToggleDrawPoints->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleDrawPointsMode));
    mf->Connect(m_itemToggleDrawVectors->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleDrawVectors));
    mf->Connect(m_itemMoveBoundaryPointLeft->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onMoveBoundaryPointsLeft));
    mf->Connect(m_itemMoveBoundaryPointRight->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onMoveBoundaryPointsRight));
    mf->Connect(m_itemToggleUseTransparency->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onUseTransparency));
    mf->Connect(m_itemToggleUseGeometryShader->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onUseGeometryShader));
    mf->Connect(m_itemToggleDrawer->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSwitchDrawer));
    mf->Connect(m_itemToggleDrawRound->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleDrawRound));
    mf->Connect(m_itemToggleDraw3d->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleDraw3d));
    mf->Connect(m_itemDrawColorPicker->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectColorPicker));
    mf->Connect(m_itemDrawS1->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectStroke1));
    mf->Connect(m_itemDrawS2->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectStroke2));
    mf->Connect(m_itemDrawS3->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectStroke3));
    mf->Connect(m_itemDrawS4->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectStroke4));
    mf->Connect(m_itemDrawS5->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectStroke5));
    mf->Connect(m_itemDrawS7->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectStroke7));
    mf->Connect(m_itemDrawS10->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectStroke10));
    mf->Connect(m_itemDrawPen->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectPen));
    mf->Connect(m_itemDrawEraser->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectEraser));
    mf->Connect(m_itemGray->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSetCMapNo));
    mf->Connect(m_itemBlueGreenPurple->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSetCMap0));
    mf->Connect(m_itemRainbow->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSetCMap1));
    mf->Connect(m_itemHotIron->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSetCMap2));
    mf->Connect(m_itemRedYellow->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSetCMap3));
    mf->Connect(m_itemBlueLightblue->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSetCMap4));
    mf->Connect(m_itemAbout->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onAbout));
    mf->Connect(m_itemKeyboardShortcuts->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onShortcuts));
    mf->Connect(m_itemScreenShot->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onScreenshot));
	mf->Connect(m_itemWarningsInfo->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onWarningsInformations));
    //mf->Connect(m_itemAxialMovie->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSlizeMovieAxi));
    //mf->Connect(m_itemCoronalMovie->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSlizeMovieCor));
    //mf->Connect(m_itemSagittalMovie->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnSlizeMovieSag));
    mf->Connect(m_itemRotateZ->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onRotateZ));
    mf->Connect(m_itemRotateY->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onRotateY));
    mf->Connect(m_itemRotateX->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onRotateX));

}

void MenuBar::updateMenuBar( MainFrame *mf )
{
    //m_itemSaveSelectedDataset->Enable(mf->m_currentListItem != -1 && (((DatasetInfo*)mf->m_currentFNObject)->getType()==HEAD_BYTE || ((DatasetInfo*)mf->m_currentFNObject)->getType()==HEAD_SHORT));
    
    m_itemToggleLighting->Check(mf->m_pDatasetHelper->m_lighting);
    m_itemToggleRuler->Check(mf->m_pDatasetHelper->m_isRulerToolActive);
    
	bool isFiberSelected = false;
	bool isFiberUsingFakeTubes = false;
	bool isFiberUsingTransparency = false;
	bool isFiberInverted = false;
	if (mf->m_pCurrentSceneObject != NULL && mf->m_currentListItem != -1)
    {
		DatasetInfo* pDatasetInfo = ((DatasetInfo*)mf->m_pCurrentSceneObject);

        if( pDatasetInfo->getType() == FIBERS )
        {
            isFiberSelected = true;
			Fibers* pFibers = (Fibers*)pDatasetInfo;
			if( pFibers )
			{
				isFiberUsingFakeTubes = pFibers->isUsingFakeTubes();
				isFiberUsingTransparency = pFibers->isUsingTransparency();
				isFiberInverted = pFibers->isFibersInverted();
			}
		}
		else if( pDatasetInfo->getType() == FIBERSGROUP )
		{
			isFiberSelected = true;
			FibersGroup* pFibersGroup = (FibersGroup*)pDatasetInfo;
			if( pFibersGroup )
			{
				int useFakeTubesNb = 0;
				int useTransparencyNb = 0;
				int isInvertedNb = 0;
				for(int i = 0; i < pFibersGroup->getFibersCount(); i++)
				{
					Fibers* pFibers = pFibersGroup->getFibersSet(i);
					
					if( pFibers->isUsingFakeTubes())
						useFakeTubesNb++;
					if( pFibers->isUsingTransparency() )
						useTransparencyNb++;
					if( pFibers->isFibersInverted() )
						isInvertedNb++;
				}
				isFiberUsingFakeTubes = ( useFakeTubesNb == pFibersGroup->getFibersCount() );
				isFiberUsingTransparency = ( useTransparencyNb == pFibersGroup->getFibersCount() );
				isFiberInverted = ( isInvertedNb == pFibersGroup->getFibersCount() );
			}
		}
	}
	m_itemSaveSelectedFibers->Enable(isFiberSelected);
	m_itemResetFibersColors->Enable(isFiberSelected);
	m_itemToogleInvertFibersSelection->Enable(isFiberSelected);
	m_itemToogleInvertFibersSelection->Check(isFiberInverted);
	m_itemToggleUseTransparency->Enable(isFiberSelected);
    m_itemToggleUseTransparency->Check(isFiberUsingTransparency);
    m_itemToggleUseFakeTubes->Enable(isFiberSelected);
	m_itemToggleUseFakeTubes->Check(isFiberUsingFakeTubes);
    
	m_itemToggleUseMorphing->Check(mf->m_pDatasetHelper->m_morphing);
    m_itemToggleUseGeometryShader->Check(mf->m_pDatasetHelper->m_useFibersGeometryShader);
#if _COMPILE_GEO_SHADERS
    m_itemToggleUseGeometryShader->Enable(mf->m_pDatasetHelper->m_geometryShadersSupported);
#else
    m_itemToggleUseGeometryShader->Enable(false);
#endif
    m_itemToggleUseMorphing->Check(mf->m_pDatasetHelper->m_morphing);
    m_itemToggleShowCrosshair->Check(mf->m_pDatasetHelper->m_showCrosshair);
    m_itemToggleShowAxial->Check(mf->m_pDatasetHelper->m_showAxial);
    m_itemToggleShowCoronal->Check(mf->m_pDatasetHelper->m_showCoronal);
    m_itemToggleShowSagittal->Check(mf->m_pDatasetHelper->m_showSagittal);
    m_itemToggleClearToBlack->Check(mf->m_pDatasetHelper->m_clearToBlack);
    m_itemToggleBlendTextureOnMesh->Check(mf->m_pDatasetHelper->m_blendTexOnMesh);
    m_itemToggleFilterISO->Check(mf->m_pDatasetHelper->m_filterIsoSurf);
    m_itemToggleShowAxes->Check(mf->m_pDatasetHelper->m_isShowAxes);
    //m_itemToggleColorMapLegend->Check(mf->m_datasetHelper->m_colorMap);
    m_itemMoveBoundaryPointLeft->Enable(mf->m_pDatasetHelper->m_surfaceLoaded);
    m_itemMoveBoundaryPointRight->Enable(mf->m_pDatasetHelper->m_surfaceLoaded);
    m_itemToggleDrawPoints->Check(mf->m_pDatasetHelper->m_pointMode);
    m_itemToggleDrawVectors->Check(mf->m_pDatasetHelper->m_drawVectors);
	
    m_itemToggleDrawer->Check(mf->m_pDatasetHelper->m_isDrawerToolActive);
	m_itemToggleDrawRound->Check(mf->m_pDatasetHelper->m_drawRound);
	m_itemToggleDraw3d->Check(mf->m_pDatasetHelper->m_draw3d);
	m_itemDrawPen->Check(mf->m_pDatasetHelper->m_drawMode == mf->m_pDatasetHelper->DRAWMODE_PEN);
	m_itemDrawEraser->Check(mf->m_pDatasetHelper->m_drawMode == mf->m_pDatasetHelper->DRAWMODE_ERASER);
	
    m_itemToggleDrawRound->Enable( mf->m_pDatasetHelper->m_isDrawerToolActive );
    m_itemToggleDraw3d->Enable(    mf->m_pDatasetHelper->m_isDrawerToolActive );
    m_itemDrawPen->Enable(         mf->m_pDatasetHelper->m_isDrawerToolActive );
    m_itemDrawEraser->Enable(      mf->m_pDatasetHelper->m_isDrawerToolActive );
    m_itemDrawColorPicker->Enable( mf->m_pDatasetHelper->m_isDrawerToolActive &&
                                   mf->m_pDatasetHelper->m_canUseColorPicker &&
                                   mf->m_pDatasetHelper->m_drawMode == mf->m_pDatasetHelper->DRAWMODE_PEN );
    
    m_itemNewSplineSurface->Enable(mf->m_pDatasetHelper->m_anatomyLoaded && !mf->m_pDatasetHelper->m_surfaceLoaded);
}
