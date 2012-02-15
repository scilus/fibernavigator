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
#include <list>
#include <wx/tokenzr.h>

#include "Anatomy.h"
#include "../Logger.h"
#include "../main.h"
#include "../dataset/DatasetManager.h"

FibersGroup::FibersGroup( DatasetHelper *pDatasetHelper )
	: DatasetInfo( pDatasetHelper ),
	m_isIntensityToggled ( false ),
	m_isOpacityToggled ( false ),
	m_isMinMaxLengthToggled ( false ),
	m_isSubsamplingToggled ( false ),
	m_isColorModeToggled ( false ),
	m_isCrossingFibersToggled( false ),
	m_isNormalColoringStateChanged ( false ),
	m_isLocalColoringStateChanged ( false )
{
    m_name = wxT( "Fibers Group" );
    m_show = true ;
    m_type = FIBERSGROUP;

	m_bufferObjects = new GLuint[3];
}

FibersGroup::~FibersGroup()
{
    Logger::getInstance()->print( wxT( "Executing FibersGroup destructor" ), LOGLEVEL_DEBUG );
}

string FibersGroup::intToString( const int number )
{
    stringstream out;
    out << number;
    return out.str();
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

void FibersGroup::saveDMRI( wxString filename )
{
	ofstream myfile;
	int totalLines = 0;
    char *pFn = NULL;
	float dist = 0.5;

	if( filename.AfterLast( '.' ) != _T( "fib" ) )
    {
        filename += _T( ".fib" );
    }

    pFn = ( char * ) malloc( filename.length() );
    strcpy( pFn, ( const char * ) filename.mb_str( wxConvUTF8 ) );
    myfile.open( pFn, ios::out );

	for(int i = 0; i < (int)m_fibersSets.size(); i++)
	{
		int nbrlines = 0;
		m_fibersSets[i]->getNbLines( nbrlines );
		totalLines += nbrlines;
	}
    
	myfile << "1 FA\n4 min max mean var\n1\n4 0 0 0 0\n4 0 0 0 0\n4 0 0 0 0\n";
	myfile << totalLines << " " << dist << "\n";

	for(int i = 0; i < (int)m_fibersSets.size(); i++)
	{
		m_fibersSets[i]->loadDMRIFibersInFile( myfile);
	}
    myfile.close();
}

/**
 * Save using the VTK binary format.
 */
void FibersGroup::save( wxString filename )
{
    ofstream myfile;
    char *pFn = NULL;
	converterByteINT32 c;
    converterByteFloat f;
	vector<char> vBuffer;
	vector<vector< float > > allPointsToSave;
	vector<vector< int > > allLinesToSave;
	vector<vector< int > > allColorsToSave;
	int allCountLines = 0;
	float pointsSize = 0.0;
	int linesSize = 0;
	int colorsSize = 0;

	if( filename.AfterLast( '.' ) != _T( "fib" ) )
    {
        filename += _T( ".fib" );
    }

    pFn = ( char * ) malloc( filename.length() );
    strcpy( pFn, ( const char * ) filename.mb_str( wxConvUTF8 ) );
    myfile.open( pFn, ios::binary );

	for(int i = 0; i < (int)m_fibersSets.size(); i++)
	{
		vector<float> pointsToSave;
		vector<int> linesToSave;
		vector<int> colorsToSave;
		int countLines = 0;
		m_fibersSets[i]->getFibersInfoToSave( pointsToSave, linesToSave, colorsToSave, countLines );
		
		allPointsToSave.push_back( pointsToSave );
		allLinesToSave.push_back( linesToSave );
		allColorsToSave.push_back( colorsToSave );
		pointsSize += pointsToSave.size();
		linesSize += linesToSave.size();
		colorsSize += colorsToSave.size();
		allCountLines += countLines;
	}

	string header1 = "# vtk DataFile Version 3.0\nvtk output\nBINARY\nDATASET POLYDATA\nPOINTS ";

	header1 += intToString( pointsSize / 3 );
	header1 += " float\n";
	for( unsigned int i = 0; i < header1.size(); ++i )
	{
		vBuffer.push_back( header1[i] );
	}
	
	for(int j = 0; j < (int)allPointsToSave.size(); j++)
	{
		for( unsigned int i = 0; i < allPointsToSave[j].size(); ++i )
		{
			f.f = allPointsToSave[j][i];
			vBuffer.push_back( f.b[3] );
			vBuffer.push_back( f.b[2] );
			vBuffer.push_back( f.b[1] );
			vBuffer.push_back( f.b[0] );
		}
	}
	
	vBuffer.push_back( '\n' );
	string header2 = "LINES " + intToString( allCountLines ) + " " + intToString( linesSize ) + "\n";
    for( unsigned int i = 0; i < header2.size(); ++i )
    {
        vBuffer.push_back( header2[i] );
    }

    for(int j = 0; j < (int)allLinesToSave.size(); j++)
	{
		for( unsigned int i = 0; i < allLinesToSave[j].size(); ++i )
		{
			c.i = allLinesToSave[j][i];
			vBuffer.push_back( c.b[3] );
			vBuffer.push_back( c.b[2] );
			vBuffer.push_back( c.b[1] );
			vBuffer.push_back( c.b[0] );
		}
	}
    
	vBuffer.push_back( '\n' );
    string header3 = "POINT_DATA ";
    header3 += intToString( pointsSize / 3 );
    header3 += " float\n";
    header3 += "COLOR_SCALARS scalars 3\n";
    for( unsigned int i = 0; i < header3.size(); ++i )
    {
        vBuffer.push_back( header3[i] );
    }

	for(int j = 0; j < (int)allColorsToSave.size(); j++)
	{
		for( unsigned int i = 0; i < allColorsToSave[j].size(); ++i )
		{
			vBuffer.push_back( allColorsToSave[j][i] );
		}
    }
	vBuffer.push_back( '\n' );

	// Put the buffer vector into a char* array.
    char* pBuffer = new char[vBuffer.size()];

    for( unsigned int i = 0; i < vBuffer.size(); ++i )
    {
        pBuffer[i] = vBuffer[i];
    }

	myfile.write( pBuffer, vBuffer.size() );
    myfile.close();
    
    delete[] pBuffer;
    pBuffer = NULL;
}

void FibersGroup::resetFibersColor()
{
	for(int i = 0; i < (int)m_fibersSets.size(); i++)
	{
		m_fibersSets[i]->resetColorArray();
	}
}

void FibersGroup::invertFibers()
{
	for(int i = 0; i < (int)m_fibersSets.size(); i++)
	{
		m_fibersSets[i]->invertFibers();
	}
}

void FibersGroup::useFakeTubes()
{
	for(int i = 0; i < (int)m_fibersSets.size(); i++)
	{
		m_fibersSets[i]->useFakeTubes();
	}
}

void FibersGroup::useTransparency()
{
	for(int i = 0; i < (int)m_fibersSets.size(); i++)
	{
		m_fibersSets[i]->useTransparency();
	}
}

void FibersGroup::createPropertiesSizer( PropertiesWindow *pParent )
{
	wxSizer *pSizer;
	pSizer = new wxBoxSizer( wxHORIZONTAL );

	
    DatasetInfo::createPropertiesSizer( pParent );
	
	
	m_pSliderFibersFilterMin = new wxSlider( pParent, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxSize( 145, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
	m_pMinLengthText = new wxStaticText( pParent, wxID_ANY , wxT( "Min Length" ), wxDefaultPosition, wxSize( 60
, -1 ), wxALIGN_CENTRE );
    pSizer->Add( m_pMinLengthText , 0, wxALIGN_CENTER );
    pSizer->Add( m_pSliderFibersFilterMin, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pSliderFibersFilterMin->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
	m_pSliderFibersFilterMax = new wxSlider( pParent, wxID_ANY, INT_MAX, 0, INT_MAX, wxDefaultPosition, wxSize( 145, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
	m_pMaxLengthText = new wxStaticText( pParent, wxID_ANY , wxT( "Max Length" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_CENTRE );
    pSizer->Add( m_pMaxLengthText , 0, wxALIGN_CENTER );
    pSizer->Add( m_pSliderFibersFilterMax, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pSliderFibersFilterMax->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
	m_pSliderFibersSampling = new wxSlider( pParent, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxSize( 145, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
	m_pSubsamplingText = new wxStaticText( pParent, wxID_ANY , wxT( "Subsampling" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_CENTRE );
	pSizer->Add( m_pSubsamplingText , 0, wxALIGN_CENTER );
    pSizer->Add( m_pSliderFibersSampling, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pSliderFibersSampling->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );
	
	pSizer = new wxBoxSizer( wxHORIZONTAL );
	m_pSliderThickness = new wxSlider( pParent, wxID_ANY, 10, 1, 20, wxDefaultPosition, wxSize( 145, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
	m_pThicknessText = new wxStaticText( pParent, wxID_ANY , wxT( "Thickness" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_CENTRE );
	pSizer->Add( m_pThicknessText , 0, wxALIGN_CENTER );
    pSizer->Add( m_pSliderThickness, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );

    pSizer = new wxBoxSizer( wxHORIZONTAL );
    m_pToggleLocalColoring = new wxToggleButton( pParent, wxID_ANY, wxT( "Local Coloring" ), wxDefaultPosition, wxSize( 145, -1 ) );
    pSizer->Add( m_pToggleLocalColoring, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
	pParent->Connect( m_pToggleLocalColoring->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleLocalColoringBtn ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    m_pToggleNormalColoring = new wxToggleButton( pParent, wxID_ANY, wxT( "Color With Overlay" ), wxDefaultPosition, wxSize( 145, -1 ) );
    pSizer->Add( m_pToggleNormalColoring, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
	pParent->Connect( m_pToggleNormalColoring->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleNormalColoringBtn ) );

    pSizer = new wxBoxSizer( wxHORIZONTAL );
	m_pColoringText = new wxStaticText( pParent, wxID_ANY, _T( "Coloring" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT );
    pSizer->Add( m_pColoringText, 0, wxALIGN_CENTER );
    pSizer->Add( 8, 1, 0 );
    m_pRadioNormalColoring = new wxRadioButton( pParent, wxID_ANY, _T( "Normal" ), wxDefaultPosition, wxSize( 145, -1 ) );
    m_pRadioNormalColoring->SetValue( true );
	pSizer->Add( m_pRadioNormalColoring );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    
	m_pRadioDistanceAnchoring  = new wxRadioButton( pParent, wxID_ANY, _T( "Dist. Anchoring" ), wxDefaultPosition, wxSize( 145, -1 ) );
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( 68, 1, 0 );
    pSizer->Add( m_pRadioDistanceAnchoring );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    
	m_pRadioMinDistanceAnchoring  = new wxRadioButton( pParent, wxID_ANY, _T( "Min Dist. Anchoring" ), wxDefaultPosition, wxSize( 145, -1 ) );
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( 68, 1, 0 );
    pSizer->Add( m_pRadioMinDistanceAnchoring );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    
	m_pRadioCurvature  = new wxRadioButton( pParent, wxID_ANY, _T( "Curvature" ), wxDefaultPosition, wxSize( 145, -1 ) );
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( 68, 1, 0 );
    pSizer->Add( m_pRadioCurvature );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    
	m_pRadioTorsion  = new wxRadioButton( pParent, wxID_ANY, _T( "Torsion" ), wxDefaultPosition, wxSize( 145, -1 ) );
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( 68, 1, 0 );
    pSizer->Add( m_pRadioTorsion );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );

	pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( 68, 8, 0 );
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );

	m_pApplyBtn = new wxButton(pParent, wxID_ANY, wxT("Apply"), wxDefaultPosition, wxSize(90, -1));
	m_pCancelBtn = new wxButton(pParent, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxSize(90, -1));
	pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_pApplyBtn,0, wxALIGN_CENTER);
	pSizer->Add(m_pCancelBtn, 0, wxALIGN_CENTER);
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
	pParent->Connect( m_pApplyBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnClickApplyBtn ) );
	pParent->Connect( m_pCancelBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnClickCancelBtn ) );

	m_propertiesSizer->AddSpacer( 8 );

    m_ptoggleIntensity = new wxToggleButton(pParent, wxID_ANY, wxT("Intensity"),wxDefaultPosition, wxSize(145,-1));
    pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_ptoggleIntensity,0,wxALIGN_LEFT);
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_ptoggleIntensity->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleIntensityBtn ) );

	m_ptoggleOpacity = new wxToggleButton(pParent, wxID_ANY, wxT("Opacity"),wxDefaultPosition, wxSize(145,-1));
    pSizer = new wxBoxSizer(wxHORIZONTAL);
	pSizer->Add(m_ptoggleOpacity,0,wxALIGN_LEFT);
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
	pParent->Connect( m_ptoggleOpacity->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleOpacityBtn ) );

	m_ptoggleMinMaxLength = new wxToggleButton(pParent, wxID_ANY, wxT("Min / Max Length"),wxDefaultPosition, wxSize(145,-1));
    pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_ptoggleMinMaxLength,0,wxALIGN_LEFT);
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
	pParent->Connect( m_ptoggleMinMaxLength->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleMinMaxLengthBtn ) );

	m_ptoggleSubsampling = new wxToggleButton(pParent, wxID_ANY, wxT("Subsampling"),wxDefaultPosition, wxSize(145,-1));
	pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_ptoggleSubsampling,0,wxALIGN_LEFT);
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
	pParent->Connect( m_ptoggleSubsampling->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleSubsamplingBtn ) );
	
	m_pToggleCrossingFibers = new wxToggleButton(pParent, wxID_ANY, wxT("Intersected Fibers"),wxDefaultPosition, wxSize(145,-1));
	pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_pToggleCrossingFibers,0,wxALIGN_LEFT);
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
	pParent->Connect( m_pToggleCrossingFibers->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleCrossingFibersBtn ) );

	m_pGeneratesFibersDensityVolume = new wxButton( pParent, wxID_ANY, wxT( "New Density Volume" ), wxDefaultPosition, wxSize( 145, -1 ) );
	pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( m_pGeneratesFibersDensityVolume, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pGeneratesFibersDensityVolume->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnClickGenerateFiberVolumeBtn ) );
	
	m_ptoggleColorMode = new wxToggleButton(pParent, wxID_ANY, wxT("ColorMode"),wxDefaultPosition, wxSize(145,-1));
	pSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(m_ptoggleColorMode,0,wxALIGN_LEFT);
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
	pParent->Connect( m_ptoggleColorMode->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleColorModeBtn ) );

	pSizer = new wxBoxSizer(wxHORIZONTAL);
	m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
	
	wxStaticText* pWarningMsg = new wxStaticText( pParent, wxID_ANY , wxT( "\n Warning: initial state of \n settings may not represent \n the current state of all fibers bundles."), wxDefaultPosition, wxSize( 200, -1 ), wxALIGN_CENTER );
	pSizer->Add( pWarningMsg , 0, wxALIGN_CENTER );
}

