/*
 *  The RTTFibers class implementation.
 *
 */

#include "RTTFibers.h"
#include "../main.h"
#include "DatasetManager.h"
#include "RTTrackingHelper.h"
#include "../Logger.h"
#include "../gfx/ShaderHelper.h"
#include "../gfx/TheScene.h"
#include "../gui/SceneManager.h"
#include "../misc/lic/FgeOffscreen.h"
#include "../gui/MainFrame.h"
#include "../misc/IsoSurface/CIsoSurface.h"
#include "../misc/IsoSurface/TriangleMesh.h"


#include <algorithm>
using std::sort;

#include <vector>
using std::vector;

//////////////////////////////////////////
//Constructor
//////////////////////////////////////////
RTTFibers::RTTFibers()
:   m_FAThreshold( 0.10f ),
    m_angleThreshold( 35.0f ),
    m_step( 1.0f ),
    m_nbSeed ( 10.0f ),
    m_nbMeshPt ( 0 ),
    m_puncture( 0.2f ),
    m_vinvout( 0.2f ),
    m_minFiberLength( 10 ),
    m_maxFiberLength( 200 ),
    m_isHARDI( false ),
    m_usingMap( false )
{
    //GPGPU
    writeTex = 0;
    readTex = 1;
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
    vector< vector< SelectionObject* > > selectionObjects = SceneManager::getInstance()->getSelectionObjects();
	 
	//Evenly distanced seeds
    if( !RTTrackingHelper::getInstance()->isShellSeeds() )
	{
		for( unsigned int b = 0; b < selectionObjects.size(); b++ )
		{
			minCorner.x = selectionObjects[b][0]->getCenter().x - selectionObjects[b][0]->getSize().x * xVoxel / 2.0f;
			minCorner.y = selectionObjects[b][0]->getCenter().y - selectionObjects[b][0]->getSize().y * yVoxel / 2.0f;
			minCorner.z = selectionObjects[b][0]->getCenter().z - selectionObjects[b][0]->getSize().z * zVoxel / 2.0f;
			maxCorner.x = selectionObjects[b][0]->getCenter().x + selectionObjects[b][0]->getSize().x * xVoxel / 2.0f;
			maxCorner.y = selectionObjects[b][0]->getCenter().y + selectionObjects[b][0]->getSize().y * yVoxel / 2.0f;
			maxCorner.z = selectionObjects[b][0]->getCenter().z + selectionObjects[b][0]->getSize().z * zVoxel / 2.0f;

			float xstep =  selectionObjects[b][0]->getSize().x * xVoxel / float( m_nbSeed - 1.0f );
			float ystep =  selectionObjects[b][0]->getSize().y * yVoxel / float( m_nbSeed - 1.0f );
			float zstep =  selectionObjects[b][0]->getSize().z * zVoxel / float( m_nbSeed - 1.0f );

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

						//glColor3f(1,0,0);
						//SceneManager::getInstance()->getScene()->drawSphere( x, y ,z, 0.2 );

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
                vector<Vector> points; // Points to be rendered
                vector<Vector> color; //Color (local directions)
                
                if(m_isHARDI)
                {
                    //Track both sides
                    performHARDIRTT( Vector(positions[k].x,positions[k].y,positions[k].z),  1, points, color); //First pass
                    m_fibersRTT.push_back( points );
                    m_colorsRTT.push_back( color );
                    points.clear();
                    color.clear();
            
                    performHARDIRTT( Vector(positions[k].x,positions[k].y,positions[k].z), -1, points, color); //Second pass
                    m_fibersRTT.push_back( points ); 
                    m_colorsRTT.push_back( color );
                }
                else
                {
                    //Track both sides
                    performDTIRTT( Vector(positions[k].x,positions[k].y,positions[k].z),  1, points, color); //First pass
                    m_fibersRTT.push_back( points );
                    m_colorsRTT.push_back( color );
                    points.clear();
                    color.clear();
            
                    performDTIRTT( Vector(positions[k].x,positions[k].y,positions[k].z), -1, points, color); //Second pass
                    m_fibersRTT.push_back( points ); 
                    m_colorsRTT.push_back( color );
               }
            }
        }
	}
    renderRTTFibers();
	RTTrackingHelper::getInstance()->setRTTDirty( false );
}

    

