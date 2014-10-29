/////////////////////////////////////////////////////////////////////////////
// Name:            RestingState.cpp
// Author:          Maxime Chamberland
/////////////////////////////////////////////////////////////////////////////

#include "RestingStateNetwork.h"

#include "DatasetManager.h"
#include "AnatomyHelper.h"
#include "RTFMRIHelper.h"
#include "../Logger.h"
#include "../gfx/ShaderHelper.h"
#include "../gfx/TheScene.h"
#include "../gui/MyListCtrl.h"
#include "../gui/SceneManager.h"
#include "../misc/nifti/nifti1_io.h"

#include <GL/glew.h>
#include <wx/math.h>
#include <wx/xml/xml.h>

#include <algorithm>
#include <fstream>
#include <limits>
#include <vector>

// TODO remove
//#include "../gfx/Image.h"
//#include "../gfx/BitmapHandling.h"
//#include "../gfx/TextureHandling.h"
#include "../main.h"

//////////////////////////////////////////
//Constructor
//////////////////////////////////////////
RestingStateNetwork::RestingStateNetwork():
m_zMin( 999.0f ),
m_zMax( 0.0f ),
m_alpha( 0.5f),
m_pointSize( 5.0f ),
m_isRealTimeOn( false ),
m_dataType( 16 ),
m_bands( 108 ),
m_corrThreshold( 1.65f ),
m_clusterLvlSliderValue( 9.0f ),
m_boxMoving( false ),
m_originL(0,0,0),
m_origin(0,0,0)
{
	m_rowsL = DatasetManager::getInstance()->getRows();
	m_columnsL = DatasetManager::getInstance()->getColumns();
	m_framesL =  DatasetManager::getInstance()->getFrames();

	m_xL = DatasetManager::getInstance()->getVoxelX();
	m_yL = DatasetManager::getInstance()->getVoxelY();
	m_zL =  DatasetManager::getInstance()->getVoxelZ();

	m_datasetSizeL = m_rowsL * m_columnsL * m_framesL;

	FMatrix &t = DatasetManager::getInstance()->getNiftiTransform();
	m_originL.x = floor(abs(t(0,3)) / m_xL);
	m_originL.y = floor(abs(t(1,3)) / m_yL);
	m_originL.z = floor(abs(t(2,3)) / m_zL);
}

//////////////////////////////////////////
//Destructor
//////////////////////////////////////////
RestingStateNetwork::~RestingStateNetwork()
{
    Logger::getInstance()->print( wxT( "RestingStateNetwork destructor called but nothing to do." ), LOGLEVEL_DEBUG );
}

//////////////////////////////////////////
//Load
//////////////////////////////////////////
bool RestingStateNetwork::load( nifti_image *pHeader, nifti_image *pBody )
{
    m_datasetSize = pHeader->dim[1] * pHeader->dim[2] * pHeader->dim[3];
	m_smallt.assign(m_datasetSize*3,0.0f);
	m_rows = pHeader->dim[1];
	m_columns = pHeader->dim[2];
	m_frames = pHeader->dim[3];
	m_bands = pHeader->dim[4];

    m_voxelSizeX = pHeader->dx;
    m_voxelSizeY = pHeader->dy;
    m_voxelSizeZ = pHeader->dz;



	if( pHeader->sform_code > 0 )
    {
		m_origin.x = floor(pHeader->sto_ijk.m[0][3]);
		m_origin.y = floor(pHeader->sto_ijk.m[1][3]);
		m_origin.z = floor(pHeader->sto_ijk.m[2][3]);
    }
    else if( pHeader->qform_code > 0 )
    {
        m_origin.x = floor(pHeader->qto_ijk.m[0][3]);
		m_origin.y = floor(pHeader->qto_ijk.m[1][3]);
        m_origin.z = floor(pHeader->qto_ijk.m[2][3]);
    }
    else
    {
        Logger::getInstance()->print( wxT( "No transformation encoded in the nifti file. Alignement will fail." ), LOGLEVEL_WARNING );
    }
 
	std::vector<short int> fileFloatData( m_datasetSize * m_bands, 0);

	if(pHeader->datatype == 4)
	{
		short int* pData = (short int*)pBody->data;
		//Prepare the data into a 1D vector, side by side
		for( int i( 0 ); i < m_datasetSize; ++i )
		{
			for( int j( 0 ); j < m_bands; ++j )
			{
				fileFloatData[i * m_bands + j] = pData[j * m_datasetSize + i];
			}
		}
	}
	else
	{
		float* pData = (float*)pBody->data;
		//Prepare the data into a 1D vector, side by side
		for( int i( 0 ); i < m_datasetSize; ++i )
		{
			for( int j( 0 ); j < m_bands; ++j )
			{
				fileFloatData[i * m_bands + j] = pData[j * m_datasetSize + i];
			}
		}
		
	}

	//Assign structure to a 2D vector of timelaps
    createStructure( fileFloatData );

    //Logger::getInstance()->print( wxT( "Resting-state network initialized" ), LOGLEVEL_MESSAGE );
    return true;
}


