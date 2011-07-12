/////////////////////////////////////////////////////////////////////////////
// Name:            mainFrame.cpp
// Author:          ---
// Creation Date:   ---
//
// Description: This is the implementation file for mainFrame class.
//
// Last modifications:
//      by : ggirard - 06-2011
/////////////////////////////////////////////////////////////////////////////


#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include <wx/colordlg.h>
#include <wx/filedlg.h>
#include <wx/statbmp.h>
#include <wx/vscroll.h>

#include "mainFrame.h"
#include "propertiesWindow.h"
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
EVT_SIZE(                                                   MainFrame::OnSize              )
EVT_MOUSE_EVENTS(                                           MainFrame::OnMouseEvent        )
EVT_CLOSE(                                                  MainFrame::OnClose              )

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
    m_lastSelectedSceneObject = NULL;
    m_lastSelectedListItem = -1;
    m_currentSceneObject = NULL;
    m_currentListItem = -1;   
    m_currentSizer = NULL;
    m_xSlider  = new wxSlider( this, ID_X_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( 175, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_ySlider  = new wxSlider( this, ID_Y_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( 175, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_zSlider  = new wxSlider( this, ID_Z_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( 175, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );

  
    m_listCtrl   = new MyListCtrl( this, ID_LIST_CTRL, wxDefaultPosition, wxSize( 308, 250 ), wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_NO_HEADER );
    m_listCtrl->SetMaxSize( wxSize( 308, 250 ) );
    m_listCtrl->SetMinSize( wxSize( 308, 250 ) );

    m_treeWidget = new MyTreeCtrl( this, ID_TREE_CTRL, wxDefaultPosition,wxSize( 308, -1 ), wxTR_HAS_BUTTONS | wxTR_SINGLE | wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS );
    m_treeWidget->SetMaxSize( wxSize( 308, -1 ) );
    m_treeWidget->SetMinSize( wxSize( 308, 100 ) );

    wxImageList* imageList = new wxImageList( 16, 16 );

    imageList->Add( ( wxImage( MyApp::respath +_T( "icons/eyes.png"   ), wxBITMAP_TYPE_PNG ) ) );
    imageList->Add( ( wxImage( MyApp::respath +_T( "icons/delete.png" ), wxBITMAP_TYPE_PNG ) ) );

    m_listCtrl->AssignImageList(imageList, wxIMAGE_LIST_SMALL);

    cout << m_listCtrl->GetColumnCount() << endl;
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
    m_gl0 = new MainCanvas( m_datasetHelper,       AXIAL, this, ID_GL_NAV_X, wxDefaultPosition, wxSize( 175, 175 ), 0, _T( "NavGLCanvasX" ), gl_attrib, m_mainGL );
    m_gl1 = new MainCanvas( m_datasetHelper,     CORONAL, this, ID_GL_NAV_Y, wxDefaultPosition, wxSize( 175, 175 ), 0, _T( "NavGLCanvasY" ), gl_attrib, m_mainGL );    
    m_gl2 = new MainCanvas( m_datasetHelper,    SAGITTAL, this, ID_GL_NAV_Z, wxDefaultPosition, wxSize( 175, 175 ), 0, _T( "NavGLCanvasZ" ), gl_attrib, m_mainGL );
#else
    m_gl0 = new MainCanvas( m_datasetHelper, axial,    m_topNavWindow,    ID_GL_NAV_X, wxDefaultPosition, wxSize( 175, 175 ), 0, _T( "NavGLCanvasX" ), gl_attrib, m_mainGL->GetContext() );
    m_gl1 = new MainCanvas( m_datasetHelper, coronal,  m_middleNavWindow, ID_GL_NAV_Y, wxDefaultPosition, wxSize( 175, 175 ), 0, _T( "NavGLCanvasY" ), gl_attrib, m_mainGL->GetContext() );
    m_gl2 = new MainCanvas( m_datasetHelper, sagittal, m_bottomNavWindow, ID_GL_NAV_Z, wxDefaultPosition, wxSize( 175, 175 ), 0, _T( "NavGLCanvasZ" ), gl_attrib, m_mainGL->GetContext() );
#endif

    m_gl0->SetMaxSize( wxSize( 175, 175 ) );
    m_gl1->SetMaxSize( wxSize( 175, 175 ) );
    m_gl2->SetMaxSize( wxSize( 175, 175 ) );

#ifndef __WXMAC__
    m_datasetHelper->m_theScene->setMainGLContext( new wxGLContext( m_mainGL ) );
#else
    m_datasetHelper->m_theScene->setMainGLContext( m_mainGL->GetContext() );
#endif

    //m_propertiesPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(242,-1));
    //wxScrolledWindow *m_propertiesWindow = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxSize(242,-1), wxVSCROLL);
    m_mainSizer         = new wxBoxSizer( wxHORIZONTAL ); // Contains everything in the UI.
    m_leftMainSizer     = new wxBoxSizer( wxVERTICAL   ); // Contains the navSizer adn the objectsizer.
    m_navSizer          = new wxBoxSizer( wxHORIZONTAL ); // Contains the 3 navigation windows with there respectiv sliders.
    m_listSizer         = new wxBoxSizer( wxVERTICAL   ); // Contains the list and the tree
    m_objectSizer       = new wxBoxSizer( wxHORIZONTAL ); // Contains the listSizer and the propertiesSizer

    wxBoxSizer *l_xSizer= new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *l_ySizer= new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *l_zSizer= new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *l_propSizer = new wxBoxSizer( wxVERTICAL );
    
    m_propertiesWindow = new PropertiesWindow(this, wxID_ANY, wxDefaultPosition, wxSize(220,350)); // Contains Scene Objects properties
    
    m_propertiesWindow->SetScrollbars( 10, 10, 50, 50 );
    m_propertiesWindow->EnableScrolling(false,true);

    l_zSizer->Add( m_gl0,     1, wxALL | wxFIXED_MINSIZE, 2 );
    l_zSizer->Add( m_zSlider, 0, wxALL,                   2 );
    l_ySizer->Add( m_gl1,     1, wxALL | wxFIXED_MINSIZE, 2 );
    l_ySizer->Add( m_ySlider, 0, wxALL,                   2 );
    l_xSizer->Add( m_gl2,     1, wxALL | wxFIXED_MINSIZE, 2 );
    l_xSizer->Add( m_xSlider, 0, wxALL ,                  2 );

    m_navSizer->Add( l_zSizer, 0, wxALL | wxFIXED_MINSIZE, 1);
    m_navSizer->Add( l_ySizer, 0, wxALL | wxFIXED_MINSIZE, 1);
    m_navSizer->Add( l_xSizer, 0, wxALL | wxFIXED_MINSIZE, 1);
    m_navSizer->SetMinSize( wxSize(520,15));
    
    m_listSizer->Add( m_listCtrl,    0, wxALL | wxEXPAND,  1 );
    m_listSizer->Add( m_treeWidget,   1, wxALL | wxEXPAND,  1 );
    
    l_propSizer->Add(m_propertiesWindow, 0, wxALL | wxEXPAND, 0);

    m_objectSizer->Add(m_listSizer, 1, wxALL | wxEXPAND, 0);
    m_objectSizer->Add(l_propSizer, 0, wxALL | wxEXPAND, 0);
    wxBoxSizer *l_spaceSizer = new wxBoxSizer(wxVERTICAL);
    l_spaceSizer->SetMinSize(wxSize(15,-1));
    m_objectSizer->Add(l_spaceSizer, 0, wxALL | wxFIXED_MINSIZE, 0);

    l_propSizer->Layout();
    
    m_objectSizer->SetMinSize( wxSize(520,15));

    m_leftMainSizer->Add( m_navSizer,  0, wxALL | wxFIXED_MINSIZE, 0 );
    m_leftMainSizer->Add( m_objectSizer,  1, wxALL | wxEXPAND, 0 );
    m_leftMainSizer->SetMinSize( wxSize(520,15));
    
    m_mainSizer->Add( m_leftMainSizer,  0,  wxALL | wxEXPAND, 0 );
    m_mainSizer->Add( m_mainGL, 1, wxEXPAND | wxALL, 2 );

    m_propertiesWindow->Fit();
    this->SetBackgroundColour(*wxLIGHT_GREY);
    this->SetSizer( m_mainSizer );
    m_mainSizer->SetSizeHints( this );
    SetAutoLayout(true);

    m_timer = new wxTimer( this );
    m_timer->Start( 100 );

    m_menuBar = new MenuBar();
    m_toolBar = new ToolBar(this);
    m_toolBar->Realize();
    m_menuBar->initMenuBar(this);
    m_toolBar->initToolBar(this);

    this->SetMenuBar(m_menuBar);
    this->SetToolBar(m_toolBar);
    updateMenus();
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
    if( m_currentSceneObject != NULL && m_currentListItem != -1 )
    {
        if( ((DatasetInfo*)m_currentSceneObject)->getType() < MESH )
        {
            Anatomy* l_anatomy = (Anatomy*)m_currentSceneObject;

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

void MainFrame::OnClose( wxCloseEvent& event )
{
    this->m_timer->Stop();
    this->Destroy();
}

void MainFrame::OnSaveSurface( wxCommandEvent& WXUNUSED(event) )
{
    if( m_currentSceneObject != NULL && m_currentListItem != -1 )
    {
        if( ((DatasetInfo*)m_currentSceneObject)->getType() == SURFACE )
        {
            Surface *l_surface = (Surface*)m_currentSceneObject;

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
        else if( ((DatasetInfo*)m_currentSceneObject)->getType() == ISO_SURFACE )
        {
            CIsoSurface *l_surface = (CIsoSurface*)m_currentSceneObject;

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
void MainFrame::OnMenuLock( wxCommandEvent& WXUNUSED(event) )
{
    m_mainGL->m_isSlizesLocked = !m_mainGL->m_isSlizesLocked;
}
void MainFrame::OnSceneLock( wxCommandEvent& WXUNUSED(event) )
{
    m_mainGL->m_isSceneLocked = !m_mainGL->m_isSceneLocked;
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

void MainFrame::OnToggleDrawPointsMode( wxCommandEvent& event )
{
    if( !m_datasetHelper->m_theScene )
        return;
    m_datasetHelper->togglePointMode();
    refreshAllGLWidgets();
}

void MainFrame::OnMoveBoundaryPointsLeft( wxCommandEvent& WXUNUSED(event) )
{
    moveBoundaryPoints(5);
}


void MainFrame::OnMoveBoundaryPointsRight(wxCommandEvent& WXUNUSED(event))
{
    moveBoundaryPoints(-5);
}

///////////////////////////////////////////////////////////////////////////
// Moves all boundary points left or right
///////////////////////////////////////////////////////////////////////////
void MainFrame::moveBoundaryPoints(int i_value)
{
    wxTreeItemId l_id, l_childid;
    wxTreeItemIdValue l_cookie = 0;
    l_id = m_treeWidget->GetFirstChild( m_tPointId, l_cookie );
    while( l_id.IsOk() )
    {
        SplinePoint* l_point = (SplinePoint*)( m_treeWidget->GetItemData( l_id ) );
        if( l_point->isBoundary())
            l_point->setX( l_point->X() + i_value );

        l_id = m_treeWidget->GetNextChild( m_tPointId, l_cookie );
    }

    m_datasetHelper->m_surfaceIsDirty = true;
    refreshAllGLWidgets();
}


void MainFrame::deleteSceneObject()
{
    if (m_currentSizer!=NULL)
    {
        m_propertiesWindow->GetSizer()->Hide(m_currentSizer, true);
        m_propertiesWindow->GetSizer()->Detach(m_currentSizer);        
        m_currentSizer = NULL;
    }
    //delete m_currentFNObject;
    m_currentSceneObject = NULL;
    m_lastSelectedSceneObject = NULL;
    m_currentListItem = -1;
    m_lastSelectedListItem = -1;
    m_propertiesWindow->GetSizer()->Layout();
    doOnSize();
}

void MainFrame::DisplayPropertiesSheet()
{   
    if (m_lastSelectedSceneObject == NULL && m_currentSceneObject == NULL)
    {
        if (m_currentSizer != NULL)
        {
           m_propertiesWindow->GetSizer()->Hide(m_currentSizer);
        }
    }
    else
    {
        if (m_lastSelectedSceneObject != NULL)
        {       
            if (m_currentSizer != NULL )
            {
                m_propertiesWindow->GetSizer()->Hide(m_currentSizer);                 
            }
            if (!m_lastSelectedSceneObject->getProprietiesSizer())
            {
                m_lastSelectedSceneObject->createPropertiesSizer(m_propertiesWindow);
            }   
            m_currentSizer = m_lastSelectedSceneObject->getProprietiesSizer();
            
            m_currentSceneObject = m_lastSelectedSceneObject;
            m_currentListItem = m_lastSelectedListItem;
            if (!m_propertiesWindow->GetSizer()->Show( m_currentSizer, true, true ))
            {
                m_propertiesWindow->GetSizer()->Add(m_currentSizer, 0, wxLeft | wxFIXED_MINSIZE, 0 );
                m_propertiesWindow->GetSizer()->Show( m_currentSizer, true, true );                
            }             
            doOnSize();            
        }        
        m_currentSceneObject->updatePropertiesSizer();
        m_lastSelectedSceneObject = NULL;
    }     
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
    m_datasetHelper->m_isBoxCreated = true;
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


void MainFrame::OnToggleDrawVectors( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_datasetHelper->m_theScene )
        return;

    m_datasetHelper->m_drawVectors = ! m_datasetHelper->m_drawVectors;
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
    if( m_currentSceneObject != NULL && m_currentListItem != -1 )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_currentSceneObject;
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

    refreshAllGLWidgets();
    this->Update(); 
}

void MainFrame::refreshAllGLWidgets()
{
    updateStatusBar();
    updateMenus();

    refreshViews();   
    if (m_datasetHelper->m_isRulerToolActive){
        wxString sbString1 = wxString::Format( wxT("%4.1fmm (%2.1fmm)" ), m_datasetHelper->m_rulerFullLength, m_datasetHelper->m_rulerPartialLength );
        m_toolBar->m_txtRuler->SetValue(sbString1);    
    }     
}

void MainFrame::refreshViews()
{
    m_propertiesWindow->Fit();
    m_propertiesWindow->AdjustScrollbars();

    m_propertiesWindow->Layout();
    DisplayPropertiesSheet();
    if ( m_mainGL ) 
        m_mainGL->Refresh(true);
    if ( m_gl0 ) 
        m_gl0->Refresh(true);
    if ( m_gl1 ) 
        m_gl1->Refresh(true);
    if ( m_gl2 ) 
        m_gl2->Refresh(true);
    
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
    m_lastSelectedSceneObject = l_info;
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

void MainFrame::deleteListItem()
{
    if (m_currentSceneObject != NULL && m_currentListItem != -1)
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
        if (((DatasetInfo*)m_listCtrl->GetItemData( m_currentListItem))->getName() == _T( "(Object)" ))
        {            
            m_datasetHelper->m_isObjCreated = false;
            m_datasetHelper->m_isObjfilled = false;
    
        }
        if (((DatasetInfo*)m_listCtrl->GetItemData( m_currentListItem))->getName() == _T( "(Background)" ))
        {            
            m_datasetHelper->m_isBckCreated = false;
            m_datasetHelper->m_isBckfilled = false;
        }
        deleteSceneObject();
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
    m_lastSelectedSceneObject = l_info;
    m_lastSelectedListItem = l_item;
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
    if (m_currentSceneObject != NULL)
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
            deleteSceneObject();
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
            m_lastSelectedSceneObject = l_selectionObject;
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
            m_lastSelectedSceneObject = l_selectedPoint;
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
void MainFrame::OnRotateZ( wxCommandEvent& event )
{
    m_datasetHelper->m_theScene->m_isRotateZ = !m_datasetHelper->m_theScene->m_isRotateZ; 
    setTimerSpeed();
}

void MainFrame::OnRotateY( wxCommandEvent& event )
{
    m_datasetHelper->m_theScene->m_isRotateY = !m_datasetHelper->m_theScene->m_isRotateY; 
    setTimerSpeed();
}

void MainFrame::OnRotateX( wxCommandEvent& event )
{
    m_datasetHelper->m_theScene->m_isRotateX = !m_datasetHelper->m_theScene->m_isRotateX; 
    setTimerSpeed();
}

void MainFrame::OnNavigateAxial( wxCommandEvent& event )
{
    m_datasetHelper->m_theScene->m_isNavAxial = !m_datasetHelper->m_theScene->m_isNavAxial;
    setTimerSpeed();
}

void MainFrame::OnNavigateSagital( wxCommandEvent& event )
{
    m_datasetHelper->m_theScene->m_isNavSagital = !m_datasetHelper->m_theScene->m_isNavSagital;    
    setTimerSpeed();  
}

void MainFrame::OnNavigateCoronal( wxCommandEvent& event )
{
    m_datasetHelper->m_theScene->m_isNavCoronal = !m_datasetHelper->m_theScene->m_isNavCoronal;
    setTimerSpeed();
}

void MainFrame::setTimerSpeed()
{
    m_timer->Stop();
    if(m_datasetHelper->m_theScene->m_isNavCoronal || m_datasetHelper->m_theScene->m_isNavAxial || m_datasetHelper->m_theScene->m_isNavSagital || m_datasetHelper->m_theScene->m_isRotateZ
        || m_datasetHelper->m_theScene->m_isRotateY || m_datasetHelper->m_theScene->m_isRotateX)
    {        
        m_timer->Start( 50 );
    }
    else
    {
        m_timer->Start( 100 );
    }
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

void MainFrame::OnSize( wxSizeEvent& WXUNUSED(event) )
{
    doOnSize();
}

void MainFrame::doOnSize()
{
    wxSize l_clientSize = this->GetClientSize();    

    if( GetSizer() )
        GetSizer()->SetDimension(0, 0, l_clientSize.x, l_clientSize.y );

    if( m_mainGL )
        m_mainGL->changeOrthoSize();

    m_propertiesWindow->SetMinSize(wxSize(220, l_clientSize.y - 236));
    m_propertiesWindow->GetSizer()->SetDimension(0,0,220, l_clientSize.y - 236);
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
            float x = ( (float)l_pos.x / NAV_GL_SIZE * m_datasetHelper->m_columns );
            float y = ( (float)( NAV_GL_SIZE - l_pos.y ) / NAV_GL_SIZE * m_datasetHelper->m_rows );

            m_xSlider->SetValue( (int)( x ) );
            m_ySlider->SetValue( (int)( y ) );
            break;
        }
        case CORONAL:
        {
            l_pos = m_gl1->getMousePos();
            float x = ( (float)l_pos.x / NAV_GL_SIZE ) * m_datasetHelper->m_columns;
            float y = ( (float)( NAV_GL_SIZE - l_pos.y ) / NAV_GL_SIZE * m_datasetHelper->m_frames );

            m_xSlider->SetValue( (int)( x ) );
            m_zSlider->SetValue( (int)( y ) );
            break;
        }
        case SAGITTAL:
        {
            l_pos = m_gl2->getMousePos();
            float x = ( (float)( NAV_GL_SIZE - l_pos.x ) / NAV_GL_SIZE ) * m_datasetHelper->m_rows;
            float y = ( (float)( NAV_GL_SIZE - l_pos.y ) / NAV_GL_SIZE * m_datasetHelper->m_frames );

            m_ySlider->SetValue( (int)( x ) );
            m_zSlider->SetValue( (int)( y ) );
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
    //Rotate animation
    if(m_datasetHelper->m_theScene->m_isRotateZ)
        m_datasetHelper->m_theScene->m_rotAngleZ++;

    if(m_datasetHelper->m_theScene->m_isRotateY)
        m_datasetHelper->m_theScene->m_rotAngleY++;

    if(m_datasetHelper->m_theScene->m_isRotateX)
        m_datasetHelper->m_theScene->m_rotAngleX++;

    
    //Navigate through slizes sagital
    if(m_datasetHelper->m_theScene->m_isNavSagital) 
        m_datasetHelper->m_theScene->m_posSagital++;
    else
        m_datasetHelper->m_theScene->m_posSagital = m_datasetHelper->m_xSlize;
    

    //Navigate through slizes axial
    if(m_datasetHelper->m_theScene->m_isNavAxial) 
        m_datasetHelper->m_theScene->m_posAxial++;
    else
        m_datasetHelper->m_theScene->m_posAxial = m_datasetHelper->m_zSlize;


    //Navigate through slizes coronal
    if(m_datasetHelper->m_theScene->m_isNavCoronal) 
        m_datasetHelper->m_theScene->m_posCoronal++;
    else
        m_datasetHelper->m_theScene->m_posCoronal = m_datasetHelper->m_ySlize;


    refreshAllGLWidgets();
    refreshViews();
    m_datasetHelper->increaseAnimationStep();    
}