void FibersGroup::OnToggleVisibleBtn()
{
	bool show = getShow();
	
    for( vector<Fibers *>::iterator it = m_fibersSets.begin(); it != m_fibersSets.end(); ++it )
	{
        (*it)->setShow( show );
	}
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
	m_pGeneratesFibersDensityVolume->Disable();
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
	m_pGeneratesFibersDensityVolume->Disable();
	m_ptoggleColorMode->Disable();	
}

void FibersGroup::OnToggleMinMaxLengthBtn()
{
	m_isMinMaxLengthToggled = true;

	// Compute min/max values from all fibers bundles
	int minLength = INT_MAX;
	int maxLength = INT_MIN;
	for(int i = 0; i < (int)m_fibersSets.size(); i++)
	{
		minLength = std::min( minLength, (int)m_fibersSets[i]->getMinFibersLength() );
		maxLength = std::max( maxLength, (int)m_fibersSets[i]->getMaxFibersLength() );
	}
	m_pSliderFibersFilterMin->SetRange( minLength, maxLength );
	m_pSliderFibersFilterMax->SetRange( minLength, maxLength );

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
	m_pGeneratesFibersDensityVolume->Disable();
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
	m_pGeneratesFibersDensityVolume->Disable();
	m_ptoggleColorMode->Disable();	
}

