#include "mainFrame.h"

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/laywin.h"
#include "wx/filedlg.h"
#include "wx/statbmp.h"
#include "wx/colordlg.h"

#include "theScene.h"

#include "splinePoint.h"
#include "curves.h"
#include "surface.h"
#include "IsoSurface/CIsoSurface.h"

#include "icons/eyes.xpm"
#include "icons/delete.xpm"

DECLARE_EVENT_TYPE(wxEVT_TREE_EVENT, -1)
DECLARE_EVENT_TYPE(wxEVT_NAVGL_EVENT, -1)

BEGIN_EVENT_TABLE(MainFrame, wxMDIParentFrame)
    EVT_MENU(VIEWER_ABOUT, MainFrame::OnAbout)
    EVT_SIZE(MainFrame::OnSize)
    EVT_MENU(VIEWER_QUIT, MainFrame::OnQuit)
    EVT_MENU(VIEWER_NEW, MainFrame::OnNew)
    EVT_MENU(VIEWER_LOAD, MainFrame::OnLoad)
    EVT_MENU(VIEWER_SAVE, MainFrame::OnSave)
    EVT_MOUSE_EVENTS(MainFrame::OnMouseEvent)
    /* mouse click in one of the three navigation windows */
    EVT_COMMAND(ID_GL_NAV_X, wxEVT_NAVGL_EVENT, MainFrame::OnGLEvent)
    EVT_COMMAND(ID_GL_NAV_Y, wxEVT_NAVGL_EVENT, MainFrame::OnGLEvent)
	EVT_COMMAND(ID_GL_NAV_Z, wxEVT_NAVGL_EVENT, MainFrame::OnGLEvent)
	EVT_COMMAND(ID_GL_MAIN, wxEVT_NAVGL_EVENT, MainFrame::OnGLEvent)
	/* slize selection slider moved */
	EVT_SLIDER(ID_X_SLIDER, MainFrame::OnXSliderMoved)
	EVT_SLIDER(ID_Y_SLIDER, MainFrame::OnYSliderMoved)
	EVT_SLIDER(ID_Z_SLIDER, MainFrame::OnZSliderMoved)
	/* click on toolbar button to toggle one of the 3 panes in the
	 * main GL window */
	EVT_MENU(VIEWER_BUTTON_AXIAL, MainFrame::OnButtonAxial)
	EVT_MENU(VIEWER_BUTTON_CORONAL, MainFrame::OnButtonCoronal)
	EVT_MENU(VIEWER_BUTTON_SAGITTAL, MainFrame::OnButtonSagittal)
	EVT_MENU(VIEWER_TOGGLE_ALPHA, MainFrame::OnToggleAlpha)
	EVT_MENU(VIEWER_NEW_SELBOX, MainFrame::OnNewSelBox)
	EVT_MENU(VIEWER_RENDER_SELBOXES, MainFrame::OnHideSelBoxes)
	EVT_MENU(VIEWER_TOGGLE_SELBOX, MainFrame::OnToggleSelBox)
	/* click on reload shaders button */
	EVT_MENU(VIEWER_RELOAD_SHADER, MainFrame::OnReloadShaders)
	/* list ctrl events */
	EVT_LIST_ITEM_ACTIVATED(LIST_CTRL, MainFrame::OnActivateListItem)
	EVT_LIST_ITEM_SELECTED(LIST_CTRL, MainFrame::OnSelectListItem)
	EVT_BUTTON(ID_BUTTON_UP, MainFrame::OnListItemUp)
	EVT_BUTTON(ID_BUTTON_DOWN, MainFrame::OnListItemDown)
	EVT_SLIDER(ID_T_SLIDER, MainFrame::OnTSliderMoved)
	/* tree ctrl events */
	EVT_TREE_SEL_CHANGED(TREE_CTRL, MainFrame::OnSelectTreeItem)
	EVT_TREE_ITEM_ACTIVATED(TREE_CTRL, MainFrame::OnActivateTreeItem)
	EVT_COMMAND(TREE_CTRL, wxEVT_TREE_EVENT, MainFrame::OnTreeEvent)
	EVT_TREE_END_LABEL_EDIT(TREE_CTRL, MainFrame::OnTreeLabelEdit)
	/* toggle drawing of points */
	EVT_MENU(VIEWER_DRAW_POINTS, MainFrame::OnTogglePointMode)
	EVT_MENU(VIEWER_NEW_SURFACE, MainFrame::OnNewSurface)
	/* KDTREE thread finished */
	EVT_MENU(KDTREE_EVENT, MainFrame::OnKdTreeThreadFinished)
	/* Button Assign Color pressed */
	EVT_MENU(VIEWER_ASSIGN_COLOR, MainFrame::OnAssignColor)
	EVT_MENU(VIEWER_TOGGLE_LIGHTING, MainFrame::OnToggleLighting)
	EVT_MENU(VIEWER_INVERT_FIBERS, MainFrame::OnInvertFibers)
	EVT_MENU(VIEWER_NEW_ISOSURF, MainFrame::OnNewIsoSurface)
	EVT_MENU(VIEWER_MOVE_POINTS1, MainFrame::OnMovePoints1)
	EVT_MENU(VIEWER_MOVE_POINTS2, MainFrame::OnMovePoints2)
	EVT_MENU(VIEWER_TOGGLE_TEXTURE_FILTERING, MainFrame::OnToggleTextureFiltering)

END_EVENT_TABLE()

