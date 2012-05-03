#include "Octree.h"

#include "DatasetManager.h"
#include "../Logger.h"
#include "../gui/SelectionObject.h"

#include <algorithm>
#include <vector>
using std::vector;

//////////////////////////////////////////
/*Constructor*/
//////////////////////////////////////////
Octree::Octree( int lvl, const std::vector< float > &pointArray, int nb )
:   m_level(lvl),
    m_countPoints(nb),
    m_pointArray(pointArray)    
{
    Logger::getInstance()->print( wxT( "Building Octree..." ), LOGLEVEL_MESSAGE );
    
    findBoundingBox();
    classifyPoints();
    
    Logger::getInstance()->print( wxT( "Octree done" ), LOGLEVEL_MESSAGE );
}

//////////////////////////////////////////
/*Destructor*/
//////////////////////////////////////////
Octree::~Octree()
{
}

//////////////////////////////////////////
//Bounding box of the points
//////////////////////////////////////////
void Octree::findBoundingBox()
{
    m_maxPointX = m_pointArray[0];
    m_maxPointY = m_pointArray[1];
    m_maxPointZ = m_pointArray[2];

    m_minPointX = m_pointArray[0];
    m_minPointY = m_pointArray[1];
    m_minPointZ = m_pointArray[2];

    //Find the bounding box for the dataSet
    for(int i=0; i < m_countPoints; i++)
    {
        if(m_pointArray[i*3] > m_maxPointX)
            m_maxPointX = m_pointArray[i*3];

        if(m_pointArray[i*3+1] > m_maxPointY)
            m_maxPointY = m_pointArray[i*3+1];

        if(m_pointArray[i*3+2] > m_maxPointZ)
            m_maxPointZ = m_pointArray[i*3+2];

        if(m_pointArray[i*3] < m_minPointX)
            m_minPointX = m_pointArray[i*3];

        if(m_pointArray[i*3+1] < m_minPointY)
            m_minPointY = m_pointArray[i*3+1];

        if(m_pointArray[i*3+2] < m_minPointZ)
            m_minPointZ = m_pointArray[i*3+2];
    }
}

//////////////////////////////////////////
//Classify points from a dataset into specifics octree divisions
// front:   in front of XY plane
// back:    behind XY plane
// bottom:  under XZ plane
// up:      over XZ plane
// left:    left of YZ plane
// right:   right of YZ plane
/////////////////////////////////////////
void Octree::classifyPoints()
{
    vector<int> bigVolume1;
    vector<int> bigVolume2;
    vector<int> bigVolume3;
    vector<int> bigVolume4;
    vector<int> bigVolume5;
    vector<int> bigVolume6;
    vector<int> bigVolume7;
    vector<int> bigVolume8;

    //Cutting planes
    float midX = (m_maxPointX + m_minPointX) / 2.0f;
    float midY = (m_maxPointY + m_minPointY) / 2.0f;
    float midZ = (m_maxPointZ + m_minPointZ) / 2.0f;

    float posX, posY, posZ;
        
    //fill the tree 
    for(int i=0; i < m_countPoints; i++)
    {
        posX = m_pointArray[i*3];
        posY = m_pointArray[i*3+1];
        posZ = m_pointArray[i*3+2];
         
        if(posX <= midX && posY <= midY && posZ <= midZ)  //front bottom left   
            bigVolume1.push_back(i);

        else if(posX <= midX && posY > midY && posZ <= midZ) //front up left
            bigVolume2.push_back(i);    

        else if(posX > midX && posY <= midY && posZ <= midZ) //front bottom right
            bigVolume3.push_back(i);  

        else if(posX > midX && posY > midY && posZ <= midZ) //front up right
            bigVolume4.push_back(i);

        else if(posX <= midX && posY <= midY && posZ > midZ) //back bottom left
            bigVolume5.push_back(i);
    
        else if(posX <= midX && posY > midY && posZ > midZ) //back up left
            bigVolume6.push_back(i);
   
        else if(posX > midX && posY <= midY && posZ > midZ) //back bottom right
            bigVolume7.push_back(i);

        else //back up right
            bigVolume8.push_back(i);
    }
    
    //Split each of the 8 boxes, into 8miniboxes
    subClassifyPoints( bigVolume1, m_minPointX, m_minPointY, m_minPointZ, midX, midY, midZ, m_quad1 ); //front bottom left 
    subClassifyPoints( bigVolume2, m_minPointX, midY, m_minPointZ, midX, m_maxPointY, midZ, m_quad2 ); //front up left
    subClassifyPoints( bigVolume3, midX, m_minPointY, m_minPointZ, m_maxPointX, midY, midZ, m_quad3 ); //front bottom right
    subClassifyPoints( bigVolume4, midX, midY, m_minPointZ, m_maxPointX, m_maxPointY, midZ, m_quad4 ); //front up right
    subClassifyPoints( bigVolume5, m_minPointX, m_minPointY, midZ, midX, midY, m_maxPointZ, m_quad5 ); //back bottom left
    subClassifyPoints( bigVolume6, m_minPointX, midY, midZ, midX, m_maxPointY, m_maxPointZ, m_quad6 ); //back up left
    subClassifyPoints( bigVolume7, midX, m_minPointY, midZ, m_maxPointX, midY, m_maxPointZ, m_quad7 ); //back bottom right
    subClassifyPoints( bigVolume8 ,midX, midY, midZ, m_maxPointX, m_maxPointY, m_maxPointZ, m_quad8 ); //back up right

}

