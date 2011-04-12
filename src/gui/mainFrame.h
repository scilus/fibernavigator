/////////////////////////////////////////////////////////////////////////////
// Name:            mainFrame.h
// Author:          ---
// Creation Date:   ---
//
// Description: mainFrame class.
//
// Last modifications:
//      by : ggirard - 02-2011
/////////////////////////////////////////////////////////////////////////////

#ifndef MAINFRAME_H_
#define MAINFRAME_H_

#include <wx/checkbox.h>
#include <wx/grid.h>
#include <wx/treectrl.h>

#include "mainCanvas.h"
#include "myListCtrl.h"
#include "SelectionObject.h"


#include "../dataset/DatasetHelper.h"
#include "../dataset/datasetInfo.h"
#include "../misc/Algorithms/Helper.h"

class DatasetHelper;
class SelectionObject;
class DatasetInfo;
class ToolBar;
class MenuBar;
class FNObject;

class MainFrame : public wxFrame
{
    friend class ToolBar;
    friend class MenuBar;
    friend class DatasetInfo;
    friend class Anatomy;
    friend class Surface;
    friend class CIsoSurface;
    friend class Mesh;
    friend class Fibers;
    friend class Glyph;
    friend class Tensors;
    friend class ODFs;
    friend class SelectionObject;
    friend class SplinePoint;

public:
    MainFrame( wxWindow* i_parent, const wxWindowID i_id, const wxString& i_title, const wxPoint& i_pos, const wxSize& i_size, const long i_style);
    ~MainFrame();

    void DisplayPropertiesSheet();
    void deleteFNObject();
    void deleteListItem();
    void deleteTreeItem();
    void refreshAllGLWidgets();
    void renewAllGLWidgets();
    void Screenshot             ( wxString         i_fileName    );
    void SetGlyphOptionsValues  ( DatasetInfo*     i_tensors     );
	void OnTreeChange();
    void OnMouseEvent                       ( wxMouseEvent&   event );
    void OnLoad                             ( wxCommandEvent& event );
    
private:
    void OnDeleteListItem                   ( wxEvent& event );
    void OnToggleShowFS                     ( wxEvent& event );
    void OnFibersFilter                     ( wxCommandEvent& event);
    void OnNewIsoSurface                    ( wxCommandEvent& event );    
    void OnReloadShaders                    ( wxCommandEvent& event );
    void OnSave                             ( wxCommandEvent& event );
    void OnSaveFibers                       ( wxCommandEvent& event );
    void OnSaveSurface                      ( wxCommandEvent& event );
    void OnSaveDataset                      ( wxCommandEvent& event );
    void OnMinimizeDataset                  ( wxCommandEvent& event );
    void OnDilateDataset                    ( wxCommandEvent& event );
    void OnErodeDataset                     ( wxCommandEvent& event );
    void OnQuit                             ( wxCommandEvent& event );
    // View
    void OnMenuViewReset                    ( wxCommandEvent& event );
    void OnMenuViewLeft                     ( wxCommandEvent& event );
    void OnMenuViewRight                    ( wxCommandEvent& event );
    void OnMenuViewTop                      ( wxCommandEvent& event );
    void OnMenuViewBottom                   ( wxCommandEvent& event );
    void OnMenuViewFront                    ( wxCommandEvent& event );
    void OnMenuViewBack                     ( wxCommandEvent& event );
    void OnMenuViewCrosshair                ( wxCommandEvent& event );
    void OnMenuViewAxes                     ( wxCommandEvent& event );
    void OnToggleShowProperties             ( wxCommandEvent& event );
    // Voi
    void OnToggleSelectionObjects           ( wxCommandEvent& event );
    void OnToggleShowSelectionObject        ( wxCommandEvent& event );
    void OnNewSelectionBox                  ( wxCommandEvent& event );
    void OnNewSelectionEllipsoid            ( wxCommandEvent& event );
    void OnNewVoiFromOverlay                ( wxCommandEvent& event );
    void OnHideSelectionObjects             ( wxCommandEvent& event );
    void OnActivateSelectionObjects         ( wxCommandEvent& event );
    void OnRenameBox                        ( wxCommandEvent& event );
    void OnToggleAndNot                     ( wxCommandEvent& event );
    void OnColorRoi                         ( wxCommandEvent& event );
    void OnVoiFlipNormals                   ( wxCommandEvent& event );
    void OnUseMorph                         ( wxCommandEvent& event );
    void OnDisplayFibersInfo                ( wxCommandEvent& event );
    void OnDistanceAnchorSet                ( wxCommandEvent& event );
    void OnDisplayMeanFiber                 ( wxCommandEvent& event );
    void OnDisplayCrossSections             ( wxCommandEvent& event );
    void OnDisplayDispersionTube            ( wxCommandEvent& event );
    void OnColorWithCurvature               ( wxCommandEvent& event );
    void OnNormalColoring                   ( wxCommandEvent& event );
    void OnColorWithTorsion                 ( wxCommandEvent& event );
    void OnGenerateFiberVolume              ( wxCommandEvent& event );
    void OnCreateFibersColorTexture         ( wxCommandEvent& event );
    void OnCreateFibersDensityTexture       ( wxCommandEvent& event );
    // Spline Surface
    void OnNewSplineSurface                 ( wxCommandEvent& event );
    void OnToggleNormal                     ( wxCommandEvent& event );
    void OnToggleDrawPointsMode             ( wxCommandEvent& event );
    void OnToggleLIC                        ( wxCommandEvent& event );
    void OnToggleDrawVectors                ( wxCommandEvent& event );
    void OnNewOffsetSurface                 ( wxCommandEvent& event );
    void OnNewDistanceMap                   ( wxCommandEvent& event );
    // Options
    void OnAssignColor                      ( wxCommandEvent& event );
    void OnResetColor                       ( wxCommandEvent& event );
    void OnToggleLighting                   ( wxCommandEvent& event );
    void OnInvertFibers                     ( wxCommandEvent& event );
    void OnUseFakeTubes                     ( wxCommandEvent& event );
    void OnClearToBlack                     ( wxCommandEvent& event );
    void OnRulerTool                        ( wxCommandEvent& event );
    void OnRulerToolClear                   ( wxCommandEvent& event );
    void OnRulerToolAdd                     ( wxCommandEvent& event );
    void OnRulerToolDel                     ( wxCommandEvent& event );
    void OnUseTransparency                  ( wxCommandEvent& event );
    void OnToggleTextureFiltering           ( wxCommandEvent& event );
    void OnToggleBlendTexOnMesh             ( wxCommandEvent& event );
    void OnToggleFilterIso                  ( wxCommandEvent& event );
    void OnClean                            ( wxCommandEvent& event );
    void OnLoop                             ( wxCommandEvent& event );
    void OnToggleColorMapLegend             ( wxCommandEvent& event );
    
