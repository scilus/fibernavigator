/*
 *  The FibersGroup class implementation.
 *
 */

#include "FibersGroup.h"

#include <iostream>
#include <fstream>
#include <cfloat>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <wx/tokenzr.h>

#include "../main.h"

FibersGroup::FibersGroup( DatasetHelper *pDatasetHelper )
	: DatasetInfo( pDatasetHelper )
{
}

FibersGroup::~FibersGroup()
{
    m_dh->printDebug( _T( "executing FibersGroup destructor" ), 1 );
}

void FibersGroup::addFibersSet(Fibers* pFibers)
{
	m_fibersSets.push_back(pFibers);
}

Fibers* FibersGroup::getFibersSet(int num)
{
	Fibers* pFibers = NULL;
	if( (int)m_fibersSets.size() > num)
	{
		pFibers = m_fibersSets[num];
	}
	return pFibers;
}

void FibersGroup::updateGroupFilters()
{
    /*int min = m_pSliderFibersFilterMin->GetValue();
    int max = m_pSliderFibersFilterMax->GetValue();
    int subSampling = m_pSliderFibersSampling->GetValue();
    int maxSubSampling = m_pSliderFibersSampling->GetMax() + 1;

    for( int i = 0; i < m_countLines; ++i )
    {
        m_filtered[i] = !( ( i % maxSubSampling ) >= subSampling && m_length[i] >= min && m_length[i] <= max );
    }*/
}

