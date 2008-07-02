#include "surface.h"



Surface::Surface() 
{
	m_radius = 3.0;
	m_my = 2.0;
	m_numDeBoorRows = 8;
	m_numDeBoorCols = 8;
	m_order = 4;
	m_sampleRateT = m_sampleRateU = 0.2;
}

FTensor Surface::getCovarianceMatrix(std::vector< std::vector< double > > points)
{
	FTensor result(3,2,true);      
	xAverage = 0, yAverage = 0, zAverage = 0;
  
	std::vector< std::vector< double > >::iterator pointsIt;
	for( pointsIt = points.begin(); pointsIt != points.end(); pointsIt++)
	{
		std::vector< double > dmy = *pointsIt;
		xAverage += dmy[0];
		yAverage += dmy[1];
		zAverage += dmy[2];
	}
  
	xAverage /= points.size();
	yAverage /= points.size();
	zAverage /= points.size();
  
	/*
    	 /        \
   		| XX XY XZ |
   		| YX YY YZ |
   		| ZX ZY ZZ |
    	 \        /
	 */  
	for( pointsIt = points.begin(); pointsIt != points.end(); pointsIt++)
	{
		std::vector< double > dmy = *pointsIt;
    
		result(0,0) += (dmy[0] - xAverage) * (dmy[0] - xAverage); //XX
		result(0,1) += (dmy[0] - xAverage) * (dmy[1] - yAverage); //XY
		result(0,2) += (dmy[0] - xAverage) * (dmy[2] - zAverage); //XZ
    
		result(1,1) += (dmy[1] - yAverage) * (dmy[1] - yAverage); //YY
		result(1,2) += (dmy[1] - yAverage) * (dmy[2] - zAverage); //YZ
     
		result(2,2) += (dmy[2] - zAverage) * (dmy[2] - zAverage); //ZZ 
	}
  
	result(1,0) = result(0,1);
	result(2,0) = result(0,2);
	result(2,1) = result(1,2);
  
	return result;
}

void Surface::getSplineSurfaceDeBoorPoints(std::vector< std::vector< double > > &givenPoints, std::vector< std::vector< double > > &deBoorPoints, int numRows, int numCols)
{
	double xMin = givenPoints[0][0];
	double xMax = givenPoints[0][0];
	double zMin = givenPoints[0][2];
	double zMax = givenPoints[0][2];
  
	std::vector< std::vector< double > >::iterator givenPointsIt;
  
	for( givenPointsIt = givenPoints.begin(); givenPointsIt != givenPoints.end(); givenPointsIt++)
	{
		std::vector< double > dmy = *givenPointsIt;
		if(dmy[0] < xMin)
			xMin = dmy[0];
		if(dmy[0] > xMax)
			xMax = dmy[0];
		if(dmy[2] < zMin)
			zMin = dmy[2];
		if(dmy[2] > zMax)
			zMax = dmy[2];
	}
  
	double dX = (xMax - xMin) / (numCols - 1);
	double dZ = (zMax - zMin) / (numRows - 1);
  
  
	for( int row = 0; row < numRows; row++)
		for( int col = 0; col < numCols; col++)
		{
			std::vector< double > dmy;
			double x = xMin + dX * col;
			double z = zMin + dZ * row;
			dmy.push_back(x);
      
			double y = 0;
			double numerator = 0, denominator = 0;
      
			//<local shepard with franke-little-weights>
			for( givenPointsIt = givenPoints.begin(); givenPointsIt != givenPoints.end(); givenPointsIt++)
			{
				std::vector< double > dmy1 = *givenPointsIt;
				FArray dmyArray(dmy1);
				dmyArray[1] = 0;
				FArray thisPoint(x,0,z);
        
				double xi; //greek alphabet
        
				if( thisPoint.distance(dmyArray) < radius)
					xi = 1 - thisPoint.distance(dmyArray) / radius;
				else 
					xi = 0;
        
				numerator += (pow(xi, my) * dmy1[1]);
				denominator += (pow(xi, my));
			}
			if( denominator == 0)
				y = 0;
			else
				y = numerator / denominator;
			//</local shepard with franke-little-weights>
			dmy.push_back(y);
			dmy.push_back(z);
      
			deBoorPoints.push_back(dmy);
		}
  return;
}

