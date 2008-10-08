/*
 * DatasetHelper.cpp
 *
 *  Created on: 27.07.2008
 *      Author: ralph
 */
#include "DatasetHelper.h"

#include "Anatomy.h"
#include "curves.h"
#include "splinePoint.h"
#include "mesh.h"
#include "KdTree.h"
#include "surface.h"
#include "myListCtrl.h"


#include "IsoSurface/CIsoSurface.h"

DatasetHelper::DatasetHelper(MainFrame* mf) {
	mainFrame = mf;

	rows = 1;
	columns = 1;
	frames = 1;
	countFibers = 0;
	threadsActive = 0;
	anatomy_loaded = false;
	fibers_loaded = false;
	surface_loaded = false;
	surface_isDirty = true;
	useVBO = true;
	lighting = false;

	showSagittal = true;
	showCoronal = true;
	showAxial = true;
	xSlize = 0.5;
	ySlize = 0.5;
	zSlize = 0.5;
	quadrant = 6;

	lastSelectedPoint = NULL;
	lastSelectedBox = NULL;
	scene = NULL;
	anatomyHelper = NULL;
	shaderHelper = NULL;

	Matrix4fSetIdentity(&m_transform);

	lastError = wxT("");
	lastPath = wxT("");
	lastGLError = GL_NO_ERROR;

	fibersInverted = false;

	m_isDragging = false;
	m_isrDragging = false;

	useLinearFiltering = true;
}

DatasetHelper::~DatasetHelper() {
#ifdef DEBUG
	printf("execute dataset helper destructor\n");
#endif
	if (scene)	delete scene;
	if (anatomyHelper) delete anatomyHelper;
	if (lastSelectedPoint) delete lastSelectedPoint;
#ifdef DEBUG
	printf("dataset helper destructor done\n");
#endif
}

bool DatasetHelper::load(int index, wxString filename)
{
	if (mainFrame->m_listCtrl->GetItemCount() > 9)
	{
		lastError = wxT("ERROR\nCan't load any more files.\nDelete some first.\n");
		return false;
	}

	if (index >= 0)
	{
		wxString caption = wxT("Choose a file");
		wxString wildcard = wxT("*.*|*.*|Header files (*.hea)|*.hea|Mesh files (*.mesh)|*.mesh|Fibers VTK (*.fib)|*.fib");
		wxString defaultDir = wxEmptyString;
		wxString defaultFilename = wxEmptyString;
		wxFileDialog dialog(mainFrame, caption, defaultDir, defaultFilename, wildcard, wxOPEN);
		dialog.SetFilterIndex(index);
		dialog.SetDirectory(lastPath);
		if (dialog.ShowModal() == wxID_OK)
		{
			lastPath = dialog.GetDirectory();
			filename = dialog.GetPath();
		}
		else return false;
	}
	// check file extension
	wxString ext = filename.AfterLast('.');

	if (ext == wxT("yav")) {
		if (!loadSettings(filename)) {
			return false;
		}
		scene->m_selBoxChanged = true;
		mainFrame->refreshAllGLWidgets();
		return true ;
	}

	else if (ext == wxT("hea"))
	{
		Anatomy *anatomy = new Anatomy(this);
		if (anatomy->load(filename))
		{
			rows = anatomy->getRows();
			columns = anatomy->getColumns();
			frames = anatomy->getFrames();
			anatomy_loaded = true;
			finishLoading(anatomy);
			return true;
		}
		else {
			if (!anatomy_loaded)
			{
				if ( anatomy->getRows() <= 0 || anatomy->getColumns() <= 0 || anatomy->getFrames() <= 0 )
				{
					lastError = wxT("couldn't parse header file");
					return false;
				}
			}
			else
			{
				if ( anatomy->getRows() != rows || anatomy->getColumns() != columns || anatomy->getFrames() != frames )
				{
					lastError = wxT("dimensions of loaded files must be the same");
					return false;
				}
			}
			lastError = wxT("couldn't load anatomy file");
			return false;
		}
	}

	else if (ext == wxT("mesh")) {
		if (!anatomy_loaded) {
			lastError = wxT("no anatomy file loaded");
			return false;
		}
		Mesh *mesh = new Mesh(this);
		if (mesh->load(filename)) {
			finishLoading(mesh);
			return true;
		}
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
		Curves *curves = new Curves(this);
		if (curves->load(filename)) {
			if (index != -1) {
				Vector3fT vc = {{mainFrame->m_xSlider->GetValue()-columns/2,
							mainFrame->m_ySlider->GetValue()-rows/2,
							mainFrame->m_zSlider->GetValue()-frames/2}};

				Vector3fT vs = {{columns/8, rows/8, frames/8}};
						SelectionBox *selBox = new SelectionBox(vc, vs, this);
						selBox->m_isTop = true;
						wxTreeItemId tNewBoxId = mainFrame->m_treeWidget->AppendItem(mainFrame->m_tSelBoxId, wxT("box"),0, -1, new MyTreeItemData(selBox, MasterBox));
						mainFrame->m_treeWidget->EnsureVisible(tNewBoxId);
						selBox->setTreeId(tNewBoxId);
			}
			finishLoading(curves);
			return true;
		}
		return false;
	}

	lastError = wxT("unsupported file format");
	return false;
}

