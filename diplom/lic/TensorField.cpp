/*
 * TensorField.cpp
 *
 *  Created on: 16.11.2008
 *      Author: ralph
 */

#include "TensorField.h"

TensorField::TensorField(DatasetHelper* dh, float* tensorData)
{
	m_dh = dh;
	m_cells = m_dh->rows * m_dh->frames * m_dh->columns;
	if ( m_dh->tensors_loaded)
	{
		theField.clear();

		for ( int i = 0 ; i < m_cells ; ++i)
		{
			FTensor t(3,2);
			t.setValue(0, tensorData[ i * 6 ]);
			t.setValue(1, tensorData[ i * 6 + 1 ]);
			t.setValue(2, tensorData[ i * 6 + 2 ]);
			t.setValue(3, tensorData[ i * 6 + 3 ]);
			t.setValue(4, tensorData[ i * 6 + 4 ]);
			t.setValue(5, tensorData[ i * 6 + 5 ]);

			theField.push_back(t);
		}
	}
	m_order = 2;
	m_posDim = 3;
}

TensorField::~TensorField()
{
	// TODO Auto-generated destructor stub
}
