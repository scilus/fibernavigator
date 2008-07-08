#include "theDataset.h"
#include "Anatomy.h"
#include "curves.h"

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

void TheDataset::printTime()
{
	wxDateTime dt = wxDateTime::Now();
	printf("[%02d:%02d:%02d] ",dt.GetHour(), dt.GetMinute(), dt.GetSecond());
}
