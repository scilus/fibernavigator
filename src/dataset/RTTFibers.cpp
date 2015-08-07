/*
 *  The RTTFibers class implementation.
 *
 */

#include "RTTFibers.h"

#include "DatasetManager.h"
#include "RTTrackingHelper.h"
#include "RTFMRIHelper.h"
#include "../Logger.h"
#include "../gfx/ShaderHelper.h"
#include "../gfx/TheScene.h"
#include "../gui/SceneManager.h"
#include "../gui/SelectionTree.h"
#include "../misc/lic/FgeOffscreen.h"
#include "../gui/MainFrame.h"
#include "../misc/IsoSurface/CIsoSurface.h"
#include "../misc/IsoSurface/TriangleMesh.h"


#include <algorithm>
using std::sort;

#include <vector>
using std::vector;
#include "../main.h"

//////////////////////////////////////////
//Constructor
//////////////////////////////////////////
RTTFibers::RTTFibers()
:   m_trackActionStep(std::numeric_limits<unsigned int>::max()),
    m_timerStep( 0 ),
    m_FAThreshold( 0.20f ),
    m_angleThreshold( 35.0f ),
    m_step( 1.0f ),
    m_GMstep( 15 ),
    m_nbSeed ( 10.0f ),
    m_nbMeshPt ( 0 ),
    m_puncture( 0.2f ),
    m_vinvout( 0.2f ),
    m_minFiberLength( 60 ),
    m_maxFiberLength( 200 ),
	m_alpha( 1.0f ),
    m_isHARDI( false ),
    m_countGMstep( 0 ),
	m_stop( false ),
    m_pExcludeInfo( NULL ),
    m_pSeedMapInfo( NULL ),
    m_pGMInfo( NULL )
{
}

