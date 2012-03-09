/*
 *  The RTTFibers class implementation.
 *
 */

#include "RTTFibers.h"

#include "DatasetManager.h"
#include "RTTrackingHelper.h"
#include "../Logger.h"
#include "../gfx/ShaderHelper.h"
#include "../gui/SceneManager.h"
#include "../misc/lic/FgeOffscreen.h"

#include <algorithm>
using std::sort;
#include <vector>
using std::vector;

//////////////////////////////////////////
//Constructor
//////////////////////////////////////////
RTTFibers::RTTFibers()
:   m_FAThreshold( 0.10f ),
    m_angleThreshold( 60.0f ),
    m_step( 1.0f ),
    m_puncture( 0.2f ),
    m_minFiberLength( 10 ),
    m_maxFiberLength( 200 )
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

    //N = 16; //Number of seeds
    texSize = 10;//(int)sqrt((double)N);

    //seeds = (float*)malloc(4*N*sizeof(float));
    //result = (float*)malloc(4*N*sizeof(float));
    //xValues = (float*)malloc(4*N*sizeof(float));

    //int i =0;

    for( unsigned int b = 0; b < selectionObjects.size(); b++ )
    {

        minCorner.x = selectionObjects[b][0]->getCenter().x - selectionObjects[b][0]->getSize().x * xVoxel / 2.0f;
        minCorner.y = selectionObjects[b][0]->getCenter().y - selectionObjects[b][0]->getSize().y * yVoxel / 2.0f;
        minCorner.z = selectionObjects[b][0]->getCenter().z - selectionObjects[b][0]->getSize().z * zVoxel / 2.0f;
        maxCorner.x = selectionObjects[b][0]->getCenter().x + selectionObjects[b][0]->getSize().x * xVoxel / 2.0f;
        maxCorner.y = selectionObjects[b][0]->getCenter().y + selectionObjects[b][0]->getSize().y * yVoxel / 2.0f;
        maxCorner.z = selectionObjects[b][0]->getCenter().z + selectionObjects[b][0]->getSize().z * zVoxel / 2.0f;
        
        //Evenly distanced seeds
        if( !RTTrackingHelper::getInstance()->isRandomSeeds() )
        {
            float xstep =  selectionObjects[b][0]->getSize().x * xVoxel / float( texSize - 1.0f );
            float ystep =  selectionObjects[b][0]->getSize().y * yVoxel / float( texSize - 1.0f );
            float zstep =  selectionObjects[b][0]->getSize().z * zVoxel / float( texSize - 1.0f );

            for( float x = minCorner.x; x < maxCorner.x + xstep/2.0f; x+= xstep )
            {
                for( float y = minCorner.y; y < maxCorner.y + ystep/2.0f; y+= ystep )
                {
                    for( float z = minCorner.z; z < maxCorner.z + zstep/2.0f; z+= zstep )
                    {
                        vector<Vector> points; // Points to be rendered
                        vector<Vector> color; //Color (local directions)
                        
                        //Track both sides
                        performRTT( Vector(x,y,z),  1, points, color); //First pass
                        m_fibersRTT.push_back( points );
                        m_colorsRTT.push_back( color );
                        points.clear();
                        color.clear();
        
                        performRTT( Vector(x,y,z), -1, points, color); //Second pass
                        m_fibersRTT.push_back( points ); 
                        m_colorsRTT.push_back( color );

                       //glColor3f(1,0,0);
                        //m_pDatasetHelper->m_theScene->drawSphere( x, y, z, 0.2);

                       //if(i < 4*texSize*texSize*texSize)
                       // {
                       //     seeds[i] = x;
                       //     seeds[i+1] = y;
                       //     seeds[i+2] = z;
                       //     seeds[i+3] = 0.0f;

                       //     xValues[i] = 2.0f;
                       //     xValues[i+1] = 2.0f;
                       //     xValues[i+2] = 2.0f;
                       //     xValues[i+3] = 0.0f;

                       //     i+=4;
                       // }
                    }
                }
            }

            //for(int k=0; k<4*N; k+=4)
            //{
            //    seeds[k] = k+1;
            //    seeds[k+1] = k+1;
            //    seeds[k+2] = k+1;
            //    seeds[k+3] = 0;

            //    xValues[k] = 100.0f;
            //    xValues[k+1] = 100.0f;
            //    xValues[k+2] = 100.0f;
            //    xValues[k+3] = 0.0f;

            //    std::cout << "BEFORESEED: " << seeds[k] << " " << seeds[k+1] << " " << seeds[k+2] << " " << seeds[k+3] << "\n";

            //}
            // std::cout << "BEFORE: " << seeds[0] << " " << seeds[1] << " " << seeds[2] << " " << seeds[3] << "\n";
            //setupALL();

            renderRTTFibers();
            RTTrackingHelper::getInstance()->setRTTDirty( false );
        }
        //Random seeds (spread within 8 quads inside voxel)
        else 
        {
            middle.x = selectionObjects[0][0]->getCenter().x;
            middle.y = selectionObjects[0][0]->getCenter().y;
            middle.z = selectionObjects[0][0]->getCenter().z;

            //L: Lower corner (min y)
            //U: Upper corner (max y)
            //F: Front corner (min z)
            //B: Back  corner (max z) 
            //R: Right corner (max x)
            //L: Left  corner (min x)

            Vector minLFR( selectionObjects[0][0]->getCenter().x, minCorner.y, minCorner.z );
            Vector maxLFR( maxCorner.x, selectionObjects[0][0]->getCenter().y, selectionObjects[0][0]->getCenter().z );
            Vector minUFL( minCorner.x, selectionObjects[0][0]->getCenter().y, minCorner.z );
            Vector maxUFL( selectionObjects[0][0]->getCenter().x, maxCorner.y, selectionObjects[0][0]->getCenter().z );
            Vector minUFR( selectionObjects[0][0]->getCenter().x, selectionObjects[0][0]->getCenter().y, minCorner.z );
            Vector maxUFR( maxCorner.x, maxCorner.y, selectionObjects[0][0]->getCenter().z );
            Vector minLBL( minCorner.x, minCorner.y, selectionObjects[0][0]->getCenter().z );
            Vector maxLBL( selectionObjects[0][0]->getCenter().x, selectionObjects[0][0]->getCenter().y, maxCorner.z );
            Vector minLBR( selectionObjects[0][0]->getCenter().x, minCorner.y, selectionObjects[0][0]->getCenter().z );
            Vector maxLBR( maxCorner.x, selectionObjects[0][0]->getCenter().y, maxCorner.z );
            Vector minUBL( minCorner.x, selectionObjects[0][0]->getCenter().y, selectionObjects[0][0]->getCenter().z );
            Vector maxUBL( selectionObjects[0][0]->getCenter().x, maxCorner.y, maxCorner.z );
         
            for( int i = 0; i < 125; i++ ) //125 seeds * 8 quads = 1000seeds
            {
                vector<Vector> points; // Points to be rendered
                vector<Vector> color; //Color (local directions)

                //Lower Front Left********
                Vector quad1 = generateRandomSeed( minCorner, middle );
                performRTT( quad1, 1, points, color ); //First pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );  
                points.clear();
                color.clear();

                performRTT( quad1, -1, points, color ); //Second pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
                points.clear();
                color.clear();

                //Lower Front Right*******
                Vector quad2 = generateRandomSeed( minLFR, maxLFR );
                performRTT( quad2, 1, points, color ); //First pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
                points.clear();
                color.clear();

                performRTT( quad2, -1, points, color ); //Second pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
                points.clear();
                color.clear();

                //Upper Front Left*******
                Vector quad3 = generateRandomSeed( minUFL, maxUFL  );
                performRTT( quad3, 1, points, color ); //First pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
                points.clear();
                color.clear();

                performRTT( quad3, -1, points, color ); //Second pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
                points.clear();
                color.clear();

                //Upper Front Right******
                Vector quad4 = generateRandomSeed( minUFR, maxUFR );
                performRTT( quad4, 1, points, color ); //First pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
                points.clear();
                color.clear();

                performRTT( quad4, -1, points, color ); //Second pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
                points.clear();
                color.clear();

                //Lower Back Left********
                Vector quad5 = generateRandomSeed( minLBL, maxLBL );
                performRTT( quad5, 1, points, color ); //First pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
                points.clear();
                color.clear();

                performRTT( quad5, -1, points, color ); //Second pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
                points.clear();
                color.clear();

                //Lower Back Right*********
                Vector quad6 = generateRandomSeed( minLBR, maxLBR );
                performRTT( quad6, 1, points, color ); //First pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
                points.clear();
                color.clear();

                performRTT( quad6, -1, points, color ); //Second pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
                points.clear();
                color.clear();

                //Upper Back Left******
                Vector quad7 = generateRandomSeed( minUBL, maxUBL );
                performRTT( quad7, 1, points, color ); //First pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
                points.clear();
                color.clear();

                performRTT( quad7, -1, points, color ); //Second pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
                points.clear();
                color.clear();
            
                //Upper Back Right******
                Vector quad8 = generateRandomSeed( middle, maxCorner );
                performRTT( quad8, 1, points, color ); //First pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
                points.clear();
                color.clear();

                performRTT( quad8, -1, points, color ); //Second pass
                m_fibersRTT.push_back( points );
                m_colorsRTT.push_back( color );        
            }
            renderRTTFibers();
        }
    }
}

