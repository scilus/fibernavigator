#include "mainFrame.h"

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/laywin.h"
#include "wx/filedlg.h"
#include "wx/statbmp.h"

DECLARE_EVENT_TYPE(wxEVT_NAVGL_EVENT, -1)
   
BEGIN_EVENT_TABLE(MainFrame, wxMDIParentFrame)
    EVT_MENU(VIEWER_ABOUT, MainFrame::OnAbout)
    EVT_SIZE(MainFrame::OnSize)
    EVT_MENU(VIEWER_QUIT, MainFrame::OnQuit)
    EVT_MENU(VIEWER_LOAD, MainFrame::OnLoad)
    EVT_MENU(VIEWER_NEW, MainFrame::OnNew)
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
    win->SetDefaultSize(wxSize(150 + NAV_SIZE, -1));
    win->SetOrientation(wxLAYOUT_HORIZONTAL);
    win->SetAlignment(wxLAYOUT_TOP);
    win->SetBackgroundColour(wxColour(0, 0, 0));
    m_leftWindowBottom = win;
    
    
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

    m_mainGL = new MainCanvas(m_scene, mainView, m_rightWindow, ID_GL_MAIN, wxDefaultPosition,
        			wxDefaultSize, 0, _T("MainGLCanvas"), gl_attrib);
    m_gl0 = new MainCanvas(m_scene, axial, m_topNavWindow, ID_GL_NAV_X, wxDefaultPosition,
    	        wxDefaultSize, 0, _T("NavGLCanvasX"), gl_attrib);
    m_gl1 = new MainCanvas(m_scene, coronal, m_middleNavWindow, ID_GL_NAV_Y, wxDefaultPosition,
        	        wxDefaultSize, 0, _T("NavGLCanvasY"), gl_attrib);
    m_gl2 = new MainCanvas(m_scene, sagittal, m_bottomNavWindow, ID_GL_NAV_Z, wxDefaultPosition,
       	        wxDefaultSize, 0, _T("NavGLCanvasZ"), gl_attrib);
}

void MainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void MainFrame::OnLoad(wxCommandEvent& WXUNUSED(event))
{
	wxString caption = wxT("Choose a file");
	wxString wildcard = wxT("Header files (*.hea)|*.hea|*.*|*.*");
	wxString defaultDir = wxEmptyString;
	wxString defaultFilename = wxEmptyString;
	wxFileDialog dialog(this, caption, defaultDir, defaultFilename, wildcard, wxOPEN);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxString path = dialog.GetPath();
		
		if (!m_dataset->load(path)) 
		{
			wxMessageBox(wxT("ERROR\n") + m_dataset->m_lastError,  wxT(""), wxOK|wxICON_INFORMATION, NULL);
			m_statusBar->SetStatusText(wxT("ERROR"),1);
			m_statusBar->SetStatusText(m_dataset->m_lastError,2);
		}
		else 
		{ 
			m_statusBar->SetStatusText(wxT("Ready"),1);
			m_statusBar->SetStatusText(dialog.GetFilename() + wxT(" loaded"),2);
			m_scene->setDataset(m_dataset);
			m_mainGL->invalidate();
			m_gl0->invalidate();
			m_gl1->invalidate();
			m_gl2->invalidate();
		}
		
		updateInfoString();
		
		m_xSlider->SetMax(wxMax(1,m_dataset->m_columns-1));
		m_xSlider->SetValue(m_dataset->m_columns/2);
		m_ySlider->SetMax(wxMax(1,m_dataset->m_rows-1));
		m_ySlider->SetValue( m_dataset->m_rows/2);
		m_zSlider->SetMax(wxMax(1,m_dataset->m_frames-1));
		m_zSlider->SetValue( m_dataset->m_frames/2);
		m_scene->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());
		refreshAllGLWidgets();
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
	case 0:
		pos = m_gl0->getMousePos();
		m_xSlider->SetValue((int)(((float)pos.x/NAV_GL_SIZE)*m_dataset->m_columns));
		m_ySlider->SetValue((int)(((float)pos.y/NAV_GL_SIZE)*m_dataset->m_rows));
		break;
	case 1:
		pos = m_gl1->getMousePos();
		m_xSlider->SetValue((int)(((float)pos.x/NAV_GL_SIZE)*m_dataset->m_columns));
		m_zSlider->SetValue((int)(((float)pos.y/NAV_GL_SIZE)*m_dataset->m_frames));
		break;
	case 2:
		pos = m_gl2->getMousePos();
		m_ySlider->SetValue((int)(((float)pos.x/NAV_GL_SIZE)*m_dataset->m_rows));
		m_zSlider->SetValue((int)(((float)pos.y/NAV_GL_SIZE)*m_dataset->m_frames));
		break;
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
	m_navWindow->SetDefaultSize(wxSize(NAV_SIZE, height));
	m_topNavWindow->SetDefaultSize(wxSize(NAV_SIZE, NAV_SIZE));
	m_middleNavWindow->SetDefaultSize(wxSize(NAV_SIZE, NAV_SIZE));
	m_bottomNavWindow->SetDefaultSize(wxSize(NAV_SIZE, NAV_SIZE));
	m_extraNavWindow->SetDefaultSize(wxSize(NAV_SIZE, NAV_SIZE));
	
	/* resize sliders */
	m_xSlider->SetSize(wxSize(NAV_GL_SIZE, -1));
	m_ySlider->SetSize(wxSize(NAV_GL_SIZE, -1));
	m_zSlider->SetSize(wxSize(NAV_GL_SIZE, -1));
	
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
	m_scene->updateBlendThreshold((float)m_tSlider->GetValue()/100.0);
	m_mainGL->render();
}

