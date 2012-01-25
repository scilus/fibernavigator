#ifndef SCENEMANAGER_H_
#define SCENEMANAGER_H_

#include <wx/string.h>

class SceneManager
{
public:
    ~SceneManager(void);
    
    static SceneManager * getInstance();

    bool load(const wxString &filename);

protected:
    SceneManager(void);

private:
    SceneManager(const SceneManager &);
    SceneManager &operator=(const SceneManager &);

private:
    static SceneManager *m_pInstance;
};

#endif SCENEMANAGER_H_