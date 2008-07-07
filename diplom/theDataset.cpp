#include "theDataset.h"
#include "Anatomy.h"
#include "curves.h"

int TheDataset::rows = 1;
int TheDataset::columns = 1;
int TheDataset::frames = 1;
bool TheDataset::dimensions_set = false;
bool TheDataset::fibers_loaded = false;

Matrix4fT TheDataset::m_transform = {  1.0f,  0.0f,  0.0f,  0.0f,
								        0.0f,  1.0f,  0.0f,  0.0f,
								        0.0f,  0.0f,  1.0f,  0.0f,
								        0.0f,  0.0f,  0.0f,  1.0f };
wxString TheDataset::lastError = wxT("");

DatasetInfo* TheDataset::load(wxString filename)
{
	// check file extension
	wxString ext = filename.AfterLast('.');
	if (ext == wxT("mesh")) {
		if (!dimensions_set) {
			lastError = wxT("no anatomy file loaded");
			return false;
		}
		Mesh *mesh = new Mesh();
		bool flag = mesh->load(filename);
		return mesh;
	}

	else if (ext == wxT("fib")) {
		if (!dimensions_set) {
			lastError = wxT("no anatomy file loaded");
			return false;
		}
		if (fibers_loaded) {
			lastError = wxT("fibers already loaded");
			return false;
		}
		Curves *curves = new Curves();
		curves->load(filename);
		return curves;
	}

	else if (ext != wxT("hea")) return NULL;

	Anatomy *anatomy = new Anatomy();
	bool flag = anatomy->load(filename);

	if (flag)
	{
		rows = anatomy->getRows();
		columns = anatomy->getColumns();
		frames = anatomy->getFrames();
		dimensions_set = true;
		return anatomy;
	}
	else
	{
		lastError = wxT("couldn't load anatomy file");
		return NULL;
	}

	if (!TheDataset::dimensions_set)
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
