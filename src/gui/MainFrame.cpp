/////////////////////////////////////////////////////////////////////////////
// Description: mainFrame class. Contains every elements of the GUI, and frame events
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/colordlg.h>
#include <wx/filedlg.h>
#include <wx/statbmp.h>
#include <wx/vscroll.h>

#include "MainFrame.h"
#include "PropertiesWindow.h"
#include "ToolBar.h"
#include "MenuBar.h"
#include "MainCanvas.h"
#include "SelectionBox.h"
#include "SelectionEllipsoid.h"
#include "../dataset/Anatomy.h"
#include "../dataset/Fibers.h"
#include "../dataset/FibersGroup.h"
#include "../dataset/ODFs.h"
#include "../dataset/SplinePoint.h"
#include "../dataset/Surface.h"
#include "../dataset/Tensors.h"
#include "../gfx/TheScene.h"
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
// List widget events
EVT_LIST_ITEM_ACTIVATED  ( ID_LIST_CTRL,                    MainFrame::onActivateListItem   )
EVT_LIST_ITEM_SELECTED   ( ID_LIST_CTRL,                    MainFrame::onSelectListItem     )

// Tree widget events
EVT_TREE_DELETE_ITEM     ( ID_TREE_CTRL,                    MainFrame::onDeleteTreeItem     )
EVT_TREE_SEL_CHANGED     ( ID_TREE_CTRL,                    MainFrame::onSelectTreeItem     )
EVT_TREE_ITEM_ACTIVATED  ( ID_TREE_CTRL,                    MainFrame::onActivateTreeItem   )
EVT_TREE_ITEM_RIGHT_CLICK( ID_TREE_CTRL,                    MainFrame::onRightClickTreeItem )
EVT_TREE_END_LABEL_EDIT  ( ID_TREE_CTRL,                    MainFrame::onTreeLabelEdit      )

//Interface events
EVT_SIZE(                                                   MainFrame::onSize               )
EVT_MOUSE_EVENTS(                                           MainFrame::onMouseEvent         )
EVT_CLOSE(                                                  MainFrame::onClose              )

// mouse click in one of the three navigation windows
EVT_COMMAND( ID_GL_NAV_X, wxEVT_NAVGL_EVENT,                MainFrame::onGLEvent            )
EVT_COMMAND( ID_GL_NAV_Y, wxEVT_NAVGL_EVENT,                MainFrame::onGLEvent            )
EVT_COMMAND( ID_GL_NAV_Z, wxEVT_NAVGL_EVENT,                MainFrame::onGLEvent            )
EVT_COMMAND( ID_GL_MAIN,  wxEVT_NAVGL_EVENT,                MainFrame::onGLEvent            )

// slider events
EVT_SLIDER( ID_X_SLIDER,                                    MainFrame::onSliderMoved        )
EVT_SLIDER( ID_Y_SLIDER,                                    MainFrame::onSliderMoved        )
EVT_SLIDER( ID_Z_SLIDER,                                    MainFrame::onSliderMoved        )

// KDTREE thread finished
EVT_MENU( KDTREE_EVENT,                                     MainFrame::onKdTreeThreadFinished )

EVT_TIMER( -1,                                              MainFrame::onTimerEvent )

END_EVENT_TABLE()