void MainFrame::refreshAllGLWidgets()
{
	updateStatusBar();
	m_gl0->render();
	m_gl1->render();
	m_gl2->render();
	m_mainGL->render();
}

void MainFrame::updateInfoString()
{
	m_textWindow->SetValue( wxT("") );
	if (m_dataset->m_dsList->size() == 0) return;
		
	DatasetList::iterator iter;
	wxString newString = wxT("");
	for (iter = m_dataset->m_dsList->begin() ; iter != m_dataset->m_dsList->end() ; ++iter)
	{
		DatasetInfo *info = *iter;
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
	m_dataset->load(wxT("/home/ralph/bin/devel/workspace/diplom/data/t1_1mm.hea"));
	//m_dataset->load(wxT("/home/ralph/bin/devel/workspace/diplom/data/overlay_swap.hea"));
	//m_dataset->load(wxT("/home/ralph/bin/devel/workspace/diplom/data/rgb.hea"));
	
	m_scene->setDataset(m_dataset);
	
	m_xSlider->SetMax(wxMax(1,m_dataset->m_columns-1));
	m_xSlider->SetValue(m_dataset->m_columns/2);
	m_ySlider->SetMax(wxMax(1,m_dataset->m_rows-1));
	m_ySlider->SetValue( m_dataset->m_rows/2);
	m_zSlider->SetMax(wxMax(1,m_dataset->m_frames-1));
	m_zSlider->SetValue( m_dataset->m_frames/2);
		
	m_mainGL->invalidate();
	m_gl0->invalidate();
	m_gl1->invalidate();
	m_gl2->invalidate();
	
	updateInfoString();
	
	m_scene->updateView(m_xSlider->GetValue(),m_ySlider->GetValue(),m_zSlider->GetValue());
	refreshAllGLWidgets();
}

void MainFrame::OnReloadShaders(wxCommandEvent& event)
{
	m_mainGL->invalidate();
	m_gl0->invalidate();
	m_gl1->invalidate();
	m_gl2->invalidate();
	refreshAllGLWidgets();
}

void MainFrame::OnNew(wxCommandEvent& event)
{
	free (m_dataset);
	m_dataset = new TheDataset();
	m_scene->releaseTextures();
	free (m_scene);
	m_scene = new TheScene();
	m_mainGL->setScene(m_scene);
	m_gl0->setScene(m_scene);
	m_gl1->setScene(m_scene);
	m_gl2->setScene(m_scene);
	updateInfoString();
}

void MainFrame::updateStatusBar()
{
	wxString sbString0;
	sbString0 = wxString::Format(wxT("Axial: %d Coronal: %d Sagittal: %d"),m_zSlider->GetValue(), m_ySlider->GetValue(), m_xSlider->GetValue()); 
	m_statusBar->SetStatusText(sbString0,0);
}
