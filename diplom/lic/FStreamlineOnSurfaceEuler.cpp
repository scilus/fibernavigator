//---------------------------------------------------------------------------
//
// Project:   FAnToM
// Module:    $RCSfile: FStreamlineOnSurfaceEuler.cc,v $
// Language:  C++
// Date:      $Date: 2003/09/30 16:05:04 $
// Author:    $Author: garth $
// Version:   $Revision: 1.8 $
//
//---------------------------------------------------------------------------

#include "FStreamlineOnSurfaceEuler.h"

#include <set>
#include <iostream>

FArray dirty;

// #define __GRAPHICS__
// #define __DEBUG__

#ifdef __GRAPHICS__
#include "Fge/src/FgePrimitiveList.hh"
#include "Fge/src/FgePrimitiveHandler.hh"
#include "Fge/src/FgePoints.hh"
#include "Fge/src/FgeLines.hh"
#include "Fge/src/Fge_config.hh"
#endif

#ifdef OUTLINE
#include "FStreamlineOnSurfaceEuler.icc"
#endif

//---------------------------------------------------------------------------

const double FStreamlineOnSurfaceEuler::epsilon = 1.0e-6;
const double epsilon2 = 1.0e-8;
F::FVector lastDirectionVector(3);
//---------------------------------------------------------------------------
#if 0
FStreamlineOnSurfaceEuler::
FStreamlineOnSurfaceEuler( DatasetHelper* dh )
/*    : allSteps(), steps(), allNormals(), vecs(), field( aField ),
      maxCells(10000), singEdges(), offset(1.0e-5),
      vertices(3), edges(3), p0p1(3), p0p2(3)*/
{
	m_dh = dh;
	m_tensorField = new TensorField(m_dh);
    try
    {
    	nbCells = m_tensorField->getCells();
    	int tensorOrder = m_tensorField->getOrder();
    	int posDim = m_tensorField->getPosDim();

#ifdef __GRAPHICS__
	FgePrimitiveHandler *primitive_handler =
	    FgePrimitiveHandler::getPrimitiveHandler();
	FgePrimitiveList *toplist;
	unsigned int seqno;
	char toplist_name[80];
	seqno = primitive_handler->newAlgoRun();
	sprintf(toplist_name, "Streamline on Surface Euler[%u]", seqno);
	toplist = new FgePrimitiveList(toplist_name);
	FgeLines *lines = new FgeLines();
	lines->setNewColor( 1., 1., 1. );
	lines->setNewLineWidth( 1. );
	FgePoints *pts = new FgePoints();
	pts->setNewColor( 0., 0., 1. );
	pts->setNewSize( 3. );
#endif

	//field->getConvenienceInfo( info );
	std::vector< FIndex  > indices;
	std::vector< FTensor > vectors(3);
	std::vector< FArray  > vertices(3);
	cell_vectors.resize( nbCells );
	FArray normal, tensor;
	FArray mean;
    FTensor tensortensor;

	//
	bool isScalar = ( tensorOrder == 0 );
    this->isTensor = ( tensorOrder == 2 );
	is2Din3D = ( posDim == 3 );
	if ( is2Din3D )
	    tensor = FArray(3);
	else
	    tensor = FArray(2);

	FArray e0, e1, p0p2, p0, p1(2), p2(2);
	p1(1) = 0.;
	double delta;

	// associate every cell with a constant vector value
	if ( info.tensorSet->getNbTensors() == info.nbCells )
	{

	    FTensor dummy;
	    for ( positive i=0 ; i<info.nbCells ; i++ )
	    {
		info.tensorSet->getTensor( dummy, FIndex(i) );
		cell_vectors[i] = FArray( dummy );
	    }
	}
	else
	{
	    // loop over cells to compute cell-based values
	    for ( positive i=0 ; i<info.nbCells ; i++ )
	    {

        if ( isTensor )
        {
   		    // tensor case: take mean tensor and compute eigenvector
		    info.cellDef->getCellVerticesIndices( FIndex(i), indices );
		    info.tensorSet->getTensor( vectors[0], indices[0] );
		    info.posSet->getPosition( vertices[0], indices[0] );

		    // loop over cell vertices: get tensors and positions
		    for ( positive j=1 ; j<3 ; j++ )
		    {
			info.tensorSet->getTensor( vectors[j], indices[j] );
			info.posSet->getPosition( vertices[j], indices[j] );
		    }

		    // mean 3D value
		    if ( is2Din3D )
		    {
              // simple, but works for tensors, too
              tensortensor = vectors[0] + vectors[1] + vectors[2];
			// normal
			normal = crossProduct( vertices[1] - vertices[0],
					       vertices[2] - vertices[0] );
			normal.normalize();

              F::FVector vals(3);
              F::FVector v[3];
              v[0].resize(3);v[1].resize(3); v[2].resize(3);
              tensortensor.getEigenSystem( vals, v );
              FTensor::sortEigenvectors( vals, v );
              tensor = v[0]; // assign largest eigenvector
			// project mean tensor onto cell plane
			tensor = tensor - ( tensor * normal ) * normal;
		    }
		    else
		    {
              //eassert( false );
#if 0
			tensor(0) = vectors[0](0) + vectors[1](0) +
			    vectors[2](0);
			tensor(1) = vectors[0](1) + vectors[1](1) +
			    vectors[2](1);
#endif
		    }
		    tensor.normalize();

		    // set new value to corresponding cell
		    cell_vectors[i] = tensor;
	     }
        else if ( !isScalar )
		{
		    // vector case: just take mean vector
		    info.cellDef->getCellVerticesIndices( FIndex(i), indices );
		    info.tensorSet->getTensor( vectors[0], indices[0] );
		    info.posSet->getPosition( vertices[0], indices[0] );

		    // loop over cell vertices: get tensors and positions
		    for ( positive j=1 ; j<3 ; j++ )
		    {
			info.tensorSet->getTensor( vectors[j], indices[j] );
			info.posSet->getPosition( vertices[j], indices[j] );
		    }

		    // mean 3D value
		    if ( is2Din3D )
		    {
              // simple, but works for tensors, too
              tensor = vectors[0] + vectors[1] + vectors[2];
              /*
			tensor(0)  = vectors[0](0) + vectors[1](0) +
			    vectors[2](0);
			tensor(1)  = vectors[0](1) + vectors[1](1) +
			    vectors[2](1);
			tensor(2)  = vectors[0](2) + vectors[1](2) +
			    vectors[2](2);
			tensor    *= 1./3; // not needed as we normalize later
                */
			// normal
			normal = crossProduct( vertices[1] - vertices[0],
					       vertices[2] - vertices[0] );
			normal.normalize();

			// project mean tensor onto cell plane
			tensor = tensor - ( tensor * normal ) * normal;
		    }
		    else
		    {
			tensor(0) = vectors[0](0) + vectors[1](0) +
			    vectors[2](0);
			tensor(1) = vectors[0](1) + vectors[1](1) +
			    vectors[2](1);
		    }
		    tensor.normalize();

		    // set new value to corresponding cell
		    cell_vectors[i] = tensor;
		}
		else
		{
		    // scalar case: take cell-wise constant gradient vector
		    info.cellDef->getCellVerticesIndices( FIndex(i), indices );
		    for ( positive j=0 ; j<3 ; j++ )
		    {
			info.tensorSet->getTensor( vectors[j], indices[j] );
			info.posSet->getPosition( vertices[j], indices[j] );
		    }

		    // create local basis
		    if ( is2Din3D )
		    {
			e0 = vertices[1] - vertices[0];
			p1(0) = e0.norm();
			p0p2 = vertices[2] - vertices[0];
			normal = crossProduct( e0, p0p2 );
			e0 *= 1. / p1(0);
			normal.normalize();
			e1 = crossProduct( normal, e0 );
		    }
		    else
		    {
			e0 = vertices[1] - vertices[0];
			p1(0) = e0.norm();
			e0 *= 1. / p1(0);
			e1 = FArray( -e0(1), e0(0) );
			p0p2 = vertices[2] - vertices[0];
		    }

		    p2(0) = p0p2 * e0;
		    p2(1) = p0p2 * e1;
		    delta = myCrossProd( p1, p2 );
		    delta = 1./ delta;
		    cell_vectors[i] = delta *
			( ( vectors[1]()-vectors[0]() ) * ( p2(1)*e0 - p2(0)*e1 ) +
			  ( vectors[2]()-vectors[0]() ) * ( p1(0)*e1 - p1(1)*e0 )
			    );
		}
	    }
	}

#ifdef __GRAPHICS__
	// display them for debugging purposes
	FgeLines *_lines = new FgeLines();
	_lines->setNewColor( 1., 0., 0. );
	_lines->setNewLineWidth( 1. );
	double length;
	for ( positive i=0 ; i<info.nbCells ; i++ )
	{
	    info.cellDef->getCellVerticesIndices( FIndex(i), indices );
	    info.posSet->getPosition( p0, indices[0] );
	    info.posSet->getPosition( p1, indices[1] );
	    info.posSet->getPosition( p2, indices[2] );
	    p0p1 = p1 - p0;
	    length = .2 * p0p1.norm() / cell_vectors[i].norm();
	    p0 = 1./3 * ( p0 + p1 + p2 );
	    _lines->setNewVertex( p0(0) - length*cell_vectors[i](0),
				  p0(1) - length*cell_vectors[i](1),
				  p0(2) - length*cell_vectors[i](2) );
	    _lines->setNewVertex( p0(0) + length*cell_vectors[i](0),
				  p0(1) + length*cell_vectors[i](1),
				  p0(2) + length*cell_vectors[i](2) );
	}
	Fge_gfxresource_id listId = primitive_handler->
	    commitAndNamePrimitive( FGE_GFXRESOURCEID_ROOTLIST,
				    toplist);

	// loop over cells to check edges for attachment / separation
	std::vector< FIndex > neighs;
	F::FVector p0p1;
	const FNeighborhoodData *neighData = info.cellDef->getNeighborhoodData();
	EdgeIndex edgeId;
	FgeLines *seplines = new FgeLines();
	FgeLines *attlines = new FgeLines();
	FgeLines *deglines = new FgeLines();
	seplines->setNewColor( 1., 0., 0. );
	attlines->setNewColor( 0., 0., 1. );
	deglines->setNewColor( 1., 1., 0. );
	seplines->setNewLineWidth( 1.5 );
	attlines->setNewLineWidth( 1.5 );
	deglines->setNewLineWidth( 2. );
	positive k;

	for ( positive i=0 ; i<info.nbCells ; i++ )
	{
	    info.cellDef->getCellVerticesIndices( FIndex(i), indices );
	    // loop over cell edges
	    for ( positive j=0 ; j<3 ; j++ )
	    {
		neighData->getCellEdgeNeighbors( FIndex(i), FIndex(j),
						 neighs );
		if ( neighs.size() )
		{
		    // this is not a boundary cell
		    edgeId = EdgeIndex( FIndex(i), neighs[0] );
		    if ( singEdges.find( edgeId ) == singEdges.end() )
		    {
			// current edge has not been processed so far
			// get egde vertices
			info.posSet->getPosition( p0, indices[j] );
			info.posSet->getPosition( p1, indices[(j+1)%3] );
			p0p1 = p1 - p0;
			// cross product test
			k = neighs[0].getIndex();
			if ( ( is2Din3D &&
			       ( crossProduct( p0p1, cell_vectors[i] ) *
				 crossProduct( p0p1, cell_vectors[k] ) < 0. ) )
			     ||
			     ( !is2Din3D &&
			       ( myCrossProd( p0p1, cell_vectors[i] ) *
				 myCrossProd( p0p1, cell_vectors[k] ) < 0. ) ) )
			{
			    p0p1.normalize();
			    info.posSet->getPosition( p2, indices[(j+2)%3] );
			    mean = 1./3. * ( p0 + p1 + p2 );

			    // check for degeneracy
			    if ( ( cell_vectors[i]*p0p1 )*( cell_vectors[k]*p0p1 ) < 0. )
			    {
				deglines->setNewVertex( p0(0), p0(1),
							is2Din3D ? p0(2) : 0. );
				deglines->setNewVertex( p1(0), p1(1),
							is2Din3D ? p1(2) : 0. );
			    }
			    else if ( (p0-mean) * cell_vectors[i] -
				      ( (p0-mean) * p0p1 ) *
				      ( p0p1 * cell_vectors[i] ) > 0. )
			    {
				attlines->setNewVertex( p0(0), p0(1),
							is2Din3D ? p0(2) : 0. );
				attlines->setNewVertex( p1(0), p1(1),
							is2Din3D ? p1(2) : 0. );
			    }
			    else
			    {
				seplines->setNewVertex( p0(0), p0(1),
							is2Din3D ? p0(2) : 0. );
				seplines->setNewVertex( p1(0), p1(1),
							is2Din3D ? p1(2) : 0. );
			    }
			}
			// edge has been processed
			if ( p0p1 * cell_vectors[i] > 0. &&
			     p0p1 * cell_vectors[k] > 0. )
			    singEdges[edgeId] = EdgePos( p0, p1 );
			else if ( p0p1 * cell_vectors[i] < 0. &&
				  p0p1 * cell_vectors[k] < 0. )
			    singEdges[edgeId] = EdgePos( p1, p0 );
			else
			{
#ifdef __DEBUG__
			    cerr << "FStreamlineOnSurfaceEuler WARNING: degenerate singular edge" << endl;
#endif
			    // edge is set to a vector with zero length
			    singEdges[edgeId] = EdgePos( p0, p0 );
			}
		    }
		}
	    }
	}
	primitive_handler->commitPrimitive(listId, seplines);
	primitive_handler->commitPrimitive(listId, attlines);
	primitive_handler->commitPrimitive(listId, deglines);
	primitive_handler->commitPrimitive(listId, _lines );
#endif

#ifdef __DEBUG__
	cout << "FStreamlineOnSurfaceEuler: exit" << endl
	     << "info.nbCells = " << (int) info.nbCells << endl;
#endif
    }
    CATCH_N_RETHROW( FException );
}

