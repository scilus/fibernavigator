#include "mainFrame.h"

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/laywin.h"
#include "wx/filedlg.h"
#include "wx/statbmp.h"

#include "icons/eyes.xpm"
#include "icons/delete.xpm"

DECLARE_EVENT_TYPE(wxEVT_NAVGL_EVENT, -1)
   
BEGIN_EVENT_TABLE(MainFrame, wxMDIParentFrame)
    EVT_MENU(VIEWER_ABOUT, MainFrame::OnAbout)
    EVT_SIZE(MainFrame::OnSize)
    EVT_MENU(VIEWER_QUIT, MainFrame::OnQuit)
    EVT_MENU(VIEWER_NEW, MainFrame::OnNew)
    EVT_MENU(VIEWER_LOAD, MainFrame::OnLoad)
    EVT_MOUSE_EVENTS(MainFrame::OnMouseEvent)
    /* mouse click in one of the three navigation windows */
    EVT_COMMAND(ID_GL_NAV_X, wxEVT_NAVGL_EVENT, MainFrame::OnGLEvent)
    EVT_COMMAND(ID_GL_NAV_Y, wxEVT_NAVGL_EVENT, MainFrame::OnGLEvent)
	EVT_COMMAND(ID_GL_NAV_Z, wxEVT_NAVGL_EVENT, MainFrame::OnGLEvent)
	/* slize selection slider moved */
	EVT_SLIDER(ID_X_SLIDER, MainFrame::OnXSliderMoved)
	EVT_SLIDER(ID_Y_SLIDER, MainFrame::OnYSliderMoved)
	EVT_SLIDER(ID_Z_SLIDER, MainFrame::OnZSliderMoved)
	EVT_SLIDER(ID_T_SLIDER, MainFrame::OnTSliderMoved)
	/* click on toolbar button to toggle one of the 3 panes in the
	 * main GL window */ 
	EVT_MENU(VIEWER_TOGGLEVIEW1, MainFrame::OnToggleView1)
	EVT_MENU(VIEWER_TOGGLEVIEW2, MainFrame::OnToggleView2)
	EVT_MENU(VIEWER_TOGGLEVIEW3, MainFrame::OnToggleView3)
	/* click on reload shaders button */
	EVT_MENU(VIEWER_RELOAD_SHADER, MainFrame::OnReloadShaders)
	/* listctrl events */
	EVT_LIST_ITEM_ACTIVATED(LIST_CTRL, MainFrame::OnActivateListItem)
	EVT_LIST_ITEM_SELECTED(LIST_CTRL, MainFrame::OnSelectListItem)
	EVT_BUTTON(ID_BUTTON_UP, MainFrame::OnListItemUp)
	EVT_BUTTON(ID_BUTTON_DOWN, MainFrame::OnListItemDown)
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
	
    win = new wxSashLayoutWindow(m_leftWindowHolder, wxID_ANY, 
      		  wxDefaultPosition, wxSize(NAV_SIZE, NAV_SIZE),
      		  wxRAISED_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    win->SetDefaultSize(wxSize(150 + NAV_SIZE, 3*NAV_SIZE + 60));
    win->SetOrientation(wxLAYOUT_HORIZONTAL);
    win->SetAlignment(wxLAYOUT_TOP);
    win->SetBackgroundColour(wxColour(0, 0, 0));
    m_leftWindowTop = win; 
    
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
    win->SetBackgroundColour(wxColour(255, 0, 0));
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
	win->SetBackgroundColour(wxColour(0, 255, 0));
	win->SetSashVisible(wxSASH_RIGHT, false);
	
	m_textWindow = new wxTextCtrl(win, wxID_ANY, wxEmptyString, 
			  wxDefaultPosition, wxDefaultSize,
			  wxTE_MULTILINE|wxSUNKEN_BORDER);
	m_textWindow->SetValue(_T("Info"));
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
    
    m_datasetListCtrl = new MyListCtrl(m_leftWindowBottom1, LIST_CTRL, wxDefaultPosition, 
    		wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    
    wxImageList* imageList = new wxImageList(16,16);
    imageList->Add(wxIcon(eyes_xpm));
    imageList->Add(wxIcon(delete_xpm));
    m_datasetListCtrl->AssignImageList(imageList, wxIMAGE_LIST_SMALL);
    
    wxListItem itemCol;
    itemCol.SetText(wxT(""));
    m_datasetListCtrl->InsertColumn(0, itemCol);
        
    itemCol.SetText(wxT("Name"));
    itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
    m_datasetListCtrl->InsertColumn(1, itemCol);
    
    itemCol.SetText(wxT("Threshold"));
    itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
    m_datasetListCtrl->InsertColumn(2, itemCol);
    
    itemCol.SetText(wxT(""));
    m_datasetListCtrl->InsertColumn(3, itemCol);
        
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
    
    m_scene = new TheScene();
    m_dataset = new TheDataset();
    m_scene->setDataset(m_dataset);
    m_scene->setDataListCtrl(m_datasetListCtrl);
    
    m_mainGL = new MainCanvas(m_scene, mainView, m_rightWindow, ID_GL_MAIN, wxDefaultPosition,
        			wxDefaultSize, 0, _T("MainGLCanvas"), gl_attrib);
    m_gl0 = new MainCanvas(m_scene, axial, m_topNavWindow, ID_GL_NAV_X, wxDefaultPosition,
    	        wxDefaultSize, 0, _T("NavGLCanvasX"), gl_attrib);
    
    m_gl1 = new MainCanvas(m_scene, coronal, m_middleNavWindow, ID_GL_NAV_Y, wxDefaultPosition,
        	        wxDefaultSize, 0, _T("NavGLCanvasY"), gl_attrib);
    m_gl2 = new MainCanvas(m_scene, sagittal, m_bottomNavWindow, ID_GL_NAV_Z, wxDefaultPosition,
       	        wxDefaultSize, 0, _T("NavGLCanvasZ"), gl_attrib);
    
    m_scene->setMainGLContext(new wxGLContext(m_mainGL));
}

void MainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void MainFrame::OnLoad(wxCommandEvent& WXUNUSED(event))
{
	if (m_datasetListCtrl->GetItemCount() > 9)
	{
		wxMessageBox(wxT("ERROR\nCan't load any more files.\nDelete some first.\n"),  wxT(""), wxOK|wxICON_INFORMATION, NULL);
		m_statusBar->SetStatusText(wxT("ERROR"),1);
		m_statusBar->SetStatusText(m_dataset->m_lastError,2);
		return;
	}
	wxString caption = wxT("Choose a file");
	wxString wildcard = wxT("Header files (*.hea)|*.hea|Mesh files (*.mesh)|*.mesh|*.*|*.*");
	wxString defaultDir = wxEmptyString;
	wxString defaultFilename = wxEmptyString;
	wxFileDialog dialog(this, caption, defaultDir, defaultFilename, wildcard, wxOPEN);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxString path = dialog.GetPath();
		DatasetInfo *info = m_dataset->load(path); 
		if ( info == NULL) 
		{
			wxMessageBox(wxT("ERROR\n") + m_dataset->m_lastError,  wxT(""), wxOK|wxICON_INFORMATION, NULL);
			m_statusBar->SetStatusText(wxT("ERROR"),1);
			m_statusBar->SetStatusText(m_dataset->m_lastError,2);
			return;
		}
		 
		int i = m_datasetListCtrl->GetItemCount();
		m_datasetListCtrl->InsertItem(i, wxT(""), 0);
		m_datasetListCtrl->SetItem(i, 1, dialog.GetFilename());
		m_datasetListCtrl->SetItem(i, 2, wxT("0.10"));
		m_datasetListCtrl->SetItem(i, 3, wxT(""), 1);
		m_datasetListCtrl->SetItemData(i, (long)info);
		m_datasetListCtrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		
		m_statusBar->SetStatusText(wxT("Ready"),1);
		m_statusBar->SetStatusText(dialog.GetFilename() + wxT(" loaded"),2);
		
		updateInfoString();
		
		m_xSlider->SetMax(wxMax(2,m_dataset->m_columns-1));
		m_xSlider->SetValue(m_dataset->m_columns/2);
		m_ySlider->SetMax(wxMax(2,m_dataset->m_rows-1));
		m_ySlider->SetValue( m_dataset->m_rows/2);
		m_zSlider->SetMax(wxMax(2,m_dataset->m_frames-1));
		m_zSlider->SetValue( m_dataset->m_frames/2);
		m_tSlider->SetValue(10);
		
		m_scene->setDataset(m_dataset);
		m_scene->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());
		
		if (m_datasetListCtrl->GetItemCount() == 1)
		{
			m_scene->assignTextures();
			renewAllGLWidgets();
		}
		else
		{
			m_scene->addTexture();
			refreshAllGLWidgets();
		}
		
	}
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
      (void)wxMessageBox(_T("Viewer\nAuthor: Ralph Schurade (c) 2008"), _T("About Viewer"));
}

