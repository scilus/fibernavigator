#ifndef MAIN_H_
#define MAIN_H_

#include <wx/app.h>

class MainFrame;

class MyApp : public wxApp
{
public:
    MyApp();
    virtual bool OnInit();
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
    static MainFrame * frame;
};


#endif // MAIN_H_