/*
 * SurfaceLIC.cpp
 *
 *  Created on: 17.11.2008
 *      Author: ralph
 */

#include "SurfaceLIC.h"
#include "../Fantom/FTensor.h"

SurfaceLIC::SurfaceLIC(DatasetHelper* dh, TriangleMesh* grid) {
	m_dh = dh;
	m_grid = grid;

	nbFold = 20;
	max_length = 10.;
	modulo = 20;

	black = true;

	min_length = 10;
	threshold = 10;

	nbCells = m_grid->getNumTriangles();
}

SurfaceLIC::~SurfaceLIC() {
	// TODO Auto-generated destructor stub
}

//---------------------------------------------------------------------------

void SurfaceLIC::execute() {
	try {
		nbVisited = 0;

		testLines.clear();

		// create textures
		output_texture.resize(m_grid->getNumTriangles());
		input_texture.resize(m_grid->getNumTriangles());
		hit_texture.resize(m_grid->getNumTriangles());

		// setup all the textures
		// create a random input texture of luminance values
#ifdef __WXMSW__
		srand(time(0));
#else
		srand48(time(0));
#endif
		for (int i = 0 ; i < m_grid->getNumTriangles() ; ++i) {
#ifdef __WXMSW__
			input_texture[i] = (float) rand()/ (RAND_MAX + 1);
#else
			input_texture[i] = (float) drand48();
#endif
			output_texture[i] = 0.0;
			hit_texture[i] = 0;
		}
		// cell-based streamlines
		streamline = new MyLICStreamline(m_dh, m_grid);
		streamline->setParams(&hit_texture, min_length, threshold);

		// iterate over all texture points
		positive m, q, n = 0;
		m = (m_grid->getNumTriangles() - 1) / modulo;
		q = (m_grid->getNumTriangles() - 1) % modulo;

		for (positive i = 0; i <= q ; ++i)
			for (positive j = 0; j <= m ; ++j, ++n) {
				calculatePixelLuminance(FIndex(modulo * j + i));
			}

		for (positive i = q + 1; i < modulo ; ++i)
			for (positive j = 0; j < m ; ++j, ++n) {
				calculatePixelLuminance(FIndex(modulo * j + i));
			}


	} catch (FException& e) {
		if (streamline)
			delete streamline;

		e.addTraceMessage("void SurfaceLIC::execute ()");
		throw ;
	}

	displayTexture();

	if ( streamline ) delete streamline;
}

//---------------------------------------------------------------------------

void SurfaceLIC::calculatePixelLuminance(const FIndex& cellId)
{
	//printf("calculatePixelLuminance: cell #%d\n", cellId.getIndex());
	//   cout << "calculatePixelLuminance: cell #" << cellId << " ";
	// already calculated ?
	if (hit_texture[cellId.getIndex()]) {
#ifdef __VERBOSE__
		cout << '0' << endl;
#endif
		return;
	}

#ifdef __VERBOSE__
	cout << 'X' << flush;
#endif

	// compute mean point
	m_grid->getCellVerticesIndices(cellId, ids);
	for (positive j = 0; j < ids.size(); j++) {
		m_grid->getPosition(pos[j], ids[j]);
	}
	start = 1. / 3. * (pos[0] + pos[1] + pos[2]);

	// integrate
	streamline->integrate(start, cellId, true, max_length);
	visitedFwd = streamline->getVisitedCells();

	std::vector<float>line;
	std::vector< FArray > steps = streamline->getIntermediateSteps();
	line.reserve(steps.size() *3);
	for ( positive i=0 ; i<steps.size() ; ++i )
	{
		line.push_back(steps[i](0));
		line.push_back(steps[i](1));
		line.push_back(steps[i](2));
	}
	testLines.push_back(line);

	streamline->integrate(start, cellId, false, max_length);
	visitedBwd = streamline->getVisitedCells();

	line.clear();
	steps = streamline->getIntermediateSteps();
	line.reserve(steps.size() *3);
	for ( positive i=0 ; i<steps.size() ; ++i )
	{
		line.push_back(steps[i](0));
		line.push_back(steps[i](1));
		line.push_back(steps[i](2));
	}
	testLines.push_back(line);


	positive total_sz = visitedFwd.size() + visitedBwd.size();


	// adjust kernel size to streamline length
	positive kernel_sz = nbFold;
	while (total_sz < 2 * kernel_sz)
		kernel_sz /= 2;
	if (kernel_sz < 2) {
#ifdef __VERBOSE__
		cout << "too short: leaving" << endl;
#endif
		return;
	}
	double div = 1. / (2. * (double) kernel_sz);

	// compute convolution

	fifo.clear();
	double sum = 0.;
	// initialize convolution kernel
	for (positive i = 0; i < kernel_sz; i++) {
		fifo.push_back(input_texture[getId(i, visitedBwd, visitedFwd)]);
		sum += fifo.back();
	}
	// loop over streamline
	positive front = kernel_sz, curr = 0;
	double mult = div;
	for (; curr < total_sz; curr++, front++) {
		if (!black)
			if (front < 2 * kernel_sz)
				mult = 1. / (double) front;
			else if (front < total_sz)
				mult = div;
			else
				mult = 1. / (double) (kernel_sz - curr + total_sz);

		positive id = getId(curr, visitedBwd, visitedFwd);
		output_texture[id] += sum * mult;

		if (!hit_texture[id])
			++nbVisited;

		++hit_texture[id];
		if (front >= 2 * kernel_sz) {
			sum -= fifo.front();
			fifo.pop_front();
		}
		if (front < total_sz) {
			fifo.push_back(input_texture[getId(front, visitedBwd, visitedFwd)]);
			sum += fifo.back();
		}
	}
}

//---------------------------------------------------------------------------

positive SurfaceLIC::getId(positive i, const std::vector<FIndex>& bwd,
		const std::vector<FIndex>& fwd) {
	//   if ( i > bwd.size()+fwd.size() )
	//     return ( positive )-1;

	if (i < bwd.size())
		return bwd[bwd.size() - i - 1].getIndex();
	else
		return fwd[i - bwd.size()].getIndex();
}

//---------------------------------------------------------------------------

class sort_ratio {
public:
	int operator()(std::pair<FIndex, double> p1, std::pair<FIndex, double> p2) const {
		return p1.second > p2.second;
	}
};

//---------------------------------------------------------------------------

void SurfaceLIC::displayTexture() {
	double gray;

	for (int i = 0 ; i < nbCells ; ++i)
	{
		if (hit_texture[i])
			gray = output_texture[i] / (double) hit_texture[i];
		else
			gray = input_texture[i];

		m_grid->setTriangleColor(i, gray, gray, gray);

	}
}

//---------------------------------------------------------------------------