//---------------------------------------------------------------------------

FStreamlineOnSurfaceEuler::~FStreamlineOnSurfaceEuler()
{
}

//---------------------------------------------------------------------------

int FStreamlineOnSurfaceEuler::integrate( const FPosition& start,
					  const FIndex& cellId, bool dir,
					  double length )
{

#ifdef __DEBUG__
    cout << endl << endl << "integrate: start=" << start << endl;
#endif

    try
    {
	steps.clear();
	vecs.clear();
	visitedCells.clear();

	double fwd = dir ? 1. : -1.;
	double stepLength;
	bool succeeded, posRot, onEdge;
	FIndex first, second;

	currLength = 0.;

	valid[0] = valid[1] = valid[2] = true;

	// --------------------------------------------------
	// initialize
	// --------------------------------------------------

	currPos  = start;
	currCell = cellId;
	steps.push_back( start );
	vecs.push_back( cell_vectors[cellId] );
	visitedCells.push_back( cellId );
	succeeded = walkThroughCell( currPos, currCell, FArray(is2Din3D ? 3: 2), first, second,
				     fwd, false, nextPos, nextCell, posRot );

	stepLength = (nextPos - start).norm();
	if ( stepLength > length )
	{
#ifdef __DEBUG__
	    cout << "step length exceeded in first cell" << endl;
#endif

	    // length reached: interrupt computation
	    double u = length / stepLength;
	    nextPos = (1.-u) * start + u * nextPos;
	    steps.push_back( nextPos );
	    vecs.push_back( cell_vectors[cellId.getIndex()] );
	    visitedCells.push_back( cellId );

	    // check results
#ifndef NODEBUG
	    /*
	    cout << "checking returned position: " << endl;
	    if ( isInside( cellId, nextPos ) )
		cout << "correct" << endl;
	    else
		cout << "error" << endl;
		*/
#endif


	    return 0;
	}
// 	else if ( stepLength < 1.0e-8 )
// 	{
// 	    cout << "step size underflow in first cell" << endl;
// 	    cout << "nextPos = " << nextPos << endl;
// 	    cout << "start = " << start << endl;
// 	    cout << "distance = " << stepLength << endl;
// 	    cout << "pos=" << currPos << endl;
// 	    cout << "cellId=" << cellId << endl;
// 	    std::vector< FIndex > ids;
// 	    info.cellDef->getCellVerticesIndices( cellId, ids );
// 	    FArray p;
// 	    for ( positive k=0 ; k<3 ; ++k )
// 	    {
// 		info.posSet->getPosition( p, ids[k] );
// 		cout << "pos #" << k << "=" << p << endl;
// 	    }

// 	    // avoid endless loop
// 	    return 2;
// 	}

	// otherwise, take full step and increment current state
	steps.push_back( nextPos );
	currLength += stepLength;
	currPos  = nextPos;
	if ( nextCell.isValid() )
	{
	    vecs.push_back( cell_vectors[cellId.getIndex()] );
	    visitedCells.push_back( cellId );
	    currCell = nextCell;
	}
	else
	{
#ifdef __DEBUG__
	    cout << "left grid in from first cell" << endl;
#endif
	    return 1;
	}

	// done
	// --------------------------------------------------


	// after initialization step we start integration from an edge
	onEdge = true;
	FArray lastStep;
	while ( proceed() )
	{
	    if ( onEdge )
	    {
		succeeded = walkThroughCell( currPos, currCell, lastStep, first, second,
					     fwd, true, nextPos, nextCell,
					     posRot );

#ifdef __DEBUG__
		if ( !succeeded )
		    cout << "walkThroughCell failed" << endl;
#endif
	    }
	    else
	    {
		// currPos is implicit in start vertex id
		info.posSet->getPosition( currPos, first );
		succeeded = walkThroughVertex( first, lastStep, first, second,
					       fwd, nextPos, currCell,
					       nextCell );

#ifdef __DEBUG__
		if ( !succeeded )
		    cout << "walkThroughVertex failed" << endl;
#endif

	    }
	    if ( !succeeded )
		return 2;

	    if ( !nextCell.isValid() )
		return 1;

	    // first, check if prescribed length has been exceeded
	    stepLength = (nextPos - currPos).norm();
	    if ( currLength+stepLength > length )
	    {
#ifdef __DEBUG__
		cout << "step length exceeded" << endl;
#endif

		// length reached: interrupt computation
		double u = ( length - currLength ) / stepLength;
		nextPos = (1.-u) * currPos + u * nextPos;

		// if step has been taken along a singular vertex
		// ensure numerical stability by pointing slightly
		// toward cell mean point.
		// You're right: this is a dirty workaround.
		if ( first.isValid() && second.isValid() &&
		     first==second )
		{
#ifdef __DEBUG__
		    cout << "slightly moving away from edge" << endl;

		    cout << "checking returned position in both given cells"
			 << endl;
		    cout << "currCell: " << endl;
		    bool bla = isInside( currCell, nextPos );
		    cout << "nextCell: " << endl;
		    bla = isInside( nextCell, nextPos );
#endif

		    std::vector< FIndex > id;
		    std::vector< FArray > p(3);
		    info.cellDef->getCellVerticesIndices( currCell, id );
		    for ( positive l=0 ; l<3 ; ++l )
			info.posSet->getPosition( p[l], id[l] );
		    FArray mean = 1./3.*( p[0]+p[1]+p[2] );
		    nextPos += .05*( mean-nextPos );
// 		    FArray normal = crossProduct( p[1]-p[0],
// 						  p[2]-p[0] );
// 		    normal.normalize();
// 		    nextPos -= ( nextPos*normal )*normal;
		}

		steps.push_back( nextPos );
		vecs.push_back( cell_vectors[currCell.getIndex()] );
		visitedCells.push_back( currCell );

		// check results
#ifdef __DEBUG__
		cout << endl << endl << "checking returned position: " << endl;
		if ( isInside( currCell, nextPos ) )
		    cout << "correct" << endl << endl;
		else
		    cout << "error" << endl << endl;
#endif

		return 0;
	    }
	    else if ( stepLength < 1.0e-8 )
		// avoid endless loop
		return 2;

	    // increment length
	    currLength += stepLength;

	    // now, we have to check for a singular edge
	    if ( onEdge )
	    {
		if ( first == second )
		{
		    onEdge = false;
		}

		lastStep = nextPos - currPos;
		steps.push_back( nextPos );
		visitedCells.push_back( currCell );
		vecs.push_back( cell_vectors[currCell.getIndex()] );
	    }
	    else
	    {
		// we started on a vertex:
		// 2 possible cases:
		//   - we entered a cell and will proceed from an edge
		//   - we followed an edge an will proceed from a vertex
		if ( first == second )
		{
		    // followed an edge
		    // mark cell on both sides of edge as visited
		    // to do so, insert next position twice
		    lastStep = nextPos - currPos;
		    steps.push_back( nextPos );
		    visitedCells.push_back( currCell );
		    vecs.push_back( cell_vectors[currCell.getIndex()] );
		    steps.push_back( nextPos );
		    visitedCells.push_back( nextCell ); // this is no next cell!
		    vecs.push_back( cell_vectors[nextCell.getIndex()] );
		    // onEdge remains false
		}
		else
		{
		    // rentered a cell
		    onEdge = true;
		    lastStep = nextPos - currPos;
		    steps.push_back( nextPos );
		    visitedCells.push_back( currCell );
		    vecs.push_back( cell_vectors[currCell.getIndex()] );
		}
	    }

	    // increment current values
	    currPos  = nextPos;
	    currCell = nextCell;
	}

	// left loop because of proceed test: assume max length has been reached
	return 3;
    }
    CATCH_N_RETHROW( FException );

    // something went wrong: probably a singular edge...
    return 2;
}

