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



class TheDataset
{
private:
	
	
public:
	TheDataset();
	~TheDataset();
	
	int load(wxString);
	bool removeNode(int);
	
	DatasetList *m_dsList;
	
	int m_rows;
	int m_columns;
	int m_frames;
	wxString m_lastError;
};

#endif /*THEDATASET_H_*/