///////////////////////////////////////////////////////////////////////////
//Rendering stage
///////////////////////////////////////////////////////////////////////////
void RTTFibers::renderRTTFibers()
{
    if( m_fibersRTT.size() > 0 )
    {
	    for( unsigned int j = 0; j < m_fibersRTT.size() - 1; j+=2 )
	    { 
		    //POINTS
		    if( SceneManager::getInstance()->isPointMode() )
		    {
			    //Forward
			    if( m_fibersRTT[j].size() > 0 )
			    {
				    for( unsigned int i = 0; i < m_fibersRTT[j].size(); i++ )
				    {  
					    glColor3f( std::abs(m_colorsRTT[j][i].x), std::abs(m_colorsRTT[j][i].y), std::abs(m_colorsRTT[j][i].z) );
					    glBegin( GL_POINTS );
						    glVertex3f( m_fibersRTT[j][i].x, m_fibersRTT[j][i].y, m_fibersRTT[j][i].z );
					    glEnd();
				    }
			    }
			    //Backward
			    if(m_fibersRTT[j+1].size() > 0)
			    {
				    for( unsigned int i = 0; i < m_fibersRTT[j+1].size(); i++ )
				    {  
					    glColor3f( std::abs(m_colorsRTT[j+1][i].x), std::abs(m_colorsRTT[j+1][i].y), std::abs(m_colorsRTT[j+1][i].z) );
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
				    for( unsigned int i = 0; i < m_fibersRTT[j].size() - 1; i++ )
				    {
					    glColor3f( std::abs(m_colorsRTT[j][i].x), std::abs(m_colorsRTT[j][i].y), std::abs(m_colorsRTT[j][i].z) );
					    glBegin( GL_LINES );
						    glVertex3f( m_fibersRTT[j][i].x, m_fibersRTT[j][i].y, m_fibersRTT[j][i].z );
						    glVertex3f( m_fibersRTT[j][i+1].x, m_fibersRTT[j][i+1].y, m_fibersRTT[j][i+1].z );
					    glEnd();
				    }
			    }
			    //Backward
			    if ( m_fibersRTT[j+1].size() > 2)
			    {
				    for( unsigned int i = 0; i < m_fibersRTT[j+1].size() - 1; i++ )
				    {
					    glColor3f( std::abs(m_colorsRTT[j+1][i].x), std::abs(m_colorsRTT[j+1][i].y), std::abs(m_colorsRTT[j+1][i].z) );
					    glBegin( GL_LINES );
						    glVertex3f( m_fibersRTT[j+1][i].x, m_fibersRTT[j+1][i].y, m_fibersRTT[j+1][i].z );
						    glVertex3f( m_fibersRTT[j+1][i+1].x, m_fibersRTT[j+1][i+1].y, m_fibersRTT[j+1][i+1].z );
					    glEnd();
				    }
			    }
		    }   
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

Vector RTTFibers::advecIntegrateHARDI( Vector vin, const std::vector<float> &sticks, float s_number ) 
{
    Vector vOut(0,0,0);
    float angleMin = 360.0f;
    float angle = 0.0f;
    float puncture = m_vinvout;
    float fa = m_pMapInfo->at(s_number);
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

    Vector res = fa * vOut + (1.0 - fa) * ( (1.0 - puncture ) * vin + puncture * vOut); 
    res.normalize();

    return res;
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
std::vector<float> RTTFibers::pickDirection(std::vector<float> initialPeaks)
{
	std::vector<float> draftedPeak;
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
		
	return draftedPeak;
}
///////////////////////////////////////////////////////////////////////////
// Returns true if no anatomy is loaded for thresholding or if above the threshold
///////////////////////////////////////////////////////////////////////////
bool RTTFibers::withinMapThreshold(unsigned int sticksNumber)
{
    bool isOk = true;

    if(m_usingMap)
    {
        isOk = false;
        if(m_pMapInfo->at(sticksNumber) > m_FAThreshold)
        {
            isOk = true;
        }
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

    if( sticksNumber < m_pMaximasInfo->getMainDirData()->size() &&  !m_pMaximasInfo->getMainDirData()->at(sticksNumber).empty() && withinMapThreshold(sticksNumber))
    {
        sticks = pickDirection(m_pMaximasInfo->getMainDirData()->at(sticksNumber)); 

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

        if( sticksNumber < m_pMaximasInfo->getMainDirData()->size() && !m_pMaximasInfo->getMainDirData()->at(sticksNumber).empty())
        {

            sticks = m_pMaximasInfo->getMainDirData()->at(sticksNumber); 

            //Advection next direction
            nextDirection = advecIntegrateHARDI( currDirection, sticks, sticksNumber );

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
            while( angle <= m_angleThreshold && withinMapThreshold(sticksNumber))
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

                if( sticksNumber > m_pMaximasInfo->getMainDirData()->size() || m_pMaximasInfo->getMainDirData()->at(sticksNumber).empty()) //Out of anatomy
                {
                    break;
                }
                
                sticks = m_pMaximasInfo->getMainDirData()->at(sticksNumber);

                //Advection next direction
                nextDirection = advecIntegrateHARDI( currDirection, sticks, sticksNumber );

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

///////////////////////////////////////////////////////////////////////////
//////////////  Animation Section   ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void RTTFibers::trackAction(bool isPlaying)
{
    if(isPlaying)
    {
        if(RTTrackingHelper::getInstance()->isTrackActionPlaying())
        {
            std::cout << "Playing \n";

            for(int j=0; j < RTTrackingHelper::getInstance()->getTrackActionStep())
            {
            //Forward
			    if( m_fibersRTT[j].size() > 2)
			    {
				    for( unsigned int i = 0; i < m_fibersRTT[j].size() - 1; i++ )
				    {
					    glColor3f( std::abs(m_colorsRTT[j][i].x), std::abs(m_colorsRTT[j][i].y), std::abs(m_colorsRTT[j][i].z) );
					    glBegin( GL_LINES );
						    glVertex3f( m_fibersRTT[j][i].x, m_fibersRTT[j][i].y, m_fibersRTT[j][i].z );
						    glVertex3f( m_fibersRTT[j][i+1].x, m_fibersRTT[j][i+1].y, m_fibersRTT[j][i+1].z );
					    glEnd();
				    }
			    }
			    //Backward
			    if ( m_fibersRTT[j+1].size() > 2)
			    {
				    for( unsigned int i = 0; i < m_fibersRTT[j+1].size() - 1; i++ )
				    {
					    glColor3f( std::abs(m_colorsRTT[j+1][i].x), std::abs(m_colorsRTT[j+1][i].y), std::abs(m_colorsRTT[j+1][i].z) );
					    glBegin( GL_LINES );
						    glVertex3f( m_fibersRTT[j+1][i].x, m_fibersRTT[j+1][i].y, m_fibersRTT[j+1][i].z );
						    glVertex3f( m_fibersRTT[j+1][i+1].x, m_fibersRTT[j+1][i+1].y, m_fibersRTT[j+1][i+1].z );
					    glEnd();
				    }
			    }
            }
        }
        else
        {
            std::cout << "Paused \n";
        }
    }
    else
    {
        std::cout << "Not Playing \n";
    }
}



///////////////////////////////////////////////////////////////////////////
//////////////  GPU-GPU SECTION - NOT IMPLEMENTED   ///////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// GPGPU: Init FBO
///////////////////////////////////////////////////////////////////////////
void RTTFibers::initFBO()
{
    // create FBO (off-screen framebuffer)
    glGenFramebuffersEXT(1, &fb); 
    // bind offscreen framebuffer (that is, skip the window-specific render target)
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

    //BUG IS HERE

    //glGetIntegerv( GL_VIEWPORT,m_pDatasetHelper->m_mainFrame->m_pMainGL->m_viewport );
    //viewport for 1:1 pixel=texture mapping
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, texSize, 0, texSize);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, texSize, texSize);
}

///////////////////////////////////////////////////////////////////////////
// GPGPU: Create Textures
///////////////////////////////////////////////////////////////////////////
void RTTFibers::createTextures(void)
{
    // create textures 
    // y gets two textures, alternatingly read-only and write-only, 
    // x is just read-only
    glGenTextures (2, yTexID);
    glGenTextures (1, &xTexID);
    // set up textures
    setupTexture (yTexID[readTex]);
    transferToTexture(seeds,yTexID[readTex]);
    setupTexture (yTexID[writeTex]);
    transferToTexture(seeds,yTexID[writeTex]);

    setupTexture (xTexID);
    transferToTexture(xValues,xTexID);
    // set texenv mode from modulate (the default) to replace
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // check if something went completely wrong
    checkGLErrors ("createFBOandTextures()");
}

///////////////////////////////////////////////////////////////////////////
// GPGPU: Computation
///////////////////////////////////////////////////////////////////////////
void RTTFibers::performComputation(void) 
{
    // attach two textures to FBO
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachmentpoints[writeTex], GL_TEXTURE_RECTANGLE_ARB, yTexID[writeTex], 0);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachmentpoints[readTex], GL_TEXTURE_RECTANGLE_ARB, yTexID[readTex], 0);
    // check if that worked
    if (!checkFramebufferStatus())
    {
        printf("glFramebufferTexture2DEXT():\t [FAIL]\n");
        exit (-5);
    }

    // enable GLSL program
    ShaderHelper::getInstance()->getRTTShader()->bind();

    glActiveTexture(GL_TEXTURE1);    
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB,xTexID);
    ShaderHelper::getInstance()->getRTTShader()->setUniSampler("xValues", 1);
    glFinish();

    for (int i=0; i<1; i++) 
    {
        // set render destination
        glDrawBuffer (attachmentpoints[writeTex]);
        // enable texture y_old (read-only)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB,yTexID[readTex]);


        ShaderHelper::getInstance()->getRTTShader()->setUniSampler("inSeed", 0);
        ShaderHelper::getInstance()->getRTTShader()->setUniInt("color", 1);
        // and render multitextured viewport-sized quad
        // depending on the texture target, switch between 
        // normalised ([0,1]^2) and unnormalised ([0,w]x[0,h])
        // texture coordinates

        // make quad filled to hit every pixel/texel 
        // (should be default but we never know)
        glPolygonMode(GL_FRONT,GL_FILL);

        // render with unnormalized texcoords
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); 
        glVertex2f(-1, -1);
        glTexCoord2f(texSize, 0.0); 
        glVertex2f(1, -1);
        glTexCoord2f(texSize,texSize ); 
        glVertex2f(1, 1);
        glTexCoord2f(0.0, texSize); 
        glVertex2f(-1, 1);
        glEnd();

        // swap role of the two textures (read-only source becomes 
        // write-only target and the other way round):
        //compareResults();
        swap();
    }
    // done, stop timer, calc MFLOP/s if neccessary

    // done, just do some checks if everything went smoothly.
    checkFramebufferStatus();
    checkGLErrors("render()");
}

///////////////////////////////////////////////////////////////////////////
// GPGPU: Compare
///////////////////////////////////////////////////////////////////////////
void RTTFibers::compareResults()
{
    transferFromTexture (result);
    //std::cout << "AFTER: " << result[0] << " " << result[1] << " " << result[2] << " " << result[3] << "\n";
    for(int k=0; k<4*N; k+=4)
    {
        std::cout << "AFTER: " << result[k] << " " << result[k+1] << " " << result[k+2] << " " << result[k+3] << "\n";
    }
}

///////////////////////////////////////////////////////////////////////////
// GPGPU: Textures Functions
///////////////////////////////////////////////////////////////////////////
void RTTFibers::transferFromTexture(float* data) 
{
    // version (a): texture is attached
    // recommended on both NVIDIA and ATI
    glReadBuffer(attachmentpoints[readTex]);
    glReadPixels(0, 0, texSize, texSize,GL_RGBA,GL_FLOAT,data);
    // version b: texture is not necessarily attached
    //glBindTexture(GL_TEXTURE_RECTANGLE_ARB,yTexID[readTex]);
    //glGetTexImage(GL_TEXTURE_RECTANGLE_ARB,0,GL_RGBA,GL_FLOAT,data);
}

void RTTFibers::transferToTexture (float* data, GLuint texID) {
    // version (a): HW-accelerated on NVIDIA 
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texID);
    glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB,0,0,0,texSize,texSize,GL_RGBA,GL_FLOAT,data);
    // version (b): HW-accelerated on ATI 
    //glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, texID, 0);
    //glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    //glRasterPos2i(0,0);
    //glDrawPixels(texSize,texSize*texSize,GL_RGBA,GL_FLOAT,data);
    //glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, 0, 0);
}

void RTTFibers::setupTexture (const GLuint texID) 
{
    // make active and bind
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB,texID);
    // turn off filtering and wrap modes
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
    // define texture with floating point format
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_RGBA32F_ARB,texSize,texSize,0,GL_RGBA,GL_FLOAT,0);
    // check if that worked
    if (glGetError() != GL_NO_ERROR)
    {
        printf("glTexImage2D():\t\t\t [FAIL]\n");
        exit (-3);
    }
}

