#ifndef SURFACE_H_
#define SURFACE_H_

#include "datasetInfo.h"

#include <vector>
#include "Fantom/FTensor.h"
#include "Fantom/FBSplineSurface.h"
#include "DatasetHelper.h"
#include "KdTree.h"

class DatasetHelper;

class Surface : public DatasetInfo{

public:
	Surface(DatasetHelper*);
	~Surface();

	bool load(wxString filename) {return false;};
	void draw();
	void generateTexture() {};
	void generateGeometry() {};
	void initializeBuffer() {};
	void createCutTexture(int, int);

	void movePoints();

	std::vector< int >getVertices() {return m_vertices;};
	std::vector< std::vector< double > > getSplinePoints() {return m_splinePoints;};
	void setSetSampleRate(float r) {m_sampleRateT = m_sampleRateU = r; execute();};

private:
	void execute();
	FTensor getCovarianceMatrix(std::vector< std::vector< double > > points);
	void getSplineSurfaceDeBoorPoints(  std::vector< std::vector< double > > &givenPoints,
										    std::vector< std::vector< double > > &deBoorPoints,
										    int numRows, int numCols);
	FVector getNormalForTriangle(const FVector*, const FVector*, const FVector*);
	void getNormalsForVertices();

	float getXValue(int y , int z, int numPoints);
	void boxTest(int left, int right, int axis);

	double m_sampleRateNormal;
	double m_sampleRateDragging;

	double m_radius;
	double m_my;
	int m_numDeBoorRows;
	int m_numDeBoorCols;
	int m_order;
	double m_sampleRateT;
	double m_sampleRateU;
	double m_xAverage;
	double m_yAverage;
	double m_zAverage;

	std::vector< std::vector< double > > m_splinePoints;
	std::vector< int > m_vertices;
	std::vector< FVector >m_normals;
	int m_numPoints;
	float *m_boxMin;
	float *m_boxMax;
	float* m_pointArray;
	float m_xValue;
	int m_count;

	KdTree *m_kdTree;
	DatasetHelper* m_dh;
};


#endif /*SURFACE_H_*/