//////////////////////////////////////////
//Subdivide the space
//////////////////////////////////////////
void Octree::subClassifyPoints( vector<int> i_bigVolume, int i_xmin, int i_ymin, int i_zmin, int i_xmax, int i_ymax, int i_zmax, vector<vector<int > >& o_tree )
{
    vector<int> volume1;
    vector<int> volume2;
    vector<int> volume3;
    vector<int> volume4;
    vector<int> volume5;
    vector<int> volume6;
    vector<int> volume7;
    vector<int> volume8;

    //Cuting planes
    float midX = (i_xmax + i_xmin) / 2.0f;
    float midY = (i_ymax + i_ymin) / 2.0f;
    float midZ = (i_zmax + i_zmin) / 2.0f;

    float posX,posY,posZ;
    int indice;
        
    //fill the tree 
    for(unsigned int i=0; i < i_bigVolume.size(); i++)
    {
        indice = i_bigVolume[i];
        posX = m_pointArray[indice*3];
        posY = m_pointArray[indice*3+1];
        posZ = m_pointArray[indice*3+2];
         
        if(posX <= midX && posY <= midY && posZ <= midZ)  //front bottom left   
            volume1.push_back(indice);

        else if(posX <= midX && posY > midY && posZ <= midZ) //front up left
            volume2.push_back(indice);    

        else if(posX > midX && posY <= midY && posZ <= midZ) //front bottom right
            volume3.push_back(indice);  

        else if(posX > midX && posY > midY && posZ <= midZ) //front up right
            volume4.push_back(indice);

        else if(posX <= midX && posY <= midY && posZ > midZ) //back bottom left
            volume5.push_back(indice);
    
        else if(posX <= midX && posY > midY && posZ > midZ) //back up left
            volume6.push_back(indice);
   
        else if(posX > midX && posY <= midY && posZ > midZ) //back bottom right
            volume7.push_back(indice);

        else //back up right
            volume8.push_back(indice);
    }
    
    o_tree.push_back(volume1);
    o_tree.push_back(volume2);
    o_tree.push_back(volume3);
    o_tree.push_back(volume4);
    o_tree.push_back(volume5);
    o_tree.push_back(volume6);
    o_tree.push_back(volume7);
    o_tree.push_back(volume8);

}

//////////////////////////////////////////
//Return points that are inside a BOX or an ELLPISOID
//////////////////////////////////////////
vector<int> Octree::getPointsInside( SelectionObject* i_selectionObject )
{
    Vector l_center = i_selectionObject->getCenter();
    Vector l_size   = i_selectionObject->getSize();

    m_boxMin.resize( 3 );
    m_boxMax.resize( 3 );

    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    m_boxMin[0] = l_center.x - l_size.x / 2 * voxelX;
    m_boxMax[0] = l_center.x + l_size.x / 2 * voxelX;
    m_boxMin[1] = l_center.y - l_size.y / 2 * voxelY;
    m_boxMax[1] = l_center.y + l_size.y / 2 * voxelY;
    m_boxMin[2] = l_center.z - l_size.z / 2 * voxelZ;
    m_boxMax[2] = l_center.z + l_size.z / 2 * voxelZ;
    
    m_id.clear();
    if(i_selectionObject->getSelectionType() == BOX_TYPE)
        boxTest( m_minPointX, m_minPointY, m_minPointZ, m_maxPointX, m_maxPointY, m_maxPointZ, 0, m_quad1 );
    else
        ellipsoidTest( m_minPointX, m_minPointY, m_minPointZ, m_maxPointX, m_maxPointY, m_maxPointZ, 0, m_quad1 );

    return m_id;

}

