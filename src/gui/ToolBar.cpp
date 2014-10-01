#include "ToolBar.h"

#include "MainFrame.h"
#include "SceneManager.h"
#include "../main.h"
#include "../dataset/Fibers.h"

ToolBar::ToolBar(wxToolBar *pToolBar)
: m_txtRuler( NULL ),
  m_drawColorIcon(16, 16, true),
  m_pToolBar(pToolBar)
{
    wxImage bmpOpen(MyApp::iconsPath+ wxT("fileopen.png" ), wxBITMAP_TYPE_PNG);
    
    m_btnOpen = m_pToolBar->AddTool(wxID_ANY, wxT("Open"), bmpOpen );
    m_pToolBar->AddSeparator();
     
    wxImage bmpAxial(MyApp::iconsPath+ wxT("axial.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpCor(MyApp::iconsPath+ wxT("cor.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpSag(MyApp::iconsPath+ wxT("sag.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpAlphaBlend(MyApp::iconsPath+ wxT("alphablend.png"), wxBITMAP_TYPE_PNG);

    m_toggleShowAxial = m_pToolBar->AddCheckTool( wxID_ANY, wxT("Show Axial"), bmpAxial, wxNullBitmap, wxT("Show Axial"));
    m_toggleShowCoronal = m_pToolBar->AddCheckTool( wxID_ANY, wxT( "Show Coronal" ), bmpCor, wxNullBitmap, wxT("Show Coronal"));
    m_toggleShowSagittal = m_pToolBar->AddCheckTool( wxID_ANY, wxT("Show Sagittal"), bmpSag, wxNullBitmap, wxT("Show Sagittal"));
    m_toggleAlphaBlending = m_pToolBar->AddCheckTool( wxID_ANY, wxT("Toggle Alpha Blending"), bmpAlphaBlend, wxNullBitmap, wxT("Toggle Alpha Blending"));
    m_pToolBar->AddSeparator();

    wxImage bmpBox(MyApp::iconsPath+ wxT("box.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpBoxEyeAll(MyApp::iconsPath+ wxT("box_eye_all.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpBoxOffAll(MyApp::iconsPath+ wxT("box_off_all.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpBoxNotAll(MyApp::iconsPath+ wxT("box_not_all.png"), wxBITMAP_TYPE_PNG);

    wxImage bmpEllipsoid(MyApp::iconsPath + wxT("ellipsoid.png"), wxBITMAP_TYPE_PNG);

    // TODO all the addtools can have some captions.
    m_btnNewSelectionBox = m_pToolBar->AddTool( wxID_ANY, wxT("New Selection Box"), bmpBox);
    m_btnNewSelectionEllipsoid = m_pToolBar->AddTool( wxID_ANY, wxT("New Selection Ellipsoid"), bmpEllipsoid );

#if !_USE_LIGHT_GUI
    m_toggleShowAllSelectionObjects = m_pToolBar->AddCheckTool( wxID_ANY, wxT( "Toggle Show All Selection Object" ), bmpBoxEyeAll, wxNullBitmap, wxT("Toggle Show All Selection Objects"));
    m_toggleActivateAllSelectionObjects = m_pToolBar->AddCheckTool( wxID_ANY, wxT( "Toggle Activate All Selection Objects" ), bmpBoxOffAll, wxNullBitmap, wxT("Toggle Activate All Selection Objects"));
    m_toggleInverseSelection = m_pToolBar->AddCheckTool( wxID_ANY, wxT( "Toggle Inverse Fibers Selection" ), bmpBoxNotAll, wxNullBitmap, wxT("Toggle Inverse Fibers Selection"));
#endif

    m_pToolBar->AddSeparator();

    wxImage bmpGridSpline(MyApp::iconsPath+ wxT("grid_spline.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpGrid(MyApp::iconsPath+ wxT("grid.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpView1(MyApp::iconsPath+ wxT("view1.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpView3(MyApp::iconsPath+ wxT("view3.png"), wxBITMAP_TYPE_PNG);

    wxImage bmpLighting(MyApp::iconsPath+ wxT("lightbulb.png"), wxBITMAP_TYPE_PNG);

#if !_USE_LIGHT_GUI
    m_toggleLighting = m_pToolBar->AddCheckTool( wxID_ANY, wxT( "Toggle Lighting" ), bmpLighting, wxNullBitmap, wxT("Toggle Lighting"));

    m_pToolBar->AddSeparator();

    wxImage bmpTubes (MyApp::iconsPath+ wxT("tubes.png"), wxBITMAP_TYPE_PNG);

    m_toggleFakeTubes = m_pToolBar->AddCheckTool( wxID_ANY, wxT("Toggle Tubes"), bmpTubes, wxNullBitmap, wxT("Toggle Tubes"));
    m_pToolBar->AddSeparator();
#endif

    wxImage bmpClearColor (MyApp::iconsPath+ wxT("background_color.png"), wxBITMAP_TYPE_PNG);

    m_toggleClearToBlack = m_pToolBar->AddCheckTool( wxID_ANY, wxT("Clear To Black"), bmpClearColor, wxNullBitmap, wxT("Clear To Black"));
    m_pToolBar->AddSeparator();

    wxImage bmpPointer (MyApp::iconsPath+ wxT("pointer.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpRuler (MyApp::iconsPath+ wxT("rulertool.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpDrawer (MyApp::iconsPath+ wxT("drawertool.png"), wxBITMAP_TYPE_PNG);

    m_selectNormalPointer = m_pToolBar->AddRadioTool( wxID_ANY, wxT("Pointer" ), bmpPointer, wxNullBitmap, wxT("Pointer"));

#if !_USE_LIGHT_GUI
    m_selectRuler = m_pToolBar->AddRadioTool( wxID_ANY, wxT("Ruler" ), bmpRuler, wxNullBitmap, wxT("Ruler"));
#endif

    m_selectDrawer = m_pToolBar->AddRadioTool( wxID_ANY, wxT("Drawer" ), bmpDrawer, wxNullBitmap, wxT("Drawer"));
    m_pToolBar->AddSeparator();

#if !_USE_LIGHT_GUI
    m_txtRuler = new wxTextCtrl(m_pToolBar, wxID_ANY,wxT("0.00mm (0.00mm)"), wxDefaultPosition, wxSize( 160, 24 ), wxTE_LEFT | wxTE_READONLY);
    m_txtRuler->SetForegroundColour(wxColour(wxT("#222222")));
    m_txtRuler->SetBackgroundColour(*wxWHITE);
    wxFont font = m_txtRuler->GetFont();
    font.SetPointSize(10);
    font.SetWeight(wxBOLD);
    m_txtRuler->SetFont(font);
    m_pToolBar->AddControl(m_txtRuler);
#endif

    //no wxImage, it will show the selected color

    m_selectColorPicker = m_pToolBar->AddTool(wxID_ANY, wxT("Color Picker" ), bmpClearColor, wxNullBitmap, wxITEM_NORMAL, wxT("Color Picker"));
    m_pToolBar->EnableTool(m_selectColorPicker->GetId(), false);

    wxImage bmpRound (MyApp::iconsPath+ wxT("draw_round.png"), wxBITMAP_TYPE_PNG);
    wxImage bmp3d (MyApp::iconsPath+ wxT("draw3D.png"), wxBITMAP_TYPE_PNG);

    m_toggleDrawRound = m_pToolBar->AddCheckTool( wxID_ANY, wxT( "Round Shape" ), bmpRound, wxNullBitmap, wxT("Round Shape"));
    m_pToolBar->EnableTool(m_toggleDrawRound->GetId(), false);
    m_toggleDraw3d = m_pToolBar->AddCheckTool( wxID_ANY, wxT( "Draw in 3d" ), bmp3d, wxNullBitmap, wxT("Draw in 3d"));
    m_pToolBar->EnableTool(m_toggleDraw3d->GetId(), false);
    m_pToolBar->AddSeparator();

    wxImage bmpPen (MyApp::iconsPath+ wxT("draw_pen.png"), wxBITMAP_TYPE_PNG);
    wxImage bmpEraser (MyApp::iconsPath+ wxT("draw_eraser.png"), wxBITMAP_TYPE_PNG);

    m_selectPen = m_pToolBar->AddRadioTool( wxID_ANY, wxT("Use Pen" ), bmpPen, wxNullBitmap, wxT("Use Pen"));
    m_pToolBar->EnableTool(m_selectPen->GetId(), false);
    m_selectEraser = m_pToolBar->AddRadioTool( wxID_ANY, wxT("Use Eraser" ), bmpEraser, wxNullBitmap, wxT("Use Eraser"));
    m_pToolBar->EnableTool(m_selectEraser->GetId(), false);
}

void ToolBar::connectToolsEvents( MainFrame *mf )
{
    mf->Connect(m_btnOpen->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onLoad));
    mf->Connect(m_toggleShowAxial->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleShowAxial));
    mf->Connect(m_toggleShowCoronal->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleShowCoronal));
    mf->Connect(m_toggleShowSagittal->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleShowSagittal));
    mf->Connect(m_toggleAlphaBlending->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleAlpha));
    mf->Connect(m_btnNewSelectionBox->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onNewSelectionBox));
    mf->Connect(m_btnNewSelectionEllipsoid->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onNewSelectionEllipsoid));
    
#if !_USE_LIGHT_GUI
    mf->Connect(m_toggleShowAllSelectionObjects->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onHideSelectionObjects));
    mf->Connect(m_toggleInverseSelection->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onInvertFibers));
    mf->Connect(m_toggleActivateAllSelectionObjects->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onActivateSelectionObjects));    
    mf->Connect(m_toggleLighting->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleLighting));    
    mf->Connect(m_toggleFakeTubes->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onUseFakeTubes));
#endif
    
    mf->Connect(m_toggleClearToBlack->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onClearToBlack));
    mf->Connect(m_selectNormalPointer->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectNormalPointer));

#if !_USE_LIGHT_GUI
    mf->Connect(m_selectRuler->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectRuler)); 