//////////////////////////////////////////
//Create structure
//////////////////////////////////////////
bool RestingStateNetwork::createStructure( std::vector< short int > &i_fileFloatData )
{
	int size = m_rows * m_columns * m_frames;
    std::vector<std::vector<short int> >   m_signal(size); //2D containing the original data
	m_signalNormalized.resize ( size );
    vector< short int >::iterator it;
    int i = 0;

    //Fetching the directions
    for( it = i_fileFloatData.begin(), i = 0; it != i_fileFloatData.end(); it += m_bands, ++i )
    { 
		m_signal[i].insert( m_signal[i].end(), it, it + m_bands );
    }
	
	//Find min/max for normalization
	vector<float> dataMax, dataMin;
	dataMax.assign(size, -std::numeric_limits<float>::infinity());
	dataMin.assign(size, std::numeric_limits<float>::infinity());
    for( int s(0); s < size; ++s )
    {
		for( int b(0); b < m_bands; ++b )
		{
			if (m_signal[s][b] > dataMax[s])
			{
				dataMax[s] = m_signal[s][b];
			}
			if (m_signal[s][b] < dataMin[s])
			{
				dataMin[s] = m_signal[s][b];
			}
		}
    }

	//Min max Rescale
    for( int s(0); s < size; ++s )
    {
		for( int b(0); b < m_bands; ++b )
		{
			if((m_signal[s][b] == 0 && dataMin[s] == 0) || (m_signal[s][b] == 16767 && dataMin[s] == 16767)) //Ensure that we dont divide by 0.
				m_signalNormalized[s].push_back(0);
			else
				m_signalNormalized[s].push_back ((m_signal[s][b] - dataMin[s]) / (dataMax[s] - dataMin[s]));
		}
    }

	m_volumes.resize(m_bands);
	m_meansAndSigmas.resize(size);
	//Transpose signal for easy acces of timelaps
    for( int s(0); s < size; ++s )
    {
		calculateMeanAndSigma(m_signalNormalized[s], m_meansAndSigmas[s]);
    }

	//Create texture made of 1st timelaps
	data.assign(size, 0.0f);

    return true;
}

//////////////////////////////////////////
//Set raw data texture from sliderValue
//////////////////////////////////////////
void RestingStateNetwork::SetTextureFromSlider(int sliderValue)
{
	std::vector<float> vol(m_datasetSizeL* 3, 0.0f);

	for(int x = 0; x < m_columnsL; x++)
	{
		for(int y = 0; y < m_rowsL; y++)
		{
			for(int z = 0; z < m_framesL; z++)
			{
				int i = z * m_columnsL * m_rowsL + y *m_columnsL + x;

				float zz = ((z - m_originL.z) * m_zL / m_voxelSizeZ) + m_origin.z;
				float yy = ((y - m_originL.y) * m_yL / m_voxelSizeY) + m_origin.y;
				float xx = ((x - m_originL.x) * m_xL / m_voxelSizeX) + m_origin.x;

				if(xx >1 && yy >1 && zz >1 && xx < m_columns && yy < m_rows && zz < m_frames)
				{
                    const int x = (unsigned int) std::min( (int)std::floor(xx / DatasetManager::getInstance()->getVoxelX() ), m_columns-1 );
                    const int y = (unsigned int) std::min( (int)std::floor(yy / DatasetManager::getInstance()->getVoxelY() ), m_rows-1 );
                    const int z = (unsigned int) std::min( (int)std::floor(zz / DatasetManager::getInstance()->getVoxelZ() ), m_frames-1 );

					const float dx = ( xx / DatasetManager::getInstance()->getVoxelX() )-x;
					const float dy = ( yy / DatasetManager::getInstance()->getVoxelY() )-y;
					const float dz = ( zz / DatasetManager::getInstance()->getVoxelZ() )-z;

                    const int nx = dx > 0.0 ? std::min( std::max(x+1,0), m_columns-1 ) : std::min( std::max(x,0), m_columns-1 );
                    const int ny = dy > 0.0 ? std::min( std::max(y+1,0), m_rows-1 ) : std::min( std::max(y,0), m_rows-1 );
                    const int nz = dz > 0.0 ? std::min( std::max(z+1,0), m_frames-1 ) : std::min( std::max(z,0), m_frames-1 );

					// Compute interpolated value at (fx,fy,fz)
					//Corresponding tensor number
					int tensor_xyz = z * m_columns * m_rows + y * m_columns + x;

					int tensor_nxyz = z * m_columns * m_rows + y * m_columns + nx;

					int tensor_xnyz = z * m_columns * m_rows + ny * m_columns + x;

					int tensor_nxnyz = z * m_columns * m_rows + ny * m_columns + nx;

					int tensor_xynz = nz * m_columns * m_rows + y * m_columns + x;

					int tensor_nxynz = nz * m_columns * m_rows + y * m_columns + nx;

					int tensor_xnynz = nz * m_columns * m_rows + ny * m_columns + x;

					int tensor_nxnynz = nz * m_columns * m_rows + ny * m_columns + nx;

					float valx0 = (1-dx) * m_signalNormalized[tensor_xyz][sliderValue]  + (dx) * m_signalNormalized[tensor_nxyz][sliderValue];
					float valx1 = (1-dx) * m_signalNormalized[tensor_xnyz][sliderValue] + (dx) * m_signalNormalized[tensor_nxnyz][sliderValue];

					const float valy0 = (1-dy) * valx0 + (dy) * valx1;
					valx0 = (1-dx) * m_signalNormalized[tensor_xynz][sliderValue]  + (dx) * m_signalNormalized[tensor_nxynz][sliderValue];
					valx1 = (1-dx) * m_signalNormalized[tensor_xnynz][sliderValue] + (dx) * m_signalNormalized[tensor_nxnynz][sliderValue];

					const float valy1 = (1-dy) * valx0 + (dy) * valx1;

					vol[i*3] = (1-dz) * valy0 + (dz) * valy1;
					vol[i*3+1] = (1-dz) * valy0 + (dz) * valy1;
					vol[i*3+2] = (1-dz) * valy0 + (dz) * valy1;

				}

			}
		}
	}

	Anatomy* pNewAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( m_index );
	pNewAnatomy->setFloatDataset(vol);
	pNewAnatomy->generateTexture();
}

