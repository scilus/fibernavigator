/////////////////////////////////////////////////////////////////////////////
// Description: mainFrame class. Contains every elements of the GUI, and frame events
/////////////////////////////////////////////////////////////////////////////

#include "MainFrame.h"

#include "MainCanvas.h"
#include "MenuBar.h"
#include "PropertiesWindow.h"
#include "SelectionBox.h"
#include "SelectionEllipsoid.h"
#include "ToolBar.h"
#include "../main.h"
#include "../Logger.h"
#include "../dataset/Anatomy.h"
#include "../dataset/DatasetManager.h"
#include "../dataset/Fibers.h"
#include "../dataset/FibersGroup.h"
#include "../dataset/Loader.h"
#include "../dataset/ODFs.h"
#include "../dataset/SplinePoint.h"
#include "../dataset/Surface.h"
#include "../dataset/Tensors.h"
#include "../gfx/TheScene.h"
#include "../gui/SceneManager.h"
#include "../misc/IsoSurface/CIsoSurface.h"

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/colordlg.h>
#include <wx/filedlg.h>
#include <wx/imaglist.h>
#include <wx/statbmp.h>
#include <wx/vscroll.h>

extern const wxEventType wxEVT_NAVGL_EVENT;

/****************************************************************************************************
 *
 * Event Table
 *
 ****************************************************************************************************/
BEGIN_EVENT_TABLE( MainFrame, wxFrame )
// List widget events
EVT_LIST_ITEM_ACTIVATED  ( ID_LIST_CTRL2,                   MainFrame::onActivateListItem2  )
EVT_LIST_ITEM_SELECTED   ( ID_LIST_CTRL2,                   MainFrame::onSelectListItem2    )
EVT_LIST_ITEM_DESELECTED ( ID_LIST_CTRL2,                   MainFrame::onDeselectListItem2  )
EVT_LIST_DELETE_ITEM     ( ID_LIST_CTRL2,                   MainFrame::onDeleteListItem2    )

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

namespace
{
// Properties to define GUI elements
#define NOT_DEFINED         -1

#define CANVAS_AXI_WIDTH    175
#define CANVAS_AXI_HEIGHT   175
#define CANVAS_COR_WIDTH    175
#define CANVAS_COR_HEIGHT   175
#define CANVAS_SAG_WIDTH    175
#define CANVAS_SAG_HEIGHT   175

#define LIST_WIDTH          268
#define LIST_HEIGHT         125
#define LIST_COL0_WIDTH     34
#define LIST_COL1_WIDTH     160
#define LIST_COL2_WIDTH     50
#define LIST_COL3_WIDTH     20

#define PROP_WND_WIDTH      260
#define PROP_WND_HEIGHT     350

#define SLIDER_AXI_WIDTH    CANVAS_AXI_WIDTH
#define SLIDER_AXI_HEIGHT   NOT_DEFINED
#define SLIDER_COR_WIDTH    CANVAS_COR_WIDTH
#define SLIDER_COR_HEIGHT   NOT_DEFINED
#define SLIDER_SAG_WIDTH    CANVAS_SAG_WIDTH
#define SLIDER_SAG_HEIGHT   NOT_DEFINED

#define TREE_WIDTH          268
#define TREE_HEIGHT         NOT_DEFINED

    void initMyTreeCtrl( MyTreeCtrl * &myTreeCtrl )
    {
        myTreeCtrl->SetMaxSize( wxSize( TREE_WIDTH, TREE_HEIGHT ) );
        myTreeCtrl->SetMinSize( wxSize( TREE_WIDTH, 100 ) );

        wxImageList* tImageList = new wxImageList( 16, 16 );

        tImageList->Add( wxImage( MyApp::respath + _T( "icons/delete.png" ), wxBITMAP_TYPE_PNG ) );
        tImageList->Add( wxImage( MyApp::respath + _T( "icons/eyes.png" ),   wxBITMAP_TYPE_PNG ) );

        myTreeCtrl->AssignImageList( tImageList );
    }

    void initListCtrl( ListCtrl * &lstCtrl )
    {
        lstCtrl->SetMaxSize( wxSize( LIST_WIDTH, LIST_HEIGHT ) );
        lstCtrl->SetMinSize( wxSize( LIST_WIDTH, LIST_HEIGHT ) );

        wxImageList* pImageList = new wxImageList( 16, 16 );

        pImageList->Add( ( wxImage( MyApp::respath + _T( "icons/eyes.png"   ),      wxBITMAP_TYPE_PNG ) ) );
        pImageList->Add( ( wxImage( MyApp::respath + _T( "icons/eyes_hidden.png" ), wxBITMAP_TYPE_PNG ) ) );
        pImageList->Add( ( wxImage( MyApp::respath + _T( "icons/delete.png" ),      wxBITMAP_TYPE_PNG ) ) );

        lstCtrl->AssignImageList(pImageList, wxIMAGE_LIST_SMALL);

        wxListItem displayCol, nameCol, thresholdCol, deleteCol;

        displayCol.SetText( wxT( "Display" ) );

        nameCol.SetText( wxT( "Name" ) );
        nameCol.SetAlign( wxLIST_FORMAT_LEFT );

        thresholdCol.SetText( wxT( "Threshold" ) );
        thresholdCol.SetAlign( wxLIST_FORMAT_RIGHT );

        deleteCol.SetText( wxT( "Delete" ) );

        lstCtrl->InsertColumn( 0, displayCol );
        lstCtrl->InsertColumn( 1, nameCol );
        lstCtrl->InsertColumn( 2, thresholdCol ) ;
        lstCtrl->InsertColumn( 3, deleteCol );

        lstCtrl->SetColumnWidth( 0, LIST_COL0_WIDTH );
        lstCtrl->SetColumnWidth( 1, LIST_COL1_WIDTH );
        lstCtrl->SetColumnWidth( 2, LIST_COL2_WIDTH );
        lstCtrl->SetColumnWidth( 3, LIST_COL3_WIDTH );
    }
}

#define FIBERS_INFO_GRID_COL_SIZE              1
#define FIBERS_INFO_GRID_ROW_SIZE              11
#define FIBERS_INFO_GRID_TITLE_LABEL_SIZE      150


MainFrame::MainFrame(wxWindow           *i_parent, 
                     const wxWindowID   i_id, 
                     const wxString     &i_title, 
                     const wxPoint      &i_pos, 
                     const wxSize       &i_size, 
                     const long         i_style)