// Define my frame constructor
MainFrame::MainFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size,
    const long style):
  wxMDIParentFrame(parent, id, title, pos, size, style)
{
	NAV_SIZE = wxMin(255,size.y/4);
	NAV_GL_SIZE = NAV_SIZE - 4;

	// A window to the left of the client window
	wxSashLayoutWindow* win = new wxSashLayoutWindow(this, wxID_ANY,
	  		  wxDefaultPosition, wxSize(NAV_SIZE, NAV_SIZE*4),
	  		  wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    win->SetDefaultSize(wxSize(150 + NAV_SIZE, 1020));
    win->SetOrientation(wxLAYOUT_VERTICAL);
    win->SetAlignment(wxLAYOUT_LEFT);
    win->SetBackgroundColour(wxColour(0, 0, 0));
    m_leftWindowHolder = win;

    // Window to hold the tree widget and nav windows
    win = new wxSashLayoutWindow(m_leftWindowHolder, wxID_ANY,
      		  wxDefaultPosition, wxSize(NAV_SIZE, NAV_SIZE),
      		  wxRAISED_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    win->SetDefaultSize(wxSize(150 + NAV_SIZE, 3*NAV_SIZE + 60));
    win->SetOrientation(wxLAYOUT_HORIZONTAL);
    win->SetAlignment(wxLAYOUT_TOP);
    win->SetBackgroundColour(wxColour(0, 0, 0));
    m_leftWindowTop = win;

    // Window to hold the list ctrl widget
    win = new wxSashLayoutWindow(m_leftWindowHolder, wxID_ANY,
          		  wxDefaultPosition, wxSize(NAV_SIZE, NAV_SIZE),
          		  wxRAISED_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    win->SetDefaultSize(wxSize(150 + NAV_SIZE, NAV_SIZE));
    win->SetOrientation(wxLAYOUT_HORIZONTAL);
    win->SetAlignment(wxLAYOUT_TOP);
    win->SetBackgroundColour(wxColour(0, 0, 0));
    m_leftWindowBottom = win;

    win = new wxSashLayoutWindow(m_leftWindowBottom, wxID_ANY,
      		  wxDefaultPosition, wxSize(NAV_SIZE, NAV_SIZE),
      		  wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    win->SetDefaultSize(wxSize(150 + NAV_SIZE, NAV_SIZE));
    win->SetOrientation(wxLAYOUT_HORIZONTAL);
    win->SetAlignment(wxLAYOUT_TOP);
    win->SetBackgroundColour(wxColour(0, 0, 0));
    m_leftWindowBottom1 = win;

    win = new wxSashLayoutWindow(m_leftWindowBottom, wxID_ANY,
      		  wxDefaultPosition, wxSize(NAV_SIZE, NAV_SIZE),
      		  wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    win->SetDefaultSize(wxSize(150 + NAV_SIZE, NAV_SIZE));
    win->SetOrientation(wxLAYOUT_HORIZONTAL);
    win->SetAlignment(wxLAYOUT_TOP);
    win->SetBackgroundColour(wxColour(255, 255, 255));
    m_leftWindowBottom2 = win;

    wxButton *button = new wxButton(m_leftWindowBottom2, ID_BUTTON_UP, wxT("up"), wxPoint(0,2), wxSize(50,19));
    button->SetFont(wxFont(6, wxDEFAULT, wxNORMAL, wxNORMAL));
    button = new wxButton(m_leftWindowBottom2, ID_BUTTON_DOWN, wxT("down"), wxPoint(50,2), wxSize(50,19));
    button->SetFont(wxFont(6, wxDEFAULT, wxNORMAL, wxNORMAL));

    m_tSlider = new wxSlider(m_leftWindowBottom2, ID_T_SLIDER, 30, 0, 100,
        		wxPoint(100,2), wxSize(150, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS);


    win = new wxSashLayoutWindow(m_leftWindowTop, wxID_ANY,
                               wxDefaultPosition, wxSize(150, 30),
                               wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
	win->SetDefaultSize(wxSize(150, 3*NAV_SIZE));
	win->SetOrientation(wxLAYOUT_VERTICAL);
	win->SetAlignment(wxLAYOUT_LEFT);
	win->SetBackgroundColour(wxColour(255, 255, 255));
	win->SetSashVisible(wxSASH_RIGHT, false);
	m_leftWindow = win;

	// navigation window with three sub windows for gl widgets
    win = new wxSashLayoutWindow(m_leftWindowTop, wxID_ANY,
  		  wxDefaultPosition, wxSize(NAV_SIZE, NAV_SIZE*4),
  		  wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    win->SetDefaultSize(wxSize(NAV_SIZE, NAV_SIZE*4));
    win->SetOrientation(wxLAYOUT_VERTICAL);
    win->SetAlignment(wxLAYOUT_LEFT);
    win->SetBackgroundColour(wxColour(0, 0, 0));
    m_navWindow = win;

    // main window right side, holds the big gl widget
    win = new wxSashLayoutWindow(this, wxID_ANY,
  		  wxDefaultPosition, wxSize(200, 30),
  		  wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    win->SetDefaultSize(wxSize(765, 765));
    win->SetMinSize(wxSize(100,100));
    win->SetMaxSize(wxSize(2000,2000));
    win->SetOrientation(wxLAYOUT_VERTICAL);
    win->SetAlignment(wxLAYOUT_LEFT);
    win->SetBackgroundColour(wxColour(0, 0, 0));
    m_rightWindowHolder = win;

    // main window right side, holds the big gl widget
    win = new wxSashLayoutWindow(m_rightWindowHolder, wxID_ANY,
  		  wxDefaultPosition, wxSize(200, 30),
  		  wxRAISED_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    win->SetDefaultSize(wxSize(700, 700));
    win->SetMinSize(wxSize(100,100));
    win->SetOrientation(wxLAYOUT_HORIZONTAL);
    win->SetAlignment(wxLAYOUT_TOP);
    win->SetBackgroundColour(wxColour(180, 180, 180));
    m_rightWindow = win;

    // extra window right side, prevent scaling of the main gl widget
    win = new wxSashLayoutWindow(m_rightWindowHolder, wxID_ANY,
  		  wxDefaultPosition, wxSize(765, 765),
  		  wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    win->SetDefaultSize(wxSize(765, 1));
    win->SetOrientation(wxLAYOUT_HORIZONTAL);
    win->SetAlignment(wxLAYOUT_BOTTOM);
    win->SetBackgroundColour(wxColour(255, 255, 255));
    m_extraRightWindow = win;

    win = new wxSashLayoutWindow(m_navWindow, wxID_ANY,
  		  wxDefaultPosition, wxSize(NAV_SIZE, NAV_SIZE),
  		  wxRAISED_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    win->SetDefaultSize(wxSize(NAV_SIZE, NAV_SIZE));
    win->SetOrientation(wxLAYOUT_HORIZONTAL);
    win->SetAlignment(wxLAYOUT_TOP);
    win->SetBackgroundColour(wxColour(0, 0, 0));
    m_topNavWindow = win;

    win = new wxSashLayoutWindow(m_navWindow, wxID_ANY,
      		  wxDefaultPosition, wxSize(NAV_SIZE, 20),
      		  wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
        win->SetDefaultSize(wxSize(NAV_SIZE, 20));
        win->SetOrientation(wxLAYOUT_HORIZONTAL);
        win->SetAlignment(wxLAYOUT_TOP);
        win->SetBackgroundColour(wxColour(255, 255, 255));

	m_zSliderHolder = win;
    m_zSlider = new wxSlider(win, ID_Z_SLIDER, 50, 0, 100,
        		wxPoint(0, 0), wxSize(NAV_SIZE, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS);

    win = new wxSashLayoutWindow(m_navWindow, wxID_ANY,
  		  wxDefaultPosition, wxSize(NAV_SIZE, NAV_SIZE),
  		  wxRAISED_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    win->SetDefaultSize(wxSize(NAV_SIZE, NAV_SIZE));
    win->SetOrientation(wxLAYOUT_HORIZONTAL);
    win->SetAlignment(wxLAYOUT_TOP);
    win->SetBackgroundColour(wxColour(0, 0, 0));
    m_middleNavWindow = win;

    win = new wxSashLayoutWindow(m_navWindow, wxID_ANY,
      		  wxDefaultPosition, wxSize(NAV_SIZE, 20),
      		  wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
        win->SetDefaultSize(wxSize(NAV_SIZE, 20));
        win->SetOrientation(wxLAYOUT_HORIZONTAL);
        win->SetAlignment(wxLAYOUT_TOP);
        win->SetBackgroundColour(wxColour(255, 255, 255));
	m_ySliderHolder = win;
    m_ySlider = new wxSlider(win, ID_Y_SLIDER, 50, 0, 100, wxPoint(0,0),
        		wxSize(NAV_SIZE, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS);

    win = new wxSashLayoutWindow(m_navWindow, wxID_ANY,
     		  wxDefaultPosition, wxSize(NAV_SIZE, NAV_SIZE),
     		  wxRAISED_BORDER | wxSW_3D | wxCLIP_CHILDREN);
       win->SetDefaultSize(wxSize(NAV_SIZE, NAV_SIZE));
       win->SetOrientation(wxLAYOUT_HORIZONTAL);
       win->SetAlignment(wxLAYOUT_TOP);
       win->SetBackgroundColour(wxColour(0, 0, 0));
    m_bottomNavWindow = win;

    win = new wxSashLayoutWindow(m_navWindow, wxID_ANY,
      		  wxDefaultPosition, wxSize(NAV_SIZE, 20),
      		  wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
        win->SetDefaultSize(wxSize(NAV_SIZE, 20));
        win->SetOrientation(wxLAYOUT_HORIZONTAL);
        win->SetAlignment(wxLAYOUT_TOP);
        win->SetBackgroundColour(wxColour(255, 255, 255));

	m_xSliderHolder = win;
    m_xSlider = new wxSlider(win, ID_X_SLIDER, 50, 0, 100, wxPoint(0,0),
            		wxSize(NAV_SIZE, -1), wxSL_HORIZONTAL | wxSL_AUTOTICKS);

    // extra window to avoid scaling of the bottom gl widget when resizing
    win = new wxSashLayoutWindow(m_navWindow,wxID_ANY,
      		  wxDefaultPosition, wxSize(NAV_SIZE, NAV_SIZE),
       		  wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
          //win->SetDefaultSize(wxSize(NAV_SIZE, NAV_SIZE));
          win->SetOrientation(wxLAYOUT_HORIZONTAL);
          win->SetAlignment(wxLAYOUT_TOP);
          win->SetBackgroundColour(wxColour(255, 255, 255));
    m_extraNavWindow = win;

    m_xSlider->SetMinSize(wxSize(1, -1));
    m_ySlider->SetMinSize(wxSize(1, -1));
    m_zSlider->SetMinSize(wxSize(1, -1));

    m_listCtrl = new MyListCtrl(m_leftWindowBottom1, LIST_CTRL, wxDefaultPosition,
    		wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);

    wxImageList* imageList = new wxImageList(16,16);
    imageList->Add(wxIcon(eyes_xpm));
    imageList->Add(wxIcon(delete_xpm));
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

    m_treeWidget = new MyTreeCtrl(m_leftWindow, TREE_CTRL, wxPoint(0, 0),
    		wxDefaultSize, wxTR_HAS_BUTTONS|wxTR_SINGLE|wxTR_HIDE_ROOT|wxTR_HAS_BUTTONS|wxTR_EDIT_LABELS);
    wxImageList* tImageList = new wxImageList(16,16);
    tImageList->Add(wxIcon(eyes_xpm));
    tImageList->Add(wxIcon(delete_xpm));
    m_treeWidget->AssignImageList(tImageList);

    m_tRootId = m_treeWidget->AddRoot(wxT("Root"));
    m_tPlanesId = m_treeWidget->AppendItem(m_tRootId, wxT("planes"), -1, -1, new MyTreeItemData(0, Label_planes));
	    m_tAxialId    = m_treeWidget->AppendItem(m_tPlanesId, wxT("axial"));
	    m_tCoronalId  = m_treeWidget->AppendItem(m_tPlanesId, wxT("coronal"));
	    m_tSagittalId = m_treeWidget->AppendItem(m_tPlanesId, wxT("sagittal"));
    m_tDatasetId = m_treeWidget->AppendItem(m_tRootId, wxT("datasets"), -1, -1, new MyTreeItemData(0, Label_datasets));
    m_tMeshId = m_treeWidget->AppendItem(m_tRootId, wxT("meshes"), -1, -1, new MyTreeItemData(0, Label_meshes));
    m_tFiberId = m_treeWidget->AppendItem(m_tRootId, wxT("fibers"), -1, -1, new MyTreeItemData(0, Label_fibers));
    m_tPointId  = m_treeWidget->AppendItem(m_tRootId, wxT("points"), -1, -1, new MyTreeItemData(0, Label_points));
    m_tSelBoxId  = m_treeWidget->AppendItem(m_tRootId, wxT("selection boxes"), -1, -1, new MyTreeItemData(0, Label_selBoxes));

    /*
     * Set OpenGL attributes
     */
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
        printf("don't have double buffer, disabling\n");
	#ifdef __WXGTK__
        gl_attrib[9] = None;
	#endif
        doubleBuffer = GL_FALSE;
    }

    m_dh = new DatasetHelper(this);

    m_dh->scene = new TheScene(m_dh);

    m_mainGL = new MainCanvas(m_dh, mainView, m_rightWindow, ID_GL_MAIN, wxDefaultPosition,
        			wxDefaultSize, 0, _T("MainGLCanvas"), gl_attrib);
    m_gl0 = new MainCanvas(m_dh, axial, m_topNavWindow, ID_GL_NAV_X, wxDefaultPosition,
    	        wxDefaultSize, 0, _T("NavGLCanvasX"), gl_attrib);

    m_gl1 = new MainCanvas(m_dh, coronal, m_middleNavWindow, ID_GL_NAV_Y, wxDefaultPosition,
        	        wxDefaultSize, 0, _T("NavGLCanvasY"), gl_attrib);
    m_gl2 = new MainCanvas(m_dh, sagittal, m_bottomNavWindow, ID_GL_NAV_Z, wxDefaultPosition,
       	        wxDefaultSize, 0, _T("NavGLCanvasZ"), gl_attrib);

    m_dh->scene->setMainGLContext(new wxGLContext(m_mainGL));

}

MainFrame::~MainFrame()
{
#ifdef DEBUG
	printf("main frame destructor\n");
#endif
	delete m_dh;

}

void MainFrame::OnSize(wxSizeEvent& WXUNUSED(event))
{
	/* resize the navigation widgets */
	int height = this->GetClientSize().y;
	NAV_SIZE = wxMin(255, height/4);
	NAV_GL_SIZE = NAV_SIZE-4;

	m_leftWindowHolder->SetDefaultSize(wxSize(150 + NAV_SIZE, height));
	m_leftWindowTop->SetDefaultSize(wxSize(150 + NAV_SIZE, NAV_SIZE*3 + 65));
	m_leftWindowBottom->SetDefaultSize(wxSize(150 + NAV_SIZE, height - m_leftWindowTop->GetSize().y));
	m_leftWindowBottom1->SetDefaultSize(wxSize(150 + NAV_SIZE, m_leftWindowBottom->GetClientSize().y - 20));
	m_leftWindowBottom2->SetDefaultSize(wxSize(150 + NAV_SIZE, 20));
	m_navWindow->SetDefaultSize(wxSize(NAV_SIZE, height));
	m_topNavWindow->SetDefaultSize(wxSize(NAV_SIZE, NAV_SIZE));
	m_middleNavWindow->SetDefaultSize(wxSize(NAV_SIZE, NAV_SIZE));
	m_bottomNavWindow->SetDefaultSize(wxSize(NAV_SIZE, NAV_SIZE));
	m_extraNavWindow->SetDefaultSize(wxSize(NAV_SIZE, NAV_SIZE));
#ifdef __WXMSW__
	int posY = m_topNavWindow->GetSize().GetY();
	m_zSliderHolder->SetPosition(wxPoint(0, posY));
	posY += m_zSliderHolder->GetSize().GetY();
	m_middleNavWindow->SetPosition(wxPoint(0, posY));
	posY += m_middleNavWindow->GetSize().GetY();
	m_ySliderHolder->SetPosition(wxPoint(0, posY));
	posY += m_ySliderHolder->GetSize().GetY();
	m_bottomNavWindow->SetPosition(wxPoint(0, posY));
	posY += m_bottomNavWindow->GetSize().GetY();
	m_xSliderHolder->SetPosition(wxPoint(0, posY));
	m_gl0->SetSize(m_topNavWindow->GetClientSize());
	m_gl1->SetSize(m_middleNavWindow->GetClientSize());
	m_gl2->SetSize(m_bottomNavWindow->GetClientSize());
#endif
	/* resize sliders */
	m_xSlider->SetSize(wxSize(NAV_GL_SIZE, -1));
	m_ySlider->SetSize(wxSize(NAV_GL_SIZE, -1));
	m_zSlider->SetSize(wxSize(NAV_GL_SIZE, -1));

	/* resize list ctrl widget */
	m_listCtrl->SetSize(0,0, m_leftWindowBottom->GetClientSize().x, m_leftWindowBottom->GetClientSize().y);
	m_listCtrl->SetColumnWidth(0, 20);
	m_listCtrl->SetColumnWidth(1, m_leftWindowBottom->GetClientSize().x - 140);
	m_listCtrl->SetColumnWidth(2, 80);
	m_listCtrl->SetColumnWidth(3, 20);

	/* resize main gl window */
	int mainSize = wxMin((this->GetClientSize().x - m_leftWindow->GetSize().x - m_navWindow->GetSize().x),
			this->GetClientSize().y);
	m_rightWindowHolder->SetDefaultSize(wxSize(mainSize, mainSize));
	m_rightWindow->SetDefaultSize(wxSize(mainSize, mainSize));

	m_dh->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());

#if wxUSE_MDI_ARCHITECTURE
    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
#endif // wxUSE_MDI_ARCHITECTURE

    GetClientWindow()->Update();
    this->Update();
}

void MainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	wxMessageDialog dialog(NULL, wxT("Really Quit?"), wxT("Really Quit?"), wxNO_DEFAULT|wxYES_NO|wxICON_INFORMATION);
	if (dialog.ShowModal() == wxID_YES)
		Close(true);
}

void MainFrame::OnLoad(wxCommandEvent& WXUNUSED(event))
{
	if ( !m_dh->load(0) )
	{
		wxMessageBox(wxT("ERROR\n") + m_dh->lastError,  wxT(""), wxOK|wxICON_INFORMATION, NULL);
		m_statusBar->SetStatusText(wxT("ERROR"),1);
		m_statusBar->SetStatusText(m_dh->lastError,2);
		return;
	}
}

void MainFrame::OnKdTreeThreadFinished(wxCommandEvent& WXUNUSED(event))
{
	m_dh->treeFinished();
}

void MainFrame::OnSave(wxCommandEvent& WXUNUSED(event))
{
	wxString caption = wxT("Choose a file");
	wxString wildcard = wxT("Viewer files (*.yav)|*.yav||*.*|*.*");
	wxString defaultDir = wxEmptyString;
	wxString defaultFilename = wxEmptyString;
	wxFileDialog dialog(this, caption, defaultDir, defaultFilename, wildcard, wxSAVE);
	dialog.SetFilterIndex(0);
	dialog.SetDirectory(m_dh->lastPath);
	if (dialog.ShowModal() == wxID_OK)
	{
		m_dh->lastPath = dialog.GetDirectory();
		m_dh->save(dialog.GetPath());
	}
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
      (void)wxMessageBox(_T("Viewer\nAuthor: Ralph Schurade (c) 2008"), _T("About Viewer"));
}

void MainFrame::OnGLEvent( wxCommandEvent &event )
{
	wxPoint pos, newpos;
	float max = wxMax(m_dh->rows, wxMax(m_dh->columns, m_dh->frames));


	switch (event.GetInt())
	{
	case axial: {
		pos = m_gl0->getMousePos();
		float x = ((float)pos.x/NAV_GL_SIZE) * max;
		float y = ((float)pos.y/NAV_GL_SIZE) * max;

		m_xSlider->SetValue( x - (max - m_dh->columns)/2.0 );
		m_ySlider->SetValue( y - (max - m_dh->rows)/2.0);
		break;
	}
	case coronal: {
		pos = m_gl1->getMousePos();
		float x = ((float)pos.x/NAV_GL_SIZE) * max;
		float y = ((float)pos.y/NAV_GL_SIZE) * max;

		m_xSlider->SetValue( x - (max - m_dh->columns)/2.0 );
		m_zSlider->SetValue( y - (max - m_dh->frames)/2.0);
		break;
	}
	case sagittal: {
		pos = m_gl2->getMousePos();
		float x = ((float)pos.x/NAV_GL_SIZE) * max;
		float y = ((float)pos.y/NAV_GL_SIZE) * max;

		m_ySlider->SetValue( x - (max - m_dh->rows)/2.0 );
		m_zSlider->SetValue( y - (max - m_dh->frames)/2.0 );
		break;
	}
	case mainView:
		int delta = m_mainGL->getDelta();

		switch (m_mainGL->getPicked())
		{
		case axial:
			m_zSlider->SetValue(wxMin(wxMax(m_zSlider->GetValue() + delta, 0), m_zSlider->GetMax()));
			break;
		case coronal:
			m_ySlider->SetValue(wxMin(wxMax(m_ySlider->GetValue() + delta, 0), m_ySlider->GetMax()));
			break;
		case sagittal:
			m_xSlider->SetValue(wxMin(wxMax(m_xSlider->GetValue() + delta, 0), m_xSlider->GetMax()));
			break;
		case 20:
			SplinePoint *point = new SplinePoint(m_mainGL->getEventCenter(), m_dh);
			wxTreeItemId pId = m_treeWidget->AppendItem(m_tPointId, wxT("point"),-1, -1, new MyTreeItemData(point, SPoint));
			m_treeWidget->EnsureVisible(pId);
			m_treeWidget->SelectItem(pId);
			point->setTreeId(pId);
		}
	}
	m_dh->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());
	updateStatusBar();
	refreshAllGLWidgets();
}

void MainFrame::OnMouseEvent(wxMouseEvent& WXUNUSED(event))
{
	this->Refresh();
}



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

void MainFrame::OnTSliderMoved(wxCommandEvent& WXUNUSED(event))
{
	float threshold = (float)m_tSlider->GetValue()/100.0;

	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1) return;
	m_listCtrl->SetItem(item, 2, wxString::Format(wxT("%.2f"), threshold ));
	DatasetInfo *info = (DatasetInfo*) m_listCtrl->GetItemData(item);
	info->setThreshold(threshold);
	if (info->getType() == Surface_)
	{
		Surface* s = (Surface*) m_listCtrl->GetItemData(item);
		s->movePoints();
	}
	if (info->getType() == IsoSurface_)
	{
		CIsoSurface* s = (CIsoSurface*) m_listCtrl->GetItemData(item);
		s->GenerateWithThreshold();
	}
	refreshAllGLWidgets();
}

void MainFrame::refreshAllGLWidgets()
{
	if (m_gl0) m_gl0->render();
	if (m_gl1) m_gl1->render();
	if (m_gl2) m_gl2->render();
	if (m_mainGL) m_mainGL->render();
	updateStatusBar();
}

void MainFrame::renewAllGLWidgets()
{
	if (m_mainGL) m_mainGL->invalidate();
	if (m_gl0) m_gl0->invalidate();
	if (m_gl1) m_gl1->invalidate();
	if (m_gl2) m_gl2->invalidate();
	refreshAllGLWidgets();
}

void MainFrame::OnButtonAxial(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene) return;

	if ( wxGetKeyState(WXK_CONTROL) )
	{
		Matrix4fSetIdentity(&m_dh->m_transform);
		m_dh->m_transform.s.M11 = -1.0;
		m_dh->m_transform.s.M22 = -1.0;
		m_mainGL->setRotation();
	}
	else if ( wxGetKeyState(WXK_SHIFT) )
	{
		Matrix4fSetIdentity(&m_dh->m_transform);
		m_dh->m_transform.s.M11 = -1.0;
		m_dh->m_transform.s.M00 = -1.0;
		m_mainGL->setRotation();
	}
	else
		m_dh->showAxial = !m_dh->showAxial;

	m_mainGL->render();
}

void MainFrame::OnButtonCoronal(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene) return;
	if ( wxGetKeyState(WXK_CONTROL) )
	{
		Matrix4fSetIdentity(&m_dh->m_transform);
		m_dh->m_transform.s.M11 = 0.0;
		m_dh->m_transform.s.M12 = -1.0;
		m_dh->m_transform.s.M21 = 1.0;
		m_dh->m_transform.s.M22 = 0.0;
		m_mainGL->setRotation();
	}
	else if ( wxGetKeyState(WXK_SHIFT) )
	{
		Matrix4fSetIdentity(&m_dh->m_transform);
		m_dh->m_transform.s.M00 = -1.0;
		m_dh->m_transform.s.M11 = 0.0;
		m_dh->m_transform.s.M22 = 0.0;
		m_dh->m_transform.s.M12 = -1.0;
		m_dh->m_transform.s.M21 = -1.0;
		m_mainGL->setRotation();
	}
	else
		m_dh->showCoronal = !m_dh->showCoronal;

	m_mainGL->render();
}

void MainFrame::OnButtonSagittal(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene) return;

	if ( wxGetKeyState(WXK_CONTROL) )
	{
		Matrix4fSetIdentity(&m_dh->m_transform);
		m_dh->m_transform.s.M00 = 0.0;
		m_dh->m_transform.s.M11 = 0.0;
		m_dh->m_transform.s.M22 = 0.0;
		m_dh->m_transform.s.M20 = -1.0;
		m_dh->m_transform.s.M01 = 1.0;
		m_dh->m_transform.s.M12 = -1.0;
		m_mainGL->setRotation();
	}
	else if ( wxGetKeyState(WXK_SHIFT) )
	{
		Matrix4fSetIdentity(&m_dh->m_transform);
		m_dh->m_transform.s.M00 = 0.0;
		m_dh->m_transform.s.M11 = 0.0;
		m_dh->m_transform.s.M22 = 0.0;
		m_dh->m_transform.s.M20 = 1.0;
		m_dh->m_transform.s.M01 = -1.0;
		m_dh->m_transform.s.M12 = -1.0;
		m_mainGL->setRotation();
	}
	else
		m_dh->showSagittal = !m_dh->showSagittal;

	m_mainGL->render();
}

void MainFrame::OnToggleAlpha(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene) return;
	if ( wxGetKeyState(WXK_CONTROL) )
	{
		Matrix4fSetIdentity(&m_dh->m_transform);

		m_dh->m_transform.s.M00 = -0.67698019742965698242f;
		m_dh->m_transform.s.M10 =  0.48420974612236022949f;
		m_dh->m_transform.s.M20 = -0.55429106950759887695;
		m_dh->m_transform.s.M01 =  0.73480975627899169922f;
		m_dh->m_transform.s.M11 =  0.40184235572814941406f;
		m_dh->m_transform.s.M21 = -0.54642277956008911133f;
		m_dh->m_transform.s.M02 = -0.04184586182236671448f;
		m_dh->m_transform.s.M12 = -0.77721565961837768555f;
		m_dh->m_transform.s.M22 = -0.62784034013748168945f;
		m_mainGL->setRotation();
	}
	else
		m_dh->scene->m_blendAlpha = !m_dh->scene->m_blendAlpha;

	m_mainGL->render();
}

void MainFrame::OnToggleSelBox(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene || !m_dh->fibers_loaded) return;

	// check if selection box selected
	wxTreeItemId tBoxId = m_treeWidget->GetSelection();

	if (((MyTreeItemData*)m_treeWidget->GetItemData(tBoxId))->getType() == ChildBox ||
			((MyTreeItemData*)m_treeWidget->GetItemData(tBoxId))->getType() == MasterBox)
	{
		SelectionBox *box =  (SelectionBox*)((MyTreeItemData*)m_treeWidget->GetItemData(tBoxId))->getData();
		m_treeWidget->SetItemImage(tBoxId, 1-  !box->m_isActive);
		box->m_isActive = !box->m_isActive;
		box->setDirty();
	}
	m_dh->scene->m_selBoxChanged = true;
	refreshAllGLWidgets();
}

void MainFrame::OnNewSelBox(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene || !m_dh->fibers_loaded) return;

	Vector3fT vc = {{m_xSlider->GetValue()-m_dh->columns/2,
					m_ySlider->GetValue()-m_dh->rows/2,
					m_zSlider->GetValue()-m_dh->frames/2}};
	Vector3fT vs = {{m_dh->columns/8,m_dh->rows/8, m_dh->frames/8}};
	SelectionBox *selBox = new SelectionBox(vc, vs, m_dh);

	// check if selection box selected

	wxTreeItemId tBoxId = m_treeWidget->GetSelection();


	if (((MyTreeItemData*)m_treeWidget->GetItemData(tBoxId))->getType() == MasterBox)
	{
		// box is under another box
		selBox->m_isTop = false;

		wxTreeItemId tNewBoxId = m_treeWidget->AppendItem(tBoxId, wxT("box"),0, -1, new MyTreeItemData(selBox, ChildBox));
		m_treeWidget->EnsureVisible(tNewBoxId);
		selBox->setTreeId(tNewBoxId);
	}
	else
	{
		// box is top
		wxTreeItemId tNewBoxId = m_treeWidget->AppendItem(m_tSelBoxId, wxT("box"),0, -1, new MyTreeItemData(selBox, MasterBox));
		m_treeWidget->EnsureVisible(tNewBoxId);
		selBox->setTreeId(tNewBoxId);
	}

	m_dh->scene->m_selBoxChanged = true;
	refreshAllGLWidgets();
}

void MainFrame::OnHideSelBoxes(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene) return;
	m_dh->scene->toggleBoxes();
	refreshAllGLWidgets();
}

void MainFrame::OnReloadShaders(wxCommandEvent& WXUNUSED(event))
{
	renewAllGLWidgets();
}

void MainFrame::OnNew(wxCommandEvent& WXUNUSED(event))
{
	for (int i = 0 ; i < m_listCtrl->GetItemCount(); ++i)
	{
		DatasetInfo *info = (DatasetInfo*) m_listCtrl->GetItemData(i);
		m_treeWidget->Delete(info->getTreeId());
		delete info;
	}

	m_listCtrl->DeleteAllItems();
	m_mainGL->invalidate();

	m_dh->columns = 1;
    m_dh->rows = 1;
    m_dh->frames = 1;
    m_dh->anatomy_loaded = false;
    m_dh->fibers_loaded = false;
    m_dh->lastError = wxT("");

	delete m_dh->scene;
	m_dh->scene = new TheScene(m_dh);

	m_dh->scene->setMainGLContext(new wxGLContext(m_mainGL));

	refreshAllGLWidgets();
}

void MainFrame::updateStatusBar()
{
	wxString sbString0 = wxT("");
	sbString0 = wxString::Format(wxT("Axial: %d Coronal: %d Sagittal: %d"),m_zSlider->GetValue(), m_ySlider->GetValue(), m_xSlider->GetValue());
	m_statusBar->SetStatusText(sbString0,0);
}

void MainFrame::OnActivateListItem(wxListEvent& event)
{
	int item = event.GetIndex();
	DatasetInfo *info = (DatasetInfo*) m_listCtrl->GetItemData(item);
	int col = m_listCtrl->getColClicked();
	switch (col)
	{
	case 0:
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
	case 1:
		if (info->getType() >= Mesh_)
		{
			if (!info->toggleShowFS())
				m_listCtrl->SetItem(item, 1, info->getName() + wxT("*"));
			else
				m_listCtrl->SetItem(item, 1, info->getName());
		}
		break;
	case 2:
		if (info->getType() >= Mesh_)
		{
			if (!info->toggleUseTex())
				m_listCtrl->SetItem(item, 2, wxT("(") + wxString::Format(wxT("%.2f"), info->getThreshold()) + wxT(")") );
			else
				m_listCtrl->SetItem(item, 2, wxString::Format(wxT("%.2f"), info->getThreshold() ));
		}
		break;
	case 3:
		if (info->hasTreeId()) m_treeWidget->Delete(info->getTreeId());
		delete info;
		m_listCtrl->DeleteItem(item);
		break;
	default:
		break;
	}
	refreshAllGLWidgets();
}

void MainFrame::OnSelectListItem(wxListEvent& event)
{
	int item = event.GetIndex();
	if (item == -1) return;
	DatasetInfo *info = (DatasetInfo*) m_listCtrl->GetItemData(item);
	m_tSlider->SetValue((int)(info->getThreshold()*100));
	m_treeWidget->SelectItem(info->getTreeId());
	m_treeWidget->EnsureVisible(info->getTreeId());
}

void MainFrame::OnListItemUp(wxCommandEvent& WXUNUSED(event))
{
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1) return;
	m_listCtrl->moveItemUp(item);
	m_listCtrl->EnsureVisible(item);
	refreshAllGLWidgets();
}

void MainFrame::OnListItemDown(wxCommandEvent& WXUNUSED(event))
{
	long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1) return;
	m_listCtrl->moveItemDown(item);
	m_listCtrl->EnsureVisible(item);
	refreshAllGLWidgets();
}

void MainFrame::OnSelectTreeItem(wxTreeEvent& WXUNUSED(event))
{
	wxTreeItemId treeid = m_treeWidget->GetSelection();
	MyTreeItemData *data = (MyTreeItemData*)m_treeWidget->GetItemData(treeid);
	if (!data) return;
	for (int i = 0 ; i < m_listCtrl->GetItemCount(); ++i)
	{
		DatasetInfo *info = (DatasetInfo*) m_listCtrl->GetItemData(i);
		if (info->getTreeId() == treeid)
		{
			m_listCtrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		}
	}

	if (((MyTreeItemData*)m_treeWidget->GetItemData(treeid))->getType() == SPoint)
	{
		if (m_dh->lastSelectedPoint) m_dh->lastSelectedPoint->unselect();
		m_dh->lastSelectedPoint = (SplinePoint*)((MyTreeItemData*)m_treeWidget->GetItemData(treeid))->getData();
		m_dh->lastSelectedPoint->select();
	}

	if (((MyTreeItemData*)m_treeWidget->GetItemData(treeid))->getType() == ChildBox ||
			((MyTreeItemData*)m_treeWidget->GetItemData(treeid))->getType() == MasterBox )
	{
		if (m_dh->lastSelectedBox) m_dh->lastSelectedBox->unselect();
		m_dh->lastSelectedBox = (SelectionBox*)((MyTreeItemData*)m_treeWidget->GetItemData(treeid))->getData();
		m_dh->lastSelectedBox->select();
	}
	refreshAllGLWidgets();
}

void MainFrame::OnActivateTreeItem(wxTreeEvent& WXUNUSED(event))
{
	wxTreeItemId treeid = m_treeWidget->GetSelection();
	/* open load dialog */
	if (((MyTreeItemData*)m_treeWidget->GetItemData(treeid))->getType() == Label_datasets)
	{
		m_dh->load(1);
	}
	else if (((MyTreeItemData*)m_treeWidget->GetItemData(treeid))->getType() == Label_meshes)
	{
		m_dh->load(2);
	}
	else if (((MyTreeItemData*)m_treeWidget->GetItemData(treeid))->getType() == Label_fibers)
	{
		m_dh->load(3);
	}
	wxTreeItemId parentid = m_treeWidget->GetItemParent(treeid);
	MyTreeItemData *data = (MyTreeItemData*)m_treeWidget->GetItemData(treeid);
	if (!data) return;
	if (((MyTreeItemData*)m_treeWidget->GetItemData(treeid))->getType() == ChildBox)
	{
		((SelectionBox*) (((MyTreeItemData*)m_treeWidget->GetItemData(treeid))->getData()))->toggleNOT();
		((SelectionBox*) (((MyTreeItemData*)m_treeWidget->GetItemData(parentid))->getData()))->setDirty();
	}
	m_dh->scene->m_selBoxChanged = true;
	refreshAllGLWidgets();
}

void MainFrame::OnTreeEvent(wxCommandEvent& WXUNUSED(event))
{
	m_dh->scene->m_selBoxChanged = true;
	refreshAllGLWidgets();
}

void MainFrame::OnTreeLabelEdit(wxTreeEvent& event)
{
	wxTreeItemId treeid = event.GetItem();
	if (((MyTreeItemData*)m_treeWidget->GetItemData(treeid))->getType() == ChildBox ||
				((MyTreeItemData*)m_treeWidget->GetItemData(treeid))->getType() == MasterBox)
	{
		((SelectionBox*) (((MyTreeItemData*)m_treeWidget->GetItemData(treeid))->getData()))->setName(event.GetLabel());
	}
}


void MainFrame::OnTogglePointMode(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene) return;
	m_dh->scene->togglePointMode();
	refreshAllGLWidgets();
}

void MainFrame::OnNewSurface(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene || m_dh->surface_loaded) return;

	if (!m_dh->fibers_loaded)
	{
		refreshAllGLWidgets();
		return;
	}

	int y = m_dh->rows/2;
	int z = m_dh->frames/2;
	int xs = m_xSlider->GetValue() - m_dh->columns/2;

	//delete all existing points
	m_treeWidget->DeleteChildren(m_tPointId);
	Curves *fibers;
	wxTreeItemIdValue cookie = 0;
	fibers = (Curves*)((MyTreeItemData*)
						m_treeWidget->GetItemData(m_treeWidget->GetFirstChild(m_tFiberId,cookie)))->getData();

	for ( int i = 0 ; i < 11 ; ++i)
		for ( int j = 0 ; j < 11 ; ++j )
		{

			int yy = (m_dh->rows/10)*i;
			int zz = (m_dh->frames/10)*j;

			// create the point
			SplinePoint *point = new SplinePoint(xs, yy-y, zz-z, m_dh);

			if (i == 0 || i == 10 || j == 0 || j == 10) {
				wxTreeItemId tId = m_treeWidget->AppendItem(m_tPointId, wxT("border point"),-1, -1, new MyTreeItemData(point, BPoint));
				m_treeWidget->EnsureVisible(tId);
				point->setTreeId(tId);
			}
			else
			{
				if (fibers->getBarycenter(point)) {
					wxTreeItemId tId = m_treeWidget->AppendItem(m_tPointId, wxT("point"),-1, -1, new MyTreeItemData(point, SPoint));
					m_treeWidget->EnsureVisible(tId);
					point->setTreeId(tId);
				}
			}
		}

	Surface *surface = new Surface(m_dh);

	int i = m_listCtrl->GetItemCount();
	m_listCtrl->InsertItem(i, wxT(""), 0);
	m_listCtrl->SetItem(i, 1, surface->getName());
	m_listCtrl->SetItem(i, 2, wxT("0.50"));
	m_listCtrl->SetItem(i, 3, wxT(""), 1);
	m_listCtrl->SetItemData(i, (long)surface);
	m_listCtrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

	refreshAllGLWidgets();
}

void MainFrame::OnAssignColor(wxCommandEvent& WXUNUSED(event))
{
	if (!m_dh->scene) return;

	wxTreeItemId tBoxId = m_treeWidget->GetSelection();

	wxColourData colorData;

	for ( int i = 0; i < 16 ; ++i)
	{
		wxColour color(i*16, i*16, i*16);
		colorData.SetCustomColour(i, color);
	}
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
		if (info->getType() == Mesh_ || info->getType() == IsoSurface_)
		{
			info->setColor(col);
			m_dh->scene->m_selBoxChanged = true;
			refreshAllGLWidgets();
			return;
		}
	}
	if (((MyTreeItemData*)m_treeWidget->GetItemData(tBoxId))->getType() == MasterBox)
	{
		SelectionBox *box = (SelectionBox*)((MyTreeItemData*)m_treeWidget->GetItemData(tBoxId))->getData();
		box->setColor(col);
		box->setDirty();
	}
	else
	{
		if (((MyTreeItemData*)m_treeWidget->GetItemData(tBoxId))->getType() == ChildBox)
		{
			SelectionBox *box = (SelectionBox*)((MyTreeItemData*)m_treeWidget->GetItemData(m_treeWidget->GetItemParent(tBoxId)))->getData();
			box->setColor(col);
			box->setDirty();
		}
	}
	m_dh->scene->m_selBoxChanged = true;
	refreshAllGLWidgets();
}

void MainFrame::OnToggleLighting(wxCommandEvent& WXUNUSED(event))
{
	m_dh->lighting = !m_dh->lighting;
	refreshAllGLWidgets();
}

void MainFrame::OnInvertFibers(wxCommandEvent& WXUNUSED(event))
{
	m_dh->invertFibers();
	m_dh->scene->m_selBoxChanged = true;
	refreshAllGLWidgets();
}

void MainFrame::OnNewIsoSurface(wxCommandEvent& WXUNUSED(event))
{
	m_dh->createIsoSurface();
	refreshAllGLWidgets();
}

/*
 * Moves all boundary points in one direction
 */
void MainFrame::OnMovePoints1(wxCommandEvent& event)
{
	int countPoints = m_treeWidget->GetChildrenCount(m_tPointId, true);

	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	for (int i = 0 ; i < countPoints ; ++i)
	{
		id = m_treeWidget->GetNextChild(m_tPointId, cookie);
		MyTreeItemData *data = (MyTreeItemData*)m_treeWidget->GetItemData(id);
		if (data->getType() == BPoint)
		{
			SplinePoint *point = (SplinePoint*)((MyTreeItemData*)m_treeWidget->GetItemData(id))->getData();
			point->setX(point->X() + 5.0);
		}
	}
	m_dh->surface_isDirty = true;
	refreshAllGLWidgets();
}

/*
 * Moves all boundary points in one direction
 */
void MainFrame::OnMovePoints2(wxCommandEvent& event)
{
	int countPoints = m_treeWidget->GetChildrenCount(m_tPointId, true);

	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	for (int i = 0 ; i < countPoints ; ++i)
	{
		id = m_treeWidget->GetNextChild(m_tPointId, cookie);
		MyTreeItemData *data = (MyTreeItemData*)m_treeWidget->GetItemData(id);
		if (data->getType() == BPoint)
		{
			SplinePoint *point = (SplinePoint*)((MyTreeItemData*)m_treeWidget->GetItemData(id))->getData();
			point->setX(point->X() - 5.0);
		}
	}
	m_dh->surface_isDirty = true;
	refreshAllGLWidgets();
}

void MainFrame::OnToggleTextureFiltering(wxCommandEvent& event)
{
	m_dh->useLinearFiltering = !m_dh->useLinearFiltering;
	refreshAllGLWidgets();
}