void DatasetHelper::finishLoading(DatasetInfo *info)
{
	//int i = mainFrame->m_listCtrl->GetItemCount();
	mainFrame->m_listCtrl->InsertItem(0, wxT(""), 0);
	mainFrame->m_listCtrl->SetItem(0, 1, info->getName());
	mainFrame->m_listCtrl->SetItem(0, 2, wxString::Format(wxT("%.2f"), info->getThreshold()));
	mainFrame->m_listCtrl->SetItem(0, 3, wxT(""), 1);
	mainFrame->m_listCtrl->SetItemData(0, (long)info);
	mainFrame->m_listCtrl->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

	mainFrame->m_statusBar->SetStatusText(wxT("Ready"),1);
	mainFrame->m_statusBar->SetStatusText(info->getName() + wxT(" loaded"),2);

	if (mainFrame->m_listCtrl->GetItemCount() == 1)
	{
		mainFrame->m_xSlider->SetMax(wxMax(2,columns-1));
		mainFrame->m_xSlider->SetValue(columns/2);
		mainFrame->m_ySlider->SetMax(wxMax(2,rows-1));
		mainFrame->m_ySlider->SetValue( rows/2);
		mainFrame->m_zSlider->SetMax(wxMax(2,frames-1));
		mainFrame->m_zSlider->SetValue( frames/2);
		mainFrame->m_tSlider->SetValue(10);
		updateView(mainFrame->m_xSlider->GetValue(),
										mainFrame->m_ySlider->GetValue(),
										mainFrame->m_zSlider->GetValue());

		mainFrame->renewAllGLWidgets();
		updateTreeDims();
		updateTreeDS(0);
	}
	else
	{
		mainFrame->refreshAllGLWidgets();
		updateTreeDS(0);
	}


}

