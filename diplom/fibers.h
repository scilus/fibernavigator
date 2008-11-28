#ifndef CURVES_H_
#define CURVES_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "datasetInfo.h"

#include "GL/glew.h"
#include "KdTree.h"
#include "selectionBox.h"
#include <vector>
#include "Fantom/FVector.h"
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
	void draw();
	void generateTexture() {};
	void generateGeometry() {};
	void initializeBuffer();
	void drawVectors() {};
	void clean() {};
	void smooth() {};
	void activateLIC() {};

	int getPointsPerLine(int);
	int getStartIndexForLine(int);
	int getLineForPoint(int);
	void setPoints(float* points) {m_pointArray = points;};
	float* getPoints() {return m_pointArray;};
	int getLineCount() {return m_countLines;};
	int getPointCount() {return m_countPoints;};
	void calculateLinePointers();

	void resetLinesShown();
	std::vector<bool> getLinesShown(SelectionBox*);
	void updateLinesShown(std::vector<std::vector<SelectionBox*> >);
	void boxTest(int, int, int);

	bool getBarycenter(SplinePoint*);
	void barycenterTest(int, int, int);

	void toggleEndianess();
	void createColorArray();
	void resetColorArray();
	void freeArrays() {delete[] m_colorArray; delete[] m_normalArray;};

	void save(wxString filename);
	void drawFakeTubes();
	void switchNormals(bool positive);

	float *m_pointArray;
	int* m_lineArray;
	float *m_colorArray;
	float *m_normalArray;
	std::vector<bool>m_inBox;

	int m_lengthPoints;
	int m_lengthLines;

private:
	std::string intToString(int number);

	int m_countLines;
	int m_countPoints;
	int *m_linePointers;
	int *m_reverse;
	KdTree *m_kdTree;

	float *m_boxMin;
	float *m_boxMax;

	int m_count;
	Vector m_barycenter;
};

#endif /*CURVES_H_*/