//////////////////////////////////////////
// Return points that are inside a bounding box
// defined from the coordinates of its corners.
//////////////////////////////////////////
vector< int > Octree::getPointsInBoundingBox( int xMin, int yMin, int zMin, int xMax, int yMax, int zMax )
{
    m_id.clear();
    m_boxMin.resize( 3 );
    m_boxMax.resize( 3 );
    
    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    m_boxMin[0] = xMin * voxelX;
    m_boxMin[1] = yMin * voxelY;
    m_boxMin[2] = zMin * voxelZ;
    m_boxMax[0] = xMax * voxelX;
    m_boxMax[1] = yMax * voxelY;
    m_boxMax[2] = zMax * voxelZ;
    
    boxTest( m_minPointX, m_minPointY, m_minPointZ, m_maxPointX, m_maxPointY, m_maxPointZ, 0, m_quad1 );
    
    return m_id;
}

void Octree::flipX()
{
    std::swap( m_quad1, m_quad3 );
    std::swap( m_quad2, m_quad4 );
    std::swap( m_quad5, m_quad7 );
    std::swap( m_quad6, m_quad8 );
    
    flipXInternalVector( m_quad1 );
    flipXInternalVector( m_quad2 );
    flipXInternalVector( m_quad3 );
    flipXInternalVector( m_quad4 );
    flipXInternalVector( m_quad5 );
    flipXInternalVector( m_quad6 );
    flipXInternalVector( m_quad7 );
    flipXInternalVector( m_quad8 );
}

void Octree::flipY()
{
    std::swap( m_quad1, m_quad2 );
    std::swap( m_quad3, m_quad4 );
    std::swap( m_quad5, m_quad6 );
    std::swap( m_quad7, m_quad8 );
    
    flipYInternalVector( m_quad1 );
    flipYInternalVector( m_quad2 );
    flipYInternalVector( m_quad3 );
    flipYInternalVector( m_quad4 );
    flipYInternalVector( m_quad5 );
    flipYInternalVector( m_quad6 );
    flipYInternalVector( m_quad7 );
    flipYInternalVector( m_quad8 );
}

void Octree::flipZ()
{
    std::swap( m_quad1, m_quad5 );
    std::swap( m_quad3, m_quad7 );
    std::swap( m_quad2, m_quad6 );
    std::swap( m_quad4, m_quad8 );
    
    flipZInternalVector( m_quad1 );
    flipZInternalVector( m_quad2 );
    flipZInternalVector( m_quad3 );
    flipZInternalVector( m_quad4 );
    flipZInternalVector( m_quad5 );
    flipZInternalVector( m_quad6 );
    flipZInternalVector( m_quad7 );
    flipZInternalVector( m_quad8 );
}

void Octree::flipXInternalVector( vector< vector< int > > &vectToFlip )
{
    std::swap( vectToFlip[0], vectToFlip[2] );
    std::swap( vectToFlip[1], vectToFlip[3] );
    std::swap( vectToFlip[4], vectToFlip[6] );
    std::swap( vectToFlip[5], vectToFlip[7] );
}

void Octree::flipYInternalVector( vector< vector< int > > &vectToFlip )
{
    std::swap( vectToFlip[0], vectToFlip[1] );
    std::swap( vectToFlip[2], vectToFlip[3] );
    std::swap( vectToFlip[4], vectToFlip[5] );
    std::swap( vectToFlip[6], vectToFlip[7] );
}

void Octree::flipZInternalVector( vector< vector< int > > &vectToFlip )
{
    std::swap( vectToFlip[0], vectToFlip[4] );
    std::swap( vectToFlip[2], vectToFlip[6] );
    std::swap( vectToFlip[1], vectToFlip[5] );
    std::swap( vectToFlip[3], vectToFlip[7] );
}