bool DatasetHelper::loadSettings(wxString filename)
{
	/*
	 * Variables to store the slice postions in, have to be set after loading
	 * the anatomy files
	 */
	long xp, yp, zp;

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
			if (anatomy_loaded) {
				if ((_rows != rows) || (_columns != columns) || (_frames != frames)) {
					lastError = wxT("dimensions of loaded files must be the same");
					return false;
				}
			}
			else {
				rows = _rows;
				columns = _columns;
				frames = _frames;
				updateTreeDims();
				anatomy_loaded = true;
			}
			xp = rows/2;
			yp = columns/2;
			zp = frames/2;
		}

		else if (child->GetName() == wxT("position"))
		{
			wxString xPos = child->GetPropVal(wxT("x"), wxT("1"));
			wxString yPos = child->GetPropVal(wxT("y"), wxT("1"));
			wxString zPos = child->GetPropVal(wxT("z"), wxT("1"));
			xPos.ToLong(&xp, 10);
			yPos.ToLong(&yp, 10);
			zPos.ToLong(&zp, 10);
		}

		else if (child->GetName() == wxT("rotation"))
		{
			wxString sM00 = child->GetPropVal(wxT("M00"), wxT("1"));
			wxString sM01 = child->GetPropVal(wxT("M01"), wxT("1"));
			wxString sM02 = child->GetPropVal(wxT("M02"), wxT("1"));
			wxString sM10 = child->GetPropVal(wxT("M10"), wxT("1"));
			wxString sM11 = child->GetPropVal(wxT("M11"), wxT("1"));
			wxString sM12 = child->GetPropVal(wxT("M12"), wxT("1"));
			wxString sM20 = child->GetPropVal(wxT("M20"), wxT("1"));
			wxString sM21 = child->GetPropVal(wxT("M21"), wxT("1"));
			wxString sM22 = child->GetPropVal(wxT("M22"), wxT("1"));
			double M00, M01, M02, M10, M11, M12, M20, M21, M22;
			sM00.ToDouble(&M00);
			sM01.ToDouble(&M01);
			sM02.ToDouble(&M02);
			sM10.ToDouble(&M10);
			sM11.ToDouble(&M11);
			sM12.ToDouble(&M12);
			sM20.ToDouble(&M20);
			sM21.ToDouble(&M21);
			sM22.ToDouble(&M22);
			Matrix4fSetIdentity(&m_transform);
			m_transform.s.M00 = M00;
			m_transform.s.M01 = M01;
			m_transform.s.M02 = M02;
			m_transform.s.M10 = M10;
			m_transform.s.M11 = M11;
			m_transform.s.M12 = M12;
			m_transform.s.M20 = M20;
			m_transform.s.M21 = M21;
			m_transform.s.M22 = M22;
			mainFrame->m_mainGL->setRotation();
		}

		else if (child->GetName() == wxT("data"))
		{
			wxXmlNode *datasetnode = child->GetChildren();
			while (datasetnode) {
				load(-1, datasetnode->GetNodeContent());

				datasetnode = datasetnode->GetNext();
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
				SplinePoint *point = new SplinePoint(_x, _y, _z, this);
				mainFrame->m_treeWidget->AppendItem(mainFrame->m_tPointId, wxT("point"),-1, -1, new MyTreeItemData(point, SPoint));
				pNode = pNode->GetNext();
			}
		}

		else if (child->GetName() == wxT("selection_boxes") /*&& TheDataset::fibers_loaded*/)
		{
			wxXmlNode *mbNode = child->GetChildren();
			while (mbNode) {
				wxXmlNode *bNode = mbNode->GetChildren();
				std::vector<SelectionBox*>vboxes;
				wxString _name;
				double cx, cy, cz, ix, iy, iz;
				double _cx, _cy, _cz, _ix, _iy, _iz;
				cx = cy = cz = ix = iy = iz =
				_cx = _cy = _cz = _ix = _iy = _iz = 0.0;
				while (bNode) {
					if (bNode->GetName() == wxT("box")) {
						wxXmlNode *cNode = bNode->GetChildren();
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
							else if (cNode->GetName() == wxT("name")) {
								_name = cNode->GetPropVal(wxT("string"), wxT("box"));
							}
							cNode = cNode->GetNext();
						}
						Vector3fT _vc = {{_cx, _cy, _cz }};
						Vector3fT _vs = {{_ix, _iy, _iz }};
						SelectionBox *selBox = new SelectionBox(_vc, _vs, this);
						selBox->m_isTop = false;
						selBox->m_isNOT = (_type == wxT("NOT"));
						selBox->setName(_name);
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
					else if (bNode->GetName() == wxT("name")) {
						_name = bNode->GetPropVal(wxT("string"), wxT("box"));
					}
					bNode = bNode->GetNext();
				}
				Vector3fT vc = {{cx, cy, cz }};
				Vector3fT vs = {{ix, iy, iz }};
				SelectionBox *selBox = new SelectionBox(vc, vs, this);
				selBox->m_isTop = true;
				selBox->setName(_name);
				wxTreeItemId boxId = mainFrame->m_treeWidget->AppendItem(mainFrame->m_tSelBoxId, selBox->getName(),0, -1, new MyTreeItemData(selBox, MasterBox));
				mainFrame->m_treeWidget->EnsureVisible(boxId);
				selBox->setTreeId(boxId);
				for (unsigned int i = 0 ; i < vboxes.size() ; ++i) {
					wxTreeItemId tNewBoxId = mainFrame->m_treeWidget->AppendItem(boxId, vboxes[i]->getName(), 0, -1, new MyTreeItemData(vboxes[i], ChildBox));
					mainFrame->m_treeWidget->EnsureVisible(tNewBoxId);
					vboxes[i]->setTreeId(tNewBoxId);
				}
				mbNode = mbNode->GetNext();

			}
		}
		child = child->GetNext();
	}
	mainFrame->m_xSlider->SetValue(xp);
	mainFrame->m_ySlider->SetValue(yp);
	mainFrame->m_zSlider->SetValue(zp);
	updateView(xp, yp, zp);

	return true;
}

