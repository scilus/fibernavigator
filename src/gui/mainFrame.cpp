/////////////////////////////////////////////////////////////////////////////
// Name:            mainFrame.cpp
// Author:          ---
// Creation Date:   ---
//
// Description: This is the implementation file for mainFrame class.
//
// Last modifications:
//      by : Imagicien - 12/11/2009
/////////////////////////////////////////////////////////////////////////////

#include "mainFrame.h"

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "mainCanvas.h"
#include "SelectionBox.h"
#include "SelectionEllipsoid.h"
#include "wx/colordlg.h"
#include "wx/filedlg.h"
#include "wx/statbmp.h"
#include "../dataset/Anatomy.h"
#include "../dataset/fibers.h"
#include "../dataset/ODFs.h"
#include "../dataset/splinePoint.h"
#include "../dataset/surface.h"
#include "../dataset/Tensors.h"
#include "../icons/delete.xpm"
#include "../icons/eyes.xpm"
#include "../gfx/theScene.h"
#include "../main.h"
#include "../misc/IsoSurface/CIsoSurface.h"

#define FIBERS_INFO_GRID_COL_SIZE              1
#define FIBERS_INFO_GRID_ROW_SIZE              11
#define FIBERS_INFO_GRID_TITLE_LABEL_SIZE      150

extern const wxEventType wxEVT_NAVGL_EVENT;

DECLARE_EVENT_TYPE( wxEVT_TREE_EVENT, -1 )
/****************************************************************************************************
 *
 * Event Table
 *
 ****************************************************************************************************/
BEGIN_EVENT_TABLE( MainFrame, wxFrame )
/*
 * Menu events
 */
// Menu File
EVT_MENU( MENU_FILE_NEW_ISOSURF,                            MainFrame::OnNewIsoSurface   )
EVT_MENU( MENU_FILE_LOAD,                                   MainFrame::OnLoad            )
EVT_MENU( MENU_FILE_RELOAD_SHADER,                          MainFrame::OnReloadShaders   )
EVT_MENU( MENU_FILE_SAVE,                                   MainFrame::OnSave            )
EVT_MENU( MENU_FILE_SAVE_FIBERS,                            MainFrame::OnSaveFibers      )
EVT_MENU( MENU_FILE_SAVE_SURFACE,                           MainFrame::OnSaveSurface     )
EVT_MENU( MENU_FILE_SAVE_DATASET,                           MainFrame::OnSaveDataset     )
EVT_MENU( MENU_FILE_MINIMIZE_DATASET,                       MainFrame::OnMinimizeDataset )
EVT_MENU( MENU_FILE_DILATE_DATASET,                         MainFrame::OnDilateDataset   )
EVT_MENU( MENU_FILE_ERODE_DATASET,                          MainFrame::OnErodeDataset    )
EVT_MENU( MENU_FILE_QUIT,                                   MainFrame::OnQuit            )
EVT_MENU( BUTTON_TOGGLE_LAYOUT,                             MainFrame::OnToggleLayout    )
// Menu View
EVT_MENU( MENU_VIEW_RESET,                                  MainFrame::OnMenuViewReset     )
EVT_MENU( MENU_VIEW_LEFT,                                   MainFrame::OnMenuViewLeft      )
EVT_MENU( MENU_VIEW_RIGHT,                                  MainFrame::OnMenuViewRight     )
EVT_MENU( MENU_VIEW_TOP,                                    MainFrame::OnMenuViewTop       )
EVT_MENU( MENU_VIEW_BOTTOM,                                 MainFrame::OnMenuViewBottom    )
EVT_MENU( MENU_VIEW_FRONT,                                  MainFrame::OnMenuViewFront     )
EVT_MENU( MENU_VIEW_BACK,                                   MainFrame::OnMenuViewBack      )
EVT_MENU( MENU_VIEW_SHOW_CROSSHAIR,                         MainFrame::OnMenuViewCrosshair )
// Menu VOI
EVT_MENU( MENU_VOI_NEW_SELECTION_OBJECT_BOX,                MainFrame::OnNewSelectionBox           )
EVT_MENU( MENU_VOI_NEW_SELECTION_OBJECT_ELLIPSOID,          MainFrame::OnNewSelectionEllipsoid     )
EVT_MENU( MENU_VOI_NEW_FROM_OVERLAY,                        MainFrame::OnNewFromOverlay            )
EVT_MENU( MENU_VOI_RENDER_SELECTION_OBJECTS,                MainFrame::OnHideSelectionObjects      )
EVT_MENU( MENU_VOI_TOGGLE_SELECTION_OBJECTS,                MainFrame::OnToggleSelectionObjects    )
EVT_MENU( MENU_VOI_TOGGLE_VISIBLE_SELECTION_OBJECT,         MainFrame::OnToggleShowSelectionObject )
EVT_MENU( MENU_VOI_RENAME_BOX,                              MainFrame::OnRenameBox                 )
EVT_MENU( MENU_VOI_TOGGLE_ANDNOT,                           MainFrame::OnToggleAndNot              )
EVT_MENU( MENU_VOI_COLOR_ROI,                               MainFrame::OnColorRoi                  )
EVT_MENU( MENU_VOI_USE_MORPH,                               MainFrame::OnUseMorph                  )
EVT_MENU( MENU_VOI_DISPLAY_FIBERS_INFO,                     MainFrame::OnDisplayFibersInfo         )
EVT_MENU( MENU_VOI_CREATE_TEXTURE,                          MainFrame::OnCreateColorTexture        )
// Menu Surfaces
EVT_MENU( MENU_SURFACE_NEW_OFFSET,                          MainFrame::OnNewOffsetMap      )
EVT_MENU( MENU_SPLINESURF_NEW,                              MainFrame::OnNewSurface        )
EVT_MENU( MENU_SPLINESURF_TOGGLE_LIC,                       MainFrame::OnToggleLIC         )
EVT_MENU( MENU_SPLINESURF_TOGGLE_NORMAL,                    MainFrame::OnToggleNormal      )
EVT_MENU( MENU_SPLINESURF_DRAW_POINTS,                      MainFrame::OnTogglePointMode   )
EVT_MENU( MENU_SPLINESURF_DRAW_VECTORS,                     MainFrame::OnToggleDrawVectors )
// Menu Options
EVT_MENU( MENU_OPTIONS_ASSIGN_COLOR,                        MainFrame::OnAssignColor            )
EVT_MENU( MENU_OPTIONS_RESET_COLOR,                         MainFrame::OnResetColor             )
EVT_MENU( MENU_OPTIONS_TOGGLE_LIGHTING,                     MainFrame::OnToggleLighting         )
EVT_MENU( MENU_OPTIONS_INVERT_FIBERS,                       MainFrame::OnInvertFibers           )
EVT_MENU( MENU_OPTIONS_USE_FAKE_TUBES,                      MainFrame::OnUseFakeTubes           )
EVT_MENU( MENU_OPTIONS_CLEAR_TO_BLACK,                      MainFrame::OnClearToBlack           )
EVT_MENU( MENU_OPTIONS_USE_TRANSPARENCY,                    MainFrame::OnUseTransparency        )
EVT_MENU( MENU_OPTIONS_TOGGLE_TEXTURE_FILTERING,            MainFrame::OnToggleTextureFiltering )
EVT_MENU( MENU_OPTIONS_BLEND_TEX_ON_MESH,                   MainFrame::OnToggleBlendTexOnMesh   )
EVT_MENU( MENU_OPTIONS_FILTER_ISO,                          MainFrame::OnToggleFilterIso        )  
EVT_MENU( MENU_OPTIONS_CLEAN,                               MainFrame::OnClean                  )
EVT_MENU( MENU_OPTIONS_LOOP,                                MainFrame::OnLoop                   )
EVT_MENU( MENU_OPTIONS_CMAP0,                               MainFrame::OnSetCMap0               )
EVT_MENU( MENU_OPTIONS_CMAP1,                               MainFrame::OnSetCMap1               )
EVT_MENU( MENU_OPTIONS_CMAP2,                               MainFrame::OnSetCMap2               )
EVT_MENU( MENU_OPTIONS_CMAP3,                               MainFrame::OnSetCMap3               )
EVT_MENU( MENU_OPTIONS_CMAP4,                               MainFrame::OnSetCMap4               )
EVT_MENU( MENU_OPTIONS_CMAP5,                               MainFrame::OnSetCMap5               )
EVT_MENU( MENU_OPTIONS_CMAPNO,                              MainFrame::OnSetCMapNo              )
EVT_MENU( MENU_OPTIONS_CMAP_LEGEND,                         MainFrame::OnToggleColorMapLegend   )
// Menu Help
EVT_MENU( MENU_HELP_ABOUT,                                  MainFrame::OnAbout         )
EVT_MENU( MENU_HELP_SHORTCUTS,                              MainFrame::OnShortcuts     )
EVT_MENU( MENU_HELP_SCREENSHOT,                             MainFrame::OnScreenshot    )
EVT_MENU( MENU_HELP_SLIZEMOVIESAG,                          MainFrame::OnSlizeMovieSag )
EVT_MENU( MENU_HELP_SLIZEMOVIECOR,                          MainFrame::OnSlizeMovieCor )
EVT_MENU( MENU_HELP_SLIZEMOVIEAXI,                          MainFrame::OnSlizeMovieAxi )
/*
 * List widget events
 */
EVT_LIST_ITEM_ACTIVATED  ( ID_LIST_CTRL,                    MainFrame::OnActivateListItem   )
EVT_LIST_ITEM_SELECTED   ( ID_LIST_CTRL,                    MainFrame::OnSelectListItem     )
EVT_LIST_ITEM_RIGHT_CLICK( ID_LIST_CTRL,                    MainFrame::OnRightClickListItem )

EVT_BUTTON( ID_BUTTON_UP,                                   MainFrame::OnListItemUp   )
EVT_BUTTON( ID_BUTTON_DOWN,                                 MainFrame::OnListItemDown )

EVT_MENU( MENU_LIST_DELETE,                                 MainFrame::OnListMenuDelete      )
EVT_MENU( MENU_LIST_TOGGLE_SHOW,                            MainFrame::OnListMenuShow        )
EVT_MENU( MENU_LIST_TOGGLE_COLOR,                           MainFrame::OnListMenuThreshold   )
EVT_MENU( MENU_LIST_TOGGLE_NAME,                            MainFrame::OnListMenuName        )
EVT_MENU( MENU_LIST_SHOW_OPTIONS,                           MainFrame::OnListMenuShowOptions )
EVT_MENU( MENU_LIST_HIDE_OPTIONS,                           MainFrame::OnListMenuHideOptions )
EVT_MENU( MENU_LIST_CUTOUT,                                 MainFrame::OnListMenuCutOut      )

/*
 * Tree widget events
 */
EVT_TREE_DELETE_ITEM     ( ID_TREE_CTRL,                    MainFrame::OnDeleteTreeItem     )
EVT_TREE_SEL_CHANGED     ( ID_TREE_CTRL,                    MainFrame::OnSelectTreeItem     )
EVT_TREE_ITEM_ACTIVATED  ( ID_TREE_CTRL,                    MainFrame::OnActivateTreeItem   )
EVT_TREE_ITEM_RIGHT_CLICK( ID_TREE_CTRL,                    MainFrame::OnRightClickTreeItem )
EVT_COMMAND              ( ID_TREE_CTRL, wxEVT_TREE_EVENT,  MainFrame::OnTreeEvent          )
EVT_TREE_END_LABEL_EDIT  ( ID_TREE_CTRL,                    MainFrame::OnTreeLabelEdit      )

EVT_BUTTON( BUTTON_LOAD_DATASETS,                           MainFrame::OnLoadDatasets          )
EVT_BUTTON( BUTTON_LOAD_MESHES,                             MainFrame::OnLoadMeshes            )
EVT_BUTTON( BUTTON_LOAD_FIBERS,                             MainFrame::OnLoadFibers            )
EVT_BUTTON( BUTTON_LOAD_TENSORS,                            MainFrame::OnLoadTensors           )
EVT_BUTTON( BUTTON_LOAD_ODFS,                               MainFrame::OnLoadODFs              )
EVT_BUTTON( BUTTON_LOAD_TEST_FIBERS,                        MainFrame::OnLoadTestFibers        )
EVT_BUTTON( BUTTON_DISPLAY_MEAN_FIBER,                      MainFrame::OnDisplayMeanFiber      )
EVT_BUTTON( BUTTON_DISPLAY_CROSS_SECTIONS,                  MainFrame::OnDisplayCrossSections  )
EVT_BUTTON( BUTTON_DISPLAY_DISPERSION_TUBE,                 MainFrame::OnDisplayDispersionTube )
EVT_BUTTON( BUTTON_COLOR_WITH_CURVATURE,                    MainFrame::OnColorWithCurvature    )
EVT_BUTTON( BUTTON_COLOR_WITH_TORSION,                      MainFrame::OnColorWithTorsion      )

/*
 * Interface events
 */
EVT_SIZE(                                                   MainFrame::OnSize       )
EVT_MOUSE_EVENTS(                                           MainFrame::OnMouseEvent )

/* mouse click in one of the three navigation windows */
EVT_COMMAND( ID_GL_NAV_X, wxEVT_NAVGL_EVENT,                MainFrame::OnGLEvent )
EVT_COMMAND( ID_GL_NAV_Y, wxEVT_NAVGL_EVENT,                MainFrame::OnGLEvent )
EVT_COMMAND( ID_GL_NAV_Z, wxEVT_NAVGL_EVENT,                MainFrame::OnGLEvent )
EVT_COMMAND( ID_GL_MAIN,  wxEVT_NAVGL_EVENT,                MainFrame::OnGLEvent )

/* slider events */
EVT_SLIDER( ID_X_SLIDER,                                    MainFrame::OnSliderMoved   )
EVT_SLIDER( ID_Y_SLIDER,                                    MainFrame::OnSliderMoved   )
EVT_SLIDER( ID_Z_SLIDER,                                    MainFrame::OnSliderMoved   )
EVT_SLIDER( ID_T_SLIDER,                                    MainFrame::OnTSliderMoved  )
EVT_SLIDER( ID_T_SLIDER2,                                   MainFrame::OnTSlider2Moved )

// Slider events for the tensors options.
EVT_SLIDER  ( ID_GLYPH_OPTIONS_MIN_HUE_SLIDER,              MainFrame::OnGlyphMinHueSliderMoved           )
EVT_SLIDER  ( ID_GLYPH_OPTIONS_MAX_HUE_SLIDER,              MainFrame::OnGlyphMaxHueSliderMoved           )
EVT_SLIDER  ( ID_GLYPH_OPTIONS_SATURATION_SLIDER,           MainFrame::OnGlyphSaturationSliderMoved       )
EVT_SLIDER  ( ID_GLYPH_OPTIONS_LUMINANCE_SLIDER,            MainFrame::OnGlyphLuminanceSliderMoved        )
EVT_SLIDER  ( ID_GLYPH_OPTIONS_LOD_SLIDER,                  MainFrame::OnGlyphLODSliderMoved              )
EVT_SLIDER  ( ID_GLYPH_OPTIONS_LIGHT_ATTENUATION_SLIDER,    MainFrame::OnGlyphLightAttenuationSliderMoved )
EVT_SLIDER  ( ID_GLYPH_OPTIONS_LIGHT_X_POSITION_SLIDER,     MainFrame::OnGlyphLightXDirectionSliderMoved  )
EVT_SLIDER  ( ID_GLYPH_OPTIONS_LIGHT_Y_POSITION_SLIDER,     MainFrame::OnGlyphLightYDirectionSliderMoved  )
EVT_SLIDER  ( ID_GLYPH_OPTIONS_LIGHT_Z_POSITION_SLIDER,     MainFrame::OnGlyphLightZDirectionSliderMoved  )
EVT_SLIDER  ( ID_GLYPH_OPTIONS_DISPLAY_SLIDER,              MainFrame::OnGlyphDisplaySliderMoved          )
EVT_SLIDER  ( ID_GLYPH_OPTIONS_SCALING_FACTOR_SLIDER,       MainFrame::OnGlyphScalingFactorSliderMoved    )
EVT_CHECKBOX( ID_GLYPH_OPTIONS_X_AXIS_FLIP,                 MainFrame::OnGlyphXAxisFlipChecked            )
EVT_CHECKBOX( ID_GLYPH_OPTIONS_Y_AXIS_FLIP,                 MainFrame::OnGlyphYAxisFlipChecked            )
EVT_CHECKBOX( ID_GLYPH_OPTIONS_Z_AXIS_FLIP,                 MainFrame::OnGlyphZAxisFlipChecked            )
EVT_CHECKBOX( ID_GLYPH_OPTIONS_COLOR_WITH_POSITION,         MainFrame::OnGlyphColorWithPosition           )

EVT_RADIOBUTTON( ID_GLYPH_OPTIONS_RADIO_NORMAL,             MainFrame::OnGlyphNormalSelected         )
EVT_RADIOBUTTON( ID_GLYPH_OPTIONS_RADIO_MAP_ON_SPHERE,      MainFrame::OnGlyphMapOnSphereSelected         )
EVT_RADIOBUTTON( ID_GLYPH_OPTIONS_RADIO_AXES,               MainFrame::OnGlyphAxesSelected         )
EVT_RADIOBUTTON( ID_GLYPH_OPTIONS_RADIO_MAIN_AXIS,          MainFrame::OnGlyphMainAxisSelected         )

/*
 * Buttons, not yet in menus
 */
EVT_MENU( BUTTON_AXIAL,                                     MainFrame::OnButtonAxial    )
EVT_MENU( BUTTON_CORONAL,                                   MainFrame::OnButtonCoronal  )
EVT_MENU( BUTTON_SAGITTAL,                                  MainFrame::OnButtonSagittal )
EVT_MENU( BUTTON_TOGGLE_ALPHA,                              MainFrame::OnToggleAlpha    )
EVT_MENU( BUTTON_MOVE_POINTS1,                              MainFrame::OnMovePoints1    )
EVT_MENU( BUTTON_MOVE_POINTS2,                              MainFrame::OnMovePoints2    )

/* KDTREE thread finished */
EVT_MENU( KDTREE_EVENT,                                     MainFrame::OnKdTreeThreadFinished )

EVT_TIMER( -1,                                              MainFrame::OnTimerEvent )

END_EVENT_TABLE()