//////////////////////////////////////////////////////////////////////////////////////////
//Set texture from Network fmri: NOTE: doesnt work functionally yet, data should be set
//////////////////////////////////////////////////////////////////////////////////////////
void RestingStateNetwork::SetTextureFromNetwork()
{
	std::vector<float> texture(m_datasetSizeL*3, 0.0f);

	for(int x = 0; x < m_columnsL; x++)
	{
		for(int y = 0; y < m_rowsL; y++)
		{
			for(int z = 0; z < m_framesL; z++)
			{
				int i = z * m_columnsL * m_rowsL + y *m_columnsL + x;

				int zz = ((z - m_originL.z) * m_zL / m_voxelSizeZ) + m_origin.z;
				int yy = ((y - m_originL.y) * m_yL / m_voxelSizeY) + m_origin.y;
				int xx = ((x - m_originL.x) * m_xL / m_voxelSizeX) + m_origin.x;

				if(xx >=0 && yy >=0 && zz >=0 && xx < m_columns && yy < m_rows && zz < m_frames)
				{
					int s = zz * m_columns * m_rows + yy * m_columns + xx ; // O

					texture[i*3] = m_smallt[s*3];
					texture[i*3 + 1] = m_smallt[s*3+1];
					texture[i*3 + 2] = m_smallt[s*3+2];
				}

			}
		}
	}

	Anatomy* pNewAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( m_index );
	pNewAnatomy->setFloatDataset(texture);
	pNewAnatomy->generateTexture();

}

