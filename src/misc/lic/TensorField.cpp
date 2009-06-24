/*
 * TensorField.cpp
 *
 *  Created on: 16.11.2008
 *      Author: ralph
 */

#include "TensorField.h"
#include "../Fantom/FVector.h"

TensorField::TensorField(DatasetHelper* dh, std::vector<float>* tensorData, int order, int posDim)
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
			t.setValue(0, tensorData->at( i * 3 ));
			t.setValue(1, tensorData->at( i * 3 + 1 ));
			t.setValue(2, tensorData->at( i * 3 + 2 ));
			theField.push_back(t);
		}
	}

	else if ( m_order == 2 && m_posDim == 3 )
	{
		for ( int i = 0 ; i < m_cells ; ++i)
		{
			FTensor t(3, 2, true);
			t.setValue(0, 0, tensorData->at( i * 6 ));
			t.setValue(1, 0, tensorData->at( i * 6 + 1 ));
			t.setValue(2, 0, tensorData->at( i * 6 + 2 ));
			t.setValue(0, 1, tensorData->at( i * 6 + 1 ));
			t.setValue(1, 1, tensorData->at( i * 6 + 3 ));
			t.setValue(2, 1, tensorData->at( i * 6 + 4 ));
			t.setValue(0, 2, tensorData->at( i * 6 + 2 ));
			t.setValue(1, 2, tensorData->at( i * 6 + 4 ));
			t.setValue(2, 2, tensorData->at( i * 6 + 5 ));
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

    float xMult = x - (int)x;
    float yMult = y - (int)y;
    float zMult = z - (int)z;

    int nextX = wxMin(m_dh->columns-1, nx+1);
    int nextY = wxMin(m_dh->rows-1, ny+1);
    int nextZ = wxMin(m_dh->frames-1, nz+1);

    int xyzIndex  =  nx + ny * m_dh->columns + nz * m_dh->columns * m_dh->rows;
    int x1yzIndex = nextX + ny * m_dh->columns + nz * m_dh->columns * m_dh->rows;
    int xy1zIndex =  nx + nextY  * m_dh->columns + nz * m_dh->columns * m_dh->rows;
    int x1y1zIndex = nextX + nextY * m_dh->columns + nz * m_dh->columns * m_dh->rows;
    int xyz1Index  =  nx + ny * m_dh->columns + nextZ * m_dh->columns * m_dh->rows;
    int x1yz1Index = nextX + ny * m_dh->columns + nextZ * m_dh->columns * m_dh->rows;
    int xy1z1Index =  nx + nextY  * m_dh->columns + nextZ * m_dh->columns * m_dh->rows;
    int x1y1z1Index = nextX + nextY * m_dh->columns + nextZ * m_dh->columns * m_dh->rows;

    FTensor txyz = getTensorAtIndex(xyzIndex);
    FTensor tx1yz = getTensorAtIndex(x1yzIndex);
    FTensor txy1z = getTensorAtIndex(xy1zIndex);
    FTensor tx1y1z = getTensorAtIndex(x1y1zIndex);
    FTensor txyz1 = getTensorAtIndex(xyz1Index);
    FTensor tx1yz1 = getTensorAtIndex(x1yz1Index);
    FTensor txy1z1 = getTensorAtIndex(xy1z1Index);
    FTensor tx1y1z1 = getTensorAtIndex(x1y1z1Index);


    FMatrix matxyz = createMatrix(txyz, txyz);
    FMatrix matx1yz = createMatrix(tx1yz, tx1yz);
    FMatrix matxy1z = createMatrix(txy1z, txy1z);
    FMatrix matx1y1z = createMatrix(tx1y1z, tx1y1z);
    FMatrix matxyz1 = createMatrix(txyz1, txyz1);
    FMatrix matx1yz1 = createMatrix(tx1yz1, tx1yz1);
    FMatrix matxy1z1 = createMatrix(txy1z1, txy1z1);
    FMatrix matx1y1z1 = createMatrix(tx1y1z1, tx1y1z1);

    FMatrix i1 = matxyz * (1. - zMult) + matxyz1 * zMult;
    FMatrix i2 = matxy1z * (1. - zMult) + matxy1z1 * zMult;
    FMatrix j1 = matx1yz * (1. - zMult) + matx1yz1 * zMult;
    FMatrix j2 = matx1y1z * (1. - zMult) + matx1y1z1 * zMult;

    FMatrix w1 = i1 * (1. - yMult) + i2 * yMult;
    FMatrix w2 = j1 * (1. - yMult) + j2 * yMult;

    FMatrix matResult = w1 * (1. - xMult) + w2 * xMult;
    std::vector<FArray>evecs;
    FArray vals(0,0,0);

    matResult.getEigenSystem(vals, evecs);

    if (vals[0] >= vals[1] && vals[0] > vals[2])      return FTensor(evecs[0]);
    else if (vals[1] > vals[0] && vals[1] >= vals[2]) return FTensor(evecs[1]);
    else if (vals[2] >= vals[0] && vals[2] > vals[1]) return FTensor(evecs[2]);
    else return FTensor(evecs[0]);

}

FMatrix TensorField::createMatrix(FTensor lhs, FTensor rhs)
{
    FArray a1(lhs);
    FArray a2(rhs);
    FMatrix result(3,3);
    result(0,0) = a1[0] * a2[0];
    result(0,1) = a1[0] * a2[1];
    result(0,2) = a1[0] * a2[2];
    result(1,0) = a1[1] * a2[0];
    result(1,1) = a1[1] * a2[1];
    result(1,2) = a1[1] * a2[2];
    result(2,0) = a1[2] * a2[0];
    result(2,1) = a1[2] * a2[1];
    result(2,2) = a1[2] * a2[2];

    return result;
}
