#ifndef THEDATASET_H_
#define THEDATASET_H_

#include "wx/wx.h"
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
	float *m_headData;
	bool is_headLoaded;
	
public:
	DatasetInfo *m_headInfo;
	
	TheDataset();
	~TheDataset(){};
	
	bool loadHead(wxString);
	
	
	/* getter methods*/
	
	float* getHeadData() {return m_headData;};
};

#endif /*THEDATASET_H_*/