void RTTFibers::setSeedMapInfo(Anatomy *info)
{
	m_pSeedMap.clear();
	m_pSeedMapInfo = info; 
	if( m_pSeedMapInfo != NULL )
	{
		int cols = DatasetManager::getInstance()->getColumns();
		int rows = DatasetManager::getInstance()->getRows();
		int frames = DatasetManager::getInstance()->getFrames();

        for( int f = 0; f < frames; ++f )
		{
			for( int r = 0; r < rows; ++r )
			{
				for( int c = 0; c < cols; ++c )
				{
					if ( m_pSeedMapInfo->at(f * rows * cols + r * cols + c) > 0.0f)
					{
						m_pSeedMap.push_back( Vector(c, r, f) );
					}
				}
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////
// Generate random seeds
///////////////////////////////////////////////////////////////////////////
Vector RTTFibers::generateRandomSeed( const Vector &min, const Vector &max )
{
    float randomX = ( (float) rand() ) / (float) RAND_MAX;
    float rangeX = max.x - min.x;  
    float seedX = ( randomX * rangeX ) + min.x;

    float randomY = ( (float) rand() ) / (float) RAND_MAX;
    float rangeY = max.y - min.y;  
    float seedY = ( randomY * rangeY ) + min.y;

    float randomZ = ( (float) rand() ) / (float) RAND_MAX;
    float rangeZ = max.z - min.z;  
    float seedZ = ( randomZ * rangeZ ) + min.z;

    return Vector( seedX, seedY, seedZ );
}
///////////////////////////////////////////////////////////////////////////
// Returns the nb of vertices for shell seeding
///////////////////////////////////////////////////////////////////////////
float RTTFibers::getShellSeedNb()
{
	float pts = m_nbSeed*m_nbSeed*m_nbSeed;
	if( m_pShellInfo != NULL )
	{
		CIsoSurface* pSurf = (CIsoSurface*) m_pShellInfo;
		std::vector< Vector > positions = pSurf->m_tMesh->getVerts();
		pts = positions.size();
	}
    return pts;
}

void RTTFibers::insert(std::vector<Vector> pointsF, std::vector<Vector> pointsB, std::vector<Vector> colorF, std::vector<Vector> colorB)
{
	if( (pointsF.size() + pointsB.size()) * getStep() > getMinFiberLength() && (pointsF.size() + pointsB.size()) * getStep() < getMaxFiberLength() )
	{
		m_fibersRTT.push_back( pointsF ); 
		m_colorsRTT.push_back( colorF );
		m_fibersRTT.push_back( pointsB ); 
		m_colorsRTT.push_back( colorB );
	}
}
///////////////////////////////////////////////////////////////////////////
// Returns the nb of seeds for Map seeding
///////////////////////////////////////////////////////////////////////////
float RTTFibers::getSeedMapNb()
{
	float pts = m_nbSeed*m_nbSeed*m_nbSeed;
	if( m_pSeedMapInfo != NULL )
	{
		pts *= m_pSeedMap.size();
	}
    return pts;
}
///////////////////////////////////////////////////////////////////////////
// Generate seeds and tracks
///////////////////////////////////////////////////////////////////////////
void RTTFibers::seed()
{
    m_fibersRTT.clear();
    m_colorsRTT.clear();
	 
    float xVoxel = DatasetManager::getInstance()->getVoxelX();
    float yVoxel = DatasetManager::getInstance()->getVoxelY();
    float zVoxel = DatasetManager::getInstance()->getVoxelZ();

    Vector minCorner, maxCorner, middle;
    SelectionTree::SelectionObjectVector selObjs = SceneManager::getInstance()->getSelectionTree().getAllObjects();

	//Evenly distanced seeds
	if( !RTTrackingHelper::getInstance()->isShellSeeds() && !RTTrackingHelper::getInstance()->isSeedMap() && !RTTrackingHelper::getInstance()->isSeedFromfMRI())
	{
		for( unsigned int b = 0; b < selObjs.size(); b++ )
		{
            if( selObjs[ b ]->getSelectionType() != BOX_TYPE )
            {
                continue;
            }
            
			minCorner.x = selObjs[b]->getCenter().x - selObjs[b]->getSize().x * xVoxel / 2.0f;
			minCorner.y = selObjs[b]->getCenter().y - selObjs[b]->getSize().y * yVoxel / 2.0f;
			minCorner.z = selObjs[b]->getCenter().z - selObjs[b]->getSize().z * zVoxel / 2.0f;
			maxCorner.x = selObjs[b]->getCenter().x + selObjs[b]->getSize().x * xVoxel / 2.0f;
			maxCorner.y = selObjs[b]->getCenter().y + selObjs[b]->getSize().y * yVoxel / 2.0f;
			maxCorner.z = selObjs[b]->getCenter().z + selObjs[b]->getSize().z * zVoxel / 2.0f;

			float xstep =  selObjs[b]->getSize().x * xVoxel / float( m_nbSeed - 1.0f );
			float ystep =  selObjs[b]->getSize().y * yVoxel / float( m_nbSeed - 1.0f );
			float zstep =  selObjs[b]->getSize().z * zVoxel / float( m_nbSeed - 1.0f );
			
			for( float x = minCorner.x; x < maxCorner.x + xstep/2.0f; x+= xstep )
			{
				for( float y = minCorner.y; y < maxCorner.y + ystep/2.0f; y+= ystep )
				{
					for( float z = minCorner.z; z < maxCorner.z + zstep/2.0f; z+= zstep )
					{
						vector<Vector> pointsF; // Points to be rendered Forward
						vector<Vector> colorF; //Color (local directions)Forward
						vector<Vector> pointsB; // Points to be rendered Backward
						vector<Vector> colorB; //Color (local directions) Backward
						m_stop = false;
                        
                        if(m_isHARDI)
                        {
						    //Track both sides
							performHARDIRTT( Vector(x,y,z),  1, pointsF, colorF); //First pass
						    performHARDIRTT( Vector(x,y,z), -1, pointsB, colorB); //Second pass
                        }
                        else
                        {
						    //Track both sides
						    performDTIRTT( Vector(x,y,z),  1, pointsF, colorF); //First pass
						    performDTIRTT( Vector(x,y,z), -1, pointsB, colorB); //Second pass
                        }
                        
						if( (pointsF.size() + pointsB.size()) * getStep() > getMinFiberLength() && (pointsF.size() + pointsB.size()) * getStep() < getMaxFiberLength() && !m_stop)
						{
							m_fibersRTT.push_back( pointsF ); 
							m_colorsRTT.push_back( colorF );
							m_fibersRTT.push_back( pointsB ); 
							m_colorsRTT.push_back( colorB );
						}
					}
				}
			}
		}
	}
	else if ( RTTrackingHelper::getInstance()->isSeedMap())
	{
		for( unsigned int s = 0; s < m_pSeedMap.size(); s++ )
		{ 
			Vector val = m_pSeedMap[s];	
			
			minCorner.x = val.x * xVoxel;
			minCorner.y = val.y * yVoxel;
			minCorner.z = val.z * zVoxel;
			maxCorner.x = val.x * xVoxel + xVoxel;
			maxCorner.y = val.y * yVoxel + yVoxel;
			maxCorner.z = val.z * zVoxel + zVoxel;

			float xstep =  xVoxel / float( m_nbSeed - 1.0f );
			float ystep =  yVoxel / float( m_nbSeed - 1.0f );
			float zstep =  zVoxel / float( m_nbSeed - 1.0f );

			for( float x = minCorner.x; x < maxCorner.x + xstep/2.0f; x+= xstep )
			{
				for( float y = minCorner.y; y < maxCorner.y + ystep/2.0f; y+= ystep )
				{
					for( float z = minCorner.z; z < maxCorner.z + zstep/2.0f; z+= zstep )
					{
						vector<Vector> pointsF; // Points to be rendered Forward
						vector<Vector> colorF; //Color (local directions)Forward
						vector<Vector> pointsB; // Points to be rendered Backward
						vector<Vector> colorB; //Color (local directions) Backward
						m_stop = false;
                        
                        if(m_isHARDI)
                        {
						    //Track both sides
							performHARDIRTT( Vector(x,y,z),  1, pointsF, colorF); //First pass
						    performHARDIRTT( Vector(x,y,z), -1, pointsB, colorB); //Second pass
                        }
                        else
                        {
						    //Track both sides
						    performDTIRTT( Vector(x,y,z),  1, pointsF, colorF); //First pass
						    performDTIRTT( Vector(x,y,z), -1, pointsB, colorB); //Second pass
                        }
                        
						if( (pointsF.size() + pointsB.size()) * getStep() > getMinFiberLength() && (pointsF.size() + pointsB.size()) * getStep() < getMaxFiberLength() && !m_stop)
						{
							m_fibersRTT.push_back( pointsF ); 
							m_colorsRTT.push_back( colorF );
							m_fibersRTT.push_back( pointsB ); 
							m_colorsRTT.push_back( colorB );
						}
					}
				}
			}
		}
	}
	//fMRI seeding
	else if(RTTrackingHelper::getInstance()->isSeedFromfMRI())
	{
		RTTrackingHelper::getInstance()->m_pSliderAxisSeedNb->SetValue( m_nbSeed );
		RTTrackingHelper::getInstance()->m_pTxtAxisSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), m_nbSeed) );
        RTTrackingHelper::getInstance()->m_pTxtTotalSeedNbBox->SetValue(wxString::Format( wxT( "%.1f"), m_nbSeed*m_nbSeed*m_nbSeed*m_pSeedFromfMRI.size()) );

		for( size_t k = 0; k < m_pSeedFromfMRI.size(); k++)
		{
			float xstep =  xVoxel / float( m_nbSeed - 1.0f );
			float ystep =  yVoxel / float( m_nbSeed - 1.0f );
			float zstep =  zVoxel / float( m_nbSeed - 1.0f );

			float xx = m_pSeedFromfMRI[k].first.x * DatasetManager::getInstance()->getVoxelX();
			float yy = m_pSeedFromfMRI[k].first.y * DatasetManager::getInstance()->getVoxelY();
			float zz = m_pSeedFromfMRI[k].first.z * DatasetManager::getInstance()->getVoxelZ();

			for( float x = xx - xVoxel; x < xx + xVoxel + xstep/2.0f; x+= xstep )
			{
				for( float y = yy - yVoxel; y < yy + yVoxel + ystep/2.0f; y+= ystep )
				{
					for( float z = zz - zVoxel; z < zz + zVoxel + zstep/2.0f; z+= zstep )
					{

						vector<Vector> pointsF; // Points to be rendered Forward
						vector<Vector> colorF; //Color (local directions)Forward
						vector<Vector> pointsB; // Points to be rendered Backward
						vector<Vector> colorB; //Color (local directions) Backward
                        
						if(m_isHARDI)
						{
							//Track both sides
							performHARDIRTT( Vector(x,y,z),  1, pointsF, colorF); //First pass
							performHARDIRTT( Vector(x,y,z), -1, pointsB, colorB); //Second pass
						}
						else
						{
							//Track both sides
							performDTIRTT( Vector(x,y,z),  1, pointsF, colorF); //First pass
							performDTIRTT( Vector(x,y,z), -1, pointsB, colorB); //Second pass
						}
                        
						if( (pointsF.size() + pointsB.size()) * getStep() > getMinFiberLength() && (pointsF.size() + pointsB.size()) * getStep() < getMaxFiberLength() )
						{
							m_fibersRTT.push_back( pointsF ); 
							m_colorsRTT.push_back( colorF );
							m_fibersRTT.push_back( pointsB ); 
							m_colorsRTT.push_back( colorB );
						}
					}
				}
			}
		}
	}
    //Mesh ShellSeeding
    else 
    {
        if ( m_pShellInfo->getType() == ISO_SURFACE )
        {
            CIsoSurface* pSurf = (CIsoSurface*) m_pShellInfo;
            std::vector< Vector > positions = pSurf->m_tMesh->getVerts();

            m_nbMeshPt = positions.size();

            for ( size_t k = 0; k < positions.size(); ++k )
            {
                vector<Vector> pointsF; // Points to be rendered Forward
				vector<Vector> colorF; //Color (local directions)Forward
				vector<Vector> pointsB; // Points to be rendered Backward
				vector<Vector> colorB; //Color (local directions) Backward
				m_stop = false;
                        
                if(m_isHARDI)
                {
					//Track both sides
					performHARDIRTT( Vector(positions[k].x,positions[k].y,positions[k].z),  1, pointsF, colorF); //First pass
					performHARDIRTT( Vector(positions[k].x,positions[k].y,positions[k].z), -1, pointsB, colorB); //Second pass
                }
                else
                {
					//Track both sides
					performDTIRTT( Vector(positions[k].x,positions[k].y,positions[k].z),  1, pointsF, colorF); //First pass
					performDTIRTT( Vector(positions[k].x,positions[k].y,positions[k].z), -1, pointsB, colorB); //Second pass
                }
                        
				if( (pointsF.size() + pointsB.size()) * getStep() > getMinFiberLength() && (pointsF.size() + pointsB.size()) * getStep() < getMaxFiberLength() && !m_stop)
				{
					m_fibersRTT.push_back( pointsF ); 
					m_colorsRTT.push_back( colorF );
					m_fibersRTT.push_back( pointsB ); 
					m_colorsRTT.push_back( colorB );
				}
            }
        }
	}
    renderRTTFibers(false);
	
	RTTrackingHelper::getInstance()->setRTTDirty( false );
}

    

///////////////////////////////////////////////////////////////////////////
//Rendering stage
///////////////////////////////////////////////////////////////////////////
void RTTFibers::renderRTTFibers(bool isPlaying)
{
    if(!isPlaying)
		m_trackActionStep = std::numeric_limits<unsigned int>::max();

    if( m_fibersRTT.size() > 0 )
    {
        glPushAttrib( GL_ALL_ATTRIB_BITS );
		if(m_alpha != 1.0f)
		{
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			glDepthMask( GL_FALSE );
		}
        //for fmri
        std::vector<Vector> positions; 
	    for( unsigned int j = 0; j < m_fibersRTT.size() - 1; j+=2 )
	    { 

		    //POINTS
		    if( SceneManager::getInstance()->isPointMode() )
		    {
			    //Forward
			    if( m_fibersRTT[j].size() > 0 )
			    {
					for( unsigned int i = 0; i < std::min((unsigned int)m_fibersRTT[j].size(), m_trackActionStep); i++ )
				    {
                        
					    glColor4f( std::abs(m_colorsRTT[j][i].x), std::abs(m_colorsRTT[j][i].y), std::abs(m_colorsRTT[j][i].z), m_alpha );
					    glBegin( GL_POINTS );
						    glVertex3f( m_fibersRTT[j][i].x, m_fibersRTT[j][i].y, m_fibersRTT[j][i].z );
					    glEnd();
				    }
			    }
			    //Backward
			    if(m_fibersRTT[j+1].size() > 0)
			    {
					for( unsigned int i = 0; i < std::min((unsigned int)m_fibersRTT[j+1].size(), m_trackActionStep); i++ )
				    {  
					    glColor4f( std::abs(m_colorsRTT[j+1][i].x), std::abs(m_colorsRTT[j+1][i].y), std::abs(m_colorsRTT[j+1][i].z), m_alpha );
					    glBegin( GL_POINTS );
						    glVertex3f( m_fibersRTT[j+1][i].x, m_fibersRTT[j+1][i].y, m_fibersRTT[j+1][i].z );
					    glEnd();
				    }
			    }
		    }
		    //LINES
		    else
		    {
			    //Forward
			    if( m_fibersRTT[j].size() > 2)
			    {
                    for( unsigned int i = 0; i < std::min((unsigned int)m_fibersRTT[j].size() - 1,m_trackActionStep); i++ )
				    {
                        if(i > m_fibersRTT[j].size() - 5 && RTTrackingHelper::getInstance()->isTractoDrivenRSN())
                        {
                            glPointSize(10.0f);
                            glColor3f( 1.0f,0.0f,0.0f );
					        glBegin( GL_POINTS );
						        glVertex3f( m_fibersRTT[j][i].x, m_fibersRTT[j][i].y, m_fibersRTT[j][i].z );
					        glEnd();
                            positions.push_back(Vector(m_fibersRTT[j][i].x,m_fibersRTT[j][i].y,m_fibersRTT[j][i].z));
                        }
                        else
                        {
                            glPointSize(1.0f);
                            glColor4f( std::abs(m_colorsRTT[j][i].x), std::abs(m_colorsRTT[j][i].y), std::abs(m_colorsRTT[j][i].z), m_alpha );
					        glBegin( GL_LINES );
						        glVertex3f( m_fibersRTT[j][i].x, m_fibersRTT[j][i].y, m_fibersRTT[j][i].z );
						        glVertex3f( m_fibersRTT[j][i+1].x, m_fibersRTT[j][i+1].y, m_fibersRTT[j][i+1].z );
					        glEnd();
                        }
					    
				    }
			    }
			    //Backward
			    if ( m_fibersRTT[j+1].size() > 2)
			    {
                    for( unsigned int i = 0; i < std::min((unsigned int)m_fibersRTT[j+1].size() - 1, m_trackActionStep); i++ )
				    {
                        if(i > m_fibersRTT[j+1].size() - 5 && RTTrackingHelper::getInstance()->isTractoDrivenRSN())
                        {
                            glPointSize(10.0f);
                            glColor3f( 1.0f, 0.0f, 0.0f );
					        glBegin( GL_POINTS );
						        glVertex3f( m_fibersRTT[j+1][i].x, m_fibersRTT[j+1][i].y, m_fibersRTT[j+1][i].z );
					        glEnd();
                            positions.push_back(Vector(m_fibersRTT[j+1][i].x,m_fibersRTT[j+1][i].y,m_fibersRTT[j+1][i].z));
                        }
                        else
                        {
                            glPointSize(1.0f);
					        glColor4f( std::abs(m_colorsRTT[j+1][i].x), std::abs(m_colorsRTT[j+1][i].y), std::abs(m_colorsRTT[j+1][i].z), m_alpha );
					        glBegin( GL_LINES );
						        glVertex3f( m_fibersRTT[j+1][i].x, m_fibersRTT[j+1][i].y, m_fibersRTT[j+1][i].z );
						        glVertex3f( m_fibersRTT[j+1][i+1].x, m_fibersRTT[j+1][i+1].y, m_fibersRTT[j+1][i+1].z );
					        glEnd();
                        }
				    }
			    }
				
		    }   
	    }
        glDisable(GL_BLEND);
		glPopAttrib();
        if(RTTrackingHelper::getInstance()->isTractoDrivenRSN())
        {
            DatasetManager::getInstance()->m_pRestingStateNetwork->setSeedFromTracto(positions);
            RTFMRIHelper::getInstance()->setRTFMRIDirty(true);
        }
	}   
}

///////////////////////////////////////////////////////////////////////////
// Trilinear interpolation for realtime tracking (tensors)
///////////////////////////////////////////////////////////////////////////
FMatrix RTTFibers::trilinearInterp( float fx, float fy, float fz )
{

    using std::min;
    using std::max;

    int columns = DatasetManager::getInstance()->getColumns();
    int rows    = DatasetManager::getInstance()->getRows();
    int frames  = DatasetManager::getInstance()->getFrames();

    const int x = (unsigned int) min( (int)std::floor(fx / DatasetManager::getInstance()->getVoxelX() ), columns-1 );
    const int y = (unsigned int) min( (int)std::floor(fy / DatasetManager::getInstance()->getVoxelY() ), rows-1 );
    const int z = (unsigned int) min( (int)std::floor(fz / DatasetManager::getInstance()->getVoxelZ() ), frames-1 );

    const float dx = ( fx / DatasetManager::getInstance()->getVoxelX() )-x;
    const float dy = ( fy / DatasetManager::getInstance()->getVoxelY() )-y;
    const float dz = ( fz / DatasetManager::getInstance()->getVoxelZ() )-z;

    const int nx = dx > 0.0 ? min( max(x+1,0), columns-1 ) : min( max(x,0), columns-1 );
    const int ny = dy > 0.0 ? min( max(y+1,0), rows-1 ) : min( max(y,0), rows-1 );
    const int nz = dz > 0.0 ? min( max(z+1,0), frames-1 ) : min( max(z,0), frames-1 );

    // Compute interpolated value at (fx,fy,fz)
    //Corresponding tensor number
    int tensor_xyz = z * columns * rows + y * columns + x;

    int tensor_nxyz = z * columns * rows + y * columns + nx;

    int tensor_xnyz = z * columns * rows + ny * columns + x;

    int tensor_nxnyz = z * columns * rows + ny * columns + nx;

    int tensor_xynz = nz * columns * rows + y * columns + x;

    int tensor_nxynz = nz * columns * rows + y * columns + nx;

    int tensor_xnynz = nz * columns * rows + ny * columns + x;

    int tensor_nxnynz = nz * columns * rows + ny * columns + nx;

    FMatrix valx0 = (1-dx) * m_pTensorsInfo->getTensorsMatrix()->at(tensor_xyz)  + (dx) * m_pTensorsInfo->getTensorsMatrix()->at(tensor_nxyz);
    FMatrix valx1 = (1-dx) * m_pTensorsInfo->getTensorsMatrix()->at(tensor_xnyz) + (dx) * m_pTensorsInfo->getTensorsMatrix()->at(tensor_nxnyz);

    const FMatrix valy0 = (1-dy) * valx0 + (dy) * valx1;
    valx0 = (1-dx) * m_pTensorsInfo->getTensorsMatrix()->at(tensor_xynz)  + (dx) * m_pTensorsInfo->getTensorsMatrix()->at(tensor_nxynz);
    valx1 = (1-dx) * m_pTensorsInfo->getTensorsMatrix()->at(tensor_xnynz) + (dx) * m_pTensorsInfo->getTensorsMatrix()->at(tensor_nxnynz);

    const FMatrix valy1 = (1-dy) * valx0 + (dy) * valx1;

    return (1-dz) * valy0 + (dz) * valy1;
}

/////////////////////////////////////////////////////////////////////
// Advection integration
// Returns the next direction for RTT
////////////////////////////////////////////////////////////////////
Vector RTTFibers::advecIntegrate( Vector vin, const FMatrix &tensor, Vector e1, Vector e2, Vector e3, float t_number ) 
{
    Vector vout, vprop, ee1, ee2, ee3;
    float dp1, dp2, dp3;
    float cl = m_pTensorsInfo->getTensorsFA()->at(t_number);
    float puncture = getPuncture();

    GLfloat flippedAxes[3];
    m_pTensorsInfo->isAxisFlipped(X_AXIS) ? flippedAxes[0] = -1.0f : flippedAxes[0] = 1.0f;
    m_pTensorsInfo->isAxisFlipped(Y_AXIS) ? flippedAxes[1] = -1.0f : flippedAxes[1] = 1.0f;
    m_pTensorsInfo->isAxisFlipped(Z_AXIS) ? flippedAxes[2] = -1.0f : flippedAxes[2] = 1.0f;

    // Unit vectors of local basis (e1 > e2 > e3)
    ee1.x = flippedAxes[0] * (tensor(0,0) * e1.x + 
            tensor(0,1) * e1.y + 
            tensor(0,2) * e1.z);

    ee1.y = flippedAxes[1] * (tensor(1,0) * e1.x + 
            tensor(1,1) * e1.y + 
            tensor(1,2) * e1.z);

    ee1.z = flippedAxes[2] * (tensor(2,0) * e1.x +
            tensor(2,1) * e1.y + 
            tensor(2,2) * e1.z);
    //e2
    ee2.x = flippedAxes[0] * (tensor(0,0) * e2.x + 
            tensor(0,1) * e2.y + 
            tensor(0,2) * e2.z);

    ee2.y = flippedAxes[1] * (tensor(1,0) * e2.x + 
            tensor(1,1) * e2.y + 
            tensor(1,2) * e2.z);

    ee2.z = flippedAxes[2] * (tensor(2,0) * e2.x +
            tensor(2,1) * e2.y + 
            tensor(2,2) * e2.z);
    //e3
    ee3.x = flippedAxes[0] * (tensor(0,0) * e3.x + 
            tensor(0,1) * e3.y + 
            tensor(0,2) * e3.z);

    ee3.y = flippedAxes[1] * (tensor(1,0) * e3.x + 
            tensor(1,1) * e3.y + 
            tensor(1,2) * e3.z);

    ee3.z = flippedAxes[2] * (tensor(2,0) * e3.x +
            tensor(2,1) * e3.y + 
            tensor(2,2) * e3.z);

    if( vin.Dot(ee1) < 0.0 )
    {
      ee1 *= -1;
    }
    
    ee1.normalize();
    ee2.normalize();
    ee3.normalize();

    // Lets first expand vin in local basis
    dp1 = vin.Dot(ee1);
    dp2 = vin.Dot(ee2);
    dp3 = vin.Dot(ee3);

    //Sort eigen values
    float eValues[] = { m_pTensorsInfo->getTensorsEV()->at(t_number)[0], m_pTensorsInfo->getTensorsEV()->at(t_number)[1], m_pTensorsInfo->getTensorsEV()->at(t_number)[2] };
    sort( eValues, eValues+3 );

    // Compute vout
    vout = dp1 * eValues[0] * ee1 + dp2 * eValues[1] * ee2 + dp3 * eValues[2] * ee3;

    // Normalization (see Lazar paper for more explanations)
    vout.normalize();
    ee1.normalize();
    vin.normalize();

    vprop = cl * ee1 + (1.0 - cl) * ( (1.0 - puncture) * vin + puncture * vout );
    return vprop;
}

Vector RTTFibers::advecIntegrateHARDI( Vector vin, const std::vector<float> &sticks, float s_number, Vector pos ) 
{
    Vector vOut(0,0,0);
    float angleMin = 360.0f;
    float angle = 0.0f;
    float puncture = m_vinvout;
    float wm = m_pMaskInfo->at(s_number);
    float gm = 0;
    if(m_pGMInfo != NULL)
    {
        gm = m_pGMInfo->at(s_number);
    }
	vin.normalize();

    for(unsigned int i=0; i < sticks.size()/3; i++)
    {
        Vector v1(sticks[i*3],sticks[i*3+1], sticks[i*3+2]);
        v1.normalize();
        
        if( vin.Dot(v1) < 0 ) //Ensures both vectors points in the same direction
        {
            v1 *= -1;
        }

        //Angle value
        float dot = vin.Dot(v1);
        float acos = std::acos( dot );
        angle = 180 * acos / M_PI;
        
        //Direction most probable
        if( angle < angleMin )
        {
            angleMin = angle;
            vOut = v1;
        }     
    }

    Vector res = 0.5f * wm * vOut + (0.5f * wm) * ( (1.0 - puncture ) * vin + puncture * vOut);
    
    if(gm != 0)
    {
        res = (1.0 - gm) * vOut + (gm) * ( (1.0 - puncture ) * vin + puncture * vOut);
    }

    //MAGNET
    bool isMagnetOn = RTTrackingHelper::getInstance()->isMagnetOn();
    if(isMagnetOn)
    {
        res = magneticField(vin, sticks, s_number, pos ); 
    }
    
        
    return res;
}

Vector RTTFibers::magneticField(Vector vin, const std::vector<float> &sticks, float s_number, Vector pos ) 
{
    SelectionTree::SelectionObjectVector selObjs = SceneManager::getInstance()->getSelectionTree().getAllObjects();
    Vector field = vin;
    for( unsigned int b = 0; b < selObjs.size(); b++ )
	{
        if( selObjs[ b ]->getSelectionType() != BOX_TYPE )
        {
            field = Vector(selObjs[ b ]->getCenter().x - pos.x, selObjs[ b ]->getCenter().y - pos.y, selObjs[ b ]->getCenter().z - pos.z);
        }
    }
    return field;
}

/////////////////////////////////////////////////////////////////////
// Classify (1 or 0) the 3 eigenVecs within Axis-Aligned vecs e1 > e2 > e3
////////////////////////////////////////////////////////////////////
void RTTFibers::setDiffusionAxis( const FMatrix &tensor, Vector& e1, Vector& e2, Vector& e3 )
{
    float lvx,lvy,lvz;

    GLfloat flippedAxes[3];
    m_pTensorsInfo->isAxisFlipped(X_AXIS) ? flippedAxes[0] = -1.0f : flippedAxes[0] = 1.0f;
    m_pTensorsInfo->isAxisFlipped(Y_AXIS) ? flippedAxes[1] = -1.0f : flippedAxes[1] = 1.0f;
    m_pTensorsInfo->isAxisFlipped(Z_AXIS) ? flippedAxes[2] = -1.0f : flippedAxes[2] = 1.0f;

    //Find the 3 axes
    lvx = flippedAxes[0] * (tensor(0,0) * tensor(0,0)
        + tensor(1,0) * tensor(1,0) 
        + tensor(2,0) * tensor(2,0));

    lvy = flippedAxes[1] * (tensor(0,1) * tensor(0,1)
        + tensor(1,1) * tensor(1,1) 
        + tensor(2,1) * tensor(2,1));

    lvz = flippedAxes[2] * (tensor(0,2) * tensor(0,2)
        + tensor(1,2) * tensor(1,2) 
        + tensor(2,2) * tensor(2,2));


    if ( lvx > lvy && lvx > lvz ) 
    {
        e1.x = 1;
        if( lvy > lvz )
        {
            e2.y = 1;
            e3.z = 1;
        }
        else
        {
            e2.z = 1;
            e3.y = 1;
        }
    }
    else if ( lvy > lvx && lvy > lvz ) 
    {
        e1.y = 1;
        if( lvx > lvz )
        {
            e2.x = 1;
            e3.z = 1;
        }
        else
        {
            e2.z = 1;
            e3.x = 1;
        }
    }
    else 
    {
        e1.z = 1;
        if( lvx > lvy )
        {
            e2.x = 1;
            e3.y = 1;
        }
        else
        {
            e2.y = 1;
            e3.x = 1;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// Performs realtime fiber tracking along direction bwdfwd (backward, forward)
///////////////////////////////////////////////////////////////////////////
void RTTFibers::performDTIRTT(Vector seed, int bwdfwd, vector<Vector>& points, vector<Vector>& color)
{   
    //Vars
    Vector currPosition(seed); //Current PIXEL position
    Vector nextPosition; //Next Pixel position
    Vector e1(0,0,0); //Direction of the tensor (axis aligned)
    Vector e2(0,0,0); //Direction of the tensor (axis aligned)
    Vector e3(0,0,0); //Direction of the tensor (axis aligned)
    Vector currDirection, nextDirection; //Directions re-aligned 

    GLfloat flippedAxes[3];
    m_pTensorsInfo->isAxisFlipped(X_AXIS) ? flippedAxes[0] = -1.0f : flippedAxes[0] = 1.0f;
    m_pTensorsInfo->isAxisFlipped(Y_AXIS) ? flippedAxes[1] = -1.0f : flippedAxes[1] = 1.0f;
    m_pTensorsInfo->isAxisFlipped(Z_AXIS) ? flippedAxes[2] = -1.0f : flippedAxes[2] = 1.0f;

    unsigned int tensorNumber; 
    int currVoxelx, currVoxely, currVoxelz;
    float FAvalue, angle; 
    float FAthreshold = getFAThreshold();
    float angleThreshold = getAngleThreshold();
    float step = getStep();

    int columns = DatasetManager::getInstance()->getColumns();
    int rows    = DatasetManager::getInstance()->getRows();

    float xVoxel = DatasetManager::getInstance()->getVoxelX();
    float yVoxel = DatasetManager::getInstance()->getVoxelY();
    float zVoxel = DatasetManager::getInstance()->getVoxelZ();

    FMatrix tensor;

    //Get the seed voxel
    currVoxelx = (int)( floor(currPosition.x / xVoxel) );
    currVoxely = (int)( floor(currPosition.y / yVoxel) );
    currVoxelz = (int)( floor(currPosition.z / zVoxel) );

    //Corresponding tensor number
    tensorNumber = currVoxelz * columns * rows + currVoxely *columns + currVoxelx;

    if( tensorNumber < m_pTensorsInfo->getTensorsMatrix()->size() )
    {
        //Use Interpolation
        if( RTTrackingHelper::getInstance()->isTensorsInterpolated() )
        {
            tensor = trilinearInterp( currPosition.x, currPosition.y, currPosition.z );
        }
        else
        {
            tensor = m_pTensorsInfo->getTensorsMatrix()->at(tensorNumber); 
        }

        //Find the MAIN axis
        setDiffusionAxis( tensor, e1, e2, e3 );

        //Align the main direction my mult AxisAlign * tensorMatrix
        currDirection.x = flippedAxes[0] * (tensor(0,0) * e1.x + 
                          tensor(0,1) * e1.y + 
                          tensor(0,2) * e1.z);

        currDirection.y = flippedAxes[1] * (tensor(1,0) * e1.x + 
                          tensor(1,1) * e1.y + 
                          tensor(1,2) * e1.z);

        currDirection.z = flippedAxes[2] * (tensor(2,0) * e1.x +
                          tensor(2,1) * e1.y + 
                          tensor(2,2) * e1.z);

        //Direction for seeding (forward or backward)
        currDirection.normalize();
        currDirection *= bwdfwd;

        //Next position
        nextPosition = currPosition + ( step * currDirection );

        //Get the voxel stepped into
        currVoxelx = (int)( floor(nextPosition.x / xVoxel) );
        currVoxely = (int)( floor(nextPosition.y / yVoxel) );
        currVoxelz = (int)( floor(nextPosition.z / zVoxel) );

        //Corresponding tensor number
        tensorNumber = currVoxelz * columns * rows + currVoxely * columns + currVoxelx;

        if( tensorNumber < m_pTensorsInfo->getTensorsMatrix()->size() )
        {
            //Use interpolation
            if( RTTrackingHelper::getInstance()->isTensorsInterpolated() )
            {
                tensor = trilinearInterp( nextPosition.x, nextPosition.y, nextPosition.z );
            }
            else
            {
                tensor = m_pTensorsInfo->getTensorsMatrix()->at(tensorNumber);
            }

            //Find the main diffusion axis
            e1.zero();
            e2.zero();
            e3.zero();
            setDiffusionAxis( tensor, e1, e2, e3 );

            //Advection next direction
            nextDirection = advecIntegrate( currDirection, tensor, e1, e2, e3, tensorNumber );

            //Direction of seeding
            nextDirection.normalize();
            nextDirection *= bwdfwd;

            if( currDirection.Dot(nextDirection) < 0 ) //Ensures the two vectors have the same directions
            {
                nextDirection *= -1;
            }

            //FA value
            FAvalue = m_pTensorsInfo->getTensorsFA()->at(tensorNumber);

            //Angle value
            angle = 180 * std::acos( currDirection.Dot(nextDirection) ) / M_PI;
            if( angle > 90 )
            {
                angle = 180 - angle; //Ensures that we have the smallest angle
            }

            ///////////////////////////
            //Tracking along the fiber
            //////////////////////////
            while( FAvalue >= FAthreshold && angle <= angleThreshold )
            {
                //Insert point to be rendered
                points.push_back( currPosition );
                color.push_back( currDirection );

                //Advance
                currPosition = nextPosition;
                currDirection = nextDirection;

                //Next position
                nextPosition = currPosition + ( step * currDirection );

                //Stepped voxels
                currVoxelx = (int)( floor(nextPosition.x / xVoxel) );
                currVoxely = (int)( floor(nextPosition.y / yVoxel) );
                currVoxelz = (int)( floor(nextPosition.z / zVoxel) );

                //Corresponding tensor number
                tensorNumber = currVoxelz * columns * rows + currVoxely * columns + currVoxelx;

                if( tensorNumber > m_pTensorsInfo->getTensorsMatrix()->size() ) //Out of anatomy
                {
                    break;
                }

                //Use interpolation
                if( RTTrackingHelper::getInstance()->isTensorsInterpolated() )
                {
                    tensor = trilinearInterp( nextPosition.x, nextPosition.y, nextPosition.z );
                }
                else
                {
                    tensor = m_pTensorsInfo->getTensorsMatrix()->at(tensorNumber);
                }

                //Find the MAIN axis
                e1.zero();
                e2.zero();
                e3.zero();
                setDiffusionAxis( tensor, e1, e2, e3 );

                //Advection next direction
                nextDirection = advecIntegrate( currDirection, tensor, e1, e2, e3, tensorNumber );

                //Direction of seeding (backward of forward)
                nextDirection.normalize();
                nextDirection *= bwdfwd;
                if( currDirection.Dot(nextDirection) < 0 ) //Ensures both vectors points in the same direction
                {
                    nextDirection *= -1;
                }

                //FA value
                FAvalue = m_pTensorsInfo->getTensorsFA()->at(tensorNumber);

                //Angle value
                angle = 180 * std::acos( currDirection.Dot(nextDirection) ) / M_PI;
                if( angle > 90 )
                {
                    angle = 180 - angle; //Ensures we have the minimal angle
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// Draft a direction to start the tracking process using a probabilistic random
// [0 --- |v1| --- |v2| --- |v3|]
///////////////////////////////////////////////////////////////////////////
std::vector<float> RTTFibers::pickDirection(std::vector<float> initialPeaks, bool initWithDir)
{
    std::vector<float> draftedPeak;
    if(!initWithDir)
    {
	    
	    float norms[3];
	    float sum = 0.0f;

	    for(unsigned int i=0; i < initialPeaks.size()/3; i++)
        {
            Vector v1(initialPeaks[i*3],initialPeaks[i*3+1], initialPeaks[i*3+2]);
		    norms[i] = v1.getLength();
		    sum += norms[i];
	    }
    
	    float random = ( (float) rand() ) / (float) RAND_MAX;
        float weight = ( random * sum );

	    if(weight < norms[0])
	    {
		    draftedPeak.push_back(initialPeaks[0]);
		    draftedPeak.push_back(initialPeaks[1]);
		    draftedPeak.push_back(initialPeaks[2]);
	    }
	    else if(weight < norms[0] + norms[1])
	    {
		    draftedPeak.push_back(initialPeaks[3]);
		    draftedPeak.push_back(initialPeaks[4]);
		    draftedPeak.push_back(initialPeaks[5]);
	    }
	    else
	    {
		    draftedPeak.push_back(initialPeaks[6]);
		    draftedPeak.push_back(initialPeaks[7]);
		    draftedPeak.push_back(initialPeaks[8]);
	    }
    }
    else
    {
        Vector vOut(0,0,0);
        float angleMin = 360.0f;
        float angle = 0.0f;
        float puncture = m_vinvout;
	     

        for(unsigned int i=0; i < initialPeaks.size()/3; i++)
        {
            Vector v1(initialPeaks[i*3],initialPeaks[i*3+1], initialPeaks[i*3+2]);
            v1.normalize();
        
            if( m_initVec.Dot(v1) < 0 ) //Ensures both vectors points in the same direction
            {
                v1 *= -1;
            }

            //Angle value
            float dot = m_initVec.Dot(v1);
            float acos = std::acos( dot );
            angle = 180 * acos / M_PI;
        
            //Direction most probable
            if( angle < angleMin )
            {
                angleMin = angle;
                vOut = v1;
            }     
        }
        draftedPeak.push_back(vOut.x);
        draftedPeak.push_back(vOut.y);
        draftedPeak.push_back(vOut.z);
    }
		
	return draftedPeak;
}

bool RTTFibers::checkExclude( unsigned int sticksNumber)
{
	bool res = true;
	if(m_pExcludeInfo != NULL)
	{
		if(m_pExcludeInfo->at(sticksNumber) != 0)
		{
			res = false;
			m_stop = true;
		}
	}

	return res;	
}

///////////////////////////////////////////////////////////////////////////
// Returns true if no anatomy is loaded for thresholding or if above the threshold
///////////////////////////////////////////////////////////////////////////
bool RTTFibers::withinMapThreshold(unsigned int sticksNumber)
{
    bool isOk = false;
    float gmVal = 0;
	if(m_pGMInfo != NULL)
    {
        gmVal = m_pGMInfo->at(sticksNumber);
	    if(gmVal > 0)
        {
            m_countGMstep++;
        }
        else
        {
            m_countGMstep = 0;
        }   
    }

	if((m_pMaskInfo->at(sticksNumber) > m_FAThreshold || gmVal > m_FAThreshold) && checkExclude(sticksNumber) && m_countGMstep < m_GMstep)
    {
        isOk = true;
    }

    return isOk;
}

///////////////////////////////////////////////////////////////////////////
// Performs realtime HARDI fiber tracking along direction bwdfwd (backward, forward)
///////////////////////////////////////////////////////////////////////////
void RTTFibers::performHARDIRTT(Vector seed, int bwdfwd, vector<Vector>& points, vector<Vector>& color)
{ 
    //Vars
    Vector currPosition(seed); //Current PIXEL position
    Vector nextPosition; //Next Pixel position
    Vector currDirection, nextDirection; //Directions re-aligned 
    

    GLfloat flippedAxes[3];
    m_pMaximasInfo->isAxisFlipped(X_AXIS) ? flippedAxes[0] = -1.0f : flippedAxes[0] = 1.0f;
    m_pMaximasInfo->isAxisFlipped(Y_AXIS) ? flippedAxes[1] = -1.0f : flippedAxes[1] = 1.0f;
    m_pMaximasInfo->isAxisFlipped(Z_AXIS) ? flippedAxes[2] = -1.0f : flippedAxes[2] = 1.0f;

    unsigned int sticksNumber; 
    int currVoxelx, currVoxely, currVoxelz;
    float angle; 

    int columns = DatasetManager::getInstance()->getColumns();
    int rows    = DatasetManager::getInstance()->getRows();

    float xVoxel = DatasetManager::getInstance()->getVoxelX();
    float yVoxel = DatasetManager::getInstance()->getVoxelY();
    float zVoxel = DatasetManager::getInstance()->getVoxelZ();

    //Get the seed voxel
    currVoxelx = (int)( floor(currPosition.x / xVoxel) );
    currVoxely = (int)( floor(currPosition.y / yVoxel) );
    currVoxelz = (int)( floor(currPosition.z / zVoxel) );

    //Corresponding stick number
    sticksNumber = currVoxelz * columns * rows + currVoxely *columns + currVoxelx;
    std::vector<float> sticks;

    m_countGMstep = 0;

    if( sticksNumber < m_pMaximasInfo->getMainDirData()->size() &&  m_pMaximasInfo->getMainDirData()->at(sticksNumber)[0] != 0 && withinMapThreshold(sticksNumber) && !m_stop)
    {
        bool initWithDir = RTTrackingHelper::getInstance()->isInitSeed();
        sticks = pickDirection(m_pMaximasInfo->getMainDirData()->at(sticksNumber), initWithDir); 

        currDirection.x = flippedAxes[0] * sticks[0];
        currDirection.y = flippedAxes[1] * sticks[1];
        currDirection.z = flippedAxes[2] * sticks[2];

        //Direction for seeding (forward or backward)
        currDirection.normalize();
        currDirection *= bwdfwd;

        //Next position
        nextPosition = currPosition + ( m_step * currDirection );

        //Get the voxel stepped into
        currVoxelx = (int)( floor(nextPosition.x / xVoxel) );
        currVoxely = (int)( floor(nextPosition.y / yVoxel) );
        currVoxelz = (int)( floor(nextPosition.z / zVoxel) );

        //Corresponding stick number
        sticksNumber = currVoxelz * columns * rows + currVoxely * columns + currVoxelx;

        if( sticksNumber < m_pMaximasInfo->getMainDirData()->size() && m_pMaximasInfo->getMainDirData()->at(sticksNumber)[0] != 0 )
        {

            sticks = m_pMaximasInfo->getMainDirData()->at(sticksNumber); 

            //Advection next direction
            nextDirection = advecIntegrateHARDI( currDirection, sticks, sticksNumber, nextPosition );

            //Direction of seeding
            nextDirection *= bwdfwd;
            nextDirection.normalize();

            if( currDirection.Dot(nextDirection) < 0 ) //Ensures the two vectors have the same directions
            {
                nextDirection *= -1;
            }

            //Angle value
            float dot = currDirection.Dot(nextDirection);
            float acos = std::acos( dot );
            angle = 180 * acos / M_PI;

            ///////////////////////////
            //Tracking along the fiber
            //////////////////////////
            while( angle <= m_angleThreshold && withinMapThreshold(sticksNumber) && !m_stop)
            {
                //Insert point to be rendered
                points.push_back( currPosition );
                color.push_back( currDirection );

                //Advance
                currPosition = nextPosition;
                currDirection = nextDirection;

                //Next position
                nextPosition = currPosition + ( m_step * currDirection );

                //Stepped voxels
                currVoxelx = (int)( floor(nextPosition.x / xVoxel) );
                currVoxely = (int)( floor(nextPosition.y / yVoxel) );
                currVoxelz = (int)( floor(nextPosition.z / zVoxel) );

                //Corresponding tensor number
                sticksNumber = currVoxelz * columns * rows + currVoxely * columns + currVoxelx;

                if( sticksNumber > m_pMaximasInfo->getMainDirData()->size() || m_pMaximasInfo->getMainDirData()->at(sticksNumber)[0] == 0) //Out of anatomy
                {
                    break;
                }
                
                sticks = m_pMaximasInfo->getMainDirData()->at(sticksNumber);

                //Advection next direction
                nextDirection = advecIntegrateHARDI( currDirection, sticks, sticksNumber, nextPosition );

                //Direction of seeding (backward of forward)
                nextDirection *= bwdfwd;
                nextDirection.normalize();

                if( currDirection.Dot(nextDirection) < 0 ) //Ensures both vectors points in the same direction
                {
                    nextDirection *= -1;
                }

                //Angle value
                float dot = currDirection.Dot(nextDirection);
                float acos = std::acos( dot );
                angle = 180 * acos / M_PI;
            }
        }
    }
}

//////////////////////////////////////////
/*Destructor*/
//////////////////////////////////////////
RTTFibers::~RTTFibers()
{
}
