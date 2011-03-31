/*
 * TensorField.h
 *
 *  Created on: 16.11.2008
 *      Author: ralph
 */

#ifndef TENSORFIELD_H_
#define TENSORFIELD_H_

#include "../../dataset/DatasetHelper.h"
#include "../Fantom/FTensor.h"

class TensorField
{
public:
    // Constructor/Destructor
	TensorField( DatasetHelper* i_datasetHelper, std::vector< float >* i_tensorData, int i_order, int i_posDim );
    TensorField( DatasetHelper* i_datasetHelper, float*                i_tensorData, int i_order, int i_posDim );
	virtual ~TensorField();

    // Get Functions
	int getCells() 	{ return m_cells;  };
	int getOrder() 	{ return m_order;  };
	int getPosDim()	{ return m_posDim; };

	FTensor getTensorAtIndex( int i_index ) { return m_theField[i_index]; };
	FTensor getInterpolatedVector( const float i_x, const float i_y, const float i_z );

protected:
    TensorField() {};

private:
    // Functions
	FMatrix createMatrix( FTensor i_lhs, FTensor i_rhs );

    // Variables
    DatasetHelper*         m_datasetHelper;
	std::vector< FTensor > m_theField;

	int m_cells;
	int m_order;
	int m_posDim;
};

#endif /* TENSORFIELD_H_ */
