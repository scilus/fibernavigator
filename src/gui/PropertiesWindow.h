/////////////////////////////////////////////////////////////////////////////
// Name:            propertiesWindow.cpp
// Author:          ---
// Creation Date:   ---
//
// Description: This is the implementation file for the properties window.
//
// Last modifications:
//      by : ggirard - 06-2011
/////////////////////////////////////////////////////////////////////////////


#ifndef PROPERTIESWINDOW_H_
#define PROPERTIESWINDOW_H_

#include <wx/scrolwin.h>
#include <wx/treectrl.h>

#include "../misc/Algorithms/Helper.h"

class MainFrame;
class PropertiesWindow: public wxScrolledWindow
{
public:
    PropertiesWindow(){};
    PropertiesWindow( MainFrame *parent, wxWindowID id, const wxPoint &pos, const wxSize &size );
    ~PropertiesWindow(){};
    void OnPaint( wxPaintEvent &event );
    void OnSize( wxSizeEvent &event ); 

    void OnListItemUp                       ( wxCommandEvent& event );
    void OnListItemDown                     ( wxCommandEvent& event );
    void OnListItemShow                     ( wxCommandEvent& event );
    void OnListItemCutOut                   ( wxCommandEvent& event );

    void OnDeleteListItem                   ( wxEvent& event );

    void OnToggleShowFS                     ( wxEvent& event );

	void OnToggleIntensityBtn				( wxEvent& event );
	void OnToggleOpacityBtn					( wxEvent& event );
	void OnToggleMinMaxLengthBtn			( wxEvent& event );
	void OnToggleSubsamplingBtn				( wxEvent& event );
	void OnToggleColorModeBtn				( wxEvent& event );
	void OnToggleLocalColoringBtn			( wxEvent& event );
	void OnToggleNormalColoringBtn			( wxEvent& event );
	
	void OnClickApplyBtn					( wxEvent& event );
	void OnClickCancelBtn					( wxEvent& event );

    void OnSliderIntensityThresholdMoved    ( wxCommandEvent& event );
    void OnSliderOpacityThresholdMoved      ( wxCommandEvent& event );

	void OnEqualizeDataset					( wxEvent& event );
    void OnRename                           ( wxCommandEvent& event );
    void OnFlipX                            ( wxCommandEvent& event );
    void OnFlipY                            ( wxCommandEvent& event );
    void OnFlipZ                            ( wxCommandEvent& event );

    void OnDilateDataset                    ( wxCommandEvent& event );
    void OnErodeDataset                     ( wxCommandEvent& event );
    void OnMinimizeDataset                  ( wxCommandEvent& event );

    void OnNewOffsetSurface                 ( wxCommandEvent& event );
    void OnNewDistanceMap                   ( wxCommandEvent& event );
    void OnNewVoiFromOverlay                ( wxCommandEvent& event );
    void OnNewIsoSurface                    ( wxCommandEvent& event ); 

    void OnSegment                          ( wxCommandEvent& event );
    void OnFloodFill                        ( wxCommandEvent& event );
    void OnSliderFloodMoved                 ( wxCommandEvent& event );
    void OnSelectBck                        ( wxCommandEvent& event );
    void OnSliderGraphSigmaMoved            ( wxCommandEvent& event );
    void OnKmeans                           ( wxCommandEvent& event );
    void OnbtnGraphCut                      ( wxCommandEvent& event );
    void OnSelectObj                        ( wxCommandEvent& event );

    void OnClean                            ( wxCommandEvent& event );
    void OnLoop                             ( wxCommandEvent& event );
    void OnToggleLIC                        ( wxCommandEvent& event );

    void OnToggleDrawPointsMode             ( wxCommandEvent& event );
    void OnMoveBoundaryPointsLeft           ( wxCommandEvent& event );
    void OnMoveBoundaryPointsRight          ( wxCommandEvent& event );
    
    void OnFibersFilter                     ( wxCommandEvent& event );
    void OnGenerateFiberVolume              ( wxCommandEvent& event );
    void OnListMenuThreshold                ( wxCommandEvent& event );
    void OnListMenuDistance                 ( wxCommandEvent& event );
    void OnListMenuMinDistance              ( wxCommandEvent& event );
    void OnColorWithCurvature               ( wxCommandEvent& event );
    void OnNormalColoring                   ( wxCommandEvent& event );
    void OnColorWithTorsion                 ( wxCommandEvent& event );
    void ColorFibers();

