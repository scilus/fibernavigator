/////////////////////////////////////////////////////////////////////////////
// Name:            mainFrame.cpp
// Author:          ---
// Creation Date:   ---
//
// Description: This is the implementation file for mainFrame class.
//
// Last modifications:
//      by : ggirard - 02-2011
/////////////////////////////////////////////////////////////////////////////


#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include <wx/colordlg.h>
#include <wx/filedlg.h>
#include <wx/statbmp.h>

#include "mainFrame.h"
#include "toolBar.h"
#include "menuBar.h"
#include "mainCanvas.h"
#include "SelectionBox.h"
#include "SelectionEllipsoid.h"
#include "../dataset/Anatomy.h"
#include "../dataset/fibers.h"
#include "../dataset/ODFs.h"
#include "../dataset/splinePoint.h"
#include "../dataset/surface.h"
#include "../dataset/Tensors.h"
#include "../gfx/theScene.h"
#include "../main.h"
#include "../misc/IsoSurface/CIsoSurface.h"


#define FIBERS_INFO_GRID_COL_SIZE              1
#define FIBERS_INFO_GRID_ROW_SIZE              11
#define FIBERS_INFO_GRID_TITLE_LABEL_SIZE      150

extern const wxEventType wxEVT_NAVGL_EVENT;

/****************************************************************************************************
 *
 * Event Table
 *
 ****************************************************************************************************/
BEGIN_EVENT_TABLE( MainFrame, wxFrame )
/* 
 * List widget events
 */
EVT_LIST_ITEM_ACTIVATED  ( ID_LIST_CTRL,                    MainFrame::OnActivateListItem   )
EVT_LIST_ITEM_SELECTED   ( ID_LIST_CTRL,                    MainFrame::OnSelectListItem     )

/*
 * Tree widget events
 */
EVT_TREE_DELETE_ITEM     ( ID_TREE_CTRL,                    MainFrame::OnDeleteTreeItem     )
EVT_TREE_SEL_CHANGED     ( ID_TREE_CTRL,                    MainFrame::OnSelectTreeItem     )
EVT_TREE_ITEM_ACTIVATED  ( ID_TREE_CTRL,                    MainFrame::OnActivateTreeItem   )
EVT_TREE_ITEM_RIGHT_CLICK( ID_TREE_CTRL,                    MainFrame::OnRightClickTreeItem )
EVT_TREE_END_LABEL_EDIT  ( ID_TREE_CTRL,                    MainFrame::OnTreeLabelEdit      )

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

/* KDTREE thread finished */
EVT_MENU( KDTREE_EVENT,                                     MainFrame::OnKdTreeThreadFinished )

EVT_TIMER( -1,                                              MainFrame::OnTimerEvent )

END_EVENT_TABLE()

