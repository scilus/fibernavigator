#include "FMRIWindow.h"

#include "MainFrame.h"
#include "SceneManager.h"
#include "SelectionBox.h"
#include "SelectionEllipsoid.h"
#include "../main.h"
#include "../dataset/Anatomy.h"
#include "../dataset/Fibers.h"
#include "../dataset/RestingStateNetwork.h"
#include "../dataset/RTFMRIHelper.h"
#include "../misc/IsoSurface/CIsoSurface.h"
#include "../misc/IsoSurface/TriangleMesh.h"

#include <wx/checkbox.h>
#include <wx/grid.h>
#include <wx/tglbtn.h>
#include <wx/treectrl.h>


IMPLEMENT_DYNAMIC_CLASS( FMRIWindow, wxScrolledWindow )

BEGIN_EVENT_TABLE( FMRIWindow, wxScrolledWindow )
END_EVENT_TABLE()


FMRIWindow::FMRIWindow( wxWindow *pParent, MainFrame *pMf, wxWindowID id, const wxPoint &pos, const wxSize &size)
:   wxScrolledWindow( pParent, id, pos, size, wxBORDER_NONE, _T("fMRI resting-state networks") ),
    m_pMainFrame( pMf ),
	showRawData( true )
{
    SetBackgroundColour( *wxLIGHT_GREY );
    m_pFMRISizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( m_pFMRISizer );
    SetAutoLayout( true );

    m_pBtnSelectFMRI = new wxButton( this, wxID_ANY,wxT("Load resting-state"), wxDefaultPosition, wxSize(230, -1) );
	pMf->Connect( m_pBtnSelectFMRI->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::onLoadAsRestingState) );
    m_pBtnSelectFMRI->SetBackgroundColour(wxColour( 255, 147, 147 ));

	m_pBtnStart = new wxToggleButton( this, wxID_ANY,wxT("Start correlation"), wxDefaultPosition, wxSize(230, 50) );
    Connect( m_pBtnStart->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(FMRIWindow::OnStartRTFMRI) );
    m_pBtnStart->Enable(false);

	wxBoxSizer *pBoxRow1 = new wxBoxSizer( wxVERTICAL );
	pBoxRow1->Add( m_pBtnSelectFMRI, 0, wxALIGN_CENTER | wxALL, 1 );
	pBoxRow1->Add( m_pBtnStart, 0, wxALIGN_CENTER | wxALL, 1 );
	m_pFMRISizer->Add( pBoxRow1, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

	m_pTextDisplayMode = new wxStaticText( this, wxID_ANY, wxT( "Display:" ), wxDefaultPosition, wxSize(200, -1) );
    m_pRadShowRawData = new wxRadioButton( this,  wxID_ANY, wxT( "Raw Data" ), wxDefaultPosition, wxSize(160, -1) );
	m_pRadShowRawData->Disable();
	m_pRadShowNetwork = new wxRadioButton( this,  wxID_ANY, wxT( "Network" ), wxDefaultPosition, wxSize(160, -1) );
	m_pRadShowNetwork->Disable();
	Connect( m_pRadShowRawData->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( FMRIWindow::onSwitchViewRaw ) );
	Connect( m_pRadShowNetwork->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( FMRIWindow::onSwitchViewNet ) );

	wxBoxSizer *pBoxRow2 = new wxBoxSizer( wxVERTICAL );
	pBoxRow2->Add( m_pTextDisplayMode, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1 );
	pBoxRow2->Add( m_pRadShowRawData, 0, wxALIGN_CENTER, 1 );
	pBoxRow2->Add( m_pRadShowNetwork, 0, wxALIGN_CENTER, 1 );
	m_pFMRISizer->Add( pBoxRow2, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

	m_pTextVolumeId = new wxStaticText( this, wxID_ANY, wxT("Volume"), wxDefaultPosition, wxSize(70, -1), wxALIGN_CENTER );
	m_pSliderRest = new MySlider( this, wxID_ANY, 0, 0, 107, wxDefaultPosition, wxSize(100, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
	m_pSliderRest->Disable();
	Connect( m_pSliderRest->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(FMRIWindow::OnSliderRestMoved) );
    m_pTxtRestBox = new wxTextCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

	wxBoxSizer *pBoxRow3 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow3->Add( m_pTextVolumeId, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxRow3->Add( m_pSliderRest,   0, wxALIGN_CENTER | wxEXPAND | wxALL, 1);
	pBoxRow3->Add( m_pTxtRestBox,   0, wxALIGN_CENTER | wxALL, 1);
	m_pFMRISizer->Add( pBoxRow3, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

	m_pTextCorrThreshold = new wxStaticText( this, wxID_ANY, wxT("Z-Threshold"), wxDefaultPosition, wxSize(70, -1), wxALIGN_CENTER );
	m_pSliderCorrThreshold = new MySlider( this, wxID_ANY, 0, 0, 500, wxDefaultPosition, wxSize(100, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
	m_pSliderCorrThreshold->SetValue( 165 );
	Connect( m_pSliderCorrThreshold->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(FMRIWindow::OnSliderCorrThreshMoved) );
    m_pTxtCorrThreshBox = new wxTextCtrl( this, wxID_ANY, wxT("1.65"), wxDefaultPosition, wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

	wxBoxSizer *pBoxRow5 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow5->Add( m_pTextCorrThreshold, 0, wxALIGN_CENTER | wxALL, 1 );
    pBoxRow5->Add( m_pSliderCorrThreshold,   0, wxALIGN_CENTER | wxALL, 1);
	pBoxRow5->Add( m_pTxtCorrThreshBox,   0, wxALIGN_CENTER | wxALL, 1);
	m_pFMRISizer->Add( pBoxRow5, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

	m_pTextClusterLvl = new wxStaticText( this, wxID_ANY, wxT("Cluster level"), wxDefaultPosition, wxSize(70, -1), wxALIGN_CENTER );
	m_pSliderClusterLvl = new MySlider( this, wxID_ANY, 0, 0, 18, wxDefaultPosition, wxSize(100, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
	m_pSliderClusterLvl->SetValue( 9 );
	Connect( m_pSliderClusterLvl->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(FMRIWindow::OnSliderClusterLevelMoved) );
    m_pTxtClusterLvlBox = new wxTextCtrl( this, wxID_ANY, wxT("9.0"), wxDefaultPosition, wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

	wxBoxSizer *pBoxRow6 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow6->Add( m_pTextClusterLvl, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxRow6->Add( m_pSliderClusterLvl,   0, wxALIGN_CENTER | wxEXPAND | wxALL, 1);
	pBoxRow6->Add( m_pTxtClusterLvlBox,   0, wxALIGN_CENTER | wxALL, 1);
	m_pFMRISizer->Add( pBoxRow6, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

	m_pTextSizeP = new wxStaticText( this, wxID_ANY, wxT("Point size"), wxDefaultPosition, wxSize(70, -1), wxALIGN_CENTER );
	m_pSliderSizeP = new MySlider( this, wxID_ANY, 0, 1, 100, wxDefaultPosition, wxSize(100, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
	m_pSliderSizeP->SetValue( 5 );
	Connect( m_pSliderSizeP->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(FMRIWindow::OnSliderSizePMoved) );
    m_pTxtSizePBox = new wxTextCtrl( this, wxID_ANY, wxT("5.0"), wxDefaultPosition, wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

	wxBoxSizer *pBoxRow7 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow7->Add( m_pTextSizeP, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxRow7->Add( m_pSliderSizeP,   0, wxALIGN_CENTER | wxEXPAND | wxALL, 1);
	pBoxRow7->Add( m_pTxtSizePBox,   0, wxALIGN_CENTER | wxALL, 1);
	m_pFMRISizer->Add( pBoxRow7, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

	m_pTextAlpha = new wxStaticText( this, wxID_ANY, wxT("Alpha blend"), wxDefaultPosition, wxSize(70, -1), wxALIGN_CENTER );
	m_pSliderAlpha = new MySlider( this, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxSize(100, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
	m_pSliderAlpha->SetValue( 50 );
	Connect( m_pSliderAlpha->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(FMRIWindow::OnSliderAlphaMoved) );
    m_pTxtAlphaBox = new wxTextCtrl( this, wxID_ANY, wxT("0.5"), wxDefaultPosition, wxSize(55, -1), wxTE_CENTRE | wxTE_READONLY );

	wxBoxSizer *pBoxRow8 = new wxBoxSizer( wxHORIZONTAL );
    pBoxRow8->Add( m_pTextAlpha, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pBoxRow8->Add( m_pSliderAlpha,   0, wxALIGN_CENTER | wxEXPAND | wxALL, 1);
	pBoxRow8->Add( m_pTxtAlphaBox,   0, wxALIGN_CENTER | wxALL, 1);
	m_pFMRISizer->Add( pBoxRow8, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

	m_pBtnConvertFMRI = new wxButton( this, wxID_ANY,wxT("Generate map"), wxDefaultPosition, wxSize(230, -1) );
	Connect( m_pBtnConvertFMRI->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(FMRIWindow::onConvertRestingState) );

    m_pBtnGenerateClusters = new wxButton( this, wxID_ANY,wxT("Generate clusters"), wxDefaultPosition, wxSize(230, -1) );
	Connect( m_pBtnGenerateClusters->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(FMRIWindow::onGenerateClusters) );

    wxBoxSizer *pBoxRow9 = new wxBoxSizer( wxVERTICAL );
	pBoxRow9->Add( m_pBtnConvertFMRI,   0, wxALIGN_CENTER | wxALL, 1);
    pBoxRow9->Add( m_pBtnGenerateClusters,   0, wxALIGN_CENTER | wxALL, 1);
	m_pFMRISizer->Add( pBoxRow9, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

}

void FMRIWindow::OnSize( wxSizeEvent &WXUNUSED(event) )
{
	
}

void FMRIWindow::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
    wxPaintDC dc( this );
}

wxSizer* FMRIWindow::getWindowSizer()
{
    return m_pFMRISizer;
}

void FMRIWindow::SetSelectButton()
{
	DatasetIndex indx = DatasetManager::getInstance()->m_pRestingStateNetwork->getIndex();
	Anatomy* pNewAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( indx );
	m_pBtnSelectFMRI->SetLabel( pNewAnatomy->getName() );
    m_pBtnSelectFMRI->SetBackgroundColour(wxNullColour);
	
	//m_pSliderRest->Enable();
	//Set slider max value according to number of timelaps
	m_pSliderRest->SetMax((int)DatasetManager::getInstance()->m_pRestingStateNetwork->getBands()-1);

	m_pRadShowRawData->Enable();
	m_pRadShowNetwork->Enable();
	m_pRadShowNetwork->SetValue(true);	
}

void FMRIWindow::onSwitchViewRaw( wxCommandEvent& WXUNUSED(event) )
{
	showRawData = true;
	m_pSliderRest->Enable();
	m_pTextVolumeId->Enable();
	m_pTxtRestBox->Enable();

	int sliderValue = m_pSliderRest->GetValue();
    m_pTxtRestBox->SetValue( wxString::Format( wxT( "%i"), sliderValue ) );
	DatasetManager::getInstance()->m_pRestingStateNetwork->SetTextureFromSlider( sliderValue );
}

void FMRIWindow::onSwitchViewNet( wxCommandEvent& WXUNUSED(event) )
{
	showRawData = false;
	m_pSliderRest->Disable();
	m_pTextVolumeId->Disable();
	m_pTxtRestBox->Disable();
	DatasetManager::getInstance()->m_pRestingStateNetwork->SetTextureFromNetwork();
}
void FMRIWindow::OnSliderRestMoved( wxCommandEvent& WXUNUSED(event) )
{
	int sliderValue = m_pSliderRest->GetValue();
    m_pTxtRestBox->SetValue( wxString::Format( wxT( "%i"), sliderValue ) );
	DatasetManager::getInstance()->m_pRestingStateNetwork->SetTextureFromSlider( sliderValue );
}

void FMRIWindow::OnSliderCorrThreshMoved(  wxCommandEvent& WXUNUSED(event) )
{
	float sliderValue = m_pSliderCorrThreshold->GetValue() / 100.0f;
    m_pTxtCorrThreshBox->SetValue( wxString::Format( wxT( "%.2f"), sliderValue ) );
	DatasetManager::getInstance()->m_pRestingStateNetwork->SetCorrThreshold( sliderValue );
	RTFMRIHelper::getInstance()->setRTFMRIDirty( true );
}

void FMRIWindow::OnSliderClusterLevelMoved(  wxCommandEvent& WXUNUSED(event) )
{
	float sliderValue = m_pSliderClusterLvl->GetValue();
	m_pTxtClusterLvlBox->SetValue( wxString::Format( wxT( "%.2f"), sliderValue ) );
	DatasetManager::getInstance()->m_pRestingStateNetwork->SetClusterLvlSliderValue( sliderValue );
	RTFMRIHelper::getInstance()->setRTFMRIDirty( true );
}

void FMRIWindow::OnSliderSizePMoved(  wxCommandEvent& WXUNUSED(event) )
{
	float sliderValue = m_pSliderSizeP->GetValue();
	m_pTxtSizePBox->SetValue( wxString::Format( wxT( "%.1f"), sliderValue ) );
	DatasetManager::getInstance()->m_pRestingStateNetwork->SetSizePSliderValue( sliderValue );
	RTFMRIHelper::getInstance()->setRTFMRIDirty( true );
}


void FMRIWindow::OnSliderAlphaMoved(  wxCommandEvent& WXUNUSED(event) )
{
	float sliderValue = m_pSliderAlpha->GetValue() / 100.0f;
	m_pTxtAlphaBox->SetValue( wxString::Format( wxT( "%.2f"), sliderValue ) );
	DatasetManager::getInstance()->m_pRestingStateNetwork->SetAlphaSliderValue( sliderValue );
	RTFMRIHelper::getInstance()->setRTFMRIDirty( true );
}

void FMRIWindow::onConvertRestingState( wxCommandEvent& WXUNUSED(event) )
{
	//Convert to anat
	DatasetManager::getInstance()->m_pRestingStateNetwork->seedBased();

	std::vector<float>* data = DatasetManager::getInstance()->m_pRestingStateNetwork->getZscores();
	int indx = DatasetManager::getInstance()->createAnatomy( data, OVERLAY );
    
	Anatomy* pNewAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( indx );
    pNewAnatomy->setShowFS(false);

    pNewAnatomy->setType(OVERLAY);
    pNewAnatomy->setDataType(16);
	pNewAnatomy->setShowFS(true);
    pNewAnatomy->setName( wxT("Z-score map") );
	pNewAnatomy->setThreshold( 0.01f );
    MyApp::frame->m_pListCtrl->InsertItem( indx );

	RTFMRIHelper::getInstance()->setRTFMRIReady(false);

	DatasetManager::getInstance()->m_pRestingStateNetwork->clear3DPoints();
	RTFMRIHelper::getInstance()->setRTFMRIDirty( false );
    m_pBtnStart->SetLabel(wxT("Start correlation"));
    m_pBtnStart->SetValue(false);

}

void FMRIWindow::onGenerateClusters( wxCommandEvent& WXUNUSED(event) )
{
	//Convert to anat each cluster inside the zmap
	DatasetManager::getInstance()->m_pRestingStateNetwork->seedBased();

    std::vector<std::vector<float>* > clusters = DatasetManager::getInstance()->m_pRestingStateNetwork->getClusters();
    for(unsigned int i=0; i < clusters.size(); i++)
    {
	    int indx = DatasetManager::getInstance()->createAnatomy( clusters[i], OVERLAY );
    
	    Anatomy* pNewAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( indx );
        pNewAnatomy->setShowFS(false);

        pNewAnatomy->setType(OVERLAY);
        pNewAnatomy->setDataType(16);
	    pNewAnatomy->setShowFS(true);
        pNewAnatomy->setName( wxString::Format ( wxT("Cluster %u"), i ));
	    pNewAnatomy->setThreshold( 0.01f );
        MyApp::frame->m_pListCtrl->InsertItem( indx );
    }

	RTFMRIHelper::getInstance()->setRTFMRIReady(false);

	DatasetManager::getInstance()->m_pRestingStateNetwork->clear3DPoints();
	RTFMRIHelper::getInstance()->setRTFMRIDirty( false );
    m_pBtnStart->SetLabel(wxT("Start correlation"));
    m_pBtnStart->SetValue(false);

}

void FMRIWindow::OnStartRTFMRI( wxCommandEvent& WXUNUSED(event) )
{
	RTFMRIHelper::getInstance()->toggleRTFMRIReady();
    RTFMRIHelper::getInstance()->setRTFMRIDirty( true );

    if( !RTFMRIHelper::getInstance()->isRTFMRIReady() )
    {
		DatasetManager::getInstance()->m_pRestingStateNetwork->clear3DPoints();
        RTFMRIHelper::getInstance()->setRTFMRIDirty( false );
        m_pBtnStart->SetLabel(wxT("Start correlation"));
    }
    else
    {
        m_pBtnStart->SetLabel(wxT("Stop correlation"));
	}
}