//---------------------------------------------------------------------------

void FStreamlineOnSurfaceEuler::setOffset( float offset )
{
    this->offset = offset;
}

//---------------------------------------------------------------------------

void FStreamlineOnSurfaceEuler::setMaxNbCells( unsigned int maxNbCells )
{
    this->maxCells = maxNbCells;
}

//---------------------------------------------------------------------------

const std::vector< FPosition >&
FStreamlineOnSurfaceEuler::getIntermediateSteps( ) const
{
    return steps;
}

//---------------------------------------------------------------------------

const std::vector< FArray >&
FStreamlineOnSurfaceEuler::getIntermediateDirections( ) const
{
    return vecs;
}

//---------------------------------------------------------------------------

const std::vector< FIndex >&
FStreamlineOnSurfaceEuler::getVisitedCells( ) const
{
    return visitedCells;
}

//---------------------------------------------------------------------------

void FStreamlineOnSurfaceEuler::showPosition( const FPosition& /*pos*/,
					      float * /*color*/ ) const
{
}

//---------------------------------------------------------------------------

void FStreamlineOnSurfaceEuler::showVector( const FPosition& /*pos*/ ) const
{
}

//---------------------------------------------------------------------------

void FStreamlineOnSurfaceEuler::baryCoords( double& b0, double& b1, double& b2,
					    double point[], double verts[][2] )
{
    delta = 1. / ( crossProd( verts[1], verts[2] ) );

    b1 = crossProd( point, verts[2] );
    b2 = crossProd( verts[1], point );
    b1 *= delta;
    b2 *= delta;
    b0 = 1.-b1-b2;
}