:   wxFrame( i_parent, i_id, i_title, i_pos, i_size, i_style ),
    m_pLastSelectedSceneObject( NULL ),
    m_lastSelectedListItem( -1 ),
    m_pCurrentSceneObject( NULL ),
    m_currentListItem( -1 ),
    m_pCurrentSizer( NULL ),
    m_lastPath( MyApp::respath + _T( "data" ) ),
    m_isDrawerToolActive( false ),
    m_drawSize(2),
    m_drawRound ( true ),
    m_draw3d ( false ),
    m_canUseColorPicker( false ),
    m_drawColor(255, 255, 255),
    m_drawColorIcon(16, 16, true),
    m_threadsActive( 0 ),
    m_pLastSelectedObj( NULL ),
    m_pLastSelectedPoint( NULL )
{
    wxImage::AddHandler(new wxPNGHandler);

    m_pDatasetHelper = new DatasetHelper();
    // TODO: Remove dependency to DatasetHelper
    DatasetManager::getInstance()->setDatasetHelper( m_pDatasetHelper );
    SceneManager::getInstance()->setTheScene( new TheScene( m_pDatasetHelper ) );

    //////////////////////////////////////////////////////////////////////////
    // MyTreeCtrl initialization
    m_pTreeWidget = new MyTreeCtrl( this, ID_TREE_CTRL, wxDefaultPosition, wxSize( TREE_WIDTH, TREE_HEIGHT ), wxTR_HAS_BUTTONS | wxTR_SINGLE | wxTR_HIDE_ROOT );
    initMyTreeCtrl( m_pTreeWidget );

    m_tRootId             = m_pTreeWidget->AddRoot( wxT( "Scene" ), -1, -1, NULL );
    m_tPointId            = m_pTreeWidget->AppendItem( m_tRootId, wxT( "Points" ), -1, -1, NULL );
    m_tSelectionObjectsId = m_pTreeWidget->AppendItem( m_tRootId, wxT( "Selection Objects" ), -1, -1, NULL );

    //////////////////////////////////////////////////////////////////////////
    // ListCtrl initialization
    m_pListCtrl2 = new ListCtrl( this, wxDefaultPosition, wxSize( LIST_WIDTH, LIST_HEIGHT ), wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_NO_HEADER );
    initListCtrl( m_pListCtrl2 );


    //////////////////////////////////////////////////////////////////////////
    // PropertiesWindow initialization
    m_pPropertiesWindow = new PropertiesWindow( this, wxID_ANY, wxDefaultPosition, wxSize( PROP_WND_WIDTH, PROP_WND_HEIGHT ), m_pListCtrl2 ); // Contains Scene Objects properties
    m_pPropertiesWindow->SetScrollbars( 10, 10, 50, 50 );
    m_pPropertiesWindow->EnableScrolling( false, true );

    //////////////////////////////////////////////////////////////////////////
    // OpenGL initialization
    initOpenGl();

    //////////////////////////////////////////////////////////////////////////
    // initLayout
    initLayout();

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


void MainFrame::initOpenGl()
{
    Logger::getInstance()->print( wxT( "Initializing OpenGL" ), LOGLEVEL_MESSAGE );

#ifdef __WXMSW__
    int *gl_attrib = NULL;
#else
    int gl_attrib[20] = 
    { WX_GL_RGBA, WX_GL_MIN_RED, 1, WX_GL_MIN_GREEN, 1, WX_GL_MIN_BLUE, 1, WX_GL_DEPTH_SIZE, 1, WX_GL_DOUBLEBUFFER,
    #if defined(__WXMAC__) || defined(__WXCOCOA__)
        GL_NONE };
    #else
        None };
    #endif
#endif

    m_pMainGL = new MainCanvas( m_pDatasetHelper, MAIN_VIEW, this, ID_GL_MAIN,  wxDefaultPosition, wxDefaultSize,     0, _T( "MainGLCanvas" ), gl_attrib );

#ifndef CTX
    m_pGL0 = new MainCanvas( m_pDatasetHelper,       AXIAL, this, ID_GL_NAV_X, wxDefaultPosition, wxSize( CANVAS_AXI_WIDTH, CANVAS_AXI_HEIGHT ), 0, _T( "NavGLCanvasX" ), gl_attrib, m_pMainGL );
    m_pGL1 = new MainCanvas( m_pDatasetHelper,     CORONAL, this, ID_GL_NAV_Y, wxDefaultPosition, wxSize( CANVAS_COR_WIDTH, CANVAS_COR_HEIGHT ), 0, _T( "NavGLCanvasY" ), gl_attrib, m_pMainGL );
    m_pGL2 = new MainCanvas( m_pDatasetHelper,    SAGITTAL, this, ID_GL_NAV_Z, wxDefaultPosition, wxSize( CANVAS_SAG_WIDTH, CANVAS_SAG_HEIGHT ), 0, _T( "NavGLCanvasZ" ), gl_attrib, m_pMainGL );
#else
    m_pGL0 = new MainCanvas( m_pDatasetHelper, axial,    m_topNavWindow,    ID_GL_NAV_X, wxDefaultPosition, wxSize( CANVAS_AXI_WIDTH, CANVAS_AXI_HEIGTH ), 0, _T( "NavGLCanvasX" ), gl_attrib, m_pMainGL->GetContext() );
    m_pGL1 = new MainCanvas( m_pDatasetHelper, coronal,  m_middleNavWindow, ID_GL_NAV_Y, wxDefaultPosition, wxSize( CANVAS_COR_WIDTH, CANVAS_COR_HEIGHT ), 0, _T( "NavGLCanvasY" ), gl_attrib, m_pMainGL->GetContext() );
    m_pGL2 = new MainCanvas( m_pDatasetHelper, sagittal, m_bottomNavWindow, ID_GL_NAV_Z, wxDefaultPosition, wxSize( CANVAS_SAG_WIDTH, CANVAS_SAG_HEIGHT ), 0, _T( "NavGLCanvasZ" ), gl_attrib, m_pMainGL->GetContext() );
#endif

    m_pGL0->SetMaxSize( wxSize( CANVAS_AXI_WIDTH, CANVAS_AXI_HEIGHT ) );
    m_pGL1->SetMaxSize( wxSize( CANVAS_COR_WIDTH, CANVAS_COR_HEIGHT ) );
    m_pGL2->SetMaxSize( wxSize( CANVAS_SAG_WIDTH, CANVAS_SAG_HEIGHT ) );

#ifndef __WXMAC__
    SceneManager::getInstance()->getScene()->setMainGLContext( new wxGLContext( m_pMainGL ) );
    glGetError(); // Removes the error code so we don't have an error message the first time we check it
#else
    SceneManager::getInstance()->getScene()->setMainGLContext( m_pMainGL->GetContext() );
#endif

}

