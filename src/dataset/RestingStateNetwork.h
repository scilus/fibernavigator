/////////////////////////////////////////////////////////////////////////////
// Name:            RestingStateNetwork.h
// Author:          Maxime Chamberland
/////////////////////////////////////////////////////////////////////////////
#ifndef RESTINGSTATENETWORK_H_
#define RESTINGSTATENETWORK_H_

#include "Anatomy.h"
#include "DatasetInfo.h"
#include "DatasetIndex.h"
#include "../misc/nifti/nifti1_io.h"

class RestingStateNetwork
{
public:

    // Constructor/Destructor
    RestingStateNetwork();
    virtual ~RestingStateNetwork();

	bool load( nifti_image *pHeader, nifti_image *pBody );
	void setNetworkInfo( DatasetIndex index ) { m_index = index; }
	void SetTextureFromSlider( int sliderValue );
	void SetTextureFromNetwork();
	void SetCorrThreshold( float thresh ) { m_corrThreshold = thresh; }
	void SetClusterLvlSliderValue (float value ) { m_clusterLvlSliderValue = value; }
	void SetSizePSliderValue (float value ) { m_pointSize = value; }
	void SetAlphaSliderValue (float value ) { m_alpha = value; }

	void render3D(bool recalculateTexture);
	void seedBased();
	size_t getSize()                               { return m_3Dpoints.size(); }
	void clear3DPoints()                           { m_3Dpoints.clear(); }
	void setBoxMoving(bool move)				   { m_boxMoving = move; }
	bool isBoxMoving() { return m_boxMoving; }

	std::vector<float>* getZscores();
	DatasetIndex getIndex()   { return m_index; }
	DatasetIndex getColumns() { return m_columns; }
	DatasetIndex getRows()    { return m_rows; }
	DatasetIndex getFrames()  { return m_frames; }
	DatasetIndex getBands()   { return m_bands; }
	
	std::vector<float> data; //Used for texture mapping
	
private:
    bool createStructure  ( std::vector< short int > &i_fileFloatData );
	void correlate(std::vector< float >& position);
	void calculateMeanAndSigma(std::vector<float> signal, std::pair<float, float>& params);
	std::vector<int> get3DIndexes(int x, int y, int z);
	void erode(std::vector<bool> &tmp, const std::vector<bool> &inMap, int curIndex);
	
    
	std::vector<std::vector<float> >   m_signalNormalized; //2D containing the data normalized
	std::vector<std::vector<float> >   m_volumes; //2D containing the data normalized volume-wise aligned
	std::vector<std::pair< float, float > > m_meansAndSigmas; 
	std::vector<std::pair<Vector,float> > m_3Dpoints; //3D points and their positions
	std::vector<float> m_smallt; //3x3x3 RGB values
	std::vector<float> m_zMap; //1x1x1 zscores
	
	float m_zMin;
	float m_zMax;
	float m_alpha;
	float m_pointSize;
	GLuint m_lookupTex;

	bool m_isRealTimeOn;
    int m_dataType;
	int m_rows;
	int m_columns;
	int m_frames;
	int m_bands;
	int m_datasetSize;
	float m_voxelSizeX;
    float m_voxelSizeY;
    float m_voxelSizeZ;
	DatasetIndex m_index;
	float m_corrThreshold;
	float m_clusterLvlSliderValue;
	int m_rowsL;
	int m_columnsL;
	int m_framesL;
	float m_xL;
	float m_yL;
	float m_zL;
	int m_datasetSizeL;
	bool m_normalize; 
	bool m_boxMoving;
	Vector m_originL;
	Vector m_origin; 

};

#endif /* RESTINGSTATENETWORK_H_ */
