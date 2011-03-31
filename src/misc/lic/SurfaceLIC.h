/*
 * SurfaceLIC.h
 *
 *  Created on: 17.11.2008
 *      Author: ralph
 */

#ifndef SURFACELIC_H_
#define SURFACELIC_H_

#include "FStreamlineOnSurfaceEuler.h"
#include "../../dataset/DatasetHelper.h"
#include "../IsoSurface/triangleMesh.h"

class SurfaceLIC {
public:
	SurfaceLIC(DatasetHelper* dh, TriangleMesh* grid);
	virtual ~SurfaceLIC();

	virtual void execute();

	std::vector< std::vector<float> >testLines;

private:

	class MyLICStreamline: public FStreamlineOnSurfaceEuler {
	public:

		MyLICStreamline(DatasetHelper* dh, TriangleMesh* grid) :
			FStreamlineOnSurfaceEuler(dh, grid) {
		}

		~MyLICStreamline(void) {
		}

		void setParams(const std::vector<positive> *the_hits, positive length,
				positive athreshold) {
			hits = the_hits;
			min_length = length;
			threshold = athreshold;
		}

		virtual bool proceed(void) {
			return (visitedCells.size() < min_length || (*hits)[currCell]
					< threshold);
		}

	private:
		const std::vector<positive> *hits;
		positive min_length;
		positive threshold;

	};

	DatasetHelper* m_dh;
	TriangleMesh* m_grid;
	int nbCells;

	// profile stuff
	double max_length;
	unsigned int nbFold;
	double offset;
	unsigned int modulo;
	bool optimizeShape, black;

	positive min_length;
	positive threshold;

	//  double normalx, normaly, normalz;
	void calculatePixelLuminance(const FIndex& cellId);

	MyLICStreamline *streamline;
	FArray start;
	std::vector<FIndex> ids, visitedFwd, visitedBwd;
	std::list<double> fifo;
	FArray pos[3];

	positive getId(positive i, const std::vector<FIndex>& bwd,
			const std::vector<FIndex>& fwd);

	std::vector<float> input_texture, output_texture;
	std::vector<positive> hit_texture;

	positive nbVisited;

	void displayTexture();
};

#endif /* SURFACELIC_H_ */