void FibersGroup::OnToggleCrossingFibersBtn()
{
	m_isCrossingFibersToggled = true;
	
	// Show crossing fibers controls
	m_pThicknessText->Show();
	m_pSliderThickness->Show();
	m_pApplyBtn->Show();
	m_pCancelBtn->Show();
	
	// Disable other toggleButtons
	m_pToggleCrossingFibers->Hide();
	m_ptoggleSubsampling->Disable();
	m_ptoggleOpacity->Disable();
	m_ptoggleIntensity->Disable();
	m_ptoggleMinMaxLength->Disable();
	m_pGeneratesFibersDensityVolume->Disable();
	m_ptoggleColorMode->Disable();	
}

void FibersGroup::OnToggleColorModeBtn()
{
	m_isColorModeToggled = true;

	// Show ColorMode controls
	m_pToggleLocalColoring->Show();
	m_pToggleNormalColoring->Show();
	m_pColoringText->Show();
	m_pRadioNormalColoring->Show();
	m_pRadioDistanceAnchoring->Show();
	m_pRadioMinDistanceAnchoring->Show();
	m_pRadioCurvature->Show();
	m_pRadioTorsion->Show();
	m_pApplyBtn->Show();
	m_pCancelBtn->Show();

	FibersColorationMode colorationMode = m_fibersSets[0]->getColorationMode();

	for(int i = 1; i < (int)m_fibersSets.size(); i++)
	{
		if( colorationMode != m_fibersSets[i]->getColorationMode())
		{
			colorationMode = NORMAL_COLOR;
			break;
		}
	}

	m_pRadioNormalColoring->SetValue( colorationMode == NORMAL_COLOR );
	m_pRadioDistanceAnchoring->SetValue( colorationMode == DISTANCE_COLOR );
	m_pRadioMinDistanceAnchoring->SetValue( colorationMode == MINDISTANCE_COLOR );
	m_pRadioCurvature->SetValue( colorationMode == CURVATURE_COLOR );
	m_pRadioTorsion->SetValue( colorationMode == TORSION_COLOR );

	// Disable other toggleButtons
	m_ptoggleIntensity->Disable();
	m_ptoggleOpacity->Disable();
	m_ptoggleMinMaxLength->Disable();
	m_ptoggleSubsampling->Disable();
	m_pGeneratesFibersDensityVolume->Disable();
	m_ptoggleColorMode->Hide();	
}

