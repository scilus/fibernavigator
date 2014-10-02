#include "DatasetInfo.h"

#include "../main.h"
#include "../gui/MainFrame.h"

#include <wx/filename.h>
#include <wx/tglbtn.h>
#include <wx/xml/xml.h>

#include <algorithm>

DatasetInfo::DatasetInfo()
:   m_length       ( 0 ),
    m_bands        ( 0 ),
    m_frames       ( 1 ),
    m_rows         ( 1 ),
    m_columns      ( 1 ),
    m_type         ( BOT_INITIALIZED ),
    m_repn         ( _T( "" ) ),
    m_isLoaded     ( false ),
    m_highest_value( 1.0 ),

    m_name         ( _T( "" ) ),
    m_fullPath     ( _T( "" ) ),

    m_threshold    ( 0.0f ),
    m_alpha        ( 1.0f ),
    m_brightness   ( 1.0f ),
    m_oldMax       ( 1.0 ),
    m_newMax       ( 1.0 ),

    m_color        ( wxColour( 128, 10, 10 ) ),
    m_GLuint       ( 0 ),

    m_show         ( true ),
    m_showFS       ( true ),
    m_useTex       ( true ),

    m_isGlyph      ( false ),
    m_bufferObjects( 0 ),

    m_voxelSizeX( 0.0f ),
    m_voxelSizeY( 0.0f ),
    m_voxelSizeZ( 0.0f )
{
}

