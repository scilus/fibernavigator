/////////////////////////////////////////////////////////////////////////////
// Name:        main.h
// Author:      Ralph Schurade
/////////////////////////////////////////////////////////////////////////////

class MyApp : public wxApp
{
public:
    bool OnInit(void);

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