void DatasetHelper::save(wxString filename)
{
	wxXmlNode *root = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, wxT("scene"));
	wxXmlNode *nodeboxes = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("selection_boxes"));
	wxXmlNode *nodepoints = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("points"));
	wxXmlNode *data = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("data"));
	wxXmlNode *anatomy = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("anatomy"));
	wxXmlNode *anatomyPos = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("position"));
	wxXmlNode *anatomyRot = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("rotation"));

	wxXmlProperty *prop1 = new wxXmlProperty(wxT("rows"), wxString::Format(wxT("%d"), rows));
	wxXmlProperty *prop2 = new wxXmlProperty(wxT("columns"), wxString::Format(wxT("%d"), columns), prop1);
	wxXmlProperty *prop3 = new wxXmlProperty(wxT("frames"), wxString::Format(wxT("%d"), frames), prop2);
	anatomy->AddProperty(prop3);

	int countPoints = mainFrame->m_treeWidget->GetChildrenCount(mainFrame->m_tPointId, true);
	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	for (int i = 0 ; i < countPoints ; ++i)
	{
		id = mainFrame->m_treeWidget->GetNextChild(mainFrame->m_tPointId, cookie);
		SplinePoint *point = (SplinePoint*)((MyTreeItemData*)mainFrame->m_treeWidget->GetItemData(id))->getData();
		wxXmlNode *pointnode = new wxXmlNode(nodepoints, wxXML_ELEMENT_NODE, wxT("point"));

		wxXmlProperty *propz = new wxXmlProperty(wxT("z"), wxString::Format(wxT("%f"), point->getCenter().s.Z));
		wxXmlProperty *propy = new wxXmlProperty(wxT("y"), wxString::Format(wxT("%f"), point->getCenter().s.Y), propz);
		wxXmlProperty *propx = new wxXmlProperty(wxT("x"), wxString::Format(wxT("%f"), point->getCenter().s.X), propy);
		pointnode->AddProperty(propx);
	}

	std::vector<std::vector<SelectionBox*> > boxes = getSelectionBoxes();
	for (unsigned int i = 0 ; i < boxes.size() ; ++i)
	{
		wxXmlNode *masterbox = new wxXmlNode(nodeboxes, wxXML_ELEMENT_NODE, wxT("master_box"));
		for (unsigned int j = 0 ; j < boxes[i].size() ; ++j)
		{
			if (j == 0)
			{
				wxXmlNode *name = new wxXmlNode(masterbox, wxXML_ELEMENT_NODE, wxT("name"));
				wxXmlProperty *propname = new wxXmlProperty(wxT("string"), boxes[i][j]->getName());
				name->AddProperty(propname);
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

				wxXmlNode *name = new wxXmlNode(box, wxXML_ELEMENT_NODE, wxT("name"));
				wxXmlProperty *propname = new wxXmlProperty(wxT("string"), boxes[i][j]->getName());
				name->AddProperty(propname);

			}
		}
	}

	int countTextures = mainFrame->m_listCtrl->GetItemCount();
	if (countTextures == 0) return;

	for (int i = countTextures ; i > 0 ; --i)
	{
		DatasetInfo* info = (DatasetInfo*)mainFrame->m_listCtrl->GetItemData(i-1);
		if (info->getType() != Surface_)
		{
			wxXmlNode *datasetnode = new wxXmlNode(data, wxXML_ELEMENT_NODE, wxT("dataset"));
			new wxXmlNode(datasetnode, wxXML_TEXT_NODE, wxT("path"), info->getPath());
		}
	}

	wxXmlProperty *propPosX = new wxXmlProperty(wxT("x"), wxString::Format(wxT("%d"), mainFrame->m_xSlider->GetValue()));
	wxXmlProperty *propPosY = new wxXmlProperty(wxT("y"), wxString::Format(wxT("%d"), mainFrame->m_ySlider->GetValue()), propPosX);
	wxXmlProperty *propPosZ = new wxXmlProperty(wxT("z"), wxString::Format(wxT("%d"), mainFrame->m_zSlider->GetValue()), propPosY);
	anatomyPos->AddProperty(propPosZ);

	wxXmlProperty *rot00 = new wxXmlProperty(wxT("M00"), wxString::Format(wxT("%.15f"), m_transform.s.M00));
	wxXmlProperty *rot01 = new wxXmlProperty(wxT("M01"), wxString::Format(wxT("%.15f"), m_transform.s.M01), rot00);
	wxXmlProperty *rot02 = new wxXmlProperty(wxT("M02"), wxString::Format(wxT("%.15f"), m_transform.s.M02), rot01);
	wxXmlProperty *rot10 = new wxXmlProperty(wxT("M10"), wxString::Format(wxT("%.15f"), m_transform.s.M10), rot02);
	wxXmlProperty *rot11 = new wxXmlProperty(wxT("M11"), wxString::Format(wxT("%.15f"), m_transform.s.M11), rot10);
	wxXmlProperty *rot12 = new wxXmlProperty(wxT("M12"), wxString::Format(wxT("%.15f"), m_transform.s.M12), rot11);
	wxXmlProperty *rot20 = new wxXmlProperty(wxT("M20"), wxString::Format(wxT("%.15f"), m_transform.s.M20), rot12);
	wxXmlProperty *rot21 = new wxXmlProperty(wxT("M21"), wxString::Format(wxT("%.15f"), m_transform.s.M21), rot20);
	wxXmlProperty *rot22 = new wxXmlProperty(wxT("M22"), wxString::Format(wxT("%.15f"), m_transform.s.M22), rot21);
	anatomyRot->AddProperty(rot22);

	wxXmlDocument doc;
	doc.SetRoot(root);
	doc.Save(filename, 2);
}