void MainFrame::OnGLEvent( wxCommandEvent &event )
{
	wxPoint pos, newpos;
	
	switch (event.GetInt())
	{
	case axial:
		pos = m_gl0->getMousePos();
		m_xSlider->SetValue((int)(((float)pos.x/NAV_GL_SIZE)*m_dataset->m_columns));
		m_ySlider->SetValue((int)(((float)pos.y/NAV_GL_SIZE)*m_dataset->m_rows));
		break;
	case coronal:
		pos = m_gl1->getMousePos();
		m_xSlider->SetValue((int)(((float)pos.x/NAV_GL_SIZE)*m_dataset->m_columns));
		m_zSlider->SetValue((int)(((float)pos.y/NAV_GL_SIZE)*m_dataset->m_frames));
		break;
	case sagittal:
		pos = m_gl2->getMousePos();
		m_ySlider->SetValue((int)(((float)pos.x/NAV_GL_SIZE)*m_dataset->m_rows));
		m_zSlider->SetValue((int)(((float)pos.y/NAV_GL_SIZE)*m_dataset->m_frames));
		break;
	case mainView:
		printf("main gl mouse event\n");
	}
	m_scene->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());
	updateStatusBar();
	refreshAllGLWidgets();
}

void MainFrame::OnMouseEvent(wxMouseEvent& event)
{	
	this->Refresh();
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
	
	/* resize sliders */
	m_xSlider->SetSize(wxSize(NAV_GL_SIZE, -1));
	m_ySlider->SetSize(wxSize(NAV_GL_SIZE, -1));
	m_zSlider->SetSize(wxSize(NAV_GL_SIZE, -1));

	/* resize list ctrl widget */
	m_datasetListCtrl->SetSize(0,0, m_leftWindowBottom->GetClientSize().x, m_leftWindowBottom->GetClientSize().y);
	m_datasetListCtrl->SetColumnWidth(0, 20);
	m_datasetListCtrl->SetColumnWidth(1, m_leftWindowBottom->GetClientSize().x - 140);
	m_datasetListCtrl->SetColumnWidth(2, 80);
	m_datasetListCtrl->SetColumnWidth(3, 20);
	
	/* resize main gl window */
	int mainSize = wxMin((this->GetClientSize().x - m_leftWindow->GetSize().x - m_navWindow->GetSize().x), 
			this->GetClientSize().y);
	m_rightWindowHolder->SetDefaultSize(wxSize(mainSize, mainSize));
	m_rightWindow->SetDefaultSize(wxSize(mainSize, mainSize));
	
	m_scene->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());

