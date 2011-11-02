/////////////////////////////////////////////////////////////////////////////
// Name:            toolBar.cpp
// Author:          GGirard
// Creation Date:   30/11/2010
//
// Description: MainFrame ToolBar
//
// Last modifications:
//      
/////////////////////////////////////////////////////////////////////////////

#include "ToolBar.h"
#include "MainFrame.h"
#include "../main.h"
#include "../dataset/Fibers.h"

ToolBar::ToolBar(wxWindow *parent)
    :wxToolBar( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxNO_BORDER )    
{    
    wxImage bmpOpen(MyApp::iconsPath+ wxT("fileopen.png" ), wxBITMAP_TYPE_PNG);

    m_btnOpen = this->AddTool(wxID_ANY, bmpOpen, wxT("Open"));    
    this->AddSeparator();


    wxImage bmpAxial(MyApp::iconsPath+ wxT("axial.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpCor(MyApp::iconsPath+ wxT("cor.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpSag(MyApp::iconsPath+ wxT("sag.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpAlphaBlend(MyApp::iconsPath+ wxT("alphablend.png"), wxBITMAP_TYPE_PNG);
    
    m_toggleShowAxial = this->AddCheckTool( wxID_ANY, wxT("Show Axial"), bmpAxial, wxNullBitmap, wxT("Show Axial"));
    m_toggleShowCoronal = this->AddCheckTool( wxID_ANY, wxT( "Show Coronal" ), bmpCor, wxNullBitmap, wxT("Show Coronal"));
    m_toggleShowSagittal = this->AddCheckTool( wxID_ANY, wxT("Show Sagittal"), bmpSag, wxNullBitmap, wxT("Show Sagittal"));
    m_toggleAlphaBlending = this->AddCheckTool( wxID_ANY, wxT("Toggle Alpha Blending"), bmpAlphaBlend, wxNullBitmap, wxT("Toggle Alpha Blending"));
    this->AddSeparator();


    wxImage bmpBox(MyApp::iconsPath+ wxT("box.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpBoxEyeAll(MyApp::iconsPath+ wxT("box_eye_all.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpBoxOffAll(MyApp::iconsPath+ wxT("box_off_all.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpBoxNotAll(MyApp::iconsPath+ wxT("box_not_all.png"), wxBITMAP_TYPE_PNG);

    m_btnNewSelectionBox = this->AddTool( wxID_ANY, bmpBox, wxT("New Selection Box"));
    m_toggleShowAllSelectionObjects = this->AddCheckTool( wxID_ANY, wxT( "Toggle Show All Selection Object" ), bmpBoxEyeAll, wxNullBitmap, wxT("Toggle Show All Selection Objects"));
    m_toggleActivateAllSelectionObjects = this->AddCheckTool( wxID_ANY, wxT( "Toggle Activate All Selection Objects" ), bmpBoxOffAll, wxNullBitmap, wxT("Toggle Activate All Selection Objects"));
    m_toggleInverseSelection = this->AddCheckTool( wxID_ANY, wxT( "Toggle Inverse Fibers Selection" ), bmpBoxNotAll, wxNullBitmap, wxT("Toggle Inverse Fibers Selection"));
    this->AddSeparator();


    wxImage bmpGridSpline(MyApp::iconsPath+ wxT("grid_spline.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpGrid(MyApp::iconsPath+ wxT("grid.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpView1(MyApp::iconsPath+ wxT("view1.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpView3(MyApp::iconsPath+ wxT("view3.png"), wxBITMAP_TYPE_PNG);
    
    //m_toggleDrawPoints = this->AddCheckTool( wxID_ANY, wxT("Toggle drawing of points"), bmpGrid, wxNullBitmap, wxT("Toggle drawing of points"));
    //m_btnNewSplineSurface = this->AddTool( wxID_ANY, bmpGridSpline, wxT("New Spline Surface"));   
    //m_btnMoveBoundaryPointLeft = this->AddTool( wxID_ANY, bmpView1, wxT("Move Boundary Points of Spline Durface Left"));
    //m_btnMoveBoundaryPointRight = this->AddTool( wxID_ANY, bmpView3, wxT("Move Boundary Points of Spline Surface Right"));
    //this->AddSeparator();

    wxImage bmpLighting(MyApp::iconsPath+ wxT("lightbulb.png"), wxBITMAP_TYPE_PNG);
    
    m_toggleLighting = this->AddCheckTool( wxID_ANY, wxT( "Toggle Lighting" ), bmpLighting, wxNullBitmap, wxT("Toggle Lighting"));
    this->AddSeparator();

    wxImage bmpTubes (MyApp::iconsPath+ wxT("tubes.png"), wxBITMAP_TYPE_PNG);
    
    m_toggleFakeTubes = this->AddCheckTool( wxID_ANY, wxT("Toggle Tubes"), bmpTubes, wxNullBitmap, wxT("Toggle Tubes"));
    this->AddSeparator();

    
    wxImage bmpClearColor (MyApp::iconsPath+ wxT("background_color.png"), wxBITMAP_TYPE_PNG);
    
    m_toggleClearToBlack = this->AddCheckTool( wxID_ANY, wxT("Clear To Black"), bmpClearColor, wxNullBitmap, wxT("Clear To Black"));
    this->AddSeparator();


    wxImage bmpRuler (MyApp::iconsPath+ wxT("rulertool.png"), wxBITMAP_TYPE_PNG);
    
    m_toggleRuler = this->AddCheckTool( wxID_ANY, wxT("Toggle Ruler" ), bmpRuler, wxNullBitmap, wxT("Toggle Ruler"));
    m_txtRuler = new wxTextCtrl(this, wxID_ANY,wxT("0.00mm (0.00mm)"),wxDefaultPosition, wxSize( 160, 24 ), wxTE_LEFT | wxTE_READONLY);
    m_txtRuler->Disable();
    m_txtRuler->SetForegroundColour(wxColour(wxT("#222222")));
    m_txtRuler->SetBackgroundColour(*wxWHITE);
    wxFont font = m_txtRuler->GetFont();
    font.SetPointSize(10);
    font.SetWeight(wxBOLD);
    m_txtRuler->SetFont(font);
    this->AddControl(m_txtRuler);
}

void ToolBar::initToolBar( MainFrame *mf )
{
    mf->Connect(m_btnOpen->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onLoad));
    mf->Connect(m_toggleShowAxial->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleShowAxial));
    mf->Connect(m_toggleShowCoronal->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleShowCoronal));
    mf->Connect(m_toggleShowSagittal->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleShowSagittal));
    mf->Connect(m_toggleAlphaBlending->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleAlpha));
    mf->Connect(m_btnNewSelectionBox->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onNewSelectionBox));
    mf->Connect(m_toggleShowAllSelectionObjects->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onHideSelectionObjects));
    mf->Connect(m_toggleInverseSelection->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onInvertFibers));
    mf->Connect(m_toggleActivateAllSelectionObjects->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onActivateSelectionObjects));
    //mf->Connect(m_btnNewSplineSurface->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnNewSplineSurface));
    //mf->Connect(m_toggleDrawPoints->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnToggleDrawPointsMode));
    //mf->Connect(m_btnMoveBoundaryPointLeft->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnMoveBoundaryPointsLeft));
    //mf->Connect(m_btnMoveBoundaryPointRight->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::OnMoveBoundaryPointsRight));
    mf->Connect(m_toggleLighting->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleLighting));
    mf->Connect(m_toggleFakeTubes->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onUseFakeTubes));
    mf->Connect(m_toggleClearToBlack->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onClearToBlack));
    mf->Connect(m_toggleRuler->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onRulerTool)); 
}