//////////////////////////////////////////////////////////////////////////////////////////
//Initiate the seed-based algorithm
//////////////////////////////////////////////////////////////////////////////////////////
void RestingStateNetwork::seedBased()
{
	m_3Dpoints.clear();
	m_smallt.assign(m_datasetSize*3,0.0f);

	m_zMin = 999.0f;
	m_zMax = 0.0f;
	m_boxMoving = true;
	 
	std::vector<float> positions; 

    Vector minCorner, maxCorner, middle;
    SelectionTree::SelectionObjectVector selObjs = SceneManager::getInstance()->getSelectionTree().getAllObjects();

	for( unsigned int b = 0; b < selObjs.size(); b++ )
	{
		minCorner.x = (int)(floor(selObjs[b]->getCenter().x - selObjs[b]->getSize().x * m_xL /  2.0f ) / m_xL );
		minCorner.y = (int)(floor(selObjs[b]->getCenter().y - selObjs[b]->getSize().y * m_yL /  2.0f ) / m_yL );
		minCorner.z = (int)(floor(selObjs[b]->getCenter().z - selObjs[b]->getSize().z * m_zL /  2.0f ) / m_zL );
		maxCorner.x = (int)(floor(selObjs[b]->getCenter().x + selObjs[b]->getSize().x * m_xL /  2.0f ) / m_xL );
		maxCorner.y = (int)(floor(selObjs[b]->getCenter().y + selObjs[b]->getSize().y * m_yL /  2.0f ) / m_yL );
		maxCorner.z = (int)(floor(selObjs[b]->getCenter().z + selObjs[b]->getSize().z * m_zL /  2.0f ) / m_zL );
		
		for( float x = minCorner.x; x <= maxCorner.x; x++)
		{
			for( float y = minCorner.y; y <= maxCorner.y; y++)
			{
				for( float z = minCorner.z; z <= maxCorner.z; z++)
				{
					//Switch to 3x3x3 from t1space
					int zz = ((z - m_originL.z) * m_zL / m_voxelSizeZ) + m_origin.z;
					int yy = ((y - m_originL.y) * m_yL/ m_voxelSizeY) + m_origin.y;
					int xx = ((x - m_originL.x) * m_xL /m_voxelSizeX) + m_origin.x;
					int i = zz * m_columns * m_rows + yy * m_columns + xx ; // O
					positions.push_back( i );
				}
			}
		}
		correlate(positions);
	}
	
	//TODO can be done in rendering directly while looping, change from fspace to t1space
    for(unsigned int s(0); s < m_3Dpoints.size(); ++s )
    {
		m_3Dpoints[s].first.x = ((m_3Dpoints[s].first.x - m_origin.x) * m_voxelSizeX / m_xL) + m_originL.x;
		m_3Dpoints[s].first.y = ((m_3Dpoints[s].first.y - m_origin.y) * m_voxelSizeY / m_yL) + m_originL.y;
		m_3Dpoints[s].first.z = ((m_3Dpoints[s].first.z - m_origin.z) * m_voxelSizeZ / m_zL) + m_originL.z;
    }

	render3D(false);
	RTFMRIHelper::getInstance()->setRTFMRIDirty(false);
}

namespace
{
template< class T > struct IndirectComp
{
    IndirectComp( const T &zvals ) :
        zvals( zvals )
    {
    }

    // Watch out: operator less, but we are sorting in descending z-order, i.e.,
    // highest z value will be first in array and painted first as well
    template< class I > bool operator()( const I &i1, const I &i2 ) const
    {
        return zvals[i1] > zvals[i2];
    }

private:
    const T &zvals;
};
}