///////////////////////////////////////////////////////////////////////////
//Rendering stage
///////////////////////////////////////////////////////////////////////////
void RTTFibers::renderRTTFibers()
{
    for( unsigned int j = 0; j < m_fibersRTT.size() - 1; j+=2 )
    { 
	    if( (m_fibersRTT[j].size() + m_fibersRTT[j+1].size()) * getStep() > getMinFiberLength() && (m_fibersRTT[j].size() + m_fibersRTT[j+1].size()) * getStep() < getMaxFiberLength() )
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
    
    FMatrix valx0 = (1-dx) * m_tensorsMatrix[tensor_xyz]  + (dx) * m_tensorsMatrix[tensor_nxyz];
    FMatrix valx1 = (1-dx) * m_tensorsMatrix[tensor_xnyz] + (dx) * m_tensorsMatrix[tensor_nxnyz];
	
    const FMatrix valy0 = (1-dy) * valx0 + (dy) * valx1;
    valx0 = (1-dx) * m_tensorsMatrix[tensor_xynz]  + (dx) * m_tensorsMatrix[tensor_nxynz];
    valx1 = (1-dx) * m_tensorsMatrix[tensor_xnynz] + (dx) * m_tensorsMatrix[tensor_nxnynz];

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
    float cl = m_tensorsFA[t_number];
    float puncture = getPuncture();
    
    // Unit vectors of local basis (e1 > e2 > e3)
    ee1.x = tensor(0,0) * e1.x + 
            tensor(0,1) * e1.y + 
            tensor(0,2) * e1.z;

    ee1.y = tensor(1,0) * e1.x + 
            tensor(1,1) * e1.y + 
            tensor(1,2) * e1.z;

    ee1.z = tensor(2,0) * e1.x +
            tensor(2,1) * e1.y + 
            tensor(2,2) * e1.z;
    //e2
    ee2.x = tensor(0,0) * e2.x + 
            tensor(0,1) * e2.y + 
            tensor(0,2) * e2.z;

    ee2.y = tensor(1,0) * e2.x + 
            tensor(1,1) * e2.y + 
            tensor(1,2) * e2.z;

    ee2.z = tensor(2,0) * e2.x +
            tensor(2,1) * e2.y + 
            tensor(2,2) * e2.z;
    //e3
    ee3.x = tensor(0,0) * e3.x + 
            tensor(0,1) * e3.y + 
            tensor(0,2) * e3.z;

    ee3.y = tensor(1,0) * e3.x + 
            tensor(1,1) * e3.y + 
            tensor(1,2) * e3.z;

    ee3.z = tensor(2,0) * e3.x +
            tensor(2,1) * e3.y + 
            tensor(2,2) * e3.z;

    if (vin.Dot(ee1) < 0.0)
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
    float eValues[] = { m_tensorsEV[t_number][0], m_tensorsEV[t_number][1], m_tensorsEV[t_number][2] };
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

/////////////////////////////////////////////////////////////////////
// Classify (1 or 0) the 3 eigenVecs within Axis-Aligned vecs e1 > e2 > e3
////////////////////////////////////////////////////////////////////
void RTTFibers::setDiffusionAxis( const FMatrix &tensor, Vector& e1, Vector& e2, Vector& e3 )
{
    float lvx,lvy,lvz;

    //Find the 3 axes
    lvx = tensor(0,0) * tensor(0,0)
        + tensor(1,0) * tensor(1,0) 
        + tensor(2,0) * tensor(2,0);

    lvy = tensor(0,1) * tensor(0,1)
        + tensor(1,1) * tensor(1,1) 
        + tensor(2,1) * tensor(2,1);

    lvz = tensor(0,2) * tensor(0,2)
        + tensor(1,2) * tensor(1,2) 
        + tensor(2,2) * tensor(2,2);
	

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
void RTTFibers::performRTT(Vector seed, int bwdfwd, vector<Vector>& points, vector<Vector>& color)
{   
    //Vars
    Vector currPosition(seed); //Current PIXEL position
    Vector nextPosition; //Next Pixel position
    Vector e1(0,0,0); //Direction of the tensor (axis aligned)
    Vector e2(0,0,0); //Direction of the tensor (axis aligned)
    Vector e3(0,0,0); //Direction of the tensor (axis aligned)
    Vector currDirection, nextDirection; //Directions re-aligned 

    unsigned int tensorNumber; 
    int currVoxelx, currVoxely, currVoxelz;
    float FAvalue, angle; 
    float FAthreshold = getFAThreshold();
    float angleThreshold = getAngleThreshold();
    float step = getStep();

    int columns = DatasetManager::getInstance()->getColumns();
    int rows    = DatasetManager::getInstance()->getRows();
    int frames  = DatasetManager::getInstance()->getFrames();

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

    if( tensorNumber < m_tensorsMatrix.size() )
    {
        //Use Interpolation
        if( RTTrackingHelper::getInstance()->isTensorsInterpolated() )
        {
            tensor = trilinearInterp( currPosition.x, currPosition.y, currPosition.z );
        }
        else
        {
            tensor = m_tensorsMatrix[tensorNumber];
        }

        //Find the MAIN axis
        setDiffusionAxis( tensor, e1, e2, e3 );
        
        //Align the main direction my mult AxisAlign * tensorMatrix
        currDirection.x = tensor(0,0) * e1.x + 
                          tensor(0,1) * e1.y + 
                          tensor(0,2) * e1.z;

        currDirection.y = tensor(1,0) * e1.x + 
                          tensor(1,1) * e1.y + 
                          tensor(1,2) * e1.z;

        currDirection.z = tensor(2,0) * e1.x +
                          tensor(2,1) * e1.y + 
                          tensor(2,2) * e1.z;
       
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

        if( tensorNumber < m_tensorsMatrix.size() )
        {
            //Use interpolation
            if( RTTrackingHelper::getInstance()->isTensorsInterpolated() )
            {
                tensor = trilinearInterp( nextPosition.x, nextPosition.y, nextPosition.z );
            }
            else
            {
                tensor = m_tensorsMatrix[tensorNumber];
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
            FAvalue = m_tensorsFA[tensorNumber];
            
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
                
                if( tensorNumber > m_tensorsMatrix.size() ) //Out of anatomy
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
                    tensor = m_tensorsMatrix[tensorNumber];
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
                FAvalue = m_tensorsFA[tensorNumber];
                
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
    if (!checkFramebufferStatus()) {
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
    if (glGetError() != GL_NO_ERROR) {
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
// 	printf("OpenGL ERROR: ");
// 	printf((char*)errStr);
// 	printf("(Label: ");
// 	printf(label);
// 	printf(")\n.");
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
