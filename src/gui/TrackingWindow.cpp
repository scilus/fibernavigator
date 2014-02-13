#include "TrackingWindow.h"

#include "MainFrame.h"
#include "SceneManager.h"
#include "SelectionBox.h"
#include "SelectionEllipsoid.h"
#include "../main.h"
#include "../dataset/Anatomy.h"
#include "../dataset/Fibers.h"
#include "../dataset/ODFs.h"
#include "../dataset/RTTrackingHelper.h"
#include "../dataset/Tensors.h"
#include "../dataset/Maximas.h"
#include "../misc/IsoSurface/CIsoSurface.h"
#include "../misc/IsoSurface/TriangleMesh.h"

#include <wx/checkbox.h>
#include <wx/grid.h>
#include <wx/tglbtn.h>
#include <wx/treectrl.h>


IMPLEMENT_DYNAMIC_CLASS( TrackingWindow, wxScrolledWindow )

BEGIN_EVENT_TABLE( TrackingWindow, wxScrolledWindow )
END_EVENT_TABLE()


TrackingWindow::TrackingWindow( wxWindow *pParent, MainFrame *pMf, wxWindowID id, const wxPoint &pos, const wxSize &size )
:   wxScrolledWindow( pParent, id, pos, size, wxBORDER_NONE, _T("DTI RTT Canvas") ),
    m_pMainFrame( pMf )
{
    SetBackgroundColour( *wxLIGHT_GREY );
    m_pTrackingSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( m_pTrackingSizer );
    SetAutoLayout( true );

    //Content of RTT panel
    /********************************/

    m_pBtnSelectFile = new wxButton( this, wxID_ANY,wxT("DTI not selected"), wxPoint(30,0), wxSize(100, -1) );
    Connect( m_pBtnSelectFile->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnSelectFileDTI) );
    m_pBtnSelectFile->SetBackgroundColour(wxColour( 255, 147, 147 ));

    m_pBtnStart = new wxToggleButton( this, wxID_ANY,wxT("Start tracking"), wxPoint(130,0), wxSize(100, -1) );
    Connect( m_pBtnStart->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnStartTracking) );
    m_pBtnStart->Enable(false);

	m_pBtnSelectShell = new wxButton( this, wxID_ANY,wxT("Shell not selected"), wxPoint(30,30), wxSize(100, -1) );
    Connect( m_pBtnSelectShell->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnSelectShell) );

    m_pToggleShell = new wxToggleButton( this, wxID_ANY,wxT("Shell seed OFF"), wxPoint(130,30), wxSize(100, -1) );
    Connect( m_pToggleShell->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnShellSeeding) );
	m_pToggleShell->Enable(false);

    m_pTextFA = new wxStaticText( this, wxID_ANY, wxT("Min FA"), wxPoint(0,60), wxSize(60, -1), wxALIGN_CENTER );
    m_pSliderFA = new MySlider( this, wxID_ANY, 0, 1, 50, wxPoint(60,60), wxSize(130, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderFA->SetValue( 10 );
    Connect( m_pSliderFA->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderFAMoved) );
    m_pTxtFABox = new wxTextCtrl( this, wxID_ANY, wxT("0.10"), wxPoint(190,60), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );
    
    m_pTextAngle = new wxStaticText( this, wxID_ANY, wxT("Max angle"), wxPoint(0,90), wxSize(60, -1), wxALIGN_CENTER );
    m_pSliderAngle = new MySlider( this, wxID_ANY, 0, 1, 90, wxPoint(60,90), wxSize(130, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderAngle->SetValue( 60 );
    Connect( m_pSliderAngle->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderAngleMoved) );
    m_pTxtAngleBox = new wxTextCtrl( this, wxID_ANY, wxT("60.0 "), wxPoint(190,90), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

    m_pTextStep = new wxStaticText( this, wxID_ANY, wxT("Step"), wxPoint(0,120), wxSize(60, -1), wxALIGN_CENTER );
    m_pSliderStep = new MySlider( this, wxID_ANY, 0, 5, 20, wxPoint(60,120), wxSize(130, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderStep->SetValue( 10 );
    Connect( m_pSliderStep->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderStepMoved) );
    m_pTxtStepBox = new wxTextCtrl( this, wxID_ANY, wxT("1.0 mm"), wxPoint(190,120), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

    m_pTextPuncture = new wxStaticText( this, wxID_ANY, wxT("Vin-Vout"), wxPoint(0,150), wxSize(60, -1), wxALIGN_CENTER );
    m_pSliderPuncture = new MySlider( this, wxID_ANY, 0, 0, 10, wxPoint(60,150), wxSize(130, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderPuncture->SetValue( 2 );
    Connect( m_pSliderPuncture->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderPunctureMoved) );
    m_pTxtPunctureBox = new wxTextCtrl( this, wxID_ANY, wxT("0.2"), wxPoint(190,150), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

    m_pTextMinLength = new wxStaticText( this, wxID_ANY, wxT("Min length"), wxPoint(0,180), wxSize(60, -1), wxALIGN_CENTER );
    m_pSliderMinLength = new MySlider( this, wxID_ANY, 0, 0, 400, wxPoint(60,180), wxSize(130, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderMinLength->SetValue( 10 );
    Connect( m_pSliderMinLength->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderMinLengthMoved) );
    m_pTxtMinLengthBox = new wxTextCtrl( this, wxID_ANY, wxT("10 mm"), wxPoint(190,180), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

    m_pTextMaxLength = new wxStaticText( this, wxID_ANY, wxT("Max length"), wxPoint(0,210), wxSize(60, -1), wxALIGN_CENTER );
    m_pSliderMaxLength = new MySlider( this, wxID_ANY, 0, 0, 300, wxPoint(60,210), wxSize(130, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderMaxLength->SetValue( 200 );
    Connect( m_pSliderMaxLength->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderMaxLengthMoved) );
    m_pTxtMaxLengthBox = new wxTextCtrl( this, wxID_ANY, wxT("200 mm"), wxPoint(190,210), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

    m_pTextAxisSeedNb = new wxStaticText( this, wxID_ANY, wxT("Seed/axis"), wxPoint(0,240), wxSize(60, -1), wxALIGN_CENTER );
    m_pSliderAxisSeedNb = new MySlider( this, wxID_ANY, 0, 1, 15, wxPoint(60,240), wxSize(130, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderAxisSeedNb->SetValue( 10 );
    Connect( m_pSliderAxisSeedNb->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderAxisSeedNbMoved) );
    m_pTxtAxisSeedNbBox = new wxTextCtrl( this, wxID_ANY, wxT("10"), wxPoint(190,240), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

    m_pTextTotalSeedNb = new wxStaticText( this, wxID_ANY, wxT("Number of current seeds"), wxPoint(7,270), wxSize(150, -1), wxALIGN_LEFT );
    m_pTxtTotalSeedNbBox = new wxTextCtrl( this, wxID_ANY, wxT("1000"), wxPoint(190,270), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

	m_pBtnConvert = new wxButton( this, wxID_ANY,wxT("Convert Fibers"), wxPoint(50,300), wxSize(140, 30) );
	Connect( m_pBtnConvert->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnConvertToFibers) );

}

TrackingWindow::TrackingWindow( wxWindow *pParent, MainFrame *pMf, wxWindowID id, const wxPoint &pos, const wxSize &size, int hardi)
:   wxScrolledWindow( pParent, id, pos, size, wxBORDER_NONE, _T("HARDI RTT Canvas") ),
    m_pMainFrame( pMf )
{
    SetBackgroundColour( *wxLIGHT_GREY );
    m_pTrackingSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( m_pTrackingSizer );
    SetAutoLayout( true );

    //Content of RTT panel
    /********************************/

    m_pBtnSelectFile = new wxButton( this, wxID_ANY,wxT("Peaks not selected"), wxPoint(30,0), wxSize(115, -1) );
    Connect( m_pBtnSelectFile->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnSelectFileHARDI) );
    m_pBtnSelectFile->SetBackgroundColour(wxColour( 255, 147, 147 ));

    m_pBtnStart = new wxToggleButton( this, wxID_ANY,wxT("Start tracking"), wxPoint(130,0), wxSize(115, -1) );
    Connect( m_pBtnStart->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnStartTracking) );
    m_pBtnStart->Enable(false);

	wxBoxSizer *pBoxRow1 = new wxBoxSizer( wxHORIZONTAL );
	pBoxRow1->Add( m_pBtnSelectFile, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 1 );
    pBoxRow1->Add( m_pBtnStart,   0, wxALIGN_CENTER_HORIZONTAL | wxALL, 1);
	m_pTrackingSizer->Add( pBoxRow1, 0, wxFIXED_MINSIZE | wxALL, 2 );

	m_pBtnSelectShell = new wxButton( this, wxID_ANY,wxT("Shell not selected"), wxPoint(30,30), wxSize(115, -1) );
    Connect( m_pBtnSelectShell->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnSelectShell) );

    m_pToggleShell = new wxToggleButton( this, wxID_ANY,wxT("Shell seed OFF"), wxPoint(130,30), wxSize(115, -1) );
    Connect( m_pToggleShell->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnShellSeeding) );
	m_pToggleShell->Enable(false);

	wxBoxSizer *pBoxRow2 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow2->Add( m_pBtnSelectShell, 0, wxALIGN_CENTER | wxALL, 1 );
    pBoxRow2->Add( m_pToggleShell,   0, wxALIGN_CENTER | wxALL, 1);
	m_pTrackingSizer->Add( pBoxRow2, 0, wxFIXED_MINSIZE | wxALL, 2 );

	m_pBtnSelectSeed = new wxButton( this, wxID_ANY,wxT("Seed not selected"), wxPoint(30,60), wxSize(115, -1) );
    Connect( m_pBtnSelectSeed->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnSelectSeedMap) );

    m_pToggleSeedMap = new wxToggleButton( this, wxID_ANY,wxT("Seed Map OFF"), wxPoint(130,60), wxSize(115, -1) );
    Connect( m_pToggleSeedMap->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnMapSeeding) );
	m_pToggleSeedMap->Enable(false);

	wxBoxSizer *pBoxRow3 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow3->Add( m_pBtnSelectSeed, 0, wxALIGN_CENTER | wxALL, 1 );
    pBoxRow3->Add( m_pToggleSeedMap,   0, wxALIGN_CENTER | wxALL, 1);
	m_pTrackingSizer->Add( pBoxRow3, 0, wxFIXED_MINSIZE | wxALL, 2 );

    m_pBtnSelectMap = new wxButton( this, wxID_ANY,wxT("Mask not selected"), wxPoint(30,90), wxSize(230, -1) );
    Connect( m_pBtnSelectMap->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnSelectMask) );
    m_pBtnSelectMap->SetBackgroundColour(wxColour( 255, 147, 147 ));

	wxBoxSizer *pBoxRow4 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow4->Add( m_pBtnSelectMap, 0, wxALIGN_CENTER | wxALL, 1 );
	m_pTrackingSizer->Add( pBoxRow4, 0, wxFIXED_MINSIZE | wxALL, 2 );

    m_pTextFA = new wxStaticText( this, wxID_ANY, wxT("Min Mask"), wxPoint(0,120), wxSize(70, -1), wxALIGN_CENTER );
    m_pSliderFA = new MySlider( this, wxID_ANY, 0, 1, 50, wxPoint(60,120), wxSize(100, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderFA->SetValue( 10 );
    Connect( m_pSliderFA->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderFAMoved) );
    m_pTxtFABox = new wxTextCtrl( this, wxID_ANY, wxT("0.10"), wxPoint(190,120), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );
    m_pTextFA->Enable(false);
    m_pSliderFA->Enable(false);
    m_pTxtFABox->Enable(false);

	wxBoxSizer *pBoxRow5 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow5->Add( m_pTextFA, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxRow5->Add( m_pSliderFA,   0, wxALIGN_LEFT | wxEXPAND | wxALL, 1);
	pBoxRow5->Add( m_pTxtFABox,   0, wxALIGN_LEFT | wxALL, 1);
	m_pTrackingSizer->Add( pBoxRow5, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    m_pTextAngle = new wxStaticText( this, wxID_ANY, wxT("Max angle"), wxPoint(0,150), wxSize(70, -1), wxALIGN_CENTER );
    m_pSliderAngle = new MySlider( this, wxID_ANY, 0, 1, 90, wxPoint(60,150), wxSize(100, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderAngle->SetValue( 35 );
    Connect( m_pSliderAngle->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderAngleMoved) );
    m_pTxtAngleBox = new wxTextCtrl( this, wxID_ANY, wxT("35.0 "), wxPoint(190,150), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

		wxBoxSizer *pBoxRow6 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow6->Add( m_pTextAngle, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxRow6->Add( m_pSliderAngle,   0, wxALIGN_LEFT | wxEXPAND | wxALL, 1);
	pBoxRow6->Add( m_pTxtAngleBox,   0, wxALIGN_LEFT | wxALL, 1);
	m_pTrackingSizer->Add( pBoxRow6, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    m_pTextStep = new wxStaticText( this, wxID_ANY, wxT("Step"), wxPoint(0,180), wxSize(70, -1), wxALIGN_CENTER );
    m_pSliderStep = new MySlider( this, wxID_ANY, 0, 5, 20, wxPoint(60,180), wxSize(100, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderStep->SetValue( 10 );
    Connect( m_pSliderStep->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderStepMoved) );
    m_pTxtStepBox = new wxTextCtrl( this, wxID_ANY, wxT("1.0 mm"), wxPoint(190,180), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

	wxBoxSizer *pBoxRow7 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow7->Add( m_pTextStep, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxRow7->Add( m_pSliderStep,   0, wxALIGN_LEFT | wxEXPAND | wxALL, 1);
	pBoxRow7->Add( m_pTxtStepBox,   0, wxALIGN_LEFT | wxALL, 1);
	m_pTrackingSizer->Add( pBoxRow7, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    m_pTextPuncture = new wxStaticText( this, wxID_ANY, wxT("g"), wxPoint(0,210), wxSize(70, -1), wxALIGN_CENTER );
    m_pSliderPuncture = new MySlider( this, wxID_ANY, 0, 1, 99, wxPoint(60,210), wxSize(100, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderPuncture->SetValue( 20 );
    Connect( m_pSliderPuncture->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderPunctureMoved) );
    m_pTxtPunctureBox = new wxTextCtrl( this, wxID_ANY, wxT("0.20"), wxPoint(190,210), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

	wxBoxSizer *pBoxRow8 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow8->Add( m_pTextPuncture, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxRow8->Add( m_pSliderPuncture,   0, wxALIGN_LEFT | wxEXPAND | wxALL, 1);
	pBoxRow8->Add( m_pTxtPunctureBox,   0, wxALIGN_LEFT | wxALL, 1);
	m_pTrackingSizer->Add( pBoxRow8, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    m_pTextMinLength = new wxStaticText( this, wxID_ANY, wxT("Min length"), wxPoint(0,240), wxSize(70, -1), wxALIGN_CENTER );
    m_pSliderMinLength = new MySlider( this, wxID_ANY, 0, 0, 400, wxPoint(60,240), wxSize(100, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderMinLength->SetValue( 10 );
    Connect( m_pSliderMinLength->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderMinLengthMoved) );
    m_pTxtMinLengthBox = new wxTextCtrl( this, wxID_ANY, wxT("10 mm"), wxPoint(190,240), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

	wxBoxSizer *pBoxRow9 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow9->Add( m_pTextMinLength, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxRow9->Add( m_pSliderMinLength,   0, wxALIGN_LEFT | wxEXPAND | wxALL, 1);
	pBoxRow9->Add( m_pTxtMinLengthBox,   0, wxALIGN_LEFT | wxALL, 1);
	m_pTrackingSizer->Add( pBoxRow9, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    m_pTextMaxLength = new wxStaticText( this, wxID_ANY, wxT("Max length"), wxPoint(0,270), wxSize(70, -1), wxALIGN_CENTER );
    m_pSliderMaxLength = new MySlider( this, wxID_ANY, 0, 0, 300, wxPoint(60,270), wxSize(100, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderMaxLength->SetValue( 200 );
    Connect( m_pSliderMaxLength->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderMaxLengthMoved) );
    m_pTxtMaxLengthBox = new wxTextCtrl( this, wxID_ANY, wxT("200 mm"), wxPoint(190,270), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

	wxBoxSizer *pBoxRow10 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow10->Add( m_pTextMaxLength, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxRow10->Add( m_pSliderMaxLength,   0, wxALIGN_LEFT | wxEXPAND | wxALL, 1);
	pBoxRow10->Add( m_pTxtMaxLengthBox,   0, wxALIGN_LEFT | wxALL, 1);
	m_pTrackingSizer->Add( pBoxRow10, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    m_pTextAxisSeedNb = new wxStaticText( this, wxID_ANY, wxT("Seed/axis"), wxPoint(0,300), wxSize(70, -1), wxALIGN_CENTER );
    m_pSliderAxisSeedNb = new MySlider( this, wxID_ANY, 0, 1, 15, wxPoint(60,300), wxSize(100, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderAxisSeedNb->SetValue( 10 );
    Connect( m_pSliderAxisSeedNb->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderAxisSeedNbMoved) );
    m_pTxtAxisSeedNbBox = new wxTextCtrl( this, wxID_ANY, wxT("10"), wxPoint(190,300), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

	wxBoxSizer *pBoxRow11 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow11->Add( m_pTextAxisSeedNb, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxRow11->Add( m_pSliderAxisSeedNb,   0, wxALIGN_LEFT | wxEXPAND | wxALL, 1);
	pBoxRow11->Add( m_pTxtAxisSeedNbBox,   0, wxALIGN_LEFT | wxALL, 1);
	m_pTrackingSizer->Add( pBoxRow11, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    m_pTextTotalSeedNb = new wxStaticText( this, wxID_ANY, wxT("Number of current seeds"), wxPoint(7,330), wxSize(170, -1), wxALIGN_LEFT );
    m_pTxtTotalSeedNbBox = new wxTextCtrl( this, wxID_ANY, wxT("1000"), wxPoint(190,330), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

	wxBoxSizer *pBoxRow12 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow12->Add( m_pTextTotalSeedNb, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
	pBoxRow12->Add( m_pTxtTotalSeedNbBox,   0, wxALIGN_LEFT | wxALL, 1);
	m_pTrackingSizer->Add( pBoxRow12, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

	m_pBtnConvert = new wxButton( this, wxID_ANY,wxT("Convert Fibers"), wxPoint(50,350), wxSize(230, 30) );
	Connect( m_pBtnConvert->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnConvertToFibers) );
	m_pTrackingSizer->Add( m_pBtnConvert, 0, wxALL, 2 );

    /*-----------------------ANIMATION SECTION -----------------------------------*/

    m_pLineSeparator = new wxStaticLine( this, wxID_ANY, wxPoint(0,390), wxSize(230,-1),wxHORIZONTAL,wxT("Separator"));

    wxTextCtrl *animationZone = new wxTextCtrl( this, wxID_ANY, wxT("Animation"), wxPoint(50,400), wxSize(150, -1), wxTE_CENTER | wxTE_READONLY );
    animationZone->SetBackgroundColour( *wxLIGHT_GREY );
    wxFont font = animationZone->GetFont();
    font.SetPointSize( 10 );
    font.SetWeight( wxFONTWEIGHT_BOLD );
    animationZone->SetFont( font );

	wxBoxSizer *pBoxRowAnim1 = new wxBoxSizer( wxVERTICAL );
    pBoxRowAnim1->Add( m_pLineSeparator, 0, wxALIGN_RIGHT | wxALL, 1 );
	pBoxRowAnim1->Add( animationZone,   0, wxALIGN_CENTER | wxALL, 1);
	m_pTrackingSizer->Add( pBoxRowAnim1, 0, wxFIXED_MINSIZE | wxALL, 2 );
    
    m_bmpPlay = wxImage(MyApp::iconsPath+ wxT("play.png"), wxBITMAP_TYPE_PNG);
    m_bmpPause = wxImage(MyApp::iconsPath+ wxT("pause.png"), wxBITMAP_TYPE_PNG);

    wxImage bmpStop(MyApp::iconsPath+ wxT("stop.png"), wxBITMAP_TYPE_PNG);

    m_pPlayPause = new wxBitmapButton( this, wxID_ANY,m_bmpPlay, wxPoint(50,440), wxSize(50, -1) );
    Connect( m_pPlayPause->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnPlay) );

    m_pBtnStop = new wxBitmapButton( this, wxID_ANY, bmpStop, wxPoint(100,440), wxSize(50, -1) );
    Connect( m_pBtnStop->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnStop) );

	wxBoxSizer *pBoxRowAnim = new wxBoxSizer( wxHORIZONTAL );
    pBoxRowAnim->Add( m_pPlayPause, 0,  wxALIGN_CENTER , 1 );
	pBoxRowAnim->Add( m_pBtnStop, 0,  wxALIGN_CENTER, 1 );
	m_pTrackingSizer->Add( pBoxRowAnim, 0, wxFIXED_MINSIZE | wxALL, 2 );

}

void TrackingWindow::OnSize( wxSizeEvent &WXUNUSED(event) )
{
}

void TrackingWindow::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
    wxPaintDC dc( this );
}

wxSizer* TrackingWindow::getWindowSizer()
{
    return m_pTrackingSizer;
}

void TrackingWindow::OnStartTracking( wxCommandEvent& WXUNUSED(event) )
{
    RTTrackingHelper::getInstance()->toggleRTTReady();
    RTTrackingHelper::getInstance()->setRTTDirty( true );

    if( !RTTrackingHelper::getInstance()->isRTTReady() )
    {
        m_pMainFrame->m_pMainGL->m_pRealTimeFibers->clearFibersRTT();
        m_pMainFrame->m_pMainGL->m_pRealTimeFibers->clearColorsRTT();
        RTTrackingHelper::getInstance()->setRTTDirty( false );
        m_pBtnStart->SetLabel(wxT("Start tracking"));
    }
    else
    {
        m_pBtnStart->SetLabel(wxT("Stop tracking"));
    }
}

void TrackingWindow::OnClearBox( wxTreeEvent&    event )
{
    m_pMainFrame->onDeleteTreeItem( event );
    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->clearFibersRTT();
    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->clearColorsRTT();
    RTTrackingHelper::getInstance()->setRTTDirty( false );
    RTTrackingHelper::getInstance()->setRTTReady( false );
    m_pBtnStart->SetValue( false );
}

void TrackingWindow::OnSliderFAMoved(wxCommandEvent& WXUNUSED(event))
{
    float sliderValue = m_pSliderFA->GetValue() / 100.0f;
    m_pTxtFABox->SetValue( wxString::Format( wxT( "%.2f"), sliderValue ) );
    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setFAThreshold( sliderValue );
    RTTrackingHelper::getInstance()->setRTTDirty( true );
}

void TrackingWindow::OnSliderAngleMoved( wxCommandEvent& WXUNUSED(event) )
{
    float sliderValue = m_pSliderAngle->GetValue();
    m_pTxtAngleBox->SetValue(wxString::Format( wxT( "%.1f "), sliderValue ) );
    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setAngleThreshold( sliderValue );
    RTTrackingHelper::getInstance()->setRTTDirty( true );
}

void TrackingWindow::OnSliderStepMoved( wxCommandEvent& WXUNUSED(event) )
{
    float sliderValue = m_pSliderStep->GetValue() / 10.0f;
    m_pTxtStepBox->SetValue(wxString::Format( wxT( "%.1f mm"), sliderValue) );
    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setStep( sliderValue );
    RTTrackingHelper::getInstance()->setRTTDirty( true );
}

void TrackingWindow::OnSliderPunctureMoved( wxCommandEvent& WXUNUSED(event) )
{
    float sliderValue = m_pSliderPuncture->GetValue() / 100.0f;
    m_pTxtPunctureBox->SetValue(wxString::Format( wxT( "%.2f"), sliderValue) );
    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setPuncture( sliderValue );
    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setVinVout( sliderValue );
    RTTrackingHelper::getInstance()->setRTTDirty( true );
}

void TrackingWindow::OnSliderMinLengthMoved( wxCommandEvent& WXUNUSED(event) )
{
    float sliderValue = m_pSliderMinLength->GetValue();
    m_pTxtMinLengthBox->SetValue(wxString::Format( wxT( "%.1f mm"), sliderValue) );
    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setMinFiberLength( sliderValue );
    RTTrackingHelper::getInstance()->setRTTDirty( true );
}

void TrackingWindow::OnSliderMaxLengthMoved( wxCommandEvent& WXUNUSED(event) )
{
    float sliderValue = m_pSliderMaxLength->GetValue();
    m_pTxtMaxLengthBox->SetValue(wxString::Format( wxT( "%.1f mm"), sliderValue) );
    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setMaxFiberLength( sliderValue );
    RTTrackingHelper::getInstance()->setRTTDirty( true );
}

void TrackingWindow::OnSelectFileDTI( wxCommandEvent& WXUNUSED(event) )
{
    //Tensor data
    long item = m_pMainFrame->getCurrentListIndex();
    Tensors* pTensorInfo = (Tensors *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( item ) );
	
	if( pTensorInfo != NULL && TENSORS == pTensorInfo->getType() && 6 == pTensorInfo->getBands()  )
    {
		//Hide tensor data
		pTensorInfo->setShow(false);
		m_pMainFrame->m_pListCtrl->UpdateSelected();
		m_pMainFrame->refreshAllGLWidgets();
        m_pBtnSelectFile->SetLabel( pTensorInfo->getName() );
        m_pBtnSelectFile->SetBackgroundColour(wxNullColour);

        //Set Step
        float step = DatasetManager::getInstance()->getVoxelX() / 2.0f;
        m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setTensorsInfo( (Tensors *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( item ) ) );

        if(SceneManager::getInstance()->getSelectionTree().isEmpty())
        {
            m_pMainFrame->createNewSelectionObject( BOX_TYPE );
            
            SelectionObject* pNewSelObj(m_pMainFrame->getCurrentSelectionObject());
            
            Vector boxSize(2/step,2/step,2/step);
            
            pNewSelObj->setSize(boxSize);
        }
        m_pMainFrame->m_pTrackingWindow->m_pBtnStart->SetBackgroundColour(wxColour( 147, 255, 239 ));
        m_pMainFrame->m_pTrackingWindow->m_pBtnStart->Enable( true );
    }
}

void TrackingWindow::OnSelectFileHARDI( wxCommandEvent& WXUNUSED(event) )
{
    //HARDI data
    long item = m_pMainFrame->getCurrentListIndex();
    Maximas* pMaximasInfo = (Maximas *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( item ) );
	
	if( pMaximasInfo != NULL && pMaximasInfo->getType() == MAXIMAS )
    {
		//Hide hardi data
		pMaximasInfo->setShow(false);
		m_pMainFrame->m_pListCtrl->UpdateSelected();
		m_pMainFrame->refreshAllGLWidgets();

        m_pBtnSelectFile->SetLabel( pMaximasInfo->getName() );
        m_pBtnSelectFile->SetBackgroundColour(wxNullColour);

        //Set Step
        float step = DatasetManager::getInstance()->getVoxelX() / 2.0f;
        m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setIsHardi( true );
        m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setHARDIInfo( (Maximas *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( item ) ) );
        
        if(SceneManager::getInstance()->getSelectionTree().isEmpty())
        {
            m_pMainFrame->createNewSelectionObject( BOX_TYPE );
            
            SelectionObject* pNewSelObj = m_pMainFrame->getCurrentSelectionObject();
            
            Vector boxSize(2/step,2/step,2/step);
            pNewSelObj->setSize(boxSize);
        }
        if(m_pTextFA->IsEnabled())
        {
            m_pMainFrame->m_pTrackingWindowHardi->m_pBtnStart->SetBackgroundColour(wxColour( 147, 255, 239 ));
            m_pMainFrame->m_pTrackingWindowHardi->m_pBtnStart->Enable( true );   
        }
    }
}

void TrackingWindow::OnSelectShell( wxCommandEvent& WXUNUSED(event) )
{
	//Select surface for seeding
    long item = m_pMainFrame->getCurrentListIndex();
	DatasetInfo* pMesh = DatasetManager::getInstance()->getDataset (MyApp::frame->m_pListCtrl->GetItem( item )); 

	if( pMesh != NULL && pMesh->getType() == ISO_SURFACE )
    {
		m_pBtnSelectShell->SetLabel( pMesh->getName() );
		m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setShellInfo( (DatasetInfo *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( item ) ) );
		m_pToggleShell->Enable(true);
        m_pToggleShell->SetValue(true);

        RTTrackingHelper::getInstance()->setShellSeed(true);
        RTTrackingHelper::getInstance()->setRTTDirty( true );
        float sliderValue = m_pSliderAxisSeedNb->GetValue();

        //Set nb of seeds depending on the seeding mode
        if( !RTTrackingHelper::getInstance()->isShellSeeds() )
        {
            m_pTxtTotalSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), sliderValue*sliderValue*sliderValue) );
            m_pToggleShell->SetLabel(wxT("Shell seed OFF"));
        }
        else
        {
            float shellSeedNb = m_pMainFrame->m_pMainGL->m_pRealTimeFibers->getShellSeedNb();
            m_pTxtTotalSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), shellSeedNb) ); 
            m_pToggleShell->SetLabel(wxT( "Shell seed ON"));
        } 
	}
}

void TrackingWindow::OnSelectSeedMap( wxCommandEvent& WXUNUSED(event) )
{
	//Select map for threshold seeding
    long item = m_pMainFrame->getCurrentListIndex();
	Anatomy* pSeedMap = (Anatomy*)DatasetManager::getInstance()->getDataset (MyApp::frame->m_pListCtrl->GetItem( item )); 

	if( pSeedMap != NULL && pSeedMap->getBands() == 1 )
    {
		m_pBtnSelectSeed->SetLabel( pSeedMap->getName() );
		m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setSeedMapInfo( (Anatomy *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( item ) ) );

		m_pToggleSeedMap->Enable(true);
        m_pToggleSeedMap->SetValue(true);

        RTTrackingHelper::getInstance()->setSeedMap(true);
        RTTrackingHelper::getInstance()->setRTTDirty( true );
        float sliderValue = m_pSliderAxisSeedNb->GetValue();

        //Set nb of seeds depending on the seeding mode
        if( !RTTrackingHelper::getInstance()->isSeedMap() )
        {
			m_pSliderAxisSeedNb->SetValue( sliderValue );
            m_pTxtTotalSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), sliderValue*sliderValue*sliderValue) );
            m_pToggleSeedMap->SetLabel(wxT("Seed map OFF"));
        }
        else
        {
			m_pSliderAxisSeedNb->SetValue( 1 );
			m_pTxtAxisSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), 1.0f) );
			m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setNbSeed( 1 );
            float seedMapNb = m_pMainFrame->m_pMainGL->m_pRealTimeFibers->getSeedMapNb();
            m_pTxtTotalSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), seedMapNb) ); 
            m_pToggleSeedMap->SetLabel(wxT( "Seed map ON"));
        } 
	}
}

void TrackingWindow::OnMapSeeding( wxCommandEvent& WXUNUSED(event) )
{
    RTTrackingHelper::getInstance()->toggleSeedMap();
    RTTrackingHelper::getInstance()->setRTTDirty( true );
	m_pBtnStart->Enable( true );
    
	//Set nb of seeds depending on the seeding mode
	if( !RTTrackingHelper::getInstance()->isSeedMap() )
    {
		m_pSliderAxisSeedNb->SetValue( 10 );
        m_pTxtTotalSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), 1000.0f) );
		m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setNbSeed( 10 );
	    m_pTxtAxisSeedNbBox->SetValue( wxString::Format( wxT( "%.1f"), 10.0f) );
        m_pToggleSeedMap->SetLabel(wxT( "Seed map OFF"));
    }
    else
    {
		m_pSliderAxisSeedNb->SetValue( 1 );
		m_pTxtAxisSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), 1.0f) );
		m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setNbSeed( 1 );
        float seedMapNb = m_pMainFrame->m_pMainGL->m_pRealTimeFibers->getSeedMapNb();
        m_pTxtTotalSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), seedMapNb) ); 
        m_pToggleSeedMap->SetLabel(wxT( "Seed map ON"));
    }
}

void TrackingWindow::OnSelectMask( wxCommandEvent& WXUNUSED(event) )
{
	//Select map for threshold seeding
    long item = m_pMainFrame->getCurrentListIndex();
	Anatomy* pMap = (Anatomy*)DatasetManager::getInstance()->getDataset (MyApp::frame->m_pListCtrl->GetItem( item )); 

	if( pMap != NULL && pMap->getBands() == 1 )
    {
		m_pBtnSelectMap->SetLabel( pMap->getName() );
        m_pBtnSelectMap->SetBackgroundColour(wxNullColour);
		m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setMaskInfo( (Anatomy *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( item ) ) );
        m_pTextFA->Enable(true);
        m_pSliderFA->Enable(true);
        m_pTxtFABox->Enable(true);
	}
    if(m_pMainFrame->m_pMainGL->m_pRealTimeFibers->isHardiSelected())
    {
        m_pMainFrame->m_pTrackingWindowHardi->m_pBtnStart->Enable( true );
        m_pMainFrame->m_pTrackingWindowHardi->m_pBtnStart->SetBackgroundColour(wxColour( 147, 255, 239 ));
    }
}

void TrackingWindow::OnShellSeeding( wxCommandEvent& WXUNUSED(event) )
{
    RTTrackingHelper::getInstance()->toggleShellSeeds();
    RTTrackingHelper::getInstance()->setRTTDirty( true );
    float sliderValue = m_pSliderAxisSeedNb->GetValue();
	m_pBtnStart->Enable( true );
    
	//Set nb of seeds depending on the seeding mode
	if( !RTTrackingHelper::getInstance()->isShellSeeds() )
    {
        m_pTxtTotalSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), sliderValue*sliderValue*sliderValue) );
        m_pToggleShell->SetLabel(wxT( "Shell seed OFF"));
    }
    else
    {
        float shellSeedNb = m_pMainFrame->m_pMainGL->m_pRealTimeFibers->getShellSeedNb();
        m_pTxtTotalSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), shellSeedNb) ); 
        m_pToggleShell->SetLabel(wxT( "Shell seed ON"));
    }
}

//Deprecated
void TrackingWindow::OnRandomSeeding( wxCommandEvent& WXUNUSED(event) )
{
    //RTTrackingHelper::getInstance()->toggleRandomSeeds();
    //RTTrackingHelper::getInstance()->setRTTDirty( true );
}

//Deprecated
void TrackingWindow::OnInterpolate( wxCommandEvent& WXUNUSED(event) )
{
    RTTrackingHelper::getInstance()->toggleInterpolateTensors();
    RTTrackingHelper::getInstance()->setRTTDirty( true );
}

void TrackingWindow::OnSliderAxisSeedNbMoved( wxCommandEvent& WXUNUSED(event) )
{
    float sliderValue = m_pSliderAxisSeedNb->GetValue();
    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setNbSeed( sliderValue );
    m_pTxtAxisSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), sliderValue) );

	if( !RTTrackingHelper::getInstance()->isShellSeeds() && !RTTrackingHelper::getInstance()->isSeedMap())
    {
        m_pTxtTotalSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), sliderValue*sliderValue*sliderValue) );
    }
	else if( RTTrackingHelper::getInstance()->isSeedMap())
	{
		float mapSeedNb = m_pMainFrame->m_pMainGL->m_pRealTimeFibers->getSeedMapNb();
		m_pTxtTotalSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), mapSeedNb) );
	}
    else
    {
        float shellSeedNb = m_pMainFrame->m_pMainGL->m_pRealTimeFibers->getNbMeshPoint();
        m_pTxtTotalSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), shellSeedNb) );
    }
    RTTrackingHelper::getInstance()->setRTTDirty( true );
}