void DatasetHelper::printTime()
{
	wxDateTime dt = wxDateTime::Now();
	printf("[%02d:%02d:%02d] ",dt.GetHour(), dt.GetMinute(), dt.GetSecond());
}

std::vector<std::vector<SelectionBox*> > DatasetHelper::getSelectionBoxes()
{
	std::vector<std::vector<SelectionBox*> > boxes;
	int countboxes = mainFrame->m_treeWidget->GetChildrenCount(mainFrame->m_tSelBoxId, false);
	wxTreeItemId id, childid;
	wxTreeItemIdValue cookie = 0;
	for (int i = 0 ; i < countboxes ; ++i)
	{
		std::vector<SelectionBox*> b;
		id = mainFrame->m_treeWidget->GetNextChild(mainFrame->m_tSelBoxId, cookie);
		if (id.IsOk()) {
			b.push_back((SelectionBox*)((MyTreeItemData*)mainFrame->m_treeWidget->GetItemData(id))->getData());
			int childboxes = mainFrame->m_treeWidget->GetChildrenCount(id);
			wxTreeItemIdValue childcookie = 0;
			for (int i = 0 ; i < childboxes ; ++i)
			{
				childid = mainFrame->m_treeWidget->GetNextChild(id, childcookie);
				if (childid.IsOk()) {
					b.push_back((SelectionBox*)((MyTreeItemData*)mainFrame->m_treeWidget->GetItemData(childid))->getData());
				}
			}
		}
		boxes.push_back(b);
	}
	return boxes;
}

