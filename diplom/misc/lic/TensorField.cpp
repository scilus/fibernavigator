/*
 * TensorField.cpp
 *
 *  Created on: 16.11.2008
 *      Author: ralph
 */

#include "TensorField.h"

TensorField::TensorField(DatasetHelper* dh, float* tensorData, int order, int posDim)
: m_order(order), m_posDim(posDim)
{
	m_dh = dh;
	m_cells = m_dh->rows * m_dh->frames * m_dh->columns;
	theField.clear();
	theField.reserve(m_cells);

	if ( m_order == 1 && m_posDim == 3 )
	{
		for ( int i = 0 ; i < m_cells ; ++i)
		{
			FTensor t(3, 1, true);
			t.setValue(0, tensorData[ i * 3 ]);
			t.setValue(1, tensorData[ i * 3 + 1 ]);
			t.setValue(2, tensorData[ i * 3 + 2 ]);
			theField.push_back(t);
		}
	}


	else if ( m_order == 2 && m_posDim == 3 )
	{
		for ( int i = 0 ; i < m_cells ; ++i)
		{
			FTensor t(3, 2, true);
			t.setValue(0, 0, tensorData[ i * 6 ]);
			t.setValue(1, 0, tensorData[ i * 6 + 1 ]);
			t.setValue(2, 0, tensorData[ i * 6 + 2 ]);
			t.setValue(0, 1, tensorData[ i * 6 + 1 ]);
			t.setValue(1, 1, tensorData[ i * 6 + 3 ]);
			t.setValue(2, 1, tensorData[ i * 6 + 4 ]);
			t.setValue(0, 2, tensorData[ i * 6 + 2 ]);
			t.setValue(1, 2, tensorData[ i * 6 + 4 ]);
			t.setValue(2, 2, tensorData[ i * 6 + 5 ]);
			theField.push_back(t);
		}
	}

}

TensorField::~TensorField()
{
	theField.clear();
}

FTensor TensorField::getInterpolatedVector(float x, float y, float z)
{
    int nx = wxMin(m_dh->columns-1, wxMax(0,(int)x));
    int ny = wxMin(m_dh->rows   -1, wxMax(0,(int)y));
    int nz = wxMin(m_dh->frames -1, wxMax(0,(int)z));
    int baseindex  =  nx + ny * m_dh->columns + nz * m_dh->columns * m_dh->rows;

    int nextX = wxMin(m_dh->columns-1, nx+1);
    int nextXIndex = nextX + ny * m_dh->columns + nz * m_dh->columns * m_dh->rows;
    int nextY = wxMin(m_dh->rows-1, ny+1);
    int nextYIndex =  nx + nextY  * m_dh->columns + nz * m_dh->columns * m_dh->rows;
    int nextZ = wxMin(m_dh->frames-1, nz+1);
    int nextZIndex =  nx + ny  * m_dh->columns + nextZ * m_dh->columns * m_dh->rows;

    float xMult = x - (int)x;
    float yMult = y - (int)y;
    float zMult = z - (int)z;

    FArray tBase = FArray(getTensorAtIndex(baseindex));
    FArray tX = FArray(getTensorAtIndex(nextXIndex));
    FArray tY = FArray(getTensorAtIndex(nextYIndex));
    FArray tZ = FArray(getTensorAtIndex(nextZIndex));


    FTensor t(3, 1, true);
    t.setValue(0, tBase[0]* xMult + tX[0] * (1.0 - xMult));
    t.setValue(1, tBase[1]* yMult + tY[1] * (1.0 - yMult));
    t.setValue(2, tBase[2]* zMult + tZ[2] * (1.0 - zMult));

    return t;
}