#if wxUSE_MDI_ARCHITECTURE
    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
#endif // wxUSE_MDI_ARCHITECTURE

    GetClientWindow()->Update();
    this->Update();
}

void MainFrame::OnXSliderMoved(wxCommandEvent& event)
{
	if (!m_dataset) return;
	 m_scene->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());
	 refreshAllGLWidgets();
}

void MainFrame::OnYSliderMoved(wxCommandEvent& event)
{
	if (!m_dataset) return;
	m_scene->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());
	refreshAllGLWidgets();
}

void MainFrame::OnZSliderMoved(wxCommandEvent& event)
{
	if (!m_dataset) return;
	m_scene->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());
	refreshAllGLWidgets();
}

void MainFrame::OnTSliderMoved(wxCommandEvent& event)
{
	if (!m_dataset) return;
	float threshold = (float)m_tSlider->GetValue()/100.0;
		
	long item = m_datasetListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1) return;
	m_datasetListCtrl->SetItem(item, 2, wxString::Format(wxT("%.2f"), threshold ));
	DatasetInfo *info = (DatasetInfo*) m_datasetListCtrl->GetItemData(item);
	info->setThreshold(threshold);
	refreshAllGLWidgets();
}

void MainFrame::refreshAllGLWidgets()
{
	m_gl0->render();
	m_gl1->render();
	m_gl2->render();
	m_mainGL->render();
	updateStatusBar();
}