void DatasetHelper::printwxT(wxString string)
{
	char* cstring;
	cstring = (char*)malloc(string.length());
	strcpy(cstring, (const char*)string.mb_str(wxConvUTF8));
	printf("%s", cstring);
}

void DatasetHelper::updateTreeDims()
{
	mainFrame->m_treeWidget->DeleteChildren(mainFrame->m_tAxialId);
	mainFrame->m_treeWidget->DeleteChildren(mainFrame->m_tCoronalId);
	mainFrame->m_treeWidget->DeleteChildren(mainFrame->m_tSagittalId);
	mainFrame->m_treeWidget->AppendItem(mainFrame->m_tAxialId, wxString::Format(wxT("%d rows"), rows));
	mainFrame->m_treeWidget->AppendItem(mainFrame->m_tAxialId, wxString::Format(wxT("%d columns"), columns));
	mainFrame->m_treeWidget->AppendItem(mainFrame->m_tCoronalId, wxString::Format(wxT("%d columns"), columns));
	mainFrame->m_treeWidget->AppendItem(mainFrame->m_tCoronalId, wxString::Format(wxT("%d frames"), frames));
	mainFrame->m_treeWidget->AppendItem(mainFrame->m_tSagittalId, wxString::Format(wxT("%d rows"), rows));
	mainFrame->m_treeWidget->AppendItem(mainFrame->m_tSagittalId, wxString::Format(wxT("%d frames"), frames));
}

void DatasetHelper::updateTreeDS(const int i)
{
	DatasetInfo* info = (DatasetInfo*)mainFrame->m_listCtrl->GetItemData(i);
	switch (info->getType())
	{
	case Head_byte:
	case Head_short:
		info->setTreeId(mainFrame->m_treeWidget->AppendItem(mainFrame->m_tDatasetId, info->getName(),
				-1, -1, new MyTreeItemData(info, Head_byte)));
		break;
	case Overlay:
		info->setTreeId(mainFrame->m_treeWidget->AppendItem(mainFrame->m_tDatasetId, info->getName(),
				-1, -1, new MyTreeItemData(info, Overlay)));
		break;
	case RGB:
		info->setTreeId(mainFrame->m_treeWidget->AppendItem(mainFrame->m_tDatasetId, info->getName(),
				-1, -1, new MyTreeItemData(info, RGB)));
		break;
	case Mesh_:
		info->setTreeId(mainFrame->m_treeWidget->AppendItem(mainFrame->m_tMeshId, info->getName(),
				-1, -1, new MyTreeItemData(info, Mesh_)));
		break;
	case Curves_:
		info->setTreeId(mainFrame->m_treeWidget->AppendItem(mainFrame->m_tFiberId, info->getName(),
				-1, -1, new MyTreeItemData(info, Curves_)));
		break;
	}
}