//---------------------------------------------------------------------------

double FStreamlineOnSurfaceEuler::crossProd( double v1[], double v2[] )
{
    return ( v1[0]*v2[1] - v1[1]*v2[0] );
}

//---------------------------------------------------------------------------

double FStreamlineOnSurfaceEuler::myCrossProd( const FArray& v1,
					       const FArray& v2 )
{
    return v1(0)*v2(1) - v1(1)*v2(0);
}

//---------------------------------------------------------------------------

bool FStreamlineOnSurfaceEuler::walkThroughCell( const FArray& entry,
						 const FIndex& cellId,
                         const FArray& lastStep,
						 FIndex& vertId1,
						 FIndex& vertId2,
						 double fwd,
						 bool onEdge,
						 FArray& exit,
						 FIndex& nextCell,
						 bool& posRot )
{
#ifdef __DEBUG__
    cout << endl << "walking through a cell" << endl;
#endif

    info.cellDef->getCellVerticesIndices( cellId, indices );
    for ( positive i=0 ; i<3 ; i++ )
	info.posSet->getPosition( vertices[i], indices[i] );

    if ( onEdge )
    {
#ifdef __DEBUG__
	cout << "starting from an edge" << endl;
#endif

	// identify current edge in list, assuming positive orientation
	// of entry edge
	positive i=0;
	while ( indices[i] != vertId1 ) { ++i; }

	// re-numerate vertices
	positive ids[3] = { i, (i+1)%3, (i+2)%3 };

	// define local basis
	basis[0] = vertices[ids[1]] - vertices[ids[0]];
	basis[1] = vertices[ids[2]] - vertices[ids[0]];
	normal = crossProduct( basis[0], basis[1] );
	normal.normalize();
	basis[0].normalize();
	basis[1] = crossProduct( normal, basis[0] );

	// local coordinates
    // FIXME: fix orientation for tensor case:
    double o = 1.;
    if(lastStep.size() != 0)
    {
      double orient = cell_vectors[cellId.getIndex()]*lastStep;
      if(orient < 0. ) o = -1.;
    }
	point[0] = ( entry - vertices[ids[0]] ) * basis[0];
	point[1] = 0.;
	vec[0]   = o*fwd * cell_vectors[cellId.getIndex()] * basis[0];
	vec[1]   = o*fwd * cell_vectors[cellId.getIndex()] * basis[1];

	// check for singular edge
	if ( vec[1] < 0. )
	{
#ifdef __DEBUG__
	    cout << "proceed along singular edge" << endl;
	    cout << "posRot is " << ( posRot ? "true" : "false" ) << endl;
	    cout << "fwd = " << fwd << endl;
	    cout << "vec[0]=" << vec[0] << endl;
#endif

	    // check for degenerate case
	    if ( ( posRot && vec[0] < 0. ) ||
		 ( !posRot && vec[0] > 0. ) )

	    {
#ifdef __DEBUG__
		cout << "FStreamlineOnSurfaceEuler WARNING: DEGENERATE CONFIGURATION ENCOUNTERED"
		     << endl;
#endif
		return false;
	    }

	    // follow edge toward next vertex
	    if ( posRot )
	    {
		exit = vertices[ids[1]];
		vertId1 = vertId2 = indices[ids[1]];
		// unable to decide about next cell
		return true;
	    }
	    else
	    {
		exit = vertices[ids[0]];
		vertId1 = vertId2 = indices[ids[0]];
		return true;
	    }
	}

#ifdef __DEBUG__
	cout << "crossing the cell" << endl;
#endif

	// cell vertices in local coordinates
	verts[ids[0]][0] = verts[ids[0]][1] = verts[ids[1]][1] = 0.;
	p0p1 = vertices[ids[1]] - vertices[ids[0]];
	verts[ids[1]][0] = p0p1 * basis[0];
	p0p2 = vertices[ids[2]] - vertices[ids[0]];
	verts[ids[2]][0] = p0p2 * basis[0];
	verts[ids[2]][1] = p0p2 * basis[1];

	// checking edge [1'-2']
	tmp[0][0] = verts[ids[1]][0]-point[0];
	tmp[0][1] = verts[ids[1]][1]-point[1];
	tmp[1][0] = verts[ids[2]][0]-verts[ids[1]][0];
	tmp[1][1] = verts[ids[2]][1]-verts[ids[1]][1];
	u = crossProd( vec, tmp[0] ) / crossProd( tmp[1], vec );
	if ( u > -epsilon && u < 1.+epsilon )
	{
#ifdef __DEBUG__
	    cout << "found intersection on edge #1, u=" << u << endl;
#endif

	    exit = (1.-u)*vertices[ids[1]] + u*vertices[ids[2]];
	    vertId1 = indices[ids[2]];
	    vertId2 = indices[ids[1]];
	    exit_edge[0] = verts[ids[1]][0] - verts[ids[2]][0];
	    exit_edge[1] = verts[ids[1]][1] - verts[ids[2]][1];
	    posRot = ( exit_edge[0]*vec[0] + exit_edge[1]*vec[1] > 0. );
#ifdef __DEBUG__
	    cout << "posRot is " << ( posRot ? "true" : "false" ) << endl;
	    cout << "fwd = " << fwd << endl;
#endif
	    // get neighboring cell
	    info.cellDef->getNeighborhoodData()->
		getCellEdgeNeighbors( cellId, ids[1], neighs );
	    if ( neighs.size() )
		nextCell = neighs[0];
	    else
		nextCell.setToInvalid();

	    // compute projection for consistency check in next cell
	    n_edge = vertices[ids[1]] - vertices[ids[2]];
	    n_edge.normalize();
	    lastProjection = ( cell_vectors[cellId.getIndex()]*n_edge ) * n_edge;

	    return true;
	}

	// checking edge [2'-0']
	tmp[0][0] = verts[ids[2]][0]-point[0];
	tmp[0][1] = verts[ids[2]][1]-point[1];
	tmp[1][0] = verts[ids[0]][0]-verts[ids[2]][0];
	tmp[1][1] = verts[ids[0]][1]-verts[ids[2]][1];
	u = crossProd( vec, tmp[0] ) / crossProd( tmp[1], vec );
	if ( u > -epsilon && u < 1.+epsilon )
	{
#ifdef __DEBUG__
	    cout << "found intersection on edge #2, u=" << u << endl;
#endif

	    exit = (1.-u)*vertices[ids[2]] + u*vertices[ids[0]];
	    vertId1 = indices[ids[0]];
	    vertId2 = indices[ids[2]];
	    exit_edge[0] = verts[ids[2]][0] - verts[ids[0]][0];
	    exit_edge[1] = verts[ids[2]][1] - verts[ids[0]][1];
	    posRot = ( exit_edge[0]*vec[0] + exit_edge[1]*vec[1] > 0. );
#ifdef __DEBUG__
	    cout << "posRot is " << ( posRot ? "true" : "false" ) << endl;
	    cout << "fwd = " << fwd << endl;
#endif
	    // get neighboring cell
	    info.cellDef->getNeighborhoodData()->
		getCellEdgeNeighbors( cellId, ids[2], neighs );
	    if ( neighs.size() )
		nextCell = neighs[0];
	    else
		nextCell.setToInvalid();

	    // compute projection for consistency check in next cell
	    n_edge = vertices[ids[2]] - vertices[ids[0]];
	    n_edge.normalize();
	    lastProjection = ( o*cell_vectors[cellId.getIndex()]*n_edge ) * n_edge;

	    return true;
	}

	// otherwise failed
#ifdef __DEBUG__
	cout << "intersection point computation failed: u=" << u << endl;
#endif

	return false;

    }
    else
    {
#ifdef __DEBUG__
	cout << "starting within the cell" << endl;
#endif

	// local basis: (basis[0], basis[1]) spans cell plane
	basis[0] = vertices[1] - vertices[0];
	p0p2 = vertices[2] - vertices[0];
	if ( is2Din3D )
	{
	    normal = crossProduct( basis[0], p0p2 );
	    basis[1] = crossProduct( normal, basis[0] );
	}
	else
	    basis[1] = FArray( -basis[0](1), basis[0](0) );

	basis[0].normalize();
	basis[1].normalize();
	// local coordinates
	//  ...cell vertices
	verts[0][0] = verts[0][1] = verts[1][1] = 0.;
	basis[0].normalize();
	basis[1].normalize();
	verts[1][0] = ( vertices[1] - vertices[0] ) * basis[0];
	verts[2][0] = p0p2 * basis[0];
	verts[2][1] = p0p2 * basis[1];
	//  ...entry point
	point[0] = ( entry - vertices[0] ) * basis[0];
	point[1] = ( entry - vertices[0] ) * basis[1];
	//  ...direction vector
    double o = 1.;
    if(lastStep.size() != 0)
    {
      double orient = cell_vectors[cellId.getIndex()]*lastStep;
      if(orient < 0. ) o = -1.;
    }
	vec[0] = o*fwd * cell_vectors[cellId] * basis[0];
	vec[1] = o*fwd * cell_vectors[cellId] * basis[1];

	// vector indicating if vec may intersect the corresponding edge
	for ( positive i=0 ; i<3 ; i++ )
	{
	    v[i][0] = ( vertices[i] - entry ) * basis[0];
	    v[i][1] = ( vertices[i] - entry ) * basis[1];
	}
	direct[0] = vec[0];
	direct[1] = vec[1];

	for ( positive i=0 ; i<3 ; i++ )
	{
	    cross = crossProd( v[i], direct );
	    if ( cross > epsilon ) {
		valid[ (i+2) % 3 ] = false;
	    }
	    else if ( cross < -epsilon ) {
		valid[ i ] = false;
	    }
	    else if ( v[i][0]*direct[0] + v[i][1]*direct[1] < 0. )
	    {
		valid[ (i+2) % 3 ] = false;
		valid[ i ] = false;
		break;
	    }
	    else if ( cross >= 0 )
	    {
#ifdef __DEBUG__
		cout << "FStreamlineOnSurfaceEuler WARNING: we are pointing toward a vertex" << endl;
#endif
		valid[ (i+2) % 3 ] = true;
		valid[ i ] = false;
		valid[ (i+1) % 3 ] = false;
		break;
	    }
	    else
	    {
#ifdef __DEBUG__
		cout << "FStreamlineOnSurfaceEuler WARNING: we are pointing toward a vertex" << endl;
#endif
		valid[ i ] = true;
		valid[ (i+1) % 3 ] = false;
		valid[ (i+2) % 3 ] = false;
		break;
	    }
	}
	id0 = 0;
	while ( !valid[ id0 ] )
	    ++id0;
	id1 = ( id0 + 1 ) % 3;

	if ( id0>2 || id1>2 )
	{
#ifdef __DEBUG__
	    cout << "no correct edge found. failed" << endl;
#endif

	    return false;
	}

	// compute intersection point
#ifdef __DEBUG__
	cout << "found intersection on edge #" << id0 << flush;
#endif

    o = 1.;
    if(isTensor )
    {
      double orient = cell_vectors[cellId.getIndex()]*lastStep;
      if(orient < 0. ) o = -1.;
    }
	double u;
	if ( is2Din3D )
	    u =
		( crossProduct( entry - vertices[id0], o*fwd * cell_vectors[cellId] )
		  * normal ) /
		( crossProduct( vertices[id1] - vertices[id0],
				fwd * cell_vectors[cellId] ) * normal );
	else
	    u =
		myCrossProd( entry - vertices[id0], o*fwd * cell_vectors[cellId] ) /
		myCrossProd( vertices[id1] - vertices[id0],
			     fwd * cell_vectors[cellId] );

#ifdef __DEBUG__
	cout << ", u=" << u << endl;
#endif

	if ( u < .01 )
	    u = .01;
	else if ( u > .98 )
	    u = .98;
	exit = (1.-u)*vertices[id0] + u*vertices[id1];

	exit_edge[0] = verts[id0][0] - verts[id1][0];
	exit_edge[1] = verts[id0][1] - verts[id1][1];
	posRot = ( exit_edge[0]*vec[0] + exit_edge[1]*vec[1] > 0. );
#ifdef __DEBUG__
	cout << "posRot is " << ( posRot ? "true" : "false" ) << endl;
	cout << "fwd = " << fwd << endl;
#endif
    }

    // get neighboring cell
    info.cellDef->getNeighborhoodData()->
	getCellEdgeNeighbors( cellId, id0, neighs );
    if ( neighs.size() )
	nextCell = neighs[0];
    else
	nextCell.setToInvalid();

    // set new values to edge vertices
    vertId1 = indices[id1];
    vertId2 = indices[id0];

    // compute projection for consistency check in next cell
    n_edge = vertices[id0] - vertices[id1];
    n_edge.normalize();
    lastProjection = ( cell_vectors[cellId.getIndex()]*n_edge ) * n_edge;


    return true;
}