void DatasetInfo::createPropertiesSizer( PropertiesWindow *pParent )
{
    SceneObject::createPropertiesSizer( pParent );

    wxBoxSizer *pBoxMain = new wxBoxSizer( wxVERTICAL );

    //////////////////////////////////////////////////////////////////////////

    m_pTxtName = new wxTextCtrl( pParent, wxID_ANY, getName(), wxDefaultPosition, wxSize( 180, -1 ), wxTE_CENTER | wxTE_READONLY );
    m_pTxtName->SetBackgroundColour( *wxLIGHT_GREY );
    wxFont font = m_pTxtName->GetFont();
    font.SetPointSize( 10 );
    font.SetWeight( wxFONTWEIGHT_BOLD );
    m_pTxtName->SetFont( font );
    pBoxMain->Add( m_pTxtName, 0, wxEXPAND | wxALL, 1 );

    //////////////////////////////////////////////////////////////////////////

    wxBoxSizer *pBoxMove = new wxBoxSizer( wxHORIZONTAL );
    wxImage bmpUp    ( MyApp::iconsPath + wxT( "view4.png" ),  wxBITMAP_TYPE_PNG );
    wxImage bmpDown  ( MyApp::iconsPath + wxT( "view2.png" ),  wxBITMAP_TYPE_PNG );
    wxImage bmpDelete( MyApp::iconsPath + wxT( "delete.png" ), wxBITMAP_TYPE_PNG );
    m_pBtnUp     = new wxBitmapButton( pParent, wxID_ANY, bmpUp,     wxDefaultPosition, wxSize( 60, -1 ) );
    m_pBtnDown   = new wxBitmapButton( pParent, wxID_ANY, bmpDown,   wxDefaultPosition, wxSize( 60, -1 ) );
    m_pBtnDelete = new wxBitmapButton( pParent, wxID_ANY, bmpDelete, wxDefaultPosition, wxSize( 60, -1 ) );
    pBoxMove->Add( m_pBtnUp,     1, wxALIGN_CENTER | wxEXPAND | wxALL, 1 );
    pBoxMove->Add( m_pBtnDown,   1, wxALIGN_CENTER | wxEXPAND | wxALL, 1 );
    pBoxMove->Add( m_pBtnDelete, 1, wxALIGN_CENTER | wxEXPAND | wxALL, 1 );
    pBoxMain->Add( pBoxMove,     0, wxEXPAND | wxALL,                  0 );

    //////////////////////////////////////////////////////////////////////////

    m_pBtnRename = new wxButton( pParent, wxID_ANY, wxT( "Rename" ), wxDefaultPosition, wxSize( 90, -1 ) );
    pBoxMain->Add( m_pBtnRename, 0, wxALIGN_CENTER | wxEXPAND | wxRIGHT | wxLEFT, 24 );

    //////////////////////////////////////////////////////////////////////////

    wxBoxSizer *pBoxFlips = new wxBoxSizer( wxHORIZONTAL );
    m_pBtnFlipX = new wxToggleButton( pParent, wxID_ANY, wxT( "Flip X" ), wxDefaultPosition, wxSize( 60, -1 ) );
    m_pBtnFlipY = new wxToggleButton( pParent, wxID_ANY, wxT( "Flip Y" ), wxDefaultPosition, wxSize( 60, -1 ) );
    m_pBtnFlipZ = new wxToggleButton( pParent, wxID_ANY, wxT( "Flip Z" ), wxDefaultPosition, wxSize( 60, -1 ) );
    pBoxFlips->Add( m_pBtnFlipX, 1, wxALIGN_CENTER | wxEXPAND | wxALL, 1 );
    pBoxFlips->Add( m_pBtnFlipY, 1, wxALIGN_CENTER | wxEXPAND | wxALL, 1 );
    pBoxFlips->Add( m_pBtnFlipZ, 1, wxALIGN_CENTER | wxEXPAND | wxALL, 1 );
    pBoxMain->Add(  pBoxFlips,   0, wxEXPAND | wxALL,                  0 );

    //////////////////////////////////////////////////////////////////////////

    wxBoxSizer *pBoxVisible = new wxBoxSizer( wxHORIZONTAL );
    m_pToggleVisibility = new wxToggleButton( pParent, wxID_ANY, wxT( "Visible" ),       wxDefaultPosition, wxSize( 90, -1 ) );
    m_pToggleFiltering  = new wxToggleButton( pParent, wxID_ANY, wxT( "Interpolation" ), wxDefaultPosition, wxSize( 90, -1 ) );
    pBoxVisible->Add( m_pToggleVisibility, 1, wxALIGN_CENTER | wxEXPAND | wxALL, 1 );
    pBoxVisible->Add( m_pToggleFiltering,  1, wxALIGN_CENTER | wxEXPAND | wxALL, 1 );
    pBoxMain->Add( pBoxVisible, 0, wxALL | wxEXPAND, 0 );

    //////////////////////////////////////////////////////////////////////////
    
    wxFlexGridSizer *pGridSliders = new wxFlexGridSizer( 2 );
    m_pSliderThresholdIntensity = new MySlider( pParent, wxID_ANY,(int)( getThreshold() * 100 ), 0, 100, wxDefaultPosition, wxSize( 160, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pIntensityText = new wxStaticText( pParent, wxID_ANY, wxT( "Intensity" ) );
    pGridSliders->Add( m_pIntensityText,            0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderThresholdIntensity, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL,    1 );

    m_pSliderOpacity = new MySlider( pParent, wxID_ANY, (int)( getAlpha() * 100 ), 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pOpacityText = new wxStaticText( pParent, wxID_ANY, wxT( "Opacity" ) );
    pGridSliders->Add( m_pOpacityText,   0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderOpacity, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL,    1 );

    pBoxMain->Add( pGridSliders, 0, wxEXPAND | wxALL, 2 );

    //////////////////////////////////////////////////////////////////////////

    m_pPropertiesSizer->Add( pBoxMain, 1, wxFIXED_MINSIZE | wxEXPAND, 0 );

    //////////////////////////////////////////////////////////////////////////
    // Connect widgets to callback function
    pParent->Connect( m_pBtnUp->GetId(),                    wxEVT_COMMAND_BUTTON_CLICKED,       wxCommandEventHandler( PropertiesWindow::OnListItemUp ) );
    pParent->Connect( m_pBtnDown->GetId(),                  wxEVT_COMMAND_BUTTON_CLICKED,       wxCommandEventHandler( PropertiesWindow::OnListItemDown ) );
    pParent->Connect( m_pBtnDelete->GetId(),                wxEVT_COMMAND_BUTTON_CLICKED,       wxEventHandler(        PropertiesWindow::OnDeleteListItem ) );
    pParent->Connect( m_pBtnRename->GetId(),                wxEVT_COMMAND_BUTTON_CLICKED,       wxCommandEventHandler( PropertiesWindow::OnRename ) );
    pParent->Connect( m_pBtnFlipX->GetId(),                 wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnFlipX ) );
    pParent->Connect( m_pBtnFlipY->GetId(),                 wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnFlipY ) );
    pParent->Connect( m_pBtnFlipZ->GetId(),                 wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnFlipZ ) );
    pParent->Connect( m_pToggleVisibility->GetId(),         wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnToggleVisibility ) );
    pParent->Connect( m_pToggleFiltering->GetId(),          wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler(        PropertiesWindow::OnToggleShowFS ) );
    pParent->Connect( m_pSliderThresholdIntensity->GetId(), wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnSliderIntensityThresholdMoved ) );
    pParent->Connect( m_pSliderOpacity->GetId(),            wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnSliderOpacityThresholdMoved ) );


    /*l_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_pbtnSmoothLoop = new wxButton(parent, wxID_ANY,wxT("Smooth"),wxDefaultPosition, wxSize(60,-1));
    l_sizer->Add(m_pbtnSmoothLoop,0,wxALIGN_CENTER);
    m_pbtnSmoothLoop->Enable(getType() == MESH ); // || getType() == MESH || getType() == SURFACE
    m_pbtnClean = new wxButton(parent, wxID_ANY,wxT("Clean"),wxDefaultPosition, wxSize(60,-1));
    l_sizer->Add(m_pbtnClean,0,wxALIGN_CENTER);
    m_pbtnClean->Enable(getType() == MESH ); //|| getType() == ISO_SURFACE
    m_ptoggleLIC = new wxToggleButton(parent, wxID_ANY,wxT("LIC"),wxDefaultPosition, wxSize(60,-1));
    l_sizer->Add(m_ptoggleLIC,0,wxALIGN_CENTER);
    m_ptoggleLIC->Enable(m_dh->m_vectorsLoaded && (getType() == SURFACE || getType() == ISO_SURFACE));
    m_pPropertiesSizer->Add(l_sizer,0,wxALIGN_CENTER);
    //parent->Connect(m_pbtnSmoothLoop->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnLoop)); 
    //parent->Connect(m_pbtnClean->GetId(),wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnClean));  
    //parent->Connect(m_ptoggleLIC->GetId(),wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler(PropertiesWindow::OnToggleLIC));*/     
}