MainFrame::MainFrame(wxWindow           *i_parent, 
                     const wxWindowID   i_id, 
                     const wxString     &i_title, 
                     const wxPoint      &i_pos, 
                     const wxSize       &i_size, 
                     const long         i_style) :
    wxFrame( i_parent, i_id, i_title, i_pos, i_size, i_style )
{
    wxImage::AddHandler(new wxPNGHandler);
    m_pLastSelectedSceneObject = NULL;
    m_lastSelectedListItem = -1;
    m_pCurrentSceneObject = NULL;
    m_currentListItem = -1;   
    m_pCurrentSizer = NULL;
    m_pXSlider  = new wxSlider( this, ID_X_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( 175, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pYSlider  = new wxSlider( this, ID_Y_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( 175, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pZSlider  = new wxSlider( this, ID_Z_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( 175, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
      
    m_pListCtrl   = new MyListCtrl( this, ID_LIST_CTRL, wxDefaultPosition, wxSize( 308, 250 ), wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_NO_HEADER );
    m_pListCtrl->SetMaxSize( wxSize( 308, 250 ) );
    m_pListCtrl->SetMinSize( wxSize( 308, 250 ) );

    m_pTreeWidget = new MyTreeCtrl( this, ID_TREE_CTRL, wxDefaultPosition,wxSize( 308, -1 ), wxTR_HAS_BUTTONS | wxTR_SINGLE | wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS );
    m_pTreeWidget->SetMaxSize( wxSize( 308, -1 ) );
    m_pTreeWidget->SetMinSize( wxSize( 308, 100 ) );

    wxImageList* imageList = new wxImageList( 16, 16 );

    imageList->Add( ( wxImage( MyApp::respath +_T( "icons/eyes.png"   ), wxBITMAP_TYPE_PNG ) ) );
    imageList->Add( ( wxImage( MyApp::respath +_T( "icons/delete.png" ), wxBITMAP_TYPE_PNG ) ) );

    m_pListCtrl->AssignImageList(imageList, wxIMAGE_LIST_SMALL);

    wxListItem itemCol;
    itemCol.SetText( wxT( "" ) );
    m_pListCtrl->InsertColumn( 0, itemCol );

    itemCol.SetText( wxT( "Name" ) );
    itemCol.SetAlign( wxLIST_FORMAT_CENTRE );
    m_pListCtrl->InsertColumn( 1, itemCol );

    itemCol.SetText( wxT( "Threshold" ) );
    itemCol.SetAlign( wxLIST_FORMAT_RIGHT );
    m_pListCtrl->InsertColumn( 2, itemCol) ;

    itemCol.SetText( wxT( "" ) );
    m_pListCtrl->InsertColumn( 3, itemCol );

    m_pListCtrl->SetColumnWidth( 0, 20  );
    m_pListCtrl->SetColumnWidth( 1, 194 );
    m_pListCtrl->SetColumnWidth( 2, 70  );
    m_pListCtrl->SetColumnWidth( 3, 20  );

    wxImageList* tImageList = new wxImageList( 16, 16 );

    tImageList->Add( wxImage( MyApp::respath + _T( "icons/delete.png" ), wxBITMAP_TYPE_PNG ) );
    tImageList->Add( wxImage( MyApp::respath + _T( "icons/eyes.png" ),   wxBITMAP_TYPE_PNG ) );

    m_pTreeWidget->AssignImageList( tImageList );

    m_tRootId  = m_pTreeWidget->AddRoot( wxT( "Scene" ), -1, -1, NULL );
    m_tPointId = m_pTreeWidget->AppendItem( m_tRootId, wxT( "points" ), -1, -1, NULL );
    m_tSelectionObjectsId = m_pTreeWidget->AppendItem( m_tRootId, wxT( "selection objects" ), -1, -1, NULL );

    m_pDatasetHelper = new DatasetHelper( this );

    /*
     * Set OpenGL attributes
     */
    m_pDatasetHelper->printDebug( _T( "Initializing OpenGL" ), 1 );
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
        m_pDatasetHelper->printDebug( _T( "don't have double buffer, disabling" ), 1 );
#ifdef __WXGTK__
        gl_attrib[9] = None;
#endif
        doubleBuffer = GL_FALSE;
    }
    m_pDatasetHelper->m_theScene = new TheScene( m_pDatasetHelper );

    m_pMainGL = new MainCanvas( m_pDatasetHelper, MAIN_VIEW, this, ID_GL_MAIN,  wxDefaultPosition, wxDefaultSize,      0, _T( "MainGLCanvas" ), gl_attrib );

#ifndef CTX
    m_pGL0 = new MainCanvas( m_pDatasetHelper,       AXIAL, this, ID_GL_NAV_X, wxDefaultPosition, wxSize( 175, 175 ), 0, _T( "NavGLCanvasX" ), gl_attrib, m_pMainGL );
    m_pGL1 = new MainCanvas( m_pDatasetHelper,     CORONAL, this, ID_GL_NAV_Y, wxDefaultPosition, wxSize( 175, 175 ), 0, _T( "NavGLCanvasY" ), gl_attrib, m_pMainGL );    
    m_pGL2 = new MainCanvas( m_pDatasetHelper,    SAGITTAL, this, ID_GL_NAV_Z, wxDefaultPosition, wxSize( 175, 175 ), 0, _T( "NavGLCanvasZ" ), gl_attrib, m_pMainGL );
#else
    m_pGL0 = new MainCanvas( m_pDatasetHelper, axial,    m_topNavWindow,    ID_GL_NAV_X, wxDefaultPosition, wxSize( 175, 175 ), 0, _T( "NavGLCanvasX" ), gl_attrib, m_pMainGL->GetContext() );
    m_pGL1 = new MainCanvas( m_pDatasetHelper, coronal,  m_middleNavWindow, ID_GL_NAV_Y, wxDefaultPosition, wxSize( 175, 175 ), 0, _T( "NavGLCanvasY" ), gl_attrib, m_pMainGL->GetContext() );
    m_pGL2 = new MainCanvas( m_pDatasetHelper, sagittal, m_bottomNavWindow, ID_GL_NAV_Z, wxDefaultPosition, wxSize( 175, 175 ), 0, _T( "NavGLCanvasZ" ), gl_attrib, m_pMainGL->GetContext() );
#endif

    m_pGL0->SetMaxSize( wxSize( 175, 175 ) );
    m_pGL1->SetMaxSize( wxSize( 175, 175 ) );
    m_pGL2->SetMaxSize( wxSize( 175, 175 ) );

#ifndef __WXMAC__
    m_pDatasetHelper->m_theScene->setMainGLContext( new wxGLContext( m_pMainGL ) );
#else
    m_pDatasetHelper->m_theScene->setMainGLContext( m_pMainGL->GetContext() );
#endif

    m_pMainSizer         = new wxBoxSizer( wxHORIZONTAL ); // Contains everything in the UI.
    m_pLeftMainSizer     = new wxBoxSizer( wxVERTICAL   ); // Contains the navSizer adn the objectsizer.
    m_pNavSizer          = new wxBoxSizer( wxHORIZONTAL ); // Contains the 3 navigation windows with there respectiv sliders.
    m_pListSizer         = new wxBoxSizer( wxVERTICAL   ); // Contains the list and the tree
    m_pObjectSizer       = new wxBoxSizer( wxHORIZONTAL ); // Contains the listSizer and the propertiesSizer

    wxBoxSizer *l_xSizer= new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *l_ySizer= new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *l_zSizer= new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *l_propSizer = new wxBoxSizer( wxVERTICAL );
    
    m_pPropertiesWindow = new PropertiesWindow(this, wxID_ANY, wxDefaultPosition, wxSize(220,350)); // Contains Scene Objects properties
    
    m_pPropertiesWindow->SetScrollbars( 10, 10, 50, 50 );
    m_pPropertiesWindow->EnableScrolling(false,true);

    l_zSizer->Add( m_pGL0,     1, wxALL | wxFIXED_MINSIZE, 2 );
    l_zSizer->Add( m_pZSlider, 0, wxALL,                   2 );
    l_ySizer->Add( m_pGL1,     1, wxALL | wxFIXED_MINSIZE, 2 );
    l_ySizer->Add( m_pYSlider, 0, wxALL,                   2 );
    l_xSizer->Add( m_pGL2,     1, wxALL | wxFIXED_MINSIZE, 2 );
    l_xSizer->Add( m_pXSlider, 0, wxALL ,                  2 );

    m_pNavSizer->Add( l_zSizer, 0, wxALL | wxFIXED_MINSIZE, 1);
    m_pNavSizer->Add( l_ySizer, 0, wxALL | wxFIXED_MINSIZE, 1);
    m_pNavSizer->Add( l_xSizer, 0, wxALL | wxFIXED_MINSIZE, 1);
    m_pNavSizer->SetMinSize( wxSize(520,15));
    
    m_pListSizer->Add( m_pListCtrl,    0, wxALL | wxEXPAND,  1 );
    m_pListSizer->Add( m_pTreeWidget,   1, wxALL | wxEXPAND,  1 );
    
    l_propSizer->Add(m_pPropertiesWindow, 0, wxALL | wxEXPAND, 0);

    m_pObjectSizer->Add(m_pListSizer, 1, wxALL | wxEXPAND, 0);
    m_pObjectSizer->Add(l_propSizer, 0, wxALL | wxEXPAND, 0);
    wxBoxSizer *l_spaceSizer = new wxBoxSizer(wxVERTICAL);
    l_spaceSizer->SetMinSize(wxSize(15,-1));
    m_pObjectSizer->Add(l_spaceSizer, 0, wxALL | wxFIXED_MINSIZE, 0);

    l_propSizer->Layout();
    
    m_pObjectSizer->SetMinSize( wxSize(520,15));

    m_pLeftMainSizer->Add( m_pNavSizer,  0, wxALL | wxFIXED_MINSIZE, 0 );
    m_pLeftMainSizer->Add( m_pObjectSizer,  1, wxALL | wxEXPAND, 0 );
    m_pLeftMainSizer->SetMinSize( wxSize(520,15));
    
    m_pMainSizer->Add( m_pLeftMainSizer,  0,  wxALL | wxEXPAND, 0 );
    m_pMainSizer->Add( m_pMainGL, 1, wxEXPAND | wxALL, 2 );

    m_pPropertiesWindow->Fit();
    this->SetBackgroundColour(*wxLIGHT_GREY);
    this->SetSizer( m_pMainSizer );
    m_pMainSizer->SetSizeHints( this );
    SetAutoLayout(true);

    m_pTimer = new wxTimer( this );
    m_pTimer->Start( 100 );

    m_pMenuBar = new MenuBar();
    m_pToolBar = new ToolBar(this);
    m_pToolBar->Realize();
    m_pMenuBar->initMenuBar(this);
    m_pToolBar->initToolBar(this);

    this->SetMenuBar(m_pMenuBar);
    this->SetToolBar(m_pToolBar);
    updateMenus();
    int widths[] = { 250, 250, -1 };
    CreateStatusBar( 2 );
    GetStatusBar()->SetFieldsCount( WXSIZEOF(widths), widths );
    GetStatusBar()->Show(); 
}

MainFrame::~MainFrame()
{
    m_pTimer->Stop();
    m_pDatasetHelper->printDebug( _T( "main frame destructor" ), 0 );
    m_pDatasetHelper->printDebug( _T( "timer stopped" ), 0 );
	if (m_pTimer != NULL)
    {
        delete m_pTimer;
    }
    if( m_pDatasetHelper != NULL)
    {
        delete m_pDatasetHelper;
    }
}

void MainFrame::onLoad( wxCommandEvent& WXUNUSED(event) )
{
    if( ! m_pDatasetHelper->load( 0 ) )
    {
        wxMessageBox( wxT( "ERROR\n" ) + m_pDatasetHelper->m_lastError, wxT( "" ), wxOK | wxICON_INFORMATION, NULL );
        GetStatusBar()->SetStatusText( wxT( "ERROR" ), 1 );
        GetStatusBar()->SetStatusText( m_pDatasetHelper->m_lastError, 2 );
        return;
    }
     refreshAllGLWidgets();
}
//////////////////////////////////////////////////////////////////////////
// This function will be called when someone click on the Load Datasets button.
//////////////////////////////////////////////////////////////////////////
void MainFrame::onLoadDatasets( wxCommandEvent& WXUNUSED(event) )
{
    loadIndex( 1 );
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when someone click on the Load Meshes button.
//////////////////////////////////////////////////////////////////////////
void MainFrame::onLoadMeshes( wxCommandEvent& WXUNUSED(event) )
{
    loadIndex( 2 );
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when someone click on the Load Fibers button.
//////////////////////////////////////////////////////////////////////////
void MainFrame::onLoadFibers( wxCommandEvent& WXUNUSED(event) )
{
    loadIndex( 5 );
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when someone click on the Load Tensors button.
//////////////////////////////////////////////////////////////////////////
void MainFrame::onLoadTensors( wxCommandEvent& WXUNUSED(event) )
{
    loadIndex( 8 );
}

//////////////////////////////////////////////////////////////////////////
// This function will be called when someone click on the Load ODFs button.
//////////////////////////////////////////////////////////////////////////
void MainFrame::onLoadODFs( wxCommandEvent& WXUNUSED(event) )
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
    if( ! m_pDatasetHelper->load( i_index ) )
    {
        wxMessageBox( wxT( "ERROR\n" ) + m_pDatasetHelper->m_lastError, wxT( "" ), wxOK | wxICON_INFORMATION, NULL );
        GetStatusBar()->SetStatusText( wxT( "ERROR" ), 1 );
        GetStatusBar()->SetStatusText( m_pDatasetHelper->m_lastError, 2 );
        return false;
    }
    else
    {
        m_pDatasetHelper->m_selBoxChanged = true;
    } 
    return true;
}

void MainFrame::onReloadShaders( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_scheduledReloadShaders = true;
    renewAllGLWidgets();
}

void MainFrame::onSave( wxCommandEvent& WXUNUSED(event) )
{
    wxString caption         = wxT( "Choose a file" );
    wxString wildcard        = wxT( "Scene files (*.scn)|*.scn|*.*|*.*" );
    wxString defaultDir      = wxEmptyString;
    wxString defaultFilename = wxEmptyString;
    wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
    dialog.SetFilterIndex( 0 );
    dialog.SetDirectory( m_pDatasetHelper->m_lastPath );

    if( m_pDatasetHelper->m_scnFileLoaded )
    {
        dialog.SetFilename( m_pDatasetHelper->m_scnFileName );
    }

    dialog.SetDirectory( m_pDatasetHelper->m_scenePath );

    if( dialog.ShowModal() == wxID_OK )
    {
        m_pDatasetHelper->m_scenePath = dialog.GetDirectory();
        m_pDatasetHelper->save(dialog.GetPath());
    }
}

void MainFrame::onSaveFibers( wxCommandEvent& WXUNUSED(event) )
{
    if( !m_pDatasetHelper->m_fibersLoaded )
    {
        return;
    }
 
    wxString caption         = wxT( "Choose a file" );
    wxString wildcard        = wxT( "VTK fiber files (*.fib)|*.fib|DMRI fiber files (*.fib)|*.fib|*.*|*.*" );
    wxString defaultDir      = wxEmptyString;
    wxString defaultFilename = wxEmptyString;
    wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
    dialog.SetFilterIndex( 0 );
    dialog.SetDirectory( m_pDatasetHelper->m_lastPath );

	if (m_pCurrentSceneObject != NULL && m_currentListItem != -1)
    {
		DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pCurrentSceneObject);
		if( dialog.ShowModal() == wxID_OK )
		{
			m_pDatasetHelper->m_lastPath = dialog.GetDirectory();
			printf("%d\n",dialog.GetFilterIndex());
			
			if( pDatasetInfo->getType() == FIBERS )
			{
				Fibers* l_fibers = NULL;
				m_pDatasetHelper->getSelectedFiberDataset( l_fibers );
				if( l_fibers )
				{
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
			else if( pDatasetInfo->getType() == FIBERSGROUP )
			{
				FibersGroup* l_fibersGroup = NULL;
				m_pDatasetHelper->getFibersGroupDataset( l_fibersGroup );
				
				if (dialog.GetFilterIndex()==1)
				{
					l_fibersGroup->saveDMRI( dialog.GetPath() );	
				}
				else
				{
					l_fibersGroup->save( dialog.GetPath() );
				}
			}
			
		}
	}
}


void MainFrame::onSaveDataset( wxCommandEvent& WXUNUSED(event) )
{
    if( m_pCurrentSceneObject != NULL && m_currentListItem != -1 )
    {
        if( ((DatasetInfo*)m_pCurrentSceneObject)->getType() < MESH )
        {
            Anatomy* l_anatomy = (Anatomy*)m_pCurrentSceneObject;

            wxString caption         = wxT( "Choose l_anatomy file" );
            wxString wildcard        = wxT( "nifti files (*.nii)|*.nii*|*.*|*.*" );
            wxString defaultDir      = wxEmptyString;
            wxString defaultFilename = wxEmptyString;
            wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
            dialog.SetFilterIndex( 0 );
            dialog.SetDirectory( m_pDatasetHelper->m_lastPath );

            if( dialog.ShowModal() == wxID_OK )
            {
                m_pDatasetHelper->m_lastPath = dialog.GetDirectory();
                l_anatomy->saveNifti( dialog.GetPath() );
            }
        }
    }
}

void MainFrame::onQuit( wxCommandEvent& WXUNUSED(event) )
{
    wxMessageDialog dialog( NULL, wxT( "Really Quit?" ), wxT( "Really Quit?" ), wxNO_DEFAULT | wxYES_NO | wxICON_INFORMATION );
    if( dialog.ShowModal() == wxID_YES )
    {              
        Close(true);
    }
}

void MainFrame::onClose( wxCloseEvent& event )
{
    this->m_pTimer->Stop();
    this->Destroy();
}

void MainFrame::onSaveSurface( wxCommandEvent& WXUNUSED(event) )
{
    if( m_pCurrentSceneObject != NULL && m_currentListItem != -1 )
    {
        if( ((DatasetInfo*)m_pCurrentSceneObject)->getType() == SURFACE )
        {
            Surface *l_surface = (Surface*)m_pCurrentSceneObject;

            wxString caption         = wxT( "Choose a file" );
            wxString wildcard        = wxT( "surfae files (*.vtk)|*.vtk" );
            wxString defaultDir      = wxEmptyString;
            wxString defaultFilename = wxEmptyString;
            wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
            dialog.SetFilterIndex( 0 );
            dialog.SetDirectory( m_pDatasetHelper->m_lastPath );
            if( dialog.ShowModal() == wxID_OK )
            {
                m_pDatasetHelper->m_lastPath = dialog.GetDirectory();
                l_surface->save( dialog.GetPath() );
            }
        }
        else if( ((DatasetInfo*)m_pCurrentSceneObject)->getType() == ISO_SURFACE )
        {
            CIsoSurface *l_surface = (CIsoSurface*)m_pCurrentSceneObject;

            wxString caption         = wxT( "Choose a file" );
            wxString wildcard        = wxT( "surfae files (*.vtk)|*.vtk" );
            wxString defaultDir      = wxEmptyString;
            wxString defaultFilename = wxEmptyString;
            wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
            dialog.SetFilterIndex( 0 );
            dialog.SetDirectory( m_pDatasetHelper->m_lastPath );
            if( dialog.ShowModal() == wxID_OK )
            {
                m_pDatasetHelper->m_lastPath = dialog.GetDirectory();
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

void MainFrame::onMenuViewReset( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_zoom  = 1;
    m_pDatasetHelper->m_xMove = 0;
    m_pDatasetHelper->m_yMove = 0;
    refreshAllGLWidgets();
}

void MainFrame::onMenuViewLeft( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_pDatasetHelper->m_transform );
    m_pDatasetHelper->m_transform.s.M00 =  0.0f;
    m_pDatasetHelper->m_transform.s.M11 =  0.0f;
    m_pDatasetHelper->m_transform.s.M22 =  0.0f;
    m_pDatasetHelper->m_transform.s.M20 = -1.0f;
    m_pDatasetHelper->m_transform.s.M01 = -1.0f;
    m_pDatasetHelper->m_transform.s.M12 =  1.0f;
    m_pMainGL->setRotation();
}

void MainFrame::onMenuViewRight( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_pDatasetHelper->m_transform );
    m_pDatasetHelper->m_transform.s.M00 = 0.0f;
    m_pDatasetHelper->m_transform.s.M11 = 0.0f;
    m_pDatasetHelper->m_transform.s.M22 = 0.0f;
    m_pDatasetHelper->m_transform.s.M20 = 1.0f;
    m_pDatasetHelper->m_transform.s.M01 = 1.0f;
    m_pDatasetHelper->m_transform.s.M12 = 1.0f;
    m_pMainGL->setRotation();
}

void MainFrame::onMenuViewTop( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_pDatasetHelper->m_transform );
    m_pMainGL->setRotation();
}

void MainFrame::onMenuViewBottom( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_pDatasetHelper->m_transform );
    m_pDatasetHelper->m_transform.s.M00 = -1.0f;
    m_pDatasetHelper->m_transform.s.M22 = -1.0f;
    m_pMainGL->setRotation();
}

void MainFrame::onMenuViewFront( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_pDatasetHelper->m_transform );
    m_pDatasetHelper->m_transform.s.M11 =  0.0f;
    m_pDatasetHelper->m_transform.s.M22 =  0.0f;
    m_pDatasetHelper->m_transform.s.M12 =  1.0f;
    m_pDatasetHelper->m_transform.s.M21 = -1.0f;
    m_pMainGL->setRotation();
}
void MainFrame::onMenuLock( wxCommandEvent& WXUNUSED(event) )
{
    m_pMainGL->m_isSlizesLocked = !m_pMainGL->m_isSlizesLocked;
}
void MainFrame::onSceneLock( wxCommandEvent& WXUNUSED(event) )
{
    m_pMainGL->m_isSceneLocked = !m_pMainGL->m_isSceneLocked;
}

void MainFrame::onMenuViewBack( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &m_pDatasetHelper->m_transform );
    m_pDatasetHelper->m_transform.s.M00 = -1.0f;
    m_pDatasetHelper->m_transform.s.M11 =  0.0f;
    m_pDatasetHelper->m_transform.s.M22 =  0.0f;
    m_pDatasetHelper->m_transform.s.M21 =  1.0f;
    m_pDatasetHelper->m_transform.s.M12 =  1.0f;
    m_pMainGL->setRotation();
}

void MainFrame::onMenuViewAxes( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_isShowAxes = !m_pDatasetHelper->m_isShowAxes;
}

void MainFrame::onMenuViewCrosshair( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_showCrosshair = !m_pDatasetHelper->m_showCrosshair;
    refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 * Menu VOI
 *
 ****************************************************************************************************/

void MainFrame::onToggleSelectionObjects( wxCommandEvent& WXUNUSED(event) )
{
    if( m_pDatasetHelper->m_theScene == NULL)
    {
        return;
    }

    // Get the selection object is selected.
    wxTreeItemId l_selectionObjectTreeId = m_pTreeWidget->GetSelection();

    if( treeSelected( l_selectionObjectTreeId ) == MASTER_OBJECT )
    {
        SelectionObject* l_selectionObject = (SelectionObject*)( m_pTreeWidget->GetItemData( l_selectionObjectTreeId ) );
        l_selectionObject->toggleIsActive();
        m_pTreeWidget->SetItemImage( l_selectionObjectTreeId, l_selectionObject->getIcon());
        l_selectionObject->setIsDirty( true );

        int l_childSelectionObject = m_pTreeWidget->GetChildrenCount( l_selectionObjectTreeId );
        wxTreeItemIdValue childcookie = 0;
        for( int i = 0; i < l_childSelectionObject; ++i )
        {
            wxTreeItemId l_childId = m_pTreeWidget->GetNextChild( l_selectionObjectTreeId, childcookie );
            if( l_childId.IsOk() )
            {
                SelectionObject* l_childSelectionObject = ( (SelectionObject*)( m_pTreeWidget->GetItemData( l_childId ) ) );
                l_childSelectionObject->setIsActive( l_selectionObject->getIsActive() );
                m_pTreeWidget->SetItemImage( l_childId, l_childSelectionObject->getIcon() );
                l_childSelectionObject->setIsDirty( true );
            }
        }
    }
    else if( treeSelected( l_selectionObjectTreeId ) == CHILD_OBJECT )
    {
        SelectionObject* l_selectionObject = (SelectionObject*)( m_pTreeWidget->GetItemData( l_selectionObjectTreeId ) );
        l_selectionObject->toggleIsActive();
        m_pTreeWidget->SetItemImage( l_selectionObjectTreeId, l_selectionObject->getIcon() );
        l_selectionObject->setIsDirty( true );
    }
    refreshAllGLWidgets();
}

void MainFrame::onToggleDrawPointsMode( wxCommandEvent& event )
{
    if( m_pDatasetHelper->m_theScene == NULL )
    {
        return;
    }
    m_pDatasetHelper->togglePointMode();
    refreshAllGLWidgets();
}

void MainFrame::onMoveBoundaryPointsLeft( wxCommandEvent& WXUNUSED(event) )
{
    moveBoundaryPoints(5);
}


void MainFrame::onMoveBoundaryPointsRight(wxCommandEvent& WXUNUSED(event))
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
    l_id = m_pTreeWidget->GetFirstChild( m_tPointId, l_cookie );
    while( l_id.IsOk() )
    {
        SplinePoint* l_point = (SplinePoint*)( m_pTreeWidget->GetItemData( l_id ) );
        if( l_point->isBoundary())
        {
            l_point->setX( l_point->X() + i_value );
        }
        l_id = m_pTreeWidget->GetNextChild( m_tPointId, l_cookie );
    }

    m_pDatasetHelper->m_surfaceIsDirty = true;
    refreshAllGLWidgets();
}


void MainFrame::deleteSceneObject()
{
    if (m_pCurrentSizer != NULL)
    {
        m_pPropertiesWindow->GetSizer()->Hide(m_pCurrentSizer, true);
        m_pPropertiesWindow->GetSizer()->Detach(m_pCurrentSizer);        
        m_pCurrentSizer = NULL;
    }
    //delete m_currentFNObject;
    m_pCurrentSceneObject = NULL;
    m_pLastSelectedSceneObject = NULL;
    m_currentListItem = -1;
    m_lastSelectedListItem = -1;
    m_pPropertiesWindow->GetSizer()->Layout();
    doOnSize();
}

void MainFrame::displayPropertiesSheet()
{   
    if (m_pLastSelectedSceneObject == NULL && m_pCurrentSceneObject == NULL)
    {
        if (m_pCurrentSizer != NULL)
        {
           m_pPropertiesWindow->GetSizer()->Hide(m_pCurrentSizer);
        }
    }
    else
    {
        if (m_pLastSelectedSceneObject != NULL)
        {       
            if (m_pCurrentSizer != NULL )
            {
                m_pPropertiesWindow->GetSizer()->Hide(m_pCurrentSizer);                 
            }
            if (!m_pLastSelectedSceneObject->getProprietiesSizer())
            {
                m_pLastSelectedSceneObject->createPropertiesSizer(m_pPropertiesWindow);
            }   
            m_pCurrentSizer = m_pLastSelectedSceneObject->getProprietiesSizer();
            
            m_pCurrentSceneObject = m_pLastSelectedSceneObject;
            m_currentListItem = m_lastSelectedListItem;
            if (!m_pPropertiesWindow->GetSizer()->Show( m_pCurrentSizer, true, true ))
            {
                m_pPropertiesWindow->GetSizer()->Add(m_pCurrentSizer, 0, wxLeft | wxFIXED_MINSIZE, 0 );
                m_pPropertiesWindow->GetSizer()->Show( m_pCurrentSizer, true, true );                
            }             
            doOnSize();            
        }        
        m_pCurrentSceneObject->updatePropertiesSizer();
        m_pLastSelectedSceneObject = NULL;
    }     
}

///////////////////////////////////////////////////////////////////////////
// This function will create a new selection object of type ellipsoid, it will be 
// triggered when the user click on Ellipsoid sub menu under the New Selection-Object
// sub menu under the VOI menu.
///////////////////////////////////////////////////////////////////////////
void MainFrame::onNewSelectionEllipsoid( wxCommandEvent& WXUNUSED(event) )
{
    createNewSelectionObject( ELLIPSOID_TYPE );
    m_pDatasetHelper->m_isBoxCreated = true;
}

///////////////////////////////////////////////////////////////////////////
// This function will create a new selection object of type box, it will be 
// triggered when the user click on Box sub menu under the New Selection-Object
// sub menu under the VOI menu.
///////////////////////////////////////////////////////////////////////////
void MainFrame::onNewSelectionBox( wxCommandEvent& WXUNUSED(event) )
{
    createNewSelectionObject( BOX_TYPE );    
    m_pDatasetHelper->m_isBoxCreated = true;
}

///////////////////////////////////////////////////////////////////////////
// This function will create a new selection object of type depending of the argument.
//
// i_newSelectionObjectType         : The type of the new selection object we wat to create.
///////////////////////////////////////////////////////////////////////////
void MainFrame::createNewSelectionObject( ObjectType i_newSelectionObjectType )
{
    if( m_pDatasetHelper->m_theScene == NULL)
    {
        return;
    }
    Vector l_center( m_pXSlider->GetValue() * m_pDatasetHelper->m_xVoxel, 
                     m_pYSlider->GetValue() * m_pDatasetHelper->m_yVoxel, 
                     m_pZSlider->GetValue() * m_pDatasetHelper->m_zVoxel );
    float l_sizeV = 10;

    Vector l_size( l_sizeV / m_pDatasetHelper->m_xVoxel, 
                   l_sizeV / m_pDatasetHelper->m_yVoxel,
                   l_sizeV / m_pDatasetHelper->m_zVoxel );
    

    SelectionObject* l_newSelectionObject;
    if( i_newSelectionObjectType == ELLIPSOID_TYPE )
    {
        l_newSelectionObject = new SelectionEllipsoid( l_center, l_size, m_pDatasetHelper );
    }
    else if( i_newSelectionObjectType == BOX_TYPE )
    {
        l_newSelectionObject = new SelectionBox( l_center, l_size, m_pDatasetHelper );
    }
    else
    {
        return;
    }
    // Check what is selected in the tree to know where to put this new selection object.
    wxTreeItemId l_treeSelectionId = m_pTreeWidget->GetSelection();

    wxTreeItemId l_newSelectionObjectId;
    
    if( treeSelected( l_treeSelectionId ) == MASTER_OBJECT )
    {
        // Our new seleciton object is under another master selection object.
        l_newSelectionObjectId = m_pTreeWidget->AppendItem( l_treeSelectionId, l_newSelectionObject->getName(), 0, -1, l_newSelectionObject );
        m_pTreeWidget->SetItemBackgroundColour( l_newSelectionObjectId, *wxGREEN );
    }
    else if( treeSelected( l_treeSelectionId ) == CHILD_OBJECT )
    {
        wxTreeItemId l_parentId = m_pTreeWidget->GetItemParent( l_treeSelectionId );

        // Our new seleciton object is under another child selection object.
        l_newSelectionObjectId = m_pTreeWidget->AppendItem( l_parentId, l_newSelectionObject->getName(), 0, -1, l_newSelectionObject );
        m_pTreeWidget->SetItemBackgroundColour( l_newSelectionObjectId, *wxGREEN );
    }
    else
    {
        // Our new seleciton object is on top.
        l_newSelectionObject->setIsMaster( true );
        l_newSelectionObjectId = m_pTreeWidget->AppendItem( m_tSelectionObjectsId, l_newSelectionObject->getName(), 0, -1, l_newSelectionObject );
        m_pTreeWidget->SetItemBackgroundColour( l_newSelectionObjectId, *wxCYAN );
    }

    m_pTreeWidget->EnsureVisible( l_newSelectionObjectId );
    m_pTreeWidget->SetItemImage( l_newSelectionObjectId, l_newSelectionObject->getIcon() );
    l_newSelectionObject->setTreeId( l_newSelectionObjectId );    
    m_pDatasetHelper->m_selBoxChanged = true;
    m_pTreeWidget->SelectItem(l_newSelectionObjectId, true);    
    refreshAllGLWidgets();
}


void MainFrame::onHideSelectionObjects( wxCommandEvent& WXUNUSED(event) )
{
    if( m_pDatasetHelper->m_theScene == NULL )
    {
        return;
    }
    m_pDatasetHelper->toggleShowAllSelectionObjects();
    refreshAllGLWidgets();
}

void MainFrame::onActivateSelectionObjects( wxCommandEvent& WXUNUSED(event) )
{
    if( m_pDatasetHelper->m_theScene == NULL)
    {
        return;
    }
    m_pDatasetHelper->toggleActivateAllSelectionObjects();
    refreshAllGLWidgets();
}

void MainFrame::onUseMorph( wxCommandEvent& WXUNUSED(event) )
{
    if( m_pDatasetHelper->m_theScene == NULL)
    {
        return;
    }
    m_pDatasetHelper->m_morphing = ! m_pDatasetHelper->m_morphing;
}

/****************************************************************************************************
 *
 * Menu Spline Surface
 *
 ****************************************************************************************************/

void MainFrame::onNewSplineSurface( wxCommandEvent& WXUNUSED(event) )
{
    //Generate KdTree for Spline Surface
    Fibers* pTmpFib = NULL;
    m_pDatasetHelper->getSelectedFiberDataset(pTmpFib);
    if(pTmpFib != NULL)
    {
        pTmpFib->generateKdTree();
    }

    if( ! m_pDatasetHelper->m_theScene || m_pDatasetHelper->m_surfaceLoaded )
    {
        return;
    }
    int l_xs = (int)( m_pXSlider->GetValue() * m_pDatasetHelper->m_xVoxel );
    int l_ys = (int)( m_pYSlider->GetValue() * m_pDatasetHelper->m_yVoxel );
    int l_zs = (int)( m_pZSlider->GetValue() * m_pDatasetHelper->m_zVoxel );

    // Delete all existing points.
    m_pTreeWidget->DeleteChildren( m_tPointId );
    Fibers* l_fibers = NULL;

    if( m_pDatasetHelper->m_fibersLoaded )
    {
        m_pDatasetHelper->getSelectedFiberDataset( l_fibers );
    }
    if( m_pDatasetHelper->m_showSagittal )
    {
        for( int i = 0; i < 11; ++i )
        {
            for( int j = 0; j < 11; ++j )
            {
                int yy = (int)( ( m_pDatasetHelper->m_rows   / 10 * m_pDatasetHelper->m_yVoxel ) * i );
                int zz = (int)( ( m_pDatasetHelper->m_frames / 10 * m_pDatasetHelper->m_zVoxel ) * j );

                // Create the point.
                SplinePoint* l_point = new SplinePoint( l_xs, yy, zz, m_pDatasetHelper );

                if( i == 0 || i == 10 || j == 0 || j == 10 )
                {
                    wxString l_name = wxT("boundary l_point");
                    wxTreeItemId tId = m_pTreeWidget->AppendItem( m_tPointId, l_name, -1, -1, l_point );
                    l_point->setTreeId( tId );
                    l_point->setName(l_name);
                    l_point->setIsBoundary( true );
                }
                else
                {
                    if( m_pDatasetHelper->m_fibersLoaded && l_fibers->getBarycenter( l_point ) )
                    {
                        wxString l_name = wxT( "l_point" );
                        wxTreeItemId tId = m_pTreeWidget->AppendItem( m_tPointId, l_name, -1, -1, l_point );
                        l_point->setTreeId( tId );
                        l_point->setName(l_name);
                        l_point->setIsBoundary( false );
                    }
                }
            }
        }
    }
    else if( m_pDatasetHelper->m_showCoronal )
    {
        for( int i = 0; i < 11; ++i )
        {
            for( int j = 0; j < 11; ++j )
            {
                int l_xx = (int)( ( m_pDatasetHelper->m_columns / 10 * m_pDatasetHelper->m_xVoxel ) * i );
                int l_zz = (int)( ( m_pDatasetHelper->m_frames  / 10 * m_pDatasetHelper->m_zVoxel ) * j );

                // Create the point.
                SplinePoint* l_point = new SplinePoint( l_xx, l_ys, l_zz, m_pDatasetHelper );

                if( i == 0 || i == 10 || j == 0 || j == 10 )
                {
                    wxTreeItemId l_treeId = m_pTreeWidget->AppendItem( m_tPointId, wxT("boundary l_point"), -1, -1, l_point );
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
                int l_xx = (int)( ( m_pDatasetHelper->m_columns / 10 * m_pDatasetHelper->m_xVoxel ) * i );
                int l_yy = (int)( ( m_pDatasetHelper->m_rows    / 10 * m_pDatasetHelper->m_yVoxel ) * j );

                // Create the point.
                SplinePoint* l_point = new SplinePoint( l_xx, l_yy, l_zs, m_pDatasetHelper );

                if (i == 0 || i == 10 || j == 0 || j == 10)
                {
                    wxTreeItemId l_treeId = m_pTreeWidget->AppendItem( m_tPointId, wxT("boundary l_point"), -1, -1, l_point );
                    l_point->setTreeId( l_treeId );
                    l_point->setIsBoundary( true );
                }
            }
        }
    }

#ifdef __WXMAC__
    // insert at zero is a well-known bug on OSX, so we append there...
    // http://trac.wxwidgets.org/ticket/4492
    long id = m_pListCtrl->GetItemCount();
#else
    long id = 0;
#endif

    Surface* l_surface = new Surface(m_pDatasetHelper);
    l_surface->execute();

    m_pListCtrl->InsertItem( id, wxT( "" ), 0 );
    m_pListCtrl->SetItem( id, 1, l_surface->getName() );
    m_pListCtrl->SetItem( id, 2, wxT( "0.50" ) );
    m_pListCtrl->SetItem( id, 3, wxT( "" ), 1 );
    m_pListCtrl->SetItemData( id, (long)l_surface );
    m_pListCtrl->SetItemState( id, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );

    m_pDatasetHelper->m_surfaceLoaded = true;
    refreshAllGLWidgets();
}


void MainFrame::onToggleDrawVectors( wxCommandEvent& WXUNUSED(event) )
{
    if(m_pDatasetHelper->m_theScene == NULL)
    {
        return;
    }
    m_pDatasetHelper->m_drawVectors = !m_pDatasetHelper->m_drawVectors;
    refreshAllGLWidgets();
}

void MainFrame::onToggleNormal( wxCommandEvent& WXUNUSED(event ))
{
    m_pDatasetHelper->m_normalDirection *= -1.0;

    for( int i = 0; i < m_pListCtrl->GetItemCount(); ++i )
    {
        DatasetInfo* l_info = (DatasetInfo*) m_pListCtrl->GetItemData( i );
        if( l_info->getType() == SURFACE )
        {
            Surface* l_surf = (Surface*)m_pListCtrl->GetItemData( i );
            l_surf->flipNormals();
        }
    }
    refreshAllGLWidgets();
}

void MainFrame::onToggleTextureFiltering( wxCommandEvent& WXUNUSED(event) )
{
    if( m_pCurrentSceneObject != NULL && m_currentListItem != -1 )
    {
        DatasetInfo* l_info = (DatasetInfo*)m_pCurrentSceneObject;
        if( l_info->getType() < MESH )
        {
            if ( !l_info->toggleShowFS() )
            {
                m_pListCtrl->SetItem( m_currentListItem, 1, l_info->getName() + wxT( "*" ) );
            }
            else
            {
                m_pListCtrl->SetItem( m_currentListItem, 1, l_info->getName() );
            }
        }
    }
    refreshAllGLWidgets();
}

void MainFrame::onToggleLighting( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_lighting =! m_pDatasetHelper->m_lighting;
    refreshAllGLWidgets();
}

void MainFrame::onToggleBlendTexOnMesh( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_blendTexOnMesh =! m_pDatasetHelper->m_blendTexOnMesh;
    refreshAllGLWidgets();
}

void MainFrame::onToggleFilterIso( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_filterIsoSurf =! m_pDatasetHelper->m_filterIsoSurf;
    refreshAllGLWidgets();
}

void MainFrame::onInvertFibers( wxCommandEvent& WXUNUSED(event) )
{
	if (m_pCurrentSceneObject != NULL && m_currentListItem != -1)
    {
		DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pCurrentSceneObject);
		if( pDatasetInfo->getType() == FIBERS )
		{
			Fibers* l_fibers = NULL;
			if( m_pDatasetHelper->getSelectedFiberDataset( l_fibers ) != NULL)
			{
				l_fibers->invertFibers();
			}
		}
		else if ( pDatasetInfo->getType() == FIBERSGROUP )
		{
			FibersGroup* l_fibersGroup = NULL;
			if( m_pDatasetHelper->getFibersGroupDataset( l_fibersGroup ) != NULL )
			{
				l_fibersGroup->invertFibers();
			}			
		}
	}

    m_pDatasetHelper->m_selBoxChanged = true;
    refreshAllGLWidgets();
}

void MainFrame::onUseFakeTubes( wxCommandEvent& WXUNUSED(event) )
{
	if (m_pCurrentSceneObject != NULL && m_currentListItem != -1)
    {
		DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pCurrentSceneObject);
		if( pDatasetInfo->getType() == FIBERS )
		{
			Fibers* l_fibers = NULL;
			if( m_pDatasetHelper->getSelectedFiberDataset( l_fibers ) != NULL)
			{
				l_fibers->useFakeTubes();
			}
		}
		else if ( pDatasetInfo->getType() == FIBERSGROUP )
		{
			FibersGroup* l_fibersGroup = NULL;
			if( m_pDatasetHelper->getFibersGroupDataset( l_fibersGroup ) != NULL )
			{
				l_fibersGroup->useFakeTubes();
			}			
		}
	}
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will changes the clear color to black or white.
///////////////////////////////////////////////////////////////////////////
void MainFrame::onClearToBlack( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_clearToBlack = !m_pDatasetHelper->m_clearToBlack;
    
    if( m_pDatasetHelper->m_clearToBlack )
    {
        glClearColor( 0.0, 0.0, 0.0, 0.0 );
    }
    else
    {
        glClearColor( 1.0, 1.0, 1.0, 0.0 );
    }
    refreshAllGLWidgets();
}

void MainFrame::onRulerTool( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_isRulerToolActive = !m_pDatasetHelper->m_isRulerToolActive;    
    if (m_pDatasetHelper->m_isRulerToolActive)
    {
        m_pToolBar->m_txtRuler->Enable();
    } 
    else 
    {
        m_pToolBar->m_txtRuler->Disable();
    }
    refreshAllGLWidgets();
}

void MainFrame::onRulerToolClear( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_rulerPts.clear();
    m_pDatasetHelper->m_rulerFullLength = 0;
    m_pDatasetHelper->m_rulerPartialLength = 0;
    refreshAllGLWidgets();
}

void MainFrame::onRulerToolAdd( wxCommandEvent& WXUNUSED(event) )
{
    if (m_pDatasetHelper->m_isRulerToolActive && m_pDatasetHelper->m_rulerPts.size()>0)
    {
        m_pDatasetHelper->m_rulerPts.push_back(m_pDatasetHelper->m_rulerPts.back());
    }
    refreshAllGLWidgets();
}

void MainFrame::onRulerToolDel( wxCommandEvent& WXUNUSED(event) )
{
    if (m_pDatasetHelper->m_isRulerToolActive && m_pDatasetHelper->m_rulerPts.size()>0)
    {
        m_pDatasetHelper->m_rulerPts.pop_back();
    }
    refreshAllGLWidgets();
}

void MainFrame::onUseTransparency( wxCommandEvent& WXUNUSED(event) )
{    
	if (m_pCurrentSceneObject != NULL && m_currentListItem != -1)
    {
		DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pCurrentSceneObject);
		if( pDatasetInfo->getType() == FIBERS )
		{
			Fibers* l_fibers = NULL;
			if( m_pDatasetHelper->getSelectedFiberDataset( l_fibers ) != NULL)
			{
				l_fibers->useTransparency();

			}
		}
		else if ( pDatasetInfo->getType() == FIBERSGROUP )
		{
			FibersGroup* l_fibersGroup = NULL;
			if( m_pDatasetHelper->getFibersGroupDataset( l_fibersGroup ) != NULL )
			{
				l_fibersGroup->useTransparency();
			}			
		}
	}
    refreshAllGLWidgets();
}

void MainFrame::onResetColor(wxCommandEvent& WXUNUSED(event))
{
	if (m_pCurrentSceneObject != NULL && m_currentListItem != -1)
    {
		DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pCurrentSceneObject);

        if( pDatasetInfo->getType() == FIBERS )
        {
            Fibers* l_fibers = NULL; // Initalize it quiet compiler.
			if( m_pDatasetHelper->getSelectedFiberDataset( l_fibers ) == NULL)
			{
				return;
			}
			l_fibers->resetColorArray();
		}
		else if ( pDatasetInfo->getType() == FIBERSGROUP )
		{
			FibersGroup* l_fibersGroup = NULL;
			if( m_pDatasetHelper->getFibersGroupDataset( l_fibersGroup ) == NULL )
			{
				return;
			}
			l_fibersGroup->resetFibersColor();
		}
	}
    
    m_pDatasetHelper->m_selBoxChanged = true;
    refreshAllGLWidgets();
}

void MainFrame::onToggleColorMapLegend( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_showColorMapLegend = !m_pDatasetHelper->m_showColorMapLegend;
    refreshAllGLWidgets();
}

void MainFrame::onSetCMap0( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_colorMap = 0;
    refreshAllGLWidgets();
}

void MainFrame::onSetCMap1( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_colorMap = 1;
    refreshAllGLWidgets();
}

void MainFrame::onSetCMap2( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_colorMap = 2;
    refreshAllGLWidgets();
}

void MainFrame::onSetCMap3( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_colorMap = 3;
    refreshAllGLWidgets();
}

void MainFrame::onSetCMap4( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_colorMap = 4;
    refreshAllGLWidgets();
}

void MainFrame::onSetCMap5( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_colorMap = 5;
    refreshAllGLWidgets();
}

void MainFrame::onSetCMapNo( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_colorMap = -1;
    refreshAllGLWidgets();
} 

/****************************************************************************************************
 *
 * Menu Help
 *
 ****************************************************************************************************/

void MainFrame::onAbout( wxCommandEvent& WXUNUSED(event) )
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

void MainFrame::onShortcuts( wxCommandEvent& WXUNUSED(event) )
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

void MainFrame::onScreenshot( wxCommandEvent& WXUNUSED(event) )
{
    wxString l_caption         = wxT( "Choose a file" );
    wxString l_wildcard        = wxT( "PPM files (*.ppm)|*.ppm|*.*|*.*" );
    wxString l_defaultDir      = wxEmptyString;
    wxString l_defaultFilename = wxEmptyString;
    wxFileDialog dialog( this, l_caption, l_defaultDir, l_defaultFilename, l_wildcard, wxSAVE );
    dialog.SetFilterIndex( 0 );
    dialog.SetDirectory( m_pDatasetHelper->m_screenshotPath );
    if( dialog.ShowModal() == wxID_OK )
    {
        m_pDatasetHelper->m_screenshotPath = dialog.GetDirectory();
        m_pDatasetHelper->m_screenshotName = dialog.GetPath();
        if( m_pDatasetHelper->m_screenshotName.AfterLast('.') != _T( "ppm" ) )
        {
            m_pDatasetHelper->m_screenshotName += _T( ".ppm" );
        }
        m_pDatasetHelper->m_scheduledScreenshot = true;
        m_pMainGL->render();
        m_pMainGL->render();
    }
}

void MainFrame::screenshot( wxString i_fileName )
{
    m_pDatasetHelper->m_screenshotPath = _( "" );
    m_pDatasetHelper->m_screenshotName = i_fileName;
    if ( m_pDatasetHelper->m_screenshotName.AfterLast( '.' ) != _T( "ppm" ) )
    {
        m_pDatasetHelper->m_screenshotName += _T( ".ppm" );
    }
    m_pDatasetHelper->m_scheduledScreenshot = true;
    m_pMainGL->render();
    m_pMainGL->render();
}

void MainFrame::onSlizeMovieSag( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->doLicMovie( 0 );
}

void MainFrame::onSlizeMovieCor( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->doLicMovie(1);
}

void MainFrame::onSlizeMovieAxi( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->doLicMovie( 2 );
}

void MainFrame::onSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->updateView( m_pXSlider->GetValue(), m_pYSlider->GetValue(), m_pZSlider->GetValue() );
    refreshAllGLWidgets();
}

void MainFrame::onToggleShowAxial( wxCommandEvent& WXUNUSED(event) )
{
    if (m_pDatasetHelper->m_theScene == NULL)
    {
        return;
    }
    m_pDatasetHelper->m_showAxial = ! m_pDatasetHelper->m_showAxial;
    m_pMainGL->Refresh();
}

void MainFrame::onToggleShowCoronal( wxCommandEvent& WXUNUSED(event) )
{
    if (m_pDatasetHelper->m_theScene == NULL)
    {
        return;
    }
    m_pDatasetHelper->m_showCoronal = ! m_pDatasetHelper->m_showCoronal;
    m_pMainGL->Refresh();
}

void MainFrame::onToggleShowSagittal( wxCommandEvent& WXUNUSED(event) )
{
    if (m_pDatasetHelper->m_theScene == NULL)
    {
        return;
    }
    m_pDatasetHelper->m_showSagittal = ! m_pDatasetHelper->m_showSagittal;
    m_pMainGL->Refresh();
}

void MainFrame::onToggleAlpha( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->m_blendAlpha = !m_pDatasetHelper->m_blendAlpha;
    refreshAllGLWidgets();
    this->Update(); 
}

void MainFrame::refreshAllGLWidgets()
{
    updateStatusBar();
    updateMenus();
    refreshViews();   
    if (m_pDatasetHelper->m_isRulerToolActive){
        wxString sbString1 = wxString::Format( wxT("%4.1fmm (%2.1fmm)" ), m_pDatasetHelper->m_rulerFullLength, m_pDatasetHelper->m_rulerPartialLength );
        m_pToolBar->m_txtRuler->SetValue(sbString1);    
    }     
}

void MainFrame::refreshViews()
{
    m_pPropertiesWindow->Fit();
    m_pPropertiesWindow->AdjustScrollbars();

    m_pPropertiesWindow->Layout();
    displayPropertiesSheet();
    if ( m_pMainGL )
    {
        m_pMainGL->Refresh(true);
    }
    if ( m_pGL0 )
    {
        m_pGL0->Refresh(true);
    }
    if ( m_pGL1 )
    {
        m_pGL1->Refresh(true);
    }
    if ( m_pGL2 )
    {
        m_pGL2->Refresh(true);
    }    
}

void MainFrame::renewAllGLWidgets()
{
    if( m_pMainGL )
    {
        m_pMainGL->invalidate();
    }
    if( m_pGL0 )
    {
        m_pGL0->invalidate();
    }
    if( m_pGL1 )
    {
        m_pGL1->invalidate();
    }
    if( m_pGL2 )
    {
        m_pGL2->invalidate();
    }
    refreshAllGLWidgets();
}

void MainFrame::updateStatusBar()
{
    wxString sbString0 = wxT( "" );
    sbString0 = wxString::Format( wxT("Position: %d  %d  %d" ),
                                  m_pXSlider->GetValue(),
                                  m_pYSlider->GetValue(),
                                  m_pZSlider->GetValue() );
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

void MainFrame::onActivateListItem( wxListEvent& event )
{
    int l_item = event.GetIndex();
    m_pTreeWidget->UnselectAll();
    DatasetInfo* l_info = (DatasetInfo*)m_pListCtrl->GetItemData( l_item );
    m_pLastSelectedSceneObject = l_info;
    m_lastSelectedListItem = l_item;
    int l_col = m_pListCtrl->getColActivated();
    switch( l_col )
    {        
        case 10:
            if (l_info->toggleShow())
            {
                m_pListCtrl->SetItem( l_item, 0, wxT( "" ), 0 );
            }
            else
            {
                m_pListCtrl->SetItem( l_item, 0, wxT( "" ), 1 );
            }
            break;
        case 11:
            if( ! l_info->toggleShowFS())
            {
                m_pListCtrl->SetItem( l_item, 1, l_info->getName().BeforeFirst( '.' ) + wxT( "*" ) );
            }
            else
            {
                m_pListCtrl->SetItem( l_item, 1, l_info->getName().BeforeFirst( '.' ) );
            }
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
    if (m_pCurrentSceneObject != NULL && m_currentListItem != -1)
    {       
        long tmp = m_currentListItem;
		if ((((DatasetInfo*)m_pListCtrl->GetItemData( m_currentListItem))->getType() == FIBERSGROUP))
		{
			return;
		}
        if (((DatasetInfo*)m_pListCtrl->GetItemData( m_currentListItem))->getType() == FIBERS)
        {            
			FibersGroup* pFibersGroup = NULL;
			m_pDatasetHelper->getFibersGroupDataset(pFibersGroup);
			if(pFibersGroup != NULL)
			{
				Fibers* pFibers = NULL;
				m_pDatasetHelper->getSelectedFiberDataset(pFibers);
				if(pFibers != NULL)
				{
					pFibersGroup->removeFibersSet(pFibers);
				}
			}
			m_pDatasetHelper->m_selBoxChanged = true;
        }
        else if (((DatasetInfo*)m_pListCtrl->GetItemData( m_currentListItem))->getType() == SURFACE)
        {
            m_pDatasetHelper->deleteAllPoints();
        }
        if (((DatasetInfo*)m_pListCtrl->GetItemData( m_currentListItem))->getName() == _T( "(Object)" ))
        {            
            m_pDatasetHelper->m_isObjCreated = false;
            m_pDatasetHelper->m_isObjfilled = false;
    
        }
        if (((DatasetInfo*)m_pListCtrl->GetItemData( m_currentListItem))->getName() == _T( "(Background)" ))
        {            
            m_pDatasetHelper->m_isBckCreated = false;
            m_pDatasetHelper->m_isBckfilled = false;
        }
        deleteSceneObject();
        m_pListCtrl->DeleteItem( tmp );
        m_pDatasetHelper->updateLoadStatus();
        refreshAllGLWidgets();
    }
}

void MainFrame::onSelectListItem( wxListEvent& event )
{
    int l_item = event.GetIndex();
    m_pTreeWidget->UnselectAll();
    DatasetInfo *l_info = (DatasetInfo*)m_pListCtrl->GetItemData( l_item) ;
    int l_col = m_pListCtrl->getColClicked();
    if (l_col == 12 && l_info->getType() >= MESH)
    {        
        if( ! l_info->toggleUseTex())
        {
            m_pListCtrl->SetItem( l_item, 2, wxT( "(" ) + wxString::Format( wxT( "%.2f" ), l_info->getThreshold() * l_info->getOldMax() ) + wxT( ")" ) );
        }
        else
        {
            m_pListCtrl->SetItem( l_item,2,wxString::Format( wxT( "%.2f" ), l_info->getThreshold() * l_info->getOldMax() ) );
        }            
    }
	if( l_info->getType() == FIBERS )
	{
		Fibers* pFibers = (Fibers*)l_info;
		if( pFibers )
		{
			pFibers->updateColorationMode();
		}
	}
    m_pLastSelectedSceneObject = l_info;
    m_lastSelectedListItem = l_item;
    refreshAllGLWidgets();
}



void MainFrame::onListMenuName( wxCommandEvent&  WXUNUSED(event) )
{
    long l_item = m_pListCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( l_item == -1 )
    {
        return;
    }
    DatasetInfo *l_info = (DatasetInfo*)m_pListCtrl->GetItemData( l_item) ;

    if( ! l_info->toggleShowFS() )
    {
        m_pListCtrl->SetItem( l_item, 1, l_info->getName().BeforeFirst( '.' ) + wxT( "*" ) );
    }
    else
    {
        m_pListCtrl->SetItem( l_item, 1, l_info->getName().BeforeFirst( '.' ) );
    }
}

/****************************************************************************************************
 *
 * Functions for tree widget event handling
 *
 ****************************************************************************************************/

///////////////////////////////////////////////////////////////////////////
// This function will be called when the delete tree item event is triggered.
///////////////////////////////////////////////////////////////////////////
void MainFrame::onDeleteTreeItem( wxTreeEvent& WXUNUSED(event) )
{    
    deleteTreeItem();
}

void MainFrame::deleteTreeItem()
{
    if (m_pCurrentSceneObject != NULL)
    {   
        wxTreeItemId l_treeId = m_pTreeWidget->GetSelection();
        if (!l_treeId.IsOk())
        {
            return;
        }
        int l_selected = treeSelected( l_treeId );  
        if (l_selected == CHILD_OBJECT)
        {
            ((SelectionObject*) ((m_pTreeWidget->GetItemData(m_pTreeWidget->GetItemParent(l_treeId)))))->setIsDirty(true);
        }
        if (l_selected == CHILD_OBJECT || l_selected == MASTER_OBJECT || l_selected == POINT_DATASET)
        {  
            deleteSceneObject();
            m_pTreeWidget->Delete(l_treeId);
            if (m_pDatasetHelper->m_lastSelectedObject != NULL)
            {
                m_pDatasetHelper->m_selBoxChanged = true;
            }
            m_pDatasetHelper->m_lastSelectedObject = NULL;
            m_pDatasetHelper->m_lastSelectedPoint = NULL;
        }   
        m_pDatasetHelper->m_selBoxChanged = true;
    }
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// This function will be called when the selection changed event in the item tree is triggered.
///////////////////////////////////////////////////////////////////////////
void MainFrame::onSelectTreeItem( wxTreeEvent& WXUNUSED(event) )
{
    wxTreeItemId l_treeId = m_pTreeWidget->GetSelection();
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
            if ( m_pDatasetHelper->m_lastSelectedObject )
            {
                m_pDatasetHelper->m_lastSelectedObject->unselect();
            }
            if ( m_pDatasetHelper->m_lastSelectedPoint )
            {
                m_pDatasetHelper->m_lastSelectedPoint->unselect();
                m_pDatasetHelper->m_lastSelectedPoint = NULL;
            }

            l_selectionObject = (SelectionObject*)( m_pTreeWidget->GetItemData( l_treeId ) );
            m_pDatasetHelper->m_lastSelectedObject = l_selectionObject;
            m_pDatasetHelper->m_lastSelectedObject->select( false );
            m_pLastSelectedSceneObject = l_selectionObject;
            m_lastSelectedListItem = -1;
            break;

        case POINT_DATASET:
            if( m_pDatasetHelper->m_lastSelectedPoint )
            {
                m_pDatasetHelper->m_lastSelectedPoint->unselect();
            }
            if( m_pDatasetHelper->m_lastSelectedObject )
            {
                m_pDatasetHelper->m_lastSelectedObject->unselect();
                m_pDatasetHelper->m_lastSelectedObject = NULL;
            }

            l_selectedPoint = (SplinePoint*)( m_pTreeWidget->GetItemData( l_treeId ) );
            m_pDatasetHelper->m_lastSelectedPoint = l_selectedPoint;
            m_pDatasetHelper->m_lastSelectedPoint->select( false );
            m_pLastSelectedSceneObject = l_selectedPoint;
            m_lastSelectedListItem = -1;
            break;

        default:
            if( m_pDatasetHelper->m_lastSelectedPoint )
            {
                m_pDatasetHelper->m_lastSelectedPoint->unselect();
                m_pDatasetHelper->m_lastSelectedPoint = NULL;
            }
            if( m_pDatasetHelper->m_lastSelectedObject )
            {
                m_pDatasetHelper->m_lastSelectedObject->unselect();
                m_pDatasetHelper->m_lastSelectedObject = NULL;
            }
            break;
    }    
#ifdef __WXMSW__
    if (m_currentListItem != -1)
    {       
        m_pListCtrl->SetItemState(m_currentListItem,0,wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);  
    }
#endif
    refreshAllGLWidgets();
}

void MainFrame::onRightClickTreeItem( wxTreeEvent& event )
{

}

void MainFrame::onActivateTreeItem( wxTreeEvent& WXUNUSED(event) )
{
    wxTreeItemId l_treeId = m_pTreeWidget->GetSelection();

    int l_selected = treeSelected( l_treeId );
    if( l_selected == MASTER_OBJECT )
    {
        SelectionObject* l_selectionObject = (SelectionObject*)( m_pTreeWidget->GetItemData( l_treeId ) );
        l_selectionObject->toggleIsActive();
        m_pTreeWidget->SetItemImage(l_treeId, l_selectionObject->getIcon());
        l_selectionObject->setIsDirty(true);

        int l_childSelectionObjects = m_pTreeWidget->GetChildrenCount( l_treeId );
        wxTreeItemIdValue l_childCookie = 0;

        for( int i = 0; i < l_childSelectionObjects; ++i )
        {
            wxTreeItemId l_childId = m_pTreeWidget->GetNextChild( l_treeId, l_childCookie );
            if( l_childId.IsOk() )
            {
                SelectionObject* l_childSelectionBox = ( (SelectionObject*)( m_pTreeWidget->GetItemData( l_childId ) ) );
                l_childSelectionBox->setIsActive( l_selectionObject->getIsActive() );
                m_pTreeWidget->SetItemImage( l_childId, l_childSelectionBox->getIcon() );
                l_childSelectionBox->setIsDirty( true );
            }
        }
    }
    else if( l_selected == CHILD_OBJECT )
    {
        SelectionObject* l_box = (SelectionObject*)( m_pTreeWidget->GetItemData( l_treeId ) );

        l_box->toggleIsNOT();
        wxTreeItemId l_parentId = m_pTreeWidget->GetItemParent( l_treeId );
        ((SelectionObject*)( m_pTreeWidget->GetItemData( l_parentId ) ) )->setIsDirty( true );

        if( l_box->getIsNOT() )
        {
            m_pTreeWidget->SetItemBackgroundColour( l_treeId, *wxRED );
        }
        else
        {
            m_pTreeWidget->SetItemBackgroundColour( l_treeId, *wxGREEN );
        }
    }
    refreshAllGLWidgets();
}

void MainFrame::onTreeChange()
{
    m_pDatasetHelper->m_selBoxChanged = true;
    refreshAllGLWidgets();
}

void MainFrame::onTreeLabelEdit( wxTreeEvent& event )
{
    wxTreeItemId l_treeId = event.GetItem();
    int l_selected = treeSelected( l_treeId );

    if( l_selected == CHILD_OBJECT || l_selected == MASTER_OBJECT )
    {
        ( (SelectionObject*)m_pTreeWidget->GetItemData( l_treeId ) )->setName( event.GetLabel() );
    }
}

//////////////////////////////////////////////////////////////////////////
// Helper function to determine what kind of item is selected in the tree widget.
//
// i_id         : The selected item id.
//////////////////////////////////////////////////////////////////////////
int MainFrame::treeSelected( wxTreeItemId i_id )
{
    if( !i_id.IsOk() )
    {
        return 0;
    }
    wxTreeItemId l_pId = m_pTreeWidget->GetItemParent( i_id );

    if( !l_pId.IsOk() )
    {
        return 0;
    }
    wxTreeItemId l_ppId = m_pTreeWidget->GetItemParent( l_pId );

    if( !l_ppId.IsOk() )
    {
        return 0;
    }

    if( l_pId == m_tSelectionObjectsId )
    {
        return MASTER_OBJECT;
    }
    else if ( l_ppId == m_tSelectionObjectsId )
    {
        return CHILD_OBJECT;
    }
    else if ( l_pId == m_tPointId )
    {
        return POINT_DATASET;
    }
    return 0;
}
void MainFrame::onRotateZ( wxCommandEvent& event )
{
    m_pDatasetHelper->m_theScene->m_isRotateZ = !m_pDatasetHelper->m_theScene->m_isRotateZ; 
    setTimerSpeed();
}

void MainFrame::onRotateY( wxCommandEvent& event )
{
    m_pDatasetHelper->m_theScene->m_isRotateY = !m_pDatasetHelper->m_theScene->m_isRotateY; 
    setTimerSpeed();
}

void MainFrame::onRotateX( wxCommandEvent& event )
{
    m_pDatasetHelper->m_theScene->m_isRotateX = !m_pDatasetHelper->m_theScene->m_isRotateX; 
    setTimerSpeed();
}

void MainFrame::onNavigateAxial( wxCommandEvent& event )
{
    m_pDatasetHelper->m_theScene->m_isNavAxial = !m_pDatasetHelper->m_theScene->m_isNavAxial;
    setTimerSpeed();
}

void MainFrame::onNavigateSagital( wxCommandEvent& event )
{
    m_pDatasetHelper->m_theScene->m_isNavSagital = !m_pDatasetHelper->m_theScene->m_isNavSagital;    
    setTimerSpeed();  
}

void MainFrame::onNavigateCoronal( wxCommandEvent& event )
{
    m_pDatasetHelper->m_theScene->m_isNavCoronal = !m_pDatasetHelper->m_theScene->m_isNavCoronal;
    setTimerSpeed();
}

void MainFrame::setTimerSpeed()
{
    m_pTimer->Stop();
    if(m_pDatasetHelper->m_theScene->m_isNavCoronal || m_pDatasetHelper->m_theScene->m_isNavAxial || m_pDatasetHelper->m_theScene->m_isNavSagital || m_pDatasetHelper->m_theScene->m_isRotateZ
        || m_pDatasetHelper->m_theScene->m_isRotateY || m_pDatasetHelper->m_theScene->m_isRotateX)
    {        
        m_pTimer->Start( 50 );
    }
    else
    {
        m_pTimer->Start( 100 );
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

void MainFrame::onSize( wxSizeEvent& WXUNUSED(event) )
{
    doOnSize();
}

void MainFrame::doOnSize()
{
    wxSize l_clientSize = this->GetClientSize();  
    if( GetSizer() )
    {
        GetSizer()->SetDimension(0, 0, l_clientSize.x, l_clientSize.y );
    }
    if( m_pMainGL != NULL)
    {
        m_pMainGL->changeOrthoSize();
    }
    m_pPropertiesWindow->SetMinSize(wxSize(220, l_clientSize.y - 236));
    m_pPropertiesWindow->GetSizer()->SetDimension(0,0,220, l_clientSize.y - 236);
}

///////////////////////////////////////////////////////////////////////////
// Gets called when a thread for the kdTree creation finishes this function
// is here because of some limitations in the event handling system.
///////////////////////////////////////////////////////////////////////////
void MainFrame::onKdTreeThreadFinished( wxCommandEvent& WXUNUSED(event) )
{
    m_pDatasetHelper->treeFinished();
}

///////////////////////////////////////////////////////////////////////////
// OnGLEvent handles mouse events in the GL Refreshing widgets.
///////////////////////////////////////////////////////////////////////////
void MainFrame::onGLEvent( wxCommandEvent &event )
{
    wxPoint l_pos, l_newPos;
    int NAV_GL_SIZE = m_pGL0->GetSize().x;

    switch( event.GetInt() )
    {
        case AXIAL:
        {
            l_pos = m_pGL0->getMousePos();
            float x = ( (float)l_pos.x / NAV_GL_SIZE * m_pDatasetHelper->m_columns );
            float y = ( (float)( NAV_GL_SIZE - l_pos.y ) / NAV_GL_SIZE * m_pDatasetHelper->m_rows );

            m_pXSlider->SetValue( (int)( x ) );
            m_pYSlider->SetValue( (int)( y ) );
            break;
        }
        case CORONAL:
        {
            l_pos = m_pGL1->getMousePos();
            float x = ( (float)l_pos.x / NAV_GL_SIZE ) * m_pDatasetHelper->m_columns;
            float y = ( (float)( NAV_GL_SIZE - l_pos.y ) / NAV_GL_SIZE * m_pDatasetHelper->m_frames );

            m_pXSlider->SetValue( (int)( x ) );
            m_pZSlider->SetValue( (int)( y ) );
            break;
        }
        case SAGITTAL:
        {
            l_pos = m_pGL2->getMousePos();
            float x = ( (float)( NAV_GL_SIZE - l_pos.x ) / NAV_GL_SIZE ) * m_pDatasetHelper->m_rows;
            float y = ( (float)( NAV_GL_SIZE - l_pos.y ) / NAV_GL_SIZE * m_pDatasetHelper->m_frames );

            m_pYSlider->SetValue( (int)( x ) );
            m_pZSlider->SetValue( (int)( y ) );
            break;
        }
        case MAIN_VIEW:
            int delta = (int) m_pMainGL->getDelta();
                switch( m_pMainGL->getPicked() )
                {
                    case AXIAL:
                        m_pZSlider->SetValue( (int) wxMin( wxMax( m_pZSlider->GetValue() + delta, 0 ), m_pZSlider->GetMax() ) );
                        break;
                    case CORONAL:
                        m_pYSlider->SetValue( (int) wxMin( wxMax( m_pYSlider->GetValue() + delta, 0 ), m_pYSlider->GetMax() ) );
                        break;
                    case SAGITTAL:
                        m_pXSlider->SetValue( (int) wxMin( wxMax( m_pXSlider->GetValue() + delta, 0 ), m_pXSlider->GetMax() ) );
                        break;
                }                
    }
    m_pDatasetHelper->updateView( m_pXSlider->GetValue(), m_pYSlider->GetValue(), m_pZSlider->GetValue() );
    updateStatusBar();
    refreshAllGLWidgets();
}

void MainFrame::onMouseEvent( wxMouseEvent& WXUNUSED(event) )
{
    //this->Refresh();
}

void MainFrame::updateMenus()
{
    m_pMenuBar->updateMenuBar(this);
    m_pToolBar->updateToolBar(this); 
}

void MainFrame::onTimerEvent( wxTimerEvent& WXUNUSED(event) )
{    
    //Rotate animation
    if(m_pDatasetHelper->m_theScene->m_isRotateZ)
    {
        m_pDatasetHelper->m_theScene->m_rotAngleZ++;
    }
    if(m_pDatasetHelper->m_theScene->m_isRotateY)
    {
        m_pDatasetHelper->m_theScene->m_rotAngleY++;
    }
    if(m_pDatasetHelper->m_theScene->m_isRotateX)
    {
        m_pDatasetHelper->m_theScene->m_rotAngleX++;
    }    
    //Navigate through slizes sagital
    if(m_pDatasetHelper->m_theScene->m_isNavSagital)
    {
        m_pDatasetHelper->m_theScene->m_posSagital++;
    }
    else
    {
        m_pDatasetHelper->m_theScene->m_posSagital = m_pDatasetHelper->m_xSlize;
    }
    //Navigate through slizes axial
    if(m_pDatasetHelper->m_theScene->m_isNavAxial)
    {
        m_pDatasetHelper->m_theScene->m_posAxial++;
    }
    else
    {
        m_pDatasetHelper->m_theScene->m_posAxial = m_pDatasetHelper->m_zSlize;
    }
    //Navigate through slizes coronal
    if(m_pDatasetHelper->m_theScene->m_isNavCoronal)
    {
        m_pDatasetHelper->m_theScene->m_posCoronal++;
    }
    else
    {
        m_pDatasetHelper->m_theScene->m_posCoronal = m_pDatasetHelper->m_ySlize;
    }
    refreshAllGLWidgets();
    refreshViews();
    m_pDatasetHelper->increaseAnimationStep();    
}