//////////////////////////////////////////
// BOX case
//////////////////////////////////////////
void Octree::boxTest( int i_minx, int i_miny, int i_minz, int i_maxx, int i_maxy, int i_maxz, int lvl, const vector<vector<int > >& currSub )
{
    if(lvl == 2)
        return;

    int indice;

    //Box extremities
    float xMin = m_boxMin[0];
    float xMax = m_boxMax[0];
    float yMin = m_boxMin[1];
    float yMax = m_boxMax[1];
    float zMin = m_boxMin[2];
    float zMax = m_boxMax[2];

    //Octree planes cutting
    float midX = (i_maxx + i_minx)  / 2.0f;
    float midY = (i_maxy + i_miny) / 2.0f;
    float midZ = (i_maxz + i_minz) / 2.0f;
    

    //Checks in which Octree the selection Box is
    vector<bool> inBoxes(8, false);

    if(xMin < midX && xMax > i_minx)
        if(yMin < midY && yMax > i_miny)
            if(zMin < midZ && zMax > i_minz)
            {
                inBoxes[0] = true;
                boxTest( i_minx, i_miny, i_minz, midX, midY, midZ, lvl+1, m_quad1 ); // Front Bottom Left

            }

   if(xMin < midX && xMax > i_minx)
        if(yMin < i_maxy && yMax > midY)
            if(zMin < midZ && zMax > i_minz)
            {
                inBoxes[1] = true;
                boxTest( i_minx, midY, i_minz, midX, i_maxy, midZ, lvl+1, m_quad2 ); //Front Up Left
            }

    if(xMin < i_maxx && xMax > midX)
        if(yMin < midY && yMax > i_miny)
            if(zMin < midZ && zMax > i_minz)
            {
                inBoxes[2] = true;
                boxTest( midX, i_miny, i_minz, i_maxx, midY, midZ, lvl+1, m_quad3 ); //Front Bottom Right
            }

    if(xMin < i_maxx && xMax > midX)
        if(yMin < i_maxy && yMax > midY)
            if(zMin < midZ && zMax > i_minz)
            {
                inBoxes[3] = true;
                boxTest( midX, midY, i_minz, i_maxx, i_maxy, midZ, lvl+1, m_quad4 ); //Front Up Right
            }

    if(xMin < midX && xMax > i_minx)
        if(yMin < midY && yMax > i_miny)
            if(zMin < i_maxz && zMax > midZ)
            {
                inBoxes[4] = true;
                boxTest( i_minx, i_miny, midZ, midX, midY, i_maxz, lvl+1, m_quad5); //Back Bottom Left
            }

    if(xMin < midX && xMax > i_minx)
        if(yMin < i_maxy && yMax > midY)
            if(zMin < i_maxz && zMax > midZ)
            {
                inBoxes[5] = true;
                boxTest( i_minx, midY, midZ, midX, i_maxy, i_maxz, lvl+1, m_quad6 ); //Back Up Left
            }

    if(xMin < i_maxx && xMax > midX)
        if(yMin < midY && yMax > i_miny)
            if(zMin < i_maxz && zMax > midZ)
            {
                inBoxes[6] = true;
                boxTest(midX, i_miny, midZ, i_maxx, midY, i_maxz, lvl+1, m_quad7 ); //Back Bottom Right

            }

    if(xMin < i_maxx && xMax > midX)
        if(yMin < i_maxy && yMax > midY)
            if(zMin < i_maxz && zMax > midZ)
            {
                inBoxes[7] = true;
                boxTest(midX, midY , midZ, i_maxx, i_maxy, i_maxz, lvl+1, m_quad8 ); //Back up Right
            }  

    if(lvl == 1)
    {
        float posX,posY,posZ;
        //Checks if any fibers are INSIDE the selection box, for Octree regions that are TRUE
        for(unsigned int k=0; k < inBoxes.size(); k++)
        {
            if(inBoxes[k])
            {
                for(unsigned int i = 0; i < currSub[k].size(); i++)
                {
                    indice = currSub[k][i];
                    posX = m_pointArray[indice*3];
                    posY = m_pointArray[indice*3+1];
                    posZ = m_pointArray[indice*3+2];


                    if(posX <= m_boxMax[0] && posX >= m_boxMin[0] && 
                       posY <= m_boxMax[1] && posY >= m_boxMin[1] &&
                       posZ <= m_boxMax[2] && posZ >= m_boxMin[2] ) //If INTO the box
                    {
                        m_id.push_back(indice);
                    }
           
                }
            }
        }
    }
}