void FibersGroup::updateGroupFilters()
{
    int min = m_pSliderFibersFilterMin->GetValue();
    int max = m_pSliderFibersFilterMax->GetValue();
    int subSampling = m_pSliderFibersSampling->GetValue();
    int maxSubSampling = m_pSliderFibersSampling->GetMax() + 1;
	int thickness = m_pSliderThickness->GetValue();

	for(int j = 0; j < (int)m_fibersSets.size(); j++)
	{
		int minLength = std::max( min, (int)m_fibersSets[j]->getMinFibersLength() );
		int maxLength = std::min( max, (int)m_fibersSets[j]->getMaxFibersLength() );
		m_fibersSets[j]->updateFibersFilters( minLength, maxLength, subSampling, maxSubSampling);
		m_fibersSets[j]->updateSliderMinLength( minLength );
		m_fibersSets[j]->updateSliderMaxLength( maxLength );
		m_fibersSets[j]->updateSliderSubsampling( subSampling );
		m_fibersSets[j]->updateSliderThickness( thickness );
	}
}

void FibersGroup::fibersLocalColoring()
{
    for( vector<Fibers *>::iterator it = m_fibersSets.begin(); it != m_fibersSets.end(); ++it )
    {
        (*it)->toggleUseTex();
        (*it)->updateToggleLocalColoring( m_pToggleLocalColoring->GetValue() );
    }
}