void TrackingWindow::OnConvertToFibers( wxCommandEvent& WXUNUSED(event) )
{
	//Convert fibers
	DatasetIndex index = DatasetManager::getInstance()->createFibers( m_pMainFrame->m_pMainGL->m_pRealTimeFibers->getRTTFibers() );

	if( !DatasetManager::getInstance()->isFibersGroupLoaded() )
    {
        DatasetIndex result = DatasetManager::getInstance()->createFibersGroup();
        m_pMainFrame->m_pListCtrl->InsertItem( result );
    }

	m_pMainFrame->m_pListCtrl->InsertItem( index );

    RTTrackingHelper::getInstance()->setRTTReady(false);

    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->clearFibersRTT();
    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->clearColorsRTT();
    RTTrackingHelper::getInstance()->setRTTDirty( false );
    m_pBtnStart->SetLabel(wxT("Start tracking"));
    m_pBtnStart->SetValue(false);

}

void TrackingWindow::OnPlay( wxCommandEvent& WXUNUSED(event) )
{
    RTTrackingHelper::getInstance()->setTrackAction(true);
    RTTrackingHelper::getInstance()->togglePlayStop();
    if(!RTTrackingHelper::getInstance()->isTrackActionPaused())
    {
        m_pPlayPause->SetBitmapLabel(m_bmpPause);
        m_pMainFrame->setTimerSpeed();
    }
    else
    {
        m_pPlayPause->SetBitmapLabel(m_bmpPlay);
        m_pMainFrame->setTimerSpeed();
    }
}

void TrackingWindow::OnStop( wxCommandEvent& WXUNUSED(event) )
{
    m_pMainFrame->m_pMainGL->m_pRealTimeFibers->m_trackActionStep = 0;
    RTTrackingHelper::getInstance()->setTrackAction(false);
    RTTrackingHelper::getInstance()->setTrackActionPause(true);
    m_pPlayPause->SetBitmapLabel(m_bmpPlay);
    m_pMainFrame->setTimerSpeed();
}