void MainFrame::renewAllGLWidgets()
{
	m_mainGL->invalidate();
	m_gl0->invalidate();
	m_gl1->invalidate();
	m_gl2->invalidate();
	refreshAllGLWidgets();
	updateInfoString();
}

void MainFrame::updateInfoString()
{
	m_textWindow->SetValue( wxT("") );
	if (m_datasetListCtrl->GetItemCount() == 0) 
	{
		m_textWindow->SetValue( wxT("Nothing loaded") );
		return;
	}
		
	wxString newString = wxT("");
	
	for (int i = 0 ; i < m_datasetListCtrl->GetItemCount() ; ++i)
	{
		DatasetInfo* info = (DatasetInfo*)m_datasetListCtrl->GetItemData(i);
		newString += info->getInfoString();
		newString += wxT("\n\n");
	}
	
	m_textWindow->SetValue( newString );

}

void MainFrame::OnToggleView1(wxCommandEvent& event)
{
	if (!m_scene) return;
	m_scene->m_showZSlize = !m_scene->m_showZSlize;
	m_mainGL->render();
}

void MainFrame::OnToggleView2(wxCommandEvent& event)
{
	if (!m_scene) return;
	m_scene->m_showYSlize = !m_scene->m_showYSlize;
	m_mainGL->render();
}

void MainFrame::OnToggleView3(wxCommandEvent& event)
{
	if (!m_scene) return;
	m_scene->m_showXSlize = !m_scene->m_showXSlize;
	m_mainGL->render();
}

void MainFrame::loadStandard()
{
	DatasetInfo *info;
	
	info = m_dataset->load(wxT("/home/ralph/bin/devel/workspace/diplom/data/t1_1mm.hea"));
	int i = m_datasetListCtrl->GetItemCount();
	m_datasetListCtrl->InsertItem(i, wxT(""), 0);
	m_datasetListCtrl->SetItem(i, 1, wxT("t1_1mm.hea"));
	m_datasetListCtrl->SetItem(i, 2, wxT("0.10"));
	m_datasetListCtrl->SetItem(i, 3, wxT(""), 1);
	m_datasetListCtrl->SetItemData(i, (long)info);
	m_datasetListCtrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	
	info = m_dataset->load(wxT("/home/ralph/bin/devel/workspace/diplom/data/overlay_swap.hea"));
	i = m_datasetListCtrl->GetItemCount();
	m_datasetListCtrl->InsertItem(i, wxT(""), 0);
	m_datasetListCtrl->SetItem(i, 1, wxT("overlay_swap.hea"));
	m_datasetListCtrl->SetItem(i, 2, wxT("0.10"));
	m_datasetListCtrl->SetItem(i, 3, wxT(""), 1);
	m_datasetListCtrl->SetItemData(i, (long)info);
	m_datasetListCtrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	
	info = m_dataset->load(wxT("/home/ralph/bin/devel/workspace/diplom/data/rgb.hea"));
	i = m_datasetListCtrl->GetItemCount();
	m_datasetListCtrl->InsertItem(i, wxT(""), 0);
	m_datasetListCtrl->SetItem(i, 1, wxT("rgb.hea"));
	m_datasetListCtrl->SetItem(i, 2, wxT("0.10"));
	m_datasetListCtrl->SetItem(i, 3, wxT(""), 1);
	m_datasetListCtrl->SetItemData(i, (long)info);
	m_datasetListCtrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	info = m_dataset->load(wxT("/home/ralph/bin/devel/workspace/diplom/data/s1_Rwhite.mesh"));
	i = m_datasetListCtrl->GetItemCount();
	m_datasetListCtrl->InsertItem(i, wxT(""), 0);
	m_datasetListCtrl->SetItem(i, 1, wxT("s1_Rwhite.mesh"));
	m_datasetListCtrl->SetItem(i, 2, wxT("0.10"));
	m_datasetListCtrl->SetItem(i, 3, wxT(""), 1);
	m_datasetListCtrl->SetItemData(i, (long)info);
	m_datasetListCtrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	
	m_scene->setDataset(m_dataset);
	
	m_xSlider->SetMax(wxMax(2,m_dataset->m_columns-1));
	m_xSlider->SetValue(m_dataset->m_columns/2);
	m_ySlider->SetMax(wxMax(2,m_dataset->m_rows-1));
	m_ySlider->SetValue( m_dataset->m_rows/2);
	m_zSlider->SetMax(wxMax(2,m_dataset->m_frames-1));
	m_zSlider->SetValue( m_dataset->m_frames/2);
		
	m_scene->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());
	updateInfoString();
	renewAllGLWidgets();
	
}

