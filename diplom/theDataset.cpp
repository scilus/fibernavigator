#include "theDataset.h"
#include "Anatomy.h"
#include "curves.h"
#include "point.h"
#include "myListCtrl.h"
#include "wx/xml/xml.h"

int TheDataset::rows = 1;
int TheDataset::columns = 1;
int TheDataset::frames = 1;
unsigned int TheDataset::countFibers = 0;
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

bool TheDataset::loadSettings(wxString filename)
{
	wxXmlDocument doc;
	if (!doc.Load(filename))
	    return false;

	wxXmlNode *child = doc.GetRoot()->GetChildren();
	while (child) {
		if (child->GetName() == wxT("anatomy"))
		{
			wxString srows = child->GetPropVal(wxT("rows"), wxT("1"));
			wxString scolumns = child->GetPropVal(wxT("columns"), wxT("1"));
			wxString sframes = child->GetPropVal(wxT("frames"), wxT("1"));
			long _rows, _columns, _frames;
			srows.ToLong(&_rows, 10);
			scolumns.ToLong(&_columns, 10);
			sframes.ToLong(&_frames, 10);
			if ((_rows != rows) || (_columns != columns) || (_frames != frames)) {
				lastError = wxT("dimensions of loaded files must be the same");
				return false;
			}
		}
		else if (child->GetName() == wxT("points"))
		{
			wxXmlNode *pNode = child->GetChildren();
			while (pNode) {
				wxString sx = pNode->GetPropVal(wxT("x"), wxT("0.0"));
				wxString sy = pNode->GetPropVal(wxT("y"), wxT("0.0"));
				wxString sz = pNode->GetPropVal(wxT("z"), wxT("0.0"));
				double _x, _y, _z;
				sx.ToDouble(&_x);
				sy.ToDouble(&_y);
				sz.ToDouble(&_z);
				Point *point = new Point(_x, _y, _z);
				treeWidget->AppendItem(tPointId, wxT("point"),-1, -1, new MyTreeItemData(point));
				pNode = pNode->GetNext();
			}
		}
		else if (child->GetName() == wxT("selection_boxes") && TheDataset::fibers_loaded)
		{
			wxXmlNode *mbNode = child->GetChildren();
			while (mbNode) {
				wxXmlNode *bNode = mbNode->GetChildren();
				std::vector<SelectionBox*>vboxes;
				double cx, cy, cz, ix, iy, iz;
				while (bNode) {
					if (bNode->GetName() == wxT("box")) {
						wxXmlNode *cNode = bNode->GetChildren();
						double _cx, _cy, _cz, _ix, _iy, _iz;
						wxString _type = bNode->GetPropVal(wxT("type"), wxT("AND"));
						while (cNode) {
							if (cNode->GetName() == wxT("size")) {
								wxString sx = cNode->GetPropVal(wxT("x"), wxT("0.0"));
								wxString sy = cNode->GetPropVal(wxT("y"), wxT("0.0"));
								wxString sz = cNode->GetPropVal(wxT("z"), wxT("0.0"));
								sx.ToDouble(&_ix);
								sy.ToDouble(&_iy);
								sz.ToDouble(&_iz);
							}
							else if (cNode->GetName() == wxT("center")) {
								wxString sx = cNode->GetPropVal(wxT("x"), wxT("0.0"));
								wxString sy = cNode->GetPropVal(wxT("y"), wxT("0.0"));
								wxString sz = cNode->GetPropVal(wxT("z"), wxT("0.0"));
								sx.ToDouble(&_cx);
								sy.ToDouble(&_cy);
								sz.ToDouble(&_cz);
							}
							cNode = cNode->GetNext();
						}
						Vector3fT _vc = {{_cx, _cy, _cz }};
						Vector3fT _vs = {{_ix, _iy, _iz }};
						SelectionBox *selBox = new SelectionBox(_vc, _vs);
						selBox->m_isTop = false;
						selBox->m_isNOT = (_type == wxT("NOT"));
						vboxes.push_back(selBox);
					}
					else if (bNode->GetName() == wxT("size")) {
						wxString sx = bNode->GetPropVal(wxT("x"), wxT("0.0"));
						wxString sy = bNode->GetPropVal(wxT("y"), wxT("0.0"));
						wxString sz = bNode->GetPropVal(wxT("z"), wxT("0.0"));
						sx.ToDouble(&ix);
						sy.ToDouble(&iy);
						sz.ToDouble(&iz);
					}
					else if (bNode->GetName() == wxT("center")) {
						wxString sx = bNode->GetPropVal(wxT("x"), wxT("0.0"));
						wxString sy = bNode->GetPropVal(wxT("y"), wxT("0.0"));
						wxString sz = bNode->GetPropVal(wxT("z"), wxT("0.0"));
						sx.ToDouble(&cx);
						sy.ToDouble(&cy);
						sz.ToDouble(&cz);
					}
					bNode = bNode->GetNext();
				}
				Vector3fT vc = {{cx, cy, cz }};
				Vector3fT vs = {{ix, iy, iz }};
				SelectionBox *selBox = new SelectionBox(vc, vs);
				selBox->m_isTop = true;
				wxTreeItemId boxId = treeWidget->AppendItem(tSelBoxId, wxT("box"),0, -1, new MyTreeItemData(selBox));
				for (int i = 0 ; i < vboxes.size() ; ++i) {
					treeWidget->AppendItem(boxId, wxT("box"), 0, -1, new MyTreeItemData(vboxes[i]));
				}
				mbNode = mbNode->GetNext();

			}
		}
		child = child->GetNext();
	}
	return true;
}

void TheDataset::save(wxString filename)
{
	wxXmlNode *root = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, wxT("scene"));
	wxXmlNode *nodeboxes = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("selection_boxes"));
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
				masterbox = new wxXmlNode(nodeboxes, wxXML_ELEMENT_NODE, wxT("master_box"));
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

void TheDataset::printwxT(wxString string)
{
	char cstring[string.length()];
	strcpy(cstring, (const char*)string.mb_str(wxConvUTF8));
	printf("%s\n", cstring);
}
