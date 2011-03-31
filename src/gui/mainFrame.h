/////////////////////////////////////////////////////////////////////////////
// Name:            mainFrame.h
// Author:          ---
// Creation Date:   ---
//
// Description: mainFrame class.
//
// Last modifications:
//      by : Imagicien - 12/11/2009
/////////////////////////////////////////////////////////////////////////////

#ifndef MAINFRAME_H_
#define MAINFRAME_H_

#include "mainCanvas.h"
#include "myListCtrl.h"
#include "SelectionObject.h"
#include "wx/grid.h"
#include "wx/treectrl.h"
#include "../dataset/DatasetHelper.h"
#include "../dataset/datasetInfo.h"
#include "../misc/Algorithms/Helper.h"

class DatasetHelper;
class SelectionObject;
class DatasetInfo;

// Define a new frame
class MainFrame : public wxFrame
{
public:
    // Constructor/Destructor
    MainFrame( wxWindow* i_parent, const wxWindowID i_id, const wxString& i_title, const wxPoint& i_pos, const wxSize& i_size, const long i_style );
    ~MainFrame();

    void DisplayFibersInfo      ( bool i_display );
    void DisplayGlyphOptions    ( bool i_display,  DatasetInfo* i_glyph );
    void refreshAllGLWidgets();
    void renewAllGLWidgets();
    void Screenshot             ( wxString         i_fileName    );
    void SetFiberInfoGridValues ( SelectionObject* i_selectedBox );
    void SetGlyphOptionsValues  ( DatasetInfo*     i_tensors     );
    void setTSlider             ( MySlider*        i_slider      )  { m_tSlider = i_slider; };
    
private:
	/*
	 * Menu Functions
	 */
	// File
	void OnNewIsoSurface                    ( wxCommandEvent& event );
	void OnLoad                             ( wxCommandEvent& event );
	void OnReloadShaders                    ( wxCommandEvent& event );
	void OnSave                             ( wxCommandEvent& event );
	void OnSaveFibers                       ( wxCommandEvent& event );
	void OnSaveSurface                      ( wxCommandEvent& event );
	void OnSaveDataset                      ( wxCommandEvent& event );
	void OnMinimizeDataset                  ( wxCommandEvent& event );
	void OnDilateDataset                    ( wxCommandEvent& event );
	void OnErodeDataset                     ( wxCommandEvent& event );
	void OnQuit                             ( wxCommandEvent& event );
	void OnToggleLayout                     ( wxCommandEvent& event );
	// View
	void OnMenuViewReset                    ( wxCommandEvent& event );
	void OnMenuViewLeft                     ( wxCommandEvent& event );
	void OnMenuViewRight                    ( wxCommandEvent& event );
	void OnMenuViewTop                      ( wxCommandEvent& event );
	void OnMenuViewBottom                   ( wxCommandEvent& event );
	void OnMenuViewFront                    ( wxCommandEvent& event );
	void OnMenuViewBack                     ( wxCommandEvent& event );
	void OnMenuViewCrosshair                ( wxCommandEvent& event );
	// Voi
    void OnToggleSelectionObjects           ( wxCommandEvent& event );
    void OnToggleShowSelectionObject        ( wxCommandEvent& event );
    void OnNewSelectionBox                  ( wxCommandEvent& event );
    void OnNewSelectionEllipsoid            ( wxCommandEvent& event );
    void OnNewFromOverlay                   ( wxCommandEvent& event );
    void OnHideSelectionObjects             ( wxCommandEvent& event );
    void OnRenameBox                        ( wxCommandEvent& event );
    void OnToggleAndNot                     ( wxCommandEvent& event );
    void OnColorRoi                         ( wxCommandEvent& event );
    void OnUseMorph                         ( wxCommandEvent& event );
	void OnDisplayFibersInfo                ( wxCommandEvent& event );
    void OnDisplayMeanFiber                 ( wxCommandEvent& event );
    void OnDisplayCrossSections             ( wxCommandEvent& event );
    void OnDisplayDispersionTube            ( wxCommandEvent& event );
    void OnColorWithCurvature               ( wxCommandEvent& event );
    void OnColorWithTorsion                 ( wxCommandEvent& event );
    void OnCreateColorTexture               ( wxCommandEvent& event );
	// Spline Surface
	void OnNewSurface                       ( wxCommandEvent& event );
	void OnToggleNormal                     ( wxCommandEvent& event );
	void OnTogglePointMode                  ( wxCommandEvent& event );
	void OnToggleLIC                        ( wxCommandEvent& event );
	void OnToggleDrawVectors                ( wxCommandEvent& event );
	void OnNewOffsetMap                     ( wxCommandEvent& event );
	// Options
	void OnAssignColor                      ( wxCommandEvent& event );
	void OnResetColor                       ( wxCommandEvent& event );
    void OnToggleLighting                   ( wxCommandEvent& event );
    void OnInvertFibers                     ( wxCommandEvent& event );
    void OnUseFakeTubes                     ( wxCommandEvent& event );
    void OnClearToBlack                     ( wxCommandEvent& event );
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