void FibersGroup::fibersNormalColoring()
{
    for( vector<Fibers *>::iterator it = m_fibersSets.begin(); it != m_fibersSets.end(); ++it )
    {
        (*it)->toggleShowFS();
        (*it)->updateToggleNormalColoring( m_pToggleNormalColoring->GetValue() );
    }
}

void FibersGroup::OnClickGenerateFiberVolumeBtn()
{	
	std::vector<Anatomy*> vAnatomies;
	// Generate fiber volume for individual bundle
	for(int j = 0; j < (int)m_fibersSets.size(); j++)
	{
		Anatomy* pAnatomy = m_fibersSets[j]->generateFiberVolume();
		vAnatomies.push_back(pAnatomy);
	}
	
	generateGlobalFiberVolume(vAnatomies);
}

void FibersGroup::generateGlobalFiberVolume(std::vector<Anatomy*> vAnatomies)
{
    int index = DatasetManager::getInstance()->createAnatomy( RGB );
    Anatomy* pGlobalAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( index );

    pGlobalAnatomy->setName( wxT( "Global Fiber-Density Volume" ) );

    m_dh->m_mainFrame->m_pListCtrl2->InsertItem( index );

    m_dh->m_mainFrame->refreshAllGLWidgets();

    for(int i = 0; i < (int)vAnatomies.size(); i++)
    {
        pGlobalAnatomy->add( vAnatomies[i] );
    }
}

