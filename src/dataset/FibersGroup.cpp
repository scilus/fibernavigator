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
	m_fibersCount = 0;
	m_isIntensityToggled = false;
	m_isOpacityToggled = false;
	m_isMinMaxLengthToggled = false;
	m_isSubsamplingToggled = false;
	m_isColorModeToggled = false;
}

FibersGroup::~FibersGroup()
{
    m_dh->printDebug( _T( "executing FibersGroup destructor" ), 1 );
}

void FibersGroup::addFibersSet(Fibers* pFibers)
{
	m_fibersSets.push_back(pFibers);
	m_fibersCount++;
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

    pSizer = new wxBoxSizer( wxHORIZONTAL );
	m_pSliderFibersFilterMin = new wxSlider( pParent, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxSize( 140, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
	m_pMinLengthText = new wxStaticText( pParent, wxID_ANY , wxT( "Min Length" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_CENTRE );
    pSizer->Add( m_pMinLengthText , 0, wxALIGN_CENTER );
    pSizer->Add( m_pSliderFibersFilterMin, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pSliderFibersFilterMin->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
	m_pSliderFibersFilterMax = new wxSlider( pParent, wxID_ANY, 100, 0, 100, wxDefaultPosition, wxSize( 140, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
	m_pMaxLengthText = new wxStaticText( pParent, wxID_ANY , wxT( "Max Length" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_CENTRE );
    pSizer->Add( m_pMaxLengthText , 0, wxALIGN_CENTER );
    pSizer->Add( m_pSliderFibersFilterMax, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pSliderFibersFilterMax->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    m_pSliderFibersSampling = new wxSlider( pParent, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxSize( 140, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
	m_pSubsamplingText = new wxStaticText( pParent, wxID_ANY , wxT( "Subsampling" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_CENTRE );
	pSizer->Add( m_pSubsamplingText , 0, wxALIGN_CENTER );
    pSizer->Add( m_pSliderFibersSampling, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pSliderFibersSampling->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );

	m_pApplyBtn = new wxButton(pParent, wxID_ANY, wxT("Apply"), wxDefaultPosition, wxSize(90, -1));
	m_pCancelBtn = new wxButton(pParent, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxSize(90, -1));
	pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_pApplyBtn,0, wxALIGN_CENTER);
	pSizer->Add(m_pCancelBtn, 0, wxALIGN_CENTER);
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
	pParent->Connect( m_pApplyBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnClickApplyBtn ) );
	pParent->Connect( m_pCancelBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnClickCancelBtn ) );

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

void FibersGroup::OnToggleIntensityBtn()
{
	m_isIntensityToggled = true;

	// Show intensity controls
	DatasetInfo::m_pIntensityText->Show();
	DatasetInfo::m_psliderThresholdIntensity->Show();
	m_pApplyBtn->Show();
	m_pCancelBtn->Show();

	// Disable other toggleButtons
	m_ptoggleIntensity->Hide();
	m_ptoggleOpacity->Disable();
	m_ptoggleMinMaxLength->Disable();
	m_ptoggleSubsampling->Disable();
	m_ptoggleColorMode->Disable();
}

void FibersGroup::OnToggleOpacityBtn()
{
	m_isOpacityToggled = true;

	// Show opacity controls
	DatasetInfo::m_pOpacityText->Show();
	DatasetInfo::m_psliderOpacity->Show();
	m_pApplyBtn->Show();
	m_pCancelBtn->Show();

	// Disable other toggleButtons
	m_ptoggleOpacity->Hide();
	m_ptoggleIntensity->Disable();
	m_ptoggleMinMaxLength->Disable();
	m_ptoggleSubsampling->Disable();
	m_ptoggleColorMode->Disable();	
}

void FibersGroup::OnToggleMinMaxLengthBtn()
{
	m_isMinMaxLengthToggled = true;

	// Show Min / Max Length controls
	m_pMinLengthText->Show();
	m_pSliderFibersFilterMin->Show();
	m_pMaxLengthText->Show();
	m_pSliderFibersFilterMax->Show();
	m_pApplyBtn->Show();
	m_pCancelBtn->Show();
	
	// Disable other toggleButtons
	m_ptoggleMinMaxLength->Hide();
	m_ptoggleIntensity->Disable();
	m_ptoggleOpacity->Disable();
	m_ptoggleSubsampling->Disable();
	m_ptoggleColorMode->Disable();	
}

void FibersGroup::OnToggleSubsamplingBtn()
{
	m_isSubsamplingToggled = true;

	// Show subsampling controls
	m_pSubsamplingText->Show();
	m_pSliderFibersSampling->Show();
	m_pApplyBtn->Show();
	m_pCancelBtn->Show();

	// Disable other toggleButtons
	m_ptoggleSubsampling->Hide();
	m_ptoggleOpacity->Disable();
	m_ptoggleIntensity->Disable();
	m_ptoggleMinMaxLength->Disable();
	m_ptoggleColorMode->Disable();	
}

void FibersGroup::OnToggleColorModeBtn()
{
	m_isColorModeToggled = true;

	// Show ColorMode controls

	// Disable other toggleButtons
}

void FibersGroup::OnClickApplyBtn()
{
	m_pApplyBtn->Hide();
	m_pCancelBtn->Hide();

	for(int i = 0; i < (int)m_fibersSets.size(); i++)
	{
		if( m_isIntensityToggled )
		{
			m_fibersSets[i]->setThreshold( DatasetInfo::m_psliderThresholdIntensity->GetValue() );
		}
		if( m_isOpacityToggled )
		{
			m_fibersSets[i]->setAlpha( (float)DatasetInfo::m_psliderOpacity->GetValue() / 100.0f );
		}
		if( m_isMinMaxLengthToggled ||  m_isSubsamplingToggled)
		{
			int minLength = m_pSliderFibersFilterMin->GetValue();
			int maxLength = m_pSliderFibersFilterMax->GetValue();
			int minSubsampling = m_pSliderFibersSampling->GetValue();
			int maxSubsampling = m_pSliderFibersSampling->GetMax();
			m_fibersSets[i]->updateFibersFilters( minLength, maxLength, minSubsampling, maxSubsampling);
		}
		if( m_isColorModeToggled )
		{
		}
	}

	m_isIntensityToggled = false;
	m_isOpacityToggled = false;
	m_isMinMaxLengthToggled = false;
	m_isSubsamplingToggled = false;
	m_isColorModeToggled = false;
}

void FibersGroup::OnClickCancelBtn()
{
	m_pApplyBtn->Hide();
	m_pCancelBtn->Hide();

	m_isIntensityToggled = false;
	m_isOpacityToggled = false;
	m_isMinMaxLengthToggled = false;
	m_isSubsamplingToggled = false;
	m_isColorModeToggled = false;
}

void FibersGroup::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();

	DatasetInfo::m_ptoggleVisibility->Hide();
	DatasetInfo::m_ptoggleFiltering->Hide();
	DatasetInfo::m_pbtnDelete->Hide();
	DatasetInfo::m_pbtnDown->Hide();
	DatasetInfo::m_pbtnUp->Hide();

	m_ptoggleIntensity->Enable();
	m_ptoggleOpacity->Enable();
	m_ptoggleMinMaxLength->Enable();
	m_ptoggleSubsampling->Enable();
	m_ptoggleColorMode->Enable();


	if( m_isIntensityToggled )
	{
		m_ptoggleIntensity->Hide();
		DatasetInfo::m_pIntensityText->Show();
		DatasetInfo::m_psliderThresholdIntensity->Show();
		m_ptoggleOpacity->Disable();
		m_ptoggleMinMaxLength->Disable();
		m_ptoggleSubsampling->Disable();
		m_ptoggleColorMode->Disable();
	}
	else
	{
		DatasetInfo::m_pIntensityText->Hide();
		DatasetInfo::m_psliderThresholdIntensity->Hide();
		m_ptoggleIntensity->SetValue(false);
		m_ptoggleIntensity->Show();
	}

	if( m_isOpacityToggled )
	{
		m_ptoggleOpacity->Hide();
		DatasetInfo::m_pOpacityText->Show();
		DatasetInfo::m_psliderOpacity->Show();
		m_ptoggleIntensity->Disable();
		m_ptoggleMinMaxLength->Disable();
		m_ptoggleSubsampling->Disable();
		m_ptoggleColorMode->Disable();
	}
	else
	{
		DatasetInfo::m_pOpacityText->Hide();
		DatasetInfo::m_psliderOpacity->Hide();
		m_ptoggleOpacity->SetValue(false);
		m_ptoggleOpacity->Show();
	}

	if( m_isMinMaxLengthToggled )
	{
		m_ptoggleMinMaxLength->Hide();
		m_pMinLengthText->Show();
		m_pSliderFibersFilterMin->Show();
		m_pMaxLengthText->Show();
		m_pSliderFibersFilterMax->Show();
		m_ptoggleIntensity->Disable();
		m_ptoggleOpacity->Disable();
		m_ptoggleSubsampling->Disable();
		m_ptoggleColorMode->Disable();
	}
	else
	{
		m_pMinLengthText->Hide();
		m_pSliderFibersFilterMin->Hide();
		m_pMaxLengthText->Hide();
		m_pSliderFibersFilterMax->Hide();
		m_ptoggleMinMaxLength->SetValue(false);;
		m_ptoggleMinMaxLength->Show();
	}

	if( m_isSubsamplingToggled )
	{
		m_ptoggleSubsampling->Hide();
		m_pSubsamplingText->Show();
		m_pSliderFibersSampling->Show();
		m_ptoggleIntensity->Disable();
		m_ptoggleOpacity->Disable();
		m_ptoggleMinMaxLength->Disable();
		m_ptoggleColorMode->Disable();
	}
	else
	{
		m_pSubsamplingText->Hide();
		m_pSliderFibersSampling->Hide();
		m_ptoggleSubsampling->SetValue(false);
		m_ptoggleSubsampling->Show();
	}

	if( m_isColorModeToggled )
	{
		m_ptoggleColorMode->Hide();
		m_ptoggleIntensity->Disable();
		m_ptoggleOpacity->Disable();
		m_ptoggleMinMaxLength->Disable();
		m_ptoggleSubsampling->Disable();
	}
	else
	{
		m_ptoggleColorMode->SetValue(false);
		m_ptoggleColorMode->Show();
	}

	if(m_isIntensityToggled || m_isOpacityToggled || m_isMinMaxLengthToggled || m_isSubsamplingToggled || m_isColorModeToggled )
	{
		m_pApplyBtn->Show();
		m_pCancelBtn->Show();
	}
	else
	{
		m_pApplyBtn->Hide();
		m_pCancelBtn->Hide();
	}	

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