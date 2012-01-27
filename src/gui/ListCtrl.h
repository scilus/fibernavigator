#ifndef LISTCTRL_H_
#define LISTCTRL_H_

#include <wx/listctrl.h>

#define ID_LIST_CTRL2 292

class DatasetInfo;

class ListCtrl : protected wxListCtrl
{
public:
    ListCtrl( wxWindow *pParent, const wxPoint &point, const wxSize &size, const long style );

    // Methods
    void AssignImageList( wxImageList *pImageList, int which );
    bool DeleteItem( long index );
    void DeleteSelectedItem();
    long InsertColumn( long col, const wxListItem& info );
    // TODO: Change for index in DatasetManager, once it is created
    void InsertItem( const DatasetInfo * const pDataset );
    void MoveItemDown();
    void MoveItemUp();
    void UpdateSelected();
    
    // Getters/Setters
    DatasetInfo * GetItem( long index ) const;
    int GetItemCount() const                        { return wxListCtrl::GetItemCount(); }
    bool SetColumnWidth( int col, int width )       { return wxListCtrl::SetColumnWidth( col, width ); }
    void SetMaxSize( const wxSize &size )           { wxListCtrl::SetMaxSize( size ); }
    void SetMinSize( const wxSize &size )           { wxListCtrl::SetMinSize( size ); }

    // Events
    void onActivate( wxListEvent& evt );
    void onLeftClick( wxMouseEvent& evt );

    // Operators
    operator wxWindow *()                           { return (wxWindow *)this; }

private:
    void Swap( long i, long j );
    void Update( long index );

private:
    int m_column;
    bool m_isFiberGroupPresent;

    DECLARE_EVENT_TABLE()
};


#endif //LISTCTRL_H_
