/*
 * SurfaceLIC.h
 *
 *  Created on: 17.11.2008
 *      Author: ralph
 */

#ifndef SURFACELIC_H_
#define SURFACELIC_H_

#include "FStreamlineOnSurfaceEuler.h"
#include "../DatasetHelper.h"
#include "../IsoSurface/triangleMesh.h"

class SurfaceLIC {
public:
	SurfaceLIC(DatasetHelper* dh, TriangleMesh* grid);
	virtual ~SurfaceLIC();

	virtual void execute();
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
	void initialize(void);
	double max_length;
	double maxArea;
	unsigned int nbFold;
	double offset;
	unsigned int maxSubdiv, modulo;
	bool optimizeShape, black, uniform;
	unsigned int mode;

	//  double normalx, normaly, normalz;
	F::FVector mynormal;
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

	void refineGrid();
	void displayTexture();

	std::list<std::string> fieldSelectionList;
	FIndex selField;
	int tfID[100];
	bool whitenoise;
	FIndex SourceField_id;
/*
	struct sourceFilter_t: public FGenPro::TensorFieldFilter {
		bool accept(FTensorFieldInfo& info) {
			return (shared_dynamic_cast<const FCellDefinitions3DTriangulation> (
					info.cellDef) && !info.isCellBased);
		}
	} src_filter;

	struct textureFilter_t: public FGenPro::TensorFieldFilter {
		bool accept(FTensorFieldInfo& info) {
			if (info.tensorOrder == 0) {
				return true;
			}
			return false;
		}
	} textureFilter;
	*/
};

#endif /* SURFACELIC_H_ */
