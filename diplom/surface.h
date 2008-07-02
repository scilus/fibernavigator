#ifndef SURFACE_H_
#define SURFACE_H_

#include <vector>

class Surface {

public:
	Surface();
	~Surface() {};
	
	void execute();

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
	
};


#endif /*SURFACE_H_*/
