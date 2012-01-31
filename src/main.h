/////////////////////////////////////////////////////////////////////////////
// Name:        main.h
// Author:      Ralph Schurade
/////////////////////////////////////////////////////////////////////////////

#ifndef MAIN_H_
#define MAIN_H_

#include <vld.h> 

#include "wx/app.h"

class MyApp : public wxApp
{
public:
    MyApp();
    bool OnInit(void);
    int  OnExit();

private:
    wxString wxFindAppPath(const wxString& argv0, const wxString& cwd,
            const wxString& appVariableName, const wxString& appName);
    static const wxString APP_NAME;
    static const wxString APP_VENDOR;

public:
    static wxString respath;
    static wxString shaderPath;
    static wxString iconsPath;
};

#endif // MAIN_H_