void FibersGroup::OnClickApplyBtn()
{
	m_pApplyBtn->Hide();
	m_pCancelBtn->Hide();

	if( m_isIntensityToggled )
	{
		for(int i = 0; i < (int)m_fibersSets.size(); i++)
		{
			m_fibersSets[i]->setThreshold(	getThreshold() );
			m_fibersSets[i]->setBrightness( getBrightness() );
		}
	}
	if( m_isOpacityToggled )
	{
		for(int i = 0; i < (int)m_fibersSets.size(); i++)
		{
			m_fibersSets[i]->setAlpha( (float)DatasetInfo::m_psliderOpacity->GetValue() / 100.0f );
		}
	}
	if( m_isMinMaxLengthToggled ||  m_isSubsamplingToggled)
	{
		updateGroupFilters();
	}
	if( m_isCrossingFibersToggled )
	{
		for(int i = 0; i < (int)m_fibersSets.size(); i++)
		{
			m_fibersSets[i]->updateSliderThickness( m_pSliderThickness->GetValue());
			m_fibersSets[i]->updateCrossingFibersThickness();
			m_fibersSets[i]->toggleCrossingFibers();
		}
	}
	if( m_isColorModeToggled )
	{
		FibersColorationMode colorationMode = NORMAL_COLOR;
		if( m_pRadioNormalColoring->GetValue() )
		{
			colorationMode = NORMAL_COLOR;
		}
		else if( m_pRadioDistanceAnchoring->GetValue() )
		{
			colorationMode  = DISTANCE_COLOR;
		}
		else if( m_pRadioMinDistanceAnchoring->GetValue() )
		{
			colorationMode = MINDISTANCE_COLOR;
		}
		else if( m_pRadioCurvature->GetValue() )
		{
			colorationMode = CURVATURE_COLOR;
		}
		else if( m_pRadioTorsion->GetValue() )
		{
			colorationMode = TORSION_COLOR;
		}			
		
		for(int i = 0; i < (int)m_fibersSets.size(); i++)
		{
			m_fibersSets[i]->setColorationMode(colorationMode);
			m_fibersSets[i]->updateFibersColors();  
		}
		
		if( m_isLocalColoringStateChanged )
		{
			fibersLocalColoring();
			m_isLocalColoringStateChanged = false;
		}
		if( m_isNormalColoringStateChanged )
		{
			fibersNormalColoring();
			m_isNormalColoringStateChanged = false;
		}
	}

    resetAllValues();
	m_isIntensityToggled = false;
	m_isOpacityToggled = false;
	m_isMinMaxLengthToggled = false;
	m_isSubsamplingToggled = false;
	m_isCrossingFibersToggled = false;
	m_isColorModeToggled = false;
}

