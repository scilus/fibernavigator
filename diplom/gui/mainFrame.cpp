#include "mainFrame.h"

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/filedlg.h"
#include "wx/statbmp.h"
#include "wx/colordlg.h"

#include "../gfx/theScene.h"

#include "../dataset/splinePoint.h"
#include "../dataset/fibers.h"
#include "../dataset/surface.h"
#include "../misc/IsoSurface/CIsoSurface.h"
#include "../dataset/Anatomy.h"

#include "../icons/eyes.xpm"
#include "../icons/delete.xpm"

#include "../main.h"

DECLARE_EVENT_TYPE(wxEVT_TREE_EVENT, -1)
DECLARE_EVENT_TYPE(wxEVT_NAVGL_EVENT, -1)

/****************************************************************************************************
 *
 * Event Table
 *
 ****************************************************************************************************/
 
BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	/*
	 * Menu events
	 */
	// Menu File
	EVT_MENU(MENU_FILE_NEW_ISOSURF, MainFrame::OnNewIsoSurface)
	EVT_MENU(MENU_FILE_LOAD, MainFrame::OnLoad)
	EVT_MENU(MENU_FILE_RELOAD_SHADER, MainFrame::OnReloadShaders)
	EVT_MENU(MENU_FILE_SAVE, MainFrame::OnSave)
    EVT_MENU(MENU_FILE_SAVE_FIBERS, MainFrame::OnSaveFibers)
    EVT_MENU(MENU_FILE_SAVE_SURFACE, MainFrame::OnSaveSurface)
    EVT_MENU(MENU_FILE_SAVE_DATASET, MainFrame::OnSaveDataset)
    EVT_MENU(MENU_FILE_MINIMIZE_DATASET, MainFrame::OnMinimizeDataset)
    EVT_MENU(MENU_FILE_DILATE_DATASET, MainFrame::OnDilateDataset)
    EVT_MENU(MENU_FILE_ERODE_DATASET, MainFrame::OnErodeDataset)
    EVT_MENU(MENU_FILE_QUIT, MainFrame::OnQuit)
    EVT_MENU(BUTTON_TOGGLE_LAYOUT, MainFrame::OnToggleLayout)
	// Menu View
    EVT_MENU(MENU_VIEW_RESET, MainFrame::OnMenuViewReset)
    EVT_MENU(MENU_VIEW_LEFT, MainFrame::OnMenuViewLeft)
	EVT_MENU(MENU_VIEW_RIGHT, MainFrame::OnMenuViewRight)
	EVT_MENU(MENU_VIEW_TOP, MainFrame::OnMenuViewTop)
	EVT_MENU(MENU_VIEW_BOTTOM, MainFrame::OnMenuViewBottom)
	EVT_MENU(MENU_VIEW_FRONT, MainFrame::OnMenuViewFront)
	EVT_MENU(MENU_VIEW_BACK, MainFrame::OnMenuViewBack)
	EVT_MENU(MENU_VIEW_SHOW_CROSSHAIR, MainFrame::OnMenuViewCrosshair)
	// Menu VOI
    EVT_MENU(MENU_VOI_NEW_SELBOX, MainFrame::OnNewSelBox)
    EVT_MENU(MENU_VOI_NEW_FROM_OVERLAY, MainFrame::OnNewFromOverlay)
	EVT_MENU(MENU_VOI_RENDER_SELBOXES, MainFrame::OnHideSelBoxes)
	EVT_MENU(MENU_VOI_TOGGLE_SELBOX, MainFrame::OnToggleSelBox)
	EVT_MENU(MENU_VOI_TOGGLE_SHOWBOX, MainFrame::OnToggleShowBox)
	EVT_MENU(MENU_VOI_RENAME_BOX, MainFrame::OnRenameBox)
	EVT_MENU(MENU_VOI_TOGGLE_ANDNOT, MainFrame::OnToggleAndNot)
	EVT_MENU(MENU_VOI_COLOR_ROI, MainFrame::OnColorRoi)
	EVT_MENU(MENU_VOI_USE_MORPH, MainFrame::OnUseMorph)
	// Menu Surfaces
	EVT_MENU(MENU_SURFACE_NEW_OFFSET, MainFrame::OnNewOffsetMap)	
	EVT_MENU(MENU_SPLINESURF_NEW, MainFrame::OnNewSurface)
	EVT_MENU(MENU_SPLINESURF_TOGGLE_LIC, MainFrame::OnToggleLIC)
	EVT_MENU(MENU_SPLINESURF_TOGGLE_NORMAL, MainFrame::OnToggleNormal)
	EVT_MENU(MENU_SPLINESURF_DRAW_POINTS, MainFrame::OnTogglePointMode)
	EVT_MENU(MENU_SPLINESURF_DRAW_VECTORS, MainFrame::OnToggleDrawVectors)
	// Menu Options
	EVT_MENU(MENU_OPTIONS_ASSIGN_COLOR, MainFrame::OnAssignColor)
	EVT_MENU(MENU_OPTIONS_RESET_COLOR, MainFrame::OnResetColor)
	EVT_MENU(MENU_OPTIONS_TOGGLE_LIGHTING, MainFrame::OnToggleLighting)
	EVT_MENU(MENU_OPTIONS_INVERT_FIBERS, MainFrame::OnInvertFibers)
	EVT_MENU(MENU_OPTIONS_USE_FAKE_TUBES, MainFrame::OnUseFakeTubes)
	EVT_MENU(MENU_OPTIONS_USE_TRANSPARENCY, MainFrame::OnUseTransparency)
	EVT_MENU(MENU_OPTIONS_TOGGLE_TEXTURE_FILTERING, MainFrame::OnToggleTextureFiltering)
	EVT_MENU(MENU_OPTIONS_BLEND_TEX_ON_MESH, MainFrame::OnToggleBlendTexOnMesh)
	EVT_MENU(MENU_OPTIONS_FILTER_ISO, MainFrame::OnToggleFilterIso)
	EVT_MENU(MENU_OPTIONS_CLEAN, MainFrame::OnClean)
	EVT_MENU(MENU_OPTIONS_LOOP, MainFrame::OnLoop)
	EVT_MENU(MENU_OPTIONS_CMAP0, MainFrame::OnSetCMap0)
	EVT_MENU(MENU_OPTIONS_CMAP1, MainFrame::OnSetCMap1)
	EVT_MENU(MENU_OPTIONS_CMAP2, MainFrame::OnSetCMap2)
	EVT_MENU(MENU_OPTIONS_CMAP3, MainFrame::OnSetCMap3)
	EVT_MENU(MENU_OPTIONS_CMAP4, MainFrame::OnSetCMap4)
	EVT_MENU(MENU_OPTIONS_CMAP5, MainFrame::OnSetCMap5)
	EVT_MENU(MENU_OPTIONS_CMAPNO, MainFrame::OnSetCMapNo)
	EVT_MENU(MENU_OPTIONS_CMAP_LEGEND, MainFrame::OnToggleColorMapLegend)
	// Menu Help
    EVT_MENU(MENU_HELP_ABOUT, MainFrame::OnAbout)
    EVT_MENU(MENU_HELP_SHORTCUTS, MainFrame::OnShortcuts)
    EVT_MENU(MENU_HELP_SCREENSHOT, MainFrame::OnScreenshot)

    /*
     * List widget events
     */
    EVT_LIST_ITEM_ACTIVATED(ID_LIST_CTRL, MainFrame::OnActivateListItem)
	EVT_LIST_ITEM_SELECTED(ID_LIST_CTRL, MainFrame::OnSelectListItem)
	EVT_BUTTON(ID_BUTTON_UP, MainFrame::OnListItemUp)
	EVT_BUTTON(ID_BUTTON_DOWN, MainFrame::OnListItemDown)
	EVT_MENU(MENU_LIST_DELETE, MainFrame::OnListMenuDelete)
	EVT_MENU(MENU_LIST_TOGGLESHOW, MainFrame::OnListMenuShow)
	EVT_MENU(MENU_LIST_TOGGLECOLOR, MainFrame::OnListMenuThreshold)
	EVT_MENU(MENU_LIST_TOGGLENAME, MainFrame::OnListMenuName)
	EVT_MENU(MENU_LIST_CUTOUT, MainFrame::OnListMenuCutOut)
	
	/*
     * Tree widget events
     */
    EVT_TREE_SEL_CHANGED(ID_TREE_CTRL, MainFrame::OnSelectTreeItem)
	EVT_TREE_ITEM_ACTIVATED(ID_TREE_CTRL, MainFrame::OnActivateTreeItem)
	EVT_COMMAND(ID_TREE_CTRL, wxEVT_TREE_EVENT, MainFrame::OnTreeEvent)
	EVT_TREE_END_LABEL_EDIT(ID_TREE_CTRL, MainFrame::OnTreeLabelEdit)
	EVT_BUTTON(ID_BUTTON_LOAD1, MainFrame::OnLoad1)
	EVT_BUTTON(ID_BUTTON_LOAD2, MainFrame::OnLoad2)
	EVT_BUTTON(ID_BUTTON_LOAD3, MainFrame::OnLoad3)
	
    /*
    * Interface events
    */
	EVT_SIZE(MainFrame::OnSize)
	EVT_MOUSE_EVENTS(MainFrame::OnMouseEvent)

	/* mouse click in one of the three navigation windows */
    EVT_COMMAND(ID_GL_NAV_X, wxEVT_NAVGL_EVENT, MainFrame::OnGLEvent)
    EVT_COMMAND(ID_GL_NAV_Y, wxEVT_NAVGL_EVENT, MainFrame::OnGLEvent)
	EVT_COMMAND(ID_GL_NAV_Z, wxEVT_NAVGL_EVENT, MainFrame::OnGLEvent)
	EVT_COMMAND(ID_GL_MAIN, wxEVT_NAVGL_EVENT, MainFrame::OnGLEvent)

	/* slider events */
	EVT_SLIDER(ID_X_SLIDER, MainFrame::OnXSliderMoved)
	EVT_SLIDER(ID_Y_SLIDER, MainFrame::OnYSliderMoved)
	EVT_SLIDER(ID_Z_SLIDER, MainFrame::OnZSliderMoved)
	EVT_SLIDER(ID_T_SLIDER, MainFrame::OnTSliderMoved)
	EVT_SLIDER(ID_T_SLIDER2, MainFrame::OnTSlider2Moved)

    /*
     * Buttons, not yet in menus
     */
    EVT_MENU(BUTTON_AXIAL, MainFrame::OnButtonAxial)
	EVT_MENU(BUTTON_CORONAL, MainFrame::OnButtonCoronal)
	EVT_MENU(BUTTON_SAGITTAL, MainFrame::OnButtonSagittal)
	EVT_MENU(BUTTON_TOGGLE_ALPHA, MainFrame::OnToggleAlpha)
	EVT_MENU(BUTTON_MOVE_POINTS1, MainFrame::OnMovePoints1)
	EVT_MENU(BUTTON_MOVE_POINTS2, MainFrame::OnMovePoints2)

	/* KDTREE thread finished */
	EVT_MENU(KDTREE_EVENT, MainFrame::OnKdTreeThreadFinished)

