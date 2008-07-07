#ifndef SURFACE_H_
#define SURFACE_H_

#include "datasetInfo.h"

#include <vector>
#include "Fantom/FTensor.hh"

class Surface : public DatasetInfo{

public:
	Surface(wxTreeCtrl* treeWidget, wxTreeItemId tPointId);
	~Surface() {};

	bool load(wxString filename) {return false;};
	void draw();
	void generateTexture() {};
	void generateGeometry() {};
	void initializeBuffer() {};

	void execute(std::vector< std::vector< double > > givenPoints);

private:

	FTensor getCovarianceMatrix(std::vector< std::vector< double > > points);
	void getSplineSurfaceDeBoorPoints(  std::vector< std::vector< double > > &givenPoints,
										    std::vector< std::vector< double > > &deBoorPoints,
										    int numRows, int numCols);

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

	wxTreeCtrl* m_treeWidget;
	wxTreeItemId m_tPointId;

};


#endif /*SURFACE_H_*/
