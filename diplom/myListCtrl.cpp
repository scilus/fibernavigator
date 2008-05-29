#include "myListCtrl.h"

BEGIN_EVENT_TABLE(MyListCtrl, wxListCtrl)
	EVT_LEFT_DOWN(MyListCtrl::OnLeftClick)
END_EVENT_TABLE()



void MyListCtrl::OnLeftClick(wxMouseEvent& event)
{
	int col;
	int x = event.GetPosition().x;
	int sizeX = 0;
	for ( col = 0 ; col < this->GetColumnCount() ; ++col)
	{
		sizeX += this->GetColumnWidth(col);
		if (x <= sizeX) break;
	}
	m_col_clicked = col;
	
	event.Skip();
}

int MyListCtrl::getColClicked()
{
	return m_col_clicked;
}