	/*
	 * Window Functions
	 */
	void OnSize                             ( wxSizeEvent&    event );
    void OnMouseEvent                       ( wxMouseEvent&   event );
    void OnGLEvent                          ( wxCommandEvent& event );

    void OnTSliderMoved                     ( wxCommandEvent& event );
    void OnTSlider2Moved                    ( wxCommandEvent& event );
    void OnSliderMoved                      ( wxCommandEvent& event );

    // Glyph options
    // The coloration.
    void OnGlyphMinHueSliderMoved           ( wxCommandEvent& event );
    void OnGlyphMaxHueSliderMoved           ( wxCommandEvent& event );
    void OnGlyphSaturationSliderMoved       ( wxCommandEvent& event );
    void OnGlyphLuminanceSliderMoved        ( wxCommandEvent& event );
    void updateGlyphColoration              ( GlyphColorModifier i_modifier, float i_value );
    // The axis flip.
    void OnGlyphXAxisFlipChecked            ( wxCommandEvent& event );
    void OnGlyphYAxisFlipChecked            ( wxCommandEvent& event );
    void OnGlyphZAxisFlipChecked            ( wxCommandEvent& event );
    void OnGlyphFlip                        ( AxisType i_axisType, bool i_isChecked );
    // The lod.
    void OnGlyphLODSliderMoved              ( wxCommandEvent& event );
    // The light attenuation.
    void OnGlyphLightAttenuationSliderMoved ( wxCommandEvent& event );
    // The light attenuation.
    void OnGlyphLightXDirectionSliderMoved  ( wxCommandEvent& event );
    void OnGlyphLightYDirectionSliderMoved  ( wxCommandEvent& event );
    void OnGlyphLightZDirectionSliderMoved  ( wxCommandEvent& event );
    void glyphLightPositionChanged          ( AxisType i_axisType, float i_position );
    // The display ratio.
    void OnGlyphDisplaySliderMoved          ( wxCommandEvent& event );
    // The scaling factor.
    void OnGlyphScalingFactorSliderMoved    ( wxCommandEvent& event );
    // The map on ellipsoid.
    void OnGlyphNormalSelected         ( wxCommandEvent& event );
    // The map on sphere.
    void OnGlyphMapOnSphereSelected         ( wxCommandEvent& event );
    // The display the 3 vector representing the ellipsoid.
    void OnGlyphAxesSelected         ( wxCommandEvent& event );
    // The display of the main vector of the ellipsoid.
    void OnGlyphMainAxisSelected         ( wxCommandEvent& event );
    // The color with position.
    void OnGlyphColorWithPosition           ( wxCommandEvent& event );