void FibersGroup::createPropertiesSizer( PropertiesWindow *pParent )
{
    DatasetInfo::createPropertiesSizer( pParent );
    
	wxSizer *pSizer;

    m_ptoggleIntensity = new wxToggleButton(pParent, wxID_ANY, wxT("Intensity"),wxDefaultPosition, wxSize(90,-1));
    pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_ptoggleIntensity,0,wxALIGN_LEFT);
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_ptoggleIntensity->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleIntensityBtn ) );

	m_ptoggleOpacity = new wxToggleButton(pParent, wxID_ANY, wxT("Opacity"),wxDefaultPosition, wxSize(90,-1));
    pSizer = new wxBoxSizer(wxHORIZONTAL);
	pSizer->Add(m_ptoggleOpacity,0,wxALIGN_LEFT);
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
	pParent->Connect( m_ptoggleOpacity->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleOpacityBtn ) );

	m_ptoggleMinMaxLength = new wxToggleButton(pParent, wxID_ANY, wxT("Min / Max Length"),wxDefaultPosition, wxSize(90,-1));
    pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_ptoggleMinMaxLength,0,wxALIGN_LEFT);
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
	pParent->Connect( m_ptoggleMinMaxLength->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleMinMaxLengthBtn ) );

	m_ptoggleSubsampling = new wxToggleButton(pParent, wxID_ANY, wxT("Subsampling"),wxDefaultPosition, wxSize(90,-1));
	pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_ptoggleSubsampling,0,wxALIGN_LEFT);
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
	pParent->Connect( m_ptoggleSubsampling->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleSubsamplingBtn ) );

	m_ptoggleColorMode = new wxToggleButton(pParent, wxID_ANY, wxT("ColorMode"),wxDefaultPosition, wxSize(90,-1));
	pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_ptoggleColorMode,0,wxALIGN_LEFT);
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
	pParent->Connect( m_ptoggleColorMode->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleColorModeBtn ) );
    
    /*pSizer = new wxBoxSizer( wxHORIZONTAL );
    m_pGeneratesFibersDensityVolume = new wxButton( pParent, wxID_ANY, wxT( "New Density Volume" ), wxDefaultPosition, wxSize( 140, -1 ) );
    pSizer->Add( m_pGeneratesFibersDensityVolume, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pGeneratesFibersDensityVolume->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnGenerateFiberVolume ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    m_pToggleLocalColoring = new wxToggleButton( pParent, wxID_ANY, wxT( "Local Coloring" ), wxDefaultPosition, wxSize( 140, -1 ) );
    pSizer->Add( m_pToggleLocalColoring, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pToggleLocalColoring->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnListMenuThreshold ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    m_pToggleNormalColoring = new wxToggleButton( pParent, wxID_ANY, wxT( "Color With Overley" ), wxDefaultPosition, wxSize( 140, -1 ) );
    pSizer->Add( m_pToggleNormalColoring, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pToggleNormalColoring->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleShowFS ) );
    
    m_propertiesSizer->AddSpacer( 8 );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( new wxStaticText( pParent, wxID_ANY, _T( "Coloring" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT ), 0, wxALIGN_CENTER );
    pSizer->Add( 8, 1, 0 );
    m_pRadioNormalColoring = new wxRadioButton( pParent, wxID_ANY, _T( "Normal" ), wxDefaultPosition, wxSize( 132, -1 ) );
    pSizer->Add( m_pRadioNormalColoring );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    m_pRadioDistanceAnchoring  = new wxRadioButton( pParent, wxID_ANY, _T( "Dist. Anchoring" ), wxDefaultPosition, wxSize( 132, -1 ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( 68, 1, 0 );
    pSizer->Add( m_pRadioDistanceAnchoring );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    m_pRadioMinDistanceAnchoring  = new wxRadioButton( pParent, wxID_ANY, _T( "Min Dist. Anchoring" ), wxDefaultPosition, wxSize( 132, -1 ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( 68, 1, 0 );
    pSizer->Add( m_pRadioMinDistanceAnchoring );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    m_pRadioCurvature  = new wxRadioButton( pParent, wxID_ANY, _T( "Curvature" ), wxDefaultPosition, wxSize( 132, -1 ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( 68, 1, 0 );
    pSizer->Add( m_pRadioCurvature );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    m_pRadioTorsion  = new wxRadioButton( pParent, wxID_ANY, _T( "Torsion" ), wxDefaultPosition, wxSize( 132, -1 ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( 68, 1, 0 );
    pSizer->Add( m_pRadioTorsion );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pRadioNormalColoring->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnNormalColoring ) );
    pParent->Connect( m_pRadioDistanceAnchoring->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnListMenuDistance ) );
    pParent->Connect( m_pRadioMinDistanceAnchoring->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnListMenuMinDistance ) );
    pParent->Connect( m_pRadioTorsion->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnColorWithTorsion ) );
    pParent->Connect( m_pRadioCurvature->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnColorWithCurvature ) );
    m_pRadioNormalColoring->SetValue( m_dh->m_fiberColorationMode == NORMAL_COLOR );*/
}

void FibersGroup::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();

	// Hide items that are not required for fibersGroup
	DatasetInfo::m_ptoggleVisibility->Hide();
	DatasetInfo::m_ptoggleFiltering->Hide();
	DatasetInfo::m_pbtnDelete->Hide();
	DatasetInfo::m_pbtnDown->Hide();
	DatasetInfo::m_pbtnUp->Hide();
	DatasetInfo::m_pIntensityText->Hide();
	DatasetInfo::m_psliderThresholdIntensity->Hide();
	DatasetInfo::m_pOpacityText->Hide();
	DatasetInfo::m_psliderOpacity->Hide();
    
	/*m_ptoggleFiltering->Enable( false );
    m_ptoggleFiltering->SetValue( false );
    m_psliderOpacity->SetValue( m_psliderOpacity->GetMin() );
    m_psliderOpacity->Enable( false );
    m_pToggleNormalColoring->SetValue( !getShowFS() );
    m_pRadioNormalColoring->Enable( getShowFS() );
    m_pRadioCurvature->Enable( getShowFS() );
    m_pRadioDistanceAnchoring->Enable( getShowFS() );
    m_pRadioMinDistanceAnchoring->Enable( getShowFS() );
    m_pRadioTorsion->Enable( getShowFS() );*/
}
