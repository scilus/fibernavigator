/*
 * TensorField.h
 *
 *  Created on: 16.11.2008
 *      Author: ralph
 */

#ifndef TENSORFIELD_H_
#define TENSORFIELD_H_

#include "../Fantom/FTensor.h"

#include <vector>

class TensorField
{
public:
    // Constructor/Destructor
    TensorField( int columns, int rows, int frames, std::vector< float > *pTensorData, int order, int posDim );
    TensorField( int columns, int rows, int frames, float                *pTensorData, int order, int posDim );
    virtual ~TensorField();

    // Get Functions
    int getCells()     { return m_cells;  };
    int getOrder()     { return m_order;  };
    int getPosDim()    { return m_posDim; };

    FTensor getTensorAtIndex( int i_index ) { return m_theField[i_index]; };
    FTensor getInterpolatedVector( const float i_x, const float i_y, const float i_z );

protected:
    TensorField() {};

private:
    // Functions
    FMatrix createMatrix( FTensor i_lhs, FTensor i_rhs );

    // Variables
    int m_columns;
    int m_rows;
    int m_frames;
    std::vector< FTensor > m_theField;

    int m_cells;
    int m_order;
    int m_posDim;
};

#endif /* TENSORFIELD_H_ */
