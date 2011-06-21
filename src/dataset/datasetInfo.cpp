#include "datasetInfo.h"
#include "../gui/mainFrame.h"
#include "../main.h"

DatasetInfo::DatasetInfo( DatasetHelper* datasetHelper ) : 
            m_dh              ( datasetHelper ),
            m_length          ( 0 ),
            m_bands           ( 0 ),
            m_frames          ( 0 ),
            m_rows            ( 0 ),
            m_columns         ( 0 ),
            m_type            ( BOT_INITIALIZED ),
            m_repn            ( _T( "" ) ),
            m_isLoaded        ( false ),
            m_highest_value   ( 1.0 ),

            m_name            ( _T( "" ) ),
            m_fullPath        ( _T( "" ) ),

            m_threshold       ( 0.0f ),
            m_alpha           ( 1.0f ),
            m_brightness      ( 1.0f ),
            m_oldMax          ( 1.0 ),
            m_newMax          ( 1.0 ),

            m_color           ( wxColour( 128, 10, 10 ) ),
            m_GLuint          ( 0 ),

            m_show            ( true ),
            m_showFS          ( true ),
            m_useTex          ( true ),

            m_isGlyph         ( false ),

            licCalculated     ( false ),
            m_useLIC          ( false ),
            m_bufferObjects   ( 0 )
{

}

void DatasetInfo::createPropertiesSizer(MainFrame *parent)
{
    FNObject::createPropertiesSizer(parent);
    wxBoxSizer *l_sizer;

    m_ptxtName = new wxTextCtrl(parent, wxID_ANY, getName(),wxDefaultPosition, wxSize(180,-1), wxTE_CENTRE | wxTE_READONLY);    
    m_ptxtName->SetBackgroundColour(*wxLIGHT_GREY);
    wxFont l_font = m_ptxtName->GetFont();
    l_font.SetPointSize(10);
    l_font.SetWeight(wxFONTWEIGHT_BOLD);
    m_ptxtName->SetFont(l_font);
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(m_ptxtName,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    
    wxImage bmpDelete(MyApp::iconsPath+ wxT("delete.png" ), wxBITMAP_TYPE_PNG);
    wxImage bmpDown(MyApp::iconsPath+ wxT("view2.png" ), wxBITMAP_TYPE_PNG);
    wxImage bmpUp(MyApp::iconsPath+ wxT("view4.png" ), wxBITMAP_TYPE_PNG);
    m_pbtnDelete = new wxBitmapButton(parent, wxID_ANY, bmpDelete, wxDefaultPosition, wxSize(60,-1));
    m_pbtnUp = new wxBitmapButton(parent, wxID_ANY, bmpUp, wxDefaultPosition, wxSize(60,-1));
    m_pbtnDown = new wxBitmapButton(parent, wxID_ANY, bmpDown, wxDefaultPosition, wxSize(60,-1));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);    
    l_sizer->Add(m_pbtnUp,0,wxALIGN_CENTER);
    l_sizer->Add(m_pbtnDown,0,wxALIGN_CENTER);
    l_sizer->Add(m_pbtnDelete,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_pbtnDown->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnListItemDown));
    parent->Connect(m_pbtnUp->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnListItemUp));
    parent->Connect(m_pbtnDelete->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxEventHandler(MainFrame::OnDeleteListItem));
    
    m_ptoggleVisibility = new wxToggleButton(parent, wxID_ANY, wxT("Visible"),wxDefaultPosition, wxSize(90,-1));
    m_ptoggleFiltering = new wxToggleButton(parent, wxID_ANY, wxT("Interpolation"),wxDefaultPosition, wxSize(90,-1));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(m_ptoggleVisibility,0,wxALIGN_CENTER);
    l_sizer->Add(m_ptoggleFiltering,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_ptoggleVisibility->GetId(),wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnListMenuShow));   
    parent->Connect(m_ptoggleFiltering->GetId(),wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler(MainFrame::OnToggleShowFS));  
    
    m_psliderThresholdIntensity = new MySlider(parent, wxID_ANY,0,0,100, wxDefaultPosition, wxSize(140,-1), wxSL_HORIZONTAL | wxSL_AUTOTICKS);
    m_psliderThresholdIntensity->SetValue((int)(getThreshold()*100));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Intensity "),wxDefaultPosition, wxSize(60,-1), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(m_psliderThresholdIntensity,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_psliderThresholdIntensity->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(MainFrame::OnSliderIntensityThresholdMoved));

    m_psliderOpacity = new MySlider(parent, wxID_ANY,0,0,100, wxDefaultPosition, wxSize(140,-1), wxSL_HORIZONTAL | wxSL_AUTOTICKS);
    m_psliderOpacity->SetValue((int)(getAlpha()*100));
    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    l_sizer->Add(new wxStaticText(parent, wxID_ANY, wxT("Opacity "),wxDefaultPosition, wxSize(60,-1), wxALIGN_RIGHT),0,wxALIGN_CENTER);
    l_sizer->Add(m_psliderOpacity,0,wxALIGN_CENTER);
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_psliderOpacity->GetId(),wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(MainFrame::OnSliderOpacityThresholdMoved));

    l_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_pbtnSmoothLoop = new wxButton(parent, wxID_ANY,wxT("Smooth"),wxDefaultPosition, wxSize(60,-1));
    l_sizer->Add(m_pbtnSmoothLoop,0,wxALIGN_CENTER);
    m_pbtnSmoothLoop->Enable(getType() == MESH ); // || getType() == MESH || getType() == SURFACE
    m_pbtnClean = new wxButton(parent, wxID_ANY,wxT("Clean"),wxDefaultPosition, wxSize(60,-1));
    l_sizer->Add(m_pbtnClean,0,wxALIGN_CENTER);
    m_pbtnClean->Enable(getType() == MESH ); //|| getType() == ISO_SURFACE
    m_ptoggleLIC = new wxToggleButton(parent, wxID_ANY,wxT("LIC"),wxDefaultPosition, wxSize(60,-1));
    l_sizer->Add(m_ptoggleLIC,0,wxALIGN_CENTER);
    m_ptoggleLIC->Enable(m_dh->m_vectorsLoaded && (getType() == SURFACE || getType() == ISO_SURFACE));
    m_propertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    parent->Connect(m_pbtnSmoothLoop->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnLoop)); 
    parent->Connect(m_pbtnClean->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnClean));  
    parent->Connect(m_ptoggleLIC->GetId(),wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnToggleLIC)); 	
}

void DatasetInfo::updatePropertiesSizer()
{
    FNObject::updatePropertiesSizer();
    m_ptoggleVisibility->SetValue(getShow());
    m_ptoggleFiltering->SetValue(getShowFS());
    m_ptoggleLIC->SetValue(getUseLIC());
	
}
