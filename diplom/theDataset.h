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

#include "AnatomyHelper.h"

class TheDataset
{
private:

public:
	static bool load(int index = 0, wxString filename = wxT(""));
	static void finishLoading(DatasetInfo*);
	static bool loadSettings(wxString);
	static bool loadTextFile(wxString*, wxString);
	static void save(wxString);

	static std::vector<std::vector<SelectionBox*> > getSelectionBoxes();
	static void updateAllSelectionBoxes();
	static Vector3fT mapMouse2World(int, int);
	static void updateTreeDims();
	static void updateTreeDS(int);

	/*
	 * Called from MainFrame when a kdTree thread signals it's finished
	 */
	static void treeFinished();

	/*
	 * Helper functions
	 */
	static void printTime();
	static void printwxT(wxString);
	/*
	 * Check for GL error
	 */
	static bool GLError();
	static void printGLError(wxString function = wxT(""));

	static void updateView(float x, float y, float z) {
		xSlize = x;
		ySlize = y;
		zSlize = z;
	};


	static int rows;
	static int columns;
	static int frames;
	static bool useVBO;
	static unsigned int countFibers;
	static Matrix4fT m_transform;
	static wxString lastError;
	static GLenum lastGLError;
	static bool anatomy_loaded;
	static bool fibers_loaded;
	static bool surface_loaded;
	static bool surface_isDirty;
	static wxString lastPath;
	static int threadsActive;

	static MainFrame* mainFrame;
	static Point* m_lastSelectedPoint;
	static TheScene* m_scene;
	static AnatomyHelper* anatomyHelper;

	static bool showSagittal;
	static bool showCoronal;
	static bool showAxial;
	static float xSlize;
	static float ySlize;
	static float zSlize;
	static int quadrant;


};

#define ID_KDTREE_FINISHED	50

#endif /*THEDATASET_H_*/
