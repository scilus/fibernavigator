#include "theDataset.h"
#include "Anatomy.h"
#include "curves.h"
#include "point.h"
#include "myListCtrl.h"
#include "wx/xml/xml.h"

int TheDataset::rows = 1;
int TheDataset::columns = 1;
int TheDataset::frames = 1;
bool TheDataset::anatomy_loaded = false;
bool TheDataset::fibers_loaded = false;
bool TheDataset::surface_loaded = false;

wxTreeCtrl* TheDataset::treeWidget = NULL;
wxTreeItemId TheDataset::tSelBoxId = NULL;
wxTreeItemId TheDataset::tPointId = NULL;
wxListCtrl* TheDataset::listCtrl = NULL;

Matrix4fT TheDataset::m_transform = {  1.0f,  0.0f,  0.0f,  0.0f,
								        0.0f,  1.0f,  0.0f,  0.0f,
								        0.0f,  0.0f,  1.0f,  0.0f,
								        0.0f,  0.0f,  0.0f,  1.0f };
wxString TheDataset::lastError = wxT("");
wxString TheDataset::lastPath = wxT("");

DatasetInfo* TheDataset::load(wxString filename)
{
	// check file extension
	wxString ext = filename.AfterLast('.');
	if (ext == wxT("mesh")) {
		if (!anatomy_loaded) {
			lastError = wxT("no anatomy file loaded");
			return false;
		}
		Mesh *mesh = new Mesh();
		if (mesh->load(filename))
			return mesh;
		return false;
	}

	else if (ext == wxT("fib")) {
		if (!anatomy_loaded) {
			lastError = wxT("no anatomy file loaded");
			return false;
		}
		if (fibers_loaded) {
			lastError = wxT("fibers already loaded");
			return false;
		}
		Curves *curves = new Curves();
		if (curves->load(filename))
			return curves;
		return false;
	}

	else if (ext != wxT("hea")) return NULL;

	Anatomy *anatomy = new Anatomy();
	bool flag = anatomy->load(filename);

	if (flag)
	{
		rows = anatomy->getRows();
		columns = anatomy->getColumns();
		frames = anatomy->getFrames();
		anatomy_loaded = true;
		return anatomy;
	}
	else
	{
		lastError = wxT("couldn't load anatomy file");
		return NULL;
	}

	if (!TheDataset::anatomy_loaded)
	{
		if ( anatomy->getRows() <= 0 || anatomy->getColumns() <= 0 || anatomy->getFrames() <= 0 )
		{
			lastError = wxT("couldn't parse header file");
			return NULL;
		}
	}
	else
	{
		if ( anatomy->getRows() != rows || anatomy->getColumns() != columns || anatomy->getFrames() != frames )
		{
			lastError = wxT("dimensions of loaded files must be the same");
			return NULL;
		}
	}
	return NULL;
}