///////////////////////////////////////////////////////////////////////////
// GPGPU
///////////////////////////////////////////////////////////////////////////
void RTTFibers::setupALL()
{
    attachmentpoints[0] = GL_COLOR_ATTACHMENT0_EXT;
    attachmentpoints[1] = GL_COLOR_ATTACHMENT1_EXT;

    // init offscreen framebuffer
    initFBO();

    //FgeOffscreen fbo( texSize, texSize, false );

    //fbo.setClearColor( 1.0f, 1.0f, 1.0f);


/*  glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D(0.0, texSize, 0.0, texSize);


    glViewport( 0, 0, texSize, texSize );*/


    // create textures for vectors
    createTextures();

    // init shader runtime
    // and start computation
    performComputation();

    // compare results
    compareResults();

    // and clean up
    ShaderHelper::getInstance()->getRTTShader()->release();

    //fbo.deactivate();
    glDeleteFramebuffersEXT(1,&fb);



    ////ROUNDTRIP BACK
    //for(int j = 0; j < 4*texSize*texSize*texSize; j+=4)
    //{
    //    glColor3f(1.0,1.0,0.0);
    //     m_pDatasetHelper->m_theScene->drawSphere( result[j], result[j+1], result[j+2], 0.2);
    //    // glBegin( GL_LINES );
    //    //    glVertex3f( seeds[j], seeds[j+1],seeds[j+2]);
    //    //    glVertex3f( result[j], result[j+1], result[j+2]);
    //    //glEnd();
    //}


    free(seeds);
    free(result);
    free(xValues);
    glDeleteTextures(2,yTexID);
    glDeleteTextures (1,&xTexID);


    //Restore Matrix parameters
    //glMatrixMode( GL_PROJECTION );
    //glLoadIdentity();
    //glOrtho( -m_pDatasetHelper->m_mainFrame->m_pMainGL->m_orthoModX, m_pDatasetHelper->m_mainFrame->m_pMainGL->m_orthoSizeNormal + m_pDatasetHelper->m_mainFrame->m_pMainGL->m_orthoModX,
    //    -m_pDatasetHelper->m_mainFrame->m_pMainGL->m_orthoModY, m_pDatasetHelper->m_mainFrame->m_pMainGL->m_orthoSizeNormal + m_pDatasetHelper->m_mainFrame->m_pMainGL->m_orthoModY, -500, 500 );

    //glViewport(m_pDatasetHelper->m_mainFrame->m_pMainGL->m_viewport[0], m_pDatasetHelper->m_mainFrame->m_pMainGL->m_viewport[1], m_pDatasetHelper->m_mainFrame->m_pMainGL->m_viewport[2], m_pDatasetHelper->m_mainFrame->m_pMainGL->m_viewport[3]);
}