void MainFrame::initLayout()
{
    //////////////////////////////////////////////////////////////////////////
    // Slider initialization
    m_pXSlider  = new wxSlider( this, ID_X_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( SLIDER_SAG_WIDTH, SLIDER_SAG_HEIGHT ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pYSlider  = new wxSlider( this, ID_Y_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( SLIDER_COR_WIDTH, SLIDER_COR_HEIGHT ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    m_pZSlider  = new wxSlider( this, ID_Z_SLIDER,  50, 0, 100, wxDefaultPosition, wxSize( SLIDER_AXI_WIDTH, SLIDER_AXI_HEIGHT ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );

    //////////////////////////////////////////////////////////////////////////
    // Sizer initialization
    wxBoxSizer *xSzr       = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *ySzr       = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *zSzr       = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *axesSzr    = new wxBoxSizer( wxHORIZONTAL );
    wxBoxSizer *lstSzr     = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *propSzr    = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *lwrLeftSzr = new wxBoxSizer( wxHORIZONTAL );
    wxBoxSizer *leftSzr    = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *mainSzr    = new wxBoxSizer( wxHORIZONTAL );

    xSzr->Add( m_pGL2,     0, wxALL | wxFIXED_MINSIZE, 2 );
    xSzr->Add( m_pXSlider, 0, wxALL | wxFIXED_MINSIZE, 2 );
    xSzr->SetMinSize( wxSize( SLIDER_SAG_WIDTH + 4, CANVAS_SAG_HEIGHT + 4 + 16 ) );

    ySzr->Add( m_pGL1,     0, wxALL | wxFIXED_MINSIZE, 2 );
    ySzr->Add( m_pYSlider, 0, wxALL | wxFIXED_MINSIZE, 2 );
    ySzr->SetMinSize( wxSize( SLIDER_COR_WIDTH + 4, CANVAS_COR_HEIGHT + 4 + 16 ) );

    zSzr->Add( m_pGL0,     0, wxALL | wxFIXED_MINSIZE, 2 );
    zSzr->Add( m_pZSlider, 0, wxALL | wxFIXED_MINSIZE, 2 );
    zSzr->SetMinSize( wxSize( SLIDER_AXI_WIDTH + 4, CANVAS_AXI_HEIGHT + 4 + 16 ) );

    axesSzr->Add( xSzr, 0, wxALL | wxFIXED_MINSIZE, 0 );
    axesSzr->Add( ySzr, 0, wxALL | wxFIXED_MINSIZE, 0 );
    axesSzr->Add( zSzr, 0, wxALL | wxFIXED_MINSIZE, 0 );

//    lstSzr->Add( m_pListCtrl, 0, wxALL | wxFIXED_MINSIZE, 2 );
    lstSzr->Add( (wxWindow *)m_pListCtrl2, 0, wxALL | wxFIXED_MINSIZE, 2 );
    lstSzr->Add( m_pTreeWidget, 1, wxALL | wxFIXED_MINSIZE | wxEXPAND, 2 );

    propSzr->Add( m_pPropertiesWindow, 1, wxALL | wxFIXED_MINSIZE | wxEXPAND, 4 );

    lwrLeftSzr->Add( lstSzr, 0, wxALL | wxFIXED_MINSIZE | wxEXPAND, 2 );
    lwrLeftSzr->Add( propSzr, 1, wxALL | wxEXPAND, 2 );

    leftSzr->Add( axesSzr, 0, wxALL | wxFIXED_MINSIZE, 2 );
    leftSzr->Add( lwrLeftSzr, 1, wxALL | wxFIXED_MINSIZE | wxEXPAND, 0);

    mainSzr->Add( leftSzr, 0, wxALL | wxFIXED_MINSIZE, 0 );
    mainSzr->Add( m_pMainGL, 1, wxALL | wxEXPAND, 2 );

    SetSizer( mainSzr );
    SetBackgroundColour( *wxLIGHT_GREY );
    SetAutoLayout( true );
    Layout();
    Fit();
}


void MainFrame::onLoad( wxCommandEvent& WXUNUSED(event) )
{
    wxArrayString l_fileNames;
    wxString l_caption          = wxT( "Choose a file" );
    wxString l_wildcard         = wxT( "*.*|*.*|Nifti (*.nii)|*.nii*|Mesh files (*.mesh)|*.mesh|Mesh files (*.surf)|*.surf|Mesh files (*.dip)|*.dip|Fibers VTK/DMRI (*.fib)|*.fib|Fibers PTK (*.bundlesdata)|*.bundlesdata|Fibers TrackVis (*.trk)|*.trk|Fibers MRtrix (*.tck)|*.tck|Scene Files (*.scn)|*.scn|Tensor files (*.nii*)|*.nii|ODF files (*.nii)|*.nii*" );
    wxString l_defaultDir       = wxEmptyString;
    wxString l_defaultFileName  = wxEmptyString;
    wxFileDialog dialog( this, l_caption, l_defaultDir, l_defaultFileName, l_wildcard, wxOPEN | wxFD_MULTIPLE );
    dialog.SetFilterIndex( 0 );
    dialog.SetDirectory( m_lastPath );
    if( dialog.ShowModal() == wxID_OK )
    {
        m_lastPath = dialog.GetDirectory();
        dialog.GetPaths( l_fileNames );
    }

    if ( for_each( l_fileNames.begin(), l_fileNames.end(), Loader( this, m_pListCtrl2 ) ).getError() )
    {
        wxMessageBox( Logger::getInstance()->getLastError(), wxT( "Error while loading" ), wxOK | wxICON_INFORMATION, NULL );
        GetStatusBar()->SetStatusText( wxT( "ERROR" ), 1 );
        GetStatusBar()->SetStatusText( Logger::getInstance()->getLastError(), 2 );
        return;
    }

    refreshAllGLWidgets();
}

//
//This function creates an Anatomy from scratch
//
void MainFrame::createNewAnatomy( DatasetType dataType )
{
	// ask user for a name
	wxString l_givenName = wxT("Anatomy");
    wxTextEntryDialog dialog(this, _T( "Please enter a new name" ) );
    dialog.SetValue( l_givenName );
    if( ( dialog.ShowModal() == wxID_OK ) && ( dialog.GetValue() != _T( "" ) ) )
	{
        l_givenName = dialog.GetValue();
	}

	//create the anatomy
    int index = DatasetManager::getInstance()->createAnatomy( dataType );
    Anatomy* pNewAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( index );
	pNewAnatomy->setName( l_givenName );

    m_pListCtrl2->InsertItem( index );

    refreshAllGLWidgets();
}

//////////////////////////////////////////////////////////////////////////

void MainFrame::updateSliders()
{
    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();

    m_pXSlider->SetMax( wxMax( 2, columns - 1 ) );
    m_pXSlider->SetValue( columns / 2 );
    m_pYSlider->SetMax( wxMax( 2, rows - 1 ) );
    m_pYSlider->SetValue( rows / 2 );
    m_pZSlider->SetMax( wxMax( 2, frames - 1 ) );
    m_pZSlider->SetValue( frames / 2 );

    SceneManager::getInstance()->updateView( m_pXSlider->GetValue(), m_pYSlider->GetValue(), m_pZSlider->GetValue() );

    m_pMainGL->changeOrthoSize();
    m_pGL0->changeOrthoSize();
    m_pGL1->changeOrthoSize();
    m_pGL2->changeOrthoSize();
}

//////////////////////////////////////////////////////////////////////////

void MainFrame::onNewAnatomyByte( wxCommandEvent& WXUNUSED(event) )
{
	createNewAnatomy( HEAD_BYTE );
}

void MainFrame::onNewAnatomyRGB( wxCommandEvent& WXUNUSED(event) )
{
	createNewAnatomy( RGB );
}

void MainFrame::onSave( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( _T("Event triggered - MainFrame::onSave"), LOGLEVEL_DEBUG );

    wxString caption         = wxT( "Choose a file" );
    wxString wildcard        = wxT( "Scene files (*.scn)|*.scn|*.*|*.*" );
    wxString defaultDir      = wxEmptyString;
    wxString defaultFilename = wxEmptyString;
    wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
    dialog.SetFilterIndex( 0 );
    dialog.SetDirectory( m_lastPath );

    if( SceneManager::getInstance()->isSceneFileLoaded() )
    {
        dialog.SetFilename( SceneManager::getInstance()->getSceneFilename() );
    }

    dialog.SetDirectory( SceneManager::getInstance()->getScenePath() );

    if( dialog.ShowModal() == wxID_OK )
    {
        SceneManager::getInstance()->setScenePath( dialog.GetDirectory() );
        SceneManager::getInstance()->save( dialog.GetPath() );
    }
}

void MainFrame::onSaveFibers( wxCommandEvent& WXUNUSED(event) )
{
    Logger::getInstance()->print( _T("Event triggered - MainFrame::onSaveFibers"), LOGLEVEL_DEBUG );

    if( !DatasetManager::getInstance()->isFibersLoaded() )
    {
        return;
    }
 
    wxString caption         = wxT( "Choose a file" );
    wxString wildcard        = wxT( "VTK fiber files (*.fib)|*.fib|DMRI fiber files (*.fib)|*.fib|*.*|*.*" );
    wxString defaultDir      = wxEmptyString;
    wxString defaultFilename = wxEmptyString;
    wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
    dialog.SetFilterIndex( 0 );
    dialog.SetDirectory( m_lastPath );

	if (m_pCurrentSceneObject != NULL && m_currentListItem != -1)
    {
		DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pCurrentSceneObject);
		if( dialog.ShowModal() == wxID_OK )
		{
            m_lastPath = dialog.GetDirectory();
			
			if( pDatasetInfo->getType() == FIBERS )
			{
				Fibers* l_fibers = DatasetManager::getInstance()->getSelectedFibers( getCurrentListItem() );
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
                FibersGroup* l_fibersGroup = DatasetManager::getInstance()->getFibersGroup();
				
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
    Logger::getInstance()->print( _T("Event triggered - MainFrame::onSaveDataset"), LOGLEVEL_DEBUG );

    if( m_pCurrentSceneObject != NULL && m_currentListItem != -1 )
    {
        if( ((DatasetInfo*)m_pCurrentSceneObject)->getType() < MESH )
        {
            Anatomy* l_anatomy = (Anatomy*)m_pCurrentSceneObject;

            wxString caption         = wxT( "Choose a file" );
            wxString wildcard        = wxT( "Nifti (*.nii)|*.nii*|All files|*.*" );
            wxString defaultDir      = wxEmptyString;
            wxString defaultFilename = wxEmptyString;
            wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
            dialog.SetFilterIndex( 0 );
            dialog.SetDirectory( m_lastPath );

            if( dialog.ShowModal() == wxID_OK )
            {
                m_lastPath = dialog.GetDirectory();
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
            wxString wildcard        = wxT( "surface files (*.vtk)|*.vtk" );
            wxString defaultDir      = wxEmptyString;
            wxString defaultFilename = wxEmptyString;
            wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
            dialog.SetFilterIndex( 0 );
            dialog.SetDirectory( m_lastPath );
            if( dialog.ShowModal() == wxID_OK )
            {
                m_lastPath = dialog.GetDirectory();
                l_surface->save( dialog.GetPath() );
            }
        }
        else if( ((DatasetInfo*)m_pCurrentSceneObject)->getType() == ISO_SURFACE )
        {
            CIsoSurface *l_surface = (CIsoSurface*)m_pCurrentSceneObject;

            wxString caption         = wxT( "Choose a file" );
            wxString wildcard        = wxT( "surface files (*.vtk)|*.vtk" );
            wxString defaultDir      = wxEmptyString;
            wxString defaultFilename = wxEmptyString;
            wxFileDialog dialog( this, caption, defaultDir, defaultFilename, wildcard, wxSAVE );
            dialog.SetFilterIndex( 0 );
            dialog.SetDirectory( m_lastPath );
            if( dialog.ShowModal() == wxID_OK )
            {
                m_lastPath = dialog.GetDirectory();
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
    SceneManager::getInstance()->setZoom( 1.0f );
    SceneManager::getInstance()->setMoveX( 0.0f );
    SceneManager::getInstance()->setMoveY( 0.0f );
//     m_pDatasetHelper->m_zoom  = 1;
//     m_pDatasetHelper->m_xMove = 0;
//     m_pDatasetHelper->m_yMove = 0;
    refreshAllGLWidgets();
}

void MainFrame::onMenuViewLeft( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fT transform = SceneManager::getInstance()->getTransform();
    Matrix4fSetIdentity( &transform );
    transform.s.M00 = 0.0f;
    transform.s.M11 =  0.0f;
    transform.s.M22 =  0.0f;
    transform.s.M20 = -1.0f;
    transform.s.M01 = -1.0f;
    transform.s.M12 =  1.0f;

//     Matrix4fSetIdentity( &m_pDatasetHelper->m_transform );
//     m_pDatasetHelper->m_transform.s.M00 =  0.0f;
//     m_pDatasetHelper->m_transform.s.M11 =  0.0f;
//     m_pDatasetHelper->m_transform.s.M22 =  0.0f;
//     m_pDatasetHelper->m_transform.s.M20 = -1.0f;
//     m_pDatasetHelper->m_transform.s.M01 = -1.0f;
//     m_pDatasetHelper->m_transform.s.M12 =  1.0f;
    m_pMainGL->setRotation();
}

void MainFrame::onMenuViewRight( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fT transform = SceneManager::getInstance()->getTransform();
    Matrix4fSetIdentity( &transform );
    transform.s.M00 = 0.0f;
    transform.s.M11 = 0.0f;
    transform.s.M22 = 0.0f;
    transform.s.M20 = 1.0f;
    transform.s.M01 = 1.0f;
    transform.s.M12 = 1.0f;

//     Matrix4fSetIdentity( &m_pDatasetHelper->m_transform );
//     m_pDatasetHelper->m_transform.s.M00 = 0.0f;
//     m_pDatasetHelper->m_transform.s.M11 = 0.0f;
//     m_pDatasetHelper->m_transform.s.M22 = 0.0f;
//     m_pDatasetHelper->m_transform.s.M20 = 1.0f;
//     m_pDatasetHelper->m_transform.s.M01 = 1.0f;
//     m_pDatasetHelper->m_transform.s.M12 = 1.0f;
    m_pMainGL->setRotation();
}

void MainFrame::onMenuViewTop( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fSetIdentity( &SceneManager::getInstance()->getTransform() );
    m_pMainGL->setRotation();
}

void MainFrame::onMenuViewBottom( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fT transform = SceneManager::getInstance()->getTransform();
    Matrix4fSetIdentity( &transform );
    transform.s.M00 = -1.0f;
    transform.s.M22 = -1.0f;

//     Matrix4fSetIdentity( &m_pDatasetHelper->m_transform );
//     m_pDatasetHelper->m_transform.s.M00 = -1.0f;
//     m_pDatasetHelper->m_transform.s.M22 = -1.0f;
    m_pMainGL->setRotation();
}

void MainFrame::onMenuViewFront( wxCommandEvent& WXUNUSED(event) )
{
    Matrix4fT transform = SceneManager::getInstance()->getTransform();
    Matrix4fSetIdentity( &transform );
    transform.s.M11 =  0.0f;
    transform.s.M22 =  0.0f;
    transform.s.M12 =  1.0f;
    transform.s.M21 = -1.0f;

//     Matrix4fSetIdentity( &m_pDatasetHelper->m_transform );
//     m_pDatasetHelper->m_transform.s.M11 =  0.0f;
//     m_pDatasetHelper->m_transform.s.M22 =  0.0f;
//     m_pDatasetHelper->m_transform.s.M12 =  1.0f;
//     m_pDatasetHelper->m_transform.s.M21 = -1.0f;
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
    Matrix4fT transform = SceneManager::getInstance()->getTransform();
    Matrix4fSetIdentity( &transform );
    transform.s.M00 = -1.0f;
    transform.s.M11 =  0.0f;
    transform.s.M22 =  0.0f;
    transform.s.M21 =  1.0f;
    transform.s.M12 =  1.0f;

//     Matrix4fSetIdentity( &m_pDatasetHelper->m_transform );
//     m_pDatasetHelper->m_transform.s.M00 = -1.0f;
//     m_pDatasetHelper->m_transform.s.M11 =  0.0f;
//     m_pDatasetHelper->m_transform.s.M22 =  0.0f;
//     m_pDatasetHelper->m_transform.s.M21 =  1.0f;
//     m_pDatasetHelper->m_transform.s.M12 =  1.0f;
    m_pMainGL->setRotation();
}

void MainFrame::onMenuViewAxes( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->toggleAxesDisplay();
}

void MainFrame::onMenuViewCrosshair( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->toggleCrosshairDisplay();
    //m_pDatasetHelper->m_showCrosshair = !m_pDatasetHelper->m_showCrosshair;
    refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 * Menu VOI
 *
 ****************************************************************************************************/

void MainFrame::onToggleSelectionObjects( wxCommandEvent& WXUNUSED(event) )
{
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
    SceneManager::getInstance()->togglePointMode();
    refreshAllGLWidgets();
}

void MainFrame::onSelectDrawer( wxCommandEvent& event )
{
	m_isDrawerToolActive = true;
    updateDrawerToolbar();
}

void MainFrame::onSwitchDrawer( wxCommandEvent& event )
{
    m_isDrawerToolActive = !m_isDrawerToolActive;
    updateDrawerToolbar();
}

void MainFrame::updateDrawerToolbar()
{
    SceneManager::getInstance()->setRulerActive( false );
    
    m_pToolBar->m_txtRuler->Disable();
    
	m_pToolBar->EnableTool( m_pToolBar->m_toggleDrawRound->GetId(), m_isDrawerToolActive );
	m_pToolBar->EnableTool( m_pToolBar->m_toggleDraw3d->GetId(), m_isDrawerToolActive );
	m_pToolBar->EnableTool( m_pToolBar->m_selectPen->GetId(), m_isDrawerToolActive );
	m_pToolBar->EnableTool( m_pToolBar->m_selectEraser->GetId(), m_isDrawerToolActive );
    m_pToolBar->EnableTool( m_pToolBar->m_selectColorPicker->GetId(), m_isDrawerToolActive );
    
    // Check if the current anatomy supports RGB
    Anatomy *pTempAnat = (Anatomy*) m_pCurrentSceneObject;
    
    if( pTempAnat != NULL && pTempAnat->getType() == RGB )
    {
        m_canUseColorPicker = true;
    }
    else
    {
        m_canUseColorPicker = false;
    }
    
	refreshAllGLWidgets();
}

void MainFrame::onToggleDrawRound( wxCommandEvent& event )
{
    m_drawRound = !m_drawRound;
    refreshAllGLWidgets();
}

void MainFrame::onToggleDraw3d( wxCommandEvent& event )
{
    m_draw3d = !m_draw3d;
    refreshAllGLWidgets();
}

void MainFrame::onSelectColorPicker( wxCommandEvent& event )
{
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
    if( dialog.ShowModal() == wxID_OK )
    {
        wxColourData l_retData = dialog.GetColourData();
        m_drawColor = l_retData.GetColour();
        wxRect fullImage(0, 0, 16, 16); //this is valid as long as toolbar items use 16x16 icons
        m_drawColorIcon.SetRGB( fullImage, 
                                m_drawColor.Red(), 
                                m_drawColor.Green(), 
                                m_drawColor.Blue() );
        m_pToolBar->SetToolNormalBitmap(m_pToolBar->m_selectColorPicker->GetId(), wxBitmap( m_drawColorIcon ) );
    }
    else
    {
        return;
    }

/*
    if( m_mainFrame->m_currentListItem != -1 )
    {
        DatasetInfo *l_info = (DatasetInfo*)m_mainFrame->m_pCurrentSceneObject;
        if( l_info->getType() == MESH || l_info->getType() == ISO_SURFACE || l_info->getType() == SURFACE || l_info->getType() == VECTORS)
        {
            l_info->setColor( l_col );
            l_info->setuseTex( false );
            m_mainFrame->m_pListCtrl->SetItem( m_mainFrame->m_currentListItem, 2, wxT( "(") + wxString::Format( wxT( "%.2f" ), l_info->getThreshold() ) + wxT( ")" ) );           
        }
    }
    else if ( m_mainFrame->m_pDatasetHelper->m_lastSelectedObject != NULL )
    {
        SelectionObject *l_selObj = (SelectionObject*)m_mainFrame->m_pCurrentSceneObject;
        if (!l_selObj->getIsMaster())
        {
            wxTreeItemId l_parentId = m_mainFrame->m_pTreeWidget->GetItemParent( m_mainFrame->m_pDatasetHelper->m_lastSelectedObject->GetId());
            l_selObj = (SelectionObject*)m_mainFrame->m_pTreeWidget->GetItemData(l_parentId);
        }
        l_selObj->setFiberColor( l_col);
        l_selObj->setIsDirty( true );
        m_mainFrame->m_pDatasetHelper->m_selBoxChanged = true;
    }    
    m_mainFrame->refreshAllGLWidgets();*/
}

void MainFrame::onSelectStroke1( wxCommandEvent& event )
{
	m_drawSize = 1;
	refreshAllGLWidgets();
}

void MainFrame::onSelectStroke2( wxCommandEvent& event )
{
	m_drawSize = 2;
	refreshAllGLWidgets();
}

void MainFrame::onSelectStroke3( wxCommandEvent& event )
{
	m_drawSize = 3;
	refreshAllGLWidgets();
}

void MainFrame::onSelectStroke4( wxCommandEvent& event )
{
	m_drawSize = 4;
	refreshAllGLWidgets();
}

void MainFrame::onSelectStroke5( wxCommandEvent& event )
{
	m_drawSize = 5;
	refreshAllGLWidgets();
}

void MainFrame::onSelectStroke7( wxCommandEvent& event )
{
	m_drawSize = 7;
	refreshAllGLWidgets();
}

void MainFrame::onSelectStroke10( wxCommandEvent& event )
{
	m_drawSize = 10;
	refreshAllGLWidgets();
}

void MainFrame::onSelectPen( wxCommandEvent& event )
{
	m_drawMode = DRAWMODE_PEN;
	//glBindTexture(GL_TEXTURE_3D, 1);    //Prepare the existing texture for updates
	refreshAllGLWidgets();
}

void MainFrame::onSelectEraser( wxCommandEvent& event )
{
	m_drawMode = DRAWMODE_ERASER;
	//glBindTexture(GL_TEXTURE_3D, 1);    //Prepare the existing texture for updates
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

    DatasetManager::getInstance()->setSurfaceDirty( true );
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
    
    m_pCurrentSceneObject = NULL;
    m_pLastSelectedSceneObject = NULL;
    m_currentListItem = -1;
    m_lastSelectedListItem = -1;
    m_pPropertiesWindow->GetSizer()->Layout();
    doOnSize();
}

//////////////////////////////////////////////////////////////////////////

void MainFrame::createCutDataset()
{
    // check l_anatomy - quit if not present
    if( !DatasetManager::getInstance()->isAnatomyLoaded() )
        return;

    long l_item = getCurrentListItem();
    if( l_item == -1 )
        return;

    DatasetInfo* info = DatasetManager::getInstance()->getDataset( m_pListCtrl2->GetItem( l_item ) );
    if ( info->getType() > OVERLAY )
        return;

    Anatomy* l_anatomy = (Anatomy*) info;

    int index = DatasetManager::getInstance()->createAnatomy();
    Anatomy* pNewAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( index );

    int   columns = DatasetManager::getInstance()->getColumns();
    int   rows    = DatasetManager::getInstance()->getRows();
    int   frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    pNewAnatomy->setZero( columns, rows, frames );

    SelectionObjList l_selectionObjects = SceneManager::getInstance()->getSelectionObjects();
    int x1, x2, y1, y2, z1, z2;

    for( unsigned int i = 0; i < l_selectionObjects.size(); ++i )
    {
        for( unsigned int j = 0; j < l_selectionObjects[i].size(); ++j )
        {
            if( l_selectionObjects[i][j]->getIsVisible() )
            {
                x1 = (int)( l_selectionObjects[i][j]->getCenter().x / voxelX - l_selectionObjects[i][j]->getSize().x / 2 );
                x2 = (int)( l_selectionObjects[i][j]->getCenter().x / voxelX + l_selectionObjects[i][j]->getSize().x / 2 );
                y1 = (int)( l_selectionObjects[i][j]->getCenter().y / voxelY - l_selectionObjects[i][j]->getSize().y / 2 );
                y2 = (int)( l_selectionObjects[i][j]->getCenter().y / voxelY + l_selectionObjects[i][j]->getSize().y / 2 );
                z1 = (int)( l_selectionObjects[i][j]->getCenter().z / voxelZ - l_selectionObjects[i][j]->getSize().z / 2 );
                z2 = (int)( l_selectionObjects[i][j]->getCenter().z / voxelZ + l_selectionObjects[i][j]->getSize().z / 2 );

                x1 = std::max( 0, std::min( x1, columns ) );
                x2 = std::max( 0, std::min( x2, columns ) );
                y1 = std::max( 0, std::min( y1, rows ) );
                y2 = std::max( 0, std::min( y2, rows ) );
                z1 = std::max( 0, std::min( z1, frames ) );
                z2 = std::max( 0, std::min( z2, frames ) );

                std::vector< float >* l_src = l_anatomy->getFloatDataset();
                std::vector< float >* l_dst = pNewAnatomy->getFloatDataset();

                for( int b = z1; b < z2; ++b )
                {
                    for( int r = y1; r < y2; ++r )
                    {
                        for( int c = x1; c < x2; ++c )
                        {
                            l_dst->at( b * rows * columns + r * columns + c ) = l_src->at( b * rows * columns + r * columns + c );
                        }
                    }
                }
            }
        }
    }

    pNewAnatomy->setName( l_anatomy->getName().BeforeFirst( '.' ) + wxT( " (cut)" ) );
    pNewAnatomy->setType( l_anatomy->getType() );
    pNewAnatomy->setDataType(l_anatomy->getDataType());
    pNewAnatomy->setNewMax(l_anatomy->getNewMax());

    m_pListCtrl2->InsertItem( index );

    refreshAllGLWidgets();
}

//////////////////////////////////////////////////////////////////////////

void MainFrame::createDistanceMap()
{
    if( !DatasetManager::getInstance()->isAnatomyLoaded() )
        return;

    long l_item = getCurrentListItem();
    if( l_item == -1 )
        return;

    DatasetInfo* l_info = DatasetManager::getInstance()->getDataset( m_pListCtrl2->GetItem( l_item ) );
    if( l_info->getType() > OVERLAY )
        return;

    Anatomy* l_anatomy = (Anatomy*)l_info;

    Logger::getInstance()->print( wxT( "Generating distance map..." ), LOGLEVEL_MESSAGE );

    int index = DatasetManager::getInstance()->createAnatomy( l_anatomy );
    Anatomy* pNewAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( index );

    Logger::getInstance()->print( wxT( "Distance map done" ), LOGLEVEL_MESSAGE );

    pNewAnatomy->setName( l_anatomy->getName().BeforeFirst('.') + wxT(" (Distance Map)"));

    m_pListCtrl2->InsertItem( index );
    refreshAllGLWidgets();
}

//////////////////////////////////////////////////////////////////////////

void MainFrame::createDistanceMapAndIso()
{
    // check l_anatomy - quit if not present
    if( !DatasetManager::getInstance()->isAnatomyLoaded() )
        return;

    long l_item = getCurrentListItem();
    if( l_item == -1 )
        return;

    DatasetInfo* l_info = DatasetManager::getInstance()->getDataset( m_pListCtrl2->GetItem( l_item ) );
    if( l_info->getType() > OVERLAY )
        return;

    Anatomy* l_anatomy = (Anatomy*)l_info;

    Logger::getInstance()->print( wxT( "Generating distance map..." ), LOGLEVEL_MESSAGE );

    Anatomy* l_tmpAnatomy = new Anatomy( m_pDatasetHelper, l_anatomy );

    Logger::getInstance()->print( wxT( "Distance map done" ), LOGLEVEL_MESSAGE );
    Logger::getInstance()->print( wxT( "Generating iso surface..." ), LOGLEVEL_MESSAGE );

    int index = DatasetManager::getInstance()->createCIsoSurface( l_tmpAnatomy );
    CIsoSurface *pIsoSurf = (CIsoSurface *)DatasetManager::getInstance()->getDataset( index );
    delete l_tmpAnatomy;

    pIsoSurf->GenerateSurface( 0.2f );

    Logger::getInstance()->print( wxT( "Iso surface done" ), LOGLEVEL_MESSAGE );

    if( pIsoSurf->IsSurfaceValid() )
    {
        wxString anatomyName = l_anatomy->getName().BeforeFirst( '.' );
        pIsoSurf->setName( anatomyName + wxT( " (Offset)" ) );

        m_pListCtrl2->InsertItem( index );
    }
    else
    {
        Logger::getInstance()->print( wxT( "Surface is not valid" ), LOGLEVEL_ERROR );
    }

    refreshAllGLWidgets();
}

//////////////////////////////////////////////////////////////////////////

void MainFrame::createIsoSurface()
{
    // check l_anatomy - quit if not present
    if( !DatasetManager::getInstance()->isAnatomyLoaded() )
        return;

    long l_item = getCurrentListItem();
    if( l_item == -1 )
        return;

    DatasetInfo* l_info = DatasetManager::getInstance()->getDataset( m_pListCtrl2->GetItem( l_item ) );
    if( l_info->getType() > OVERLAY )
        return;

    Anatomy *l_anatomy = (Anatomy *)l_info;

    Logger::getInstance()->print( wxT( "Generating iso surface..." ), LOGLEVEL_MESSAGE );

    int index = DatasetManager::getInstance()->createCIsoSurface( l_anatomy );
    CIsoSurface *pIsoSurf = (CIsoSurface *)DatasetManager::getInstance()->getDataset( index );
    pIsoSurf->GenerateSurface( 0.4f );

    Logger::getInstance()->print( wxT( "Iso surface done" ), LOGLEVEL_MESSAGE );

    if( pIsoSurf->IsSurfaceValid() )
    {
        wxString l_anatomyName = l_anatomy->getName().BeforeFirst( '.' );
        pIsoSurf->setName( l_anatomyName + wxT( " (Iso Surface)" ) );

        m_pListCtrl2->InsertItem( index );
    }
    else
    {
        Logger::getInstance()->print( wxT( "Surface is not valid" ), LOGLEVEL_ERROR );
    }

    refreshAllGLWidgets();
}

//////////////////////////////////////////////////////////////////////////

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
            if (!m_pLastSelectedSceneObject->getPropertiesSizer())
            {
                m_pLastSelectedSceneObject->createPropertiesSizer(m_pPropertiesWindow);
            }   
            m_pCurrentSizer = m_pLastSelectedSceneObject->getPropertiesSizer();
            
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
//     m_pDatasetHelper->m_isBoxCreated = true;
}

///////////////////////////////////////////////////////////////////////////
// This function will create a new selection object of type box, it will be 
// triggered when the user click on Box sub menu under the New Selection-Object
// sub menu under the VOI menu.
///////////////////////////////////////////////////////////////////////////
void MainFrame::onNewSelectionBox( wxCommandEvent& WXUNUSED(event) )
{
    createNewSelectionObject( BOX_TYPE );    
//     m_pDatasetHelper->m_isBoxCreated = true;
}

///////////////////////////////////////////////////////////////////////////
// This function will create a new selection object of type depending of the argument.
//
// i_newSelectionObjectType         : The type of the new selection object we wat to create.
///////////////////////////////////////////////////////////////////////////
void MainFrame::createNewSelectionObject( ObjectType i_newSelectionObjectType )
{
    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    Vector l_center( m_pXSlider->GetValue() * voxelX, 
                     m_pYSlider->GetValue() * voxelY, 
                     m_pZSlider->GetValue() * voxelZ );
    float l_sizeV = 10;

    Vector l_size( l_sizeV / voxelX, 
                   l_sizeV / voxelY,
                   l_sizeV / voxelZ );
    

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
        // Our new selection object is under another master selection object.
        l_newSelectionObjectId = m_pTreeWidget->AppendItem( l_treeSelectionId, l_newSelectionObject->getName(), 0, -1, l_newSelectionObject );
        m_pTreeWidget->SetItemBackgroundColour( l_newSelectionObjectId, *wxGREEN );
    }
    else if( treeSelected( l_treeSelectionId ) == CHILD_OBJECT )
    {
        wxTreeItemId l_parentId = m_pTreeWidget->GetItemParent( l_treeSelectionId );

        // Our new selection object is under another child selection object.
        l_newSelectionObjectId = m_pTreeWidget->AppendItem( l_parentId, l_newSelectionObject->getName(), 0, -1, l_newSelectionObject );
        m_pTreeWidget->SetItemBackgroundColour( l_newSelectionObjectId, *wxGREEN );
    }
    else
    {
        // Our new selection object is on top.
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
    SceneManager::getInstance()->toggleShowAllSelObj();
    refreshAllGLWidgets();
}

void MainFrame::onActivateSelectionObjects( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->toggleActivateAllSelObj();
    refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 * Menu Spline Surface
 *
 ****************************************************************************************************/

void MainFrame::onNewSplineSurface( wxCommandEvent& WXUNUSED(event) )
{
    if( DatasetManager::getInstance()->isSurfaceLoaded() )
    {
        return;
    }

    //Generate KdTree for Spline Surface
    Fibers* pTmpFib = DatasetManager::getInstance()->getSelectedFibers( getCurrentListItem() );
    if(pTmpFib != NULL)
    {
        pTmpFib->generateKdTree();
    }

    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    int l_xs = (int)( m_pXSlider->GetValue() * voxelX );
    int l_ys = (int)( m_pYSlider->GetValue() * voxelY );
    int l_zs = (int)( m_pZSlider->GetValue() * voxelZ );

    // Delete all existing points.
    m_pTreeWidget->DeleteChildren( m_tPointId );
    Fibers* l_fibers = NULL;

    if( DatasetManager::getInstance()->isFibersLoaded() )
    {
        l_fibers = DatasetManager::getInstance()->getSelectedFibers( getCurrentListItem() );
    }

    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();

    if( SceneManager::getInstance()->isSagittalDisplayed() )
    {
        for( int i = 0; i < 11; ++i )
        {
            for( int j = 0; j < 11; ++j )
            {
                int yy = (int)( rows   * 0.1f * voxelY * i );
                int zz = (int)( frames * 0.1f * voxelZ * j );

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
                    if( DatasetManager::getInstance()->isFibersLoaded() && l_fibers->getBarycenter( l_point ) )
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
    else if( SceneManager::getInstance()->isCoronalDisplayed() )
    {
        for( int i = 0; i < 11; ++i )
        {
            for( int j = 0; j < 11; ++j )
            {
                int l_xx = (int)( columns * 0.1f * voxelX * i );
                int l_zz = (int)( frames  * 0.1f * voxelZ * j );

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
                int l_xx = (int)( columns * 0.1f * voxelX * i );
                int l_yy = (int)( rows    * 0.1f * voxelY * j );

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

    int index = DatasetManager::getInstance()->createSurface();
    Surface *pSurface = (Surface *)DatasetManager::getInstance()->getDataset( index );
    pSurface->execute();

    m_pListCtrl2->InsertItem( index );

    refreshAllGLWidgets();
}


void MainFrame::onToggleDrawVectors( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->toggleVectorsDisplay();
    //m_pDatasetHelper->m_drawVectors = !m_pDatasetHelper->m_drawVectors;
    refreshAllGLWidgets();
}

void MainFrame::onToggleNormal( wxCommandEvent& WXUNUSED(event ))
{
    float normal = SceneManager::getInstance()->getNormalDirection();
    SceneManager::getInstance()->setNormalDirection( -normal );
    //m_pDatasetHelper->m_normalDirection *= -1.0;

    for( unsigned int i( 0 ); i < static_cast<unsigned int>( m_pListCtrl2->GetItemCount() ); ++i )
    {
        DatasetInfo* l_info = DatasetManager::getInstance()->getDataset( m_pListCtrl2->GetItem( i ) );
        if( l_info->getType() == SURFACE )
        {
            ((Surface*)l_info)->flipNormals();
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
            l_info->toggleShowFS();
            m_pListCtrl2->UpdateSelected();
        }
    }
    refreshAllGLWidgets();
}

void MainFrame::onToggleLighting( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->toggleLightingDisplay();
    refreshAllGLWidgets();
}

void MainFrame::onToggleBlendTexOnMesh( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->toggleTexBlendOnMesh();
    refreshAllGLWidgets();
}

void MainFrame::onToggleFilterIso( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->toggleIsoSurfaceFiltered();
    refreshAllGLWidgets();
}

void MainFrame::onInvertFibers( wxCommandEvent& WXUNUSED(event) )
{
	if (m_pCurrentSceneObject != NULL && m_currentListItem != -1)
    {
		DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pCurrentSceneObject);
		if( pDatasetInfo->getType() == FIBERS )
		{
			Fibers* l_fibers = DatasetManager::getInstance()->getSelectedFibers( getCurrentListItem() );
			if( l_fibers != NULL )
			{
				l_fibers->invertFibers();
			}
		}
		else if ( pDatasetInfo->getType() == FIBERSGROUP )
		{
			FibersGroup* l_fibersGroup = DatasetManager::getInstance()->getFibersGroup();
			if( l_fibersGroup != NULL )
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
			Fibers* l_fibers = DatasetManager::getInstance()->getSelectedFibers( getCurrentListItem() );
			if(l_fibers != NULL)
			{
				l_fibers->useFakeTubes();
			}
		}
		else if ( pDatasetInfo->getType() == FIBERSGROUP )
		{
			FibersGroup* l_fibersGroup = DatasetManager::getInstance()->getFibersGroup();
			if( l_fibersGroup != NULL )
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
    if( SceneManager::getInstance()->toggleClearToBlack() )
    {
        glClearColor( 0.0, 0.0, 0.0, 0.0 );
    }
    else
    {
        glClearColor( 1.0, 1.0, 1.0, 0.0 );
    }
    refreshAllGLWidgets();
}

void MainFrame::onSelectNormalPointer( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->setRulerActive( false );
    m_isDrawerToolActive = false;

    m_pToolBar->m_txtRuler->Disable();
    m_pToolBar->EnableTool(m_pToolBar->m_selectColorPicker->GetId(), false);
    m_pToolBar->EnableTool(m_pToolBar->m_toggleDrawRound->GetId(), false);
    m_pToolBar->EnableTool(m_pToolBar->m_toggleDraw3d->GetId(), false);
    m_pToolBar->EnableTool(m_pToolBar->m_selectPen->GetId(), false);
    m_pToolBar->EnableTool(m_pToolBar->m_selectEraser->GetId(), false);
    refreshAllGLWidgets();
}

void MainFrame::onSelectRuler( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->setRulerActive( true );
    m_isDrawerToolActive = false;

    m_pToolBar->m_txtRuler->Enable();
    m_pToolBar->EnableTool(m_pToolBar->m_selectColorPicker->GetId(), false);
    m_pToolBar->EnableTool(m_pToolBar->m_toggleDrawRound->GetId(), false);
    m_pToolBar->EnableTool(m_pToolBar->m_toggleDraw3d->GetId(), false);
    m_pToolBar->EnableTool(m_pToolBar->m_selectPen->GetId(), false);
    m_pToolBar->EnableTool(m_pToolBar->m_selectEraser->GetId(), false);
    refreshAllGLWidgets();
}

void MainFrame::onRulerToolClear( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->getRulerPts().clear();
    SceneManager::getInstance()->setRulerFullLength( 0.0 );
    SceneManager::getInstance()->setRulerPartialLength( 0.0 );
    refreshAllGLWidgets();
}

void MainFrame::onRulerToolAdd( wxCommandEvent& WXUNUSED(event) )
{
    vector< Vector > v = SceneManager::getInstance()->getRulerPts();
    
    if( SceneManager::getInstance()->isRulerActive() && !v.empty() )
    {
        v.push_back( v.back() );
    }
    refreshAllGLWidgets();
}

void MainFrame::onRulerToolDel( wxCommandEvent& WXUNUSED(event) )
{
    if( SceneManager::getInstance()->isRulerActive() && !SceneManager::getInstance()->getRulerPts().empty() )
    {
        SceneManager::getInstance()->getRulerPts().pop_back();
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
			Fibers* l_fibers = DatasetManager::getInstance()->getSelectedFibers( getCurrentListItem() );
			if( l_fibers != NULL)
			{
				l_fibers->useTransparency();

			}
		}
		else if ( pDatasetInfo->getType() == FIBERSGROUP )
		{
			FibersGroup* l_fibersGroup = DatasetManager::getInstance()->getFibersGroup();
			if( l_fibersGroup != NULL )
			{
				l_fibersGroup->useTransparency();
			}			
		}
	}
    refreshAllGLWidgets();
}

//////////////////////////////////////////////////////////////////////////

void MainFrame::onUseGeometryShader( wxCommandEvent& event )
{
    SceneManager::getInstance()->toggleFibersGeomShaderActive();
    refreshAllGLWidgets();
}

//////////////////////////////////////////////////////////////////////////

void MainFrame::onResetColor(wxCommandEvent& WXUNUSED(event))
{
	if (m_pCurrentSceneObject != NULL && m_currentListItem != -1)
    {
		DatasetInfo* pDatasetInfo = ((DatasetInfo*)m_pCurrentSceneObject);

        if( pDatasetInfo->getType() == FIBERS )
        {
            Fibers* l_fibers = DatasetManager::getInstance()->getSelectedFibers( getCurrentListItem() );
			if( l_fibers  != NULL)
			{
				l_fibers->resetColorArray();
			}
		}
		else if ( pDatasetInfo->getType() == FIBERSGROUP )
		{
			FibersGroup* l_fibersGroup = DatasetManager::getInstance()->getFibersGroup();
			if( l_fibersGroup != NULL )
			{
				l_fibersGroup->resetFibersColor();
			}

		}
	}
    
    m_pDatasetHelper->m_selBoxChanged = true;
    refreshAllGLWidgets();
}

void MainFrame::onToggleColorMapLegend( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->toggleColorMapLegendDisplay();
    refreshAllGLWidgets();
}

void MainFrame::onSetCMap0( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->setColorMap( 0 );
    refreshAllGLWidgets();
}

void MainFrame::onSetCMap1( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->setColorMap( 1 );
    refreshAllGLWidgets();
}

void MainFrame::onSetCMap2( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->setColorMap( 2 );
    refreshAllGLWidgets();
}

void MainFrame::onSetCMap3( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->setColorMap( 3 );
    refreshAllGLWidgets();
}

void MainFrame::onSetCMap4( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->setColorMap( 4 );
    refreshAllGLWidgets();
}

void MainFrame::onSetCMap5( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->setColorMap( 5 );
    refreshAllGLWidgets();
}

void MainFrame::onSetCMapNo( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->setColorMap( -1 );
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

void MainFrame::onWarningsInformations( wxCommandEvent& WXUNUSED(event) )
{
	wxString nl = _T( "\n" );
    (void)wxMessageBox(
		_T("Please take note that the values of the settings used when using the fibers group may not reflect the current value of the settings of all fibers.") 
		+ nl + nl
		+ _T("Since it is possible to modify a setting globally, then to modify it locally to a fiber bundle, it is impossible to have only one value reflecting the different values of each bundle.") 
		+ nl + nl
		+ _T("Therefore, when using the fibers group, all settings are set to their default values. For example, if you want to set the minimal length of the displayed fibers to the lowest possible value, even if the slider is displayed as being to the lowest value, you have to click the Apply button to make sure that it is applied."),
		wxT("Warnings Informations about Fibers Group functionalities"));
}

void MainFrame::onScreenshot( wxCommandEvent& WXUNUSED(event) )
{
    wxString l_caption         = wxT( "Choose a file" );
    wxString l_wildcard        = wxT( "PPM files (*.ppm)|*.ppm|*.*|*.*" );
    wxString l_defaultDir      = wxEmptyString;
    wxString l_defaultFilename = wxEmptyString;
    wxFileDialog dialog( this, l_caption, l_defaultDir, l_defaultFilename, l_wildcard, wxSAVE );
    dialog.SetFilterIndex( 0 );
    dialog.SetDirectory( SceneManager::getInstance()->getScreenshotPath() );
    if( dialog.ShowModal() == wxID_OK )
    {
        screenshot( dialog.GetDirectory(), dialog.GetPath() );
    }
}

void MainFrame::screenshot( const wxString &path, const wxString &filename )
{
    SceneManager::getInstance()->setScreenshotName( filename );
    SceneManager::getInstance()->setScreenshotPath( path );

    if ( filename.AfterLast( '.' ) != _T( "ppm" ) )
    {
        SceneManager::getInstance()->setScreenshotName( filename + wxT( ".ppm" ) );
    }

    SceneManager::getInstance()->setScreenshotScheduled( true );
    m_pMainGL->render();
    m_pMainGL->render();
}

void MainFrame::onSliderMoved( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->updateView( m_pXSlider->GetValue(), m_pYSlider->GetValue(), m_pZSlider->GetValue() );
    refreshAllGLWidgets();
}

void MainFrame::onToggleShowAxial( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->toggleAxialDisplay();
}

void MainFrame::onToggleShowCoronal( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->toggleCoronalDisplay();
}

void MainFrame::onToggleShowSagittal( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->toggleSagittalDisplay();
}

void MainFrame::onToggleAlpha( wxCommandEvent& WXUNUSED(event) )
{
    SceneManager::getInstance()->toggleAlphaBlend();
//    refreshAllGLWidgets();
//    this->Update(); 
}

void MainFrame::refreshAllGLWidgets()
{
    updateMenus();
    refreshViews();   
    if( SceneManager::getInstance()->isRulerActive() )
    {
        wxString sbString1 = wxString::Format( wxT( "%4.1fmm (%2.1fmm) " ), SceneManager::getInstance()->getRulerFullLength(), SceneManager::getInstance()->getRulerPartialLenth() );
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

//////////////////////////////////////////////////////////////////////////

// TODO: Call this method when done loading
void MainFrame::updateStatusBar()
{
    GetStatusBar()->SetStatusText( wxString::Format( 
        wxT("Position: %d  %d  %d" ), m_pXSlider->GetValue(), m_pYSlider->GetValue(),m_pZSlider->GetValue() ) );
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

void MainFrame::onActivateListItem2( wxListEvent& evt )
{
    Logger::getInstance()->print( _T( "Event triggered - MainFrame::onActivateListItem2" ), LOGLEVEL_DEBUG );

    if( 3 == m_pListCtrl2->GetColumnClicked() )
    {
        deleteListItem();
    }

    refreshAllGLWidgets();
}

void MainFrame::deleteListItem()
{
    if (m_pCurrentSceneObject != NULL && m_currentListItem != -1)
    {       
        long tmp = m_currentListItem;
        DatasetInfo * pInfo = DatasetManager::getInstance()->getDataset( m_pListCtrl2->GetItem( m_currentListItem ) );
		
        if( SURFACE == pInfo->getType() )
        {
            m_pDatasetHelper->deleteAllPoints();
        }

//         if ( wxT( "(Object)" ) == pInfo->getName() )
//         {            
//             m_pDatasetHelper->m_isObjCreated = false;
//             m_pDatasetHelper->m_isObjfilled = false;
//     
//         }
//         else if( wxT( "(Background)" ) == pInfo->getName() )
//         {            
//             m_pDatasetHelper->m_isBckCreated = false;
//             m_pDatasetHelper->m_isBckfilled = false;
//         }

        deleteSceneObject();
        m_pListCtrl2->DeleteItem( tmp );
        refreshAllGLWidgets();
    }
}

// void MainFrame::onSelectListItem( wxListEvent& evt )
// {
//     Logger::getInstance()->print( _T( "Event triggered - MainFrame::onSelectListItem" ), LOGLEVEL_DEBUG );
// 
//     int l_item = evt.GetIndex();
//     m_pTreeWidget->UnselectAll();
//     DatasetInfo *l_info = (DatasetInfo*)m_pListCtrl->GetItemData( l_item) ;
//     int l_col = m_pListCtrl->getColClicked();
//     if (l_col == 12 && l_info->getType() >= MESH)
//     {
//         m_pListCtrl->SetItem( l_item, 2, wxString::Format( l_info->toggleUseTex() ? wxT( "%.2f" ) : wxT( "(%.2f)" ), l_info->getThreshold() * l_info->getOldMax() ) );
//     }
// 	if( l_info->getType() == FIBERS )
// 	{
// 		Fibers* pFibers = (Fibers*)l_info;
// 		if( pFibers )
// 		{
// 			pFibers->updateColorationMode();
// 		}
// 	}
//     m_pLastSelectedSceneObject = l_info;
//     m_lastSelectedListItem = l_item;
//     
//     // Check if it is RGB
//     if( l_info->getType() == RGB )
//     {
//         m_pDatasetHelper->m_canUseColorPicker = true;
//     }
//     else
//     {
//         m_pDatasetHelper->m_canUseColorPicker = false;
//     }
//     
//     refreshAllGLWidgets();
// }

//////////////////////////////////////////////////////////////////////////

void MainFrame::onDeleteListItem2( wxListEvent& evt )
{
    Logger::getInstance()->print( _T( "Event triggered - MainFrame::onDeleteListItem2" ), LOGLEVEL_DEBUG );

    DatasetIndex index = (DatasetIndex)evt.GetData();
    DatasetManager::getInstance()->remove( index );
}

//////////////////////////////////////////////////////////////////////////

void MainFrame::onDeselectListItem2( wxListEvent& evt )
{
    Logger::getInstance()->print( _T( "Event triggered - MainFrame::onDeselectListItem2" ), LOGLEVEL_DEBUG );

    m_pLastSelectedSceneObject = NULL;
    m_lastSelectedListItem = -1;
}

//////////////////////////////////////////////////////////////////////////

void MainFrame::onSelectListItem2( wxListEvent& evt )
{
    Logger::getInstance()->print( _T( "Event triggered - MainFrame::onSelectListItem2" ), LOGLEVEL_DEBUG );

    int index = evt.GetIndex();
    m_pTreeWidget->UnselectAll();
    DatasetInfo * pInfo = DatasetManager::getInstance()->getDataset( m_pListCtrl2->GetItem( index ) );

    if( NULL == pInfo )
    {
        Logger::getInstance()->print( wxT( "Null pointer" ), LOGLEVEL_DEBUG );
        return;
    }

    m_pLastSelectedSceneObject = pInfo;
    m_lastSelectedListItem = index;

    // Check if it is RGB
    if( RGB == pInfo->getType() )
    {
        m_canUseColorPicker = true;
    }
    else
    {
        m_canUseColorPicker = false;
    }

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
void MainFrame::onDeleteTreeItem( wxTreeEvent& WXUNUSED(event) )
{    
    deleteTreeItem();
}

void MainFrame::deleteTreeItem()
{
    if( m_pCurrentSceneObject != NULL )
    {   
        wxTreeItemId l_treeId = m_pTreeWidget->GetSelection();
        if( !l_treeId.IsOk() )
        {
            return;
        }
        int l_selected = treeSelected( l_treeId );  
        if( l_selected == CHILD_OBJECT )
        {
            ((SelectionObject*) ((m_pTreeWidget->GetItemData(m_pTreeWidget->GetItemParent(l_treeId)))))->setIsDirty(true);
        }
        if( l_selected == CHILD_OBJECT || l_selected == MASTER_OBJECT || l_selected == POINT_DATASET )
        {  
            deleteSceneObject();
            m_pTreeWidget->Delete( l_treeId );
            m_pLastSelectedObj = NULL;
            m_pLastSelectedPoint = NULL;
        }   
        m_pDatasetHelper->m_selBoxChanged = true;
    }
    //refreshAllGLWidgets();
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
            if ( m_pLastSelectedObj != NULL )
            {
                m_pLastSelectedObj->unselect();
            }
            if ( m_pLastSelectedPoint != NULL )
            {
                m_pLastSelectedPoint->unselect();
                m_pLastSelectedPoint = NULL;
            }

            l_selectionObject = (SelectionObject*)( m_pTreeWidget->GetItemData( l_treeId ) );
            m_pLastSelectedObj = l_selectionObject;
            m_pLastSelectedObj->select( false );
            m_pLastSelectedSceneObject = l_selectionObject;
            m_lastSelectedListItem = -1;
            break;

        case POINT_DATASET:
            if( m_pLastSelectedPoint != NULL )
            {
                m_pLastSelectedPoint->unselect();
            }
            if( m_pLastSelectedObj != NULL )
            {
                m_pLastSelectedObj->unselect();
                m_pLastSelectedObj = NULL;
            }

            l_selectedPoint = (SplinePoint*)( m_pTreeWidget->GetItemData( l_treeId ) );
            m_pLastSelectedPoint = l_selectedPoint;
            m_pLastSelectedPoint->select( false );
            m_pLastSelectedSceneObject = l_selectedPoint;
            m_lastSelectedListItem = -1;
            break;

        default:
            if( m_pLastSelectedPoint != NULL )
            {
                m_pLastSelectedPoint->unselect();
                m_pLastSelectedPoint = NULL;
            }
            if( m_pLastSelectedObj != NULL )
            {
                m_pLastSelectedObj->unselect();
                m_pLastSelectedObj = NULL;
            }
            break;
    }    
#ifdef __WXMSW__
    if( m_currentListItem != -1 )
    {
        m_pListCtrl2->UnselectAll();
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
    SceneManager::getInstance()->getScene()->toggleIsRotateZ();
    setTimerSpeed();
}

void MainFrame::onRotateY( wxCommandEvent& event )
{
    SceneManager::getInstance()->getScene()->toggleIsRotateY();
    setTimerSpeed();
}

void MainFrame::onRotateX( wxCommandEvent& event )
{
    SceneManager::getInstance()->getScene()->toggleIsRotateX();
    setTimerSpeed();
}

void MainFrame::onNavigateAxial( wxCommandEvent& event )
{
    SceneManager::getInstance()->getScene()->toggleIsNavAxial();
    setTimerSpeed();
}

void MainFrame::onNavigateSagital( wxCommandEvent& event )
{
    SceneManager::getInstance()->getScene()->toggleIsNavSagittal();
    setTimerSpeed();  
}

void MainFrame::onNavigateCoronal( wxCommandEvent& event )
{
    SceneManager::getInstance()->getScene()->toggleIsNavCoronal();
    setTimerSpeed();
}

void MainFrame::setTimerSpeed()
{
    m_pTimer->Stop();
    if(    SceneManager::getInstance()->getScene()->m_isNavAxial 
        || SceneManager::getInstance()->getScene()->m_isNavCoronal
        || SceneManager::getInstance()->getScene()->m_isNavSagital 
        || SceneManager::getInstance()->getScene()->m_isRotateX
        || SceneManager::getInstance()->getScene()->m_isRotateY 
        || SceneManager::getInstance()->getScene()->m_isRotateZ )
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
 * System event functions
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
    m_threadsActive--;

    if ( m_threadsActive > 0 )
        return;

    Logger::getInstance()->print( wxT( "Tree finished" ), LOGLEVEL_MESSAGE );
    SceneManager::getInstance()->updateAllSelectionObjects();
    m_pDatasetHelper->m_selBoxChanged = true;
    
    refreshAllGLWidgets();
}

///////////////////////////////////////////////////////////////////////////
// OnGLEvent handles mouse events in the GL Refreshing widgets.
///////////////////////////////////////////////////////////////////////////
void MainFrame::onGLEvent( wxCommandEvent &event )
{
    wxPoint l_pos, l_newPos;
    int NAV_GL_SIZE = m_pGL0->GetSize().x;

    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();

    switch( event.GetInt() )
    {
        case AXIAL:
        {
            l_pos = m_pGL0->getMousePos();
            float x = ( (float)l_pos.x / NAV_GL_SIZE * columns );
            float y = ( (float)( NAV_GL_SIZE - l_pos.y ) / NAV_GL_SIZE * rows );

            m_pXSlider->SetValue( (int)( x ) );
            m_pYSlider->SetValue( (int)( y ) );
            break;
        }
        case CORONAL:
        {
            l_pos = m_pGL1->getMousePos();
            float x = ( (float)l_pos.x / NAV_GL_SIZE ) * columns;
            float y = ( (float)( NAV_GL_SIZE - l_pos.y ) / NAV_GL_SIZE * frames );

            m_pXSlider->SetValue( (int)( x ) );
            m_pZSlider->SetValue( (int)( y ) );
            break;
        }
        case SAGITTAL:
        {
            l_pos = m_pGL2->getMousePos();
            float x = ( (float)( NAV_GL_SIZE - l_pos.x ) / NAV_GL_SIZE ) * rows;
            float y = ( (float)( NAV_GL_SIZE - l_pos.y ) / NAV_GL_SIZE * frames );

            m_pYSlider->SetValue( (int)( x ) );
            m_pZSlider->SetValue( (int)( y ) );
            break;
        }
        case MAIN_VIEW:
        {
            int delta = (int)m_pMainGL->getDelta();
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
            break;
        }
    }
    SceneManager::getInstance()->updateView( m_pXSlider->GetValue(), m_pYSlider->GetValue(), m_pZSlider->GetValue() );
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
    if( SceneManager::getInstance()->getScene()->m_isRotateZ )
    {
        SceneManager::getInstance()->getScene()->m_rotAngleZ++;
    }
    if( SceneManager::getInstance()->getScene()->m_isRotateY )
    {
        SceneManager::getInstance()->getScene()->m_rotAngleY++;
    }
    if( SceneManager::getInstance()->getScene()->m_isRotateX )
    {
        SceneManager::getInstance()->getScene()->m_rotAngleX++;
    } 

    //Navigate through slices sagittal
    if( SceneManager::getInstance()->getScene()->m_isNavSagital )
    {
        SceneManager::getInstance()->getScene()->m_posSagital++;
    }
    else
    {
        SceneManager::getInstance()->getScene()->m_posSagital = SceneManager::getInstance()->getSliceX();
    }

    //Navigate through slices axial
    if( SceneManager::getInstance()->getScene()->m_isNavAxial )
    {
        SceneManager::getInstance()->getScene()->m_posAxial++;
    }
    else
    {
        SceneManager::getInstance()->getScene()->m_posAxial = SceneManager::getInstance()->getSliceZ();
    }

    //Navigate through slices coronal
    if( SceneManager::getInstance()->getScene()->m_isNavCoronal )
    {
        SceneManager::getInstance()->getScene()->m_posCoronal++;
    }
    else
    {
        SceneManager::getInstance()->getScene()->m_posCoronal = SceneManager::getInstance()->getSliceY();
    }

    refreshAllGLWidgets();
    refreshViews();
    SceneManager::getInstance()->increaseAnimStep();
}

//////////////////////////////////////////////////////////////////////////

MainFrame::~MainFrame()
{
    Logger::getInstance()->print( wxT( "MainFrame destructor" ), LOGLEVEL_DEBUG );
    m_pTimer->Stop();
    Logger::getInstance()->print( wxT( "Timer stopped" ), LOGLEVEL_DEBUG );

    delete m_pTimer;
    m_pTimer = NULL;
    
    delete m_pDatasetHelper;
    m_pDatasetHelper = NULL;
}