void DatasetInfo::updatePropertiesSizer()
{
    SceneObject::updatePropertiesSizer();
    m_pToggleVisibility->SetValue(getShow());
    m_pToggleFiltering->SetValue(getShowFS());
    //m_ptoggleLIC->SetValue(getUseLIC());
    
}

//////////////////////////////////////////////////////////////////////////

bool DatasetInfo::save( wxXmlNode *pNode, const wxString &rootPath ) const
{
    assert( pNode != NULL );

    wxXmlNode *pStatus = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "status" ) );
    wxXmlNode *pPath   = new wxXmlNode( NULL, wxXML_ELEMENT_NODE, wxT( "path" ) );

    pNode->AddChild( pStatus );
    pNode->AddChild( pPath );

    pStatus->AddAttribute( new wxXmlAttribute( wxT( "name" ), m_name ) );
    pStatus->AddAttribute( new wxXmlAttribute( wxT( "active" ), m_show ? wxT( "yes" ) : wxT( "no" ) ) );
    pStatus->AddAttribute( new wxXmlAttribute( wxT( "showFS" ), m_showFS ? wxT( "yes" ) : wxT( "no" ) ) );
    pStatus->AddAttribute( new wxXmlAttribute( wxT( "alpha" ), wxString::Format( wxT( "%.2f" ), m_alpha ) ) );
    pStatus->AddAttribute( new wxXmlAttribute( wxT( "threshold" ), wxString::Format( wxT( "%.2f" ), m_threshold ) ) );
    pStatus->AddAttribute( new wxXmlAttribute( wxT( "useTex" ), m_useTex ? wxT( "yes" ) : wxT( "no" ) ) );
    pStatus->AddAttribute( new wxXmlAttribute( wxT( "isFiberGroup" ), wxT( "no" ) ) );

    wxFileName tempName( m_fullPath );
    tempName.MakeRelativeTo( rootPath );
    
    pPath->AddChild( new wxXmlNode( NULL, wxXML_TEXT_NODE, wxT( "path"), tempName.GetFullPath() ) );

    return true;
}

//////////////////////////////////////////////////////////////////////////

void DatasetInfo::swap( DatasetInfo &d )
{
    // Not swapping GUI elements
    SceneObject::swap( d );
    std::swap( m_tMesh, d.m_tMesh );
    std::swap( m_length, d.m_length );
    std::swap( m_bands, d.m_bands );
    std::swap( m_frames, d.m_frames );
    std::swap( m_rows, d.m_rows );
    std::swap( m_columns, d.m_columns );
    std::swap( m_type, d.m_type );
    std::swap( m_repn, d.m_repn );
    std::swap( m_isLoaded, d.m_isLoaded );
    std::swap( m_highest_value, d.m_highest_value );
    std::swap( m_name, d.m_name );
    std::swap( m_fullPath, d.m_fullPath );
    std::swap( m_threshold, d.m_threshold );
    std::swap( m_alpha, d.m_alpha );
    std::swap( m_brightness, d.m_brightness );
    std::swap( m_oldMax, d.m_oldMax );
    std::swap( m_newMax, d.m_newMax );
    std::swap( m_color, d.m_color );
    std::swap( m_GLuint, d.m_GLuint );
    std::swap( m_show, d.m_show );
    std::swap( m_showFS, d.m_showFS );
    std::swap( m_useTex, d.m_useTex );
    std::swap( m_isGlyph, d.m_isGlyph );
    std::swap( m_bufferObjects, d.m_bufferObjects );
    std::swap( m_voxelSizeX, d.m_voxelSizeX );
    std::swap( m_voxelSizeY, d.m_voxelSizeY );
    std::swap( m_voxelSizeZ, d.m_voxelSizeZ );
}

//////////////////////////////////////////////////////////////////////////

DatasetInfo::~DatasetInfo()
{
}