END_EVENT_TABLE()

/*****************************************************************************************************
 *
 * Main Constructor
 *
 ****************************************************************************************************/
MainFrame::MainFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size,
    const long style):
  wxFrame(parent, id, title, pos, size, style)
{
	enlargeNav = 0;
	
	m_xSlider = new wxSlider(this, ID_X_SLIDER, 50, 0, 100, wxDefaultPosition,
           		wxSize(150, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS);

	m_ySlider = new wxSlider(this, ID_Y_SLIDER, 50, 0, 100, wxDefaultPosition,
       			wxSize(150, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS);

	m_zSlider = new wxSlider(this, ID_Z_SLIDER, 50, 0, 100, wxDefaultPosition, 
			 	wxSize(150, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS);
	
	m_tSlider = new MySlider(this, ID_T_SLIDER, 30, 0, 100, wxDefaultPosition, 
				wxSize(110, 19), wxSL_HORIZONTAL | wxSL_AUTOTICKS);
	
	m_tSlider2 = new wxSlider(this, ID_T_SLIDER2, 30, 0, 100, wxDefaultPosition, 
				wxSize(103, 19), wxSL_HORIZONTAL | wxSL_AUTOTICKS);
	
	buttonUp = new wxButton(this, ID_BUTTON_UP, wxT("up"), wxDefaultPosition, wxSize(40,19));
	buttonUp->SetFont(wxFont(6, wxDEFAULT, wxNORMAL, wxNORMAL));
	buttonDown = new wxButton(this, ID_BUTTON_DOWN, wxT("down"), wxDefaultPosition, wxSize(40,19));
	buttonDown->SetFont(wxFont(6, wxDEFAULT, wxNORMAL, wxNORMAL));
	
	buttonLoad1 = new wxButton(this, ID_BUTTON_LOAD1, wxT("Load Datasets"), wxDefaultPosition, wxSize(148,20));
	buttonLoad1->SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxNORMAL));
	buttonLoad2 = new wxButton(this, ID_BUTTON_LOAD2, wxT("Load Meshes"), wxDefaultPosition, wxSize(148,20));
	buttonLoad2->SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxNORMAL));
	buttonLoad3 = new wxButton(this, ID_BUTTON_LOAD3, wxT("Load Fibers"), wxDefaultPosition, wxSize(148,20));
	buttonLoad3->SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxNORMAL));
	
	m_listCtrl = new MyListCtrl(this, ID_LIST_CTRL, wxDefaultPosition,
   		wxSize(293,-1), wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_NO_HEADER);
	
	
	m_treeWidget = new MyTreeCtrl(this, ID_TREE_CTRL, wxDefaultPosition,
	   		wxSize(150,-1), wxTR_HAS_BUTTONS|wxTR_SINGLE|wxTR_HIDE_ROOT|wxTR_HAS_BUTTONS);
	m_treeWidget->SetMaxSize(wxSize(150,-1));
	
	
   wxImageList* imageList = new wxImageList(16,16);
#ifndef __WXMAC__
   imageList->Add(wxIcon(eyes_xpm));
   imageList->Add(wxIcon(delete_xpm));
#else
   imageList->Add((wxImage( MyApp::respath +_T( "icons/eyes.png" ), wxBITMAP_TYPE_PNG )));
   imageList->Add((wxImage( MyApp::respath +_T( "icons/delete.png" ), wxBITMAP_TYPE_PNG )));
#endif
   m_listCtrl->AssignImageList(imageList, wxIMAGE_LIST_SMALL);

   wxListItem itemCol;
   itemCol.SetText(wxT(""));
   m_listCtrl->InsertColumn(0, itemCol);

   itemCol.SetText(wxT("Name"));
   itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
   m_listCtrl->InsertColumn(1, itemCol);

   itemCol.SetText(wxT("Threshold"));
   itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
   m_listCtrl->InsertColumn(2, itemCol);

   itemCol.SetText(wxT(""));
   m_listCtrl->InsertColumn(3, itemCol);
   
   m_listCtrl->SetColumnWidth(0, 20);
   m_listCtrl->SetColumnWidth(1, 194);
   m_listCtrl->SetColumnWidth(2, 70);
   m_listCtrl->SetColumnWidth(3, 20);
   

   wxImageList* tImageList = new wxImageList(16,16);
#ifndef __WXMAC__
   tImageList->Add(wxIcon(delete_xpm));
   tImageList->Add(wxIcon(eyes_xpm));
#else
   tImageList->Add(wxImage(MyApp::respath + _T( "icons/delete.png" ), wxBITMAP_TYPE_PNG));
   tImageList->Add(wxImage(MyApp::respath + _T( "icons/eyes.png" ), wxBITMAP_TYPE_PNG));
#endif
   m_treeWidget->AssignImageList(tImageList);

   m_tRootId = m_treeWidget->AddRoot(wxT("Scene"), -1, -1, NULL );
   m_tPointId  = m_treeWidget->AppendItem(m_tRootId, wxT("points"), -1, -1, NULL);
   m_tSelBoxId  = m_treeWidget->AppendItem(m_tRootId, wxT("selection boxes"), -1, -1, NULL);
   
   m_dh = new DatasetHelper(this);
   
   	/*
    * Set OpenGL attributes
    */
    m_dh->printDebug(_T("Initializing OpenGL"), 1);
    GLboolean doubleBuffer = GL_TRUE;
    #ifdef __WXMSW__
    	int *gl_attrib = NULL;
	#else
    int gl_attrib[20] = { WX_GL_RGBA, WX_GL_MIN_RED, 1, WX_GL_MIN_GREEN, 1,
        WX_GL_MIN_BLUE, 1, WX_GL_DEPTH_SIZE, 1,
        WX_GL_DOUBLEBUFFER,
	#if defined(__WXMAC__) || defined(__WXCOCOA__)
        GL_NONE };
	#else
        None };
	#endif
	#endif
    if(!doubleBuffer)
    {
    	m_dh->printDebug(_T("don't have double buffer, disabling"), 1);
	#ifdef __WXGTK__
        gl_attrib[9] = None;
	#endif
        doubleBuffer = GL_FALSE;
    }
    
    
    
    m_dh->scene = new TheScene(m_dh);
    
    m_mainGL = new MainCanvas(m_dh, mainView, this, ID_GL_MAIN, wxDefaultPosition,
                                wxDefaultSize, 0, _T("MainGLCanvas"), gl_attrib);
   
#ifndef CTX
       m_gl0 = new MainCanvas(m_dh, axial, this, ID_GL_NAV_X, wxDefaultPosition,
	                wxSize(150,150), 0, _T("NavGLCanvasX"), gl_attrib, m_mainGL);
	    m_gl0->SetMaxSize(wxSize(150,150));
	    m_gl1 = new MainCanvas(m_dh, coronal, this, ID_GL_NAV_Y, wxDefaultPosition,
	    			wxSize(150,150), 0, _T("NavGLCanvasY"), gl_attrib, m_mainGL);
	    m_gl1->SetMaxSize(wxSize(150,150));
	    m_gl2 = new MainCanvas(m_dh, sagittal, this, ID_GL_NAV_Z, wxDefaultPosition,
	    			wxSize(150,150), 0, _T("NavGLCanvasZ"), gl_attrib, m_mainGL);
	    m_gl2->SetMaxSize(wxSize(150,150));
    
#else
    m_gl0 = new MainCanvas(m_dh, axial, m_topNavWindow, ID_GL_NAV_X, wxDefaultPosition,
                wxDefaultSize, 0, _T("NavGLCanvasX"), gl_attrib, m_mainGL->GetContext());

    m_gl1 = new MainCanvas(m_dh, coronal, m_middleNavWindow, ID_GL_NAV_Y, wxDefaultPosition,
                        wxDefaultSize, 0, _T("NavGLCanvasY"), gl_attrib, m_mainGL->GetContext());
    m_gl2 = new MainCanvas(m_dh, sagittal, m_bottomNavWindow, ID_GL_NAV_Z, wxDefaultPosition,
                wxDefaultSize, 0, _T("NavGLCanvasZ"), gl_attrib, m_mainGL->GetContext());
#endif

#ifndef __WXMAC__
    m_dh->scene->setMainGLContext(new wxGLContext(m_mainGL)); // I don't understand this (mario)
#else
    m_dh->scene->setMainGLContext( m_mainGL->GetContext() );
#endif
    topSizer 		= new wxBoxSizer( wxHORIZONTAL );
	leftSizer 		= new wxBoxSizer( wxVERTICAL );
	leftTopSizer 	= new wxBoxSizer( wxHORIZONTAL );
	navSizer 		= new wxBoxSizer( wxVERTICAL );
	buttonSizer 	= new wxBoxSizer( wxHORIZONTAL );
	treeSizer		= new wxBoxSizer( wxVERTICAL );
	
	navSizer->Add( m_gl0, 1, wxALL | wxEXPAND | wxSHAPED, 1 );
	navSizer->Add( m_zSlider, 0, wxALL, 1 );
	navSizer->Add( m_gl1, 1, wxALL  | wxEXPAND | wxSHAPED, 1 );
	navSizer->Add( m_ySlider, 0, wxALL, 1 );
	navSizer->Add( m_gl2, 1, wxALL | wxEXPAND | wxSHAPED, 1 );
	navSizer->Add( m_xSlider, 0, wxALL, 1 );
	
	treeSizer->Add( buttonLoad1, 0, wxALL | wxALIGN_CENTER, 0 );
	treeSizer->Add( buttonLoad2, 0, wxALL | wxALIGN_CENTER, 0 );
	treeSizer->Add( buttonLoad3, 0, wxALL | wxALIGN_CENTER, 0 );
	treeSizer->Add( m_treeWidget, 1, wxALL, 1 );
	
	leftTopSizer->Add( treeSizer, 0, wxALL | wxEXPAND, 0 );
	leftTopSizer->Add( navSizer, 1, wxALL | wxEXPAND, 0 );
    
    buttonSizer->Add( buttonUp, 0, wxALL, 1 );
    buttonSizer->Add( buttonDown, 0, wxALL, 1 );
    buttonSizer->Add( m_tSlider, 0, wxALL, 1 );
    buttonSizer->Add( m_tSlider2, 0, wxALL, 1 );
    
    leftSizer->Add( leftTopSizer, 0, wxALL, 1 );
    leftSizer->Add( m_listCtrl, 1, wxALL | wxEXPAND, 1 );
    leftSizer->Add( buttonSizer, 0, wxALIGN_BOTTOM | wxALL, 1 );
    
    topSizer->Add( leftSizer, 0, wxEXPAND | wxALL, 0 );
    topSizer->Add( m_mainGL, 1, wxEXPAND | wxALL, 2 );

    SetSizer( topSizer );
    topSizer->SetSizeHints( this );

}

