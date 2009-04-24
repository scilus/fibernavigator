/*
 * TensorField.cpp
 *
 *  Created on: 16.11.2008
 *      Author: ralph
 */

#include "TensorField.h"

TensorField::TensorField(DatasetHelper* dh, float* tensorData, bool isVector)
{
	m_dh = dh;
	m_cells = m_dh->rows * m_dh->frames * m_dh->columns;
	theField.clear();
	theField.reserve(m_cells);

	if (isVector)
	{
		for ( int i = 0 ; i < m_cells ; ++i)
		{
			FTensor t(3,1,true);
			t.setValue(0, tensorData[ i * 3 ]);
			t.setValue(1, tensorData[ i * 3 + 1 ]);
			t.setValue(2, tensorData[ i * 3 + 2 ]);
			theField.push_back(t);
		}
		m_order = 1;
		m_posDim = 3;
	}
	else
	{
		for ( int i = 0 ; i < m_cells ; ++i)
		{
			FTensor t(3,2,true);
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
		m_order = 2;
		m_posDim = 3;
	}
}

TensorField::~TensorField()
{
	// TODO Auto-generated destructor stub
}