void FibersGroup::OnToggleLocalColoring()
{
	m_isLocalColoringStateChanged = true;
}

void FibersGroup::OnToggleNormalColoring()
{
	m_isNormalColoringStateChanged = true;
}

void FibersGroup::OnClickCancelBtn()
{
	m_pApplyBtn->Hide();
	m_pCancelBtn->Hide();

	m_isIntensityToggled = false;
	m_isOpacityToggled = false;
	m_isMinMaxLengthToggled = false;
	m_isSubsamplingToggled = false;
	m_isCrossingFibersToggled = false;
	m_isColorModeToggled = false;
}

void FibersGroup::resetAllValues()
{
    DatasetInfo::m_psliderThresholdIntensity->SetValue( DatasetInfo::m_psliderThresholdIntensity->GetMin() );
    DatasetInfo::m_psliderOpacity->SetValue( DatasetInfo::m_psliderOpacity->GetMin() );
    m_pSliderFibersFilterMin->SetValue( m_pSliderFibersFilterMin->GetMin() );
    m_pSliderFibersFilterMax->SetValue( m_pSliderFibersFilterMax->GetMax() );
    m_pSliderFibersSampling->SetValue( m_pSliderFibersSampling->GetMin() );
	m_pSliderThickness->SetValue( 10.0 );
    m_pToggleLocalColoring->SetValue(false);
    m_pToggleNormalColoring->SetValue(false);
}

