#ifndef THEDATASET_H_
#define THEDATASET_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/laywin.h"
#include "wx/textfile.h"
#include "wx/file.h"
#include "wx/image.h"
#include "wx/wfstream.h"
#include "wx/datstrm.h"

#include "datasetInfo.h"
#include "mesh.h"

class TheDataset
{
private:
	
	
public:
	TheDataset();
	~TheDataset();
	
	DatasetInfo* load(wxString);
	Mesh* loadMesh(wxString);
		
	int m_rows;
	int m_columns;
	int m_frames;
	wxString m_lastError;
	Mesh* m_mesh;
	bool meshLoaded;
};

#endif /*THEDATASET_H_*/
