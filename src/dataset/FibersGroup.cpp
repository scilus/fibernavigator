/*
 *  The FibersGroup class implementation.
 *
 */

#include "FibersGroup.h"

#include "Anatomy.h"
#include "DatasetManager.h"
#include "../Logger.h"
#include "../main.h"
#include "../gui/MainFrame.h"
#include "../misc/XmlHelper.h"

#include <wx/tglbtn.h>
#include <wx/tokenzr.h>
#include <wx/xml/xml.h>

#include <cmath>
#include <cfloat>
#include <fstream>
using std::ofstream;

#include <iostream>
#include <limits>
using std::numeric_limits;

#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string>
using std::string;

#include <sstream>
using std::stringstream;

#include <vector>
using std::vector;

#define DEF_POS wxDefaultPosition
#define DEF_SIZE     wxDefaultSize

FibersGroup::FibersGroup()
:   DatasetInfo(),
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
    myfile.open( pFn, std::ios::out );

    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
    {
        int nbrLines = 0;
        (*it)->getNbLines( nbrLines );
        totalLines += nbrLines;
    }

    myfile << "1 FA\n4 min max mean var\n1\n4 0 0 0 0\n4 0 0 0 0\n4 0 0 0 0\n";
    myfile << totalLines << " " << dist << "\n";

    for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
    {
        (*it)->loadDMRIFibersInFile( myfile );
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
    myfile.open( pFn, std::ios::binary );

    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
    {
        vector<float> pointsToSave;
        vector<int> linesToSave;
        vector<int> colorsToSave;
        int countLines = 0;
        (*it)->getFibersInfoToSave( pointsToSave, linesToSave, colorsToSave, countLines );

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

//////////////////////////////////////////////////////////////////////////

bool FibersGroup::save( wxXmlNode *pNode ) const
{
    assert( pNode != NULL );

    pNode->SetName( wxT( "dataset" ) );
    DatasetInfo::save( pNode );
    wxXmlNode *pStatus = getXmlNodeByName( wxT( "status" ), pNode );

    if( NULL != pStatus )
    {
        wxXmlProperty *pProp = getXmlPropertyByName( wxT( "isFiberGroup" ), pStatus );

        if( NULL != pProp )
        {
            pProp->SetValue( wxT( "yes" ) );
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////

void FibersGroup::resetFibersColor()
{
    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
    {
        (*it)->resetColorArray();
    }
}

void FibersGroup::invertFibers()
{
    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
    {
        (*it)->invertFibers();
    }
}

void FibersGroup::useFakeTubes()
{
    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
    {
        (*it)->useFakeTubes();
    }
}

void FibersGroup::useTransparency()
{
    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
    {
        (*it)->useTransparency();
    }
}

void FibersGroup::createPropertiesSizer( PropertiesWindow *pParent )
{
    DatasetInfo::createPropertiesSizer( pParent );

    wxBoxSizer *pBoxMain = new wxBoxSizer( wxVERTICAL );

    //////////////////////////////////////////////////////////////////////////
    m_pLblMinLength   = new wxStaticText( pParent, wxID_ANY, wxT( "Min Length" ) );
    m_pLblMaxLength   = new wxStaticText( pParent, wxID_ANY, wxT( "Max Length" ) );
    m_pLblSubsampling = new wxStaticText( pParent, wxID_ANY, wxT( "Subsampling" ) );
    m_pLblThickness   = new wxStaticText( pParent, wxID_ANY, wxT( "Thickness" ) );
    m_pLblColoring    = new wxStaticText( pParent, wxID_ANY, wxT( "Coloring" ) );
    m_pSliderFibersFilterMin = new wxSlider( pParent, wxID_ANY, 0,       0, INT_MAX, DEF_POS, wxSize( 140, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderFibersFilterMax = new wxSlider( pParent, wxID_ANY, INT_MAX, 0, INT_MAX, DEF_POS, DEF_SIZE,         wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderFibersSampling  = new wxSlider( pParent, wxID_ANY, 0,       0,     100, DEF_POS, DEF_SIZE,         wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pSliderInterFibersThickness = new wxSlider(   pParent, wxID_ANY,  10,  1,  20, DEF_POS, DEF_SIZE,         wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pToggleLocalColoring  = new wxToggleButton(   pParent, wxID_ANY, wxT( "Local Coloring" ) );
    m_pToggleNormalColoring = new wxToggleButton(   pParent, wxID_ANY, wxT( "Color With Overlay" ) );
    m_pToggleInterFibers = new wxToggleButton(   pParent, wxID_ANY, wxT( "Intersected Fibers" ) );
    m_pRadNormalColoring       = new wxRadioButton( pParent, wxID_ANY, wxT( "Normal" ), DEF_POS, DEF_SIZE, wxRB_GROUP );
    m_pRadDistanceAnchoring    = new wxRadioButton( pParent, wxID_ANY, wxT( "Dist. Anchoring" ) );
    m_pRadMinDistanceAnchoring = new wxRadioButton( pParent, wxID_ANY, wxT( "Min Dist. Anchoring" ) );
    m_pRadCurvature            = new wxRadioButton( pParent, wxID_ANY, wxT( "Curvature" ) );
    m_pRadTorsion              = new wxRadioButton( pParent, wxID_ANY, wxT( "Torsion" ) );
    m_pRadConstantColor        = new wxRadioButton( pParent, wxID_ANY, wxT( "Constant color" ) );
    m_pApplyBtn  = new wxButton( pParent, wxID_ANY, wxT( "Apply" ) );
    m_pCancelBtn = new wxButton( pParent, wxID_ANY, wxT( "Cancel" ) );
    m_pBtnGeneratesDensityVolume = new wxButton( pParent, wxID_ANY, wxT( "New Density Volume" ) );

    m_pBtnOpacity      = new wxButton( pParent, wxID_ANY, wxT( "Opacity" ) );
    m_pBtnIntensity    = new wxButton( pParent, wxID_ANY, wxT( "Intensity" ) );
    m_pBtnMinMaxLength = new wxButton( pParent, wxID_ANY, wxT( "Min / Max Length" ) );
    m_pBtnSubsampling  = new wxButton( pParent, wxID_ANY, wxT( "Subsampling" ) );
    m_pBtnColorMode    = new wxButton( pParent, wxID_ANY, wxT( "ColorMode" ) );

    //////////////////////////////////////////////////////////////////////////

    pBoxMain->AddSpacer( 8 );

    wxFlexGridSizer *pGridSliders = new wxFlexGridSizer( 2 );

    pGridSliders->Add( m_pLblMinLength, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderFibersFilterMin, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pGridSliders->Add( m_pLblMaxLength, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderFibersFilterMax, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pGridSliders->Add( m_pLblSubsampling, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderFibersSampling, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pGridSliders->Add( m_pLblThickness, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    pGridSliders->Add( m_pSliderInterFibersThickness, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 1 );

    pBoxMain->Add( pGridSliders, 0, wxEXPAND | wxALL, 2 );

    //////////////////////////////////////////////////////////////////////////

    pBoxMain->Add( m_pToggleLocalColoring,  0, wxEXPAND | wxLEFT | wxRIGHT, 24 );
    pBoxMain->Add( m_pToggleNormalColoring, 0, wxEXPAND | wxLEFT | wxRIGHT, 24 );

    //////////////////////////////////////////////////////////////////////////

    wxBoxSizer *pBoxColoring = new wxBoxSizer( wxVERTICAL );
    pBoxColoring->Add( m_pLblColoring, 0, wxALIGN_LEFT | wxALL, 1 );

    wxBoxSizer *pBoxColoringRadios = new wxBoxSizer( wxVERTICAL );
    pBoxColoringRadios->Add( m_pRadNormalColoring,       0, wxALIGN_LEFT | wxALL, 1 );
    pBoxColoringRadios->Add( m_pRadDistanceAnchoring,    0, wxALIGN_LEFT | wxALL, 1 );
    pBoxColoringRadios->Add( m_pRadMinDistanceAnchoring, 0, wxALIGN_LEFT | wxALL, 1 );
    pBoxColoringRadios->Add( m_pRadCurvature,            0, wxALIGN_LEFT | wxALL, 1 );
    pBoxColoringRadios->Add( m_pRadTorsion,              0, wxALIGN_LEFT | wxALL, 1 );
    pBoxColoringRadios->Add( m_pRadConstantColor,        0, wxALIGN_LEFT | wxALL, 1 );
    pBoxColoring->Add( pBoxColoringRadios, 0, wxALIGN_LEFT | wxLEFT, 32 );

    pBoxMain->Add( pBoxColoring, 0, wxFIXED_MINSIZE | wxEXPAND | wxTOP | wxBOTTOM, 8 );

    //////////////////////////////////////////////////////////////////////////

    wxBoxSizer *pBoxApplyCancel = new wxBoxSizer( wxHORIZONTAL );
    pBoxApplyCancel->Add( m_pApplyBtn,  1, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8 );
    pBoxApplyCancel->Add( m_pCancelBtn, 1, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8 );
    pBoxMain->Add( pBoxApplyCancel, 0, wxEXPAND | wxTOP | wxBOTTOM, 12 );

    //////////////////////////////////////////////////////////////////////////

    pBoxMain->Add( m_pBtnOpacity,      0, wxEXPAND | wxLEFT | wxRIGHT, 24 );
    pBoxMain->Add( m_pBtnIntensity,    0, wxEXPAND | wxLEFT | wxRIGHT, 24 );
    pBoxMain->Add( m_pBtnMinMaxLength, 0, wxEXPAND | wxLEFT | wxRIGHT, 24 );
    pBoxMain->Add( m_pBtnSubsampling,  0, wxEXPAND | wxLEFT | wxRIGHT, 24 );
    pBoxMain->Add( m_pBtnColorMode,    0, wxEXPAND | wxLEFT | wxRIGHT, 24 );

    //////////////////////////////////////////////////////////////////////////

    pBoxMain->Add( m_pBtnGeneratesDensityVolume, 0, wxEXPAND | wxLEFT | wxRIGHT, 24 );
    pBoxMain->Add( m_pToggleInterFibers, 0, wxEXPAND | wxLEFT | wxRIGHT, 24 );

    //////////////////////////////////////////////////////////////////////////

    pBoxMain->Add( new wxStaticText( pParent, wxID_ANY , wxT( "\n Warning: initial state of \n settings may not represent \n the current state of all fibers bundles."), DEF_POS, wxSize( 100, -1 ), wxALIGN_CENTER ),
        0, wxEXPAND | wxLEFT | wxRIGHT, 24 );

    //////////////////////////////////////////////////////////////////////////

    m_pPropertiesSizer->Add( pBoxMain, 0, wxFIXED_MINSIZE | wxEXPAND, 0 );

    //////////////////////////////////////////////////////////////////////////

    pParent->Connect( m_pSliderFibersFilterMin->GetId(),     wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );
    pParent->Connect( m_pSliderFibersFilterMax->GetId(),     wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );
    pParent->Connect( m_pSliderFibersSampling->GetId(),      wxEVT_COMMAND_SLIDER_UPDATED,       wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );
    pParent->Connect( m_pToggleLocalColoring->GetId(),       wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler(        PropertiesWindow::OnToggleLocalColoringBtn ) );
    pParent->Connect( m_pToggleNormalColoring->GetId(),      wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler(        PropertiesWindow::OnToggleNormalColoringBtn ) );
    pParent->Connect( m_pApplyBtn->GetId(),                  wxEVT_COMMAND_BUTTON_CLICKED,       wxEventHandler(        PropertiesWindow::OnClickApplyBtn ) );
    pParent->Connect( m_pCancelBtn->GetId(),                 wxEVT_COMMAND_BUTTON_CLICKED,       wxEventHandler(        PropertiesWindow::OnClickCancelBtn ) );
    pParent->Connect( m_pBtnIntensity->GetId(),              wxEVT_COMMAND_BUTTON_CLICKED,       wxEventHandler(        PropertiesWindow::OnToggleIntensityBtn ) );
    pParent->Connect( m_pBtnOpacity->GetId(),                wxEVT_COMMAND_BUTTON_CLICKED,       wxEventHandler(        PropertiesWindow::OnToggleOpacityBtn ) );
    pParent->Connect( m_pBtnMinMaxLength->GetId(),           wxEVT_COMMAND_BUTTON_CLICKED,       wxEventHandler(        PropertiesWindow::OnToggleMinMaxLengthBtn ) );
    pParent->Connect( m_pBtnSubsampling->GetId(),            wxEVT_COMMAND_BUTTON_CLICKED,       wxEventHandler(        PropertiesWindow::OnToggleSubsamplingBtn ) );
    pParent->Connect( m_pToggleInterFibers->GetId(),         wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler(        PropertiesWindow::OnToggleCrossingFibersBtn ) );
    pParent->Connect( m_pBtnGeneratesDensityVolume->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,       wxEventHandler(        PropertiesWindow::OnClickGenerateFiberVolumeBtn ) );
    pParent->Connect( m_pBtnColorMode->GetId(),              wxEVT_COMMAND_BUTTON_CLICKED,       wxEventHandler(        PropertiesWindow::OnToggleColorModeBtn ) );
}

void FibersGroup::OnToggleVisibleBtn()
{
    bool show = getShow();

    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
    {
        (*it)->setShow( show );
    }
}

void FibersGroup::OnToggleIntensityBtn()
{
    m_isIntensityToggled = true;

    // Show intensity controls
    DatasetInfo::m_pIntensityText->Show();
    DatasetInfo::m_pSliderThresholdIntensity->Show();
    m_pApplyBtn->Show();
    m_pCancelBtn->Show();

    // Disable other toggleButtons
    m_pBtnIntensity->Hide();
    m_pBtnOpacity->Disable();
    m_pBtnMinMaxLength->Disable();
    m_pBtnSubsampling->Disable();
    m_pBtnGeneratesDensityVolume->Disable();
    m_pBtnColorMode->Disable();
}

void FibersGroup::OnToggleOpacityBtn()
{
    m_isOpacityToggled = true;

    // Show opacity controls
    DatasetInfo::m_pOpacityText->Show();
    DatasetInfo::m_pSliderOpacity->Show();
    m_pApplyBtn->Show();
    m_pCancelBtn->Show();

    // Disable other toggleButtons
    m_pBtnOpacity->Hide();
    m_pBtnIntensity->Disable();
    m_pBtnMinMaxLength->Disable();
    m_pBtnSubsampling->Disable();
    m_pBtnGeneratesDensityVolume->Disable();
    m_pBtnColorMode->Disable();
}

void FibersGroup::OnToggleMinMaxLengthBtn()
{
    m_isMinMaxLengthToggled = true;

    // Compute min/max values from all fibers bundles
    float minLength = numeric_limits<float>::max();
    float maxLength = numeric_limits<float>::min();

    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
    {
        minLength = std::min( minLength, (*it)->getMinFibersLength() );
        maxLength = std::max( maxLength, (*it)->getMaxFibersLength() );
    }

    m_pSliderFibersFilterMin->SetRange( minLength, maxLength );
    m_pSliderFibersFilterMax->SetRange( minLength, maxLength );

    // Show Min / Max Length controls
    m_pLblMinLength->Show();
    m_pSliderFibersFilterMin->Show();
    m_pLblMaxLength->Show();
    m_pSliderFibersFilterMax->Show();
    m_pApplyBtn->Show();
    m_pCancelBtn->Show();

    // Disable other toggleButtons
    m_pBtnMinMaxLength->Hide();
    m_pBtnIntensity->Disable();
    m_pBtnOpacity->Disable();
    m_pBtnSubsampling->Disable();
    m_pBtnGeneratesDensityVolume->Disable();
    m_pBtnColorMode->Disable();
}

void FibersGroup::OnToggleSubsamplingBtn()
{
    m_isSubsamplingToggled = true;

    // Show subsampling controls
    m_pLblSubsampling->Show();
    m_pSliderFibersSampling->Show();
    m_pApplyBtn->Show();
    m_pCancelBtn->Show();

    // Disable other toggleButtons
    m_pBtnSubsampling->Hide();
    m_pBtnOpacity->Disable();
    m_pBtnIntensity->Disable();
    m_pBtnMinMaxLength->Disable();
    m_pBtnGeneratesDensityVolume->Disable();
    m_pBtnColorMode->Disable();
}

void FibersGroup::OnToggleCrossingFibersBtn()
{
    m_isCrossingFibersToggled = true;

    // Show crossing fibers controls
    m_pLblThickness->Show();
    m_pSliderInterFibersThickness->Show();
    m_pApplyBtn->Show();
    m_pCancelBtn->Show();

    // Disable other toggleButtons
    m_pToggleInterFibers->Hide();
    m_pBtnSubsampling->Disable();
    m_pBtnOpacity->Disable();
    m_pBtnIntensity->Disable();
    m_pBtnMinMaxLength->Disable();
    m_pBtnGeneratesDensityVolume->Disable();
    m_pBtnColorMode->Disable();
}

void FibersGroup::OnToggleColorModeBtn()
{
    m_isColorModeToggled = true;

    // Show ColorMode controls
    m_pToggleLocalColoring->Show();
    m_pToggleNormalColoring->Show();
    m_pLblColoring->Show();
    m_pRadNormalColoring->Show();
    m_pRadDistanceAnchoring->Show();
    m_pRadMinDistanceAnchoring->Show();
    m_pRadCurvature->Show();
    m_pRadTorsion->Show();
    m_pRadConstantColor->Show();
    m_pApplyBtn->Show();
    m_pCancelBtn->Show();

    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    vector<Fibers *>::const_iterator it = fibers.begin();

    FibersColorationMode colorationMode = (*it)->getColorationMode();
    ++it;

    for( ; it != fibers.end(); ++it )
    {
        if( colorationMode != (*it)->getColorationMode() )
        {
            colorationMode = NORMAL_COLOR;
            break;
        }
    }

    m_pRadNormalColoring->SetValue( colorationMode == NORMAL_COLOR );
    m_pRadDistanceAnchoring->SetValue( colorationMode == DISTANCE_COLOR );
    m_pRadMinDistanceAnchoring->SetValue( colorationMode == MINDISTANCE_COLOR );
    m_pRadCurvature->SetValue( colorationMode == CURVATURE_COLOR );
    m_pRadTorsion->SetValue( colorationMode == TORSION_COLOR );
    m_pRadConstantColor->SetValue( colorationMode == CONSTANT_COLOR );

    // Disable other toggleButtons
    m_pBtnIntensity->Disable();
    m_pBtnOpacity->Disable();
    m_pBtnMinMaxLength->Disable();
    m_pBtnSubsampling->Disable();
    m_pBtnGeneratesDensityVolume->Disable();
    m_pBtnColorMode->Hide();
}

void FibersGroup::updateGroupFilters()
{
    int min = m_pSliderFibersFilterMin->GetValue();
    int max = m_pSliderFibersFilterMax->GetValue();
    int subSampling = m_pSliderFibersSampling->GetValue();
    int maxSubSampling = m_pSliderFibersSampling->GetMax() + 1;
    int thickness = m_pSliderInterFibersThickness->GetValue();

    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
    {
        int minLength = std::max( min, (int)(*it)->getMinFibersLength() );
        int maxLength = std::min( max, (int)(*it)->getMaxFibersLength() );
        (*it)->updateFibersFilters( minLength, maxLength, subSampling, maxSubSampling );
        (*it)->updateSliderMinLength( minLength );
        (*it)->updateSliderMaxLength( maxLength );
        (*it)->updateSliderSubsampling( subSampling );
        (*it)->updateSliderThickness( thickness );
    }
}

void FibersGroup::fibersLocalColoring()
{
    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
    {
        (*it)->toggleUseTex();
        (*it)->updateToggleLocalColoring( m_pToggleLocalColoring->GetValue() );
    }
}

void FibersGroup::fibersNormalColoring()
{
    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
    {
        (*it)->toggleShowFS();
        (*it)->updateToggleNormalColoring( m_pToggleNormalColoring->GetValue() );
    }
}

void FibersGroup::OnClickGenerateFiberVolumeBtn()
{
    std::vector<Anatomy*> vAnatomies;

    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    // Generate fiber volume for individual bundle
    for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
    {
        Anatomy* pAnatomy = (*it)->generateFiberVolume();
        vAnatomies.push_back( pAnatomy );
    }

    generateGlobalFiberVolume( vAnatomies );
}

void FibersGroup::generateGlobalFiberVolume(std::vector<Anatomy*> vAnatomies)
{
    int index = DatasetManager::getInstance()->createAnatomy( RGB );
    Anatomy* pGlobalAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( index );

    pGlobalAnatomy->setName( wxT( "Global Fiber-Density Volume" ) );

    MyApp::frame->m_pListCtrl->InsertItem( index );

    MyApp::frame->refreshAllGLWidgets();

    for(int i = 0; i < (int)vAnatomies.size(); i++)
    {
        pGlobalAnatomy->add( vAnatomies[i] );
    }
}

void FibersGroup::OnClickApplyBtn()
{
    m_pApplyBtn->Hide();
    m_pCancelBtn->Hide();

    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();


    if( m_isIntensityToggled )
    {
        for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
        {
            (*it)->setThreshold( getThreshold() );
            (*it)->setBrightness( getBrightness() );
        }
    }
    else if( m_isOpacityToggled )
    {
        for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
        {
            (*it)->setAlpha( (float)DatasetInfo::m_pSliderOpacity->GetValue() / 100.0f );
        }
    }
    else if( m_isMinMaxLengthToggled || m_isSubsamplingToggled )
    {
        updateGroupFilters();
    }
    else if( m_isCrossingFibersToggled )
    {
        for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
        {
            (*it)->updateSliderThickness( m_pSliderInterFibersThickness->GetValue() );
            (*it)->updateCrossingFibersThickness();
            (*it)->toggleCrossingFibers();
        }
    }
    else if( m_isColorModeToggled )
    {
        FibersColorationMode colorationMode = NORMAL_COLOR;
        if( m_pRadNormalColoring->GetValue() )
        {
            colorationMode = NORMAL_COLOR;
        }
        else if( m_pRadDistanceAnchoring->GetValue() )
        {
            colorationMode  = DISTANCE_COLOR;
        }
        else if( m_pRadMinDistanceAnchoring->GetValue() )
        {
            colorationMode = MINDISTANCE_COLOR;
        }
        else if( m_pRadCurvature->GetValue() )
        {
            colorationMode = CURVATURE_COLOR;
        }
        else if( m_pRadTorsion->GetValue() )
        {
            colorationMode = TORSION_COLOR;
        }
        else if( m_pRadConstantColor->GetValue() )
        {
            colorationMode = CONSTANT_COLOR;
        }

        for( vector<Fibers *>::const_iterator it = fibers.begin(); it != fibers.end(); ++it )
        {
            (*it)->setColorationMode( colorationMode );
            (*it)->updateFibersColors();
            (*it)->updateColorationMode();
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
    DatasetInfo::m_pSliderThresholdIntensity->SetValue( DatasetInfo::m_pSliderThresholdIntensity->GetMin() );
    DatasetInfo::m_pSliderOpacity->SetValue( DatasetInfo::m_pSliderOpacity->GetMin() );
    m_pSliderFibersFilterMin->SetValue( m_pSliderFibersFilterMin->GetMin() );
    m_pSliderFibersFilterMax->SetValue( m_pSliderFibersFilterMax->GetMax() );
    m_pSliderFibersSampling->SetValue( m_pSliderFibersSampling->GetMin() );
    m_pSliderInterFibersThickness->SetValue( 10.0 );
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
    m_pBtnOpacity->Hide();

    vector<Fibers *> fibers = DatasetManager::getInstance()->getFibers();
    if( fibers.size() > 0 )
    {
        m_pBtnIntensity->Enable();
        m_pBtnOpacity->Enable();
        m_pBtnMinMaxLength->Enable();
        m_pBtnSubsampling->Enable();
        m_pToggleInterFibers->Enable();
        m_pBtnGeneratesDensityVolume->Enable();
        m_pBtnColorMode->Enable();
    }
    else
    {
        m_pBtnIntensity->Disable();
        m_pBtnOpacity->Disable();
        m_pBtnMinMaxLength->Disable();
        m_pBtnSubsampling->Disable();
        m_pToggleInterFibers->Disable();
        m_pBtnGeneratesDensityVolume->Disable();
        m_pBtnColorMode->Disable();
    }

    m_pSliderOpacity->Enable( false );
    m_pRadNormalColoring->Enable( DatasetInfo::getShowFS() );
    m_pRadCurvature->Enable( DatasetInfo::getShowFS() );
    m_pRadDistanceAnchoring->Enable( DatasetInfo::getShowFS() );
    m_pRadMinDistanceAnchoring->Enable( DatasetInfo::getShowFS() );
    m_pRadTorsion->Enable( DatasetInfo::getShowFS() );
    m_pRadConstantColor->Enable( DatasetInfo::getShowFS() );

    if( m_isIntensityToggled )
    {
        m_pBtnIntensity->Hide();
        DatasetInfo::m_pIntensityText->Show();
        DatasetInfo::m_pSliderThresholdIntensity->Show();
        m_pBtnOpacity->Disable();
        m_pBtnMinMaxLength->Disable();
        m_pBtnSubsampling->Disable();
        m_pToggleInterFibers->Disable();
        m_pBtnGeneratesDensityVolume->Disable();
        m_pBtnColorMode->Disable();
    }
    else
    {
        DatasetInfo::m_pIntensityText->Hide();
        DatasetInfo::m_pSliderThresholdIntensity->Hide();
        m_pBtnIntensity->Show();
    }

    if( m_isOpacityToggled )
    {
        m_pBtnOpacity->Hide();
        DatasetInfo::m_pOpacityText->Show();
        DatasetInfo::m_pSliderOpacity->Show();
        m_pBtnIntensity->Disable();
        m_pBtnMinMaxLength->Disable();
        m_pBtnSubsampling->Disable();
        m_pToggleInterFibers->Disable();
        m_pBtnGeneratesDensityVolume->Disable();
        m_pBtnColorMode->Disable();
        m_pSliderOpacity->SetValue( getAlpha()*100 );
    }
    else
    {
        DatasetInfo::m_pOpacityText->Hide();
        DatasetInfo::m_pSliderOpacity->Hide();
        // Hide temporarily opacity functionality
        //m_pBtnOpacity->Show();
    }

    if( m_isMinMaxLengthToggled )
    {
        m_pBtnMinMaxLength->Hide();
        m_pLblMinLength->Show();
        m_pSliderFibersFilterMin->Show();
        m_pLblMaxLength->Show();
        m_pSliderFibersFilterMax->Show();
        m_pBtnIntensity->Disable();
        m_pBtnOpacity->Disable();
        m_pBtnSubsampling->Disable();
        m_pToggleInterFibers->Disable();
        m_pBtnGeneratesDensityVolume->Disable();
        m_pBtnColorMode->Disable();
    }
    else
    {
        m_pLblMinLength->Hide();
        m_pSliderFibersFilterMin->Hide();
        m_pLblMaxLength->Hide();
        m_pSliderFibersFilterMax->Hide();
        m_pBtnMinMaxLength->Show();
    }

    if( m_isSubsamplingToggled )
    {
        m_pBtnSubsampling->Hide();
        m_pLblSubsampling->Show();
        m_pSliderFibersSampling->Show();
        m_pBtnIntensity->Disable();
        m_pBtnOpacity->Disable();
        m_pBtnMinMaxLength->Disable();
        m_pToggleInterFibers->Disable();
        m_pBtnGeneratesDensityVolume->Disable();
        m_pBtnColorMode->Disable();
    }
    else
    {
        m_pLblSubsampling->Hide();
        m_pSliderFibersSampling->Hide();
        m_pBtnSubsampling->Show();
    }

    if( m_isCrossingFibersToggled )
    {
        m_pToggleInterFibers->Hide();
        m_pLblThickness->Show();
        m_pSliderInterFibersThickness->Show();
        m_pBtnIntensity->Disable();
        m_pBtnOpacity->Disable();
        m_pBtnMinMaxLength->Disable();
        m_pBtnSubsampling->Disable();
        m_pBtnGeneratesDensityVolume->Disable();
        m_pBtnColorMode->Disable();
    }
    else
    {
        m_pLblThickness->Hide();
        m_pSliderInterFibersThickness->Hide();
        m_pToggleInterFibers->SetValue( false );
        m_pToggleInterFibers->Show();
    }

    if( m_isColorModeToggled )
    {
        m_pToggleLocalColoring->Show();
        m_pToggleNormalColoring->Show();
        m_pLblColoring->Show();
        m_pRadNormalColoring->Show();
        m_pRadDistanceAnchoring->Show();
        m_pRadMinDistanceAnchoring->Show();
        m_pRadCurvature->Show();
        m_pRadTorsion->Show();
        m_pRadConstantColor->Show();
        m_pBtnColorMode->Hide();
        m_pBtnIntensity->Disable();
        m_pBtnOpacity->Disable();
        m_pBtnMinMaxLength->Disable();
        m_pBtnSubsampling->Disable();
        m_pToggleInterFibers->Disable();
        m_pBtnGeneratesDensityVolume->Disable();
    }
    else
    {
        m_pToggleLocalColoring->Hide();
        m_pToggleNormalColoring->Hide();
        m_pLblColoring->Hide();
        m_pRadNormalColoring->Hide();
        m_pRadDistanceAnchoring->Hide();
        m_pRadMinDistanceAnchoring->Hide();
        m_pRadCurvature->Hide();
        m_pRadTorsion->Hide();
        m_pRadConstantColor->Hide();
        m_pBtnColorMode->Show();
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