void Surface::execute ()
{
	try
	{
    
		std::vector< std::vector< double > > givenPoints;
		std::vector< std::vector< double > > deBoorPoints;
		std::vector< std::vector< double > > splinePoints;
    
		//loadPointsFromFile("/u/oesterling/shk/FAnToM/visAlgos/Testy/pointsSurface", givenPoints);
		loadPointsFromFile(fileName.c_str(), givenPoints);
    
		FTensor myTensor = getCovarianceMatrix(givenPoints);
    
		FArray eigenValues(3);
		FArray eigenVectors[3];      
		eigenVectors[0] = FArray(3);
		eigenVectors[1] = FArray(3);
		eigenVectors[2] = FArray(3);
    
		myTensor.getEigenSystem(eigenValues, eigenVectors);
    
		eigenVectors[0].normalize();
		eigenVectors[1].normalize();
		eigenVectors[2].normalize();
    
		FTensor::sortEigenvectors(eigenValues, eigenVectors);
    
		FMatrix transMatrix = FMatrix(3,3);
		transMatrix(0,0) = eigenVectors[1][0];
		transMatrix(0,1) = eigenVectors[1][1];
		transMatrix(0,2) = eigenVectors[1][2];
    
		transMatrix(1,0) = eigenVectors[2][0];
		transMatrix(1,1) = eigenVectors[2][1];
		transMatrix(1,2) = eigenVectors[2][2];
    
		transMatrix(2,0) = eigenVectors[0][0];
		transMatrix(2,1) = eigenVectors[0][1];
		transMatrix(2,2) = eigenVectors[0][2];

		std::vector< std::vector< double > >::iterator pointsIt;
        
		//translate and orientate given points to origin
		for( pointsIt = givenPoints.begin(); pointsIt != givenPoints.end(); pointsIt++)
		{
			(*pointsIt)[0] -= xAverage;
			(*pointsIt)[1] -= yAverage;
			(*pointsIt)[2] -= zAverage;
      
			FArray dmy(*pointsIt);
      
			FVector result = transMatrix * dmy;
			(*pointsIt)[0] = result[0];
			(*pointsIt)[1] = result[1];
			(*pointsIt)[2] = result[2];
		}
    
		//get de Boor points using shepard's method
		getSplineSurfaceDeBoorPoints(givenPoints, deBoorPoints, numDeBoorRows, numDeBoorCols);
      
		//translate and orientate de Boor points back
		transMatrix.invert();
		for( pointsIt = deBoorPoints.begin(); pointsIt != deBoorPoints.end(); pointsIt++)
		{
			FArray dmy(*pointsIt);
      
			FVector result = transMatrix * dmy;
			(*pointsIt)[0] = result[0];
			(*pointsIt)[1] = result[1];
			(*pointsIt)[2] = result[2];
			
			(*pointsIt)[0] += xAverage;
			(*pointsIt)[1] += yAverage;
			(*pointsIt)[2] += zAverage;
		}

		FBSplineSurface splineSurface(order, order, deBoorPoints, numDeBoorCols, numDeBoorRows);

		splineSurface.samplePoints(splinePoints, sampleRateT, sampleRateU);
            
		std::vector< double > positions;
		for( std::vector< std::vector< double > >::iterator posIt = splinePoints.begin(); posIt != splinePoints.end(); posIt++)
		{
			positions.push_back((*posIt)[0]);
			positions.push_back((*posIt)[1]);
			positions.push_back((*posIt)[2]);
		}
    
		shared_ptr< FPositionSet > positionSet( new FPositionSet3DArbitrary( positions ));
		std::vector< FIndex > vertices;
    
		int renderpointsPerCol = splineSurface.getNumSamplePointsU();
		int renderpointsPerRow = splineSurface.getNumSamplePointsT();
    
		for(int z = 0; z < renderpointsPerCol - 1; z++)
		{
			for(int x = 0; x < renderpointsPerRow - 1; x++)
			{
				vertices.push_back(z * renderpointsPerCol + x);
				vertices.push_back(z * renderpointsPerCol + x + 1);
				vertices.push_back((z+1) * renderpointsPerCol + x);
        
				vertices.push_back((z+1) * renderpointsPerCol + x);
				vertices.push_back(z * renderpointsPerCol + x + 1);
				vertices.push_back((z+1) * renderpointsPerCol + x + 1);
			}
		}
    
		shared_ptr< FCellDefinitions3DTriangulation > cellDefinitions(new FCellDefinitions3DTriangulation( splinePoints.size(), "grid", vertices, true ));
    
		shared_ptr<FGrid> grid = FGrid::constructSuitableGrid( "grid", cellDefinitions, positionSet );
    
		shared_ptr< FTensorSet > tensorSet(new  FTensorSet(1, 0, shared_ptr<FDummyArray>( new FDummyArray(0., splinePoints.size())), "surface tensor set"));
		shared_ptr<FTensorField> tensorField( new FTensorField("surface", tensorSet, grid, false));
    
		theDataSet->addTensorField( tensorField );
    
	}
	CATCH_N_RETHROW( FException );
}
