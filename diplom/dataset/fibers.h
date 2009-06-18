#ifndef CURVES_H_
#define CURVES_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "datasetInfo.h"

#include "GL/glew.h"
#include "KdTree.h"
#include "../gui/selectionBox.h"
#include <vector>
#include "../misc/Fantom/FVector.h"
#include "splinePoint.h"

enum FiberFileType {
	asciiFiber,
	asciiVTK,
	binaryVTK,
};

class Fibers : public DatasetInfo , public wxTreeItemData
{
public:
	Fibers(DatasetHelper*);
	~Fibers();

	bool load(wxString filename);
	bool loadVTK(wxString filename);
	bool loadPTK(wxString filename);
	bool loadCamino(wxString filename);
	void save(wxString filename);
	void draw();
	void generateTexture() {};
	void generateGeometry() {};
	void initializeBuffer();
	void drawVectors() {};
	void clean() {};
	void smooth() {};
	void activateLIC() {};
	GLuint getGLuint() {return 0;};

	int getPointsPerLine(int);
	int getStartIndexForLine(int);
	int getLineForPoint(int);

	float getPointValue(int index);
	int getLineCount();
	int getPointCount();

	bool getBarycenter(SplinePoint*);

    void updateLinesShown();
    void resetColorArray();
    void switchNormals(bool positive);
    bool isSelected(int fiber);


private:
	std::string intToString(int number);
    void calculateLinePointers();
    void resetLinesShown();
    std::vector<bool> getLinesShown(SelectionBox*);
    void boxTest(int, int, int);
    void barycenterTest(int, int, int);
    void toggleEndianess();
    void createColorArray(bool colorsLoadedFromFile);
    void freeArrays();
    void drawFakeTubes();
    void drawSortedLines();

	bool m_isInitialized;
	bool m_normalsPositive;
	int m_countLines;
	int m_countPoints;
    int m_count;

    std::vector<float> m_pointArray;
    std::vector<float> m_colorArray;
    std::vector<float> m_normalArray;

    std::vector<int> m_lineArray;
	std::vector<int> m_linePointers;
	std::vector<int> m_reverse;
	std::vector<bool> m_selected;
	std::vector<float> m_boxMin;
	std::vector<float> m_boxMax;

	KdTree *m_kdTree;
	Vector m_barycenter;
};

#endif /*CURVES_H_*/
