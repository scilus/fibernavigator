#ifndef THEDATASET_H_
#define THEDATASET_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/textfile.h"
#include "wx/file.h"
#include "wx/image.h"
#include "wx/wfstream.h"
#include "wx/datstrm.h"
#include "wx/txtstrm.h"

#include <vector>
#include "ArcBall.h"

#include "datasetInfo.h"
#include "selectionBox.h"

#include "mainFrame.h"
#include "theScene.h"
#include "point.h"

class TheDataset
{
private:

public:
	static DatasetInfo* load(wxString);
	static void finishLoading(DatasetInfo*);
	static bool loadSettings(wxString);
	static void save(wxString);
	static void printTime();
	static void printwxT(wxString);
	static std::vector<std::vector<SelectionBox*> > getSelectionBoxes();
	static void updateTreeDims();
	static void updateTreeDS(int);

	static int rows;
	static int columns;
	static int frames;
	static unsigned int countFibers;
	static Matrix4fT m_transform;
	static wxString lastError;
	static bool anatomy_loaded;
	static bool fibers_loaded;
	static bool surface_loaded;
	static wxString lastPath;

	static MainFrame* mainFrame;
	static Point* m_lastSelectedPoint;
	static TheScene* m_scene;
};

#endif /*THEDATASET_H_*/