//////////////////////////////////////////
//ELLIPSOID case
//////////////////////////////////////////
void Octree::ellipsoidTest( int i_minx, int i_miny, int i_minz, int i_maxx, int i_maxy, int i_maxz, int lvl, const vector<vector<int > >& currSub )
{
    if(lvl == 2)
        return;

    int indice;

    //Box extremities
    float xMin = m_boxMin[0];
    float xMax = m_boxMax[0];
    float yMin = m_boxMin[1];
    float yMax = m_boxMax[1];
    float zMin = m_boxMin[2];
    float zMax = m_boxMax[2];

    //Octree planes cutting
    float midX = (i_maxx + i_minx)  / 2.0f;
    float midY = (i_maxy + i_miny) / 2.0f;
    float midZ = (i_maxz + i_minz) / 2.0f;
    

    //Checks in which Octree the selection Box is 
    vector<bool> inBoxes(8, false);

    if(xMin < midX && xMax > i_minx)
        if(yMin < midY && yMax > i_miny)
            if(zMin < midZ && zMax > i_minz)
            {
                inBoxes[0] = true;
                boxTest( i_minx, i_miny, i_minz, midX, midY, midZ, lvl+1, m_quad1 ); // Front Bottom Left

            }

   if(xMin < midX && xMax > i_minx)
        if(yMin < i_maxy && yMax > midY)
            if(zMin < midZ && zMax > i_minz)
            {
                inBoxes[1] = true;
                boxTest( i_minx, midY, i_minz, midX, i_maxy, midZ, lvl+1, m_quad2 ); //Front Up Left
            }

    if(xMin < i_maxx && xMax > midX)
        if(yMin < midY && yMax > i_miny)
            if(zMin < midZ && zMax > i_minz)
            {
                inBoxes[2] = true;
                boxTest( midX, i_miny, i_minz, i_maxx, midY, midZ, lvl+1, m_quad3 ); //Front Bottom Right
            }

    if(xMin < i_maxx && xMax > midX)
        if(yMin < i_maxy && yMax > midY)
            if(zMin < midZ && zMax > i_minz)
            {
                inBoxes[3] = true;
                boxTest( midX, midY, i_minz, i_maxx, i_maxy, midZ, lvl+1, m_quad4 ); //Front Up Right
            }

    if(xMin < midX && xMax > i_minx)
        if(yMin < midY && yMax > i_miny)
            if(zMin < i_maxz && zMax > midZ)
            {
                inBoxes[4] = true;
                boxTest( i_minx, i_miny, midZ, midX, midY, i_maxz, lvl+1, m_quad5 ); //Back Bottom Left
            }

    if(xMin < midX && xMax > i_minx)
        if(yMin < i_maxy && yMax > midY)
            if(zMin < i_maxz && zMax > midZ)
            {
                inBoxes[5] = true;
                boxTest( i_minx, midY, midZ, midX, i_maxy, i_maxz, lvl+1, m_quad6 ); //Back Up Left
            }

    if(xMin < i_maxx && xMax > midX)
        if(yMin < midY && yMax > i_miny)
            if(zMin < i_maxz && zMax > midZ)
            {
                inBoxes[6] = true;
                boxTest( midX, i_miny, midZ, i_maxx, midY, i_maxz, lvl+1, m_quad7 ); //Back Bottom Right

            }

    if(xMin < i_maxx && xMax > midX)
        if(yMin < i_maxy && yMax > midY)
            if(zMin < i_maxz && zMax > midZ)
            {
                inBoxes[7] = true;
                boxTest( midX, midY ,midZ, i_maxx, i_maxy, i_maxz, lvl+1, m_quad8 ); //Back up Right
            }  

    if(lvl == 1)
    {
        float posX,posY,posZ;
        float l_axisRadius  = ( m_boxMax[0]  - m_boxMin[0] ) / 2.0f;
        float l_axis1Radius = ( m_boxMax[1] - m_boxMin[1] ) / 2.0f;
        float l_axis2Radius = ( m_boxMax[2] - m_boxMin[2] ) / 2.0f;
        float l_axisCenter  = m_boxMax[0]  - l_axisRadius;
        float l_axis1Center = m_boxMax[1] - l_axis1Radius;
        float l_axis2Center = m_boxMax[2] - l_axis2Radius;

        //Checks if any fibers are INSIDE the selection box, for Octree regions that are TRUE
        for(unsigned int k=0; k < inBoxes.size(); k++)
        {
            if(inBoxes[k])
            {
                for(unsigned int i = 0; i < currSub[k].size(); i++)
                {
                    indice = currSub[k][i];
                    posX = m_pointArray[indice*3];
                    posY = m_pointArray[indice*3+1];
                    posZ = m_pointArray[indice*3+2];

                    if( (posX  - l_axisCenter)*(posX  - l_axisCenter) / ( l_axisRadius  * l_axisRadius  ) + 
                        (posY - l_axis1Center)*(posY - l_axis1Center) / ( l_axis1Radius * l_axis1Radius ) + 
                        (posZ - l_axis2Center)*(posZ - l_axis2Center) / ( l_axis2Radius * l_axis2Radius ) <= 1.0f )
                    {
                        m_id.push_back(indice);
                    }
                }
            }
        }
    }
}

