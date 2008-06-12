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
	int getPointsPerLine(int line) {return m_pointsPerLine[line];};
	void setPoints(float* points) {m_points = points;}; 
	float* getPoints() {return m_points;};
	int getCountLines() {return m_lines;};
	
	float *m_points;

private:
	int m_lines;
	int *m_pointsPerLine;
};

#endif /*CURVES_H_*/
