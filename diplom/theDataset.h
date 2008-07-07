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
#include "wx/txtstrm.h"

#include "datasetInfo.h"
#include "mesh.h"
#include "ArcBall.h"

class TheDataset
{
private:


public:
	//TheDataset();
	//~TheDataset();

	static DatasetInfo* load(wxString);

	static void printTime();

	static int rows;
	static int columns;
	static int frames;
	static Matrix4fT m_transform;
	static wxString lastError;
	static bool dimensions_set;
	static bool fibers_loaded;
};

#endif /*THEDATASET_H_*/