///////////////////////////////////////////////////////////////////////////
// Constructor
//
// i_parent     :
// i_id         :
// i_title      :
// i_pos        :
// i_size       :
// i_style      :
///////////////////////////////////////////////////////////////////////////
MainFrame::MainFrame(      wxWindow*   i_parent, 
                     const wxWindowID i_id, 
                     const wxString&  i_title, 
                     const wxPoint&   i_pos, 
                     const wxSize&    i_size, 
                     const long       i_style ) :
    wxFrame( i_parent, i_id, i_title, i_pos, i_size, i_style )
{
    m_enlargeNav = 0;

    m_xSlider  = new wxSlider( this, ID_X_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( 150, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_ySlider  = new wxSlider( this, ID_Y_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( 150, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_zSlider  = new wxSlider( this, ID_Z_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( 150, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_tSlider  = new MySlider( this, ID_T_SLIDER,  30, 0, 100, wxDefaultPosition, wxSize( 110, 19 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_tSlider2 = new wxSlider( this, ID_T_SLIDER2, 30, 0, 100, wxDefaultPosition, wxSize( 103, 19 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );

    m_buttonUp   = new wxButton( this, ID_BUTTON_UP,   wxT( "up" ),   wxDefaultPosition, wxSize( 40, 19 ) );
    m_buttonDown = new wxButton( this, ID_BUTTON_DOWN, wxT( "down" ), wxDefaultPosition, wxSize( 40, 19 ) );

    m_buttonUp->SetFont  ( wxFont( 6, wxDEFAULT, wxNORMAL, wxNORMAL ) );
    m_buttonDown->SetFont( wxFont( 6, wxDEFAULT, wxNORMAL, wxNORMAL ) );

    m_buttonLoadDatasets   = new wxButton( this, BUTTON_LOAD_DATASETS,        wxT( "Load Datasets" ),    wxDefaultPosition, wxSize( 148, 20 ) );
    m_buttonLoadMeshes     = new wxButton( this, BUTTON_LOAD_MESHES,          wxT( "Load Meshes" ),      wxDefaultPosition, wxSize( 148, 20 ) );
    m_buttonLoadFibers     = new wxButton( this, BUTTON_LOAD_FIBERS,          wxT( "Load Fibers" ),      wxDefaultPosition, wxSize( 148, 20 ) );
    m_buttonLoadTensors    = new wxButton( this, BUTTON_LOAD_TENSORS,         wxT( "Load Tensors" ),     wxDefaultPosition, wxSize( 148, 20 ) );
    m_buttonLoadODFs       = new wxButton( this, BUTTON_LOAD_ODFS,            wxT( "Load ODFs" ),        wxDefaultPosition, wxSize( 148, 20 ) );
    m_buttonLoadTestFibers = new wxButton( this, BUTTON_LOAD_TEST_FIBERS,     wxT( "Load Test Fibers" ), wxDefaultPosition, wxSize( 148, 20 ) );
    // This button is only to perform some test, so we hide it if we do not need it.
    m_buttonLoadTestFibers->Show( false );

    m_buttonLoadODFs->SetFont      ( wxFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL ) );
    m_buttonLoadTensors->SetFont   ( wxFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL ) );
    m_buttonLoadFibers->SetFont    ( wxFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL ) );
    m_buttonLoadMeshes->SetFont    ( wxFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL ) );
    m_buttonLoadDatasets->SetFont  ( wxFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL ) );
    m_buttonLoadTestFibers->SetFont( wxFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL ) );

    m_listCtrl   = new MyListCtrl( this, ID_LIST_CTRL, wxDefaultPosition, wxSize( 293, -1 ), wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_NO_HEADER );

    m_treeWidget = new MyTreeCtrl( this, ID_TREE_CTRL, wxDefaultPosition,wxSize( 150, -1 ), wxTR_HAS_BUTTONS | wxTR_SINGLE | wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS );
    m_treeWidget->SetMaxSize( wxSize( 150, -1 ) );

    wxImageList* imageList = new wxImageList( 16, 16 );

	// The m_fibersInfoSizer and all its component will be created if and when
    // the user click on the display fibers info button.
    m_fibersInfoSizer                 = NULL;
	m_fibersInfoGrid                  = NULL;
    m_buttonDisplayMeanFiber          = NULL;
    m_buttonDisplayCrossSections      = NULL;
    m_buttonDisplayDispersionTube     = NULL;
    m_buttonColorWithCurvature        = NULL;
    m_buttonColorWithTorsion          = NULL;

    // The m_glyphOptionsSizer and all its component will be created if and when the user
    // click on the show option button after a right click in the items list on a glyph item.
    m_glyphOptionsSizer               = NULL;
    m_glyphMinHueValue                = NULL;
    m_glyphMaxHueValue                = NULL;
    m_glyphSaturationValue            = NULL;
    m_glyphLuminanceValue             = NULL;
    m_glyphLODValue                   = NULL;
    m_glyphLightAttenuation           = NULL;
    m_glyphLightXPosition             = NULL;
    m_glyphLightYPosition             = NULL;
    m_glyphLightZPosition             = NULL;
    m_glyphDisplayValue               = NULL;
    m_glyphScalingFactor              = NULL;
    m_xAxisFlip                       = NULL;
    m_yAxisFlip                       = NULL;
    m_zAxisFlip                       = NULL;    
    m_colorWithPosition               = NULL;
    m_minHueSliderText                = NULL;
    m_maxHueSliderText                = NULL;
    m_saturationSliderText            = NULL;
    m_luminanceSliderText             = NULL;
    m_LODSliderText                   = NULL;
    m_lightAttenuationSliderText      = NULL;
    m_lightXPositionSliderText        = NULL;
    m_lightYPositionSliderText        = NULL;
    m_lightZPositionSliderText        = NULL;
    m_displayRatioSliderText          = NULL;
    m_scalingFactorText               = NULL;
    m_radioAxes                       = NULL;
    m_radioNormal                  = NULL;
    m_radioMainAxis                   = NULL;
    m_radioMapOnSphere                = NULL;

#ifndef __WXMAC__
    imageList->Add( wxIcon( eyes_xpm   ) );
    imageList->Add( wxIcon( delete_xpm ) );
#else
    imageList->Add( ( wxImage( MyApp::respath +_T( "icons/eyes.png"   ), wxBITMAP_TYPE_PNG ) ) );
    imageList->Add( ( wxImage( MyApp::respath +_T( "icons/delete.png" ), wxBITMAP_TYPE_PNG ) ) );
#endif
    m_listCtrl->AssignImageList(imageList, wxIMAGE_LIST_SMALL);

    wxListItem itemCol;
    itemCol.SetText( wxT( "" ) );
    m_listCtrl->InsertColumn( 0, itemCol );

    itemCol.SetText( wxT( "Name" ) );
    itemCol.SetAlign( wxLIST_FORMAT_CENTRE );
    m_listCtrl->InsertColumn( 1, itemCol );

    itemCol.SetText( wxT( "Threshold" ) );
    itemCol.SetAlign( wxLIST_FORMAT_RIGHT );
    m_listCtrl->InsertColumn( 2, itemCol) ;

    itemCol.SetText( wxT( "" ) );
    m_listCtrl->InsertColumn( 3, itemCol );

    m_listCtrl->SetColumnWidth( 0, 20  );
    m_listCtrl->SetColumnWidth( 1, 194 );
    m_listCtrl->SetColumnWidth( 2, 70  );
    m_listCtrl->SetColumnWidth( 3, 20  );

    wxImageList* tImageList = new wxImageList( 16, 16 );
#ifndef __WXMAC__
    tImageList->Add( wxIcon( delete_xpm ) );
    tImageList->Add( wxIcon( eyes_xpm   ) );
#else
    tImageList->Add( wxImage( MyApp::respath + _T( "icons/delete.png" ), wxBITMAP_TYPE_PNG ) );
    tImageList->Add( wxImage( MyApp::respath + _T( "icons/eyes.png" ),   wxBITMAP_TYPE_PNG ) );
#endif
    m_treeWidget->AssignImageList( tImageList );

    m_tRootId  = m_treeWidget->AddRoot( wxT( "Scene" ), -1, -1, NULL );
    m_tPointId = m_treeWidget->AppendItem( m_tRootId, wxT( "points" ), -1, -1, NULL );
    m_tSelectionObjectsId = m_treeWidget->AppendItem( m_tRootId, wxT( "selection objects" ), -1, -1, NULL );

    m_datasetHelper = new DatasetHelper( this );

    /*
     * Set OpenGL attributes
     */
    m_datasetHelper->printDebug( _T( "Initializing OpenGL" ), 1 );
    GLboolean doubleBuffer = GL_TRUE;
#ifdef __WXMSW__
    int *gl_attrib = NULL;
#else
    int gl_attrib[20] =
    { WX_GL_RGBA, WX_GL_MIN_RED, 1, WX_GL_MIN_GREEN, 1, WX_GL_MIN_BLUE, 1,
            WX_GL_DEPTH_SIZE, 1, WX_GL_DOUBLEBUFFER,
#if defined(__WXMAC__) || defined(__WXCOCOA__)
            GL_NONE};
#else
            None };
#endif
#endif
    if( ! doubleBuffer )
    {
        m_datasetHelper->printDebug( _T( "don't have double buffer, disabling" ), 1 );
#ifdef __WXGTK__
        gl_attrib[9] = None;
#endif
        doubleBuffer = GL_FALSE;
    }

    m_datasetHelper->m_theScene = new TheScene( m_datasetHelper );

    m_mainGL = new MainCanvas( m_datasetHelper, MAIN_VIEW, this, ID_GL_MAIN,  wxDefaultPosition, wxDefaultSize,      0, _T( "MainGLCanvas" ), gl_attrib );

#ifndef CTX
    m_gl0 = new MainCanvas( m_datasetHelper,       AXIAL, this, ID_GL_NAV_X, wxDefaultPosition, wxSize( 150, 150 ), 0, _T( "NavGLCanvasX" ), gl_attrib, m_mainGL );
    m_gl0->SetMaxSize( wxSize( 150, 150 ) );
    m_gl1 = new MainCanvas( m_datasetHelper,     CORONAL, this, ID_GL_NAV_Y, wxDefaultPosition, wxSize( 150, 150 ), 0, _T( "NavGLCanvasY" ), gl_attrib, m_mainGL );
    m_gl1->SetMaxSize( wxSize( 150, 150 ) );
    m_gl2 = new MainCanvas( m_datasetHelper,    SAGITTAL, this, ID_GL_NAV_Z, wxDefaultPosition, wxSize( 150, 150 ), 0, _T( "NavGLCanvasZ" ), gl_attrib, m_mainGL );
    m_gl2->SetMaxSize( wxSize( 150, 150 ) );

#else
    m_gl0 = new MainCanvas( m_datasetHelper, axial,    m_topNavWindow,    ID_GL_NAV_X, wxDefaultPosition, wxDefaultSize, 0, _T( "NavGLCanvasX" ), gl_attrib, m_mainGL->GetContext() );
    m_gl1 = new MainCanvas( m_datasetHelper, coronal,  m_middleNavWindow, ID_GL_NAV_Y, wxDefaultPosition, wxDefaultSize, 0, _T( "NavGLCanvasY" ), gl_attrib, m_mainGL->GetContext() );
    m_gl2 = new MainCanvas( m_datasetHelper, sagittal, m_bottomNavWindow, ID_GL_NAV_Z, wxDefaultPosition, wxDefaultSize, 0, _T( "NavGLCanvasZ" ), gl_attrib, m_mainGL->GetContext() );
#endif

#ifndef __WXMAC__
    m_datasetHelper->m_theScene->setMainGLContext( new wxGLContext( m_mainGL ) ); // I don't understand this (mario)
#else
    m_datasetHelper->m_theScene->setMainGLContext( m_mainGL->GetContext() );
#endif
    m_mainSizer		 = new wxBoxSizer( wxHORIZONTAL ); // Contains everything in the UI.
    m_leftMainSizer	 = new wxBoxSizer( wxVERTICAL );   // Contains everything left in the UI (leftSizer, listCtrl and buttonSizer).
    m_leftSizer		 = new wxBoxSizer( wxHORIZONTAL ); // Contains the treeSizer and the navSizer.
    m_navSizer		 = new wxBoxSizer( wxVERTICAL );   // Contains the 3 navigation windows with there respectiv sliders.
    m_buttonSizer	 = new wxBoxSizer( wxHORIZONTAL ); // Contains the up, down button and the 2 sliders at the bottom left corner of the UI.
    m_treeSizer		 = new wxBoxSizer( wxVERTICAL );   // Contains the tree and the loadDataSets, loadMeshes and the loadFibers buttons.
	m_rightMainSizer = new wxBoxSizer( wxHORIZONTAL ); // Contains the OpenGl window and the rightSizer.

    m_navSizer->Add( m_gl0,     1, wxALL | wxEXPAND | wxSHAPED, 1 );
    m_navSizer->Add( m_zSlider, 0, wxALL,                       1 );
    m_navSizer->Add( m_gl1,     1, wxALL | wxEXPAND | wxSHAPED, 1 );
    m_navSizer->Add( m_ySlider, 0, wxALL,                       1 );
    m_navSizer->Add( m_gl2,     1, wxALL | wxEXPAND | wxSHAPED, 1 );
    m_navSizer->Add( m_xSlider, 0, wxALL,                       1 );

    m_treeSizer->Add( m_buttonLoadDatasets,   0, wxALL | wxALIGN_CENTER, 0 );
    m_treeSizer->Add( m_buttonLoadMeshes,     0, wxALL | wxALIGN_CENTER, 0 );
    m_treeSizer->Add( m_buttonLoadFibers,     0, wxALL | wxALIGN_CENTER, 0 );
    m_treeSizer->Add( m_buttonLoadTensors,    0, wxALL | wxALIGN_CENTER, 0 );
    m_treeSizer->Add( m_buttonLoadODFs,       0, wxALL | wxALIGN_CENTER, 0 );
    m_treeSizer->Add( m_buttonLoadTestFibers, 0, wxALL | wxALIGN_CENTER, 0 );
    m_treeSizer->Add( m_treeWidget,           1, wxALL,                  1 );

    m_leftSizer->Add( m_treeSizer, 0, wxALL | wxEXPAND, 0 );
    m_leftSizer->Add( m_navSizer,  1, wxALL | wxEXPAND, 0 );

    m_buttonSizer->Add( m_buttonUp,   0, wxALL, 1 );
    m_buttonSizer->Add( m_buttonDown, 0, wxALL, 1 );
    m_buttonSizer->Add( m_tSlider,  0, wxALL, 1 );
    m_buttonSizer->Add( m_tSlider2, 0, wxALL, 1 );

    m_leftMainSizer->Add( m_leftSizer,   0, wxALL,                  1 );
    m_leftMainSizer->Add( m_listCtrl,    1, wxALL | wxEXPAND,       1 );
    m_leftMainSizer->Add( m_buttonSizer, 0, wxALIGN_BOTTOM | wxALL, 1 );

	m_rightMainSizer->Add( m_mainGL, 1, wxEXPAND | wxALL, 2 );
	
    m_mainSizer->Add( m_leftMainSizer,  0, wxEXPAND | wxALL, 0 );
	m_mainSizer->Add( m_rightMainSizer, 1, wxEXPAND | wxALL, 0 );

    SetSizer( m_mainSizer );
    m_mainSizer->SetSizeHints( this );

    m_timer = new wxTimer( this );
    m_timer->Start( 40 );
}

///////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////
MainFrame::~MainFrame()
{
    m_datasetHelper->printDebug( _T( "main frame destructor" ), 0 );

    if( m_datasetHelper )               
        delete m_datasetHelper;
}

/****************************************************************************************************
 *
 * Menu event functions
 *
 ****************************************************************************************************/
/****************************************************************************************************
 *
 * Menu File
 *
 ****************************************************************************************************/

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnNewIsoSurface( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->createIsoSurface();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnNewOffsetMap( wxCommandEvent& WXUNUSED(event ))
{
    m_datasetHelper->createDistanceMap();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnQuit( wxCommandEvent& WXUNUSED(event) )
{
    wxMessageDialog dialog( NULL, wxT( "Really Quit?" ), wxT( "Really Quit?" ), wxNO_DEFAULT | wxYES_NO | wxICON_INFORMATION );
    if( dialog.ShowModal() == wxID_YES )
        Close(true);
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnLoad( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->load( 0 ) )
    {
        wxMessageBox( wxT( "ERROR\n" ) + m_datasetHelper->m_lastError, wxT( "" ), wxOK | wxICON_INFORMATION, NULL );
        GetStatusBar()->SetStatusText( wxT( "ERROR" ), 1 );
        GetStatusBar()->SetStatusText( m_datasetHelper->m_lastError, 2 );
        return;
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnReloadShaders( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_scheduledReloadShaders = true;
    renewAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSave( wxCommandEvent& WXUNUSED(event) )
{
    wxString caption         = wxT( "Choose a file" );
    wxString wildcard        = wxT( "Scene files (*.scn)|*.scn|*.*|*.*" );
    wxString defaultDir      = wxEmptyString;
    wxString defaultFilename = wxEmptyString;
    wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
    dialog.SetFilterIndex( 0 );
    dialog.SetDirectory( m_datasetHelper->m_lastPath );

    if( m_datasetHelper->m_scnFileLoaded )
        dialog.SetFilename( m_datasetHelper->m_scnFileName );

    dialog.SetDirectory( m_datasetHelper->m_scenePath );

    if( dialog.ShowModal() == wxID_OK )
    {
        m_datasetHelper->m_scenePath = dialog.GetDirectory();
        m_datasetHelper->save(dialog.GetPath());
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSaveFibers( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_fibersLoaded )
        return;

    Fibers* l_fibers = NULL;
    m_datasetHelper->getFiberDataset( l_fibers );

    wxString caption         = wxT( "Choose a file" );
    wxString wildcard        = wxT( "fiber files (*.fib)|*.fib|*.*|*.*" );
    wxString defaultDir      = wxEmptyString;
    wxString defaultFilename = wxEmptyString;
    wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
    dialog.SetFilterIndex( 0 );
    dialog.SetDirectory( m_datasetHelper->m_lastPath );

    if( dialog.ShowModal() == wxID_OK )
    {
        m_datasetHelper->m_lastPath = dialog.GetDirectory();
        l_fibers->save( dialog.GetPath() );
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSaveDataset( wxCommandEvent& WXUNUSED(event) )
{
    Anatomy* l_anatomy = NULL;

    long item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item != -1 )
    {
        DatasetInfo* info = (DatasetInfo*)m_listCtrl->GetItemData( item );
        if( info->getType() < MESH )
        {
            l_anatomy = (Anatomy*)m_listCtrl->GetItemData( item );

            wxString caption         = wxT( "Choose l_anatomy file" );
            wxString wildcard        = wxT( "nifti files (*.nii)|*.nii||*.*|*.*" );
            wxString defaultDir      = wxEmptyString;
            wxString defaultFilename = wxEmptyString;
            wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
            dialog.SetFilterIndex( 0 );
            dialog.SetDirectory( m_datasetHelper->m_lastPath );

            if( dialog.ShowModal() == wxID_OK )
            {
                m_datasetHelper->m_lastPath = dialog.GetDirectory();
                l_anatomy->saveNifti( dialog.GetPath() );
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnMinimizeDataset( wxCommandEvent& WXUNUSED(event) )
{
    Anatomy* l_anatomy = NULL;

    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item != -1 )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
        if( l_info->getType() < MESH )
        {
            l_anatomy = (Anatomy*)m_listCtrl->GetItemData( l_item );
            l_anatomy->minimize();
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnDilateDataset( wxCommandEvent& WXUNUSED(event) )
{
    Anatomy* l_anatomy = NULL;

    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item != -1 )
    {
        DatasetInfo* l_info = (DatasetInfo*) m_listCtrl->GetItemData( l_item );
        if(l_info->getType() < MESH )
        {
            l_anatomy = (Anatomy*) m_listCtrl->GetItemData( l_item );
            l_anatomy->dilate();
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnErodeDataset(wxCommandEvent& WXUNUSED(event))
{
    Anatomy* l_anatomy = NULL;

    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item != -1 )
    {
        DatasetInfo* l_info = (DatasetInfo*) m_listCtrl->GetItemData( l_item );
        if( l_info->getType() < MESH )
        {
            l_anatomy = (Anatomy*)m_listCtrl->GetItemData( l_item );
            l_anatomy->erode();
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSaveSurface( wxCommandEvent& WXUNUSED(event) )
{
    long l_item = m_listCtrl->GetNextItem(- 1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item != -1 )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
        if( l_info->getType() == SURFACE )
        {
            Surface *l_surface = (Surface*)l_info;

            std::cout << "got l_surface: " << l_surface << std::endl;

            wxString caption         = wxT( "Choose a file" );
            wxString wildcard        = wxT( "surfae files (*.vtk)|*.vtk" );
            wxString defaultDir      = wxEmptyString;
            wxString defaultFilename = wxEmptyString;
            wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
            dialog.SetFilterIndex( 0 );
            dialog.SetDirectory( m_datasetHelper->m_lastPath );
            if( dialog.ShowModal() == wxID_OK )
            {
                std::cout << " start saving" << std::endl;
                m_datasetHelper->m_lastPath = dialog.GetDirectory();
                l_surface->save( dialog.GetPath() );
                std::cout << " done saving" << std::endl;
            }

        }
        if( l_info->getType() == ISO_SURFACE )
        {
            CIsoSurface *l_surface = (CIsoSurface*)l_info;

            std::cout << "got l_surface: " << l_surface << std::endl;

            wxString caption         = wxT( "Choose a file" );
            wxString wildcard        = wxT( "surfae files (*.vtk)|*.vtk" );
            wxString defaultDir      = wxEmptyString;
            wxString defaultFilename = wxEmptyString;
            wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
            dialog.SetFilterIndex( 0 );
            dialog.SetDirectory( m_datasetHelper->m_lastPath );
            if( dialog.ShowModal() == wxID_OK )
            {
                std::cout << " start saving" << std::endl;
                m_datasetHelper->m_lastPath = dialog.GetDirectory();
                l_surface->save( dialog.GetPath() );
                std::cout << " done saving" << std::endl;
            }
        }
    }
}

/****************************************************************************************************
 *
 * Menu View
 * Sets the main GL widget to some standard positions
 *
 ****************************************************************************************************/

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnMenuViewReset( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_zoom  = 1;
    m_datasetHelper->m_xMove = 0;
    m_datasetHelper->m_yMove = 0;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnMenuViewLeft( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_datasetHelper->m_transform );
    m_datasetHelper->m_transform.s.M00 =  0.0f;
    m_datasetHelper->m_transform.s.M11 =  0.0f;
    m_datasetHelper->m_transform.s.M22 =  0.0f;
    m_datasetHelper->m_transform.s.M20 = -1.0f;
    m_datasetHelper->m_transform.s.M01 = -1.0f;
    m_datasetHelper->m_transform.s.M12 =  1.0f;
    m_mainGL->setRotation();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnMenuViewRight( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_datasetHelper->m_transform );
    m_datasetHelper->m_transform.s.M00 = 0.0f;
    m_datasetHelper->m_transform.s.M11 = 0.0f;
    m_datasetHelper->m_transform.s.M22 = 0.0f;
    m_datasetHelper->m_transform.s.M20 = 1.0f;
    m_datasetHelper->m_transform.s.M01 = 1.0f;
    m_datasetHelper->m_transform.s.M12 = 1.0f;
    m_mainGL->setRotation();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnMenuViewTop( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_datasetHelper->m_transform );
    m_mainGL->setRotation();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnMenuViewBottom( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_datasetHelper->m_transform );
    m_datasetHelper->m_transform.s.M00 = -1.0f;
    m_datasetHelper->m_transform.s.M22 = -1.0f;
    m_mainGL->setRotation();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnMenuViewFront( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_datasetHelper->m_transform );
    m_datasetHelper->m_transform.s.M11 =  0.0f;
    m_datasetHelper->m_transform.s.M22 =  0.0f;
    m_datasetHelper->m_transform.s.M12 =  1.0f;
    m_datasetHelper->m_transform.s.M21 = -1.0f;
    m_mainGL->setRotation();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnMenuViewBack( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_datasetHelper->m_transform );
    m_datasetHelper->m_transform.s.M00 = -1.0f;
    m_datasetHelper->m_transform.s.M11 =  0.0f;
    m_datasetHelper->m_transform.s.M22 =  0.0f;
    m_datasetHelper->m_transform.s.M21 =  1.0f;
    m_datasetHelper->m_transform.s.M12 =  1.0f;
    m_mainGL->setRotation();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnMenuViewCrosshair( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_showCrosshair = ! m_datasetHelper->m_showCrosshair;
    refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 * Menu VOI
 *
 ****************************************************************************************************/

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnToggleSelectionObjects( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene || !m_datasetHelper->m_fibersLoaded )
        return;

    // Get what selection object is selected.
    wxTreeItemId l_selectionObjectTreeId = m_treeWidget->GetSelection();

    if( treeSelected( l_selectionObjectTreeId ) == MASTER_OBJECT )
    {
        SelectionObject* l_selectionObject = (SelectionObject*)( m_treeWidget->GetItemData( l_selectionObjectTreeId ) );
        l_selectionObject->toggleIsActive();
        m_treeWidget->SetItemImage( l_selectionObjectTreeId, l_selectionObject->getIcon());
        l_selectionObject->setIsDirty( true );

        int l_childSelectionObject = m_treeWidget->GetChildrenCount( l_selectionObjectTreeId );
        wxTreeItemIdValue childcookie = 0;
        for( int i = 0; i < l_childSelectionObject; ++i )
        {
            wxTreeItemId l_childId = m_treeWidget->GetNextChild( l_selectionObjectTreeId, childcookie );
            if( l_childId.IsOk() )
            {
                SelectionObject* l_childSelectionObject = ( (SelectionObject*)( m_treeWidget->GetItemData( l_childId ) ) );
                l_childSelectionObject->setIsActive( l_selectionObject->getIsActive() );
                m_treeWidget->SetItemImage( l_childId, l_childSelectionObject->getIcon() );
                l_childSelectionObject->setIsDirty( true );
            }
        }
    }
    else if( treeSelected( l_selectionObjectTreeId ) == CHILD_OBJECT )
    {
        SelectionObject* l_selectionObject = (SelectionObject*)( m_treeWidget->GetItemData( l_selectionObjectTreeId ) );
        l_selectionObject->toggleIsActive();
        m_treeWidget->SetItemImage( l_selectionObjectTreeId, l_selectionObject->getIcon() );
        l_selectionObject->setIsDirty( true );
    }

    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnToggleAndNot( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene || ! m_datasetHelper->m_fibersLoaded )
        return;

    // Get what selection object is selected.
    wxTreeItemId l_selectionObjectTreeId = m_treeWidget->GetSelection();

    if( treeSelected(l_selectionObjectTreeId) == CHILD_OBJECT )
    {
        SelectionObject* l_box = (SelectionObject*)( m_treeWidget->GetItemData( l_selectionObjectTreeId ) );
        l_box->toggleIsNOT();

        wxTreeItemId l_parentId = m_treeWidget->GetItemParent( l_selectionObjectTreeId );
        ((SelectionObject*)( m_treeWidget->GetItemData( l_parentId ) ) )->setIsDirty( true );

        if( ( (SelectionObject*)( m_treeWidget->GetItemData( l_selectionObjectTreeId ) ) )->getIsNOT() )
            m_treeWidget->SetItemBackgroundColour( l_selectionObjectTreeId, *wxRED   );
        else
            m_treeWidget->SetItemBackgroundColour( l_selectionObjectTreeId, *wxGREEN );

        m_treeWidget->SetItemImage( l_selectionObjectTreeId, l_box->getIcon() );
        l_box->setIsDirty( true );
    }

    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnToggleShowSelectionObject( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene || ! m_datasetHelper->m_fibersLoaded )
        return;

    // Get the selected selection object.
    wxTreeItemId l_selectionObjectTreeId = m_treeWidget->GetSelection();

    if( treeSelected( l_selectionObjectTreeId ) == MASTER_OBJECT )
    {
        SelectionObject* l_selecitonObject = (SelectionObject*)( m_treeWidget->GetItemData( l_selectionObjectTreeId ) );
        l_selecitonObject->toggleIsVisible();
        m_treeWidget->SetItemImage( l_selectionObjectTreeId, l_selecitonObject->getIcon() );
        l_selecitonObject->setIsDirty( true );

        int l_childSelectionObjects = m_treeWidget->GetChildrenCount( l_selectionObjectTreeId );
        wxTreeItemIdValue childcookie = 0;
        for( int i = 0; i < l_childSelectionObjects; ++i )
        {
            wxTreeItemId l_childId = m_treeWidget->GetNextChild( l_selectionObjectTreeId, childcookie );
            if( l_childId.IsOk() )
            {
                SelectionObject* childBox = ( (SelectionObject*)( m_treeWidget->GetItemData( l_childId ) ) );
                childBox->setIsVisible( l_selecitonObject->getIsVisible() );
                m_treeWidget->SetItemImage( l_childId, childBox->getIcon() );
                childBox->setIsDirty( true );
            }
        }
    }
    else if( treeSelected( l_selectionObjectTreeId ) == CHILD_OBJECT )
    {
        SelectionObject *l_selectionObject = (SelectionObject*)( m_treeWidget->GetItemData( l_selectionObjectTreeId ) );
        l_selectionObject->toggleIsVisible();
        m_treeWidget->SetItemImage( l_selectionObjectTreeId, l_selectionObject->getIcon() );
        l_selectionObject->setIsDirty( true );
    }

    m_datasetHelper->m_selBoxChanged = true;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the rename option is clicked on the right
// click menu of a SelectionObject item in the tree.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnRenameBox( wxCommandEvent& WXUNUSED(event) )
{
    wxTreeItemId l_treeBoxId = m_treeWidget->GetSelection();
    if( treeSelected( l_treeBoxId ) == MASTER_OBJECT || treeSelected( l_treeBoxId ) == CHILD_OBJECT )
    {
        SelectionObject* l_box = (SelectionObject*)( m_treeWidget->GetItemData( l_treeBoxId ) );

        wxTextEntryDialog dialog(this, _T( "Please enter a new name" ) );
        dialog.SetValue( l_box->getName() );

        if( ( dialog.ShowModal() == wxID_OK ) && ( dialog.GetValue() != _T( "" ) ) )
            l_box->setName( dialog.GetValue() );

        m_treeWidget->SetItemText( l_treeBoxId, l_box->getName() );

        SetFiberInfoGridName( l_box->getName() );
    }

    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display fibers 
// info after a right click in the tree on a selectio object.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnDisplayFibersInfo( wxCommandEvent& WXUNUSED(event) )
{
	m_datasetHelper->m_displayFibersInfo = ! m_datasetHelper->m_displayFibersInfo;
    
    DisplayFibersInfo( m_datasetHelper->m_displayFibersInfo );
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display mean fiber 
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnDisplayMeanFiber( wxCommandEvent& WXUNUSED(event) )
{
	m_datasetHelper->m_displayMeanFiber = ! m_datasetHelper->m_displayMeanFiber;
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display cross sections
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnDisplayCrossSections( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_displayCrossSections = (CrossSectionsDisplay)( ( (int)m_datasetHelper->m_displayCrossSections ) + 1 );
    if( m_datasetHelper->m_displayCrossSections == CS_NB_OF_CHOICES )
        m_datasetHelper->m_displayCrossSections = CS_NOTHING;
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display dispersion tube
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnDisplayDispersionTube( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_displayDispersionCone = (DispersionConeDisplay)( ( (int)m_datasetHelper->m_displayDispersionCone ) + 1 );
    if( m_datasetHelper->m_displayDispersionCone == DC_NB_OF_CHOICES )
        m_datasetHelper->m_displayDispersionCone = DC_NOTHING;
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the color with curvature button
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnColorWithCurvature( wxCommandEvent& WXUNUSED(event) )
{
	if( m_datasetHelper->m_fiberColorationMode !=  CURVATURE_COLOR )
        m_datasetHelper->m_fiberColorationMode = CURVATURE_COLOR;
    else
        m_datasetHelper->m_fiberColorationMode = NORMAL_COLOR;

    ColorFibers();
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display min/max cross section
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnColorWithTorsion( wxCommandEvent& WXUNUSED(event) )
{
	if( m_datasetHelper->m_fiberColorationMode !=  TORSION_COLOR )
        m_datasetHelper->m_fiberColorationMode = TORSION_COLOR;
    else
        m_datasetHelper->m_fiberColorationMode = NORMAL_COLOR;

    ColorFibers();
}

///////////////////////////////////////////////////////////////////////////
// This function will call the updateFibersColors function on the currently loaded fiber set.
///////////////////////////////////////////////////////////////////////////
void MainFrame::ColorFibers()
{   
    for( int i = 0; i < m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* info = (DatasetInfo*)m_listCtrl->GetItemData( i );

        if( info->getType() == FIBERS )
            ( (Fibers*)m_listCtrl->GetItemData( i ) )->updateFibersColors();
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will display or hide the fibers info grid depending on 
// the value of the argument.
//
// i_display        : If true, the grid will be shown, if false, the grid will be hiden.
///////////////////////////////////////////////////////////////////////////
void MainFrame::DisplayFibersInfo( bool i_display )
{
	m_datasetHelper->m_displayFibersInfo = i_display;
    
    // We need to show the grid.
	if( i_display )
	{
		if( m_fibersInfoSizer == NULL )
	    {
		    // Contains the grid and the buttons that are part of the fibers info 
            // (this sizer will take effet when the "display fibers info" options is enable).
		    m_fibersInfoSizer = new wxBoxSizer( wxVERTICAL );
            InitializeFibersInfoSizerElements();
            m_rightMainSizer->Add( m_fibersInfoSizer, 0, wxALL, 1 );
	    }

        SetFiberInfoGridValues( m_datasetHelper->m_lastSelectedObject );
        m_rightMainSizer->Show( m_fibersInfoSizer, true, true );
	    m_rightMainSizer->Layout();
	}
	else // We need to hide the grid.
	{
		// There is no point in hiding the grid if its not even created.
        if( m_fibersInfoSizer == NULL || m_fibersInfoGrid == NULL )
            return;

        m_rightMainSizer->Hide( m_fibersInfoSizer, true );
	    m_rightMainSizer->Layout();
	}
	
    // Adding or removing the m_fibersInfoGrid will cause the OpenGl window to resize, so we need to refresh it.
	if( m_mainGL )
        m_mainGL->changeOrthoSize();
}

///////////////////////////////////////////////////////////////////////////
// This function will initialize the elements that are part of fiber info sizer.
///////////////////////////////////////////////////////////////////////////
void MainFrame::InitializeFibersInfoSizerElements()
{ 
    // Initialize the grid.
    m_fibersInfoGrid = new wxGrid( this, ID_FIBERS_INFO_GRID );
	m_fibersInfoGrid->CreateGrid( FIBERS_INFO_GRID_ROW_SIZE, FIBERS_INFO_GRID_COL_SIZE, wxGrid::wxGridSelectCells );
    SetFiberInfoGridLabelTitlesValues();

    // Initialize the buttons.
    m_buttonDisplayMeanFiber          = new wxButton( this, BUTTON_DISPLAY_MEAN_FIBER,      wxT( "Display Mean Fiber"      ) );
    m_buttonDisplayCrossSections      = new wxButton( this, BUTTON_DISPLAY_CROSS_SECTIONS,  wxT( "Display Cross Section"   ) );
    m_buttonDisplayDispersionTube     = new wxButton( this, BUTTON_DISPLAY_DISPERSION_TUBE, wxT( "Display Dispersion Tube" ) );
    m_buttonColorWithCurvature        = new wxButton( this, BUTTON_COLOR_WITH_CURVATURE,    wxT( "Color With Curvature"    ) );
    m_buttonColorWithTorsion          = new wxButton( this, BUTTON_COLOR_WITH_TORSION,      wxT( "Color With Torsion"      ) );

    // Set the elements font.
    m_fibersInfoGrid->SetFont             ( wxFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL ) );
    m_buttonDisplayMeanFiber->SetFont     ( wxFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL ) );
    m_buttonDisplayCrossSections->SetFont ( wxFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL ) );
    m_buttonDisplayDispersionTube->SetFont( wxFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL ) );
    m_buttonColorWithCurvature->SetFont   ( wxFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL ) );
    m_buttonColorWithTorsion->SetFont     ( wxFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL ) );

    // Add the elements to the fiber info sizer.
    m_fibersInfoSizer->Add( m_fibersInfoGrid,              0, wxALL                          , 0 );
    m_fibersInfoSizer->Add( m_buttonDisplayMeanFiber,      0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_fibersInfoSizer->Add( m_buttonDisplayCrossSections,  0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_fibersInfoSizer->Add( m_buttonDisplayDispersionTube, 0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_fibersInfoSizer->Add( m_buttonColorWithCurvature,    0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_fibersInfoSizer->Add( m_buttonColorWithTorsion,      0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
}

///////////////////////////////////////////////////////////////////////////
// This function will set the correct information in the fiber info grid, 
// the information will be fetch from the SelectionObject*(function argument).
//
// i_selectedObject     : The information will be fetch from this selection object.
///////////////////////////////////////////////////////////////////////////
void MainFrame::SetFiberInfoGridValues( SelectionObject* i_selectedObject )
{
    // Only update the fiber grid if its actually on display.
    if( ! m_datasetHelper->m_displayFibersInfo || m_fibersInfoGrid == NULL )
        return;

    if( i_selectedObject == NULL )
    {
        DisplayFibersInfo( false );
        return;
    }
    
    int l_itemId = treeSelected( i_selectedObject->GetId() );
    
    // If we do not have a childbox or a masterbox, we simply dont update the info grid.
    if( l_itemId != CHILD_OBJECT && l_itemId != MASTER_OBJECT )
        return;

    // If we have a childbox, we update its masterbox.
    if( l_itemId == CHILD_OBJECT )
        i_selectedObject = (SelectionObject*)( m_treeWidget->GetItemData( m_treeWidget->GetItemParent( i_selectedObject->GetId() ) ) );

    // Set the name of the selection object as the "title" of the grid.
    SetFiberInfoGridName( i_selectedObject->getName() );

    FibersInfoGridParams l_params;
    i_selectedObject->calculateGridParams( l_params );

    // Fill the values.
    m_fibersInfoGrid->SetCellValue( 0,  0, wxString::Format( wxT( "%d" ), l_params.m_count            ) );
    m_fibersInfoGrid->SetCellValue( 1,  0, wxString::Format( wxT( "%f" ), l_params.m_meanValue        ) );
    m_fibersInfoGrid->SetCellValue( 2,  0, wxString::Format( wxT( "%f" ), l_params.m_meanLength       ) );
    m_fibersInfoGrid->SetCellValue( 3,  0, wxString::Format( wxT( "%f" ), l_params.m_minLength        ) );
    m_fibersInfoGrid->SetCellValue( 4,  0, wxString::Format( wxT( "%f" ), l_params.m_maxLength        ) );
    m_fibersInfoGrid->SetCellValue( 5,  0, wxString::Format( wxT( "%f" ), l_params.m_meanCrossSection ) );
    m_fibersInfoGrid->SetCellValue( 6,  0, wxString::Format( wxT( "%f" ), l_params.m_minCrossSection  ) );
    m_fibersInfoGrid->SetCellValue( 7,  0, wxString::Format( wxT( "%f" ), l_params.m_maxCrossSection  ) );
    m_fibersInfoGrid->SetCellValue( 8,  0, wxString::Format( wxT( "%f" ), l_params.m_meanCurvature    ) );
    m_fibersInfoGrid->SetCellValue( 9,  0, wxString::Format( wxT( "%f" ), l_params.m_meanTorsion      ) );
    m_fibersInfoGrid->SetCellValue( 10, 0, wxString::Format( wxT( "%f" ), l_params.m_dispersion       ) );
}

///////////////////////////////////////////////////////////////////////////
// This function set the title of the info grid.
///////////////////////////////////////////////////////////////////////////
void MainFrame::SetFiberInfoGridName( wxString i_name )
{
    if( m_fibersInfoGrid == NULL )
        return;
    
    m_fibersInfoGrid->SetColLabelValue( 0, i_name );
}

///////////////////////////////////////////////////////////////////////////
// This function set the titles in the fibers info grid, since those
// will not change, we only need to set them once!
///////////////////////////////////////////////////////////////////////////
void MainFrame::SetFiberInfoGridLabelTitlesValues()
{
    m_fibersInfoGrid->SetRowLabelValue( 0,  wxT( "Count"                   ) );
    m_fibersInfoGrid->SetRowLabelValue( 1,  wxT( "Mean Value"              ) );
    m_fibersInfoGrid->SetRowLabelValue( 2,  wxT( "Mean Length (mm)"        ) );
    m_fibersInfoGrid->SetRowLabelValue( 3,  wxT( "Min Length (mm)"         ) );
    m_fibersInfoGrid->SetRowLabelValue( 4,  wxT( "Max Length (mm)"         ) );
    m_fibersInfoGrid->SetRowLabelValue( 5,  wxT( "Mean Cross Section (mm)" ) );
    m_fibersInfoGrid->SetRowLabelValue( 6,  wxT( "Min Cross Section (mm)"  ) );
    m_fibersInfoGrid->SetRowLabelValue( 7,  wxT( "Max Cross Section (mm)"  ) );
    m_fibersInfoGrid->SetRowLabelValue( 8,  wxT( "Mean Curvature"          ) );
    m_fibersInfoGrid->SetRowLabelValue( 9,  wxT( "Mean Torsion"            ) );
    m_fibersInfoGrid->SetRowLabelValue( 10, wxT( "Dispersion"              ) );
    // Setting the size manually.
    m_fibersInfoGrid->SetRowLabelSize( FIBERS_INFO_GRID_TITLE_LABEL_SIZE );
}

///////////////////////////////////////////////////////////////////////////
// This function will display or hide the glyph options depending on 
// the value of the argument.
//
// i_display        : If true, the options will be shown, if false,
//                    the options will be hiden.
// i_glyph          : The glyph item that we need to hide/show the options.
///////////////////////////////////////////////////////////////////////////
void MainFrame::DisplayGlyphOptions( bool i_display, DatasetInfo* i_glyph )
{      
    // We need to show the glyph options.
	if( i_display )
	{
		if( m_glyphOptionsSizer == NULL )
	    {
		    // Contains the glyph options items.
		    m_glyphOptionsSizer = new wxBoxSizer( wxVERTICAL );
            InitializeGlpyhOptionsSizerElements( i_glyph );
            m_rightMainSizer->Add( m_glyphOptionsSizer, 0, wxALL, 1 );
	    }
        
        m_datasetHelper->m_displayGlyphOptions = true;
        SetGlyphOptionsValues( i_glyph );
        m_rightMainSizer->Show( m_glyphOptionsSizer, true, true );
	    m_rightMainSizer->Layout();
	}
	else // We need to hide the glyph options.
	{
		// There is no point in hiding the glyph options if they are not even created.
        if( m_glyphOptionsSizer == NULL )
            return;

        m_datasetHelper->m_displayGlyphOptions = false;
        m_rightMainSizer->Hide( m_glyphOptionsSizer, true );
	    m_rightMainSizer->Layout();
	}
	
    // Adding or removing the m_glyphOptionsSizer will cause the OpenGl window to resize, so we need to refresh it.
	if( m_mainGL )
        m_mainGL->changeOrthoSize();
}

///////////////////////////////////////////////////////////////////////////
// This function will initialize the elements that are part of the glyph options sizer.
// The information will be fetch from i_glyph*(function argument).
//
// i_glyph     : The information will be fetch from this tensors.
///////////////////////////////////////////////////////////////////////////
void MainFrame::InitializeGlpyhOptionsSizerElements( DatasetInfo* i_glyph )
{ 
    // Initialize the color slider.
    m_glyphMinHueValue     = new wxSlider( this, ID_GLYPH_OPTIONS_MIN_HUE_SLIDER,    ( ( (Glyph*)i_glyph )->getColor( MIN_HUE )    * 100 ), 0, 100, wxDefaultPosition, wxSize( 103, 19 ) );
    m_glyphMaxHueValue     = new wxSlider( this, ID_GLYPH_OPTIONS_MAX_HUE_SLIDER,    ( ( (Glyph*)i_glyph )->getColor( MAX_HUE )    * 100 ), 0, 100, wxDefaultPosition, wxSize( 103, 19 ) );
    m_glyphSaturationValue = new wxSlider( this, ID_GLYPH_OPTIONS_SATURATION_SLIDER, ( ( (Glyph*)i_glyph )->getColor( SATURATION ) * 100 ), 0, 100, wxDefaultPosition, wxSize( 103, 19 ) );
    m_glyphLuminanceValue  = new wxSlider( this, ID_GLYPH_OPTIONS_LUMINANCE_SLIDER,  ( ( (Glyph*)i_glyph )->getColor( LUMINANCE )  * 100 ), 0, 100, wxDefaultPosition, wxSize( 103, 19 ) );

    // Initialize the slider static text.
    m_minHueSliderText           = new wxStaticText( this, ID_GLYPH_OPTIONS_SLIDER_TEXT, _T( "Min Hue" )           );
    m_maxHueSliderText           = new wxStaticText( this, ID_GLYPH_OPTIONS_SLIDER_TEXT, _T( "Max Hue" )           );
    m_saturationSliderText       = new wxStaticText( this, ID_GLYPH_OPTIONS_SLIDER_TEXT, _T( "Saturation" )        );
    m_luminanceSliderText        = new wxStaticText( this, ID_GLYPH_OPTIONS_SLIDER_TEXT, _T( "Luminance" )         );
    m_LODSliderText              = new wxStaticText( this, ID_GLYPH_OPTIONS_SLIDER_TEXT, _T( "Details" )           );
    m_lightAttenuationSliderText = new wxStaticText( this, ID_GLYPH_OPTIONS_SLIDER_TEXT, _T( "Light Attenuation" ) );
    m_lightXPositionSliderText   = new wxStaticText( this, ID_GLYPH_OPTIONS_SLIDER_TEXT, _T( "Light x Position" )  );
    m_lightYPositionSliderText   = new wxStaticText( this, ID_GLYPH_OPTIONS_SLIDER_TEXT, _T( "Light y Position" )  );
    m_lightZPositionSliderText   = new wxStaticText( this, ID_GLYPH_OPTIONS_SLIDER_TEXT, _T( "Light z Position" )  );
    m_displayRatioSliderText     = new wxStaticText( this, ID_GLYPH_OPTIONS_SLIDER_TEXT, _T( "Display" )           );
    m_scalingFactorText          = new wxStaticText( this, ID_GLYPH_OPTIONS_SLIDER_TEXT, _T( "Scaling Factor" )    );

    // Initialize the LOD slider.
    m_glyphLODValue         = new wxSlider    ( this, ID_GLYPH_OPTIONS_LOD_SLIDER,              ( (Glyph*)i_glyph )->getLOD(), 0, NB_OF_LOD - 1, wxDefaultPosition, wxSize( 103, 19 ) );
    // Initialize the light attenuation slider.
    m_glyphLightAttenuation = new wxSlider    ( this, ID_GLYPH_OPTIONS_LIGHT_ATTENUATION_SLIDER,( (Glyph*)i_glyph )->getLightAttenuation() * 100, 0, 100, wxDefaultPosition, wxSize( 103, 19 ) );
    // Initialize the light position sliders.
    m_glyphLightXPosition   = new wxSlider    ( this, ID_GLYPH_OPTIONS_LIGHT_X_POSITION_SLIDER, ( (Glyph*)i_glyph )->getLightPosition( X_AXIS ) * 100, -100, 100, wxDefaultPosition, wxSize( 103, 19 ) );
    m_glyphLightYPosition   = new wxSlider    ( this, ID_GLYPH_OPTIONS_LIGHT_Y_POSITION_SLIDER, ( (Glyph*)i_glyph )->getLightPosition( Y_AXIS ) * 100, -100, 100, wxDefaultPosition, wxSize( 103, 19 ) );
    m_glyphLightZPosition   = new wxSlider    ( this, ID_GLYPH_OPTIONS_LIGHT_Z_POSITION_SLIDER, ( (Glyph*)i_glyph )->getLightPosition( Z_AXIS ) * 100, -100, 100, wxDefaultPosition, wxSize( 103, 19 ) );
    // Initialize the display slider.
    m_glyphDisplayValue     = new wxSlider( this, ID_GLYPH_OPTIONS_DISPLAY_SLIDER,              ( (Glyph*)i_glyph )->getDisplayFactor(), 1,  20, wxDefaultPosition, wxSize( 103, 19 ) );
    // Initialize the scaling slider.
    m_glyphScalingFactor    = new wxSlider( this, ID_GLYPH_OPTIONS_SCALING_FACTOR_SLIDER,       ( (Glyph*)i_glyph )->getScalingFactor(), 1, 20, wxDefaultPosition, wxSize( 103, 19 ) );
    
    // Initialize the flip x, y z and map on sphere checkboxes.
    m_xAxisFlip         = new wxCheckBox( this, ID_GLYPH_OPTIONS_X_AXIS_FLIP,         _T( "x Flip" )              );
    m_yAxisFlip         = new wxCheckBox( this, ID_GLYPH_OPTIONS_Y_AXIS_FLIP,         _T( "Y Flip" )              );
    m_zAxisFlip         = new wxCheckBox( this, ID_GLYPH_OPTIONS_Z_AXIS_FLIP,         _T( "Z Flip" )              );
    m_colorWithPosition = new wxCheckBox( this, ID_GLYPH_OPTIONS_COLOR_WITH_POSITION, _T( "Color With Position" ) );
    
    m_radioNormal       = new wxRadioButton(this, ID_GLYPH_OPTIONS_RADIO_NORMAL,        _T( "Normal" ));
    m_radioMapOnSphere  = new wxRadioButton(this, ID_GLYPH_OPTIONS_RADIO_MAP_ON_SPHERE, _T( "Map On Sphere" ));
    m_radioAxes         = new wxRadioButton(this, ID_GLYPH_OPTIONS_RADIO_AXES,          _T( "Diffusion Axes" ));
    m_radioMainAxis     = new wxRadioButton(this, ID_GLYPH_OPTIONS_RADIO_MAIN_AXIS,     _T( "Main Diffusion Axis" ));



    // Add the elements to the glyph options sizer.
    m_glyphOptionsSizer->Add( m_minHueSliderText,           0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_glyphMinHueValue,           0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_maxHueSliderText,           0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_glyphMaxHueValue,           0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_saturationSliderText,       0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_glyphSaturationValue,       0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_luminanceSliderText,        0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_glyphLuminanceValue,        0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_LODSliderText,              0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_glyphLODValue,              0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_lightAttenuationSliderText, 0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_glyphLightAttenuation,      0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_lightXPositionSliderText,   0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_glyphLightXPosition,        0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_lightYPositionSliderText,   0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_glyphLightYPosition,        0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_lightZPositionSliderText,   0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_glyphLightZPosition,        0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_displayRatioSliderText,     0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_glyphDisplayValue,          0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_scalingFactorText,          0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_glyphScalingFactor,         0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_xAxisFlip,                  0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_yAxisFlip,                  0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_zAxisFlip,                  0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_colorWithPosition,          0, wxALL | wxALIGN_CENTER | wxGROW, 0 );
    m_glyphOptionsSizer->Add( m_radioNormal,             0, wxALL | wxALIGN_CENTER | wxGROW, 0 );   
    m_glyphOptionsSizer->Add( m_radioMapOnSphere,           0, wxALL | wxALIGN_CENTER | wxGROW, 0 );      
    m_glyphOptionsSizer->Add( m_radioAxes,                  0, wxALL | wxALIGN_CENTER | wxGROW, 0 );   
    m_glyphOptionsSizer->Add( m_radioMainAxis,              0, wxALL | wxALIGN_CENTER | wxGROW, 0 );

    // We need to refresh those newly created slider for them to start at the good value.
    m_glyphMinHueValue->Refresh();
    m_glyphMaxHueValue->Refresh();
    m_glyphLuminanceValue->Refresh();
    m_glyphSaturationValue->Refresh();
    m_glyphLODValue->Refresh();
    m_glyphLightAttenuation->Refresh();
    m_glyphLightXPosition->Refresh();
    m_glyphLightYPosition->Refresh();
    m_glyphLightZPosition->Refresh();
    m_glyphDisplayValue->Refresh();
    m_glyphScalingFactor->Refresh();
}

///////////////////////////////////////////////////////////////////////////
// This function will set the correct information in the glyph options menu,
// the information will be fetch from i_glyph*(function argument).
//
// i_info       : The information will be fetch from this tensors.
///////////////////////////////////////////////////////////////////////////
void MainFrame::SetGlyphOptionsValues( DatasetInfo* i_info )
{
    // Validate the pointer and only update the glyph options menu if its actually on display.
    if( i_info == NULL || ! m_datasetHelper->m_displayGlyphOptions )
        return;

    if( i_info->getType() == TENSORS || i_info->getType() == ODFS )
    {
        m_glyphMinHueValue->SetValue     (      (   (Glyph*)i_info )->getColor( MIN_HUE )    * 100     );
        m_glyphMaxHueValue->SetValue     (      (   (Glyph*)i_info )->getColor( MAX_HUE )    * 100     );
        m_glyphSaturationValue->SetValue (      (   (Glyph*)i_info )->getColor( SATURATION ) * 100     );
        m_glyphLuminanceValue->SetValue  (      (   (Glyph*)i_info )->getColor( LUMINANCE )  * 100     );
        m_glyphLODValue->SetValue        ( (int)( ( (Glyph*)i_info )->getLOD() )                       );
        m_glyphLightAttenuation->SetValue(      (   (Glyph*)i_info )->getLightAttenuation() * 100      );
        m_glyphLightXPosition->SetValue  (      (   (Glyph*)i_info )->getLightPosition( X_AXIS ) * 100 );
        m_glyphLightYPosition->SetValue  (      (   (Glyph*)i_info )->getLightPosition( Y_AXIS ) * 100 );
        m_glyphLightZPosition->SetValue  (      (   (Glyph*)i_info )->getLightPosition( Z_AXIS ) * 100 );
        m_glyphDisplayValue->SetValue    (      (   (Glyph*)i_info )->getDisplayFactor()               );
        m_glyphScalingFactor->SetValue   (      (   (Glyph*)i_info )->getScalingFactor()               );
        m_xAxisFlip->SetValue            (      (   (Glyph*)i_info )->isAxisFlipped( X_AXIS )          );
        m_yAxisFlip->SetValue            (      (   (Glyph*)i_info )->isAxisFlipped( Y_AXIS )          );
        m_zAxisFlip->SetValue            (      (   (Glyph*)i_info )->isAxisFlipped( Z_AXIS )          );
        m_colorWithPosition->SetValue    (      (   (Glyph*)i_info )->getColorWithPosition()           );
        m_radioNormal->SetValue          (      (   (Glyph*)i_info )->isDisplayShape(NORMAL)           );
        m_radioMapOnSphere->SetValue     (      (   (Glyph*)i_info )->isDisplayShape(SPHERE)           );
        m_radioAxes->SetValue            (      (   (Glyph*)i_info )->isDisplayShape(AXES)             );
        m_radioMainAxis->SetValue        (      ((  Glyph*)i_info )->isDisplayShape(AXIS)              );

        // Since some options are only available for the tensors or the odfs, we disable or enable them here.
        if( i_info->getType() == TENSORS )
        {
            m_glyphLightAttenuation->Enable();
            m_glyphLightXPosition->Enable  ();
            m_glyphLightYPosition->Enable  ();
            m_glyphLightZPosition->Enable  ();
            m_glyphScalingFactor->Enable   ();
            
            m_radioAxes->Enable            ();
            m_radioMainAxis->Enable        ();
        }
        else
        {           
            m_radioAxes->Enable            ( false );
            m_radioMainAxis->Enable        ( false );
            m_glyphLightAttenuation->Enable( false );
            m_glyphLightXPosition->Enable  ( false );
            m_glyphLightYPosition->Enable  ( false );
            m_glyphLightZPosition->Enable  ( false );
            m_glyphScalingFactor->Enable   ( false );
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnCreateColorTexture( wxCommandEvent& WXUNUSED(event) )
{
    Fibers* l_fibers = NULL;

    if( ! m_datasetHelper->getFiberDataset(l_fibers) )
        return ;

    int l_x,l_y,l_z;

    Anatomy* l_newAnatomy = new Anatomy( m_datasetHelper );
    l_newAnatomy->setRGBZero( m_datasetHelper->m_columns, m_datasetHelper->m_rows, m_datasetHelper->m_frames );

    wxTreeItemId l_treeObjectId = m_treeWidget->GetSelection();
    if( treeSelected( l_treeObjectId ) == MASTER_OBJECT )
    {
        SelectionObject* l_object = (SelectionObject*)( m_treeWidget->GetItemData( l_treeObjectId ) );
        wxColour l_color = l_object->getFiberColor();

        std::vector<float>* l_dataset = l_newAnatomy->getFloatDataset();

        for( int l = 0; l < l_fibers->getLineCount(); ++l )
        {
            if( l_object->m_inBranch[l] )
            {
                unsigned int pc = l_fibers->getStartIndexForLine( l ) * 3;

                for( int j = 0; j < l_fibers->getPointsPerLine( l ) ; ++j )
                {
                    l_x = (int)( l_fibers->getPointValue( pc ) / m_datasetHelper->m_xVoxel );
                    ++pc;
                    l_y = (int)( l_fibers->getPointValue( pc ) / m_datasetHelper->m_yVoxel );
                    ++pc;
                    l_z = (int)( l_fibers->getPointValue( pc ) / m_datasetHelper->m_zVoxel );
                    ++pc;

                    int index = ( l_x + l_y * m_datasetHelper->m_columns + l_z * m_datasetHelper->m_columns * m_datasetHelper->m_rows ) * 3;
                    l_dataset->at( index )     = l_color.Red()   / 255.0f;
                    l_dataset->at( index + 1 ) = l_color.Green() / 255.0f;
                    l_dataset->at( index + 2 ) = l_color.Blue()  / 255.0f;
                }
            }
        }
    }

    l_newAnatomy->setName( wxT( " (fiber_colors)" ) );

    m_listCtrl->InsertItem( 0, wxT( "" ), 0 );
    m_listCtrl->SetItem( 0, 1, l_newAnatomy->getName() );
    m_listCtrl->SetItem( 0, 2, wxT( "0.00" ) );
    m_listCtrl->SetItem( 0, 3, wxT( "" ), 1 );
    m_listCtrl->SetItemData( 0, (long)l_newAnatomy );
    m_listCtrl->SetItemState( 0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );

    refreshAllGLWidgets();

    Anatomy* l_newAnatomy2 = new Anatomy( m_datasetHelper );
    l_newAnatomy2->setZero( m_datasetHelper->m_columns, m_datasetHelper->m_rows, m_datasetHelper->m_frames );
    l_newAnatomy2->setType( OVERLAY );
    float l_max = 0.0f;
    if( treeSelected( l_treeObjectId ) == MASTER_OBJECT )
    {
        SelectionObject* l_object = (SelectionObject*)( m_treeWidget->GetItemData( l_treeObjectId ) );

        std::vector<float>* l_dataset = l_newAnatomy2->getFloatDataset();

        for( int l = 0; l < l_fibers->getLineCount(); ++l )
        {
            if( l_object->m_inBranch[l] )
            {
                unsigned int pc = l_fibers->getStartIndexForLine(l)*3;

                for( int j = 0; j < l_fibers->getPointsPerLine(l) ; ++j )
                {
                    l_x = (int)( l_fibers->getPointValue(pc) / m_datasetHelper->m_xVoxel );
                    ++pc;
                    l_y = (int)( l_fibers->getPointValue(pc) / m_datasetHelper->m_yVoxel );
                    ++pc;
                    l_z = (int)( l_fibers->getPointValue(pc) / m_datasetHelper->m_zVoxel );
                    ++pc;

                    int index =( l_x + l_y * m_datasetHelper->m_columns + l_z * m_datasetHelper->m_columns * m_datasetHelper->m_rows );
                    l_dataset->at(index) += 1.0;
                    l_max = wxMax( l_max,l_dataset->at(index) );
                }
            }
        }
        for( int i = 0 ; i < m_datasetHelper->m_columns * m_datasetHelper->m_rows * m_datasetHelper->m_frames ; ++i )
        {
            l_dataset->at(i) /= l_max;
        }
    }

    l_newAnatomy2->setName( wxT(" (fiber_density)" ) );
    l_newAnatomy2->setOldMax( l_max );
    m_listCtrl->InsertItem( 0, wxT( "" ), 0 );
    m_listCtrl->SetItem( 0, 1, l_newAnatomy2->getName() );
    m_listCtrl->SetItem( 0, 2, wxT( "0.00" ) );
    m_listCtrl->SetItem( 0, 3, wxT( "" ), 1 );
    m_listCtrl->SetItemData( 0, (long) l_newAnatomy2 );
    m_listCtrl->SetItemState( 0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );

    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will create a new selection object of type ellipsoid, it will be 
// triggered when the user click on Ellipsoid sub menu under the New Selection-Object
// sub menu under the VOI menu.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnNewSelectionEllipsoid( wxCommandEvent& WXUNUSED(event) )
{
    CreateNewSelectionObject( ELLIPSOID_TYPE );
}

///////////////////////////////////////////////////////////////////////////
// This function will create a new selection object of type box, it will be 
// triggered when the user click on Box sub menu under the New Selection-Object
// sub menu under the VOI menu.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnNewSelectionBox( wxCommandEvent& WXUNUSED(event) )
{
    CreateNewSelectionObject( BOX_TYPE );
}

///////////////////////////////////////////////////////////////////////////
// This function will create a new selection object of type depending of the argument.
//
// i_newSelectionObjectType         : The type of the new selection object we wat to create.
///////////////////////////////////////////////////////////////////////////
void MainFrame::CreateNewSelectionObject( ObjectType i_newSelectionObjectType )
{
    if( ! m_datasetHelper->m_theScene || ! m_datasetHelper->m_fibersLoaded )
        return;

    Vector l_center( m_xSlider->GetValue() * m_datasetHelper->m_xVoxel, 
                     m_ySlider->GetValue() * m_datasetHelper->m_yVoxel, 
                     m_zSlider->GetValue() * m_datasetHelper->m_zVoxel );

    float xs   = m_datasetHelper->m_columns * m_datasetHelper->m_xVoxel;
    float ys   = m_datasetHelper->m_rows    * m_datasetHelper->m_yVoxel;
    float zs   = m_datasetHelper->m_frames  * m_datasetHelper->m_zVoxel;
    float mins = wxMax( xs, wxMax( ys, zs ) ) / 8;

    Vector l_size( mins / m_datasetHelper->m_xVoxel, 
                   mins / m_datasetHelper->m_yVoxel,
                   mins / m_datasetHelper->m_zVoxel );

    SelectionObject* l_newSelectionObject;
    if( i_newSelectionObjectType == ELLIPSOID_TYPE )
        l_newSelectionObject = new SelectionEllipsoid( l_center, l_size, m_datasetHelper );
    else if( i_newSelectionObjectType == BOX_TYPE )
        l_newSelectionObject = new SelectionBox( l_center, l_size, m_datasetHelper );
    else
        return;

    // Check what is selected in the tree to know where to put this new selection object.
	wxTreeItemId l_treeSelectionId = m_treeWidget->GetSelection();

    wxTreeItemId l_newSelectionObjectId;
    
    if( treeSelected( l_treeSelectionId ) == MASTER_OBJECT )
    {
        // Our new seleciton object is under another master selection object.
		l_newSelectionObjectId = m_treeWidget->AppendItem( l_treeSelectionId, l_newSelectionObject->getName(), 0, -1, l_newSelectionObject );
        m_treeWidget->SetItemBackgroundColour( l_newSelectionObjectId, *wxGREEN );
    }
    else if( treeSelected( l_treeSelectionId ) == CHILD_OBJECT )
    {
		wxTreeItemId l_parentId = m_treeWidget->GetItemParent( l_treeSelectionId );

        // Our new seleciton object is under another child selection object.
        l_newSelectionObjectId = m_treeWidget->AppendItem( l_parentId, l_newSelectionObject->getName(), 0, -1, l_newSelectionObject );
        m_treeWidget->SetItemBackgroundColour( l_newSelectionObjectId, *wxGREEN );
    }
    else
    {
        // Our new seleciton object is on top.
        l_newSelectionObject->setIsMaster( true );
        l_newSelectionObjectId = m_treeWidget->AppendItem( m_tSelectionObjectsId, l_newSelectionObject->getName(), 0, -1, l_newSelectionObject );
        m_treeWidget->SetItemBackgroundColour( l_newSelectionObjectId, *wxCYAN );
    }

    m_treeWidget->EnsureVisible( l_newSelectionObjectId );
    m_treeWidget->SetItemImage( l_newSelectionObjectId, l_newSelectionObject->getIcon() );
    l_newSelectionObject->setTreeId( l_newSelectionObjectId );

    m_datasetHelper->m_selBoxChanged = true;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnNewFromOverlay( wxCommandEvent& WXUNUSED(event) )
{
    wxTreeItemId     l_treeObjectId = m_treeWidget->GetSelection();
    wxTreeItemId     l_parentId    = m_treeWidget->GetItemParent( l_treeObjectId );
    SelectionObject* l_selectionObject = NULL;
    Anatomy*         l_anatomy      = NULL;

    long item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if(item != -1)
    {
        DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( item );
        if (l_info->getType() < RGB)
        {
            l_anatomy = (Anatomy*)m_listCtrl->GetItemData( item );
            l_selectionObject = new SelectionBox( m_datasetHelper, l_anatomy );
            float trs = l_anatomy->getThreshold();
            if( trs == 0.0 )
                trs = 0.01f;
            l_selectionObject->setThreshold( trs );
        }
        else
            return;
    }
    else
        return;

    if( treeSelected( l_treeObjectId ) == MASTER_OBJECT )
    {
        wxTreeItemId l_treeNewObjectId = m_treeWidget->AppendItem( l_treeObjectId, wxT( "ROI" ), 0, -1, l_selectionObject );
        m_treeWidget->SetItemBackgroundColour( l_treeNewObjectId, *wxGREEN );
        m_treeWidget->EnsureVisible( l_treeNewObjectId );
        m_treeWidget->SetItemImage( l_treeNewObjectId, l_selectionObject->getIcon() );
        l_selectionObject->setTreeId( l_treeNewObjectId );
        l_selectionObject->setIsMaster( false );
    }
    else
    {
        wxTreeItemId l_treeNewObjectId = m_treeWidget->AppendItem( m_tSelectionObjectsId, wxT( "ROI" ), 0, -1, l_selectionObject );
        m_treeWidget->SetItemBackgroundColour( l_treeNewObjectId, *wxCYAN );
        m_treeWidget->EnsureVisible( l_treeNewObjectId );
        m_treeWidget->SetItemImage( l_treeNewObjectId, l_selectionObject->getIcon() );
        l_selectionObject->setTreeId( l_treeNewObjectId );
        l_selectionObject->setIsMaster( true );

    }

    l_anatomy->m_roi = l_selectionObject;

    m_datasetHelper->m_selBoxChanged = true;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnHideSelectionObjects( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene )
        return;

    m_datasetHelper->toggleSelectionObjects();
    refreshAllGLWidgets();
}


///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnUseMorph( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene )
        return;

    m_datasetHelper->m_morphing = ! m_datasetHelper->m_morphing;
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnColorRoi( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene || ! m_datasetHelper->m_fibersLoaded )
        return;

    // Get the currently selected object.
    wxTreeItemId l_selectionObjectTreeId = m_treeWidget->GetSelection();
    SelectionObject* l_selectionObject = (SelectionObject*)( m_treeWidget->GetItemData( l_selectionObjectTreeId ) );

    wxColourData l_colorData;

    for( int i = 0; i < 10; ++i )
    {
        wxColour l_color( i * 28, i * 28, i * 28 );
        l_colorData.SetCustomColour( i, l_color );
    }

    int i = 10;
    wxColour color ( 255, 0,   0   );
    wxColour color1( 0,   255, 0   );
    wxColour color2( 0,   0,   255 );
    wxColour color3( 255, 255, 0   );
    wxColour color4( 255, 0,   255 );
    wxColour color5( 0,   255, 255 );

    l_colorData.SetCustomColour( i++, color  );
    l_colorData.SetCustomColour( i++, color1 );
    l_colorData.SetCustomColour( i++, color2 );
    l_colorData.SetCustomColour( i++, color3 );
    l_colorData.SetCustomColour( i++, color4 );
    l_colorData.SetCustomColour( i++, color5 );

    wxColourDialog dialog( this, &l_colorData );
    wxColour l_color;

    if( dialog.ShowModal() == wxID_OK )
    {
        wxColourData retData = dialog.GetColourData();
        l_color = retData.GetColour();
    }
    else
        return;

    l_selectionObject->setColor( l_color );

    refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 * Menu Spline Surface
 *
 ****************************************************************************************************/

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnNewSurface( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene || m_datasetHelper->m_surfaceLoaded )
        return;

    int l_xs = (int)( m_xSlider->GetValue() * m_datasetHelper->m_xVoxel );
    int l_ys = (int)( m_ySlider->GetValue() * m_datasetHelper->m_yVoxel );
    int l_zs = (int)( m_zSlider->GetValue() * m_datasetHelper->m_zVoxel );

    // Delete all existing points.
    m_treeWidget->DeleteChildren( m_tPointId );
    Fibers* l_fibers = NULL;

    if( m_datasetHelper->m_fibersLoaded )
        m_datasetHelper->getFiberDataset( l_fibers );

    if( m_datasetHelper->m_showSagittal )
    {
        for( int i = 0; i < 11; ++i )
        {
            for( int j = 0; j < 11; ++j )
            {
                int yy = (int)( ( m_datasetHelper->m_rows   / 10 * m_datasetHelper->m_yVoxel ) * i );
                int zz = (int)( ( m_datasetHelper->m_frames / 10 * m_datasetHelper->m_zVoxel ) * j );

                // Create the point.
                SplinePoint* l_point = new SplinePoint( l_xs, yy, zz, m_datasetHelper );

                if( i == 0 || i == 10 || j == 0 || j == 10 )
                {
                    wxTreeItemId tId = m_treeWidget->AppendItem( m_tPointId, wxT("boundary l_point"), -1, -1, l_point );
                    l_point->setTreeId( tId );
                    l_point->setIsBoundary( true );
                }
                else
                {
                    if( m_datasetHelper->m_fibersLoaded && l_fibers->getBarycenter( l_point ) )
                    {
                        wxTreeItemId tId = m_treeWidget->AppendItem( m_tPointId, wxT( "l_point" ), -1, -1, l_point );
                        l_point->setTreeId( tId );
                        l_point->setIsBoundary( false );
                    }
                }
            }
        }
    }
    else if( m_datasetHelper->m_showCoronal )
    {
        for( int i = 0; i < 11; ++i )
        {
            for( int j = 0; j < 11; ++j )
            {
                int l_xx = (int)( ( m_datasetHelper->m_columns / 10 * m_datasetHelper->m_xVoxel ) * i );
                int l_zz = (int)( ( m_datasetHelper->m_frames  / 10 * m_datasetHelper->m_zVoxel ) * j );

                // Create the point.
                SplinePoint* l_point = new SplinePoint( l_xx, l_ys, l_zz, m_datasetHelper );

                if( i == 0 || i == 10 || j == 0 || j == 10 )
                {
                    wxTreeItemId l_treeId = m_treeWidget->AppendItem( m_tPointId, wxT("boundary l_point"), -1, -1, l_point );
                    l_point->setTreeId( l_treeId );
                    l_point->setIsBoundary( true );
                }
            }
        }
    }
    else
    {
        for( int i = 0; i < 11; ++i )
        {
            for( int j = 0; j < 11; ++j )
            {
                int l_xx = (int)( ( m_datasetHelper->m_columns / 10 * m_datasetHelper->m_xVoxel ) * i );
                int l_yy = (int)( ( m_datasetHelper->m_rows    / 10 * m_datasetHelper->m_yVoxel ) * j );

                // Create the point.
                SplinePoint* l_point = new SplinePoint( l_xx, l_yy, l_zs, m_datasetHelper );

                if (i == 0 || i == 10 || j == 0 || j == 10)
                {
                    wxTreeItemId l_treeId = m_treeWidget->AppendItem( m_tPointId, wxT("boundary l_point"), -1, -1, l_point );
                    l_point->setTreeId( l_treeId );
                    l_point->setIsBoundary( true );
                }
            }
        }
    }

#ifdef __WXMAC__
    // insert at zero is a well-known bug on OSX, so we append there...
    // http://trac.wxwidgets.org/ticket/4492
    long id = m_listCtrl->GetItemCount();
#else
    long id = 0;
#endif

    Surface* l_surface = new Surface(m_datasetHelper);
    l_surface->execute();

    m_listCtrl->InsertItem( id, wxT( "" ), 0 );
    m_listCtrl->SetItem( id, 1, l_surface->getName() );
    m_listCtrl->SetItem( id, 2, wxT( "0.50" ) );
    m_listCtrl->SetItem( id, 3, wxT( "" ), 1 );
    m_listCtrl->SetItemData( id, (long)l_surface );
    m_listCtrl->SetItemState( id, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );

    m_datasetHelper->m_surfaceLoaded = true;

    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnTogglePointMode( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene )
        return;

    m_datasetHelper->togglePointMode();
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnToggleDrawVectors( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene )
        return;

    m_datasetHelper->m_drawVectors = ! m_datasetHelper->m_drawVectors;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnToggleLIC( wxCommandEvent& WXUNUSED(event) )
{
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item != -1 && m_datasetHelper->m_vectorsLoaded )
    {
        DatasetInfo* l_info = (DatasetInfo*) m_listCtrl->GetItemData( l_item );
        l_info->activateLIC();
    }
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnToggleNormal( wxCommandEvent& WXUNUSED(event ))
{
    m_datasetHelper->m_normalDirection *= -1.0;

    for( int i = 0; i < m_listCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_info = (DatasetInfo*) m_listCtrl->GetItemData( i );
        if( l_info->getType() == SURFACE )
        {
            Surface* l_surf = (Surface*)m_listCtrl->GetItemData( i );
            l_surf->flipNormals();
        }
    }
    refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 * Menu Options
 *
 ****************************************************************************************************/

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnToggleTextureFiltering( wxCommandEvent& WXUNUSED(event) )
{
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item != -1 )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
        if( l_info->getType() < MESH )
        {
            if( ! l_info->toggleShowFS() )
                m_listCtrl->SetItem( l_item, 1, l_info->getName() + wxT( "*" ) );
            else
                m_listCtrl->SetItem( l_item, 1, l_info->getName() );
        }
    }
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnToggleLighting( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_lighting = ! m_datasetHelper->m_lighting;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnToggleBlendTexOnMesh( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_blendTexOnMesh = ! m_datasetHelper->m_blendTexOnMesh;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnToggleFilterIso( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_filterIsoSurf = ! m_datasetHelper->m_filterIsoSurf;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnClean( wxCommandEvent& WXUNUSED(event) )
{
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if(l_item != -1 )
    {
        DatasetInfo* l_info = (DatasetInfo*) m_listCtrl->GetItemData( l_item );
        if( l_info->getType() == MESH || l_info->getType() == ISO_SURFACE)
           l_info->clean();
    }
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnLoop( wxCommandEvent& WXUNUSED(event) )
{
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item != -1 )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
        l_info->smooth();
    }
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnInvertFibers( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->invertFibers();
    m_datasetHelper->m_selBoxChanged = true;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnUseFakeTubes( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_useFakeTubes = !m_datasetHelper->m_useFakeTubes;
    Fibers* l_fiber;
    if( m_datasetHelper->getFiberDataset( l_fiber ) )
        l_fiber->switchNormals( ! m_datasetHelper->m_useFakeTubes );
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will changes the clear color to black or white.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnClearToBlack( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_clearToBlack = ! m_datasetHelper->m_clearToBlack;
    
    if( m_datasetHelper->m_clearToBlack )
        glClearColor( 0.0, 0.0, 0.0, 0.0 );
    else
        glClearColor( 1.0, 1.0, 1.0, 0.0 );

    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnUseTransparency( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_useTransparency = ! m_datasetHelper->m_useTransparency;
    //	Fibers* f;
    //	if ( m_datasetHelper->getFiberDataset(f))
    //		f->switchNormals(!m_datasetHelper->m_useFakeTubes);
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnAssignColor( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene )
        return;

    wxColourData l_colorData;

    for( int i = 0; i < 10; ++i )
    {
        wxColour l_color(i * 28, i * 28, i * 28);
        l_colorData.SetCustomColour(i, l_color);
    }

    int i = 10;
    wxColour l_color ( 255, 0, 0 );
    l_colorData.SetCustomColour( i++, l_color );
    wxColour l_color1( 0, 255, 0 );
    l_colorData.SetCustomColour( i++, l_color1 );
    wxColour l_color2( 0, 0, 255 );
    l_colorData.SetCustomColour( i++, l_color2 );
    wxColour l_color3( 255, 255, 0 );
    l_colorData.SetCustomColour( i++, l_color3 );
    wxColour l_color4( 255, 0, 255 );
    l_colorData.SetCustomColour( i++, l_color4 );
    wxColour l_color5( 0, 255, 255 );
    l_colorData.SetCustomColour( i++, l_color5 );

    wxColourDialog dialog( this, &l_colorData );
    wxColour l_col;
    if( dialog.ShowModal() == wxID_OK )
    {
        wxColourData l_retData = dialog.GetColourData();
        l_col = l_retData.GetColour();
    }
    else
        return;

    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item != -1 )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
        if( l_info->getType() == MESH ||
            l_info->getType() == ISO_SURFACE ||
            l_info->getType() == SURFACE || 
            l_info->getType() == VECTORS)
        {
            l_info->setColor( l_col );
            l_info->setuseTex( false );
            m_listCtrl->SetItem( l_item, 2, wxT( "(") + wxString::Format( wxT( "%.2f" ), l_info->getThreshold() ) + wxT( ")" ) );
            refreshAllGLWidgets();
            return;
        }
    }

    wxTreeItemId l_selectionObjectTreeId = m_treeWidget->GetSelection();
    if( treeSelected( l_selectionObjectTreeId ) == MASTER_OBJECT )
    {
        SelectionObject* l_selectionObject = (SelectionObject*)( m_treeWidget->GetItemData( l_selectionObjectTreeId ) );
        l_selectionObject->setFiberColor( l_col );
        l_selectionObject->setIsDirty( true );
    }
    else
    {
        if(treeSelected( l_selectionObjectTreeId ) == CHILD_OBJECT )
        {
            SelectionObject* l_selectionObject = (SelectionObject*)( m_treeWidget->GetItemData( m_treeWidget->GetItemParent( l_selectionObjectTreeId ) ) );
            l_selectionObject->setFiberColor( l_col );
            l_selectionObject->setIsDirty( true );
        }
    }

    m_datasetHelper->m_selBoxChanged = true;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnResetColor(wxCommandEvent& WXUNUSED(event))
{
    Fibers* l_fibers = NULL; // Initalize it quiet compiler.
    if( ! m_datasetHelper->getFiberDataset( l_fibers ) )
        return;
    l_fibers->resetColorArray();
    m_datasetHelper->m_selBoxChanged = true;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnToggleColorMapLegend( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_showColorMapLegend = !m_datasetHelper->m_showColorMapLegend;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSetCMap0( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_colorMap = 0;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSetCMap1( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_colorMap = 1;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSetCMap2( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_colorMap = 2;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSetCMap3( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_colorMap = 3;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSetCMap4( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_colorMap = 4;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSetCMap5( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_colorMap = 5;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSetCMapNo( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_colorMap = -1;
    refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 * Menu Help
 *
 ****************************************************************************************************/
///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnAbout( wxCommandEvent& WXUNUSED(event) )
{
    wxString rev = _T( "$Revision: 760 $" );
    rev = rev.AfterFirst('$');
    rev = rev.BeforeLast('$');
    wxString date = _T( "$Date: 2009-09-01 04:58:51 -0400 (mar., 01 sept. 2009) $" );
    date = date.AfterFirst( '$' );
    date = date.BeforeLast( '$' );
    (void)wxMessageBox( _T("Fiber Navigator\nAuthor: Ralph Schurade (c) 2008\n\n" )
                        + rev + _T( "\n" ) + date, _T( "About Fiber Navigator" ) );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnShortcuts( wxCommandEvent& WXUNUSED(event) )
{
    wxString nl = _T( "\n" );
    (void)wxMessageBox(
            _T( "Keyboard Shortcuts" ) + nl
                    + _T( "_________________________________________________________")
                    + nl + _T( "Move selected box:" ) + nl
                    + _T( "   cursor up/down/left/right, page up/down" ) + nl
                    + _T( "Move selected object larger steps:" ) + nl
                    + _T( "   shift + cursor up/down/left/right, page up/down" )
                    + nl + _T( "Resize selected box:" ) + nl
                    + _T( "   ctrl + cursor up/down/left/right, page up/down" )
                    + nl + _T( "Resize selected object larger steps:" ) + nl
                    + _T( "   ctrl + shift + cursor up/down/left/right, page up/down" )
                    + nl + _T( "Delete selected object and all sub objects:" ) + nl
                    + _T( "   del" ) + nl );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnScreenshot( wxCommandEvent& WXUNUSED(event) )
{
    wxString l_caption         = wxT( "Choose a file" );
    wxString l_wildcard        = wxT( "PPM files (*.ppm)|*.ppm|*.*|*.*" );
    wxString l_defaultDir      = wxEmptyString;
    wxString l_defaultFilename = wxEmptyString;
    wxFileDialog dialog( this, l_caption, l_defaultDir, l_defaultFilename, l_wildcard, wxSAVE );
    dialog.SetFilterIndex( 0 );
    dialog.SetDirectory( m_datasetHelper->m_screenshotPath );
    if( dialog.ShowModal() == wxID_OK )
    {
        m_datasetHelper->m_screenshotPath = dialog.GetDirectory();
        m_datasetHelper->m_screenshotName = dialog.GetPath();
        if( m_datasetHelper->m_screenshotName.AfterLast('.') != _T( "ppm" ) )
            m_datasetHelper->m_screenshotName += _T( ".ppm" );
        m_datasetHelper->m_scheduledScreenshot = true;
        m_mainGL->render();
        m_mainGL->render();
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
//
// i_fileName       :
///////////////////////////////////////////////////////////////////////////
void MainFrame::Screenshot( wxString i_fileName )
{
    m_datasetHelper->m_screenshotPath = _( "" );
    m_datasetHelper->m_screenshotName = i_fileName;
    if ( m_datasetHelper->m_screenshotName.AfterLast( '.' ) != _T( "ppm" ) )
        m_datasetHelper->m_screenshotName += _T( ".ppm" );
    m_datasetHelper->m_scheduledScreenshot = true;
    m_mainGL->render();
    m_mainGL->render();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSlizeMovieSag( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->doLicMovie( 0 );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSlizeMovieCor( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->doLicMovie(1);
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSlizeMovieAxi( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->doLicMovie( 2 );
}

///////////////////////////////////////////////////////////////////////////
// This function will do the proper actions when a slider moved.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->updateView( m_xSlider->GetValue(), m_ySlider->GetValue(), m_zSlider->GetValue() );
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the min hue value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphMinHueSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
   updateGlyphColoration( MIN_HUE, m_glyphMinHueValue->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the max hue value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphMaxHueSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
   updateGlyphColoration( MAX_HUE, m_glyphMaxHueValue->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the saturation value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphSaturationSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
   updateGlyphColoration( SATURATION, m_glyphSaturationValue->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the luminance value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphLuminanceSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
   updateGlyphColoration( LUMINANCE, m_glyphLuminanceValue->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will set the value of a glyph color modifier by the value
// on its corresponding slider.
//
// i_modifier       : The modifier indicating what GlyphColorModifier needs to be updated.
// i_value          : The value of the modifier to set.
///////////////////////////////////////////////////////////////////////////
void MainFrame::updateGlyphColoration( GlyphColorModifier i_modifier, float i_value )
{
    // We get the currently selected item.
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
            
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    
    if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
        ( (Glyph*)l_info )->setColor( i_modifier, i_value );
}

///////////////////////////////////////////////////////////////////////////
// This function will set the LOD of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphLODSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    // We get the currently selected item.
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    
    if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
        ( (Glyph*)l_info )->setLOD( (LODChoices)m_glyphLODValue->GetValue() );
}

///////////////////////////////////////////////////////////////////////////
// This function will set the light attenuation of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphLightAttenuationSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    // We get the currently selected item.
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    
    if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
        ( (Glyph*)l_info )->setLighAttenuation( m_glyphLightAttenuation->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the light x position slider moved.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphLightXDirectionSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    glyphLightPositionChanged( X_AXIS, m_glyphLightXPosition->GetValue() / 100.0f  );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the light y position slider moved.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphLightYDirectionSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    glyphLightPositionChanged( Y_AXIS, m_glyphLightYPosition->GetValue() / 100.0f  );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the light z position slider moved.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphLightZDirectionSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    glyphLightPositionChanged( Z_AXIS, m_glyphLightZPosition->GetValue() / 100.0f  );
}

///////////////////////////////////////////////////////////////////////////
// This function will set the light position for the proper axis.
//
// i_axisType       : The axis that we want to set the lght position for.
// i_position       : The value of the position.
///////////////////////////////////////////////////////////////////////////
void MainFrame::glyphLightPositionChanged( AxisType i_axisType, float i_position )
{
    // We get the currently selected item.
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    
    if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
        ( (Glyph*)l_info )->setLightPosition( i_axisType, i_position );
}

///////////////////////////////////////////////////////////////////////////
// This function will set the display value of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphDisplaySliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    // We get the currently selected item.
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    
    if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
        ( (Glyph*)l_info )->setDisplayFactor( m_glyphDisplayValue->GetValue() );
}

///////////////////////////////////////////////////////////////////////////
// This function will set the scaling factor of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphScalingFactorSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    // We get the currently selected item.
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    
    if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
        ( (Glyph*)l_info )->setScalingFactor( m_glyphScalingFactor->GetValue() );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the x flip check box in the 
// glyph options is checked/unchecked.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphXAxisFlipChecked( wxCommandEvent& event )
{
    OnGlyphFlip( X_AXIS, event.IsChecked() );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the y flip check box in the 
// glyph options is checked/unchecked.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphYAxisFlipChecked( wxCommandEvent& event )
{
    OnGlyphFlip( Y_AXIS, event.IsChecked() );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the z flip check box in the 
// glyph options is checked/unchecked.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphZAxisFlipChecked( wxCommandEvent& event )
{
    OnGlyphFlip( Z_AXIS, event.IsChecked() );
}

///////////////////////////////////////////////////////////////////////////
// This function will simply find the currently displayed glyph and call 
// the flipAxis function with the proper parameter.
//
// i_axisType               : Determines on what axis we want to do the flip.
// i_isChecked              : Determines if the item is checked or not.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphFlip( AxisType i_axisType, bool i_isChecked )
{
    // We get the currently selected item.
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    
    if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
        ( (Glyph*)l_info )->flipAxis( i_axisType, i_isChecked );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the map on sphere radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphMapOnSphereSelected( wxCommandEvent& event )
{
    // We get the currently selected item.
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    
    if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
        ( (Glyph*)l_info )->setDisplayShape( SPHERE );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the normal display radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphNormalSelected( wxCommandEvent& event )
{
    // We get the currently selected item.
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    
    if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
        ( (Glyph*)l_info )->setDisplayShape( NORMAL );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the axes display radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphAxesSelected( wxCommandEvent& event )
{
    // We get the currently selected item.
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    
    if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
        ( (Glyph*)l_info )->setDisplayShape( AXES );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the main axis display radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphMainAxisSelected( wxCommandEvent& event )
{
    // We get the currently selected item.
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    
    if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
        ( (Glyph*)l_info )->setDisplayShape( AXIS );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the color with position check box in the 
// glyph options is checked/unchecked.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphColorWithPosition( wxCommandEvent& event )
{
    // We get the currently selected item.
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    
    if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
        ( (Glyph*)l_info )->setColorWithPosition( event.IsChecked() );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnTSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    float l_threshold = (float)m_tSlider->GetValue() / 100.0f;

    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;

    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    if( l_info->getUseTex() )
        m_listCtrl->SetItem( l_item, 2, wxString::Format( wxT( "%.2f" ),   l_threshold * l_info->getOldMax() ) );
    else
        m_listCtrl->SetItem( l_item, 2, wxString::Format( wxT( "(%.2f)" ), l_threshold * l_info->getOldMax() ) );

    l_info->setThreshold( l_threshold );
    if( l_info->getType() == SURFACE )
    {
        Surface* s = (Surface*)m_listCtrl->GetItemData( l_item );
        s->movePoints();
    }
    if( l_info->getType() == ISO_SURFACE && ! m_tSlider->leftDown() )
    {
        CIsoSurface* s = (CIsoSurface*)m_listCtrl->GetItemData( l_item );
        s->GenerateWithThreshold();
    }
    if( l_info->getType() < RGB )
    {
        Anatomy* a = (Anatomy*)m_listCtrl->GetItemData( l_item );
        if( a->m_roi )
            a->m_roi->setThreshold( l_threshold );
    }

    // This slider will set the Brightness level. Currently only the glyphs uses this value.
    l_info->setBrightness( 1.0f - l_threshold );

    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnTSlider2Moved( wxCommandEvent& WXUNUSED(event) )
{
    float l_alpha = (float)m_tSlider2->GetValue() / 100.0f;

    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;

    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    l_info->setAlpha( l_alpha );

    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnButtonAxial( wxCommandEvent& WXUNUSED(event) )
{
    if (m_datasetHelper->m_theScene)
    {
        m_datasetHelper->m_showAxial = ! m_datasetHelper->m_showAxial;
        //m_mainGL->render();
		m_mainGL->Refresh();
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnButtonCoronal( wxCommandEvent& WXUNUSED(event) )
{
    if( m_datasetHelper->m_theScene )
    {
        m_datasetHelper->m_showCoronal = ! m_datasetHelper->m_showCoronal;
        //m_mainGL->render();
		m_mainGL->Refresh();
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnButtonSagittal( wxCommandEvent& WXUNUSED(event) )
{
    if( m_datasetHelper->m_theScene )
    {
        m_datasetHelper->m_showSagittal = ! m_datasetHelper->m_showSagittal;
        //m_mainGL->render();
		m_mainGL->Refresh();
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnToggleAlpha( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_blendAlpha = ! m_datasetHelper->m_blendAlpha;

    updateMenus();
    this->Update();
    //this->Refresh();

    //m_mainGL->render();
	m_mainGL->Refresh();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnToggleLayout( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene )
        return;

    m_enlargeNav = ( m_enlargeNav + 1 ) % 3;
    wxSize clientSize = GetClientSize();
    wxSize windowSize = GetSize();

    switch( m_enlargeNav )
    {
        case 1:
        {
            m_treeSizer->Show  ( false );
            m_buttonSizer->Show( false );
            m_listCtrl->Show   ( false );

            int newSize = ( clientSize.y - 65 ) / 3;

            m_gl0->SetMinSize( wxSize( 100, 100 ) );
            m_gl1->SetMinSize( wxSize( 100, 100 ) );
            m_gl2->SetMinSize( wxSize( 100, 100 ) );
            m_gl0->SetMaxSize( wxSize( newSize, newSize ) );
            m_gl1->SetMaxSize( wxSize( newSize, newSize ) );
            m_gl2->SetMaxSize( wxSize( newSize, newSize ) );

            m_xSlider->SetMinSize( wxSize( newSize, -1 ) );
            m_ySlider->SetMinSize( wxSize( newSize, -1 ) );
            m_zSlider->SetMinSize( wxSize( newSize, -1 ) );

            m_navSizer1 = new wxBoxSizer( wxVERTICAL );
            m_navSizer1->Add( m_gl0,     1, wxALL | wxEXPAND | wxSHAPED, 1 );
            m_navSizer1->Add( m_zSlider, 0, wxALL,                       1 );
            m_navSizer1->Add( m_gl1,     1, wxALL | wxEXPAND | wxSHAPED, 1 );
            m_navSizer1->Add( m_ySlider, 0, wxALL,                       1 );
            m_navSizer1->Add( m_gl2,     1, wxALL | wxEXPAND | wxSHAPED, 1 );
            m_navSizer1->Add( m_xSlider, 0, wxALL,                       1 );

            m_mainSizer1 = new wxBoxSizer( wxHORIZONTAL );
            m_mainSizer1->Add( m_navSizer1, 0, wxALL | wxEXPAND, 1 );
            m_mainSizer1->Add( m_mainGL,    1, wxEXPAND | wxALL, 2 );

            SetSizer(m_mainSizer1);
            m_mainSizer1->SetSizeHints(this);
            break;
        }
        case 2:
        {
            m_treeWidget->Show            ( true  );
            m_listCtrl->Show              ( true  );
            m_xSlider->Show               ( false );
            m_ySlider->Show               ( false );
            m_zSlider->Show               ( false );
            m_tSlider->Show               ( true  );
            m_tSlider2->Show              ( true  );
            m_buttonUp->Show              ( true  );
            m_buttonDown->Show            ( true  );
            m_buttonLoadDatasets->Show    ( true  );
            m_buttonLoadMeshes->Show      ( true  );
            m_buttonLoadFibers->Show      ( true  );
            m_buttonLoadTensors->Show     ( true  );
            m_buttonLoadODFs->Show        ( true  );
            // This button is only to perform some test, so we hide it if we do not need it.
            m_buttonLoadTestFibers->Show  ( false );

            m_gl0->SetMinSize   ( wxSize( 100,   100 ) );
            m_gl1->SetMinSize   ( wxSize( 100,   100 ) );
            m_gl2->SetMinSize   ( wxSize( 100,   100 ) );
            m_mainGL->SetMinSize( wxSize( 100,   100 ) );
            m_gl0->SetMaxSize   ( wxSize( 10000, 10000 ) );
            m_gl1->SetMaxSize   ( wxSize( 10000, 10000 ) );
            m_gl2->SetMaxSize   ( wxSize( 10000, 10000 ) );
            m_mainGL->SetMaxSize( wxSize( 10000, 10000 ) );

            m_mainSizer     = new wxBoxSizer( wxHORIZONTAL );
            m_leftMainSizer = new wxBoxSizer( wxVERTICAL   );
            m_buttonSizer   = new wxBoxSizer( wxHORIZONTAL );
            m_treeSizer     = new wxBoxSizer( wxVERTICAL   );

            m_treeWidget->SetMinSize( wxSize( 150, 350 ) );
            m_treeSizer->Add( m_buttonLoadDatasets,   0, wxALL | wxALIGN_CENTER, 0 );
            m_treeSizer->Add( m_buttonLoadMeshes,     0, wxALL | wxALIGN_CENTER, 0 );
            m_treeSizer->Add( m_buttonLoadFibers,     0, wxALL | wxALIGN_CENTER, 0 );
            m_treeSizer->Add( m_buttonLoadTensors,    0, wxALL | wxALIGN_CENTER, 0 );
            m_treeSizer->Add( m_buttonLoadODFs,       0, wxALL | wxALIGN_CENTER, 0 );
            m_treeSizer->Add( m_buttonLoadTestFibers, 0, wxALL | wxALIGN_CENTER, 0 );
            m_treeSizer->Add( m_treeWidget,           1, wxALL,                  1 );

            m_buttonSizer->Add( m_buttonUp,   0, wxALL, 1 );
            m_buttonSizer->Add( m_buttonDown, 0, wxALL, 1 );
            m_buttonSizer->Add( m_tSlider,    0, wxALL, 1 );
            m_buttonSizer->Add( m_tSlider2,   0, wxALL, 1 );

            m_leftMainSizer->Add( m_treeSizer,   0, wxALL,                  1 );
            m_leftMainSizer->Add( m_listCtrl,    1, wxALL | wxEXPAND,       1 );
            m_leftMainSizer->Add( m_buttonSizer, 0, wxALIGN_BOTTOM | wxALL, 1 );

            m_mainSizer2 = new wxFlexGridSizer( 2, 2, 1, 1 );
            m_mainSizer2->Add( m_gl0,    1, wxALL | wxEXPAND | wxSHAPED, 1 );
            m_mainSizer2->Add( m_gl1,    1, wxALL | wxEXPAND | wxSHAPED, 1 );
            m_mainSizer2->Add( m_gl2,    1, wxALL | wxEXPAND | wxSHAPED, 1 );
            m_mainSizer2->Add( m_mainGL, 1, wxALL | wxEXPAND | wxSHAPED, 1 );

            m_mainSizer->Add( m_leftMainSizer, 0, wxALL | wxEXPAND, 1 );
            m_mainSizer->Add( m_mainSizer2,    1, wxALL | wxEXPAND, 1 );

            SetSizer( m_mainSizer );
            m_mainSizer->SetSizeHints( this );
            break;
        }
        default:
        {
            m_treeWidget->Show          ( true  );
            m_listCtrl->Show            ( true  );
            m_xSlider->Show             ( true  );
            m_ySlider->Show             ( true  );
            m_zSlider->Show             ( true  );
            m_tSlider->Show             ( true  );
            m_tSlider2->Show            ( true  );
            m_buttonUp->Show            ( true  );
            m_buttonDown->Show          ( true  );
            m_buttonLoadDatasets->Show  ( true  );
            m_buttonLoadMeshes->Show    ( true  );
            m_buttonLoadFibers->Show    ( true  );
            m_buttonLoadTensors->Show   ( true  );
            m_buttonLoadODFs->Show      ( true  );
            // This button is only to perform some test, so we hide it if we do not need it.
            m_buttonLoadTestFibers->Show( false );
            m_gl0->SetMinSize         ( wxSize( 150,   150   ) );
            m_gl1->SetMinSize         ( wxSize( 150,   150   ) );
            m_gl2->SetMinSize         ( wxSize( 150,   150   ) );
            m_mainGL->SetMinSize      ( wxSize( 400,   400   ) );
            m_gl0->SetMaxSize         ( wxSize( 150,   150   ) );
            m_gl1->SetMaxSize         ( wxSize( 150,   150   ) );
            m_gl2->SetMaxSize         ( wxSize( 150,   150   ) );
            m_mainGL->SetMaxSize      ( wxSize( 10000, 10000 ) );

            m_xSlider->SetMinSize( wxSize( 150, -1 ) );
            m_ySlider->SetMinSize( wxSize( 150, -1 ) );
            m_zSlider->SetMinSize( wxSize( 150, -1 ) );

            m_mainSizer     = new wxBoxSizer( wxHORIZONTAL );
            m_leftMainSizer = new wxBoxSizer( wxVERTICAL   );
            m_leftSizer     = new wxBoxSizer( wxHORIZONTAL );
            m_navSizer      = new wxBoxSizer( wxVERTICAL   );
            m_buttonSizer   = new wxBoxSizer( wxHORIZONTAL );
            m_treeSizer     = new wxBoxSizer( wxVERTICAL   );

            m_navSizer->Add( m_gl0,     1, wxALL | wxEXPAND | wxSHAPED, 1 );
            m_navSizer->Add( m_zSlider, 0, wxALL,                       1 );
            m_navSizer->Add( m_gl1,     1, wxALL | wxEXPAND | wxSHAPED, 1 );
            m_navSizer->Add( m_ySlider, 0, wxALL,                       1 );
            m_navSizer->Add( m_gl2,     1, wxALL | wxEXPAND | wxSHAPED, 1 );
            m_navSizer->Add( m_xSlider, 0, wxALL,                       1 );

            m_treeSizer->Add( m_buttonLoadDatasets,   0, wxALL | wxALIGN_CENTER, 0 );
            m_treeSizer->Add( m_buttonLoadMeshes,     0, wxALL | wxALIGN_CENTER, 0 );
            m_treeSizer->Add( m_buttonLoadFibers,     0, wxALL | wxALIGN_CENTER, 0 );
            m_treeSizer->Add( m_buttonLoadTensors,    0, wxALL | wxALIGN_CENTER, 0 );
            m_treeSizer->Add( m_buttonLoadODFs,       0, wxALL | wxALIGN_CENTER, 0 );
            m_treeSizer->Add( m_buttonLoadTestFibers, 0, wxALL | wxALIGN_CENTER, 0 );
            m_treeSizer->Add( m_treeWidget,           1, wxALL,                  1 );

            m_leftSizer->Add( m_treeSizer, 0, wxALL | wxEXPAND, 0 );
            m_leftSizer->Add( m_navSizer,  1, wxALL | wxEXPAND, 0 );

            m_buttonSizer->Add( m_buttonUp,   0, wxALL, 1 );
            m_buttonSizer->Add( m_buttonDown, 0, wxALL, 1 );
            m_buttonSizer->Add( m_tSlider,    0, wxALL, 1 );
            m_buttonSizer->Add( m_tSlider2,   0, wxALL, 1 );

            m_leftMainSizer->Add( m_leftSizer,   0, wxALL,                  1 );
            m_leftMainSizer->Add( m_listCtrl,    1, wxALL | wxEXPAND,       1 );
            m_leftMainSizer->Add( m_buttonSizer, 0, wxALIGN_BOTTOM | wxALL, 1 );

            m_mainSizer->Add( m_leftMainSizer, 0, wxEXPAND | wxALL, 0 );
            m_mainSizer->Add( m_mainGL,        1, wxEXPAND | wxALL, 2 );

            SetSizer( m_mainSizer );
            m_mainSizer->SetSizeHints( this );
            break;
        }
    }
    GetSizer()->SetDimension( 0, 0, clientSize.x, clientSize.y );
    m_listCtrl->SetColumnWidth( 1, m_listCtrl->GetSize().x - 110 );
    SetSize( windowSize );

    m_mainGL->changeOrthoSize();
    updateMenus();
    this->Update();
    this->Refresh();

    //m_mainGL->render();
	m_mainGL->Refresh();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::refreshAllGLWidgets()
{
    updateStatusBar();
    updateMenus();

//#if defined(__WXMAC__)
    if ( m_gl0 ) 
        m_gl0->Refresh();
    if ( m_gl1 ) 
        m_gl1->Refresh();
    if ( m_gl2 ) 
        m_gl2->Refresh();
    if ( m_mainGL ) 
        m_mainGL->Refresh();
//#else
//    if ( m_gl0 )
//        m_gl0->render();
//    if ( m_gl1 )
//        m_gl1->render();
//    if ( m_gl2 )
//        m_gl2->render();
//    if ( m_mainGL )
//        m_mainGL->render();
//#endif
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::renewAllGLWidgets()
{
    if( m_mainGL )
        m_mainGL->invalidate();
    if( m_gl0 )
        m_gl0->invalidate();
    if( m_gl1 )
        m_gl1->invalidate();
    if( m_gl2 )
        m_gl2->invalidate();

    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::updateStatusBar()
{
    wxString sbString0 = wxT( "" );
    sbString0 = wxString::Format( wxT("Position: %d  %d  %d" ),
                                  m_xSlider->GetValue(),
                                  m_ySlider->GetValue(),
                                  m_zSlider->GetValue() );
    GetStatusBar()->SetStatusText( sbString0, 0 );
}

/****************************************************************************************************
 *
 * Functions for list widget event handling
 *
 ****************************************************************************************************/
/****************************************************************************************************
 *
 * OnActivateListItem gets called when a list item gets
 * double clicked,
 *
 ****************************************************************************************************/

///////////////////////////////////////////////////////////////////////////
// COMMENT
//
// event        :
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnActivateListItem( wxListEvent& event )
{
    int l_item = event.GetIndex();
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    int l_col = m_listCtrl->getColActivated();
    switch( l_col )
    {
        case 11:
            if( ! l_info->toggleShowFS())
                m_listCtrl->SetItem( l_item, 1, l_info->getName().BeforeFirst( '.' ) + wxT( "*" ) );
            else
                m_listCtrl->SetItem( l_item, 1, l_info->getName().BeforeFirst( '.' ) );
            break;
        case 13:
            delete l_info;
            m_listCtrl->DeleteItem( l_item );
            m_datasetHelper->updateLoadStatus();
            break;
        default:
            break;
    }

    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
//
// event        :
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnRightClickListItem( wxListEvent& event )
{
    wxMenu* l_menu = new wxMenu;

    long l_item = event.GetIndex();
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );

    if( l_info->getType() < MESH )
    {
        l_menu->Append( MENU_LIST_CUTOUT,           _T( "Cut Area" ) );
        l_menu->Append( MENU_FILE_SAVE_DATASET,     _T( "Save" )     );
        l_menu->Append( MENU_FILE_MINIMIZE_DATASET, _T( "Minimize" ) );
        l_menu->Append( MENU_FILE_DILATE_DATASET,   _T( "Dilate" )   );
        l_menu->Append( MENU_FILE_ERODE_DATASET,    _T( "Erode" )    );
        l_menu->AppendSeparator();
        if( l_info->getShowFS() )
            l_menu->Append( MENU_LIST_TOGGLE_NAME, _T( "No Interpolation" ) );
        else
            l_menu->Append( MENU_LIST_TOGGLE_NAME, _T( "Interpolation" )    );
    }
    else if( l_info->getType() == MESH || l_info->getType() == ISO_SURFACE )
    {
        if( l_info->getShowFS() )
            l_menu->Append( MENU_LIST_TOGGLE_NAME,  _T( "Cut Front Sector" )  );
        else
            l_menu->Append( MENU_LIST_TOGGLE_NAME,  _T( "Show Front Sector" ) );
        if( l_info->getUseTex() )
            l_menu->Append( MENU_LIST_TOGGLE_COLOR, _T( "Use Coloring" )      );
        else
            l_menu->Append( MENU_LIST_TOGGLE_COLOR, _T( "Use Textures" )      );
    }
    else if( l_info->getType() == FIBERS )
    {
        if( l_info->getShowFS( ))
            l_menu->Append( MENU_LIST_TOGGLE_NAME,  _T( "Local Coloring" )     );
        else
            l_menu->Append( MENU_LIST_TOGGLE_NAME,  _T( "Global Coloring" )    );
        if( l_info->getUseTex() )
            l_menu->Append( MENU_LIST_TOGGLE_COLOR, _T( "Color With Overlay" ) );
        else
            l_menu->Append( MENU_LIST_TOGGLE_COLOR, _T( "Normal Coloring" )    );
    }
    else if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
    {
        if( m_datasetHelper->m_displayGlyphOptions )
            l_menu->Append( MENU_LIST_HIDE_OPTIONS, _T( "Hide Options" ) );
        else
            l_menu->Append( MENU_LIST_SHOW_OPTIONS, _T( "Show Options" ) );
    }

    l_menu->AppendSeparator();
    if( l_info->getShow() )
        l_menu->Append( MENU_LIST_TOGGLE_SHOW, _T( "Deactivate" ) );
    else
        l_menu->Append( MENU_LIST_TOGGLE_SHOW, _T( "Activate" )   );

    l_menu->Append(     MENU_LIST_DELETE,      _T( "Delete" )     );

#ifdef __WXMSW__
    int yAdjust = 40;
#else
    int yAdjust = 80;
#endif

    int mx = wxGetMousePosition().x - this->GetScreenPosition().x;
    int my = wxGetMousePosition().y - this->GetScreenPosition().y - yAdjust;

    PopupMenu( l_menu, wxPoint( mx, my ) );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
//
// event        :
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSelectListItem( wxListEvent& event )
{
    int l_item = event.GetIndex();
    if( m_datasetHelper->m_guiBlocked || l_item == -1 )
    {
        DisplayGlyphOptions( false, NULL );
        return;
    }

    DatasetInfo *l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item) ;
    updateMenus();

    m_tSlider->SetValue ( (int)( l_info->getThreshold() * 100 ) );
    m_tSlider2->SetValue( (int)( l_info->getAlpha() * 100 ) );

    int l_col = 0;
    l_col = m_listCtrl->getColClicked();
    switch( l_col )
    {
        case 10:
            if (l_info->toggleShow())
                m_listCtrl->SetItem( l_item, 0, wxT( "" ), 0 );
            else
                m_listCtrl->SetItem( l_item, 0, wxT( "" ), 1 );

            refreshAllGLWidgets();
            break;
        case 12:
            if( l_info->getType() >= MESH )
            {
                if( ! l_info->toggleUseTex() )
                    m_listCtrl->SetItem( l_item, 
                                         2,
                                         wxT( "(" ) + wxString::Format( wxT( "%.2f" ),
                                         l_info->getThreshold() * l_info->getOldMax() ) + wxT( ")" ) );
                else
                    m_listCtrl->SetItem( l_item,
                                         2,
                                         wxString::Format( wxT( "%.2f" ),
                                         l_info->getThreshold() * l_info->getOldMax() ) );
            }
            break;

        default:
            break;
    }

    if( l_info->getType() == TENSORS || l_info->getType() == ODFS  )
        DisplayGlyphOptions(  m_datasetHelper->m_displayGlyphOptions, l_info );
    else
        DisplayGlyphOptions( false, NULL );

    m_mainGL->changeOrthoSize();
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnListItemUp(wxCommandEvent& WXUNUSED(event))
{
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    m_listCtrl->moveItemUp( l_item );
    m_listCtrl->EnsureVisible( l_item );
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnListItemDown( wxCommandEvent& WXUNUSED(event) )
{
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    m_listCtrl->moveItemDown( l_item );
    m_listCtrl->EnsureVisible( l_item );
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnListMenuName( wxCommandEvent&  WXUNUSED(event) )
{
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    DatasetInfo *l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    if( ! l_info->toggleShowFS() )
        m_listCtrl->SetItem( l_item, 1, l_info->getName().BeforeFirst( '.' ) + wxT( "*" ) );
    else
        m_listCtrl->SetItem( l_item, 1, l_info->getName().BeforeFirst( '.' ) );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnListMenuThreshold( wxCommandEvent&  WXUNUSED(event) )
{
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    if( l_info->getType() >= MESH )
    {
        if( ! l_info->toggleUseTex() )
            m_listCtrl->SetItem( l_item,
                                 2,
                                 wxT( "(" ) + wxString::Format( wxT( "%.2f" ),
                                 l_info->getThreshold() * l_info->getOldMax()) + wxT( ")" ) );
        else
            m_listCtrl->SetItem( l_item,
                                 2,
                                 wxString::Format( wxT( "%.2f" ),
                                 l_info->getThreshold() * l_info->getOldMax() ) );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the delete button after a right 
// click in the item list is clicked.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnListMenuDelete( wxCommandEvent&  WXUNUSED(event) )
{
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;

    m_listCtrl->DeleteItem( l_item );
    // Anatomy deleted? check if another one is still present.
    m_datasetHelper->updateLoadStatus();
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the show button after a right 
// click in the item list is clicked.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnListMenuShow( wxCommandEvent&  WXUNUSED(event) )
{
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;

    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    if( l_info->toggleShow() )
        m_listCtrl->SetItem( l_item, 0, wxT( "" ), 0 );
    else
        m_listCtrl->SetItem( l_item, 0, wxT( "" ), 1 );

    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the show options 
// info after a right click on a glyph type item in the item list.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnListMenuShowOptions( wxCommandEvent&  WXUNUSED(event) )
{
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;

    DisplayGlyphOptions( true, (DatasetInfo*)m_listCtrl->GetItemData( l_item ) );
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the hide options 
// info after a right click on a glyph type item in the item list.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnListMenuHideOptions( wxCommandEvent&  WXUNUSED(event) )
{
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;

    DisplayGlyphOptions( false, (DatasetInfo*)m_listCtrl->GetItemData( l_item ) );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the cut out button after a right 
// click in the item list is clicked.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnListMenuCutOut( wxCommandEvent&  WXUNUSED(event) )
{
    m_datasetHelper->createCutDataset();

    refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 * Functions for tree widget event handling
 *
 ****************************************************************************************************/

///////////////////////////////////////////////////////////////////////////
// This function will be called when the delete tree item event is triggered.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnDeleteTreeItem( wxTreeEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_lastSelectedObject = NULL;
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the selection changed event in the item tree is triggered.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSelectTreeItem( wxTreeEvent& WXUNUSED(event) )
{
    wxTreeItemId l_treeId = m_treeWidget->GetSelection();
    int l_selected = treeSelected( l_treeId );

    SelectionObject* l_selectionObject = NULL;

    switch( l_selected )
    {
        case MASTER_OBJECT:
        case CHILD_OBJECT:
            if( m_datasetHelper->m_lastSelectedObject )
                m_datasetHelper->m_lastSelectedObject->unselect();

            l_selectionObject = (SelectionObject*)( m_treeWidget->GetItemData( l_treeId ) );
            m_datasetHelper->m_lastSelectedObject = l_selectionObject;
            m_datasetHelper->m_lastSelectedObject->select( false );
            break;

        case POINT_DATASET:
            if( m_datasetHelper->m_lastSelectedPoint )
                m_datasetHelper->m_lastSelectedPoint->unselect();

            m_datasetHelper->m_lastSelectedPoint = (SplinePoint*)( m_treeWidget->GetItemData( l_treeId ) );
            m_datasetHelper->m_lastSelectedPoint->select( false );
            break;

        default: break;
    }

    // Update the information in the fibers info grid.
    SetFiberInfoGridValues( l_selectionObject );

    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will create and display a menu when someone right click
// on a tree item. For now there is only 2 different types of items in this
// tree that will generate a menu -> MASTER_OBJECT and CHILD_OBJECT.
//
// event            : The event info;
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnRightClickTreeItem( wxTreeEvent& event )
{
	m_treeWidget->SelectItem( event.GetItem() );
	wxTreeItemId  l_treeId = m_treeWidget->GetSelection();
	SelectionObject* l_selectionObject =( (SelectionObject*)( m_treeWidget->GetItemData( l_treeId ) ) );

	wxMenu* l_menu = new wxMenu;

	switch( treeSelected( l_treeId ) )
	{
		case MASTER_OBJECT:
			FillMasterBoxMenu( l_selectionObject, l_menu );
			break;

		case CHILD_OBJECT:
			FillChildBoxMenu( l_selectionObject, l_menu );
			break;

		default: return;
	}

#ifdef __WXMSW__
	int yAdjust = 40;
#else
	int yAdjust = 80;
#endif

	int l_mx = wxGetMousePosition().x - this->GetScreenPosition().x;
	int l_my = wxGetMousePosition().y - this->GetScreenPosition().y - yAdjust;

	PopupMenu( l_menu, wxPoint( l_mx, l_my ) );
}

///////////////////////////////////////////////////////////////////////////
// This function will add in the menu what needs to be shown when 
// someone right click on a MASTER_OBJECT item in the tree.
//
// i_selectionObject        : The selection object that the user clicked on.
// o_menu                   : The menu that will be filled.
///////////////////////////////////////////////////////////////////////////
void MainFrame::FillMasterBoxMenu( SelectionObject* i_selectionObject, wxMenu* o_menu )
{	
	DefaultFillMenu( i_selectionObject, o_menu );
	
    o_menu->AppendSeparator();

    o_menu->Append( MENU_VOI_CREATE_TEXTURE, _T( "Create Color Texture" ) );

	o_menu->AppendSeparator();

	if( ! i_selectionObject->isSelectionObject())
	{
		o_menu->Append( MENU_VOI_COLOR_ROI, _T( "Set Color" ) );
		o_menu->AppendSeparator();
	}

	o_menu->Append( MENU_VOI_RENAME_BOX,  _T( "Rename" ) );
	o_menu->Append( TREE_CTRL_DELETE_BOX, _T( "Delete" ) );
}

//////////////////////////////////////////////////////////////////////////
// This function will add in the menu what needs to be shown when 
// someone right click on a CHILD_OBJECT item in the tree.
//
// i_selectionObject        : The selection object that the user clicked on.
// o_menu                   : The menu that will be filled.
///////////////////////////////////////////////////////////////////////////
void MainFrame::FillChildBoxMenu( SelectionObject* i_selectionObject, wxMenu* o_menu )
{
	o_menu->Append( MENU_VOI_TOGGLE_ANDNOT, _T( "Toggle AND/NOT" ) );
	o_menu->AppendSeparator();

	DefaultFillMenu( i_selectionObject, o_menu );

	o_menu->AppendSeparator();

	if( ! i_selectionObject->isSelectionObject())
	{
		o_menu->Append( MENU_VOI_COLOR_ROI, _T( "Set Color" ) );
		o_menu->AppendSeparator();
	}

	o_menu->Append( MENU_VOI_RENAME_BOX,  _T( "Rename" ) );
	o_menu->Append( TREE_CTRL_DELETE_BOX, _T( "Delete" ) );
}

//////////////////////////////////////////////////////////////////////////
// Since the MASTER_OBJECT and the CHILD_OBJECT shared those menu item, they 
// were put in a separe function to avoid code duplication.
//
// i_selectionObject        : The selection object that the user clicked on. 
// o_menu                   : The menu that will be filled.
///////////////////////////////////////////////////////////////////////////
void MainFrame::DefaultFillMenu( SelectionObject* i_selectionObject, wxMenu* o_menu )
{
	wxMenuItem* l_menuItem = o_menu->AppendCheckItem( MENU_VOI_TOGGLE_SELECTION_OBJECTS, _T( "Activate" ), wxEmptyString );	
	l_menuItem->Check( i_selectionObject->getIsActive() );

	l_menuItem = o_menu->AppendCheckItem( MENU_VOI_TOGGLE_VISIBLE_SELECTION_OBJECT, _T( "Visible" ), wxEmptyString );
	l_menuItem->Check( i_selectionObject->getIsVisible() );

	o_menu->AppendSeparator();

	l_menuItem = o_menu->AppendCheckItem( MENU_VOI_DISPLAY_FIBERS_INFO, _T( "Display Fibers Info" ), wxEmptyString );
	l_menuItem->Check( m_datasetHelper->m_displayFibersInfo );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnActivateTreeItem( wxTreeEvent& WXUNUSED(event) )
{
    wxTreeItemId l_treeId = m_treeWidget->GetSelection();

    int l_selected = treeSelected( l_treeId );
    if( l_selected == MASTER_OBJECT )
    {
        SelectionObject* l_selectionObject = (SelectionObject*)( m_treeWidget->GetItemData( l_treeId ) );
        l_selectionObject->toggleIsActive();
        m_treeWidget->SetItemImage(l_treeId, l_selectionObject->getIcon());
        l_selectionObject->setIsDirty(true);

        int l_childSelectionObjects = m_treeWidget->GetChildrenCount( l_treeId );
        wxTreeItemIdValue l_childCookie = 0;

        for( int i = 0; i < l_childSelectionObjects; ++i )
        {
            wxTreeItemId l_childId = m_treeWidget->GetNextChild( l_treeId, l_childCookie );
            if( l_childId.IsOk() )
            {
                SelectionObject* l_childSelectionBox = ( (SelectionObject*)( m_treeWidget->GetItemData( l_childId ) ) );
                l_childSelectionBox->setIsActive( l_selectionObject->getIsActive() );
                m_treeWidget->SetItemImage( l_childId, l_childSelectionBox->getIcon() );
                l_childSelectionBox->setIsDirty( true );
            }
        }
    }
    else if( l_selected == CHILD_OBJECT )
    {
        SelectionObject* l_box = (SelectionObject*)( m_treeWidget->GetItemData( l_treeId ) );

        l_box->toggleIsNOT();
        wxTreeItemId l_parentId = m_treeWidget->GetItemParent( l_treeId );
        ((SelectionObject*)( m_treeWidget->GetItemData( l_parentId ) ) )->setIsDirty( true );

        if( l_box->getIsNOT() )
            m_treeWidget->SetItemBackgroundColour( l_treeId, *wxRED );
        else
            m_treeWidget->SetItemBackgroundColour( l_treeId, *wxGREEN );
    }

    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnTreeEvent( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_selBoxChanged = true;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnTreeLabelEdit( wxTreeEvent& event )
{
    wxTreeItemId l_treeId = event.GetItem();
    int l_selected = treeSelected( l_treeId );

    if( l_selected == CHILD_OBJECT || l_selected == MASTER_OBJECT )
        ( (SelectionObject*)m_treeWidget->GetItemData( l_treeId ) )->setName( event.GetLabel() );
}

//////////////////////////////////////////////////////////////////////////
// Helper function to determine what kind of item is selected in the tree widget.
//
// i_id         : The selected item id.
//////////////////////////////////////////////////////////////////////////
int MainFrame::treeSelected( wxTreeItemId i_id )
{
    if( ! i_id.IsOk() ) 
        return 0;

    wxTreeItemId l_pId = m_treeWidget->GetItemParent( i_id );

    if( ! l_pId.IsOk() ) 
        return 0;

    wxTreeItemId l_ppId = m_treeWidget->GetItemParent( l_pId );

    if( ! l_ppId.IsOk() ) 
        return 0;

    if( l_pId == m_tSelectionObjectsId )
        return MASTER_OBJECT;
    else if ( l_ppId == m_tSelectionObjectsId )
        return CHILD_OBJECT;
    else if ( l_pId == m_tPointId )
        return POINT_DATASET;

    return 0;
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when someone click on the Load Datasets button.
//////////////////////////////////////////////////////////////////////////
void MainFrame::OnLoadDatasets( wxCommandEvent& WXUNUSED(event) )
{
    loadIndex( 1 );
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when someone click on the Load Meshes button.
//////////////////////////////////////////////////////////////////////////
void MainFrame::OnLoadMeshes( wxCommandEvent& WXUNUSED(event) )
{
    loadIndex( 2 );
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when someone click on the Load Fibers button.
//////////////////////////////////////////////////////////////////////////
void MainFrame::OnLoadFibers( wxCommandEvent& WXUNUSED(event) )
{
    loadIndex( 5 );
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when someone click on the Load Tensors button.
//////////////////////////////////////////////////////////////////////////
void MainFrame::OnLoadTensors( wxCommandEvent& WXUNUSED(event) )
{
    loadIndex( 8 );
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when someone click on the Load ODFs button.
//////////////////////////////////////////////////////////////////////////
void MainFrame::OnLoadODFs( wxCommandEvent& WXUNUSED(event) )
{
    loadIndex( 9 );
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when someone click on the Load Test Fibers button.
//////////////////////////////////////////////////////////////////////////
void MainFrame::OnLoadTestFibers( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_anatomyLoaded )
    {
        m_datasetHelper->m_lastError = wxT( "No anatomy file loaded" );
    }
    else if( m_datasetHelper->m_fibersLoaded )
    {
        m_datasetHelper->m_lastError = wxT( "Fibers already loaded" );
    }
    else
    {
        Fibers* l_fibers = new Fibers( m_datasetHelper );

        l_fibers->loadTestFibers();

        m_datasetHelper->m_fibersLoaded = true;

        l_fibers->setThreshold( 0.0f );
        l_fibers->setShow     ( true );
        l_fibers->setShowFS   ( true );
        l_fibers->setuseTex   ( true );

        m_datasetHelper->finishLoading( l_fibers );

        return;
    }

    wxMessageBox( wxT( "ERROR\n" ) + m_datasetHelper->m_lastError, wxT( "" ), wxOK | wxICON_INFORMATION, NULL );
    GetStatusBar()->SetStatusText( wxT( "ERROR" ), 1 );
    GetStatusBar()->SetStatusText( m_datasetHelper->m_lastError, 2 );
}

//////////////////////////////////////////////////////////////////////////
// This function will load a specific type specified by the index in argument.
//
// i_index          : The index we are trying to load.
//
// Returns true if there was no errors, false otherwise.
//////////////////////////////////////////////////////////////////////////
bool MainFrame::loadIndex( int i_index )
{
    if( ! m_datasetHelper->load( i_index ) )
    {
        wxMessageBox( wxT( "ERROR\n" ) + m_datasetHelper->m_lastError, wxT( "" ), wxOK | wxICON_INFORMATION, NULL );
        GetStatusBar()->SetStatusText( wxT( "ERROR" ), 1 );
        GetStatusBar()->SetStatusText( m_datasetHelper->m_lastError, 2 );
        return false;
    }
    else
        m_datasetHelper->m_selBoxChanged = true;

    return true;
}

///////////////////////////////////////////////////////////////////////////
// Moves all boundary points in one direction.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnMovePoints1( wxCommandEvent& WXUNUSED(event) )
{
    wxTreeItemId l_id, l_childid;
    wxTreeItemIdValue l_cookie = 0;
    l_id = m_treeWidget->GetFirstChild( m_tPointId, l_cookie );
    while( l_id.IsOk() )
    {
        SplinePoint* l_point = (SplinePoint*)( m_treeWidget->GetItemData( l_id ) );
        if( l_point->isBoundary() )
            l_point->setX( l_point->X() + 5.0 );

        l_id = m_treeWidget->GetNextChild( m_tPointId, l_cookie );
    }

    m_datasetHelper->m_surfaceIsDirty = true;
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// Moves all boundary points in one direction.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnMovePoints2(wxCommandEvent& WXUNUSED(event))
{
    wxTreeItemId l_id, l_childid;
    wxTreeItemIdValue l_cookie = 0;
    l_id = m_treeWidget->GetFirstChild( m_tPointId, l_cookie );
    while( l_id.IsOk() )
    {
        SplinePoint* l_point = (SplinePoint*)( m_treeWidget->GetItemData( l_id ) );
        if( l_point->isBoundary())
            l_point->setX( l_point->X() - 5.0 );

        l_id = m_treeWidget->GetNextChild( m_tPointId, l_cookie );
    }

    m_datasetHelper->m_surfaceIsDirty = true;
    refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 * System event funtions
 *
 *
 ****************************************************************************************************/

/****************************************************************************************************
 *
 * OnSize gets called when the size of the main window changes
 *
 *
 ****************************************************************************************************/

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSize( wxSizeEvent& WXUNUSED(event) )
{
    wxSize l_clientSize = this->GetClientSize();

    if( m_enlargeNav == 1 )
    {
        int l_newSize = ( l_clientSize.y - 65 ) / 3;

        if( m_gl0 )
        {
            m_gl0->SetMinSize( wxSize( l_newSize, l_newSize ) );
            m_gl1->SetMinSize( wxSize( l_newSize, l_newSize ) );
            m_gl2->SetMinSize( wxSize( l_newSize, l_newSize ) );
            m_gl0->SetMaxSize( wxSize( l_newSize, l_newSize ) );
            m_gl1->SetMaxSize( wxSize( l_newSize, l_newSize ) );
            m_gl2->SetMaxSize( wxSize( l_newSize, l_newSize ) );

            m_xSlider->SetMinSize( wxSize( l_newSize, -1 ) );
            m_ySlider->SetMinSize( wxSize( l_newSize, -1 ) );
            m_zSlider->SetMinSize( wxSize( l_newSize, -1 ) );
        }
    }
    if( m_enlargeNav == 2 )
    {
        int l_newSize = wxMin( ( l_clientSize.x - m_leftMainSizer->GetSize().x - 5 )/2 ,l_clientSize.y /2 -5 );

        m_gl0->SetMinSize( wxSize( l_newSize, l_newSize ) );
        m_gl1->SetMinSize( wxSize( l_newSize, l_newSize ) );
        m_gl2->SetMinSize( wxSize( l_newSize, l_newSize ) );
        m_mainGL->SetMinSize( wxSize( l_newSize, l_newSize ) );

        m_gl0->SetMaxSize( wxSize( l_newSize, l_newSize ) );
        m_gl1->SetMaxSize( wxSize( l_newSize, l_newSize ) );
        m_gl2->SetMaxSize( wxSize( l_newSize, l_newSize ) );
        m_mainGL->SetMaxSize( wxSize( l_newSize, l_newSize ) );

        m_xSlider->SetMinSize( wxSize( l_newSize, -1) );
        m_ySlider->SetMinSize( wxSize( l_newSize, -1) );
        m_zSlider->SetMinSize( wxSize( l_newSize, -1) );
    }

    if( GetSizer() )
        GetSizer()->SetDimension(0, 0, l_clientSize.x, l_clientSize.y );

    if( m_mainGL )
        m_mainGL->changeOrthoSize();

    this->Update();
    this->Refresh();
}

///////////////////////////////////////////////////////////////////////////
// Gets called when a thread for the kdTree creation finishes this function
// is here because of some limitations in the event handling system.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnKdTreeThreadFinished( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->treeFinished();
}

///////////////////////////////////////////////////////////////////////////
// OnGLEvent handles mouse events in the GL Refreshing widgets.
//
// event            :
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGLEvent( wxCommandEvent &event )
{
    wxPoint l_pos, l_newPos;
    float l_max = wxMax( m_datasetHelper->m_columns, wxMax( m_datasetHelper->m_rows, m_datasetHelper->m_frames ) );    

    int NAV_GL_SIZE = m_gl0->GetSize().x;

    switch( event.GetInt() )
    {
        case AXIAL:
        {
            l_pos = m_gl0->getMousePos();
            float x = ( (float)l_pos.x / NAV_GL_SIZE * l_max );
            float y = ( (float)( NAV_GL_SIZE - l_pos.y ) / NAV_GL_SIZE * l_max );

            m_xSlider->SetValue( (int)( x - ( l_max - m_datasetHelper->m_columns) / 2.0f ) );
            m_ySlider->SetValue( (int)( y - ( l_max - m_datasetHelper->m_rows) / 2.0f ) );
            break;
        }
        case CORONAL:
        {
            l_pos = m_gl1->getMousePos();
            float x = ( (float)l_pos.x / NAV_GL_SIZE ) * l_max;
            float y = ( (float)( NAV_GL_SIZE - l_pos.y ) / NAV_GL_SIZE * l_max );

            m_xSlider->SetValue( (int)( x - ( l_max - m_datasetHelper->m_columns) / 2.0f ) );
            m_zSlider->SetValue( (int)( y - ( l_max - m_datasetHelper->m_frames)  / 2.0f ) );
            break;
        }
        case SAGITTAL:
        {
            l_pos = m_gl2->getMousePos();
            float x = ( (float)( NAV_GL_SIZE - l_pos.x ) / NAV_GL_SIZE ) * l_max;
            float y = ( (float)( NAV_GL_SIZE - l_pos.y ) / NAV_GL_SIZE * l_max );

            m_ySlider->SetValue( (int)( x - ( l_max - m_datasetHelper->m_rows)   / 2.0f ) );
            m_zSlider->SetValue( (int)( y - ( l_max - m_datasetHelper->m_frames) / 2.0f ) );
            break;
        }
        case MAIN_VIEW:
            int delta = (int) m_mainGL->getDelta();

            switch( m_mainGL->getPicked() )
            {
                case AXIAL:
                    m_zSlider->SetValue( (int) wxMin( wxMax( m_zSlider->GetValue() + delta, 0 ), m_zSlider->GetMax() ) );
                    break;
                case CORONAL:
                    m_ySlider->SetValue( (int) wxMin( wxMax( m_ySlider->GetValue() + delta, 0 ), m_ySlider->GetMax() ) );
                    break;
                case SAGITTAL:
                    m_xSlider->SetValue( (int) wxMin( wxMax( m_xSlider->GetValue() + delta, 0 ), m_xSlider->GetMax() ) );
                    break;
            }
    }

    m_datasetHelper->updateView( m_xSlider->GetValue(), m_ySlider->GetValue(), m_zSlider->GetValue() );
    updateStatusBar();
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// OnMouseEvent just repaints the Window.
//
// event        :
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnMouseEvent( wxMouseEvent& WXUNUSED(event) )
{
    //this->Refresh();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::updateMenus()
{
    // Get the options menu.
    wxMenu* l_oMenu = GetMenuBar()->GetMenu( 4 );
    l_oMenu->Check( l_oMenu->FindItem( _T( "Toggle Fiber Lighting" ) ),  m_datasetHelper->m_lighting       );
    l_oMenu->Check( l_oMenu->FindItem( _T( "Invert Fiber Selection" ) ), m_datasetHelper->m_fibersInverted );

    l_oMenu->Check(  l_oMenu->FindItem( _T( "Use Tubes" ) ),              m_datasetHelper->m_useFakeTubes       );
    l_oMenu->Check(  l_oMenu->FindItem( _T( "Use Transparent Fibers" ) ), m_datasetHelper->m_useTransparency    );
    l_oMenu->Check(  l_oMenu->FindItem( _T( "Show Color Map" ) ),         m_datasetHelper->m_showColorMapLegend );

    wxMenu* l_sMenu = GetMenuBar()->GetMenu( 3 );
    l_sMenu->Check( l_sMenu->FindItem( _T( "Blend Texture on Mesh" ) ),         m_datasetHelper->m_blendTexOnMesh );
    l_sMenu->Check( l_sMenu->FindItem( _T( "Filter Dataset for IsoSurface" ) ), m_datasetHelper->m_filterIsoSurf  );

    GetToolBar()->ToggleTool( BUTTON_AXIAL,                      m_datasetHelper->m_showAxial    );
    GetToolBar()->ToggleTool( BUTTON_CORONAL,                    m_datasetHelper->m_showCoronal  );
    GetToolBar()->ToggleTool( BUTTON_SAGITTAL,                   m_datasetHelper->m_showSagittal );
    GetToolBar()->ToggleTool( BUTTON_TOGGLE_ALPHA,               m_datasetHelper->m_blendAlpha   );
    GetToolBar()->ToggleTool( MENU_OPTIONS_TOGGLE_LIGHTING,      m_datasetHelper->m_lighting     );
    GetToolBar()->ToggleTool( MENU_VOI_RENDER_SELECTION_OBJECTS, m_datasetHelper->m_showObjects  );
    GetToolBar()->ToggleTool( MENU_OPTIONS_USE_FAKE_TUBES,       m_datasetHelper->m_useFakeTubes );
    GetToolBar()->ToggleTool( MENU_SPLINESURF_DRAW_POINTS,       m_datasetHelper->m_pointMode    );

    wxMenu* l_voiMenu = GetMenuBar()->GetMenu( 2 );
    l_voiMenu->Check ( l_voiMenu->FindItem( _T( "active"   ) ), false );
    l_voiMenu->Check ( l_voiMenu->FindItem( _T( "visible"  ) ), false );
    l_voiMenu->Enable( l_voiMenu->FindItem( _T( "active"   ) ), false );
    l_voiMenu->Enable( l_voiMenu->FindItem( _T( "visible"  ) ), false );
    l_voiMenu->Check ( l_voiMenu->FindItem( _T( "morphing" ) ), m_datasetHelper->m_morphing );

    wxMenu* l_viewMenu = GetMenuBar()->GetMenu( 1 );
    l_viewMenu->Check( l_viewMenu->FindItem( _T( "show crosshair") ), m_datasetHelper->m_showCrosshair );

    wxTreeItemId l_treeId = m_treeWidget->GetSelection();
    int l_selected = treeSelected(l_treeId);

    if( l_selected == CHILD_OBJECT || l_selected == MASTER_OBJECT )
    {
        l_voiMenu->Enable( l_voiMenu->FindItem( _T( "active"  ) ), true );
        l_voiMenu->Enable( l_voiMenu->FindItem( _T( "visible" ) ), true );
        l_voiMenu->Check ( l_voiMenu->FindItem( _T( "active"  ) ), m_datasetHelper->m_lastSelectedObject->getIsActive() );
        l_voiMenu->Check ( l_voiMenu->FindItem( _T( "visible" ) ), m_datasetHelper->m_lastSelectedObject->getIsVisible() );

        GetToolBar()->ToggleTool( MENU_VOI_RENDER_SELECTION_OBJECTS, m_datasetHelper->m_showObjects );
        GetToolBar()->ToggleTool( MENU_VOI_TOGGLE_SELECTION_OBJECTS, ! m_datasetHelper->m_lastSelectedObject->getIsActive() );
    }

    //MENU_FILE_NEW_ISOSURF
    l_sMenu->Enable( l_sMenu->FindItem( _T( "New Spline Surface" ) ), ! m_datasetHelper->m_surfaceLoaded );
    GetToolBar()->EnableTool( MENU_SPLINESURF_NEW, !m_datasetHelper->m_surfaceLoaded );
    l_sMenu->Enable( l_sMenu->FindItem( _T( "New Iso Surface" ) ),              false );
    GetToolBar()->EnableTool( MENU_FILE_NEW_ISOSURF, false );
    l_sMenu->Enable( l_sMenu->FindItem( _T( "New Distance Map" ) ),             false );
    GetToolBar()->EnableTool( MENU_SURFACE_NEW_OFFSET, false) ;
    l_sMenu->Enable( l_sMenu->FindItem( _T( "Toggle Texture Mode" ) ),          false );
    l_sMenu->Enable( l_sMenu->FindItem( _T( "Toggle Lic" ) ),                   false );
    l_sMenu->Enable( l_sMenu->FindItem( _T( "Toggle Normal Direction" ) ),      false );
    l_sMenu->Enable( l_sMenu->FindItem( _T( "Clean Artefacts from Surface" ) ), false );
    l_sMenu->Enable( l_sMenu->FindItem( _T( "Smooth Surface (Loop SubD)" ) ),   false );

    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item != -1 )
    {
        DatasetInfo *l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
        if( l_info->getType() < RGB )
        {
            l_sMenu->Enable( l_sMenu->FindItem( _T( "New Iso Surface" ) ),  true );
            GetToolBar()->EnableTool(MENU_FILE_NEW_ISOSURF, true );
            l_sMenu->Enable( l_sMenu->FindItem( _T( "New Distance Map" ) ), true );
            GetToolBar()->EnableTool(MENU_SURFACE_NEW_OFFSET, true );
        }
        if( l_info->getType() < MESH )
        {
            l_sMenu->Enable( l_sMenu->FindItem( _T( "Toggle Texture Mode" ) ), true );
            l_sMenu->Check(  l_sMenu->FindItem( _T( "Toggle Texture Mode" ) ), ! l_info->getShowFS() );
        }
        if( l_info->getType() == ISO_SURFACE )
        {
            l_sMenu->Enable( l_sMenu->FindItem( _T( "Toggle Lic" ) ), m_datasetHelper->m_vectorsLoaded );
            l_sMenu->Enable( l_sMenu->FindItem( _T( "Clean Artefacts from Surface" ) ), true );
            l_sMenu->Enable( l_sMenu->FindItem( _T( "Smooth Surface (Loop SubD)" ) ),   true );
            l_sMenu->Check(  l_sMenu->FindItem( _T( "Toggle Lic" ) ), l_info->getUseLIC() );
        }
        if( l_info->getType() == SURFACE )
        {
            l_sMenu->Enable( l_sMenu->FindItem( _T( "Toggle Normal Direction" ) ), true );
            l_sMenu->Enable( l_sMenu->FindItem( _T( "Toggle Lic" ) ), m_datasetHelper->m_vectorsLoaded );
            l_sMenu->Check(  l_sMenu->FindItem( _T( "Toggle Lic" ) ), l_info->getUseLIC() );
            l_sMenu->Check(  l_sMenu->FindItem( _T( "Toggle Normal Direction" ) ), ( m_datasetHelper->m_normalDirection < 0 ) );
            l_sMenu->Enable( l_sMenu->FindItem( _T( "Smooth Surface (Loop SubD)" ) ), true );
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnTimerEvent( wxTimerEvent& WXUNUSED(event) )
{
    refreshAllGLWidgets();
    m_datasetHelper->increaseAnimationStep();
}