    /*
     * Button functions
     */
    void OnButtonAxial                      ( wxCommandEvent& event );
    void OnButtonCoronal                    ( wxCommandEvent& event );
    void OnButtonSagittal                   ( wxCommandEvent& event );
    void OnToggleAlpha                      ( wxCommandEvent& event );
    void OnMovePoints1                      ( wxCommandEvent& event );
    void OnMovePoints2                      ( wxCommandEvent& event );

    /*
     * List widget event functions
     */
    void OnActivateListItem                 ( wxListEvent&    event );
    void OnRightClickListItem               ( wxListEvent&    event );
    void OnSelectListItem                   ( wxListEvent&    event );
    void OnListItemUp                       ( wxCommandEvent& event );
    void OnListItemDown                     ( wxCommandEvent& event );

    void OnListMenuName                     ( wxCommandEvent& event );
    void OnListMenuThreshold                ( wxCommandEvent& event );
    void OnListMenuDelete                   ( wxCommandEvent& event );
    void OnListMenuShow                     ( wxCommandEvent& event );
    void OnListMenuShowOptions              ( wxCommandEvent& event );
    void OnListMenuHideOptions              ( wxCommandEvent& event );
    void OnListMenuCutOut                   ( wxCommandEvent& event );

    /*
     * Tree widget event functions
     */
    void OnDeleteTreeItem                   ( wxTreeEvent&    event );
    void OnSelectTreeItem                   ( wxTreeEvent&    event );
	void OnRightClickTreeItem               ( wxTreeEvent&    event );
    void OnUnSelectTreeItem                 ( wxTreeEvent&    event );
    void OnActivateTreeItem                 ( wxTreeEvent&    event );
    void OnTreeEvent                        ( wxCommandEvent& event );
    void OnTreeLabelEdit                    ( wxTreeEvent&    event );
    int  treeSelected                       ( wxTreeItemId    i_id  );
    void OnLoadDatasets                     ( wxCommandEvent& event );
    void OnLoadMeshes                       ( wxCommandEvent& event );
    void OnLoadFibers                       ( wxCommandEvent& event );
    void OnLoadTensors                      ( wxCommandEvent& event );
    void OnLoadODFs                         ( wxCommandEvent& event );
    void OnLoadTestFibers                   ( wxCommandEvent& event );
    bool loadIndex                          ( int             i_index );

    /*
     * System functions
     */
    void OnKdTreeThreadFinished             ( wxCommandEvent& event );
    void updateStatusBar();
    void updateMenus();
    void OnTimerEvent                       ( wxTimerEvent&   event );

    void CreateNewSelectionObject( ObjectType i_newSelectionObjectType );

    // Functions use for the fiber info sizer.
    void ColorFibers();
    void DefaultFillMenu     ( SelectionObject* i_selBox, wxMenu* o_menu );
	void FillMasterBoxMenu   ( SelectionObject* i_selBox, wxMenu* o_menu );
	void FillChildBoxMenu    ( SelectionObject* i_selBox, wxMenu* o_menu );
    void InitializeFibersInfoSizerElements();
    void SetFiberInfoGridLabelTitlesValues();
    void SetFiberInfoGridName( wxString i_name );

    // Functions use for the glyph options sizer
    void InitializeGlpyhOptionsSizerElements( DatasetInfo* i_tensors );

public:
	MainCanvas*         m_gl0;
	MainCanvas*         m_gl1;
	MainCanvas*         m_gl2;
	MainCanvas*         m_mainGL;

	wxSlider*           m_xSlider;
    wxSlider*           m_ySlider;
    wxSlider*           m_zSlider;
    MySlider*           m_tSlider;
    wxSlider*           m_tSlider2;

    wxButton*           m_buttonUp;
    wxButton*           m_buttonDown;
    wxButton*           m_buttonLoadDatasets;
    wxButton*           m_buttonLoadMeshes;
    wxButton*           m_buttonLoadFibers;
    wxButton*           m_buttonLoadTensors;
    wxButton*           m_buttonLoadODFs;
    wxButton*           m_buttonLoadTestFibers;

