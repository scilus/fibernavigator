/////////////////////////////////////////////////////////////////////////////
// Name:            Octree.h
// Author:          Maxime Chamberland
// Creation Date:   aout 2011
//
// Description: Octree class.
//
//The implementation is "Hardcoded" for a level 2 Octree so far
//and further ameliorations will be made for recursion.
/////////////////////////////////////////////////////////////////////////////
#ifndef OCTREE_H_
#define OCTREE_H_

#include <vector>

class SelectionObject;

class Octree 
{
public:
    Octree( int lvl, const std::vector< float > &pointArray, int nb ); //Constructor
    ~Octree(); //Destructor

    //Functions
    std::vector< int > getPointsInside( SelectionObject* selectionObject );
    std::vector< int > getPointsInBoundingBox( int xMin, int yMin, int zMin, int xMax, int yMax, int zMax );
    
    // Updates the data to represent the fiber structure when fibers are flipped.
    void flipX();
    void flipY();
    void flipZ();

private:
    //Octree divisions
    std::vector< std::vector<int> > m_quad1; 
    std::vector< std::vector<int> > m_quad2;
    std::vector< std::vector<int> > m_quad3;
    std::vector< std::vector<int> > m_quad4;
    std::vector< std::vector<int> > m_quad5;
    std::vector< std::vector<int> > m_quad6;
    std::vector< std::vector<int> > m_quad7;
    std::vector< std::vector<int> > m_quad8;

    std::vector<int> m_id; //Points selected

    int m_level;  //Lvl of subdivision
    int m_countPoints; //Nb of points from dataset
    float m_maxPointX; //Quad corner
    float m_maxPointY; //Quad corner
    float m_maxPointZ; //Quad corner
    float m_minPointX; //Quad corner
    float m_minPointY; //Quad corner
    float m_minPointZ; //Quad corner
    const std::vector< float > &m_pointArray; // Points (x,y,z)
    std::vector< float > m_boxMax; //SelectionObject
    std::vector< float > m_boxMin; //SelectionObject
    
    void findBoundingBox(); //BB of the points
    void classifyPoints(); //Classify the points according to their position
    void subClassifyPoints( std::vector<int> i_bigVolume, int i_xmin, int i_ymin, int i_zmin, int i_xmax, int i_ymax, int i_zmax, std::vector< std::vector<int > > &o_tree); //Subclassify for lvl 2
    
    void flipXInternalVector( std::vector< std::vector< int > > &vectToFlip );
    void flipYInternalVector( std::vector< std::vector< int > > &vectToFlip );
    void flipZInternalVector( std::vector< std::vector< int > > &vectToFlip );
    
    void boxTest( int i_minx, int i_miny, int i_minz, int i_maxx, int i_maxy, int i_maxz, int lvl, const std::vector< std::vector<int > > &currSub);
    void ellipsoidTest( int i_minx, int i_miny, int i_minz, int i_maxx, int i_maxy, int i_maxz, int lvl, const std::vector< std::vector<int > > &currSub);
    
};

#endif /*OCTREE_H_*/