MainFrame::~MainFrame()
{
	m_dh->printDebug(_T("main frame destructor"), 0);

	delete m_dh;
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
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnNewIsoSurface(wxCommandEvent& WXUNUSED(event))
{
	m_dh->createIsoSurface();
}
	
void MainFrame::OnNewOffsetMap(wxCommandEvent& WXUNUSED(event))
{
	m_dh->createDistanceMap();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	wxMessageDialog dialog(NULL, wxT("Really Quit?"), wxT("Really Quit?"), wxNO_DEFAULT|wxYES_NO|wxICON_INFORMATION);
	if (dialog.ShowModal() == wxID_YES)
		Close(true);
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnLoad(wxCommandEvent& WXUNUSED(event))
{
	if ( !m_dh->load(0) )
	{
		wxMessageBox(wxT("ERROR\n") + m_dh->lastError,  wxT(""), wxOK|wxICON_INFORMATION, NULL);
		GetStatusBar()->SetStatusText(wxT("ERROR"),1);
		GetStatusBar()->SetStatusText(m_dh->lastError,2);
		return;
	}
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnReloadShaders(wxCommandEvent& WXUNUSED(event))
{
	m_dh->scheduledReloadShaders = true;
	renewAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnSave(wxCommandEvent& WXUNUSED(event))
{
	wxString caption = wxT("Choose a file");
	wxString wildcard = wxT("Scene files (*.scn)|*.scn||*.*|*.*");
	wxString defaultDir = wxEmptyString;
	wxString defaultFilename = wxEmptyString;
	wxFileDialog dialog(this, caption, defaultDir, defaultFilename, wildcard, wxSAVE);
	dialog.SetFilterIndex(0);
	dialog.SetDirectory(m_dh->lastPath);
	if (m_dh->m_scnFileLoaded)
		dialog.SetFilename(m_dh->m_scnFileName);
		dialog.SetPath(m_dh->m_scenePath);
	if (dialog.ShowModal() == wxID_OK)
	{
		m_dh->lastPath = dialog.GetDirectory();
		m_dh->save(dialog.GetPath());
	}
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnSaveFibers(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->fibers_loaded) return;

	Fibers *fibers = NULL;
	m_dh->getFiberDataset(fibers);

	wxString caption = wxT("Choose a file");
	wxString wildcard = wxT("fiber files (*.fib)|*.fib||*.*|*.*");
	wxString defaultDir = wxEmptyString;
	wxString defaultFilename = wxEmptyString;
	wxFileDialog dialog(this, caption, defaultDir, defaultFilename, wildcard, wxSAVE);
	dialog.SetFilterIndex(0);
	dialog.SetDirectory(m_dh->lastPath);
	if (dialog.ShowModal() == wxID_OK)
	{
		m_dh->lastPath = dialog.GetDirectory();
		fibers->save(dialog.GetPath());
	}
}

/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnSaveDataset(wxCommandEvent& WXUNUSED(event))
{
	Anatomy* a = NULL;
		
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		DatasetInfo* info = (DatasetInfo*)m_listCtrl->GetItemData(item);
		if (info->getType() < Mesh_ )
		{
			a = (Anatomy*) m_listCtrl->GetItemData(item);
			
			wxString caption = wxT("Choose a file");
			wxString wildcard = wxT("nifti files (*.nii)|*.nii||*.*|*.*");
			wxString defaultDir = wxEmptyString;
			wxString defaultFilename = wxEmptyString;
			wxFileDialog dialog(this, caption, defaultDir, defaultFilename, wildcard, wxSAVE);
			dialog.SetFilterIndex(0);
			dialog.SetDirectory(m_dh->lastPath);
			if (dialog.ShowModal() == wxID_OK)
			{
				m_dh->lastPath = dialog.GetDirectory();
				a->saveNifti(dialog.GetPath());
			}		
		}
	}
}

/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnMinimizeDataset(wxCommandEvent& WXUNUSED(event))
{
	Anatomy* a = NULL;
		
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		DatasetInfo* info = (DatasetInfo*)m_listCtrl->GetItemData(item);
		if (info->getType() < Mesh_ )
		{
			a = (Anatomy*) m_listCtrl->GetItemData(item);
			a->minimize();
		}
	}
}

/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnDilateDataset(wxCommandEvent& WXUNUSED(event))
{
	Anatomy* a = NULL;
		
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		DatasetInfo* info = (DatasetInfo*)m_listCtrl->GetItemData(item);
		if (info->getType() < Mesh_ )
		{
			a = (Anatomy*) m_listCtrl->GetItemData(item);
			a->dilate();
		}
	}
}

/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnErodeDataset(wxCommandEvent& WXUNUSED(event))
{
	Anatomy* a = NULL;
		
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		DatasetInfo* info = (DatasetInfo*)m_listCtrl->GetItemData(item);
		if (info->getType() < Mesh_ )
		{
			a = (Anatomy*) m_listCtrl->GetItemData(item);
			a->erode();
		}
	}
}


void MainFrame::OnSaveSurface(wxCommandEvent& WXUNUSED(event))
{
	// if ...
	
	Surface *surface = NULL;
	if(!m_dh->getSurfaceDataset(surface)) return;
	std::cout << "got surface: " << surface << std::endl;

	wxString caption = wxT("Choose a file");
	wxString wildcard = wxT("surfae files (*.vtk)|*.vtk");
	wxString defaultDir = wxEmptyString;
	wxString defaultFilename = wxEmptyString;
	wxFileDialog dialog(this, caption, defaultDir, defaultFilename, wildcard, wxSAVE);
	dialog.SetFilterIndex(0);
	dialog.SetDirectory(m_dh->lastPath);
	if (dialog.ShowModal() == wxID_OK)
	{
		std::cout << " start saving" << std::endl;
		m_dh->lastPath = dialog.GetDirectory();
		surface->save(dialog.GetPath());
		std::cout << " done saving" << std::endl;
	}
}

/****************************************************************************************************
 *
 * Menu View
 * Sets the main GL widget to some standard positions
 *
 ****************************************************************************************************/