void MainFrame::OnReloadShaders(wxCommandEvent& event)
{
	renewAllGLWidgets();
}

void MainFrame::OnNew(wxCommandEvent& event)
{
	m_datasetListCtrl->DeleteAllItems();
	m_mainGL->invalidate();
	delete m_dataset;
	m_dataset = new TheDataset();
	delete m_scene;
	m_scene = new TheScene();
	m_scene->setDataset(m_dataset);
	m_scene->setDataListCtrl(m_datasetListCtrl);
	m_scene->setMainGLContext(new wxGLContext(m_mainGL));
	
	m_mainGL->setScene(m_scene);
	m_gl0->setScene(m_scene);
	m_gl1->setScene(m_scene);
	m_gl2->setScene(m_scene);
	updateInfoString();
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
	DatasetInfo *info = (DatasetInfo*) m_datasetListCtrl->GetItemData(item);
	int col = m_datasetListCtrl->getColClicked();
	switch (col)
	{
	case 0:
		if (info->toggleShow())
		{
			m_datasetListCtrl->SetItem(item, 0, wxT(""), 0);
		}
		else
		{
			m_datasetListCtrl->SetItem(item, 0, wxT(""), 1);
		}
		refreshAllGLWidgets();
		break;
	case 1:
		if (info->getType() == Mesh_)
		{
			info->toggleShowFS();
		}
		break;
	case 2:
		if (info->getType() == Mesh_)
		{
			info->toggleUseTex();
		}
		break;
	case 3:
		delete info;
		m_datasetListCtrl->DeleteItem(item);
		renewAllGLWidgets();
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
	DatasetInfo *info = (DatasetInfo*) m_datasetListCtrl->GetItemData(item);
	m_tSlider->SetValue((int)(info->getThreshold()*100));
}

void MainFrame::OnListItemUp(wxCommandEvent& event)
{
	long item = m_datasetListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1) return;
	m_datasetListCtrl->moveItemUp(item);
	m_datasetListCtrl->EnsureVisible(item);
	if (item > 0) m_scene->swapTextures(item, item -1);
	refreshAllGLWidgets();
}
    
void MainFrame::OnListItemDown(wxCommandEvent& event)
{
	long item = m_datasetListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1) return;
	m_datasetListCtrl->moveItemDown(item);
	m_datasetListCtrl->EnsureVisible(item);
	if (item < m_datasetListCtrl->GetItemCount() - 1) m_scene->swapTextures(item, item + 1);
	refreshAllGLWidgets();
}
