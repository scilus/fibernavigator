#ifndef CURVES_H_
#define CURVES_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GL/glew.h"
#include "ArcBall.h"
#include "KdTree.h"
#include "selectionBox.h"
#include <vector>

enum CurveFileType {
	asciiCurve,
	asciiVTK,
	binaryVTK,
};

class Curves
{
public:
	Curves(int, int);
	~Curves();
	int getPointsPerLine(int);
	int getStartIndexForLine(int);
	int getLineForPoint(int);
	void setPoints(float* points) {m_pointArray = points;}; 
	float* getPoints() {return m_pointArray;};
	int getLineCount() {return m_lineCount;};
	int getPointCount() {return m_pointCount;};
	void calculateLinePointers();
	
	void resetLinesShown();
	std::vector<bool> getLinesShown(SelectionBox*);
	void updateLinesShown(std::vector<std::vector<SelectionBox*> >);
	void boxTest(int, int, int);
	
	void toggleEndianess();
	void createColorArray();
	void buildkDTree();
	void freeArrays() {delete[] m_colorArray; delete[] m_normalArray;};
	
	float *m_pointArray;
	int* m_lineArray;
	float *m_colorArray;
	float *m_normalArray;
	std::vector<bool>m_inBox;
	
	int m_lengthPoints;
	int m_lengthLines;

private:
	
	
	int m_lineCount;
	int m_pointCount;
	int *m_linePointers;
	int *m_reverse; 
	KdTree *m_kdTree;
	
	float *m_boxMin;
	float *m_boxMax;
};

#endif /*CURVES_H_*/