void MainFrame::OnMenuViewReset(wxCommandEvent& WXUNUSED(event))
{
	m_dh->zoom = 1;
	m_dh->xMove = 0;
	m_dh->yMove = 0;
	refreshAllGLWidgets();
}
void MainFrame::OnMenuViewLeft(wxCommandEvent& WXUNUSED(event))
{
	Matrix4fSetIdentity(&m_dh->m_transform);
	m_dh->m_transform.s.M00 = 0.0;
	m_dh->m_transform.s.M11 = 0.0;
	m_dh->m_transform.s.M22 = 0.0;
	m_dh->m_transform.s.M20 = -1.0;
	m_dh->m_transform.s.M01 = -1.0;
	m_dh->m_transform.s.M12 = 1.0;
	m_mainGL->setRotation();
}
void MainFrame::OnMenuViewRight(wxCommandEvent& WXUNUSED(event))
{
	Matrix4fSetIdentity(&m_dh->m_transform);
	m_dh->m_transform.s.M00 = 0.0;
	m_dh->m_transform.s.M11 = 0.0;
	m_dh->m_transform.s.M22 = 0.0;
	m_dh->m_transform.s.M20 = 1.0;
	m_dh->m_transform.s.M01 = 1.0;
	m_dh->m_transform.s.M12 = 1.0;
	m_mainGL->setRotation();
}
void MainFrame::OnMenuViewTop(wxCommandEvent& WXUNUSED(event))
{
	Matrix4fSetIdentity(&m_dh->m_transform);
	m_mainGL->setRotation();
}
void MainFrame::OnMenuViewBottom(wxCommandEvent& WXUNUSED(event))
{
	Matrix4fSetIdentity(&m_dh->m_transform);
	m_dh->m_transform.s.M00 = -1.0;
	m_dh->m_transform.s.M22 = -1.0;
	m_mainGL->setRotation();
}
void MainFrame::OnMenuViewFront(wxCommandEvent& WXUNUSED(event))
{
	Matrix4fSetIdentity(&m_dh->m_transform);
	m_dh->m_transform.s.M11 = 0.0;
	m_dh->m_transform.s.M22 = 0.0;
	m_dh->m_transform.s.M12 = 1.0;
	m_dh->m_transform.s.M21 = -1.0;
	m_mainGL->setRotation();
}
void MainFrame::OnMenuViewBack(wxCommandEvent& WXUNUSED(event))
{
	Matrix4fSetIdentity(&m_dh->m_transform);
	m_dh->m_transform.s.M00 = -1.0;
	m_dh->m_transform.s.M11 = 0.0;
	m_dh->m_transform.s.M22 = 0.0;
	m_dh->m_transform.s.M21 = 1.0;
	m_dh->m_transform.s.M12 = 1.0;
	m_mainGL->setRotation();
}
void MainFrame::OnMenuViewCrosshair(wxCommandEvent& WXUNUSED(event))
{
	m_dh->showCrosshair = !m_dh->showCrosshair;
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 * Menu VOI
 *
 ****************************************************************************************************/
void MainFrame::OnToggleSelBox(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene || !m_dh->fibers_loaded) return;

	// check if selection box selected
	wxTreeItemId tBoxId = m_treeWidget->GetSelection();

	if (treeSelected(tBoxId) == MasterBox)
	{
		SelectionBox *box =  (SelectionBox*)(m_treeWidget->GetItemData(tBoxId));
		box->toggleActive();
		m_treeWidget->SetItemImage(tBoxId, box->getIcon());
		box->setDirty(true);

		int childboxes = m_treeWidget->GetChildrenCount(tBoxId);
		wxTreeItemIdValue childcookie = 0;
		for (int i = 0 ; i < childboxes ; ++i)
		{
			wxTreeItemId childId = m_treeWidget->GetNextChild(tBoxId, childcookie);
			if (childId.IsOk()) {
				SelectionBox *childBox = ((SelectionBox*)(m_treeWidget->GetItemData(childId)));
				childBox->setActive ( box->getActive() );
				m_treeWidget->SetItemImage(childId, childBox->getIcon());
				childBox->setDirty(true);
			}
		}
	}
	else if (treeSelected(tBoxId) == ChildBox)
	{
		SelectionBox *box =  (SelectionBox*)(m_treeWidget->GetItemData(tBoxId));
		box->toggleActive();
		m_treeWidget->SetItemImage(tBoxId, box->getIcon());
		box->setDirty(true);
	}

	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 * 
 *
 ****************************************************************************************************/
void MainFrame::OnToggleAndNot(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene || !m_dh->fibers_loaded) return;

	// check if selection box selected
	wxTreeItemId tBoxId = m_treeWidget->GetSelection();


	if (treeSelected(tBoxId) == ChildBox)
	{
		SelectionBox *box =  (SelectionBox*)(m_treeWidget->GetItemData(tBoxId));
		box->toggleNOT();
		
	
		wxTreeItemId parentid = m_treeWidget->GetItemParent(tBoxId);
		((SelectionBox*) (m_treeWidget->GetItemData(parentid)))->setDirty(true);

		if (((SelectionBox*) (m_treeWidget->GetItemData(tBoxId)))->getNOT())
			m_treeWidget->SetItemBackgroundColour(tBoxId, *wxRED);
		else
			m_treeWidget->SetItemBackgroundColour(tBoxId, *wxGREEN);
				
		m_treeWidget->SetItemImage(tBoxId, box->getIcon());
		box->setDirty(true);
	}

	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnToggleShowBox(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene || !m_dh->fibers_loaded) return;

	// check if selection box selected
	wxTreeItemId tBoxId = m_treeWidget->GetSelection();

	if (treeSelected(tBoxId) == MasterBox)
	{
		SelectionBox *box =  (SelectionBox*)(m_treeWidget->GetItemData(tBoxId));
		box->toggleShow();
		m_treeWidget->SetItemImage(tBoxId, box->getIcon());
		box->setDirty(true);

		int childboxes = m_treeWidget->GetChildrenCount(tBoxId);
		wxTreeItemIdValue childcookie = 0;
		for (int i = 0 ; i < childboxes ; ++i)
		{
			wxTreeItemId childId = m_treeWidget->GetNextChild(tBoxId, childcookie);
			if (childId.IsOk()) {
				SelectionBox *childBox = ((SelectionBox*)(m_treeWidget->GetItemData(childId)));
				childBox->setVisible( box->getShow() );
				m_treeWidget->SetItemImage(childId, childBox->getIcon());
				childBox->setDirty(true);
			}
		}
	}
	else if (treeSelected(tBoxId) == ChildBox)
	{
		SelectionBox *box =  (SelectionBox*)(m_treeWidget->GetItemData(tBoxId));
		box->toggleShow();
		m_treeWidget->SetItemImage(tBoxId, box->getIcon());
		box->setDirty(true);
	}

	m_dh->m_selBoxChanged = true;
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnRenameBox(wxCommandEvent& WXUNUSED(event))
{
	wxTreeItemId tBoxId = m_treeWidget->GetSelection();
	if (treeSelected(tBoxId) == MasterBox || treeSelected(tBoxId) == ChildBox)
	{
		SelectionBox *box =  (SelectionBox*)(m_treeWidget->GetItemData(tBoxId));

		wxTextEntryDialog dialog(this, _T("Please enter a new name"));
		dialog.SetValue(box->getName());
		if ( (dialog.ShowModal() == wxID_OK ) && ( dialog.GetValue() != _T("") ))
			box->setName(dialog.GetValue());
		m_treeWidget->SetItemText(tBoxId, box->getName());
	}
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnNewSelBox(wxCommandEvent& WXUNUSED(event))
{
	createNewSelBox();
}
void MainFrame::createNewSelBox()
{
	if (!m_dh->scene || !m_dh->fibers_loaded) return;

	Vector vc (m_xSlider->GetValue(), m_ySlider->GetValue(), m_zSlider->GetValue() );
	Vector vs ( m_dh->columns/8,m_dh->rows/8, m_dh->frames/8 );
	SelectionBox *selBox = new SelectionBox(vc, vs, m_dh);

	// check if selection box selected
	wxTreeItemId tBoxId = m_treeWidget->GetSelection();
	wxTreeItemId pId = m_treeWidget->GetItemParent(tBoxId);

	if (treeSelected(tBoxId) == MasterBox)
	{
		// box is under another box
		wxTreeItemId tNewBoxId = m_treeWidget->AppendItem(tBoxId, wxT("box"),0, -1, selBox);
		m_treeWidget->SetItemBackgroundColour(tNewBoxId, *wxGREEN);
		m_treeWidget->EnsureVisible(tNewBoxId);
		m_treeWidget->SetItemImage(tNewBoxId, selBox->getIcon());
		selBox->setTreeId(tNewBoxId);
	}
	else
	{
		// box is top
		selBox->setIsMaster(true);
		wxTreeItemId tNewBoxId = m_treeWidget->AppendItem(m_tSelBoxId, wxT("box"),0, -1, selBox);
		m_treeWidget->SetItemBackgroundColour(tNewBoxId, *wxCYAN);
		m_treeWidget->EnsureVisible(tNewBoxId);
		m_treeWidget->SetItemImage(tNewBoxId, selBox->getIcon());
		selBox->setTreeId(tNewBoxId);
	}

	m_dh->m_selBoxChanged = true;
	refreshAllGLWidgets();	
}

/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnNewFromOverlay(wxCommandEvent& WXUNUSED(event))
{
	wxTreeItemId tBoxId = m_treeWidget->GetSelection();
	wxTreeItemId pId = m_treeWidget->GetItemParent(tBoxId);
	SelectionBox *selBox = NULL;
	Anatomy* a = NULL;
	
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		DatasetInfo* info = (DatasetInfo*)m_listCtrl->GetItemData(item);
		if (info->getType() == Overlay )
		{
			a = (Anatomy*) m_listCtrl->GetItemData(item);
			selBox = new SelectionBox(m_dh, a);
			selBox->setThreshold(a->getThreshold());
		}
	}
	else 
		return;
		
	if (treeSelected(tBoxId) == MasterBox)
	{
		wxTreeItemId tNewBoxId = m_treeWidget->AppendItem(tBoxId, wxT("ROI"),0, -1, selBox);
		m_treeWidget->SetItemBackgroundColour(tNewBoxId, *wxGREEN);
		m_treeWidget->EnsureVisible(tNewBoxId);
		m_treeWidget->SetItemImage(tNewBoxId, selBox->getIcon());
		selBox->setTreeId(tNewBoxId);
		selBox->setIsMaster(false);
	}
	else
	{
		wxTreeItemId tNewBoxId = m_treeWidget->AppendItem(m_tSelBoxId, wxT("ROI"),0, -1, selBox);
		m_treeWidget->SetItemBackgroundColour(tNewBoxId, *wxCYAN);
		m_treeWidget->EnsureVisible(tNewBoxId);
		m_treeWidget->SetItemImage(tNewBoxId, selBox->getIcon());
		selBox->setTreeId(tNewBoxId);
		selBox->setIsMaster(true);
		 
	}
	
	a->m_roi = selBox;
			
	m_dh->m_selBoxChanged = true;
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnHideSelBoxes(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene) return;
	m_dh->toggleBoxes();
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnUseMorph(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene) return;
	m_dh->morphing = !m_dh->morphing;
}
/****************************************************************************************************
 *
 * 
 *
 ****************************************************************************************************/
void MainFrame::OnColorRoi(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene || !m_dh->fibers_loaded) return;

	// check if selection box selected
	wxTreeItemId tBoxId = m_treeWidget->GetSelection();
	SelectionBox *box =  (SelectionBox*)(m_treeWidget->GetItemData(tBoxId));

	
	
	wxColourData colorData;

	for ( int i = 0; i < 10 ; ++i)
	{
		wxColour color(i*28, i*28, i*28);
		colorData.SetCustomColour(i, color);
	}
	int i = 10;
	wxColour color(255, 0, 0);
	colorData.SetCustomColour(i++, color);
	wxColour color1(0, 255, 0);
	colorData.SetCustomColour(i++, color1);
	wxColour color2(0, 0, 255);
	colorData.SetCustomColour(i++, color2);
	wxColour color3(255, 255, 0);
	colorData.SetCustomColour(i++, color3);
	wxColour color4(255, 0, 255);
	colorData.SetCustomColour(i++, color4);
	wxColour color5(0, 255, 255);
	colorData.SetCustomColour(i++, color5);

	wxColourDialog dialog(this, &colorData);
	wxColour col;
	if (dialog.ShowModal() == wxID_OK)
	{
		wxColourData retData = dialog.GetColourData();
		col = retData.GetColour();
	}
	else return;

	box->setColor(col);
	
	refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 * Menu Spline Surface
 *
 ****************************************************************************************************/
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnNewSurface(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene || m_dh->surface_loaded) return;

	int xs = m_xSlider->GetValue();

	//delete all existing points
	m_treeWidget->DeleteChildren(m_tPointId);
	Fibers *fibers = NULL;

	if ( m_dh->fibers_loaded )
		m_dh->getFiberDataset(fibers);

	for ( int i = 0 ; i < 11 ; ++i)
		for ( int j = 0 ; j < 11 ; ++j )
		{

			int yy = (m_dh->rows/10)*i;
			int zz = (m_dh->frames/10)*j;

			// create the point
			SplinePoint *point = new SplinePoint(xs, yy, zz, m_dh);

			if (i == 0 || i == 10 || j == 0 || j == 10)
			{
				wxTreeItemId tId = m_treeWidget->AppendItem(m_tPointId, wxT("boundary point"),-1, -1, point);
				point->setTreeId(tId);
				point->setIsBoundary(true);
			}
			else
			{
				if (m_dh->fibers_loaded && fibers->getBarycenter(point)) {
					wxTreeItemId tId = m_treeWidget->AppendItem(m_tPointId, wxT("point"),-1, -1, point);
					point->setTreeId(tId);
					point->setIsBoundary(false);
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

	Surface *surface = new Surface(m_dh);
	m_listCtrl->InsertItem(id, wxT(""), 0);
	m_listCtrl->SetItem(id, 1, surface->getName());
	m_listCtrl->SetItem(id, 2, wxT("0.50"));
	m_listCtrl->SetItem(id, 3, wxT(""), 1);
	m_listCtrl->SetItemData(id, (long)surface);
	m_listCtrl->SetItemState(id, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnTogglePointMode(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene) return;
	m_dh->togglePointMode();
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnToggleDrawVectors(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene) return;
	m_dh->drawVectors = !m_dh->drawVectors;
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnToggleLIC(wxCommandEvent& WXUNUSED(event))
{
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1 && m_dh->vectors_loaded)
	{
		DatasetInfo* info = (DatasetInfo*)m_listCtrl->GetItemData(item);
		info->activateLIC();
	}
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnToggleNormal(wxCommandEvent& WXUNUSED(event))
{
	m_dh->normalDirection *= -1.0;

	for (int i = 0 ; i < m_listCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_listCtrl->GetItemData(i);
		if (info->getType() == Surface_)
		{
			Surface* surf = (Surface*)m_listCtrl->GetItemData(i);
			surf->flipNormals();
		}
	}
	refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 * Menu Options
 *
 ****************************************************************************************************/
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnToggleTextureFiltering(wxCommandEvent& WXUNUSED(event))
{
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		DatasetInfo* info = (DatasetInfo*)m_listCtrl->GetItemData(item);
		if (info->getType() < Mesh_ )
		{
			if (!info->toggleShowFS())
				m_listCtrl->SetItem(item, 1, info->getName() + wxT("*"));
			else
				m_listCtrl->SetItem(item, 1, info->getName());
		}
	}
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnToggleLighting(wxCommandEvent& WXUNUSED(event))
{
	m_dh->lighting = !m_dh->lighting;
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnToggleBlendTexOnMesh(wxCommandEvent& WXUNUSED(event))
{
	m_dh->blendTexOnMesh = !m_dh->blendTexOnMesh;
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnToggleFilterIso(wxCommandEvent& WXUNUSED(event))
{
	m_dh->filterIsoSurf = !m_dh->filterIsoSurf;
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnClean(wxCommandEvent& WXUNUSED(event))
{
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		DatasetInfo* info = (DatasetInfo*)m_listCtrl->GetItemData(item);
		if (info->getType() == Mesh_ || info->getType() ==  IsoSurface_)
		{
			info->clean();
		}
	}
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnLoop(wxCommandEvent& WXUNUSED(event))
{
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		DatasetInfo* info = (DatasetInfo*)m_listCtrl->GetItemData(item);
		info->smooth();
	}
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnInvertFibers(wxCommandEvent& WXUNUSED(event))
{
	m_dh->invertFibers();
	m_dh->m_selBoxChanged = true;
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnUseFakeTubes(wxCommandEvent& WXUNUSED(event))
{
	m_dh->useFakeTubes = !m_dh->useFakeTubes;
	Fibers* f;
	if ( m_dh->getFiberDataset(f))
		f->switchNormals(!m_dh->useFakeTubes);
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnUseTransparency(wxCommandEvent& WXUNUSED(event))
{
	m_dh->useTransparency = !m_dh->useTransparency;
//	Fibers* f;
//	if ( m_dh->getFiberDataset(f))
//		f->switchNormals(!m_dh->useFakeTubes);
	refreshAllGLWidgets();
}/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnAssignColor(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene) return;

	wxColourData colorData;

	for ( int i = 0; i < 10 ; ++i)
	{
		wxColour color(i*28, i*28, i*28);
		colorData.SetCustomColour(i, color);
	}
	int i = 10;
	wxColour color(255, 0, 0);
	colorData.SetCustomColour(i++, color);
	wxColour color1(0, 255, 0);
	colorData.SetCustomColour(i++, color1);
	wxColour color2(0, 0, 255);
	colorData.SetCustomColour(i++, color2);
	wxColour color3(255, 255, 0);
	colorData.SetCustomColour(i++, color3);
	wxColour color4(255, 0, 255);
	colorData.SetCustomColour(i++, color4);
	wxColour color5(0, 255, 255);
	colorData.SetCustomColour(i++, color5);

	wxColourDialog dialog(this, &colorData);
	wxColour col;
	if (dialog.ShowModal() == wxID_OK)
	{
		wxColourData retData = dialog.GetColourData();
		col = retData.GetColour();
	}
	else return;

	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		DatasetInfo* info = (DatasetInfo*)m_listCtrl->GetItemData(item);
		if (info->getType() == Mesh_ || info->getType() == IsoSurface_ || info->getType() == Surface_ )
		{
			info->setColor(col);
			info->setuseTex(false);
			m_listCtrl->SetItem(item, 2, wxT("(") + wxString::Format(wxT("%.2f"), info->getThreshold()) + wxT(")") );
			refreshAllGLWidgets();
			return;
		}
	}

	wxTreeItemId tBoxId = m_treeWidget->GetSelection();
	if (treeSelected(tBoxId) == MasterBox)
	{
		SelectionBox *box = (SelectionBox*)(m_treeWidget->GetItemData(tBoxId));
		box->setFiberColor(col);
		box->setDirty(true);
	}
	else
	{
		if (treeSelected(tBoxId) == ChildBox)
		{
			SelectionBox *box = (SelectionBox*)(m_treeWidget->GetItemData(m_treeWidget->GetItemParent(tBoxId)));
			box->setFiberColor(col);
			box->setDirty(true);
		}
	}
	m_dh->m_selBoxChanged = true;
	refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnResetColor(wxCommandEvent& WXUNUSED(event))
{
	Fibers* fibers = NULL; // initalize it quiet compiler
	if (!m_dh->getFiberDataset(fibers)) return;
	fibers->resetColorArray();
	m_dh->m_selBoxChanged = true;
	refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnToggleColorMapLegend(wxCommandEvent& WXUNUSED(event))
{
	m_dh->showColorMapLegend = !m_dh->showColorMapLegend;
	refreshAllGLWidgets();
}

void MainFrame::OnSetCMap0(wxCommandEvent& WXUNUSED(event)) {m_dh->colorMap = 0;refreshAllGLWidgets();}
void MainFrame::OnSetCMap1(wxCommandEvent& WXUNUSED(event)) {m_dh->colorMap = 1;refreshAllGLWidgets();}
void MainFrame::OnSetCMap2(wxCommandEvent& WXUNUSED(event)) {m_dh->colorMap = 2;refreshAllGLWidgets();}
void MainFrame::OnSetCMap3(wxCommandEvent& WXUNUSED(event)) {m_dh->colorMap = 3;refreshAllGLWidgets();}
void MainFrame::OnSetCMap4(wxCommandEvent& WXUNUSED(event)) {m_dh->colorMap = 4;refreshAllGLWidgets();}
void MainFrame::OnSetCMap5(wxCommandEvent& WXUNUSED(event)) {m_dh->colorMap = 5;refreshAllGLWidgets();}
void MainFrame::OnSetCMapNo(wxCommandEvent& WXUNUSED(event)) {m_dh->colorMap = -1;refreshAllGLWidgets();}
/****************************************************************************************************
 *
 * Menu Help
 *
 ****************************************************************************************************/
void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxString rev = _T("$Revision$");
	rev = rev.AfterFirst('$');
	rev = rev.BeforeLast('$');
	wxString date = _T("$Date$");
	date = date.AfterFirst('$');
	date = date.BeforeLast('$');
	(void)wxMessageBox(_T("Fiber Navigator\nAuthor: Ralph Schurade (c) 2008\n\n") + rev + _T("\n") + date, _T("About Fiber Navigator"));
}

void MainFrame::OnShortcuts(wxCommandEvent& WXUNUSED(event))
{
	wxString nl = _T("\n");
	(void)wxMessageBox(_T("Keyboard Shortcuts") + nl
			+ _T("_________________________________________________________") + nl
			+ _T("Move selected box:") + nl
			+ _T("   cursor up/down/left/right, page up/down") + nl
			+ _T("Move selected box larger steps:") + nl
			+ _T("   shift + cursor up/down/left/right, page up/down") + nl
			+ _T("Resize selected box:") + nl
			+ _T("   ctrl + cursor up/down/left/right, page up/down") + nl
			+ _T("Resize selected box larger steps:") + nl
			+ _T("   ctrl + shift + cursor up/down/left/right, page up/down") + nl
			+ _T("Delete selected box and all sub boxes:") + nl
			+ _T("   del") + nl
);
}

void MainFrame::OnScreenshot(wxCommandEvent& WXUNUSED(event))
{
	wxString caption = wxT("Choose a file");
	wxString wildcard = wxT("PPM files (*.ppm)|*.ppm||*.*|*.*");
	wxString defaultDir = wxEmptyString;
	wxString defaultFilename = wxEmptyString;
	wxFileDialog dialog(this, caption, defaultDir, defaultFilename, wildcard, wxSAVE);
	dialog.SetFilterIndex(0);
	dialog.SetDirectory(m_dh->m_screenshotPath);
	if (dialog.ShowModal() == wxID_OK)
	{
		m_dh->m_screenshotPath = dialog.GetDirectory();
		m_dh->m_screenshotName = dialog.GetPath();
		m_dh->scheduledScreenshot = true;
		m_mainGL->render();
		m_mainGL->render();
	}	
}

/****************************************************************************************************
 *
 * Events on Widgets, Sliders and Buttons
 *
 ****************************************************************************************************/
void MainFrame::OnXSliderMoved(wxCommandEvent& WXUNUSED(event))
{
	 m_dh->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());
	 refreshAllGLWidgets();
}

void MainFrame::OnYSliderMoved(wxCommandEvent& WXUNUSED(event))
{
	m_dh->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());
	refreshAllGLWidgets();
}

void MainFrame::OnZSliderMoved(wxCommandEvent& WXUNUSED(event))
{
	m_dh->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnTSliderMoved(wxCommandEvent& WXUNUSED(event))
{
	float threshold = (float)m_tSlider->GetValue()/100.0;

	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1) return;
	
	DatasetInfo *info = (DatasetInfo*) m_listCtrl->GetItemData(item);
	m_listCtrl->SetItem(item, 2, wxString::Format(wxT("%.2f"), threshold * info->getOldMax() ));
	
	info->setThreshold(threshold);
	if ( info->getType() == Surface_ )
	{
		Surface* s = (Surface*) m_listCtrl->GetItemData(item);
		s->movePoints();
	}
	if ( info->getType() == IsoSurface_ && !m_tSlider->leftDown())
	{
		CIsoSurface* s = (CIsoSurface*) m_listCtrl->GetItemData(item);
		s->GenerateWithThreshold();
	}
	if ( info->getType() == Overlay )
	{
		Anatomy* a = (Anatomy*) m_listCtrl->GetItemData(item);
		if ( a->m_roi )
		{
			a->m_roi->setThreshold( threshold );
		}
	}
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnTSlider2Moved(wxCommandEvent& WXUNUSED(event))
{
	float alpha = (float)m_tSlider2->GetValue()/100.0;

	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1) return;
	//m_listCtrl->SetItem(item, 2, wxString::Format(wxT("%.2f"), threshold ));

	DatasetInfo *info = (DatasetInfo*) m_listCtrl->GetItemData(item);
	info->setAlpha(alpha);

	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnButtonAxial(wxCommandEvent& WXUNUSED(event))
{
	if (m_dh->scene)
	{
		m_dh->showAxial = !m_dh->showAxial;
		m_mainGL->render();
	}
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnButtonCoronal(wxCommandEvent& WXUNUSED(event))
{
	if (m_dh->scene)
	{
		m_dh->showCoronal = !m_dh->showCoronal;
		m_mainGL->render();
	}
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnButtonSagittal(wxCommandEvent& WXUNUSED(event))
{
	if (m_dh->scene)
	{
		m_dh->showSagittal = !m_dh->showSagittal;
		m_mainGL->render();
	}
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnToggleAlpha(wxCommandEvent& WXUNUSED(event))
{
	m_dh->blendAlpha = !m_dh->blendAlpha;

	updateMenus();
	this->Update();
	//this->Refresh();

	
	m_mainGL->render();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnToggleLayout(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene) return;
	enlargeNav = (enlargeNav + 1) % 3;
	wxSize clientSize = GetClientSize();
	wxSize windowSize = GetSize();
	switch (enlargeNav)
	{
		case 1:
		{
			treeSizer->Show(false);
			buttonSizer->Show(false);
			m_listCtrl->Show(false);
			
			int newSize = (clientSize.y - 65)/3;
			
			m_gl0->SetMinSize(wxSize(100,100));
			m_gl1->SetMinSize(wxSize(100,100));
			m_gl2->SetMinSize(wxSize(100,100));
			m_gl0->SetMaxSize(wxSize(newSize,newSize));
			m_gl1->SetMaxSize(wxSize(newSize,newSize));
			m_gl2->SetMaxSize(wxSize(newSize,newSize));
			
			m_xSlider->SetMinSize(wxSize(newSize,-1));
			m_ySlider->SetMinSize(wxSize(newSize,-1));
			m_zSlider->SetMinSize(wxSize(newSize,-1));
			
			navSizer1 = new wxBoxSizer( wxVERTICAL );
			navSizer1->Add( m_gl0, 1, wxALL | wxEXPAND | wxSHAPED, 1 );
			navSizer1->Add( m_zSlider, 0, wxALL, 1 );
			navSizer1->Add( m_gl1, 1, wxALL  | wxEXPAND | wxSHAPED, 1 );
			navSizer1->Add( m_ySlider, 0, wxALL, 1 );
			navSizer1->Add( m_gl2, 1, wxALL | wxEXPAND | wxSHAPED, 1 );
			navSizer1->Add( m_xSlider, 0, wxALL, 1 );
			
			topSizer1 = new wxBoxSizer( wxHORIZONTAL );
			topSizer1->Add(navSizer1, 0, wxALL | wxEXPAND, 1);
			topSizer1->Add( m_mainGL, 1, wxEXPAND | wxALL, 2 );
	
			SetSizer( topSizer1 );
			topSizer1->SetSizeHints( this );
			break;
		}
		case 2:
		{
			m_treeWidget->Show(true);
			m_listCtrl->Show(true);
			m_xSlider->Show(false);
			m_ySlider->Show(false);
			m_zSlider->Show(false);
			m_tSlider->Show(true);
			m_tSlider2->Show(true);
			buttonUp->Show(true);
			buttonDown->Show(true);
			buttonLoad1->Show(true);
			buttonLoad2->Show(true);
			buttonLoad3->Show(true);
			
			m_gl0->SetMinSize(wxSize(100, 100));
			m_gl1->SetMinSize(wxSize(100, 100));
			m_gl2->SetMinSize(wxSize(100, 100));
			m_mainGL->SetMinSize(wxSize(100, 100));
			m_gl0->SetMaxSize(wxSize(10000, 10000));
			m_gl1->SetMaxSize(wxSize(10000, 10000));
			m_gl2->SetMaxSize(wxSize(10000, 10000));
			m_mainGL->SetMaxSize(wxSize(10000, 10000));
						
			topSizer 		= new wxBoxSizer( wxHORIZONTAL );
			leftSizer 		= new wxBoxSizer( wxVERTICAL );
			buttonSizer 	= new wxBoxSizer( wxHORIZONTAL );
			treeSizer		= new wxBoxSizer( wxVERTICAL );
			
			m_treeWidget->SetMinSize(wxSize(150, 350));
			treeSizer->Add( buttonLoad1, 0, wxALL | wxALIGN_CENTER, 0 );
			treeSizer->Add( buttonLoad2, 0, wxALL | wxALIGN_CENTER, 0 );
			treeSizer->Add( buttonLoad3, 0, wxALL | wxALIGN_CENTER, 0 );
			treeSizer->Add( m_treeWidget, 1, wxALL, 1 );
					    
		    buttonSizer->Add( buttonUp, 0, wxALL, 1 );
		    buttonSizer->Add( buttonDown, 0, wxALL, 1 );
		    buttonSizer->Add( m_tSlider, 0, wxALL, 1 );
		    buttonSizer->Add( m_tSlider2, 0, wxALL, 1 );
		    
		    leftSizer->Add(treeSizer, 0, wxALL, 1 );
		    leftSizer->Add( m_listCtrl, 1, wxALL | wxEXPAND, 1 );
		    leftSizer->Add( buttonSizer, 0, wxALIGN_BOTTOM | wxALL, 1 );
		
			topSizer2 = new wxFlexGridSizer(2,2,1,1);
			topSizer2->Add(m_gl0, 1, wxALL | wxEXPAND | wxSHAPED, 1);
			topSizer2->Add(m_gl1, 1, wxALL | wxEXPAND | wxSHAPED, 1);
			topSizer2->Add(m_gl2, 1, wxALL | wxEXPAND | wxSHAPED, 1);
			topSizer2->Add(m_mainGL, 1, wxALL | wxEXPAND | wxSHAPED, 1);
					
			topSizer->Add(leftSizer, 0 , wxALL | wxEXPAND, 1);
			topSizer->Add(topSizer2, 1 , wxALL | wxEXPAND, 1);
			
			SetSizer( topSizer );
			topSizer->SetSizeHints( this );
			break;
		}
		default:
		{
			m_treeWidget->Show(true);
			m_listCtrl->Show(true);
			m_xSlider->Show(true);
			m_ySlider->Show(true);
			m_zSlider->Show(true);
			m_tSlider->Show(true);
			m_tSlider2->Show(true);
			buttonUp->Show(true);
			buttonDown->Show(true);
			buttonLoad1->Show(true);
			buttonLoad2->Show(true);
			buttonLoad3->Show(true);
			m_gl0->SetMinSize(wxSize(150,150));
			m_gl1->SetMinSize(wxSize(150,150));
			m_gl2->SetMinSize(wxSize(150,150));
			m_mainGL->SetMinSize(wxSize(400,400));
			m_gl0->SetMaxSize(wxSize(150,150));
			m_gl1->SetMaxSize(wxSize(150,150));
			m_gl2->SetMaxSize(wxSize(150,150));
			m_mainGL->SetMaxSize(wxSize(10000,10000));
			
			m_xSlider->SetMinSize(wxSize(150,-1));
			m_ySlider->SetMinSize(wxSize(150,-1));
			m_zSlider->SetMinSize(wxSize(150,-1));
			
			topSizer 		= new wxBoxSizer( wxHORIZONTAL );
			leftSizer 		= new wxBoxSizer( wxVERTICAL );
			leftTopSizer 	= new wxBoxSizer( wxHORIZONTAL );
			navSizer 		= new wxBoxSizer( wxVERTICAL );
			buttonSizer 	= new wxBoxSizer( wxHORIZONTAL );
			treeSizer		= new wxBoxSizer( wxVERTICAL );
			
			navSizer->Add( m_gl0, 1, wxALL | wxEXPAND | wxSHAPED, 1 );
			navSizer->Add( m_zSlider, 0, wxALL, 1 );
			navSizer->Add( m_gl1, 1, wxALL  | wxEXPAND | wxSHAPED, 1 );
			navSizer->Add( m_ySlider, 0, wxALL, 1 );
			navSizer->Add( m_gl2, 1, wxALL | wxEXPAND | wxSHAPED, 1 );
			navSizer->Add( m_xSlider, 0, wxALL, 1 );
			
			treeSizer->Add( buttonLoad1, 0, wxALL | wxALIGN_CENTER, 0 );
			treeSizer->Add( buttonLoad2, 0, wxALL | wxALIGN_CENTER, 0 );
			treeSizer->Add( buttonLoad3, 0, wxALL | wxALIGN_CENTER, 0 );
			treeSizer->Add( m_treeWidget, 1, wxALL, 1 );
			
			leftTopSizer->Add( treeSizer, 0, wxALL | wxEXPAND, 0 );
			leftTopSizer->Add( navSizer, 1, wxALL | wxEXPAND, 0 );
		    
		    buttonSizer->Add( buttonUp, 0, wxALL, 1 );
		    buttonSizer->Add( buttonDown, 0, wxALL, 1 );
		    buttonSizer->Add( m_tSlider, 0, wxALL, 1 );
		    buttonSizer->Add( m_tSlider2, 0, wxALL, 1 );
		    
		    leftSizer->Add( leftTopSizer, 0, wxALL, 1 );
		    leftSizer->Add( m_listCtrl, 1, wxALL | wxEXPAND, 1 );
		    leftSizer->Add( buttonSizer, 0, wxALIGN_BOTTOM | wxALL, 1 );
		    
		    topSizer->Add( leftSizer, 0, wxEXPAND | wxALL, 0 );
		    topSizer->Add( m_mainGL, 1, wxEXPAND | wxALL, 2 );
	
		    SetSizer( topSizer );
		    topSizer->SetSizeHints( this );
			break;
		}
	}
	GetSizer()->SetDimension(0,0, clientSize.x, clientSize.y);
	m_listCtrl->SetColumnWidth(1, m_listCtrl->GetSize().x - 110);
	SetSize(windowSize);

	m_mainGL->changeOrthoSize();
	updateMenus();
	this->Update();
	this->Refresh();

	
	m_mainGL->render();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::refreshAllGLWidgets()
{
	updateStatusBar();
	updateMenus();
	
#if defined(__WXMAC__)
	if (m_gl0) m_gl0->Refresh();
	if (m_gl1) m_gl1->Refresh();
	if (m_gl2) m_gl2->Refresh();
	if (m_mainGL) m_mainGL->Refresh();
#else
	if (m_gl0) m_gl0->render();
	if (m_gl1) m_gl1->render();
	if (m_gl2) m_gl2->render();
	if (m_mainGL) m_mainGL->render();
#endif
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::renewAllGLWidgets()
{
	if (m_mainGL) m_mainGL->invalidate();
	if (m_gl0) m_gl0->invalidate();
	if (m_gl1) m_gl1->invalidate();
	if (m_gl2) m_gl2->invalidate();
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::updateStatusBar()
{
	wxString sbString0 = wxT("");
	sbString0 = wxString::Format(wxT("Position: %d  %d  %d"), m_xSlider->GetValue(), m_ySlider->GetValue(), m_zSlider->GetValue());
	GetStatusBar()->SetStatusText(sbString0,0);
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
void MainFrame::OnActivateListItem(wxListEvent& event)
{
	int item = event.GetIndex();
	DatasetInfo *info = (DatasetInfo*) m_listCtrl->GetItemData(item);
	int col = m_listCtrl->getColActivated();
	switch (col)
	{
	case 11:
		if (!info->toggleShowFS())
			m_listCtrl->SetItem(item, 1, info->getName().BeforeFirst('.') + wxT("*"));
		else
			m_listCtrl->SetItem(item, 1, info->getName().BeforeFirst('.'));
		break;
	case 13:
		delete info;
		m_listCtrl->DeleteItem(item);
		m_dh->updateLoadStatus();
		break;
	default:
		break;
	}

	refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnSelectListItem(wxListEvent& event)
{
	int item = event.GetIndex();
	if (item == -1) return;
	DatasetInfo *info = (DatasetInfo*) m_listCtrl->GetItemData(item);
	updateMenus();

	m_tSlider->SetValue((int)(info->getThreshold()*100));
	m_tSlider2->SetValue((int)(info->getAlpha()*100));

	int col = m_listCtrl->getColClicked();
	switch (col)
	{
	case 10:
		if (info->toggleShow())
		{
			m_listCtrl->SetItem(item, 0, wxT(""), 0);
		}
		else
		{
			m_listCtrl->SetItem(item, 0, wxT(""), 1);
		}
		refreshAllGLWidgets();
		break;
	case 12:
		if (info->getType() >= Mesh_)
		{
			if (!info->toggleUseTex())
				m_listCtrl->SetItem(item, 2, wxT("(") + wxString::Format(wxT("%.2f"), info->getThreshold()) + wxT(")") );
			else
				m_listCtrl->SetItem(item, 2, wxString::Format(wxT("%.2f"), info->getThreshold() ));
		}
		break;

	default:
		break;
	}
	m_mainGL->changeOrthoSize();
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnListItemUp(wxCommandEvent& WXUNUSED(event))
{
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1) return;
	m_listCtrl->moveItemUp(item);
	m_listCtrl->EnsureVisible(item);
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnListItemDown(wxCommandEvent& WXUNUSED(event))
{
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1) return;
	m_listCtrl->moveItemDown(item);
	m_listCtrl->EnsureVisible(item);
	refreshAllGLWidgets();
}

void MainFrame::OnListMenuName(wxCommandEvent&  WXUNUSED(event))
{
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1) return;
	DatasetInfo *info = (DatasetInfo*) m_listCtrl->GetItemData(item);
	if (!info->toggleShowFS())
		m_listCtrl->SetItem(item, 1, info->getName().BeforeFirst('.') + wxT("*"));
	else
		m_listCtrl->SetItem(item, 1, info->getName().BeforeFirst('.') );
}

void MainFrame::OnListMenuThreshold(wxCommandEvent&  WXUNUSED(event))
{
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1) return;
	DatasetInfo *info = (DatasetInfo*) m_listCtrl->GetItemData(item);
	if (info->getType() >= Mesh_)
	{
		if (!info->toggleUseTex())
			m_listCtrl->SetItem(item, 2, wxT("(") + wxString::Format(wxT("%.2f"), info->getThreshold()) + wxT(")") );
		else
			m_listCtrl->SetItem(item, 2, wxString::Format(wxT("%.2f"), info->getThreshold() ));
	}
}

void MainFrame::OnListMenuDelete(wxCommandEvent&  WXUNUSED(event))
{
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1) return;
	m_listCtrl->DeleteItem(item);
	// anatomy deleted? check if another one is still present
	m_dh->updateLoadStatus();
	refreshAllGLWidgets();
}

void MainFrame::OnListMenuShow(wxCommandEvent&  WXUNUSED(event))
{

	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1) return;
	DatasetInfo *info = (DatasetInfo*) m_listCtrl->GetItemData(item);
	if (info->toggleShow())
	{
		m_listCtrl->SetItem(item, 0, wxT(""), 0);
	}
	else
	{
		m_listCtrl->SetItem(item, 0, wxT(""), 1);
	}
	refreshAllGLWidgets();
}

void MainFrame::OnListMenuCutOut(wxCommandEvent&  WXUNUSED(event))
{
	m_dh->createCutDataset();	
	
	refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 * Functions for tree widget event handling
 *
 ****************************************************************************************************/
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnSelectTreeItem(wxTreeEvent& WXUNUSED(event))
{
	wxTreeItemId treeid = m_treeWidget->GetSelection();
	int selected = treeSelected(treeid);

	bool flag = true;

	switch (selected)
	{
	case MasterBox:
	case ChildBox:
		if (m_dh->lastSelectedBox) m_dh->lastSelectedBox->unselect();
		m_dh->lastSelectedBox = (SelectionBox*)(m_treeWidget->GetItemData(treeid));
		m_dh->lastSelectedBox->select(false);
		/* Commented out for now, until we find a better solution 
		 * 
		for (int i = 0 ; i < m_listCtrl->GetItemCount() ; ++i)
		{
			DatasetInfo* info = (DatasetInfo*) m_listCtrl->GetItemData(i);
			if (info->getType() == Fibers_ )
			{
				m_listCtrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
			}
		}
		*/
		break;
	case Point_:
		if (m_dh->lastSelectedPoint) m_dh->lastSelectedPoint->unselect();
		m_dh->lastSelectedPoint = (SplinePoint*)(m_treeWidget->GetItemData(treeid));
		m_dh->lastSelectedPoint->select(false);
		break;
	default:
		break;
	}

	if ( !flag )
	{
		wxMessageBox(wxT("ERROR\n") + m_dh->lastError,  wxT(""), wxOK|wxICON_INFORMATION, NULL);
		GetStatusBar()->SetStatusText(wxT("ERROR"),1);
		GetStatusBar()->SetStatusText(m_dh->lastError,2);
		return;
	}

	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnActivateTreeItem(wxTreeEvent& WXUNUSED(event))
{
	wxTreeItemId treeid = m_treeWidget->GetSelection();

	int selected = treeSelected(treeid);
	if ( selected == MasterBox )
	{
		SelectionBox *box =  (SelectionBox*)(m_treeWidget->GetItemData(treeid));
		box->toggleActive();
		m_treeWidget->SetItemImage(treeid, box->getIcon());
		box->setDirty(true);

		int childboxes = m_treeWidget->GetChildrenCount(treeid);
		wxTreeItemIdValue childcookie = 0;
		for (int i = 0 ; i < childboxes ; ++i)
		{
			wxTreeItemId childId = m_treeWidget->GetNextChild(treeid, childcookie);
			if (childId.IsOk()) {
				SelectionBox *childBox = ((SelectionBox*)(m_treeWidget->GetItemData(childId)));
				childBox->setActive ( box->getActive() );
				m_treeWidget->SetItemImage(childId, childBox->getIcon());
				childBox->setDirty(true);
			}
		}
	}
	else if ( selected == ChildBox )
	{
		SelectionBox* box =  (SelectionBox*) (m_treeWidget->GetItemData(treeid));
		
		box->toggleNOT();
		wxTreeItemId parentid = m_treeWidget->GetItemParent(treeid);
		((SelectionBox*) (m_treeWidget->GetItemData(parentid)))->setDirty(true);
		if ( box->getNOT() )
			m_treeWidget->SetItemBackgroundColour(treeid, *wxRED);
		else
			m_treeWidget->SetItemBackgroundColour(treeid, *wxGREEN);
	}
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnTreeEvent(wxCommandEvent& WXUNUSED(event))
{
	m_dh->m_selBoxChanged = true;
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::OnTreeLabelEdit(wxTreeEvent& event)
{
	wxTreeItemId treeid = event.GetItem();
	int selected = treeSelected(treeid);
	if ( selected == ChildBox ||  selected == MasterBox )
	{
		((SelectionBox*) m_treeWidget->GetItemData(treeid))->setName(event.GetLabel());
	}
}
/****************************************************************************************************
 *
 * Helper function to determine what kind of item is selected in the tree widget
 *
 ****************************************************************************************************/
int MainFrame::treeSelected(wxTreeItemId id)
{
	wxTreeItemId pId = m_treeWidget->GetItemParent(id); 
	//if ( !pId.IsOk() ) return 0;
	wxTreeItemId ppId = m_treeWidget->GetItemParent(pId);
	//if ( !ppId.IsOk() ) return 0;

	if ( pId == m_tSelBoxId )
		return MasterBox;
	else if ( ppId == m_tSelBoxId )
		return ChildBox;
	else if ( pId == m_tPointId )
		return Point_;
	return 0;
}
/****************************************************************************************************
 *
 * 
 *
 ****************************************************************************************************/
void MainFrame::OnLoad1(wxCommandEvent& WXUNUSED(event))
{
	bool flag = m_dh->load(1);
	if (flag) m_dh->m_selBoxChanged = true;
}
void MainFrame::OnLoad2(wxCommandEvent& WXUNUSED(event))
{
	bool flag = m_dh->load(2);
	if (flag) m_dh->m_selBoxChanged = true;
}
void MainFrame::OnLoad3(wxCommandEvent& WXUNUSED(event))
{
	bool flag = m_dh->load(5);
	if (flag) m_dh->m_selBoxChanged = true;
}


/****************************************************************************************************
 *
 * Moves all boundary points in one direction
 *
 ****************************************************************************************************/
void MainFrame::OnMovePoints1(wxCommandEvent& WXUNUSED(event))
{
	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	id = m_treeWidget->GetFirstChild(m_tPointId, cookie);
	while ( id.IsOk() )
	{
		SplinePoint *point = (SplinePoint*)(m_treeWidget->GetItemData(id));
		if (point->isBoundary())
			point->setX(point->X() + 5.0);

		id = m_treeWidget->GetNextChild(m_tPointId, cookie);
	}

	m_dh->surface_isDirty = true;
	refreshAllGLWidgets();
}
/****************************************************************************************************
 *
 * Moves all boundary points in one direction
 *
 ****************************************************************************************************/
void MainFrame::OnMovePoints2(wxCommandEvent& WXUNUSED(event))
{
	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	id = m_treeWidget->GetFirstChild(m_tPointId, cookie);
	while ( id.IsOk() )
	{
		SplinePoint *point = (SplinePoint*)(m_treeWidget->GetItemData(id));
		if (point->isBoundary())
			point->setX(point->X() - 5.0);

		id = m_treeWidget->GetNextChild(m_tPointId, cookie);
	}

	m_dh->surface_isDirty = true;
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
void MainFrame::OnSize(wxSizeEvent& WXUNUSED(event))
{
	wxSize clientSize = this->GetClientSize();
	
	if (enlargeNav == 1)
	{
		int newSize = (clientSize.y - 65)/3;
		
if(m_gl0)
{	
		m_gl0->SetMinSize(wxSize(newSize,newSize));
		m_gl1->SetMinSize(wxSize(newSize,newSize));
		m_gl2->SetMinSize(wxSize(newSize,newSize));
		m_gl0->SetMaxSize(wxSize(newSize,newSize));
		m_gl1->SetMaxSize(wxSize(newSize,newSize));
		m_gl2->SetMaxSize(wxSize(newSize,newSize));
		
		m_xSlider->SetMinSize(wxSize(newSize,-1));
		m_ySlider->SetMinSize(wxSize(newSize,-1));
		m_zSlider->SetMinSize(wxSize(newSize,-1));
}
	}
	if (enlargeNav == 2)
	{
		int newSize = wxMin((clientSize.x - leftSizer->GetSize().x - 5)/2 ,clientSize.y /2 -5);
					
		m_gl0->SetMinSize(wxSize(newSize,newSize));
		m_gl1->SetMinSize(wxSize(newSize,newSize));
		m_gl2->SetMinSize(wxSize(newSize,newSize));
		m_mainGL->SetMinSize(wxSize(newSize,newSize));
		m_gl0->SetMaxSize(wxSize(newSize,newSize));
		m_gl1->SetMaxSize(wxSize(newSize,newSize));
		m_gl2->SetMaxSize(wxSize(newSize,newSize));
		m_mainGL->SetMaxSize(wxSize(newSize,newSize));
		
		m_xSlider->SetMinSize(wxSize(newSize,-1));
		m_ySlider->SetMinSize(wxSize(newSize,-1));
		m_zSlider->SetMinSize(wxSize(newSize,-1));
	}

if(GetSizer())
	GetSizer()->SetDimension(0,0, clientSize.x, clientSize.y);

if(m_mainGL)	
	m_mainGL->changeOrthoSize();

	this->Update();
	this->Refresh();
}

/****************************************************************************************************
 *
 * gets called when a thread for the kdTree creation finishes
 * this function is here because of some limitations in the event
 * handling system
 *
 ****************************************************************************************************/
void MainFrame::OnKdTreeThreadFinished(wxCommandEvent& WXUNUSED(event))
{
	m_dh->treeFinished();
}

/****************************************************************************************************
 *
 * OnGLEvent handles mouse events in the GL Refreshing widgets
 *
 ****************************************************************************************************/
void MainFrame::OnGLEvent( wxCommandEvent &event )
{
	wxPoint pos, newpos;
	float max = wxMax(m_dh->rows, wxMax(m_dh->columns, m_dh->frames));

#ifdef __WXMSW__
			float speedup = 2.0;
#else
			float speedup = 1.0;
#endif
	int NAV_GL_SIZE = m_gl0->GetSize().x;		

	switch (event.GetInt())
	{
	case axial: {
		pos = m_gl0->getMousePos();
		float x = ((float)pos.x/NAV_GL_SIZE) * max;
		float y = ((float)(NAV_GL_SIZE - pos.y)/NAV_GL_SIZE * max);

		m_xSlider->SetValue( (int)(x - (max - m_dh->columns)/2.0) );
		m_ySlider->SetValue( (int)(y) );
		break;
	}
	case coronal: {
		pos = m_gl1->getMousePos();
		float x = ((float)pos.x/NAV_GL_SIZE) * max;
		float y = ((float)(NAV_GL_SIZE - pos.y)/NAV_GL_SIZE * max);

		m_xSlider->SetValue( (int)(x - (max - m_dh->columns)/2.0) );
		m_zSlider->SetValue( (int)(y/max*m_dh->rows - 20 ) );
		break;
	}
	case sagittal: {
		pos = m_gl2->getMousePos();
		float x = ((float)pos.x/NAV_GL_SIZE) * max;
		float y = ((float)(NAV_GL_SIZE - pos.y)/NAV_GL_SIZE * max);

		m_ySlider->SetValue( (int)(max - x) );
		m_zSlider->SetValue( (int)(y/max*m_dh->rows - 20 ) );
		break;
	}
	case mainView:
		int delta = (int)m_mainGL->getDelta();

		switch (m_mainGL->getPicked())
		{
		case axial:
			m_zSlider->SetValue((int)wxMin(wxMax(m_zSlider->GetValue() + delta*speedup, 0), m_zSlider->GetMax()));
			break;
		case coronal:
			m_ySlider->SetValue((int)wxMin(wxMax(m_ySlider->GetValue() + delta*speedup, 0), m_ySlider->GetMax()));
			break;
		case sagittal:
			m_xSlider->SetValue((int)wxMin(wxMax(m_xSlider->GetValue() + delta*speedup, 0), m_xSlider->GetMax()));
			break;
		}
	}
	m_dh->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());
	updateStatusBar();
	refreshAllGLWidgets();
}

/****************************************************************************************************
 *
 * OnMouseEvent just repaints the Window
 *
 ****************************************************************************************************/
void MainFrame::OnMouseEvent(wxMouseEvent& event)
{
	//this->Refresh();
}

/****************************************************************************************************
 *
 *
 *
 ****************************************************************************************************/
void MainFrame::updateMenus()
{
	// get the options menu
	//#ifndef __WXMSW__

	// globals
	wxMenu* oMenu = GetMenuBar()->GetMenu(4);
	oMenu->Check(oMenu->FindItem(_T("Toggle Fiber Lighting")), m_dh->lighting);
	oMenu->Check(oMenu->FindItem(_T("Invert Fiber Selection")), m_dh->fibersInverted);
	oMenu->Check(oMenu->FindItem(_T("Use Tubes")), m_dh->useFakeTubes);
	oMenu->Check(oMenu->FindItem(_T("Use Transparent Fibers")), m_dh->useTransparency);
	oMenu->Check(oMenu->FindItem(_T("Show Color Map Legend")), m_dh->showColorMapLegend);
	wxMenu* sMenu = GetMenuBar()->GetMenu(3);
	sMenu->Check(sMenu->FindItem(_T("Blend Texture on Mesh")), m_dh->blendTexOnMesh);
	sMenu->Check(sMenu->FindItem(_T("Filter Dataset for IsoSurface")), m_dh->filterIsoSurf);

	GetToolBar()->ToggleTool(BUTTON_AXIAL, m_dh->showAxial);
	GetToolBar()->ToggleTool(BUTTON_CORONAL, m_dh->showCoronal);
	GetToolBar()->ToggleTool(BUTTON_SAGITTAL, m_dh->showSagittal);
	GetToolBar()->ToggleTool(BUTTON_TOGGLE_ALPHA, m_dh->blendAlpha);
	GetToolBar()->ToggleTool(MENU_OPTIONS_TOGGLE_LIGHTING, m_dh->lighting);
	GetToolBar()->ToggleTool(MENU_VOI_RENDER_SELBOXES, m_dh->showBoxes);
	// FIXME GetToolBar()->ToggleTool(MENU_OPTIONS_USE_FAKE_TUBES, m_dh->useFakeTubes);
	GetToolBar()->ToggleTool(MENU_SPLINESURF_DRAW_POINTS, m_dh->pointMode);

	wxMenu* voiMenu = GetMenuBar()->GetMenu(2);
	voiMenu->Check(voiMenu->FindItem(_T("active")), false);
	voiMenu->Check(voiMenu->FindItem(_T("visible")), false);
	voiMenu->Enable(voiMenu->FindItem(_T("active")), false);
	voiMenu->Enable(voiMenu->FindItem(_T("visible")), false);
	voiMenu->Check(voiMenu->FindItem(_T("morphing")), m_dh->morphing);	
	wxMenu* viewMenu = GetMenuBar()->GetMenu(1);
	viewMenu->Check(viewMenu->FindItem(_T("show crosshair")), m_dh->showCrosshair);


	wxTreeItemId treeid = m_treeWidget->GetSelection();
	int selected = treeSelected(treeid);

	if ( selected == ChildBox ||  selected == MasterBox )
	{
		voiMenu->Enable(voiMenu->FindItem(_T("active")), true);
		voiMenu->Enable(voiMenu->FindItem(_T("visible")), true);
		voiMenu->Check(voiMenu->FindItem(_T("active")), m_dh->lastSelectedBox->getActive());
		voiMenu->Check(voiMenu->FindItem(_T("visible")), m_dh->lastSelectedBox->getShow());

		GetToolBar()->ToggleTool(MENU_VOI_RENDER_SELBOXES, m_dh->showBoxes);
		GetToolBar()->ToggleTool(MENU_VOI_TOGGLE_SELBOX, !m_dh->lastSelectedBox->getActive());
	}
	//MENU_FILE_NEW_ISOSURF
	sMenu->Enable(sMenu->FindItem(_T("New Iso Surface")), false);
	GetToolBar()->EnableTool(MENU_FILE_NEW_ISOSURF, false);
	sMenu->Enable(sMenu->FindItem(_T("Toggle Texture Mode")), false);
	sMenu->Enable(sMenu->FindItem(_T("Toggle Lic")), false);
	sMenu->Enable(sMenu->FindItem(_T("Toggle Normal Direction")), false);
	//sMenu->Enable(sMenu->FindItem(_T("Draw Vectors")), false);
	sMenu->Enable(sMenu->FindItem(_T("Clean Artefacts from Surface")), false);
	sMenu->Enable(sMenu->FindItem(_T("Smooth Surface (Loop SubD)")), false);



	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		DatasetInfo *info = (DatasetInfo*) m_listCtrl->GetItemData(item);
		if (info->getType() < RGB)
		{
			sMenu->Enable(sMenu->FindItem(_T("New Iso Surface")), true);
			GetToolBar()->EnableTool(MENU_FILE_NEW_ISOSURF, true);
		}
		if (info->getType() < Mesh_)
		{
			sMenu->Enable(sMenu->FindItem(_T("Toggle Texture Mode")), true);
			sMenu->Check(sMenu->FindItem(_T("Toggle Texture Mode")), !info->getShowFS());
		}
		if (info->getType() == IsoSurface_ )
		{
			sMenu->Enable(sMenu->FindItem(_T("Toggle Lic")), true);
			sMenu->Enable(sMenu->FindItem(_T("Clean Artefacts from Surface")), true);
			sMenu->Enable(sMenu->FindItem(_T("Smooth Surface (Loop SubD)")), true);
			sMenu->Check(sMenu->FindItem(_T("Toggle Lic")), info->getUseLIC());
			sMenu->Check(sMenu->FindItem(_T("Draw Vectors")), m_dh->drawVectors);
		}
		if (info->getType() == Surface_)
		{
			sMenu->Enable(sMenu->FindItem(_T("Toggle Normal Direction")), true);
			sMenu->Enable(sMenu->FindItem(_T("Draw Vectors")), true);
			sMenu->Enable(sMenu->FindItem(_T("Toggle Lic")), true);
			sMenu->Check(sMenu->FindItem(_T("Toggle Lic")), info->getUseLIC());
			sMenu->Check(sMenu->FindItem(_T("Toggle Normal Direction")), (m_dh->normalDirection < 0));
			sMenu->Check(sMenu->FindItem(_T("Draw Vectors")), m_dh->drawVectors);
			sMenu->Enable(sMenu->FindItem(_T("Smooth Surface (Loop SubD)")), true);
		}
	}
	//#endif
}
