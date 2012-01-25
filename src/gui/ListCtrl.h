#ifndef LISTCTRL_H_
#define LISTCTRL_H_

#include <wx/listctrl.h>

#define ID_LIST_CTRL2 292

class DatasetInfo;

class ListCtrl : protected wxListCtrl
{
public:
    ListCtrl( wxWindow *pParent, const wxPoint &point, const wxSize &size, const long style );

    void AssignImageList( wxImageList *pImageList, int which );
    void DeleteSelectedItem();
    DatasetInfo *GetItem( long index );
    long InsertColumn( long col, const wxListItem& info );
    // TODO: Change for index in DatasetManager, once it is created
    void InsertItem( const DatasetInfo * const pDataset );
    void onActivate( wxListEvent& evt );
    void onLeftClick( wxMouseEvent& evt );
    bool SetColumnWidth( int col, int width );
    void SetMaxSize( const wxSize &size );
    void SetMinSize( const wxSize &size );
    void UpdateSelected();

    operator wxWindow *() { return (wxWindow *)this; }

private:
    void Update( long index );

private:
    int m_column;
    bool m_isFiberGroupPresent;

    DECLARE_EVENT_TABLE()
};


#endif //LISTCTRL_H_