void TheDataset::save(wxString filename)
{
	wxXmlNode *root = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, wxT("scene"));
	wxXmlNode *nodeboxes = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("selection boxes"));
	wxXmlNode *nodepoints = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("points"));
	wxXmlNode *anatomy = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("anatomy"));

	wxXmlProperty *prop1 = new wxXmlProperty(wxT("rows"), wxString::Format(wxT("%d"), rows));
	wxXmlProperty *prop2 = new wxXmlProperty(wxT("columns"), wxString::Format(wxT("%d"), columns), prop1);
	wxXmlProperty *prop3 = new wxXmlProperty(wxT("frames"), wxString::Format(wxT("%d"), frames), prop2);
	anatomy->AddProperty(prop3);

	int countPoints = TheDataset::treeWidget->GetChildrenCount(TheDataset::tPointId, true);
	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	for (int i = 0 ; i < countPoints ; ++i)
	{
		id = TheDataset::treeWidget->GetNextChild(TheDataset::tPointId, cookie);
		Point *point = (Point*)((MyTreeItemData*)TheDataset::treeWidget->GetItemData(id))->getData();
		wxXmlNode *pointnode = new wxXmlNode(nodepoints, wxXML_ELEMENT_NODE, wxT("point"));

		wxXmlProperty *propz = new wxXmlProperty(wxT("z"), wxString::Format(wxT("%f"), point->getCenter().s.Z));
		wxXmlProperty *propy = new wxXmlProperty(wxT("y"), wxString::Format(wxT("%f"), point->getCenter().s.Y), propz);
		wxXmlProperty *propx = new wxXmlProperty(wxT("x"), wxString::Format(wxT("%f"), point->getCenter().s.X), propy);
		pointnode->AddProperty(propx);
	}
	wxXmlNode *masterbox;
	std::vector<std::vector<SelectionBox*> > boxes = getSelectionBoxes();
	for (uint i = 0 ; i < boxes.size() ; ++i)
	{
		for (uint j = 0 ; j < boxes[i].size() ; ++j)
		{
			if (j == 0)
			{
				masterbox = new wxXmlNode(nodeboxes, wxXML_ELEMENT_NODE, wxT("master box"));
				wxXmlNode *center = new wxXmlNode(masterbox, wxXML_ELEMENT_NODE, wxT("center"));
				wxXmlProperty *propz = new wxXmlProperty(wxT("z"), wxString::Format(wxT("%f"), boxes[i][j]->getCenter().s.Z));
				wxXmlProperty *propy = new wxXmlProperty(wxT("y"), wxString::Format(wxT("%f"), boxes[i][j]->getCenter().s.Y), propz);
				wxXmlProperty *propx = new wxXmlProperty(wxT("x"), wxString::Format(wxT("%f"), boxes[i][j]->getCenter().s.X), propy);
				center->AddProperty(propx);
				wxXmlNode *size = new wxXmlNode(masterbox, wxXML_ELEMENT_NODE, wxT("size"));
				propz = new wxXmlProperty(wxT("z"), wxString::Format(wxT("%f"), boxes[i][j]->getSize().s.Z));
				propy = new wxXmlProperty(wxT("y"), wxString::Format(wxT("%f"), boxes[i][j]->getSize().s.Y), propz);
				propx = new wxXmlProperty(wxT("x"), wxString::Format(wxT("%f"), boxes[i][j]->getSize().s.X), propy);
				size->AddProperty(propx);
			}
			else
			{
				wxXmlNode *box = new wxXmlNode(masterbox, wxXML_ELEMENT_NODE, wxT("box"));
				wxXmlProperty *proptype = new wxXmlProperty(wxT("type"), (boxes[i][j]->m_isNOT) ? wxT("NOT") : wxT("AND"));
				box->AddProperty(proptype);
				wxXmlNode *center = new wxXmlNode(box, wxXML_ELEMENT_NODE, wxT("center"));
				wxXmlProperty *propz = new wxXmlProperty(wxT("z"), wxString::Format(wxT("%f"), boxes[i][j]->getCenter().s.Z));
				wxXmlProperty *propy = new wxXmlProperty(wxT("y"), wxString::Format(wxT("%f"), boxes[i][j]->getCenter().s.Y), propz);
				wxXmlProperty *propx = new wxXmlProperty(wxT("x"), wxString::Format(wxT("%f"), boxes[i][j]->getCenter().s.X), propy);
				center->AddProperty(propx);
				wxXmlNode *size = new wxXmlNode(box, wxXML_ELEMENT_NODE, wxT("size"));
				propz = new wxXmlProperty(wxT("z"), wxString::Format(wxT("%f"), boxes[i][j]->getSize().s.Z));
				propy = new wxXmlProperty(wxT("y"), wxString::Format(wxT("%f"), boxes[i][j]->getSize().s.Y), propz);
				propx = new wxXmlProperty(wxT("x"), wxString::Format(wxT("%f"), boxes[i][j]->getSize().s.X), propy);
				size->AddProperty(propx);

			}
		}
	}


	wxXmlDocument doc;
	doc.SetRoot(root);
	doc.Save(filename, 2);
}
/*
void TheDataset::save(wxString filename)
{
	wxFileOutputStream file( filename );
	wxTextOutputStream text( file );
	text << wxT("[Anatomy]\n");
	text << rows << wxT(" ") << columns << wxT(" ") << frames << wxT("\n");

	text << wxT("[Points]\n");
	int countPoints = TheDataset::treeWidget->GetChildrenCount(TheDataset::tPointId, true);
	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	for (int i = 0 ; i < countPoints ; ++i)
	{
		id = TheDataset::treeWidget->GetNextChild(TheDataset::tPointId, cookie);
		Point *point = (Point*)((MyTreeItemData*)TheDataset::treeWidget->GetItemData(id))->getData();

		text << point->getCenter().s.X << wxT(" ");
		text << point->getCenter().s.Y << wxT(" ");
		text << point->getCenter().s.Z << wxT("\n");
	}

	text << wxT("[Selection Boxes]\n");
	std::vector<std::vector<SelectionBox*> > boxes = getSelectionBoxes();
	for (uint i = 0 ; i < boxes.size() ; ++i)
	{
		text << wxT("[Box]\n");
		for (uint j = 0 ; j < boxes[i].size() ; ++j)
		{
			if (j != 0) text << wxT("[Child Box]\n");
			text << boxes[i][j]->getCenter().s.X << wxT(" ");
			text << boxes[i][j]->getCenter().s.Y << wxT(" ");
			text << boxes[i][j]->getCenter().s.Z << wxT("\n");
		}
	}
}
*/
void TheDataset::printTime()
{
	wxDateTime dt = wxDateTime::Now();
	printf("[%02d:%02d:%02d] ",dt.GetHour(), dt.GetMinute(), dt.GetSecond());
}

std::vector<std::vector<SelectionBox*> > TheDataset::getSelectionBoxes()
{
	std::vector<std::vector<SelectionBox*> > boxes;
	int countboxes = TheDataset::treeWidget->GetChildrenCount(TheDataset::tSelBoxId, false);
	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	for (int i = 0 ; i < countboxes ; ++i)
	{
		std::vector<SelectionBox*> b;
		id = TheDataset::treeWidget->GetNextChild(TheDataset::tSelBoxId, cookie);
		if (id.IsOk()) {
			b.push_back((SelectionBox*)((MyTreeItemData*)TheDataset::treeWidget->GetItemData(id))->getData());
			int childboxes = TheDataset::treeWidget->GetChildrenCount(id);
			wxTreeItemIdValue childcookie = 0;
			for (int i = 0 ; i < childboxes ; ++i)
			{
				childid = TheDataset::treeWidget->GetNextChild(id, childcookie);
				if (childid.IsOk()) {
					b.push_back((SelectionBox*)((MyTreeItemData*)TheDataset::treeWidget->GetItemData(childid))->getData());
				}
			}
		}
		boxes.push_back(b);
	}
	return boxes;
}