#endif
    
    mf->Connect(m_selectDrawer->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectDrawer)); 
    //mf->Connect(m_selectDrawRound->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleDrawRound)); 
    //mf->Connect(m_selectDraw3d->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleDraw3d)); 
    mf->Connect(m_selectColorPicker->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectColorPicker)); 
    mf->Connect(m_toggleDrawRound->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleDrawRound)); 
    mf->Connect(m_toggleDraw3d->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onToggleDraw3d)); 
    mf->Connect(m_selectPen->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectPen)); 
    mf->Connect(m_selectEraser->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainFrame::onSelectEraser)); 

    setColorPickerColor( mf->getDrawColor() );
}

void ToolBar::updateToolBar( MainFrame *mf )
{
    m_pToolBar->ToggleTool( m_toggleShowAxial->GetId(),     SceneManager::getInstance()->isAxialDisplayed() );
    m_pToolBar->ToggleTool( m_toggleShowCoronal->GetId(),   SceneManager::getInstance()->isCoronalDisplayed() );
    m_pToolBar->ToggleTool( m_toggleShowSagittal->GetId(),  SceneManager::getInstance()->isSagittalDisplayed() );
    m_pToolBar->ToggleTool( m_toggleAlphaBlending->GetId(), SceneManager::getInstance()->isAlphaBlend() );

#if !_USE_LIGHT_GUI
    m_pToolBar->ToggleTool( m_toggleLighting->GetId(),      SceneManager::getInstance()->isLightingActive() );
    m_pToolBar->ToggleTool( m_toggleShowAllSelectionObjects->GetId(), SceneManager::getInstance()->getShowAllSelObj() );
    m_pToolBar->ToggleTool( m_toggleActivateAllSelectionObjects->GetId(), !SceneManager::getInstance()->getActivateAllSelObj() );
#endif

    bool isFiberSelected = false;
    bool isFiberUsingFakeTubes = false;
    bool isFiberInverted = false;
    if (mf->m_pCurrentSceneObject != NULL && mf->m_currentListIndex != -1)
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
    
#if !_USE_LIGHT_GUI
    m_pToolBar->ToggleTool( m_toggleFakeTubes->GetId(), isFiberSelected && isFiberUsingFakeTubes);
    
    m_pToolBar->ToggleTool( m_toggleInverseSelection->GetId(), isFiberSelected && isFiberInverted);
#endif
    
    m_pToolBar->ToggleTool( m_toggleClearToBlack->GetId(), SceneManager::getInstance()->getClearToBlack() );
    m_pToolBar->ToggleTool( m_selectNormalPointer->GetId(), SceneManager::getInstance()->isRulerActive() && mf->isDrawerToolActive() );

#if !_USE_LIGHT_GUI
    m_pToolBar->ToggleTool( m_selectRuler->GetId(), SceneManager::getInstance()->isRulerActive() );
#endif
    
    m_pToolBar->ToggleTool( m_selectDrawer->GetId(), mf->isDrawerToolActive() );
    m_pToolBar->ToggleTool( m_toggleDrawRound->GetId(), mf->canDrawRound() );
    m_pToolBar->ToggleTool( m_toggleDraw3d->GetId(), mf->canDraw3D() );
    m_pToolBar->ToggleTool( m_selectPen->GetId(), DRAWMODE_PEN == mf->getDrawMode() );
    m_pToolBar->ToggleTool( m_selectEraser->GetId(), DRAWMODE_ERASER == mf->getDrawMode() );
    
    // Check if the currently selected anatomy can use the Color Picker.
    m_pToolBar->EnableTool( m_selectColorPicker->GetId(),
                            mf->isDrawerToolActive() && DRAWMODE_PEN == mf->getDrawMode()
                            && mf->canUseColorPicker() );
}

void ToolBar::updateDrawerToolBar( const bool drawingActive )
{
    // Need to check to avoid crash when using the light weight version.
    if( m_txtRuler != NULL )
    {
        m_txtRuler->Disable();
    }
     
     m_pToolBar->EnableTool( m_toggleDrawRound->GetId(), drawingActive );
     m_pToolBar->EnableTool( m_toggleDraw3d->GetId(), drawingActive );
     m_pToolBar->EnableTool( m_selectPen->GetId(), drawingActive );
     m_pToolBar->EnableTool( m_selectEraser->GetId(), drawingActive );
     m_pToolBar->EnableTool( m_selectColorPicker->GetId(), drawingActive );
}

void ToolBar::setColorPickerColor( const wxColour &color )
{
    wxRect fullImage(0, 0, 16, 16); //this is valid as long as toolbar items use 16x16 icons
    m_drawColorIcon.SetRGB( fullImage,
                            color.Red(),
                            color.Green(),
                            color.Blue() );
    m_pToolBar->SetToolNormalBitmap( m_selectColorPicker->GetId(), wxBitmap( m_drawColorIcon ) );
}