    void OnSetCMap0                         ( wxCommandEvent& event );
    void OnSetCMap1                         ( wxCommandEvent& event );
    void OnSetCMap2                         ( wxCommandEvent& event );
    void OnSetCMap3                         ( wxCommandEvent& event );
    void OnSetCMap4                         ( wxCommandEvent& event );
    void OnSetCMap5                         ( wxCommandEvent& event );
    void OnSetCMapNo                        ( wxCommandEvent& event );

    // Help
    void OnAbout                            ( wxCommandEvent& event );
    void OnShortcuts                        ( wxCommandEvent& event );
    void OnScreenshot                       ( wxCommandEvent& event );
    void OnSlizeMovieSag                    ( wxCommandEvent& event );
    void OnSlizeMovieCor                    ( wxCommandEvent& event );
    void OnSlizeMovieAxi                    ( wxCommandEvent& event );

    void OnSize                             ( wxSizeEvent&    event );    
    void OnGLEvent                          ( wxCommandEvent& event );
    
    void OnSliderMoved                      ( wxCommandEvent& event );
    void OnSliderIntensityThresholdMoved    ( wxCommandEvent& event );
    void OnSliderOpacityThresholdMoved      ( wxCommandEvent& event );
    
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
    void OnGlyphLightPositionChanged          ( AxisType i_axisType, float i_position );    
    void OnGlyphDisplaySliderMoved          ( wxCommandEvent& event ); // The display ratio.    
    void OnGlyphScalingFactorSliderMoved    ( wxCommandEvent& event ); // The scaling factor.    
    void OnGlyphNormalSelected              ( wxCommandEvent& event ); // The map on ellipsoid.    
    void OnGlyphMapOnSphereSelected         ( wxCommandEvent& event ); // The map on sphere.    
    void OnGlyphAxesSelected                ( wxCommandEvent& event ); // The display the 3 vector representing the ellipsoid.    
    void OnGlyphMainAxisSelected            ( wxCommandEvent& event ); // The display of the main vector of the ellipsoid.    
    void OnGlyphColorWithPosition           ( wxCommandEvent& event ); // The color with position.
	void OnNormalizeTensors                 ( wxCommandEvent& event ); //normalise tensors values;

    void OnToggleShowAxial                  ( wxCommandEvent& event );
    void OnToggleShowCoronal                ( wxCommandEvent& event );
    void OnToggleShowSagittal               ( wxCommandEvent& event );
    void OnToggleAlpha                      ( wxCommandEvent& event );
    void OnMoveBoundaryPointsLeft           ( wxCommandEvent& event );
    void OnMoveBoundaryPointsRight          ( wxCommandEvent& event );