	MyListCtrl*         m_listCtrl;
	wxTreeCtrl*         m_treeWidget;
	wxTreeItemId        m_tRootId;
	wxTreeItemId        m_tPointId;
	wxTreeItemId        m_tSelectionObjectsId;

	wxBoxSizer*         m_mainSizer;
	wxBoxSizer*         m_mainSizer1;
	wxFlexGridSizer*    m_mainSizer2;
	wxBoxSizer*         m_leftMainSizer;
	wxBoxSizer*         m_leftSizer;
	wxBoxSizer*         m_navSizer;
	wxBoxSizer*         m_navSizer1;
	wxBoxSizer*         m_buttonSizer;
	wxBoxSizer*         m_treeSizer;
	wxBoxSizer*         m_rightMainSizer;
	wxBoxSizer*         m_fibersInfoSizer;
    wxBoxSizer*         m_glyphOptionsSizer;

	int                 m_enlargeNav;
	wxTimer*            m_timer;

    // Items related to the fiber info sizer.
	wxGrid*             m_fibersInfoGrid;
    wxButton*           m_buttonDisplayMeanFiber;
    wxButton*           m_buttonDisplayCrossSections;
    wxButton*           m_buttonDisplayDispersionTube;
    wxButton*           m_buttonColorWithCurvature;
    wxButton*           m_buttonColorWithTorsion;

    // Items related to the glyph options sizer.
    wxSlider*           m_glyphMinHueValue;
    wxSlider*           m_glyphMaxHueValue;
    wxSlider*           m_glyphSaturationValue;
    wxSlider*           m_glyphLuminanceValue;
    wxSlider*           m_glyphLODValue;
    wxSlider*           m_glyphLightAttenuation;
    wxSlider*           m_glyphLightXPosition;
    wxSlider*           m_glyphLightYPosition;
    wxSlider*           m_glyphLightZPosition;
    wxSlider*           m_glyphDisplayValue;
    wxSlider*           m_glyphScalingFactor;
    wxCheckBox*         m_xAxisFlip;
    wxCheckBox*         m_yAxisFlip;
    wxCheckBox*         m_zAxisFlip;
    wxCheckBox*         m_colorWithPosition;
    wxStaticText*       m_minHueSliderText;
    wxStaticText*       m_maxHueSliderText;
    wxStaticText*       m_saturationSliderText;
    wxStaticText*       m_luminanceSliderText;
    wxStaticText*       m_LODSliderText;
    wxStaticText*       m_lightAttenuationSliderText;
    wxStaticText*       m_lightXPositionSliderText;
    wxStaticText*       m_lightYPositionSliderText;
    wxStaticText*       m_lightZPositionSliderText;
    wxStaticText*       m_displayRatioSliderText;
    wxStaticText*       m_scalingFactorText;
    wxRadioButton*      m_radioNormal;
    wxRadioButton*      m_radioMapOnSphere;
    wxRadioButton*      m_radioAxes;
    wxRadioButton*      m_radioMainAxis;

	DatasetHelper*      m_datasetHelper;

DECLARE_EVENT_TABLE()
};

/*
 * Defines for Menu Events
 */
//Menu File
#define MENU_FILE_NEW_ISOSURF						100
#define MENU_FILE_LOAD								101
#define MENU_FILE_RELOAD_SHADER						102
#define MENU_FILE_SAVE								103
#define MENU_FILE_SAVE_FIBERS						104
#define MENU_FILE_SAVE_SURFACE						105
#define MENU_FILE_QUIT        						wxID_EXIT //5006
#define MENU_FILE_SAVE_DATASET						106
#define MENU_FILE_MINIMIZE_DATASET					107
#define MENU_FILE_DILATE_DATASET					108
#define MENU_FILE_ERODE_DATASET						109
#define BUTTON_TOGGLE_LAYOUT						120

