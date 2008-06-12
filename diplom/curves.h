#ifndef CURVES_H_
#define CURVES_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

enum CurveFileType {
	asciiCurve,
	asciiVTK,
	binaryVTK,
};

class Curves
{
public:
	Curves(int);
	~Curves();
	void setPointsPerLine(int line, int value) {m_pointsPerLine[line] = value;};
	int getPointsPerLine(int);
	void setPoints(float* points) {m_pointArray = points;}; 
	float* getPoints() {return m_pointArray;};
	int getCountLines() {return m_lineCount;};
	void toggleEndianess();
	
	float *m_pointArray;
	int* m_lineArray;
	wxUint8 *m_colorArray;
	
	int m_lengthPoints;
	int m_lengthLines;

private:
	void calculatePointsPerLine();
	
	int m_lineCount;
	int *m_pointsPerLine;
	bool m_pointsPerLineCalculated;
};

#endif /*CURVES_H_*/
