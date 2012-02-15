/*
 * TensorField.cpp
 *
 *  Created on: 16.11.2008
 *      Author: ralph
 */

#include "TensorField.h"

#include "../Fantom/FVector.h"
#include "../../dataset/DatasetManager.h"

#include <algorithm>
#include <vector>
using std::vector;

///////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////
TensorField::TensorField( int columns, int rows, int frames, vector< float > *pTensorData, int order, int posDim )
:   m_columns( columns ),
    m_rows( rows ),
    m_frames( frames ),
    m_order( order ), 
    m_posDim( posDim )
{
    m_cells = m_columns * m_rows * m_frames;

    m_theField.clear();
    m_theField.reserve( m_cells );

    if( m_order == 1 && m_posDim == 3 )
    {
        for( int i = 0 ; i < m_cells ; ++i )
        {
            FTensor l_tensor( 3, 1, true );

            l_tensor.setValue( 0, pTensorData->at( i * 3     ) );
            l_tensor.setValue( 1, pTensorData->at( i * 3 + 1 ) );
            l_tensor.setValue( 2, pTensorData->at( i * 3 + 2 ) );

            m_theField.push_back( l_tensor );
        }
    }
    else if( m_order == 2 && m_posDim == 3 )
    {
        for( int i = 0 ; i < m_cells ; ++i )
        {
            FTensor l_tensor( 3, 2, true );

            l_tensor.setValue( 0, 0, pTensorData->at( i * 6     ) );
            l_tensor.setValue( 1, 0, pTensorData->at( i * 6 + 1 ) );
            l_tensor.setValue( 2, 0, pTensorData->at( i * 6 + 2 ) );
            l_tensor.setValue( 0, 1, pTensorData->at( i * 6 + 1 ) );
            l_tensor.setValue( 1, 1, pTensorData->at( i * 6 + 3 ) );
            l_tensor.setValue( 2, 1, pTensorData->at( i * 6 + 4 ) );
            l_tensor.setValue( 0, 2, pTensorData->at( i * 6 + 2 ) );
            l_tensor.setValue( 1, 2, pTensorData->at( i * 6 + 4 ) );
            l_tensor.setValue( 2, 2, pTensorData->at( i * 6 + 5 ) );

            m_theField.push_back( l_tensor );
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////
TensorField::TensorField( int columns, int rows, int frames, float *pTensorData, int order, int posDim )
:   m_columns( columns ),
    m_rows( rows ),
    m_frames( frames ),
    m_order( order ), 
    m_posDim( posDim )
{
    m_cells = m_columns * m_rows * m_frames;

    m_theField.clear();
    m_theField.reserve( m_cells );

    if( m_order == 1 && m_posDim == 3 )
    {
        for( int i = 0 ; i < m_cells ; ++i )
        {
            FTensor l_tensor( 3, 1, true );

            l_tensor.setValue( 0, pTensorData[ i * 3     ] );
            l_tensor.setValue( 1, pTensorData[ i * 3 + 1 ] );
            l_tensor.setValue( 2, pTensorData[ i * 3 + 2 ] );

            m_theField.push_back( l_tensor );
        }
    }
    else if( m_order == 2 && m_posDim == 3 )
    {
        for( int i = 0 ; i < m_cells ; ++i )
        {
            FTensor l_tensor( 3, 2, true );

            l_tensor.setValue( 0, 0, pTensorData[  i * 6     ] );
            l_tensor.setValue( 1, 0, pTensorData[  i * 6 + 1 ] );
            l_tensor.setValue( 2, 0, pTensorData[  i * 6 + 2 ] );
            l_tensor.setValue( 0, 1, pTensorData[  i * 6 + 1 ] );
            l_tensor.setValue( 1, 1, pTensorData[  i * 6 + 3 ] );
            l_tensor.setValue( 2, 1, pTensorData[  i * 6 + 4 ] );
            l_tensor.setValue( 0, 2, pTensorData[  i * 6 + 2 ] );
            l_tensor.setValue( 1, 2, pTensorData[  i * 6 + 4 ] );
            l_tensor.setValue( 2, 2, pTensorData[  i * 6 + 5 ] );

            m_theField.push_back( l_tensor );
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////
TensorField::~TensorField()
{
    // This is useless, the vector will be deleted no need to clear it ...
    //m_theField.clear();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
FTensor TensorField::getInterpolatedVector( float i_x, float i_y, float i_z )
{
    using std::min;
    using std::max;

    int columns = DatasetManager::getInstance()->getColumns();
    int rows    = DatasetManager::getInstance()->getRows();
    int frames  = DatasetManager::getInstance()->getFrames();

    int nx = std::min( columns - 1, std::max( 0, (int)i_x ) );
    int ny = std::min( rows    - 1, std::max( 0, (int)i_y ) );
    int nz = std::min( frames  - 1, std::max( 0, (int)i_z ) );

    float xMult = i_x - (int)i_x;
    float yMult = i_y - (int)i_y;
    float zMult = i_z - (int)i_z;

    int nextX = std::min( columns - 1, nx + 1 );
    int nextY = std::min( rows    - 1, ny + 1 );
    int nextZ = std::min( frames  - 1, nz + 1 );

    int xyzIndex    = nx    + ny    * columns + nz    * columns * rows;
    int x1yzIndex   = nextX + ny    * columns + nz    * columns * rows;
    int xy1zIndex   = nx    + nextY * columns + nz    * columns * rows;
    int x1y1zIndex  = nextX + nextY * columns + nz    * columns * rows;
    int xyz1Index   = nx    + ny    * columns + nextZ * columns * rows;
    int x1yz1Index  = nextX + ny    * columns + nextZ * columns * rows;
    int xy1z1Index  = nx    + nextY * columns + nextZ * columns * rows;
    int x1y1z1Index = nextX + nextY * columns + nextZ * columns * rows;

    FTensor txyz    = getTensorAtIndex( xyzIndex    );
    FTensor tx1yz   = getTensorAtIndex( x1yzIndex   );
    FTensor txy1z   = getTensorAtIndex( xy1zIndex   );
    FTensor tx1y1z  = getTensorAtIndex( x1y1zIndex  );
    FTensor txyz1   = getTensorAtIndex( xyz1Index   );
    FTensor tx1yz1  = getTensorAtIndex( x1yz1Index  );
    FTensor txy1z1  = getTensorAtIndex( xy1z1Index  );
    FTensor tx1y1z1 = getTensorAtIndex( x1y1z1Index );

    FMatrix matxyz    = createMatrix( txyz,    txyz    );
    FMatrix matx1yz   = createMatrix( tx1yz,   tx1yz   );
    FMatrix matxy1z   = createMatrix( txy1z,   txy1z   );
    FMatrix matx1y1z  = createMatrix( tx1y1z,  tx1y1z  );
    FMatrix matxyz1   = createMatrix( txyz1,   txyz1   );
    FMatrix matx1yz1  = createMatrix( tx1yz1,  tx1yz1  );
    FMatrix matxy1z1  = createMatrix( txy1z1,  txy1z1  );
    FMatrix matx1y1z1 = createMatrix( tx1y1z1, tx1y1z1 );

    FMatrix i1 = matxyz   * ( 1. - zMult ) + matxyz1   * zMult;
    FMatrix i2 = matxy1z  * ( 1. - zMult ) + matxy1z1  * zMult;
    FMatrix j1 = matx1yz  * ( 1. - zMult ) + matx1yz1  * zMult;
    FMatrix j2 = matx1y1z * ( 1. - zMult ) + matx1y1z1 * zMult;

    FMatrix w1 = i1 * ( 1. - yMult ) + i2 * yMult;
    FMatrix w2 = j1 * ( 1. - yMult ) + j2 * yMult;

    FMatrix matResult = w1 * ( 1. - xMult ) + w2 * xMult;

    std::vector< FArray > evecs;
    FArray vals( 0, 0, 0 );

    matResult.getEigenSystem( vals, evecs );

    if( vals[0] >= vals[1] && vals[0] > vals[2] )
        return FTensor( evecs[0] );
    else if( vals[1] > vals[0] && vals[1] >= vals[2] )
        return FTensor( evecs[1] );
    else if( vals[2] >= vals[0] && vals[2] > vals[1] )
        return FTensor( evecs[2] );
    else
        return FTensor( evecs[0] );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
FMatrix TensorField::createMatrix( FTensor i_lhs, FTensor l_rhs )
{
    FArray l_a1( i_lhs );
    FArray l_a2( l_rhs );

    FMatrix l_result( 3, 3 );

    l_result( 0, 0 ) = l_a1[0] * l_a2[0];
    l_result( 0, 1 ) = l_a1[0] * l_a2[1];
    l_result( 0, 2 ) = l_a1[0] * l_a2[2];
    l_result( 1, 0 ) = l_a1[1] * l_a2[0];
    l_result( 1, 1 ) = l_a1[1] * l_a2[1];
    l_result( 1, 2 ) = l_a1[1] * l_a2[2];
    l_result( 2, 0 ) = l_a1[2] * l_a2[0];
    l_result( 2, 1 ) = l_a1[2] * l_a2[1];
    l_result( 2, 2 ) = l_a1[2] * l_a2[2];

    return l_result;
}