//---------------------------------------------------------------------------

bool FStreamlineOnSurfaceEuler::walkThroughVertex( const FIndex& vertId,
						   const FArray& lastStep,
						   FIndex& vertId1,
						   FIndex& vertId2,
						   double fwd, FArray& exit,
						   FIndex& currCell,
						   FIndex& nextCell )
{

#ifdef __DEBUG__
    cout << endl << endl << "walking through a vertex" << endl;
#endif

    positive k, neighId;
    int rightAngle=-1;
    bool canProceed[15] = { false, false, false, false, false,
			    false, false, false, false, false,
			    false, false, false, false, false };

    // get incident triangles
    info.cellDef->getNeighborhoodData()->
	getPositionCellNeighbors( vertId, neighs );

    // loop over neighbors
    for ( positive i=0 ; i<neighs.size() ; i++ )
    {
	info.cellDef->getCellVerticesIndices( neighs[i], indices );
	for ( positive j=0 ; j<3 ; j++ )
	{
	    info.posSet->getPosition( vertices[j], indices[j] );
	}
	neighId = neighs[i].getIndex();

	k=0;
	while ( indices[k] != vertId ) { ++k; }

	// re-numerate vertices
	positive ids[3] = { k, (k+1)%3, (k+2)%3 };

	p0p1 = vertices[ids[1]] - vertices[ids[0]];
	p0p2 = vertices[ids[2]] - vertices[ids[0]];
	p0p1.normalize();
	p0p2.normalize();
	normal = crossProduct( p0p1, p0p2 );
	normal.normalize();

	// check if cell value lies within angular domain
	// only consider those domains that correspond to forward motion
	if ( // forward motion
	     lastStep * ( p0p1 + p0p2 ) > 0. &&
	     // acceptable angle value
	     ( crossProduct( p0p1, fwd*cell_vectors[neighId] ) *
	       crossProduct( p0p2, fwd*cell_vectors[neighId] ) < 0.) &&
	     fwd*cell_vectors[neighId] * ( p0p1 + p0p2 ) > 0. )
	{
	    canProceed[i] = true;
	}

	// check if last step lies within angular domain
	projected = lastStep - ( lastStep*normal )*normal;
	double proj_norm = projected.norm();
	double cross1 = crossProduct( p0p1, projected ) * normal;
	double cross2 = crossProduct( p0p2, projected ) * normal;
	double eps = proj_norm * epsilon;
	if ( rightAngle < 0 &&
	     ( fabs( cross1 ) < eps ||
	       fabs( cross2 ) < eps ||
	       ( cross1 * cross2 <= 0. ) ) &&
	     projected * ( p0p1 + p0p2 ) >= 0. )
	{
	    rightAngle = i;
	    if ( canProceed[i] )
	    {
		break;
	    }
	}
    }

    if ( rightAngle < 0 )
    {
	return false;
    }

    // simplest case: can proceed in right angular domain or in another one
    int next = -1;
    if ( canProceed[rightAngle] )
	next = rightAngle;
    else
	for ( positive i=0 ; i<neighs.size() ; i++ )
	    if ( canProceed[i] )
	    {
		next = i;
		break;
	    }

    if ( next >= 0 )
    {
	info.cellDef->getCellVerticesIndices( neighs[next], indices );
	for ( positive j=0 ; j<3 ; j++ )
	    info.posSet->getPosition( vertices[j], indices[j] );

	k=0;
	while ( indices[k] != vertId ) { ++k; }

	// edge to intersect is [k+1,k+2]
	// re-numerate vertices
	positive ids[3] = { k, (k+1)%3, (k+2)%3 };

	p0p1 = vertices[ids[1]] - vertices[ids[0]];
	p0p2 = vertices[ids[2]] - vertices[ids[0]];
	normal = crossProduct( p0p1, p0p2 );
	normal.normalize();
	basis[0] = p0p1;
	basis[0].normalize();
	basis[1] = crossProduct( normal, basis[0] );
	verts[1][0] = p0p1 * basis[0];
	verts[1][1] = 0.;
	verts[2][0] = p0p2 * basis[0];
	verts[2][1] = p0p2 * basis[1];

	// direction vector
	vec[0] = fwd * cell_vectors[neighs[next]] * basis[0];
	vec[1] = fwd * cell_vectors[neighs[next]] * basis[1];

	// compute exit position
	u = - ( verts[1][0]*vec[1]-verts[1][1]*vec[0] ) /
	    ( (verts[2][0]-verts[1][0])*vec[1] -
	      (verts[2][1]-verts[1][1])*vec[0] );
	exit = (1.-u)*vertices[ids[1]] + u*vertices[ids[2]];

	// get neighboring cell
	currCell = neighs[next];
	info.cellDef->getNeighborhoodData()->
	    getCellEdgeNeighbors( neighs[next], ids[1], neighs );
	if ( neighs.size() )
	    nextCell = neighs[0];
	else
	    nextCell.setToInvalid();
	vertId1 = indices[ids[2]];
	vertId2 = indices[ids[1]];

	return true;
    }

    // otherwise we have to look for a singular edge emanating from
    // current vertex along which we can proceed
    // start search from right angular domain

#ifdef __DEBUG__
    cout << "must look for a singular edge to proceed" << endl;
#endif

    FIndex curr = neighs[rightAngle];
    FIndex n1, n2, n3, n4;
    // get 2 neighbors in 1-neighborhood
    info.cellDef->getCellVerticesIndices( curr, indices );

    for ( positive i=0 ; i<3 ; i++ )
    {
	info.posSet->getPosition( vertices[i], indices[i] );
    }

    k=0;
    while ( indices[k] != vertId ) { ++k; }

    // 1st neighbor
    info.cellDef->getNeighborhoodData()->
	getCellEdgeNeighbors( curr, k, neighs2 );
    if ( neighs2.size() )
    {
	n1 = neighs2[0];

	p0p1 = vertices[(k+1)%3] - vertices[k];
	if ( crossProduct( p0p1, fwd*cell_vectors[n1.getIndex()] ) *
	     crossProduct( p0p1, fwd*cell_vectors[curr.getIndex()] ) < 0. )
	{
	    // found
	    vertId1 = vertId2 = indices[(k+1)%3];
	    info.posSet->getPosition( exit, indices[(k+1)%3] );
	    // unable to determine next cell
	    // use parameters to return cells sharing the edge we follow
	    currCell = curr;
	    nextCell = n1;

#ifdef __DEBUG__
	    cout << "found neighbor #1" << endl;
#endif
	    return true;
	}
    }

    // 2nd neighbor
    info.cellDef->getNeighborhoodData()->
	getCellEdgeNeighbors( curr, (k+2)%3, neighs2 );
    if ( neighs2.size() )
    {
	n2 = neighs2[0];
	p0p1 = vertices[(k+2)%3] - vertices[k];
	if ( crossProduct( p0p1, fwd*cell_vectors[n2.getIndex()] ) *
	     crossProduct( p0p1, fwd*cell_vectors[curr.getIndex()] ) < 0. )
	{
	    // found
	    vertId1 = vertId2 = indices[(k+2)%3];
	    info.posSet->getPosition( exit, indices[(k+2)%3] );
	    // unable to determine next cell
	    // use parameters to return cells sharing the edge we follow
	    currCell = curr;
	    nextCell = n1;

#ifdef __DEBUG__
	    cout << "found neighbor #2" << endl;
#endif
	    return true;
	}
    }

    // get further neighbors
    if ( n1.isValid() )
    {
	info.cellDef->getCellVerticesIndices( n1, indices );
	for ( positive i=0 ; i<3 ; i++ )
	{
	    info.posSet->getPosition( vertices[i], indices[i] );
	}

	k=0;
	while ( indices[k] != vertId ) { ++k; }

	info.cellDef->getNeighborhoodData()->
	    getCellEdgeNeighbors( n1, k, neighs2 );
	if ( neighs2.size() && neighs2[0] != curr )
	{
	    n3 = neighs2[0];
	    p0p1 = vertices[(k+1)%3] - vertices[k];
	    if ( crossProduct( p0p1, fwd*cell_vectors[n3.getIndex()] ) *
		 crossProduct( p0p1, fwd*cell_vectors[n1.getIndex()] ) < 0. )
	    {
		// found
		vertId1 = vertId2 = indices[(k+1)%3];
		info.posSet->getPosition( exit, indices[(k+1)%3] );
		// unable to determine next cell
		// use parameters to return cells sharing the edge we follow
		currCell = n1;
		nextCell = n3;

#ifdef __DEBUG__
		cout << "found neighbor #3" << endl;
#endif
		return true;
	    }
	}
	info.cellDef->getNeighborhoodData()->
	    getCellEdgeNeighbors( n1, (k+2)%3, neighs2 );
	if ( neighs2.size() && neighs2[0] != curr )
	{
	    n3 = neighs2[0];

	    p0p1 = vertices[(k+2)%3] - vertices[k];
	    if ( crossProduct( p0p1, fwd*cell_vectors[n3.getIndex()] ) *
		 crossProduct( p0p1, fwd*cell_vectors[n1.getIndex()] ) < 0. )
	    {
		// found
		vertId1 = vertId2 = indices[(k+2)%3];
		info.posSet->getPosition( exit, indices[(k+2)%3] );
		// unable to determine next cell
		// use parameters to return cells sharing the edge we follow
		currCell = n1;
		nextCell = n3;

#ifdef __DEBUG__
		cout << "found neighbor #4" << endl;
#endif
		return true;
	    }
	}
    }

    if ( n2.isValid() )
    {
	info.cellDef->getCellVerticesIndices( n2, indices );
	for ( positive i=0 ; i<3 ; i++ )
	{
	    info.posSet->getPosition( vertices[i], indices[i] );
	}

	k=0;
	while ( indices[k] != vertId ) { ++k; }

	info.cellDef->getNeighborhoodData()->
	    getCellEdgeNeighbors( n2, (k+1)%3, neighs2 );
	if ( neighs2.size() && neighs2[0] != curr )
	{
	    n4 = neighs2[0];
	    p0p1 = vertices[(k+1)%3] - vertices[k];
	    if ( crossProduct( p0p1, fwd*cell_vectors[n4.getIndex()] ) *
		 crossProduct( p0p1, fwd*cell_vectors[n2.getIndex()] ) < 0. )
	    {
		// found
		vertId1 = vertId2 = indices[(k+1)%3];
		info.posSet->getPosition( exit, indices[(k+1)%3] );
		// unable to determine next cell
		// use parameters to return cells sharing the edge we follow
		currCell = n2;
		nextCell = n4;

#ifdef __DEBUG__
		cout << "found neighbor #5" << endl;
#endif
		return true;
	    }
	}
	info.cellDef->getNeighborhoodData()->
	    getCellEdgeNeighbors( n2, (k+2)%3, neighs2 );
	if ( neighs2.size() && neighs2[0] != curr )
	{
	    n4 = neighs2[0];

	    p0p1 = vertices[(k+2)%3] - vertices[k];
	    if ( crossProduct( p0p1, fwd*cell_vectors[n4.getIndex()] ) *
		 crossProduct( p0p1, fwd*cell_vectors[n2.getIndex()] ) < 0. )
	    {
		// found
		vertId1 = vertId2 = indices[(k+2)%3];
		info.posSet->getPosition( exit, indices[(k+2)%3] );
		// unable to determine next cell
		// use parameters to return cells sharing the edge we follow
		currCell = n2;
		nextCell = n4;

#ifdef __DEBUG__
		cout << "found neighbor #6" << endl;
#endif
		return true;
	    }
	}
    }

    // otherwise give up
#ifdef __DEBUG__
    cout << "no satisfying singular edge found" << endl;
#endif
    return false;
}