//////////////////////////////////////////////////////////////////////////////////////////
//Rendering function, for both 3D sprites and textures options.
//////////////////////////////////////////////////////////////////////////////////////////
void RestingStateNetwork::render3D(bool recalculateTexture)
{
	if( m_3Dpoints.size() > 0 )
    {
		std::vector<float> texture(m_datasetSizeL*3, 0.0f);


        ////Test Sort////
        GLfloat projMatrix[16];
        glGetFloatv( GL_PROJECTION_MATRIX, projMatrix );
        size_t siz = m_3Dpoints.size();

    
        // Compute z values of lines (in our case: starting points only).
        vector< float > zVals( siz );


        unsigned int *pSnippletSort = NULL;
        pSnippletSort = new unsigned int[siz + 1];
      

        for(unsigned int i = 0; i < siz; ++i )
        {
            zVals[i] = ( m_3Dpoints[i].first.x * projMatrix[2] + m_3Dpoints[i].first.y * projMatrix[6]
                          + m_3Dpoints[i].first.z * projMatrix[10] + projMatrix[14] ) / ( m_3Dpoints[i].first.x * projMatrix[3]
                                  + m_3Dpoints[i].first.y * projMatrix[7] + m_3Dpoints[i].first.z * projMatrix[11] + projMatrix[15] );
            pSnippletSort[i] = i;
        }

        sort( &pSnippletSort[0], &pSnippletSort[siz], IndirectComp< vector< float > > ( zVals ) );

		//Apply ColorMap
		for (unsigned int ii = 0; ii < m_3Dpoints.size(); ii++)
		{
            unsigned int s = pSnippletSort[ii]; //id to render by Z
			float R,G,B;
            bool render = true;

			float mid = (m_zMin + m_zMax) / 2.0f;
			float quart = 1.0f* (m_zMin + m_zMax) / 4.0f;
			float trois_quart = 3.0f* (m_zMin + m_zMax) / 4.0f;
			float v = (m_3Dpoints[s].second - m_zMin) / (m_zMax - m_zMin);

			if(m_3Dpoints[s].second < quart)
			{
                R = (m_3Dpoints[s].second - m_zMin) / (quart - m_zMin);
                G = 0.0f;
                B = 0.0f;
                render = false;
			}
			else if(m_3Dpoints[s].second >= quart && m_3Dpoints[s].second < trois_quart)
			{
				R = 1.0f;
				G = (m_3Dpoints[s].second - quart) / (trois_quart - quart);
				B = 0.0f;
			}
			else
			{
				R = 1.0f;
				G = 1.0f;
				B = v;
			}

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_POINT_SPRITE);
			glPointSize(m_3Dpoints[s].second * m_pointSize + 1.0f);
			glColor4f(R,G,B,(m_3Dpoints[s].second / m_zMax) * m_alpha + 0.1f);

			//glActiveTexture(GL_TEXTURE0);
			//glEnable( GL_TEXTURE_2D );
			//glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
			//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
			//glBindTexture(GL_TEXTURE_2D, m_lookupTex);

            if(render)
            {
			    glBegin(GL_POINTS);
				    glVertex3f(m_3Dpoints[s].first.x * m_xL, m_3Dpoints[s].first.y * m_yL, m_3Dpoints[s].first.z * m_zL);
			    glEnd();
                render = true;
            }

			//glDisable( GL_TEXTURE_2D );
			glDisable(GL_POINT_SPRITE);
			glDisable(GL_BLEND);

			int zz = ((m_3Dpoints[s].first.z - m_originL.z) * m_zL / m_voxelSizeZ) + m_origin.z;
				int yy = ((m_3Dpoints[s].first.y - m_originL.y) * m_yL / m_voxelSizeY) + m_origin.y;
				int xx = ((m_3Dpoints[s].first.x - m_originL.x) * m_xL / m_voxelSizeX) + m_origin.x;

				int ss = zz * m_columns * m_rows + yy * m_columns + xx ; // O
				m_smallt[ss*3] = R;
				m_smallt[ss*3+1] = G;
				m_smallt[ss*3+2] = B;	
		}


		//TEXTURE
		//if(recalculateTexture)
		//{

		//	for(int x = 0; x < m_columnsL; x++)
		//	{
		//		for(int y = 0; y < m_rowsL; y++)
		//		{
		//			for(int z = 0; z < m_framesL; z++)
		//			{
		//				int i = z * m_columnsL * m_rowsL + y *m_columnsL + x;

		//				int zz = ((z - m_originL.z) * m_zL / m_voxelSizeZ) + m_origin.z;
		//				int yy = ((y - m_originL.y) * m_yL / m_voxelSizeY) + m_origin.y;
		//				int xx = ((x - m_originL.x) * m_xL / m_voxelSizeX) + m_origin.x;

		//				if(xx >=0 && yy >=0 && zz >=0 && xx < m_columns && yy < m_rows && zz < m_frames)
		//				{
		//					int s = zz * m_columns * m_rows + yy * m_columns + xx ; // O

		//					texture[i*3] = m_smallt[s*3];
		//					texture[i*3 + 1] = m_smallt[s*3+1];
		//					texture[i*3 + 2] = m_smallt[s*3+2];
		//				}

		//			}
		//		}
		//	}

		//	Anatomy* pNewAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( m_index );
		//	pNewAnatomy->setFloatDataset(texture);
		//	pNewAnatomy->generateTexture();

		//}
        delete[] pSnippletSort;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
//Correlation function given a position, with all other time series
//////////////////////////////////////////////////////////////////////////////////////////
void RestingStateNetwork::correlate(std::vector<float>& positions)
{
	//Mean signal inside box
	std::vector<float> meanSignal;
	for(int i=0; i < m_bands; i++)
	{
		float sum = 0;
		for(unsigned int j=0; j < positions.size(); j++)
		{	
			int idx = positions[j];
			sum += m_signalNormalized[idx][i];
		}
		sum /= positions.size();
		meanSignal.push_back( sum );
	}

	//Get mean and sigma of it
	std::pair<float, float> RefMeanAndSigma;
	calculateMeanAndSigma(meanSignal, RefMeanAndSigma);
	std::vector<float> corrFactors;
	corrFactors.assign(m_datasetSize, 0.0f);
	float corrSum = 0.0f;
	int nb = 0;

	//Correlate with rest of the brain, i.e find corr factors
	for( float x = 0; x < m_columns; x++)
	{
		for( float y = 0; y < m_rows; y++)
		{
			for( float z = 0; z < m_frames; z++)
			{
				int i = z * m_columns * m_rows + y *m_columns + x;
				if(m_meansAndSigmas[i].first != 0)
				{
					float num = 0.0f;
					float denum = 0.0f;
					
					for(int j = 0; j < m_bands; j++)
					{
						num += (meanSignal[j] - RefMeanAndSigma.first) * ( m_signalNormalized[i][j] - m_meansAndSigmas[i].first);
					}
					float value = num / ( RefMeanAndSigma.second * m_meansAndSigmas[i].second);
					value /= (m_bands);
				
					if(value > 0)
					{
						corrSum+=value;
						corrFactors[i] = value;
						nb++;
					}
					else
						corrFactors[i] = -1;
				}
				else
					corrFactors[i] = 0.0f;
			}
		}
	}

	//Find mean and sigma of all corr factors.
	float meanCorr = corrSum / nb;
	float sigma = 0.0f;
	for( float x = 0; x < m_columns; x++)
	{
		for( float y = 0; y < m_rows; y++)
		{
			for( float z = 0; z < m_frames; z++)
			{
				int i = z * m_columns * m_rows + y *m_columns + x;
				if(corrFactors[i] > 0.0f)
				{
					sigma += (corrFactors[i] - meanCorr)*(corrFactors[i] - meanCorr);	
				}		
			}
		}
	}

	//Calculate z-scores, and save them.
	sigma /= nb;
	sigma = sqrt(sigma);
	vector<float> zErode(m_datasetSize, 0);
	vector<bool> binErode(m_datasetSize, false);

	for( float x = 0; x < m_columns; x++)
	{
		for( float y = 0; y < m_rows; y++)
		{
			for( float z = 0; z < m_frames; z++)
			{
				int i = z * m_columns * m_rows + y *m_columns + x;
				
				if(m_corrThreshold == 0.0f && corrFactors[i] != 0)
				{	
					m_3Dpoints.push_back(std::pair<Vector,float>(Vector(x,y,z),0.0f));
				}

				if(corrFactors[i] > 0)
				{
					float zScore = (corrFactors[i] - meanCorr) / sigma;
					if(zScore < m_zMin && zScore > 0.0f)
						m_zMin = zScore;
					if(zScore > m_zMax)
						m_zMax = zScore;
					if(zScore > m_corrThreshold)
					{
						//m_3Dpoints.push_back(std::pair<Vector,float>(Vector(x,y,z),zScore));
						zErode[i] = zScore;
						binErode[i] = 1;
					}
				}
			}
		}
	}

	//vector<int> eroded = erode(toErode);
	vector<bool> tmp(m_datasetSize, false);
	for( float x = 1; x < m_columns-1; x++)
	{
		for( float y = 1; y < m_rows-1; y++)
		{
			for( float z = 1; z < m_frames-1; z++)
			{
				int i = z * m_columns * m_rows + y *m_columns + x;
				if(binErode[i])
				{
					erode(tmp,binErode,i);

					if(tmp[i])
					{
						m_3Dpoints.push_back(std::pair<Vector,float>(Vector(x,y,z),zErode[i]));
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//Export Zscore to T1 map for saving
//////////////////////////////////////////////////////////////////////////////////////////
std::vector<float>* RestingStateNetwork::getZscores()
{
	m_zMap.assign(m_datasetSizeL, 0.0f);
	std::vector<float> dataf(m_datasetSize, 0.0f);

	//First, create 3x3x3 texture with zscores instead of RGB values like done in render3d.
	for( size_t s = 0; s < m_3Dpoints.size(); s++)
	{
		int zz = ((m_3Dpoints[s].first.z - m_originL.z) * m_zL / m_voxelSizeZ) + m_origin.z;
		int yy = ((m_3Dpoints[s].first.y - m_originL.y) * m_yL / m_voxelSizeY) + m_origin.y;
		int xx = ((m_3Dpoints[s].first.x - m_originL.x) * m_xL / m_voxelSizeX) + m_origin.x;

		int ss = zz * m_columns * m_rows + yy * m_columns + xx ; // O
		dataf[ss] = m_3Dpoints[s].second;
	}

	//Then, generate 1x1x1 texture (trilinear can be done here)
	for(int x = 0; x < m_columnsL; x++)
	{
		for(int y = 0; y < m_rowsL; y++)
		{
			for(int z = 0; z < m_framesL; z++)
			{
				int i = z * m_columnsL * m_rowsL + y *m_columnsL + x;

				//int zz = ((z - m_originL.z) * m_zL / m_voxelSizeZ) + m_origin.z;
				//int yy = ((y - m_originL.y) * m_yL / m_voxelSizeY) + m_origin.y;
				//int xx = ((x - m_originL.x) * m_xL / m_voxelSizeX) + m_origin.x;

				//if(xx >=0 && yy >=0 && zz >=0 && xx <= m_columns && yy <= m_rows && zz <= m_frames)
				//{
				//	int s = zz * m_columns * m_rows + yy * m_columns + xx ; // O
				//	m_zMap[i] = dataf[s];
				//}

				float zz = ((z - m_originL.z) * m_zL / m_voxelSizeZ) + m_origin.z;
				float yy = ((y - m_originL.y) * m_yL / m_voxelSizeY) + m_origin.y;
				float xx = ((x - m_originL.x) * m_xL / m_voxelSizeX) + m_origin.x;

				if(xx >1 && yy >1 && zz >1 && xx < m_columns && yy < m_rows && zz < m_frames)
				{
                    const int x = (unsigned int) std::min( (int)std::floor(xx / DatasetManager::getInstance()->getVoxelX() ), m_columns-1 );
					const int y = (unsigned int) std::min( (int)std::floor(yy / DatasetManager::getInstance()->getVoxelY() ), m_rows-1 );
					const int z = (unsigned int) std::min( (int)std::floor(zz / DatasetManager::getInstance()->getVoxelZ() ), m_frames-1 );

					const float dx = ( xx / DatasetManager::getInstance()->getVoxelX() )-x;
					const float dy = ( yy / DatasetManager::getInstance()->getVoxelY() )-y;
					const float dz = ( zz / DatasetManager::getInstance()->getVoxelZ() )-z;

					const int nx = dx > 0.0 ? std::min( std::max(x+1,0), m_columns-1 ) : std::min( std::max(x,0), m_columns-1 );
					const int ny = dy > 0.0 ? std::min( std::max(y+1,0), m_rows-1 ) : std::min( std::max(y,0), m_rows-1 );
					const int nz = dz > 0.0 ? std::min( std::max(z+1,0), m_frames-1 ) : std::min( std::max(z,0), m_frames-1 );

					// Compute interpolated value at (fx,fy,fz)
					//Corresponding tensor number
					int tensor_xyz = z * m_columns * m_rows + y * m_columns + x;

					int tensor_nxyz = z * m_columns * m_rows + y * m_columns + nx;

					int tensor_xnyz = z * m_columns * m_rows + ny * m_columns + x;

					int tensor_nxnyz = z * m_columns * m_rows + ny * m_columns + nx;

					int tensor_xynz = nz * m_columns * m_rows + y * m_columns + x;

					int tensor_nxynz = nz * m_columns * m_rows + y * m_columns + nx;

					int tensor_xnynz = nz * m_columns * m_rows + ny * m_columns + x;

					int tensor_nxnynz = nz * m_columns * m_rows + ny * m_columns + nx;

					float valx0 = (1-dx) * dataf[tensor_xyz]  + (dx) * dataf[tensor_nxyz];
					float valx1 = (1-dx) * dataf[tensor_xnyz] + (dx) * dataf[tensor_nxnyz];

					const float valy0 = (1-dy) * valx0 + (dy) * valx1;
					valx0 = (1-dx) * dataf[tensor_xynz]  + (dx) * dataf[tensor_nxynz];
					valx1 = (1-dx) * dataf[tensor_xnynz] + (dx) * dataf[tensor_nxnynz];

					const float valy1 = (1-dy) * valx0 + (dy) * valx1;

					m_zMap[i] = (1-dz) * valy0 + (dz) * valy1;

				}
				
			}
		}
	}

	return &m_zMap;
}

float RestingStateNetwork::getElement(int i,int j,int k, std::vector<float>* vect)
{
    return (*vect)[ i + ( j * m_columnsL ) + ( k * m_rowsL * m_columnsL ) ];
}

vector<vector<float>* > RestingStateNetwork::getClusters()
{
    vector<vector<float>*> clusters;
    vector<float>* zMap = getZscores();
    vector<float> visited(m_datasetSizeL,0.0f);
    
    bool first = true;
    bool foundClust = false;
    
    //Intensity of the current voxel

    std::list<Vector> toVisit;
    int north, south, east, west, front, back, x, y, z;
    float NorthV, EastV, SouthV, WestV, FrontV, BackV;
    float resultNorth, resultEast, resultSouth, resultWest, resultFront, resultBack;

    //Add pixel to the top
    for(int xx = 0; xx < m_columnsL; xx++)
	{
		for(int yy = 0; yy < m_rowsL; yy++)
		{
			for(int zz = 0; zz < m_framesL; zz++)
			{
                vector<float>* singleClust = new vector<float>;

				int i = zz * m_columnsL * m_rowsL + yy *m_columnsL + xx;

                if(zMap->at(i) != 0 && first)
                {  
                    singleClust->assign(m_datasetSizeL,0.0f);
                    visited[i] = 1.0f;
                    singleClust->at( i ) = 1.0f; //Mark as read
                    toVisit.push_front(Vector(xx,yy,zz));
                    first = false;
                }
                else if(zMap->at(i) != 0 && visited[i] == 0)
                {
                    singleClust->assign(m_datasetSizeL,0.0f);
                    visited[i] = 1.0f;
                    singleClust->at( i ) = 1.0f; //Mark as read
                    toVisit.push_front(Vector(xx,yy,zz));
                }

                //While there's still pixel to visit
                while(!toVisit.empty())
                {
                    foundClust = true;
                    x = toVisit.front()[0];
                    y = toVisit.front()[1];
                    z = toVisit.front()[2];
                    toVisit.pop_front();

                    singleClust->at( x + y * m_columnsL + z * m_rowsL * m_columnsL ) = 1.0f; //Mark as read
                    visited[x + y * m_columnsL + z * m_rowsL * m_columnsL] = 1.0f;

                    north = std::max( 0, y - 1 );
                    south = std::min( m_rowsL - 1, y + 1 );
                    east  = std::min( m_columnsL - 1, x + 1 );
                    west  = std::max( 0, x - 1 );
                    front = std::max( 0, z - 1 );
                    back  = std::min( m_framesL - 1, z + 1 );

                    NorthV = getElement( x, north, z, zMap );
                    SouthV = getElement( x, south, z, zMap );
                    EastV = getElement( east, y, z, zMap );
                    WestV = getElement( west, y, z, zMap );
                    FrontV = getElement( x, y, front, zMap );
                    BackV = getElement( x, y, back, zMap );

                    resultNorth = getElement( x, north, z, singleClust );
                    resultSouth = getElement( x, south, z, singleClust );
                    resultEast = getElement( east, y, z, singleClust );
                    resultWest = getElement( west, y, z, singleClust );
                    resultFront = getElement( x, y, front, singleClust );
                    resultBack = getElement( x, y, back, singleClust );

                    if(NorthV > 0.0f && resultNorth != 1.0f) //North
                    {
                        toVisit.push_front(Vector(x,north,z));
                    }

                    if(SouthV > 0.0f && resultSouth != 1.0f) //South
                    {
                        toVisit.push_front(Vector(x,south,z));
                    }

                    if(EastV > 0.0f && resultEast != 1.0f) //East
                    {
                        toVisit.push_front(Vector(east,y,z));
                    }

                    if(WestV > 0.0f && resultWest != 1.0f) //West
                    {
                        toVisit.push_front(Vector(west,y,z));
                    }

                    if(FrontV > 0.0f && resultFront != 1.0f) //Front
                    {
                        toVisit.push_front(Vector(x,y,front));
                    }

                    if(BackV > 0.0f && resultBack != 1.0f) //Back
                    {
                        toVisit.push_front(Vector(x,y,back));
                    }
                }

                if(foundClust)
                {
                    clusters.push_back(singleClust);
                    foundClust = false;
                }
            }
        }
    }
    return clusters;
}

void RestingStateNetwork::erode(std::vector<bool> &tmp, const std::vector<bool> &inMap, int curIndex)
{
	float acc  = inMap[curIndex - 1]
    + inMap[curIndex + 1]
    + inMap[curIndex - m_columns - 1]
    + inMap[curIndex - m_columns]
    + inMap[curIndex - m_columns + 1]
    + inMap[curIndex + m_columns - 1]
    + inMap[curIndex + m_columns]
    + inMap[curIndex + m_columns + 1]
    + inMap[curIndex - m_columns * m_rows - 1]
    + inMap[curIndex - m_columns * m_rows]
    + inMap[curIndex - m_columns * m_rows + 1]
    + inMap[curIndex + m_columns * m_rows - 1]
    + inMap[curIndex + m_columns * m_rows]
    + inMap[curIndex + m_columns * m_rows + 1]
    + inMap[curIndex - m_columns * m_rows - m_columns]
    + inMap[curIndex - m_columns * m_rows + m_columns]
    + inMap[curIndex + m_columns * m_rows - m_columns]
    + inMap[curIndex + m_columns * m_rows + m_columns];

    if( acc > m_clusterLvlSliderValue )
    {
        tmp.at( curIndex ) = true;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
//Calculate Mean and Sigma for the signal inside the box
//////////////////////////////////////////////////////////////////////////////////////////
void RestingStateNetwork::calculateMeanAndSigma(std::vector<float> signal, std::pair<float, float>& params)
{
	float mean = 0.0f;
	float sigma = 0.0f;
	
	//mean
	for(unsigned int i=0; i < signal.size(); i++)
	{
		mean+=signal[i];
	}
	mean /= signal.size();

	//sigma
    for(unsigned int i = 0; i < signal.size(); i++)
    {
         sigma += (signal[i] - mean) * (signal[i] - mean) ;
    }
    sigma /= signal.size();

	params.first = mean;
	params.second = sqrt(sigma);
}