MainFrame::MainFrame(      wxWindow*   i_parent, 
                     const wxWindowID i_id, 
                     const wxString   &i_title, 
                     const wxPoint    &i_pos, 
                     const wxSize     &i_size, 
                     const long       i_style) :
    wxFrame( i_parent, i_id, i_title, i_pos, i_size, i_style )
{
    wxImage::AddHandler(new wxPNGHandler);
    m_isDisplayProperties = true;
    m_lastSelectedFNObject = NULL;
    m_lastSelectedListItem = -1;
    m_currentFNObject = NULL;
    m_currentListItem = -1;   
    m_currentSizer = NULL;
    m_enlargeNav = 0;

    m_xSlider  = new wxSlider( this, ID_X_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( 150, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_ySlider  = new wxSlider( this, ID_Y_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( 150, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_zSlider  = new wxSlider( this, ID_Z_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( 150, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );

  
    m_listCtrl   = new MyListCtrl( this, ID_LIST_CTRL, wxDefaultPosition, wxSize( 293, -1 ), wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_NO_HEADER );

    m_treeWidget = new MyTreeCtrl( this, ID_TREE_CTRL, wxDefaultPosition,wxSize( 150, -1 ), wxTR_HAS_BUTTONS | wxTR_SINGLE | wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS );
    m_treeWidget->SetMaxSize( wxSize( 150, -1 ) );

    wxImageList* imageList = new wxImageList( 16, 16 );

    imageList->Add( ( wxImage( MyApp::respath +_T( "icons/eyes.png"   ), wxBITMAP_TYPE_PNG ) ) );
    imageList->Add( ( wxImage( MyApp::respath +_T( "icons/delete.png" ), wxBITMAP_TYPE_PNG ) ) );

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

    tImageList->Add( wxImage( MyApp::respath + _T( "icons/delete.png" ), wxBITMAP_TYPE_PNG ) );
    tImageList->Add( wxImage( MyApp::respath + _T( "icons/eyes.png" ),   wxBITMAP_TYPE_PNG ) );

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

    m_mainSizer         = new wxBoxSizer( wxHORIZONTAL ); // Contains everything in the UI.
    m_leftMainSizer     = new wxBoxSizer( wxVERTICAL );   // Contains everything left in the UI (leftSizer, listCtrl and buttonSizer).
    m_leftSizer         = new wxBoxSizer( wxHORIZONTAL ); // Contains the treeSizer and the navSizer.
    m_navSizer          = new wxBoxSizer( wxVERTICAL );   // Contains the 3 navigation windows with there respectiv sliders.
    m_treeSizer         = new wxBoxSizer( wxVERTICAL );   // Contains the tree and the loadDataSets, loadMeshes and the loadFibers buttons.
    m_rightMainSizer    = new wxBoxSizer( wxHORIZONTAL ); // Contains the OpenGl window and the rightSizer.

    m_noSelectionSizer = new wxBoxSizer( wxVERTICAL );
    m_noSelectionSizer->Add(new wxStaticText( this, wxID_ANY, wxT("No Object Selected"),wxDefaultPosition, wxSize(200,15)));

    m_navSizer->Add( m_gl0,     1, wxALL | wxEXPAND | wxSHAPED, 1 );
    m_navSizer->Add( m_zSlider, 0, wxALL,                       1 );
    m_navSizer->Add( m_gl1,     1, wxALL | wxEXPAND | wxSHAPED, 1 );
    m_navSizer->Add( m_ySlider, 0, wxALL,                       1 );
    m_navSizer->Add( m_gl2,     1, wxALL | wxEXPAND | wxSHAPED, 1 );
    m_navSizer->Add( m_xSlider, 0, wxALL,                       1 );

    m_treeSizer->Add( m_treeWidget, 1, wxALL, 1 );
    
    m_leftSizer->Add( m_treeSizer, 0, wxALL | wxEXPAND, 0 );
    m_leftSizer->Add( m_navSizer,  1, wxALL | wxEXPAND, 0 );

    m_leftMainSizer->Add( m_leftSizer,   0, wxALL,                  1 );
    m_leftMainSizer->Add( m_listCtrl,    1, wxALL | wxEXPAND,       1 );

    m_rightMainSizer->Add( m_mainGL, 1, wxEXPAND | wxALL, 2 );
    m_rightMainSizer->Add(m_noSelectionSizer, 0, wxALL, 1 );

    m_mainSizer->Add( m_leftMainSizer,  0, wxEXPAND | wxALL, 0 );
    m_mainSizer->Add( m_rightMainSizer, 1, wxEXPAND | wxALL, 0 );
        
    this->SetBackgroundColour(*wxLIGHT_GREY);
    SetSizer( m_mainSizer );
    m_mainSizer->SetSizeHints( this );

    m_timer = new wxTimer( this );
    m_timer->Start( 50 );

    m_menuBar = new MenuBar();
    m_toolBar = new ToolBar(this);
    m_toolBar->Realize();
    m_menuBar->initMenuBar(this);
    m_toolBar->initToolBar(this);

    SetMenuBar(m_menuBar);
    SetToolBar(m_toolBar);

    int widths[] = { 250, 250, -1 };
    CreateStatusBar( 2 );
    GetStatusBar()->SetFieldsCount( WXSIZEOF(widths), widths );
    GetStatusBar()->Show(); 
}

MainFrame::~MainFrame()
{
    m_timer->Stop();
    m_datasetHelper->printDebug( _T( "main frame destructor" ), 0 );
    m_datasetHelper->printDebug( _T( "timer stoped" ), 0 );    
    
    if( m_datasetHelper )               
        delete m_datasetHelper;
}

void MainFrame::OnNewIsoSurface( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->createIsoSurface();
}

void MainFrame::OnNewOffsetSurface( wxCommandEvent& WXUNUSED(event ))
{
    m_datasetHelper->createDistanceMapAndIso();
}

void MainFrame::OnNewDistanceMap (wxCommandEvent& WXUNUSED(event))
{
    m_datasetHelper->createDistanceMap();
}

void MainFrame::OnLoad( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->load( 0 ) )
    {
        wxMessageBox( wxT( "ERROR\n" ) + m_datasetHelper->m_lastError, wxT( "" ), wxOK | wxICON_INFORMATION, NULL );
        GetStatusBar()->SetStatusText( wxT( "ERROR" ), 1 );
        GetStatusBar()->SetStatusText( m_datasetHelper->m_lastError, 2 );
        return;
    }
    refreshAllGLWidgets();
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

void MainFrame::OnReloadShaders( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_scheduledReloadShaders = true;
    renewAllGLWidgets();
}

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

void MainFrame::OnSaveFibers( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_fibersLoaded )
        return;

    Fibers* l_fibers = NULL;
    m_datasetHelper->getFiberDataset( l_fibers );

    wxString caption         = wxT( "Choose a file" );
    wxString wildcard        = wxT( "VTK fiber files (*.fib)|*.fib|DMRI fiber files (*.fib)|*.fib|*.*|*.*" );
    wxString defaultDir      = wxEmptyString;
    wxString defaultFilename = wxEmptyString;
    wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
    dialog.SetFilterIndex( 0 );
    dialog.SetDirectory( m_datasetHelper->m_lastPath );

    if( dialog.ShowModal() == wxID_OK )
    {
        m_datasetHelper->m_lastPath = dialog.GetDirectory();
        printf("%d\n",dialog.GetFilterIndex());
        if (dialog.GetFilterIndex()==1)
        {
             l_fibers->saveDMRI( dialog.GetPath() );
        }
        else
        {
            l_fibers->save( dialog.GetPath() );
        }
    }
}


void MainFrame::OnSaveDataset( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        if( ((DatasetInfo*)m_currentFNObject)->getType() < MESH )
        {
            Anatomy* l_anatomy = (Anatomy*)m_currentFNObject;

            wxString caption         = wxT( "Choose l_anatomy file" );
            wxString wildcard        = wxT( "nifti files (*.nii)|*.nii*|*.*|*.*" );
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

void MainFrame::OnQuit( wxCommandEvent& WXUNUSED(event) )
{
    wxMessageDialog dialog( NULL, wxT( "Really Quit?" ), wxT( "Really Quit?" ), wxNO_DEFAULT | wxYES_NO | wxICON_INFORMATION );
    if( dialog.ShowModal() == wxID_YES )
    {  
        Close(true);
    }
}

void MainFrame::OnMinimizeDataset( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        if( ((DatasetInfo*)m_currentFNObject)->getType() < MESH )
        {
             ((Anatomy*)m_currentFNObject)->minimize();
        }
    }
}

void MainFrame::OnDilateDataset( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
         if( ((DatasetInfo*)m_currentFNObject)->getType() < MESH )
        {
             ((Anatomy*)m_currentFNObject)->dilate();
        }
    }
}

void MainFrame::OnErodeDataset(wxCommandEvent& WXUNUSED(event))
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        if( ((DatasetInfo*)m_currentFNObject)->getType() < MESH )
        {
            ((Anatomy*)m_currentFNObject)->erode();
        }
    }
}

void MainFrame::OnSaveSurface( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        if( ((DatasetInfo*)m_currentFNObject)->getType() == SURFACE )
        {
            Surface *l_surface = (Surface*)m_currentFNObject;

            wxString caption         = wxT( "Choose a file" );
            wxString wildcard        = wxT( "surfae files (*.vtk)|*.vtk" );
            wxString defaultDir      = wxEmptyString;
            wxString defaultFilename = wxEmptyString;
            wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
            dialog.SetFilterIndex( 0 );
            dialog.SetDirectory( m_datasetHelper->m_lastPath );
            if( dialog.ShowModal() == wxID_OK )
            {
                m_datasetHelper->m_lastPath = dialog.GetDirectory();
                l_surface->save( dialog.GetPath() );
            }
        }
        else if( ((DatasetInfo*)m_currentFNObject)->getType() == ISO_SURFACE )
        {
            CIsoSurface *l_surface = (CIsoSurface*)m_currentFNObject;

            wxString caption         = wxT( "Choose a file" );
            wxString wildcard        = wxT( "surfae files (*.vtk)|*.vtk" );
            wxString defaultDir      = wxEmptyString;
            wxString defaultFilename = wxEmptyString;
            wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
            dialog.SetFilterIndex( 0 );
            dialog.SetDirectory( m_datasetHelper->m_lastPath );
            if( dialog.ShowModal() == wxID_OK )
            {
                m_datasetHelper->m_lastPath = dialog.GetDirectory();
                l_surface->save( dialog.GetPath() );
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

void MainFrame::OnMenuViewReset( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_zoom  = 1;
    m_datasetHelper->m_xMove = 0;
    m_datasetHelper->m_yMove = 0;
    refreshAllGLWidgets();
}

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

void MainFrame::OnMenuViewTop( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_datasetHelper->m_transform );
    m_mainGL->setRotation();
}

void MainFrame::OnMenuViewBottom( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_datasetHelper->m_transform );
    m_datasetHelper->m_transform.s.M00 = -1.0f;
    m_datasetHelper->m_transform.s.M22 = -1.0f;
    m_mainGL->setRotation();
}

void MainFrame::OnMenuViewFront( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_datasetHelper->m_transform );
    m_datasetHelper->m_transform.s.M11 =  0.0f;
    m_datasetHelper->m_transform.s.M22 =  0.0f;
    m_datasetHelper->m_transform.s.M12 =  1.0f;
    m_datasetHelper->m_transform.s.M21 = -1.0f;
    m_mainGL->setRotation();
}

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

void MainFrame::OnMenuViewAxes( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_isShowAxes = ! m_datasetHelper->m_isShowAxes;
}

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

void MainFrame::OnToggleSelectionObjects( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene)
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

void MainFrame::OnToggleAndNot( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene)
        return;

    // Get what selection object is selected.
    wxTreeItemId l_selectionObjectTreeId = m_treeWidget->GetSelection();

    if( treeSelected(l_selectionObjectTreeId) == CHILD_OBJECT)
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

void MainFrame::OnToggleShowSelectionObject( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene)
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
    }
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display fibers 
// info after a right click in the tree on a selectio object.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnDisplayFibersInfo( wxCommandEvent& WXUNUSED(event) )
{
    ((SelectionObject*)m_currentFNObject)->SetFiberInfoGridValues();
}

//////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user clicks on the "Set as distance
// anchor option after right clicking on a selection object in the tree.
//////////////////////////////////////////////////////////////////////////

void MainFrame::OnDistanceAnchorSet( wxCommandEvent& event )
{
   if (m_datasetHelper->m_lastSelectedObject!=NULL)
   {
       m_datasetHelper->m_lastSelectedObject->UseForDistanceColoring(!m_datasetHelper->m_lastSelectedObject->IsUsedForDistanceColoring());
       ColorFibers();
   }
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display mean fiber 
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnDisplayMeanFiber( wxCommandEvent& WXUNUSED(event) )
{

}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display cross sections
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnDisplayCrossSections( wxCommandEvent& WXUNUSED(event) )
{
    ((SelectionObject*)m_currentFNObject)->m_displayCrossSections = (CrossSectionsDisplay)( ( (int)((SelectionObject*)m_currentFNObject)->m_displayCrossSections ) + 1 );
    if( ((SelectionObject*)m_currentFNObject)->m_displayCrossSections == CS_NB_OF_CHOICES )
        ((SelectionObject*)m_currentFNObject)->m_displayCrossSections = CS_NOTHING;
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display dispersion tube
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnDisplayDispersionTube( wxCommandEvent& WXUNUSED(event) )
{
    ((SelectionObject*)m_currentFNObject)->m_displayDispersionCone = (DispersionConeDisplay)( ( (int)((SelectionObject*)m_currentFNObject)->m_displayDispersionCone ) + 1 );
    if( ((SelectionObject*)m_currentFNObject)->m_displayDispersionCone == DC_NB_OF_CHOICES )
        ((SelectionObject*)m_currentFNObject)->m_displayDispersionCone = DC_NOTHING;
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the color with curvature button
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnColorWithCurvature( wxCommandEvent& WXUNUSED(event) )
{
    if(m_datasetHelper->m_fiberColorationMode != CURVATURE_COLOR)
    {
        m_datasetHelper->m_fiberColorationMode = CURVATURE_COLOR;
        ColorFibers();
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be triggered when the user click on the display min/max cross section
// button that is located in the m_fibersInfoSizer.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnColorWithTorsion( wxCommandEvent& WXUNUSED(event) )
{
    if(m_datasetHelper->m_fiberColorationMode != TORSION_COLOR)
    {
        m_datasetHelper->m_fiberColorationMode = TORSION_COLOR;
        ColorFibers();
    }
}

void MainFrame::OnNormalColoring( wxCommandEvent& WXUNUSED(event) )
{
    if(m_datasetHelper->m_fiberColorationMode != NORMAL_COLOR)
    {
        m_datasetHelper->m_fiberColorationMode = NORMAL_COLOR;
        ColorFibers();
    }
}

void MainFrame::OnGenerateFiberVolume( wxCommandEvent& WXUNUSED(event) )
{
    Fibers* pTmpFib = NULL;
    m_datasetHelper->getFiberDataset(pTmpFib);

    if(pTmpFib != NULL)
    {
        pTmpFib->generateFiberVolume();
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will call the updateFibersColors function on the currently loaded fiber set.
///////////////////////////////////////////////////////////////////////////
void MainFrame::ColorFibers()
{   
    if (m_currentFNObject != NULL && m_currentListItem != -1)
    {
        ((Fibers*)m_currentFNObject)->updateFibersColors();  
    }  
}

void MainFrame::deleteFNObject()
{
    if (m_currentSizer!=NULL)
    {
        m_rightMainSizer->Hide(m_currentSizer, true);
        m_rightMainSizer->Detach(m_currentSizer);        
        m_currentSizer = NULL;
    }
    //delete m_currentFNObject;
    m_currentFNObject = NULL;
    m_lastSelectedFNObject = NULL;
    m_currentListItem = -1;
    m_lastSelectedListItem = -1;

    m_rightMainSizer->Layout();
    if( m_mainGL )
    {
        m_mainGL->changeOrthoSize();
    }
}

void MainFrame::DisplayPropertiesSheet()
{   
    if (!m_isDisplayProperties)
    {
        m_rightMainSizer->Hide( m_noSelectionSizer, true); 
        if (m_currentSizer != NULL)
        {
            m_rightMainSizer->Hide( m_currentSizer, true );   
        }
    }
    else if (m_lastSelectedFNObject == NULL && m_currentFNObject == NULL)
    {        
        if (m_currentSizer != NULL)
        {
            m_rightMainSizer->Hide( m_currentSizer, true );   
        }
        m_rightMainSizer->Show( m_noSelectionSizer, true, true); 
    }
    else
    {
        FNObject *l_info = NULL;
        if (m_lastSelectedFNObject != m_currentFNObject)
        {
            l_info = m_lastSelectedFNObject;
        }
        else
        {
            l_info = m_currentFNObject;
        }
        
        if (l_info != NULL)
        {       
            m_rightMainSizer->Hide(m_noSelectionSizer);
            if (m_currentFNObject != l_info)
            {
                if (m_currentSizer != NULL )
                {
                    m_rightMainSizer->Hide(m_currentSizer);                    
                }
                if (!l_info->getProprietiesSizer())
                {
                    l_info->createPropertiesSizer(this);
                }   
                m_currentSizer = l_info->getProprietiesSizer();
                if (!m_rightMainSizer->Show( m_currentSizer, true, true ))
                {
                    m_rightMainSizer->Add(m_currentSizer, 0, wxALL, 1 );
                    m_rightMainSizer->Show( m_currentSizer, true, true );
                }                
            }
            else
            {
                m_rightMainSizer->Show( m_currentSizer, true, true );
            }        
            l_info->updatePropertiesSizer();
        }
        else
        {
            if (m_currentSizer != NULL)
            {
                m_rightMainSizer->Hide( m_currentSizer, true ); 
                m_currentSizer = NULL;
            }
            m_rightMainSizer->Show( m_noSelectionSizer, true, true); 
        }
        m_currentFNObject = l_info;
        m_currentListItem = m_lastSelectedListItem;
    }

    m_rightMainSizer->Layout();
    if( m_mainGL )
    {
        m_mainGL->changeOrthoSize();
    }        
}

void MainFrame::OnCreateFibersDensityTexture( wxCommandEvent& WXUNUSED(event) )
{
    Fibers* l_fibers = NULL;

    if( ! m_datasetHelper->getFiberDataset(l_fibers) )
        return ;

    int l_x,l_y,l_z;

    Anatomy* l_newAnatomy = new Anatomy( m_datasetHelper );
    l_newAnatomy->setZero( m_datasetHelper->m_columns, m_datasetHelper->m_rows, m_datasetHelper->m_frames );
    l_newAnatomy->setDataType( 16 );
    l_newAnatomy->setType( OVERLAY );
    float l_max = 0.0f;
    wxTreeItemId l_treeObjectId = m_treeWidget->GetSelection();
    if( treeSelected( l_treeObjectId ) == MASTER_OBJECT )
    {
        SelectionObject* l_object = (SelectionObject*)( m_treeWidget->GetItemData( l_treeObjectId ) );

        std::vector<float>* l_dataset = l_newAnatomy->getFloatDataset();

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

    l_newAnatomy->setName( wxT(" (fiber_density)" ) );
    l_newAnatomy->setOldMax( l_max );
    m_listCtrl->InsertItem( 0, wxT( "" ), 0 );
    m_listCtrl->SetItem( 0, 1, l_newAnatomy->getName() );
    m_listCtrl->SetItem( 0, 2, wxT( "0.00" ) );
    m_listCtrl->SetItem( 0, 3, wxT( "" ), 1 );
    m_listCtrl->SetItemData( 0, (long) l_newAnatomy );
    m_listCtrl->SetItemState( 0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    refreshAllGLWidgets();
}

void MainFrame::OnCreateFibersColorTexture( wxCommandEvent& WXUNUSED(event) )
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
    if( ! m_datasetHelper->m_theScene)
        return;

    Vector l_center( m_xSlider->GetValue() * m_datasetHelper->m_xVoxel, 
                     m_ySlider->GetValue() * m_datasetHelper->m_yVoxel, 
                     m_zSlider->GetValue() * m_datasetHelper->m_zVoxel );
    float l_sizeV = 10;

    Vector l_size( l_sizeV / m_datasetHelper->m_xVoxel, 
                   l_sizeV / m_datasetHelper->m_yVoxel,
                   l_sizeV / m_datasetHelper->m_zVoxel );
    

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
    m_treeWidget->SelectItem(l_newSelectionObjectId, true);    
    refreshAllGLWidgets();
}

void MainFrame::OnNewVoiFromOverlay( wxCommandEvent& WXUNUSED(event) )
{
    wxTreeItemId     l_treeObjectId     = m_tSelectionObjectsId;
    SelectionObject* l_selectionObject  = NULL;
    Anatomy*         l_anatomy          = NULL;

    if (m_datasetHelper->m_lastSelectedObject !=NULL)
    {
        l_treeObjectId = m_datasetHelper->m_lastSelectedObject->GetId();
    }
      
    if(m_currentFNObject != NULL && m_currentListItem != -1)
    {
        if ( ((DatasetInfo*)m_currentFNObject)->getType() < RGB)
        {
            l_anatomy = (Anatomy*)m_currentFNObject;
            l_selectionObject = new SelectionBox( m_datasetHelper, l_anatomy );
            float trs = l_anatomy->getThreshold();
            if( trs == 0.0 )
                trs = 0.01f;
            l_selectionObject->setThreshold( trs );
        }
        else
        {
            return;
        }
    }
    else
    {
        return;
    }

    if( treeSelected( l_treeObjectId ) == MASTER_OBJECT)
    {        
        wxTreeItemId l_treeNewObjectId  = m_treeWidget->AppendItem( l_treeObjectId, l_selectionObject->getName(), 0, -1,l_selectionObject);
        m_treeWidget->SetItemBackgroundColour( l_treeNewObjectId, *wxGREEN );
        m_treeWidget->EnsureVisible( l_treeNewObjectId );
        m_treeWidget->SetItemImage( l_treeNewObjectId, l_selectionObject->getIcon() );
        l_selectionObject->setTreeId( l_treeNewObjectId );
        l_selectionObject->setIsMaster( false );        
    }
    else
    {
        wxTreeItemId l_treeNewObjectId = m_treeWidget->AppendItem( m_tSelectionObjectsId, l_selectionObject->getName(), 0, -1, l_selectionObject );
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

void MainFrame::OnHideSelectionObjects( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene )
        return;

    m_datasetHelper->toggleShowAllSelectionObjects();
    refreshAllGLWidgets();
}

void MainFrame::OnActivateSelectionObjects( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene )
        return;

    m_datasetHelper->toggleActivateAllSelectionObjects();
    refreshAllGLWidgets();
}

void MainFrame::OnUseMorph( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene )
        return;

    m_datasetHelper->m_morphing = ! m_datasetHelper->m_morphing;
}

void MainFrame::OnColorRoi( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene )
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
#ifdef __WXMAC__
    wxColourDialog dialog( this);
#else
    wxColourDialog dialog( this, &l_colorData );
#endif
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

void MainFrame::OnVoiFlipNormals( wxCommandEvent& WXUNUSED(event) )
{
    wxTreeItemId l_selectionObjectTreeId = m_treeWidget->GetSelection();
    SelectionObject* l_selectionObject = (SelectionObject*)( m_treeWidget->GetItemData( l_selectionObjectTreeId ) );

    if(l_selectionObject->getSelectionType() == CISO_SURFACE_TYPE)
    {
        l_selectionObject->FlipNormals();        
    }
}

/****************************************************************************************************
 *
 * Menu Spline Surface
 *
 ****************************************************************************************************/

void MainFrame::OnNewSplineSurface( wxCommandEvent& WXUNUSED(event) )
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
                    wxString l_name = wxT("boundary l_point");
                    wxTreeItemId tId = m_treeWidget->AppendItem( m_tPointId, l_name, -1, -1, l_point );
                    l_point->setTreeId( tId );
                    l_point->setName(l_name);
                    l_point->setIsBoundary( true );
                }
                else
                {
                    if( m_datasetHelper->m_fibersLoaded && l_fibers->getBarycenter( l_point ) )
                    {
                        wxString l_name = wxT( "l_point" );
                        wxTreeItemId tId = m_treeWidget->AppendItem( m_tPointId, l_name, -1, -1, l_point );
                        l_point->setTreeId( tId );
                        l_point->setName(l_name);
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

void MainFrame::OnToggleDrawPointsMode( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene )
        return;

    m_datasetHelper->togglePointMode();
    refreshAllGLWidgets();
}

void MainFrame::OnToggleDrawVectors( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene )
        return;

    m_datasetHelper->m_drawVectors = ! m_datasetHelper->m_drawVectors;
    refreshAllGLWidgets();
}

void MainFrame::OnToggleLIC( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 && m_datasetHelper->m_vectorsLoaded )
    {
        ((DatasetInfo*) m_currentFNObject)->activateLIC();
    }
    refreshAllGLWidgets();
}

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

void MainFrame::OnToggleTextureFiltering( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_currentFNObject;
        if( l_info->getType() < MESH )
        {
            if( ! l_info->toggleShowFS() )
                m_listCtrl->SetItem( m_currentListItem, 1, l_info->getName() + wxT( "*" ) );
            else
                m_listCtrl->SetItem( m_currentListItem, 1, l_info->getName() );
        }
    }
    refreshAllGLWidgets();
}

void MainFrame::OnToggleLighting( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_lighting = ! m_datasetHelper->m_lighting;
    refreshAllGLWidgets();
}

void MainFrame::OnToggleBlendTexOnMesh( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_blendTexOnMesh = ! m_datasetHelper->m_blendTexOnMesh;
    refreshAllGLWidgets();
}

void MainFrame::OnToggleFilterIso( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_filterIsoSurf = ! m_datasetHelper->m_filterIsoSurf;
    refreshAllGLWidgets();
}

void MainFrame::OnClean( wxCommandEvent& WXUNUSED(event) )
{
     if(m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        DatasetInfo* l_info = (DatasetInfo*) m_currentFNObject;
        if( l_info->getType() == MESH || l_info->getType() == ISO_SURFACE)
           l_info->clean();
    }
    refreshAllGLWidgets();
}

void MainFrame::OnLoop( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        ((DatasetInfo*)m_currentFNObject)->smooth();
    }
    refreshAllGLWidgets();
}

void MainFrame::OnInvertFibers( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->invertFibers();
    m_datasetHelper->m_selBoxChanged = true;
    refreshAllGLWidgets();
}

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

void MainFrame::OnRulerTool( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_isRulerToolActive = !m_datasetHelper->m_isRulerToolActive;    
    if (m_datasetHelper->m_isRulerToolActive){
        m_toolBar->m_txtRuler->Enable();
    } else {
        m_toolBar->m_txtRuler->Disable();
    }

    refreshAllGLWidgets();
}

void MainFrame::OnRulerToolClear( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_rulerPts.clear();
    m_datasetHelper->m_rulerFullLength = 0;
    m_datasetHelper->m_rulerPartialLength = 0;

    refreshAllGLWidgets();
}

void MainFrame::OnRulerToolAdd( wxCommandEvent& WXUNUSED(event) )
{
    if (m_datasetHelper->m_isRulerToolActive && m_datasetHelper->m_rulerPts.size()>0){
        m_datasetHelper->m_rulerPts.push_back(m_datasetHelper->m_rulerPts.back());
    }
    refreshAllGLWidgets();
}

void MainFrame::OnRulerToolDel( wxCommandEvent& WXUNUSED(event) )
{
    if (m_datasetHelper->m_isRulerToolActive && m_datasetHelper->m_rulerPts.size()>0){
            m_datasetHelper->m_rulerPts.pop_back();
        }
    refreshAllGLWidgets();
}

void MainFrame::OnUseTransparency( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_useTransparency = ! m_datasetHelper->m_useTransparency;
    refreshAllGLWidgets();
}

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
    {
        return;
    }

    
    if( m_currentListItem != -1 )
    {
        DatasetInfo *l_info = (DatasetInfo*)m_currentFNObject;
        if( l_info->getType() == MESH || l_info->getType() == ISO_SURFACE || l_info->getType() == SURFACE || l_info->getType() == VECTORS)
        {
            l_info->setColor( l_col );
            l_info->setuseTex( false );
            m_listCtrl->SetItem( m_currentListItem, 2, wxT( "(") + wxString::Format( wxT( "%.2f" ), l_info->getThreshold() ) + wxT( ")" ) );           
        }
    }
    else if ( m_datasetHelper->m_lastSelectedObject != NULL )
    {
        SelectionObject *l_selObj = (SelectionObject*)m_currentFNObject;
        if (!l_selObj->getIsMaster())
        {
            wxTreeItemId l_parentId = m_treeWidget->GetItemParent( m_datasetHelper->m_lastSelectedObject->GetId());
            l_selObj = (SelectionObject*)m_treeWidget->GetItemData(l_parentId);
        }
        l_selObj->setFiberColor( l_col);
        l_selObj->setIsDirty( true );
        m_datasetHelper->m_selBoxChanged = true;
    }    
    refreshAllGLWidgets();
}

void MainFrame::OnResetColor(wxCommandEvent& WXUNUSED(event))
{
    Fibers* l_fibers = NULL; // Initalize it quiet compiler.
    if( ! m_datasetHelper->getFiberDataset( l_fibers ) )
        return;
    l_fibers->resetColorArray();
    m_datasetHelper->m_selBoxChanged = true;
    refreshAllGLWidgets();
}

void MainFrame::OnToggleColorMapLegend( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_showColorMapLegend = !m_datasetHelper->m_showColorMapLegend;
    refreshAllGLWidgets();
}

void MainFrame::OnSetCMap0( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_colorMap = 0;
    refreshAllGLWidgets();
}

void MainFrame::OnSetCMap1( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_colorMap = 1;
    refreshAllGLWidgets();
}

void MainFrame::OnSetCMap2( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_colorMap = 2;
    refreshAllGLWidgets();
}

void MainFrame::OnSetCMap3( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_colorMap = 3;
    refreshAllGLWidgets();
}

void MainFrame::OnSetCMap4( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_colorMap = 4;
    refreshAllGLWidgets();
}

void MainFrame::OnSetCMap5( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_colorMap = 5;
    refreshAllGLWidgets();
}

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

void MainFrame::OnAbout( wxCommandEvent& WXUNUSED(event) )
{
    wxString rev = _T( "$Revision$" );
    rev = rev.AfterFirst('$');
    rev = rev.BeforeLast('$');
    wxString date = _T( "$Date$" );
    date = date.AfterFirst( '$' );
    date = date.BeforeLast( '$' );
    (void)wxMessageBox( _T("Fiber Navigator\nAuthors:http://code.google.com/p/fibernavigator/people/list \n\n" )
                        + rev + _T( "\n" ) + date, _T( "About Fiber Navigator" ) );
}

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

void MainFrame::OnSlizeMovieSag( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->doLicMovie( 0 );
}

void MainFrame::OnSlizeMovieCor( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->doLicMovie(1);
}

void MainFrame::OnSlizeMovieAxi( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->doLicMovie( 2 );
}

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
   updateGlyphColoration( MIN_HUE, ((Glyph*)m_currentFNObject)->m_psliderMinHueValue->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the max hue value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphMaxHueSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
   updateGlyphColoration( MAX_HUE, ((Glyph*)m_currentFNObject)->m_psliderMaxHueValue->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the saturation value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphSaturationSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
   updateGlyphColoration( SATURATION,((Glyph*)m_currentFNObject)->m_psliderSaturationValue->GetValue() / 100.0f );
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the slider for the luminance value in 
// the glyph options panel moved.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphLuminanceSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
   updateGlyphColoration( LUMINANCE, ((Glyph*)m_currentFNObject)->m_psliderLuminanceValue->GetValue() / 100.0f );
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
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {            
        DatasetInfo* l_info = (DatasetInfo*)m_currentFNObject;    
        if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
            ( (Glyph*)l_info )->setColor( i_modifier, i_value );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the LOD of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphLODSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {            
        DatasetInfo* l_info = (DatasetInfo*)m_currentFNObject;
        if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
            ( (Glyph*)l_info )->setLOD( (LODChoices)((Glyph*)m_currentFNObject)->m_psliderLODValue->GetValue() );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the light attenuation of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphLightAttenuationSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {            
        DatasetInfo* l_info = (DatasetInfo*)m_currentFNObject;
        if( l_info->getType() == TENSORS || l_info->getType() == ODFS )
            ( (Glyph*)l_info )->setLighAttenuation( ((Glyph*)m_currentFNObject)->m_psliderLightAttenuation->GetValue() / 100.0f );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the light x position slider moved.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphLightXDirectionSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        OnGlyphLightPositionChanged( X_AXIS, ((Glyph*)m_currentFNObject)->m_psliderLightXPosition->GetValue() / 100.0f  );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the light y position slider moved.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphLightYDirectionSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        OnGlyphLightPositionChanged( Y_AXIS, ((Glyph*)m_currentFNObject)->m_psliderLightYPosition->GetValue() / 100.0f  );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the light z position slider moved.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphLightZDirectionSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        OnGlyphLightPositionChanged( Z_AXIS, ((Glyph*)m_currentFNObject)->m_psliderLightZPosition->GetValue() / 100.0f  );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the light position for the proper axis.
//
// i_axisType       : The axis that we want to set the lght position for.
// i_position       : The value of the position.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphLightPositionChanged( AxisType i_axisType, float i_position )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        ((Glyph*)m_currentFNObject)->setLightPosition( i_axisType, i_position);
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the display value of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphDisplaySliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        ((Glyph*)m_currentFNObject)->setDisplayFactor( ((Glyph*)m_currentFNObject)->m_psliderDisplayValue->GetValue());
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will set the scaling factor of a glyph by the value of its slider.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphScalingFactorSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        ((Glyph*)m_currentFNObject)->setScalingFactor( ((Glyph*)m_currentFNObject)->m_psliderScalingFactor->GetValue());
    }
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
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        ((Glyph*)m_currentFNObject)->flipAxis( i_axisType, i_isChecked );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the map on sphere radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphMapOnSphereSelected( wxCommandEvent& event )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        ((Glyph*)m_currentFNObject)->setDisplayShape( SPHERE );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the normal display radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphNormalSelected( wxCommandEvent& event )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        ((Glyph*)m_currentFNObject)->setDisplayShape( NORMAL );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the axes display radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphAxesSelected( wxCommandEvent& event )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        ((Glyph*)m_currentFNObject)->setDisplayShape( AXES );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the main axis display radio button in the 
// glyph options is selected.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphMainAxisSelected( wxCommandEvent& event )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        ((Glyph*)m_currentFNObject)->setDisplayShape( AXIS );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the color with position check box in the 
// glyph options is checked/unchecked.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnGlyphColorWithPosition( wxCommandEvent& event )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        ((Glyph*)m_currentFNObject)->setColorWithPosition( event.IsChecked() );
    }
}

void MainFrame::OnSliderIntensityThresholdMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        DatasetInfo* l_current = (DatasetInfo*)m_currentFNObject;
        float l_threshold = (float)l_current->m_psliderThresholdIntensity->GetValue() / 100.0f;

        if( l_current->getUseTex() )
            m_listCtrl->SetItem( m_currentListItem, 2, wxString::Format( wxT( "%.2f" ),   l_threshold * l_current->getOldMax() ) );
        else
            m_listCtrl->SetItem( m_currentListItem, 2, wxString::Format( wxT( "(%.2f)" ), l_threshold * l_current->getOldMax() ) );

        l_current->setThreshold( l_threshold );
        if( l_current->getType() == SURFACE )
        {
            Surface* s = (Surface*)l_current;
            s->movePoints();
        }
        if( l_current->getType() == ISO_SURFACE && ! l_current->m_psliderThresholdIntensity->leftDown() )
        {
            CIsoSurface* s = (CIsoSurface*)l_current;
            s->GenerateWithThreshold();
        }
        if( l_current->getType() < RGB )
        {
            Anatomy* a = (Anatomy*)l_current;
            if( a->m_roi )
                a->m_roi->setThreshold( l_threshold );
        }
        // This slider will set the Brightness level. Currently only the glyphs uses this value.
        l_current->setBrightness( 1.0f - l_threshold );
        refreshAllGLWidgets();
    }
}

void MainFrame::OnSliderOpacityThresholdMoved( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentFNObject != NULL && m_currentListItem != -1 )
    {
        DatasetInfo* l_current = (DatasetInfo*)m_currentFNObject;
        l_current->setAlpha( (float)l_current->m_psliderOpacity->GetValue() / 100.0f);
        refreshAllGLWidgets();
    }
}

void MainFrame::OnOriginalShBasis( wxCommandEvent& WXUNUSED(event) )
{
	ODFs* l_dataset = new ODFs( m_datasetHelper );
	((ODFs*)m_currentFNObject)->changeShBasis(l_dataset, m_datasetHelper, 0);
}

void MainFrame::OnDescoteauxShBasis( wxCommandEvent& WXUNUSED(event) )
{
	ODFs* l_dataset = new ODFs( m_datasetHelper );
	((ODFs*)m_currentFNObject)->changeShBasis(l_dataset, m_datasetHelper, 1);
}

void MainFrame::OnTournierShBasis( wxCommandEvent& WXUNUSED(event) )
{
	ODFs* l_dataset = new ODFs( m_datasetHelper );
	((ODFs*)m_currentFNObject)->changeShBasis(l_dataset, m_datasetHelper, 2);
}

void MainFrame::OnPTKShBasis( wxCommandEvent& WXUNUSED(event) )
{
	ODFs* l_dataset = new ODFs( m_datasetHelper );
	((ODFs*)m_currentFNObject)->changeShBasis(l_dataset, m_datasetHelper, 3);
}

void MainFrame::OnFiberFilterSlider( wxCommandEvent& WXUNUSED(event) )
{
    refreshAllGLWidgets();
}

void MainFrame::OnToggleShowAxial( wxCommandEvent& WXUNUSED(event) )
{
    if (m_datasetHelper->m_theScene)
    {
        m_datasetHelper->m_showAxial = ! m_datasetHelper->m_showAxial;
        m_mainGL->Refresh();
    }
}

void MainFrame::OnToggleShowCoronal( wxCommandEvent& WXUNUSED(event) )
{
    if( m_datasetHelper->m_theScene )
    {
        m_datasetHelper->m_showCoronal = ! m_datasetHelper->m_showCoronal;
        m_mainGL->Refresh();
    }
}

void MainFrame::OnToggleShowSagittal( wxCommandEvent& WXUNUSED(event) )
{
    if( m_datasetHelper->m_theScene )
    {
        m_datasetHelper->m_showSagittal = ! m_datasetHelper->m_showSagittal;
        m_mainGL->Refresh();
    }
}

void MainFrame::OnToggleAlpha( wxCommandEvent& WXUNUSED(event) )
{
    m_datasetHelper->m_blendAlpha = ! m_datasetHelper->m_blendAlpha;

    updateMenus();
    this->Update();  
    m_mainGL->Refresh();
}

void MainFrame::refreshAllGLWidgets()
{
    updateStatusBar();
    updateMenus();
    DisplayPropertiesSheet();

    if ( m_mainGL ) 
        m_mainGL->Refresh(true);
     if ( m_gl0 ) 
      m_gl0->Refresh(true);
    if ( m_gl1 ) 
        m_gl1->Refresh(true);
    if ( m_gl2 ) 
        m_gl2->Refresh(true);

        
    if (m_datasetHelper->m_isRulerToolActive){
        wxString sbString1 = wxString::Format( wxT("%4.1fmm (%2.1fmm)" ), m_datasetHelper->m_rulerFullLength, m_datasetHelper->m_rulerPartialLength );
        m_toolBar->m_txtRuler->SetValue(sbString1);    
    } 
}

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

void MainFrame::OnActivateListItem( wxListEvent& event )
{
    int l_item = event.GetIndex();
    m_treeWidget->UnselectAll();
    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item );
    m_lastSelectedFNObject = l_info;
    m_lastSelectedListItem = l_item;
    int l_col = m_listCtrl->getColActivated();
    switch( l_col )
    {        
        case 10:
            if (l_info->toggleShow())
                m_listCtrl->SetItem( l_item, 0, wxT( "" ), 0 );
            else
                m_listCtrl->SetItem( l_item, 0, wxT( "" ), 1 );            
            break;
        case 11:
            if( ! l_info->toggleShowFS())
                m_listCtrl->SetItem( l_item, 1, l_info->getName().BeforeFirst( '.' ) + wxT( "*" ) );
            else
                m_listCtrl->SetItem( l_item, 1, l_info->getName().BeforeFirst( '.' ) );
            break;
        case 13:
             deleteListItem();
            break;
        default:
            return;
            break;
    }
    refreshAllGLWidgets();
}

void MainFrame::OnToggleShowFS( wxEvent& WXUNUSED(event) )
{
    if (m_currentFNObject != NULL && m_currentListItem != -1)
    {
        if( ! ((DatasetInfo*)m_currentFNObject)->toggleShowFS())
            m_listCtrl->SetItem( m_currentListItem, 1, ((DatasetInfo*)m_currentFNObject)->getName().BeforeFirst( '.' ) + wxT( "*" ) );
        else
            m_listCtrl->SetItem( m_currentListItem, 1, ((DatasetInfo*)m_currentFNObject)->getName().BeforeFirst( '.' ) );
        refreshAllGLWidgets();
    }
}

void MainFrame::OnDeleteListItem( wxEvent& WXUNUSED(event) )
{
    deleteListItem();
}

void MainFrame::deleteListItem()
{
    if (m_currentFNObject != NULL && m_currentListItem != -1)
    {       
        long tmp = m_currentListItem;
        if (((DatasetInfo*)m_listCtrl->GetItemData( m_currentListItem))->getType() == FIBERS)
        {            
            m_datasetHelper->m_selBoxChanged = true;
        }
        else if (((DatasetInfo*)m_listCtrl->GetItemData( m_currentListItem))->getType() == SURFACE)
        {
            m_datasetHelper->deleteAllPoints();
        }
        deleteFNObject();
        m_listCtrl->DeleteItem( tmp );
        m_datasetHelper->updateLoadStatus();
        refreshAllGLWidgets();
    }
}

void MainFrame::OnSelectListItem( wxListEvent& event )
{
    int l_item = event.GetIndex();
    m_treeWidget->UnselectAll();
    DatasetInfo *l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item) ;
    int l_col = m_listCtrl->getColClicked();
    if (l_col == 12 && l_info->getType() >= MESH)
    {        
        if( ! l_info->toggleUseTex())
        {
            m_listCtrl->SetItem( l_item, 2, wxT( "(" ) + wxString::Format( wxT( "%.2f" ), l_info->getThreshold() * l_info->getOldMax() ) + wxT( ")" ) );
        }
        else
        {
            m_listCtrl->SetItem( l_item,2,wxString::Format( wxT( "%.2f" ), l_info->getThreshold() * l_info->getOldMax() ) );
        }            
    }
    m_lastSelectedFNObject = l_info;
    m_lastSelectedListItem = l_item;
    refreshAllGLWidgets();
}

