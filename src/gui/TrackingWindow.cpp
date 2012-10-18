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
EVT_PAINT( TrackingWindow::OnPaint )
EVT_SIZE( TrackingWindow::OnSize )
END_EVENT_TABLE()


TrackingWindow::TrackingWindow( wxWindow *pParent, MainFrame *pMf, wxWindowID id, const wxPoint &pos, const wxSize &size )
:   wxScrolledWindow( pParent, id, pos, size, wxBORDER_NONE, _T("DTI RTT Canvas") ),
    m_pMainFrame( pMf )
{
    SetBackgroundColour( *wxLIGHT_GREY );
    SetCursor( wxCursor( wxCURSOR_HAND ) );
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

    //wxImage bmpDelete(MyApp::iconsPath+ wxT("delete.png" ), wxBITMAP_TYPE_PNG);
    //wxBitmapButton *m_pbtnDelete = new wxBitmapButton(this, wxID_ANY, bmpDelete, wxPoint(170,30), wxSize(60,-1));
    //Connect(m_pbtnDelete->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxTreeEventHandler(TrackingWindow::OnClearBox));

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

    //wxToggleButton *m_pToggleRandom = new wxToggleButton( this, wxID_ANY,wxT("Use random seeds"), wxPoint(50,240), wxSize(140, -1) );
    //Connect( m_pToggleRandom->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnRandomSeeding) );

    //wxToggleButton *m_pToggleInterp = new wxToggleButton( m_pTrackingWindow, wxID_ANY,wxT("Interpolation"), wxPoint(0,270), wxSize(140, -1) );
    //m_pTrackingWindow->Connect( m_pToggleInterp->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnInterpolate) );

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
    SetCursor( wxCursor( wxCURSOR_HAND ) );
    m_pTrackingSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( m_pTrackingSizer );
    SetAutoLayout( true );

    //Content of RTT panel
    /********************************/

    m_pBtnSelectFile = new wxButton( this, wxID_ANY,wxT("HARDI not selected"), wxPoint(30,0), wxSize(100, -1) );
    Connect( m_pBtnSelectFile->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnSelectFileHARDI) );
    m_pBtnSelectFile->SetBackgroundColour(wxColour( 255, 147, 147 ));

    m_pBtnStart = new wxToggleButton( this, wxID_ANY,wxT("Start tracking"), wxPoint(130,0), wxSize(100, -1) );
    Connect( m_pBtnStart->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnStartTracking) );
    m_pBtnStart->Enable(false);

    //wxImage bmpDelete(MyApp::iconsPath+ wxT("delete.png" ), wxBITMAP_TYPE_PNG);
    //wxBitmapButton *m_pbtnDelete = new wxBitmapButton(this, wxID_ANY, bmpDelete, wxPoint(170,30), wxSize(60,-1));
    //Connect(m_pbtnDelete->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxTreeEventHandler(TrackingWindow::OnClearBox));

	m_pBtnSelectShell = new wxButton( this, wxID_ANY,wxT("Shell not selected"), wxPoint(30,30), wxSize(100, -1) );
    Connect( m_pBtnSelectShell->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnSelectShell) );

    m_pToggleShell = new wxToggleButton( this, wxID_ANY,wxT("Shell seed OFF"), wxPoint(130,30), wxSize(100, -1) );
    Connect( m_pToggleShell->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnShellSeeding) );
	m_pToggleShell->Enable(false);

    m_pBtnSelectMap = new wxButton( this, wxID_ANY,wxT("Map not selected"), wxPoint(30,60), wxSize(200, -1) );
    Connect( m_pBtnSelectMap->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnSelectMap) );
    m_pBtnSelectMap->SetBackgroundColour(wxColour( 255, 147, 147 ));

    m_pTextFA = new wxStaticText( this, wxID_ANY, wxT("Min FA"), wxPoint(0,90), wxSize(60, -1), wxALIGN_CENTER );
    m_pSliderFA = new MySlider( this, wxID_ANY, 0, 1, 50, wxPoint(60,90), wxSize(130, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderFA->SetValue( 10 );
    Connect( m_pSliderFA->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderFAMoved) );
    m_pTxtFABox = new wxTextCtrl( this, wxID_ANY, wxT("0.10"), wxPoint(190,90), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );
    m_pTextFA->Enable(false);
    m_pSliderFA->Enable(false);
    m_pTxtFABox->Enable(false);

    m_pTextAngle = new wxStaticText( this, wxID_ANY, wxT("Max angle"), wxPoint(0,120), wxSize(60, -1), wxALIGN_CENTER );
    m_pSliderAngle = new MySlider( this, wxID_ANY, 0, 1, 90, wxPoint(60,120), wxSize(130, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderAngle->SetValue( 35 );
    Connect( m_pSliderAngle->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderAngleMoved) );
    m_pTxtAngleBox = new wxTextCtrl( this, wxID_ANY, wxT("35.0 "), wxPoint(190,120), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

    m_pTextStep = new wxStaticText( this, wxID_ANY, wxT("Step"), wxPoint(0,150), wxSize(60, -1), wxALIGN_CENTER );
    m_pSliderStep = new MySlider( this, wxID_ANY, 0, 5, 20, wxPoint(60,150), wxSize(130, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderStep->SetValue( 10 );
    Connect( m_pSliderStep->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderStepMoved) );
    m_pTxtStepBox = new wxTextCtrl( this, wxID_ANY, wxT("1.0 mm"), wxPoint(190,150), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

    m_pTextPuncture = new wxStaticText( this, wxID_ANY, wxT("Vin-Vout"), wxPoint(0,180), wxSize(60, -1), wxALIGN_CENTER );
    m_pSliderPuncture = new MySlider( this, wxID_ANY, 0, 1, 99, wxPoint(60,180), wxSize(130, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderPuncture->SetValue( 80 );
    Connect( m_pSliderPuncture->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderPunctureMoved) );
    m_pTxtPunctureBox = new wxTextCtrl( this, wxID_ANY, wxT("0.8"), wxPoint(190,180), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

    m_pTextMinLength = new wxStaticText( this, wxID_ANY, wxT("Min length"), wxPoint(0,210), wxSize(60, -1), wxALIGN_CENTER );
    m_pSliderMinLength = new MySlider( this, wxID_ANY, 0, 0, 400, wxPoint(60,210), wxSize(130, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderMinLength->SetValue( 10 );
    Connect( m_pSliderMinLength->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderMinLengthMoved) );
    m_pTxtMinLengthBox = new wxTextCtrl( this, wxID_ANY, wxT("10 mm"), wxPoint(190,210), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

    m_pTextMaxLength = new wxStaticText( this, wxID_ANY, wxT("Max length"), wxPoint(0,240), wxSize(60, -1), wxALIGN_CENTER );
    m_pSliderMaxLength = new MySlider( this, wxID_ANY, 0, 0, 300, wxPoint(60,240), wxSize(130, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderMaxLength->SetValue( 200 );
    Connect( m_pSliderMaxLength->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderMaxLengthMoved) );
    m_pTxtMaxLengthBox = new wxTextCtrl( this, wxID_ANY, wxT("200 mm"), wxPoint(190,240), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

    //wxToggleButton *m_pToggleRandom = new wxToggleButton( this, wxID_ANY,wxT("Use random seeds"), wxPoint(50,240), wxSize(140, -1) );
    //Connect( m_pToggleRandom->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnRandomSeeding) );

    //wxToggleButton *m_pToggleInterp = new wxToggleButton( m_pTrackingWindow, wxID_ANY,wxT("Interpolation"), wxPoint(0,270), wxSize(140, -1) );
    //m_pTrackingWindow->Connect( m_pToggleInterp->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnInterpolate) );

    m_pTextAxisSeedNb = new wxStaticText( this, wxID_ANY, wxT("Seed/axis"), wxPoint(0,270), wxSize(60, -1), wxALIGN_CENTER );
    m_pSliderAxisSeedNb = new MySlider( this, wxID_ANY, 0, 1, 15, wxPoint(60,270), wxSize(130, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderAxisSeedNb->SetValue( 10 );
    Connect( m_pSliderAxisSeedNb->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(TrackingWindow::OnSliderAxisSeedNbMoved) );
    m_pTxtAxisSeedNbBox = new wxTextCtrl( this, wxID_ANY, wxT("10"), wxPoint(190,270), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

    m_pTextTotalSeedNb = new wxStaticText( this, wxID_ANY, wxT("Number of current seeds"), wxPoint(7,300), wxSize(150, -1), wxALIGN_LEFT );
    m_pTxtTotalSeedNbBox = new wxTextCtrl( this, wxID_ANY, wxT("1000"), wxPoint(190,300), wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

	m_pBtnConvert = new wxButton( this, wxID_ANY,wxT("Convert Fibers"), wxPoint(50,330), wxSize(140, 30) );
	Connect( m_pBtnConvert->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrackingWindow::OnConvertToFibers) );
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
	
	//Hide tensor data
	pTensorInfo->setShow(false);
	m_pMainFrame->m_pListCtrl->UpdateSelected();
    m_pMainFrame->refreshAllGLWidgets();

    if( pTensorInfo != NULL )
    {
        m_pBtnSelectFile->SetLabel( pTensorInfo->getName() );
        m_pBtnSelectFile->SetBackgroundColour(wxNullColour);

        //Set Step
        float step = DatasetManager::getInstance()->getVoxelX() / 2.0f;
        m_pSliderStep->SetValue( step * 10.0f );
        m_pTxtStepBox->SetValue( wxString::Format( wxT( "%.1f mm"), step) );
        m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setStep( step );

        m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setTensorsInfo( (Tensors *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( item ) ) );
        
        std::vector< std::vector< SelectionObject* > > selectionObjects = SceneManager::getInstance()->getSelectionObjects();

        if(selectionObjects.empty())
        {
            m_pMainFrame->createNewSelectionObject( BOX_TYPE );
            Vector boxSize(2/step,2/step,2/step);
            ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->setSize(boxSize);
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
	
	////Hide tensor data
	//pMaximasInfo->setShow(false);
	//m_pMainFrame->m_pListCtrl->UpdateSelected();
 //   m_pMainFrame->refreshAllGLWidgets();

    if( pMaximasInfo != NULL )
    {
        m_pBtnSelectFile->SetLabel( pMaximasInfo->getName() );
        m_pBtnSelectFile->SetBackgroundColour(wxNullColour);

        //Set Step
        float step = DatasetManager::getInstance()->getVoxelX() / 2.0f;
        m_pSliderStep->SetValue( step * 10.0f );
        m_pTxtStepBox->SetValue( wxString::Format( wxT( "%.1f mm"), step) );
        m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setStep( step );

        m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setIsHardi( true );
        m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setHARDIInfo( (Maximas *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( item ) ) );
        
        std::vector< std::vector< SelectionObject* > > selectionObjects = SceneManager::getInstance()->getSelectionObjects();

        if(selectionObjects.empty())
        {
            m_pMainFrame->createNewSelectionObject( BOX_TYPE ); 
            Vector boxSize(2/step,2/step,2/step);
            ((SelectionObject*)m_pMainFrame->m_pCurrentSceneObject)->setSize(boxSize);
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

	if( pMesh != NULL )
    {
		m_pBtnSelectShell->SetLabel( pMesh->getName() );
		m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setShellInfo( (DatasetInfo *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( item ) ) );
		m_pToggleShell->Enable(true);
        m_pToggleShell->SetValue(true);

        RTTrackingHelper::getInstance()->toggleShellSeeds();
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

void TrackingWindow::OnSelectMap( wxCommandEvent& WXUNUSED(event) )
{
	//Select map for threshold seeding
    long item = m_pMainFrame->getCurrentListIndex();
	Anatomy* pMap = (Anatomy*)DatasetManager::getInstance()->getDataset (MyApp::frame->m_pListCtrl->GetItem( item )); 

	if( pMap != NULL )
    {
		m_pBtnSelectMap->SetLabel( pMap->getName() );
        m_pBtnSelectMap->SetBackgroundColour(wxNullColour);
		m_pMainFrame->m_pMainGL->m_pRealTimeFibers->setMapInfo( (Anatomy *)DatasetManager::getInstance()->getDataset( m_pMainFrame->m_pListCtrl->GetItem( item ) ) );
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

    if( !RTTrackingHelper::getInstance()->isShellSeeds() )
    {
        m_pTxtTotalSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), sliderValue*sliderValue*sliderValue) );
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
	//DatasetInfo *pDataset = DatasetManager::getInstance()->getDataset( index );

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