// Menu View
#define MENU_VIEW_LEFT								121
#define MENU_VIEW_RIGHT								122
#define MENU_VIEW_FRONT								123
#define MENU_VIEW_BACK								124
#define MENU_VIEW_TOP								125
#define MENU_VIEW_BOTTOM							126
#define MENU_VIEW_RESET								127
#define MENU_VIEW_SHOW_CROSSHAIR					140
// Menu Voi
#define MENU_VOI_NEW_SELECTION_OBJECT               141
#define MENU_VOI_NEW_SELECTION_OBJECT_BOX		    142
#define MENU_VOI_NEW_SELECTION_OBJECT_ELLIPSOID    	143
#define MENU_VOI_RENDER_SELECTION_OBJECTS		    144
#define MENU_VOI_TOGGLE_SELECTION_OBJECTS           145
#define MENU_VOI_TOGGLE_VISIBLE_SELECTION_OBJECT    146
#define MENU_VOI_RENAME_BOX							147
#define MENU_VOI_NEW_FROM_OVERLAY					148
#define MENU_VOI_TOGGLE_ANDNOT						149
#define MENU_VOI_COLOR_ROI							150
#define MENU_VOI_USE_MORPH							151
#define MENU_VOI_DISPLAY_FIBERS_INFO                152
#define MENU_VOI_CREATE_TEXTURE                     170
// Menu Spline Surface
#define MENU_SPLINESURF_DRAW_POINTS					171
#define MENU_SPLINESURF_NEW							172
#define MENU_SPLINESURF_TOGGLE_LIC					173
#define MENU_SPLINESURF_TOGGLE_NORMAL				174
#define MENU_SPLINESURF_DRAW_VECTORS				175
#define MENU_SURFACE_NEW_OFFSET						180
// Menu Options
#define MENU_OPTIONS_ASSIGN_COLOR					181
#define MENU_OPTIONS_TOGGLE_LIGHTING				182
#define MENU_OPTIONS_INVERT_FIBERS					183
#define MENU_OPTIONS_TOGGLE_TEXTURE_FILTERING		184
#define MENU_OPTIONS_BLEND_TEX_ON_MESH				185
#define MENU_OPTIONS_USE_FAKE_TUBES					186
#define MENU_OPTIONS_FILTER_ISO						187
#define MENU_OPTIONS_USE_TRANSPARENCY				188
#define MENU_OPTIONS_COLOR_MAPS						189
#define MENU_OPTIONS_CMAP0							190
#define MENU_OPTIONS_CMAP1							191
#define MENU_OPTIONS_CMAP2							192
#define MENU_OPTIONS_CMAP3							193
#define MENU_OPTIONS_CMAP4							194
#define MENU_OPTIONS_CMAP5							195
#define MENU_OPTIONS_RESET_COLOR					196
#define MENU_OPTIONS_CLEAN							197
#define MENU_OPTIONS_LOOP							198
#define MENU_OPTIONS_CMAP_LEGEND					199
#define MENU_OPTIONS_CMAPNO							200
#define MENU_OPTIONS_CLEAR_TO_BLACK                 220
// Menu Help
#define MENU_HELP_ABOUT       						wxID_ABOUT //5015
#define MENU_HELP_SHORTCUTS    						221
#define MENU_HELP_SCREENSHOT   						222
#define MENU_HELP_SLIZEMOVIE                        223
#define MENU_HELP_SLIZEMOVIESAG                     224
#define MENU_HELP_SLIZEMOVIECOR                     225
#define MENU_HELP_SLIZEMOVIEAXI                     230
/*
 * Defines for Buttons, will move into Menus eventually
 */