void DatasetHelper::treeFinished()
{
	threadsActive--;
	if (threadsActive > 0) return;
	printTime();
	printf ("tree finished\n");
	fibers_loaded = true;
	updateAllSelectionBoxes();
	scene->m_selBoxChanged = true;
	mainFrame->refreshAllGLWidgets();
}

void DatasetHelper::updateAllSelectionBoxes()
{
	std::vector<std::vector<SelectionBox*> > boxes = getSelectionBoxes();
	for (unsigned int i = 0 ; i < boxes.size() ; ++i)
		for (unsigned int j = 0 ; j < boxes[i].size() ; ++j)
			boxes[i][j]->setDirty();
}

Vector3fT DatasetHelper::mapMouse2World(int x, int y)
{
	glPushMatrix();
	glMultMatrixf(m_transform.M);
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY;

	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetIntegerv( GL_VIEWPORT, viewport );

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	GLdouble posX, posY, posZ;
	gluUnProject( winX, winY, 0, modelview, projection, viewport, &posX, &posY, &posZ);
	glPopMatrix();
	Vector3fT v = {{posX, posY, posZ}};
	return v;
}

Vector3fT DatasetHelper::mapMouse2WorldBack(int x, int y)
{
	glPushMatrix();
	glMultMatrixf(m_transform.M);
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY;

	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetIntegerv( GL_VIEWPORT, viewport );

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	GLdouble posX, posY, posZ;
	gluUnProject( winX, winY, 1, modelview, projection, viewport, &posX, &posY, &posZ);
	glPopMatrix();
	Vector3fT v = {{posX, posY, posZ}};
	return v;
}

bool DatasetHelper::GLError()
{
	lastGLError = glGetError();
	if (lastGLError == GL_NO_ERROR) return false;
	return true;
}

void DatasetHelper::printGLError(wxString function)
{
	printwxT(function);
	printf(" : ERROR: %s\n", gluErrorString(lastGLError));
}

bool DatasetHelper::loadTextFile(wxString* string, wxString filename)
{
	wxTextFile file;
	*string = wxT("");
	if (file.Open(filename))
	{
		size_t i;
		for (i = 0 ; i < file.GetLineCount() ; ++i)
		{
			*string += file.GetLine(i);
		}
		return true;
	}
	return false;
}

void DatasetHelper::createIsoSurface()
{
	// check anatomy - quit if not present
	if (!anatomy_loaded) return;
	// get top most anatomy dataset
	DatasetInfo* info;
	Anatomy* anatomy;
	bool flag = false;
	for (int i = 0 ; i < mainFrame->m_listCtrl->GetItemCount() ; ++i)
	{
		info = (DatasetInfo*)mainFrame->m_listCtrl->GetItemData(i);
		if (info->getType() == Head_byte) {
			anatomy = (Anatomy*)info;
			flag = true;
			break;
		}
	}
	if (!flag) return;

	CIsoSurface *isosurf = new CIsoSurface(this);
	isosurf->GenerateSurface(anatomy->getByteDataset(), 50, columns-1, rows-1, frames-1, 1.0, 1.0, 1.0);

	if (isosurf->IsSurfaceValid())
	{
		printf("surface is valid\n");
		isosurf->generateGeometry();
		isosurf->setName(wxT("iso surface"));

		mainFrame->m_listCtrl->InsertItem(0, wxT(""), 0);
		mainFrame->m_listCtrl->SetItem(0, 1, isosurf->getName());
		mainFrame->m_listCtrl->SetItem(0, 2, wxT("0.20"));
		mainFrame->m_listCtrl->SetItem(0, 3, wxT(""), 1);
		mainFrame->m_listCtrl->SetItemData(0, (long)isosurf);
		mainFrame->m_listCtrl->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

	}
	else
	{
		printf("surface is not valid\n");
	}
}
