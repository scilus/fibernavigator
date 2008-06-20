#ifndef CURVES_H_
#define CURVES_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "KdTree.h"

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
	void setPointsPerLine(int line, int value) {m_pointsPerLine[line] = value;};
	int getPointsPerLine(int);
	int getStartIndexForLine(int);
	void setPoints(float* points) {m_pointArray = points;}; 
	float* getPoints() {return m_pointArray;};
	int getLineCount() {return m_lineCount;};
	int getPointCount() {return m_pointCount;};
	
	void toggleEndianess();
	void createColorArray();
	void buildkDTree();
	
	float *m_pointArray;
	int* m_lineArray;
	float *m_colorArray;
	float *m_normalArray;
	
	int m_lengthPoints;
	int m_lengthLines;

private:
	void calculatePointsPerLine();
	
	int m_lineCount;
	int m_pointCount;
	int *m_pointsPerLine;
	bool m_pointsPerLineCalculated;
	KdTree *m_kdTree;
};

#endif /*CURVES_H_*/