    void OnGlyphMinHueSliderMoved           ( wxCommandEvent& event ); // The coloration.
    void OnGlyphMaxHueSliderMoved           ( wxCommandEvent& event ); // The coloration.
    void OnGlyphSaturationSliderMoved       ( wxCommandEvent& event ); // The coloration.
    void OnGlyphLuminanceSliderMoved        ( wxCommandEvent& event ); // The coloration.
    void updateGlyphColoration              ( GlyphColorModifier i_modifier, float i_value );    
    void OnGlyphXAxisFlipChecked            ( wxCommandEvent& event ); // The axis flip.
    void OnGlyphYAxisFlipChecked            ( wxCommandEvent& event ); // The axis flip.
    void OnGlyphZAxisFlipChecked            ( wxCommandEvent& event ); // The axis flip.
    void OnGlyphFlip                        ( AxisType i_axisType, bool i_isChecked );    
    void OnGlyphLODSliderMoved              ( wxCommandEvent& event ); // The lod.    
    void OnGlyphLightAttenuationSliderMoved ( wxCommandEvent& event ); // The light attenuation.    
    void OnGlyphLightXDirectionSliderMoved  ( wxCommandEvent& event ); // The light attenuation.
    void OnGlyphLightYDirectionSliderMoved  ( wxCommandEvent& event ); // The light attenuation.
    void OnGlyphLightZDirectionSliderMoved  ( wxCommandEvent& event ); // The light attenuation.
    void OnGlyphLightPositionChanged        ( AxisType i_axisType, float i_position );    
    void OnGlyphDisplaySliderMoved          ( wxCommandEvent& event ); // The display ratio.    
    void OnGlyphScalingFactorSliderMoved    ( wxCommandEvent& event ); // The scaling factor.    
    void OnGlyphNormalSelected              ( wxCommandEvent& event ); // The map on ellipsoid.    
    void OnGlyphMapOnSphereSelected         ( wxCommandEvent& event ); // The map on sphere.    
    void OnGlyphAxesSelected                ( wxCommandEvent& event ); // The display the 3 vector representing the ellipsoid.    
    void OnGlyphMainAxisSelected            ( wxCommandEvent& event ); // The display of the main vector of the ellipsoid.    
    void OnGlyphColorWithPosition           ( wxCommandEvent& event ); // The color with position.
    
    void OnNormalizeTensors                 ( wxCommandEvent& event ); //normalise tensors values;

    void OnOriginalShBasis                  ( wxCommandEvent& event );
    void OnDescoteauxShBasis                ( wxCommandEvent& event );
    void OnTournierShBasis                  ( wxCommandEvent& event );
    void OnPTKShBasis                       ( wxCommandEvent& event );
    
    void OnToggleShowSelectionObject        ( wxCommandEvent& event );
    void OnDeleteTreeItem                   ( wxTreeEvent&    event );
    void OnActivateTreeItem                 ( wxTreeEvent&    event );
    void OnRenameBox                        ( wxCommandEvent& event );
    void OnToggleAndNot                     ( wxCommandEvent& event );
    void OnColorRoi                         ( wxCommandEvent& event );
    void OnVoiFlipNormals                   ( wxCommandEvent& event );
    void OnAssignColor                      ( wxCommandEvent& event );
    void OnDistanceAnchorSet                ( wxCommandEvent& event );
    void OnDisplayFibersInfo                ( wxCommandEvent& event );
    void OnDisplayMeanFiber                 ( wxCommandEvent& event );
    void OnDisplayCrossSections             ( wxCommandEvent& event );
    void OnDisplayDispersionTube            ( wxCommandEvent& event );
    void OnCreateFibersColorTexture         ( wxCommandEvent& event );
    void OnCreateFibersDensityTexture       ( wxCommandEvent& event );
    void OnBoxPositionX                     ( wxCommandEvent& event );
    void OnBoxPositionY                     ( wxCommandEvent& event );
    void OnBoxPositionZ                     ( wxCommandEvent& event );
    void OnBoxSizeX                         ( wxCommandEvent& event );
    void OnBoxSizeY                         ( wxCommandEvent& event );
    void OnBoxSizeZ                         ( wxCommandEvent& event );
    void OnSliderAxisMoved                  ( wxCommandEvent& event );
    void OnRecalcMainDir                    ( wxCommandEvent& event );

    MainFrame *m_mainFrame;

private:
    wxSizer *propertiesSizer;
    PropertiesWindow(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size );
    DECLARE_DYNAMIC_CLASS(PropertiesWindow)
    DECLARE_EVENT_TABLE()
};

#endif /*PROPERTUIESWINDOW*/