void FibersGroup::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();

	DatasetInfo::m_pToggleFiltering->Hide();
	DatasetInfo::m_pBtnFlipX->Hide();
	DatasetInfo::m_pBtnFlipY->Hide();
	DatasetInfo::m_pBtnFlipZ->Hide();

	// Hide temporarily opacity functionality
	m_ptoggleOpacity->Hide();
	
	if(m_fibersSets.size() > 0)
	{
		m_ptoggleIntensity->Enable();
		m_ptoggleOpacity->Enable();
		m_ptoggleMinMaxLength->Enable();
		m_ptoggleSubsampling->Enable();
		m_pToggleCrossingFibers->Enable();
		m_pGeneratesFibersDensityVolume->Enable();
		m_ptoggleColorMode->Enable();
	}
	else
	{
		m_ptoggleIntensity->Disable();
		m_ptoggleOpacity->Disable();
		m_ptoggleMinMaxLength->Disable();
		m_ptoggleSubsampling->Disable();
		m_pToggleCrossingFibers->Disable();
		m_pGeneratesFibersDensityVolume->Disable();
		m_ptoggleColorMode->Disable();
	}

	m_psliderOpacity->Enable( false );
	m_pRadioNormalColoring->Enable( DatasetInfo::getShowFS() );
    m_pRadioCurvature->Enable( DatasetInfo::getShowFS() );
    m_pRadioDistanceAnchoring->Enable( DatasetInfo::getShowFS() );
    m_pRadioMinDistanceAnchoring->Enable( DatasetInfo::getShowFS() );
    m_pRadioTorsion->Enable( DatasetInfo::getShowFS() );

	if( m_isIntensityToggled )
	{
		m_ptoggleIntensity->Hide();
		DatasetInfo::m_pIntensityText->Show();
		DatasetInfo::m_psliderThresholdIntensity->Show();
		m_ptoggleOpacity->Disable();
		m_ptoggleMinMaxLength->Disable();
		m_ptoggleSubsampling->Disable();
		m_pToggleCrossingFibers->Disable();
		m_pGeneratesFibersDensityVolume->Disable();
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
		m_pToggleCrossingFibers->Disable();
		m_pGeneratesFibersDensityVolume->Disable();
		m_ptoggleColorMode->Disable();
		m_psliderOpacity->SetValue( getAlpha()*100 );
	}
	else
	{
		DatasetInfo::m_pOpacityText->Hide();
		DatasetInfo::m_psliderOpacity->Hide();
		m_ptoggleOpacity->SetValue(false);
		// Hide temporarily opacity functionality
		//m_ptoggleOpacity->Show();
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
		m_pToggleCrossingFibers->Disable();
		m_pGeneratesFibersDensityVolume->Disable();
		m_ptoggleColorMode->Disable();
	}
	else
	{
		m_pMinLengthText->Hide();
		m_pSliderFibersFilterMin->Hide();
		m_pMaxLengthText->Hide();
		m_pSliderFibersFilterMax->Hide();
		m_ptoggleMinMaxLength->SetValue(false);
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
		m_pToggleCrossingFibers->Disable();
		m_pGeneratesFibersDensityVolume->Disable();
		m_ptoggleColorMode->Disable();
	}
	else
	{
		m_pSubsamplingText->Hide();
		m_pSliderFibersSampling->Hide();
		m_ptoggleSubsampling->SetValue(false);
		m_ptoggleSubsampling->Show();
	}
	
	if( m_isCrossingFibersToggled )
	{
		m_pToggleCrossingFibers->Hide();
		m_pThicknessText->Show();
		m_pSliderThickness->Show();
		m_ptoggleIntensity->Disable();
		m_ptoggleOpacity->Disable();
		m_ptoggleMinMaxLength->Disable();
		m_ptoggleSubsampling->Disable();
		m_pGeneratesFibersDensityVolume->Disable();
		m_ptoggleColorMode->Disable();
	}
	else
	{
		m_pThicknessText->Hide();
		m_pSliderThickness->Hide();
		m_pToggleCrossingFibers->SetValue(false);
		m_pToggleCrossingFibers->Show();
	}

	if( m_isColorModeToggled )
	{
		m_pToggleLocalColoring->Show();
		m_pToggleNormalColoring->Show();
		m_pColoringText->Show();
		m_pRadioNormalColoring->Show();
		m_pRadioDistanceAnchoring->Show();
		m_pRadioMinDistanceAnchoring->Show();
		m_pRadioCurvature->Show();
		m_pRadioTorsion->Show();
		m_ptoggleColorMode->Hide();
		m_ptoggleIntensity->Disable();
		m_ptoggleOpacity->Disable();
		m_ptoggleMinMaxLength->Disable();
		m_ptoggleSubsampling->Disable();
		m_pToggleCrossingFibers->Disable();
		m_pGeneratesFibersDensityVolume->Disable();
	}
	else
	{
		m_pToggleLocalColoring->Hide();
		m_pToggleNormalColoring->Hide();
		m_pColoringText->Hide();
		m_pRadioNormalColoring->Hide();
		m_pRadioDistanceAnchoring->Hide();
		m_pRadioMinDistanceAnchoring->Hide();
		m_pRadioCurvature->Hide();
		m_pRadioTorsion->Hide();
		m_ptoggleColorMode->SetValue(false);
		m_ptoggleColorMode->Show();
	}

	if(m_isIntensityToggled || m_isOpacityToggled || m_isMinMaxLengthToggled || m_isSubsamplingToggled || m_isCrossingFibersToggled || m_isColorModeToggled )
	{
		m_pApplyBtn->Show();
		m_pCancelBtn->Show();
	}
	else
	{
		m_pApplyBtn->Hide();
		m_pCancelBtn->Hide();
	}	
}