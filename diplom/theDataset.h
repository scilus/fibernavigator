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
	float *m_dataHead;
	float *m_dataOverlay;
	float *m_dataRGB;
	
	bool m_headLoaded;
	bool m_overlayLoaded;
	bool m_rgbLoaded;
	
public:
	DatasetInfo *m_headInfo;
	DatasetInfo *m_overlayInfo;
	DatasetInfo *m_rgbInfo;
	
	TheDataset();
	~TheDataset(){};
	
	bool loadHead(wxString);
	bool loadOverlay(wxString);
	bool loadRGB(wxString);
	
	
	/* getter methods*/
	bool headIsLoaded() {return m_headLoaded;};
	bool overlayIsLoaded() {return m_overlayLoaded;};
	bool rgbIsLoaded() {return m_rgbLoaded;};
	
	
	float* getDataHead() {return m_dataHead;};
	float* getDataOverlay() {return m_dataOverlay;};
	float* getDataRGB() {return m_dataRGB;};
};

#endif /*THEDATASET_H_*/