//---------------------------------------------------------------------------

bool FStreamlineOnSurfaceEuler::proceed( void )
{
    return true;
}

//---------------------------------------------------------------------------

bool FStreamlineOnSurfaceEuler::isInside( const FIndex& cellId,
					  const FArray& pos )
{
    try
    {
    std::vector< FArray > p(3), v(3);
    std::vector< FIndex > ids;
    FArray e0, e1, normal, loc;

    info.cellDef->getCellVerticesIndices( cellId, ids );
    for ( positive i=0 ; i<3 ; ++i )
	info.posSet->getPosition( p[i], ids[i] );
    e0 = p[1]-p[0];
    e1 = p[2]-p[0];
    normal = crossProduct( e0, e1 );
    normal.normalize();
    e1 = crossProduct( normal, e0 );
    e0.normalize();
    e1.normalize();

    if ( fabs( (pos-p[0])*normal ) > 1.0e-5 )
    {
#ifdef __DEBUG__
	cout << "distance to plane too large: " << fabs( (pos-p[0])*normal )
	     << endl;
	cout << "corresponding positions are: " << endl
	     << "* curr = " << pos << endl
	     << "* p0 = " << p[0] << endl
	     << "* p1 = " << p[1] << endl
	     << "* p2 = " << p[2] << endl << endl;
#endif
	return false;
    }

    loc = FArray( ( pos-p[0] )*e0, ( pos-p[0] )*e1 );

    v[0] = FArray( 0., 0. );
    v[1] = FArray( ( p[1]-p[0] )*e0, 0. );
    v[2] = FArray( ( p[2]-p[0] )*e0, ( p[2]-p[0] )*e1 );

    double b[2] = {0., 0.};
    double denom = ( (v[0][0]-v[2][0])*(v[1][1]-v[2][1]) -
		     (v[0][1]-v[2][1])*(v[1][0]-v[2][0]) );
    double alpha = - 1.0e-6 * denom;

    b[0] = ( (loc[0]-v[2][0])*(v[1][1]-v[2][1]) -
	     (loc[1]-v[2][1])*(v[1][0]-v[2][0]) );

    if ( ( (denom > 0.) && (b[0] > alpha) ) ||
	 ( (denom < 0.) && (b[0] < alpha) ) ) {

      b[1] = ( (v[0][0]-v[2][0])*(loc[1]-v[2][1]) -
	       (v[0][1]-v[2][1])*(loc[0]-v[2][0]) );

      if ( ( (denom > 0.) && (b[1] > alpha) &&
	     (denom - b[0] - b[1] > alpha) ) ||
	   ( (denom < 0.) && (b[1] < alpha) &&
	     (denom - b[0] - b[1] < alpha) ) )
	  return true;
#ifdef __DEBUG__
      else
	  cout << "wrong barycentric coordinates: "
	       << "b[1] = " << b[1]/denom << endl;
#endif
    }

#ifdef __DEBUG__
    cout << "wrong barycentric coordinates: "
	 << "b[0] = " << b[0]/denom << endl;
    cout << "corresponding positions are: " << endl
	 << "* curr = " << pos << endl
	 << "* p0 = " << p[0] << endl
	 << "* p1 = " << p[1] << endl
	 << "* p2 = " << p[2] << endl << endl;
#endif

    return false;
    }
    catch ( FException& e )
    {
    	printf("caught exception in isInside:\n ");
	//cout << "caught exception in isInside: " << e << endl;
	return false;
    }
}
#endif
