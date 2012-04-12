#ifndef LISTCTRL_H_
#define LISTCTRL_H_

#include "../dataset/DatasetManager.h"

#include <wx/listctrl.h>
#include <vector>

#define ID_LIST_CTRL 292

class DatasetInfo;
class Anatomy;
class Fibers;
class FibersGroup;
class Mesh;
class ODFs;
class Tensors;

class ListCtrl : protected wxListCtrl
{
public:
    ListCtrl( wxWindow *pParent, const wxPoint &point, const wxSize &size, const long style );
    ~ListCtrl();

    // Methods
    void AssignImageList( wxImageList *pImageList, int which );
    void Clear()                                    { wxListCtrl::DeleteAllItems(); }
    bool DeleteItem( long index );
    void DeleteSelectedItem();
    long InsertColumn( long col, wxListItem& info );
    void InsertItem( DatasetIndex datasetIndex );
    void InsertItemRange( const std::vector<DatasetIndex> &items );
    void MoveItemDown();
    void MoveItemUp();
    void UnselectAll();
    void UpdateFibers();
    void UpdateSelected();
    
    // Getters/Setters
    int  GetColumnClicked() const                   { return m_column; }
    DatasetIndex GetItem( long index ) const;
    int  GetItemCount() const                       { return wxListCtrl::GetItemCount(); }
    bool SetColumnWidth( int col, int width )       { return wxListCtrl::SetColumnWidth( col, width ); }
    void SetMaxSize( const wxSize &size )           { wxListCtrl::SetMaxSize( size ); }
    void SetMinSize( const wxSize &size )           { wxListCtrl::SetMinSize( size ); }

    // Events
    void onActivate( wxListEvent& evt );
    void onLeftClick( wxMouseEvent& evt );

    // Operators
    operator wxWindow *()                           { return (wxWindow *)this; }

private:
    ListCtrl( const ListCtrl & );
    ListCtrl & operator= ( const ListCtrl & );

    long FindFiberGroupPosition();
    void Swap( long i, long j );
    void Update( long index );

private:
    int m_column;

    DECLARE_EVENT_TABLE()
};


#endif //LISTCTRL_H_
