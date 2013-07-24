/////////////////////////////////////////////////////////////////////////////
// Name:            TrackingWindow.h
// Author:          ---
// Creation Date:   ---
//
// Description: This is the implementation file for the Realtime tracking window.
/////////////////////////////////////////////////////////////////////////////


#ifndef TRACKINGWINDOW_H_
#define TRACKINGWINDOW_H_

#include "MainCanvas.h"
#include "MyListCtrl.h"

#include "../misc/Algorithms/Helper.h"

#include <wx/scrolwin.h>
#include <wx/statline.h>

class MainFrame;
class wxToggleButton;

class TrackingWindow: public wxScrolledWindow
{
public:
    TrackingWindow(){};
    TrackingWindow( wxWindow *pParent, MainFrame *pMf, wxWindowID id, const wxPoint &pos, const wxSize &size );
    TrackingWindow( wxWindow *pParent, MainFrame *pMf, wxWindowID id, const wxPoint &pos, const wxSize &size, int hardi);

    ~TrackingWindow(){};
    void OnPaint( wxPaintEvent &event );
    void OnSize( wxSizeEvent &event );
    wxSizer* getWindowSizer();


    void OnStartTracking                       ( wxCommandEvent& event );
	void OnSelectShell						   ( wxCommandEvent& event );
    void OnClearBox                            ( wxTreeEvent&    event );
    void OnSliderFAMoved                       ( wxCommandEvent& event );
    void OnSliderAngleMoved                    ( wxCommandEvent& event );
    void OnSliderStepMoved                     ( wxCommandEvent& event );
    void OnSelectFileDTI                       ( wxCommandEvent& event );
    void OnSelectFileHARDI                     ( wxCommandEvent& event );
    void OnRandomSeeding                       ( wxCommandEvent& event );
    void OnShellSeeding                        ( wxCommandEvent& event );
    void OnSelectMask                           ( wxCommandEvent& event );
    void OnInterpolate                         ( wxCommandEvent& event );
    void OnSliderPunctureMoved                 ( wxCommandEvent& event );
    void OnSliderMinLengthMoved                ( wxCommandEvent& event );
    void OnSliderMaxLengthMoved                ( wxCommandEvent& event );
	void OnConvertToFibers					   ( wxCommandEvent& event );
    void OnSliderAxisSeedNbMoved               ( wxCommandEvent& event );
	void OnMapSeeding                          ( wxCommandEvent& event );
	void OnSelectSeedMap                       ( wxCommandEvent& event );

    void OnPlay                                ( wxCommandEvent& event );
    void OnStop                                ( wxCommandEvent& event );

public:
    wxToggleButton      *m_pBtnStart;
    wxTextCtrl          *m_pTxtTotalSeedNbBox;


private:
    MainFrame           *m_pMainFrame;
    wxSlider            *m_pSliderFA;
    wxTextCtrl          *m_pTxtFABox;
    wxStaticText        *m_pTextFA;
    wxSlider            *m_pSliderAngle;
    wxStaticText        *m_pTextAngle;
    wxTextCtrl          *m_pTxtAngleBox;
    wxSlider            *m_pSliderStep;
    wxStaticText        *m_pTextStep;
    wxTextCtrl          *m_pTxtStepBox;
    wxSlider            *m_pSliderPuncture;
    wxStaticText        *m_pTextPuncture;
    wxTextCtrl          *m_pTxtPunctureBox;
    wxButton            *m_pBtnSelectFile;
	wxButton			*m_pBtnSelectShell;
	wxToggleButton		*m_pToggleShell;
	wxButton			*m_pBtnSelectSeed;
	wxToggleButton		*m_pToggleSeedMap;
    wxButton            *m_pBtnSelectMap;
    wxStaticText        *m_pTextMinLength;
    wxSlider            *m_pSliderMinLength;
    wxTextCtrl          *m_pTxtMinLengthBox;
    wxSlider            *m_pSliderMaxLength;
    wxStaticText        *m_pTextMaxLength;
    wxTextCtrl          *m_pTxtMaxLengthBox;
	wxButton			*m_pBtnConvert;
    wxSlider            *m_pSliderAxisSeedNb;
    wxTextCtrl          *m_pTxtAxisSeedNbBox;
    wxStaticText        *m_pTextAxisSeedNb;
    wxStaticText        *m_pTextTotalSeedNb;
    wxStaticLine        *m_pLineSeparator;
    wxBitmapButton      *m_pPlayPause;
    wxBitmapButton      *m_pBtnStop;
    wxImage             m_bmpPause;
    wxImage             m_bmpPlay;
    


private:
    wxSizer *m_pTrackingSizer;
    TrackingWindow( wxWindow *pParent, wxWindowID id, const wxPoint &pos, const wxSize &size );
    DECLARE_DYNAMIC_CLASS( TrackingWindow )
    DECLARE_EVENT_TABLE()
};

#endif /*TRACKINGWINDOW*/
