//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FStreamlineOnSurfaceEuler.hh,v $
// Language:  C++
// Date:      $Date: 2003/09/23 08:20:43 $
// Author:    $Author: tricoche $
// Version:   $Revision: 1.3 $
//
//---------------------------------------------------------------------------

#ifndef __FStreamlineOnSurfaceEuler_hh
#define __FStreamlineOnSurfaceEuler_hh

#include "../Fantom/FIndex.h"
//#include "math/src/FPosition.hh"
//#include "math/src/FVector.hh"
#include "../Fantom/FTensor.h"
#include "../Fantom/FArray.h"
//#include "analysis/src/FAMSingularEdge.hh"
#include <map>
//#include "dataSet/src/FTensorField.hh"
//#include "dataSet/src/FTensorSet.hh"

#include "../../dataset/DatasetHelper.h"
#include "TensorField.h"
#include "../IsoSurface/triangleMesh.h"

class FDataSet;
class FGrid;



// Class for cell-wise integration of streamlines in tangential vector
// fields defined on surfaces. Computation is carried out on a field
// that is cell-wise constant, so streamlines are piecewise straight lines.
class FStreamlineOnSurfaceEuler
{
public:

    // structure to handle a pair of ids associated with an edge
    struct EdgeIndex
    {
	EdgeIndex();
	EdgeIndex( FIndex anId1, FIndex anId2 );
	EdgeIndex( const EdgeIndex& a );
	bool operator==( const EdgeIndex& a ) const;
	bool operator<( const EdgeIndex& a ) const;
	void display() const;

	FIndex id1, id2;
    };

    // edge geometry
    struct EdgePos
    {
	EdgePos();
	EdgePos( const FArray& pos1, const FArray& pos2 );
	EdgePos( const EdgePos& edgePos );

	FArray a, b;
    };


    // structure to handle paths over surfaces embedded in 3D space
    struct Path
    {
	// position array
	std::vector< FArray > pos;

	// corresponding cells
	std::vector< FIndex > cells;

	// corresponding cells' normal vectors
	std::vector< F::FVector > normals;

	// geodesic's length
	double length;
    };


    // structure to handle singular edges during streamline integration
    struct singSegment
    {
	// normalized vector between both segment vertices
	F::FVector dist;

	// values required for (numerically stable) crossing check
	double delta[3];
	FPosition coord;
    };


    FStreamlineOnSurfaceEuler( DatasetHelper *dh, TriangleMesh* grid );
    virtual ~FStreamlineOnSurfaceEuler();


//---------------------------------------------------------------------------
//
//   ODE integration interface
//
//---------------------------------------------------------------------------

    // 0: length reached
    // 1: left grid
    // 2: singularity or singular edge
    // 3: proceed test failed
    virtual int integrate( const FPosition& start, const FIndex& cellId,
			   bool dir, double length );

    virtual bool proceed( void );


//---------------------------------------------------------------------------
//
//   integration parameters
//
//---------------------------------------------------------------------------

    void setMaxNbCells( unsigned int maxCells );


//---------------------------------------------------------------------------
//
//   geometric control
//
//---------------------------------------------------------------------------

    void setIntermediateStepsDistance( double dist );
    void setIntermediateStepsAngle( double angle );
    void setOffset( float offset );
    void setSaveIntermediateSteps( bool save );


//---------------------------------------------------------------------------
//
//   integration results
//
//---------------------------------------------------------------------------

    const std::vector< FPosition >& getIntermediateSteps( ) const;
    const std::vector< FArray >& getIntermediateDirections( ) const;
    const std::vector< double >& getIntermediateParameters( ) const;
    unsigned int getNbSteps();
    const std::vector< FIndex >& getVisitedCells( ) const;


protected:

    // state variables

    FIndex currCell, nextCell;
    FArray currPos, nextPos;

    std::vector< FArray  > allSteps, steps;
    std::vector< F::FVector > allNormals;
    std::vector< FArray  > vecs;
    std::vector< FIndex  > visitedCells;

    double currLength;



protected:

	DatasetHelper* m_dh;
	TensorField* m_tensorField;
	TriangleMesh* m_grid;


    static const double epsilon;

    // TODO
    //shared_ptr< const FTensorField > field;
    //FTensorFieldInfo info;


    bool dir, save;

    unsigned int nbCells;
    unsigned int maxCells;

    // data structure to handle piecewise constant vector fields
    std::vector< FArray  > cell_vectors;

    std::map< EdgeIndex, EdgePos > singEdges;

    // debug
    void showPosition( const FPosition& pos, float *color ) const;
    void showVector( const FPosition& pos ) const;

    positive tSetId;

    float offset;

    bool walkThroughCell( const FArray& entry, const FIndex& cellId, const FArray& lastStep,
			  FIndex& vertId1, FIndex& vertId2,
			  double fwd, bool onEdge, FArray& exit,
			  FIndex& nextCell, bool& posRot );

    bool walkThroughVertex( const FIndex& vertId, const FArray& lastStep,
			    FIndex& vertId1, FIndex& vertId2,
			    double fwd, FArray& exit,
			    FIndex& currCell,
			    FIndex& nextCell );


    std::vector< FIndex > indices;
    std::vector< FArray > vertices;
    std::vector< FArray > edges;
    FArray basis[2], normal;
    FArray p0p1,p0p2;
    double b0, b1, b2, delta;
    double verts[3][2], point[2], vec[2], tmp[2][2], u;
    positive edge;
    positive id0, id1;
    double min;
    int firstId, secondId;
    bool valid[3];
    double cross;
    double v[3][2], direct[2];
    double exit_edge[2];
    std::vector< FIndex > neighs, neighs2;
    FArray projected;


    unsigned int localIndices[3];
    double E0[2], E1[2];

    // old school C-like computation
    void baryCoords( double& b0, double& b1, double& b2,
		     double point[], double verts[][2] );

    double crossProd( double v1[], double v2[] );

    double myCrossProd( const FArray& v1, const FArray& v2 );

    FArray lastProjection;
    FArray n_edge;

    bool isInside( const FIndex& cellId, const FArray& pos );
    bool isTensor;
    float scalarAdd;
};

//===========================================================================
#ifndef OUTLINE
#include "FStreamlineOnSurfaceEuler.icc"
#endif
//===========================================================================

#endif // __FStreamlineOnSurfaceEuler_hh