    /*
     * List widget event functions
     */
    void OnActivateListItem                 ( wxListEvent&    event );
    void OnSelectListItem                   ( wxListEvent&    event );
    void OnListItemUp                       ( wxCommandEvent& event );
    void OnListItemDown                     ( wxCommandEvent& event );

    void OnListMenuName                     ( wxCommandEvent& event );
    void OnListMenuThreshold                ( wxCommandEvent& event );
    void OnListMenuShow                     ( wxCommandEvent& event );
    void OnListMenuCutOut                   ( wxCommandEvent& event );
    void OnListMenuDistance                 ( wxCommandEvent& event );
    void OnListMenuMinDistance              ( wxCommandEvent& event );

    /*
     * Tree widget event functions
     */
    void OnDeleteTreeItem                   ( wxTreeEvent&    event );
    void OnSelectTreeItem                   ( wxTreeEvent&    event );
    void OnRightClickTreeItem               ( wxTreeEvent&    event );
    void OnUnSelectTreeItem                 ( wxTreeEvent&    event );
    void OnActivateTreeItem                 ( wxTreeEvent&    event );
    void OnTreeLabelEdit                    ( wxTreeEvent&    event );
    int  treeSelected                       ( wxTreeItemId    i_id  );  
    void OnFiberFilterSlider                ( wxCommandEvent& event );
    /*
     * System functions
     */
    void OnKdTreeThreadFinished             ( wxCommandEvent& event );
    void updateStatusBar();
    void updateMenus();
    void OnTimerEvent                       ( wxTimerEvent&   event );

    void CreateNewSelectionObject( ObjectType i_newSelectionObjectType );
    void ColorFibers();

    void OnLoadDatasets                     ( wxCommandEvent& event );
    void OnLoadMeshes                       ( wxCommandEvent& event );
    void OnLoadFibers                       ( wxCommandEvent& event );
    void OnLoadTensors                      ( wxCommandEvent& event );
    void OnLoadODFs                         ( wxCommandEvent& event );
    bool loadIndex                          ( int i_index );
    
private:
    ToolBar             *m_toolBar;
    MenuBar             *m_menuBar;       
    wxBoxSizer          *m_currentSizer;
    wxBoxSizer          *m_noSelectionSizer;
    FNObject            *m_currentFNObject;
    FNObject            *m_lastSelectedFNObject;
    bool                m_isDisplayProperties; 
    long                m_currentListItem;
    long                m_lastSelectedListItem;

    wxBoxSizer          *m_mainSizer;
    wxBoxSizer          *m_mainSizer1;
    wxFlexGridSizer     *m_mainSizer2;
    wxBoxSizer          *m_leftMainSizer;
    wxBoxSizer          *m_leftSizer;
    wxBoxSizer          *m_navSizer;
    wxBoxSizer          *m_navSizer1;
    wxBoxSizer          *m_treeSizer;
    wxBoxSizer          *m_rightMainSizer;

    int                 m_enlargeNav;
    wxTimer             *m_timer;

public:
    MainCanvas*         m_mainGL;
    MainCanvas*         m_gl0;
    MainCanvas*         m_gl1;
    MainCanvas*         m_gl2;
    MyListCtrl*         m_listCtrl;
    MyTreeCtrl*         m_treeWidget;
    wxSlider*           m_xSlider;
    wxSlider*           m_ySlider;
    wxSlider*           m_zSlider;
    DatasetHelper*      m_datasetHelper;
    wxTreeItemId        m_tRootId;
    wxTreeItemId        m_tPointId;
    wxTreeItemId        m_tSelectionObjectsId;

DECLARE_EVENT_TABLE()
};

/*
 * Defines for interface items and other events
 */
#define KDTREE_EVENT                                270

#define ID_GL_NAV_X                                 271
#define ID_GL_NAV_Y                                 272
#define ID_GL_NAV_Z                                 273
#define ID_GL_MAIN                                  290

#define ID_LIST_CTRL                                291
#define ID_TREE_CTRL                                300

#define ID_X_SLIDER                                 301
#define ID_Y_SLIDER                                 302
#define ID_Z_SLIDER                                 303
 
#define TREE_CTRL_TOGGLE_ANDNOT                     351
#define TREE_CTRL_DELETE_BOX                        352
#define TREE_CTRL_TOGGLE_BOX_ACTIVE                 353
#define TREE_CTRL_TOGGLE_BOX_SHOW                   370

#endif /*MAINFRAME_H_*/