void MainFrame::OnListItemUp(wxCommandEvent& WXUNUSED(event))
{
    m_listCtrl->moveItemUp(m_currentListItem);
    m_listCtrl->EnsureVisible(m_currentListItem);   
    refreshAllGLWidgets();
}

void MainFrame::OnListItemDown( wxCommandEvent& WXUNUSED(event) )
{
    m_listCtrl->moveItemDown(m_currentListItem);
    m_listCtrl->EnsureVisible(m_currentListItem);
    refreshAllGLWidgets();
}

void MainFrame::OnListMenuName( wxCommandEvent&  WXUNUSED(event) )
{
    long l_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
        return;
    DatasetInfo *l_info = (DatasetInfo*)m_listCtrl->GetItemData( l_item) ;

    if( ! l_info->toggleShowFS() )
        m_listCtrl->SetItem( l_item, 1, l_info->getName().BeforeFirst( '.' ) + wxT( "*" ) );
    else
        m_listCtrl->SetItem( l_item, 1, l_info->getName().BeforeFirst( '.' ) );

}

void MainFrame::OnListMenuThreshold( wxCommandEvent&  WXUNUSED(event) )
{
    if( m_currentFNObject == NULL && m_currentListItem != -1)
        return;
    DatasetInfo* l_info = (DatasetInfo*)m_currentFNObject;
    if( l_info->getType() >= MESH )
    {
        if( ! l_info->toggleUseTex() )
            m_listCtrl->SetItem( m_currentListItem,
                                 2,
                                 wxT( "(" ) + wxString::Format( wxT( "%.2f" ),
                                 l_info->getThreshold() * l_info->getOldMax()) + wxT( ")" ) );
        else
            m_listCtrl->SetItem( m_currentListItem,
                                 2,
                                 wxString::Format( wxT( "%.2f" ),
                                 l_info->getThreshold() * l_info->getOldMax() ) );
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the show button after a right 
// click in the item list is clicked.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnListMenuShow( wxCommandEvent&  WXUNUSED(event) )
{
    if( m_currentFNObject == NULL && m_currentListItem != -1)
        return;

    DatasetInfo* l_info = (DatasetInfo*)m_listCtrl->GetItemData( m_currentListItem );
    if( l_info->toggleShow() )
        m_listCtrl->SetItem( m_currentListItem, 0, wxT( "" ), 0 );
    else
        m_listCtrl->SetItem( m_currentListItem, 0, wxT( "" ), 1 );

    refreshAllGLWidgets();
}

void MainFrame::OnToggleShowProperties( wxCommandEvent& event )
{
    m_isDisplayProperties = !m_isDisplayProperties; 
    DisplayPropertiesSheet();
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

//////////////////////////////////////////////////////////////////////////
// This function will be called when the distance coloring option is
// selected when right-clicking on a fiber.
//////////////////////////////////////////////////////////////////////////
void MainFrame::OnListMenuDistance( wxCommandEvent& WXUNUSED(event))
{
    m_datasetHelper->m_fiberColorationMode = DISTANCE_COLOR;
    ColorFibers();
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when the minimum distance coloring option is
// selected when right-clicking on a fiber.
//////////////////////////////////////////////////////////////////////////
void MainFrame::OnListMenuMinDistance( wxCommandEvent& WXUNUSED(event))
{
    m_datasetHelper->m_fiberColorationMode = MINDISTANCE_COLOR;
    ColorFibers();
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
    deleteTreeItem();

}

void MainFrame::deleteTreeItem()
{
    if (m_currentFNObject != NULL)
    {   
        wxTreeItemId l_treeId = m_treeWidget->GetSelection();
        if (!l_treeId.IsOk())
        {
            return;
        }
        int l_selected = treeSelected( l_treeId );  
        if (l_selected == CHILD_OBJECT)
        {
            ((SelectionObject*) ((m_treeWidget->GetItemData(m_treeWidget->GetItemParent(l_treeId)))))->setIsDirty(true);
        }
        if (l_selected == CHILD_OBJECT || l_selected == MASTER_OBJECT || l_selected == POINT_DATASET)
        {  
            deleteFNObject();
            m_treeWidget->Delete(l_treeId);
            if (m_datasetHelper->m_lastSelectedObject != NULL)
            {
                m_datasetHelper->m_selBoxChanged = true;
            }
            m_datasetHelper->m_lastSelectedObject = NULL;
            m_datasetHelper->m_lastSelectedPoint = NULL;
        }   
        m_datasetHelper->m_selBoxChanged = true;
    }
    refreshAllGLWidgets();
}

void MainFrame::OnFibersFilter( wxCommandEvent& event)
{
    Fibers* data;
    m_datasetHelper->getFiberDataset(data);
    data->updateFibersFilters();
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the selection changed event in the item tree is triggered.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnSelectTreeItem( wxTreeEvent& WXUNUSED(event) )
{
    wxTreeItemId l_treeId = m_treeWidget->GetSelection();
    if (!l_treeId.IsOk())
    {
        return;
    }
    int l_selected = treeSelected( l_treeId );
    SelectionObject* l_selectionObject;
    SplinePoint* l_selectedPoint;

    switch( l_selected )
    {
        case MASTER_OBJECT:
        case CHILD_OBJECT:
            if( m_datasetHelper->m_lastSelectedObject )
                m_datasetHelper->m_lastSelectedObject->unselect();
            if( m_datasetHelper->m_lastSelectedPoint )
            {
                m_datasetHelper->m_lastSelectedPoint->unselect();
                m_datasetHelper->m_lastSelectedPoint = NULL;
            }

            l_selectionObject = (SelectionObject*)( m_treeWidget->GetItemData( l_treeId ) );
            m_datasetHelper->m_lastSelectedObject = l_selectionObject;
            m_datasetHelper->m_lastSelectedObject->select( false );
            m_lastSelectedFNObject = l_selectionObject;
            m_lastSelectedListItem = -1;
            break;

        case POINT_DATASET:
            if( m_datasetHelper->m_lastSelectedPoint )
                m_datasetHelper->m_lastSelectedPoint->unselect();
            if( m_datasetHelper->m_lastSelectedObject )
            {
                m_datasetHelper->m_lastSelectedObject->unselect();
                m_datasetHelper->m_lastSelectedObject = NULL;
            }

            l_selectedPoint = (SplinePoint*)( m_treeWidget->GetItemData( l_treeId ) );
            m_datasetHelper->m_lastSelectedPoint = l_selectedPoint;
            m_datasetHelper->m_lastSelectedPoint->select( false );
            m_lastSelectedFNObject = l_selectedPoint;
            m_lastSelectedListItem = -1;
            break;

        default:
            if( m_datasetHelper->m_lastSelectedPoint )
            {
                m_datasetHelper->m_lastSelectedPoint->unselect();
                m_datasetHelper->m_lastSelectedPoint = NULL;
            }
            if( m_datasetHelper->m_lastSelectedObject )
            {
                m_datasetHelper->m_lastSelectedObject->unselect();
                m_datasetHelper->m_lastSelectedObject = NULL;
            }
            break;
    }    
#ifdef __WXMSW__
    if (m_currentListItem != -1)
    {       
        m_listCtrl->SetItemState(m_currentListItem,0,wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);  
    }
#endif
    refreshAllGLWidgets();
}

void MainFrame::OnRightClickTreeItem( wxTreeEvent& event )
{

}

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

void MainFrame::OnTreeChange()
{
	m_datasetHelper->m_selBoxChanged = true;
	refreshAllGLWidgets();
}

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
 ****************************************************************************************************/

///////////////////////////////////////////////////////////////////////////
// Moves all boundary points in one direction.
///////////////////////////////////////////////////////////////////////////
void MainFrame::OnMoveBoundaryPointsLeft( wxCommandEvent& WXUNUSED(event) )
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
void MainFrame::OnMoveBoundaryPointsRight(wxCommandEvent& WXUNUSED(event))
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

void MainFrame::OnMouseEvent( wxMouseEvent& WXUNUSED(event) )
{
    //this->Refresh();
}

void MainFrame::updateMenus()
{
    m_menuBar->updateMenuBar(this);
    m_toolBar->updateToolBar(this); 
}

void MainFrame::OnTimerEvent( wxTimerEvent& WXUNUSED(event) )
{    
    refreshAllGLWidgets();
    m_datasetHelper->increaseAnimationStep();    
}
