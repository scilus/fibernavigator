/////////////////////////////////////////////////////////////////////////////
// Name:        main.h
// Purpose:     main window
// Author:      Ralph Schurade
// Modified by:
// Created:     03/27/08
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Define a new application
class MyApp: public wxApp
{
  public:
    bool OnInit(void);

  private:
    static const wxString APP_NAME;
    static const wxString APP_VENDOR;

  public:
    static wxString respath;
};