/**
 * Checks framebuffer status.
 * Copied directly out of the spec, modified to deliver a return value.
 */
bool RTTFibers::checkFramebufferStatus() {
    GLenum status;
    status = (GLenum) glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            return true;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        printf("Framebuffer incomplete, incomplete attachment\n");
            return false;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        printf("Unsupported framebuffer format\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        printf("Framebuffer incomplete, missing attachment\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        printf("Framebuffer incomplete, attached images must have same dimensions\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        printf("Framebuffer incomplete, attached images must have same format\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        printf("Framebuffer incomplete, missing draw buffer\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        printf("Framebuffer incomplete, missing read buffer\n");
            return false;
    }
    return false;
}


void RTTFibers::checkGLErrors( const char *label )
{
    Logger::getInstance()->printIfGLError( wxString( label, wxConvUTF8 ) );
//     GLenum errCode;
//     const GLubyte *errStr;
//     
//     if ((errCode = glGetError()) != GL_NO_ERROR) 
//     {
//  errStr = gluErrorString(errCode);
//     printf("OpenGL ERROR: ");
//     printf((char*)errStr);
//     printf("(Label: ");
//     printf(label);
//     printf(")\n.");
//     }
}

///////////////////////////////////////////////////////////////////////////
// GPGPU swap framebuffers
///////////////////////////////////////////////////////////////////////////
void RTTFibers::swap(void) 
{
    if (writeTex == 0)
    {
        writeTex = 1;
        readTex = 0;
    } 
    else 
    {
        writeTex = 0;
        readTex = 1;
    }
}

//////////////////////////////////////////
/*Destructor*/
//////////////////////////////////////////
RTTFibers::~RTTFibers()
{
}