void ToolBar::updateToolBar( MainFrame *mf )
{
    //EnableTool(m_btnNewSplineSurface->GetId(),mf->m_datasetHelper->m_anatomyLoaded && !mf->m_datasetHelper->m_surfaceLoaded);
    //EnableTool(m_btnMoveBoundaryPointLeft->GetId(),mf->m_datasetHelper->m_surfaceLoaded);
    //EnableTool(m_btnMoveBoundaryPointRight->GetId(),mf->m_datasetHelper->m_surfaceLoaded);
    ToggleTool(m_toggleShowAxial->GetId(), mf->m_pDatasetHelper->m_showAxial);
    ToggleTool(m_toggleShowCoronal->GetId(), mf->m_pDatasetHelper->m_showCoronal);
    ToggleTool(m_toggleShowSagittal->GetId(), mf->m_pDatasetHelper->m_showSagittal);
    ToggleTool(m_toggleAlphaBlending->GetId(), mf->m_pDatasetHelper->m_blendAlpha);
    ToggleTool(m_toggleLighting->GetId(), mf->m_pDatasetHelper->m_lighting);
    ToggleTool(m_toggleShowAllSelectionObjects->GetId(), mf->m_pDatasetHelper->m_showObjects);
    ToggleTool(m_toggleActivateAllSelectionObjects->GetId(), !mf->m_pDatasetHelper->m_activateObjects);
    //ToggleTool(m_toggleDrawPoints->GetId(), mf->m_datasetHelper->m_pointMode);
    ToggleTool(m_toggleRuler->GetId(), mf->m_pDatasetHelper->m_isRulerToolActive);
    ToggleTool(m_toggleClearToBlack->GetId(), mf->m_pDatasetHelper->m_clearToBlack);
    //EnableTool(m_btnNewSelectionBox->GetId(), mf->m_datasetHelper->m_fibersLoaded);

	bool isFiberSelected = false;
	bool isFiberUsingFakeTubes = false;
	bool isFiberInverted = false;
	if (mf->m_pCurrentSceneObject != NULL && mf->m_currentListItem != -1)
    {
		DatasetInfo* pDatasetInfo = ((DatasetInfo*)mf->m_pCurrentSceneObject);

        if( pDatasetInfo->getType() == FIBERS || pDatasetInfo->getType() == FIBERSGROUP )
        {
            isFiberSelected = true;
			Fibers* pFibers = (Fibers*)pDatasetInfo;
			if( pFibers )
			{
				isFiberUsingFakeTubes = pFibers->isUsingFakeTubes();
				isFiberInverted = pFibers->isFibersInverted();
			}
		}
	}
	ToggleTool(m_toggleFakeTubes->GetId(), isFiberSelected && isFiberUsingFakeTubes);
	ToggleTool(m_toggleInverseSelection->GetId(), isFiberSelected && isFiberInverted);
}