#define BUTTON_AXIAL						 		231
#define BUTTON_CORONAL 								232
#define BUTTON_SAGITTAL 							233
#define BUTTON_TOGGLE_ALPHA 						234
#define BUTTON_MOVE_POINTS1 						235
#define BUTTON_MOVE_POINTS2 						236
#define BUTTON_LOAD_DATASETS						237
#define BUTTON_LOAD_MESHES						    238
#define BUTTON_LOAD_FIBERS						    239
#define BUTTON_LOAD_TENSORS						    240
#define BUTTON_LOAD_ODFS						    241
#define BUTTON_LOAD_TEST_FIBERS                     242
#define BUTTON_DISPLAY_MEAN_FIBER                   243
#define BUTTON_DISPLAY_CROSS_SECTIONS               244
#define BUTTON_DISPLAY_DISPERSION_TUBE              245
#define BUTTON_COLOR_WITH_CURVATURE                 246
#define BUTTON_COLOR_WITH_TORSION                   250

/*
 * Menu entries for right click on list widget
 */
#define	MENU_LIST_DELETE                            251
#define MENU_LIST_TOGGLE_SHOW                       252
#define MENU_LIST_TOGGLE_COLOR                      253
#define MENU_LIST_TOGGLE_NAME                       254
#define MENU_LIST_SHOW_OPTIONS                      255
#define MENU_LIST_HIDE_OPTIONS                      256
#define MENU_LIST_CUTOUT                            260
/*
 * Defines for interface items and other events
 */
#define KDTREE_EVENT								270

#define ID_GL_NAV_X 								271
#define ID_GL_NAV_Y  								272
#define ID_GL_NAV_Z   								273
#define ID_GL_MAIN									290

#define ID_LIST_CTRL								291
#define ID_TREE_CTRL								300

#define ID_X_SLIDER 								301
#define ID_Y_SLIDER 								302
#define ID_Z_SLIDER 								303
#define ID_T_SLIDER									304
#define ID_T_SLIDER2								305
#define ID_GLYPH_OPTIONS_MIN_HUE_SLIDER             306
#define ID_GLYPH_OPTIONS_MAX_HUE_SLIDER             307
#define ID_GLYPH_OPTIONS_SATURATION_SLIDER          308
#define ID_GLYPH_OPTIONS_LUMINANCE_SLIDER           309
#define ID_GLYPH_OPTIONS_LOD_SLIDER                 310
#define ID_GLYPH_OPTIONS_LIGHT_ATTENUATION_SLIDER   311
#define ID_GLYPH_OPTIONS_LIGHT_X_POSITION_SLIDER    312
#define ID_GLYPH_OPTIONS_LIGHT_Y_POSITION_SLIDER    313
#define ID_GLYPH_OPTIONS_LIGHT_Z_POSITION_SLIDER    314
#define ID_GLYPH_OPTIONS_DISPLAY_SLIDER             315
#define ID_GLYPH_OPTIONS_SCALING_FACTOR_SLIDER      316
#define ID_GLYPH_OPTIONS_X_AXIS_FLIP                317
#define ID_GLYPH_OPTIONS_Y_AXIS_FLIP                318
#define ID_GLYPH_OPTIONS_Z_AXIS_FLIP                319
#define ID_GLYPH_OPTIONS_COLOR_WITH_POSITION        321
#define ID_GLYPH_OPTIONS_SLIDER_TEXT                322
#define ID_GLYPH_OPTIONS_RADIO_NORMAL               323
#define ID_GLYPH_OPTIONS_RADIO_MAP_ON_SPHERE        324
#define ID_GLYPH_OPTIONS_RADIO_AXES                 325
#define ID_GLYPH_OPTIONS_RADIO_MAIN_AXIS            326



#define ID_FIBERS_INFO_GRID							330

#define ID_BUTTON_UP 								331
#define ID_BUTTON_DOWN 								350

#define TREE_CTRL_TOGGLE_ANDNOT						351
#define TREE_CTRL_DELETE_BOX						352
#define TREE_CTRL_TOGGLE_BOX_ACTIVE					353
#define TREE_CTRL_TOGGLE_BOX_SHOW					370

#endif /*MAINFRAME_H_*/
