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

#include "DatasetHelper.h"

class Octree {
public:
    Octree(int lvl,const std::vector< float > &pointArray,int nb, DatasetHelper* dh); //Constructor
    ~Octree(); //Destructor

    //Functions
    vector<int> getPointsInside(SelectionObject* selectionObject);
    void boxTest( int i_minx, int i_miny, int i_minz, int i_maxx, int i_maxy, int i_maxz, int lvl, const vector<vector<int > >& currSub);
    void ellipsoidTest( int i_minx, int i_miny, int i_minz, int i_maxx, int i_maxy, int i_maxz, int lvl, const vector<vector<int > >& currSub);



private:
    //Octree divisions
    vector<vector<int> > m_quad1; 
    vector<vector<int> > m_quad2;
    vector<vector<int> > m_quad3;
    vector<vector<int> > m_quad4;
    vector<vector<int> > m_quad5;
    vector<vector<int> > m_quad6;
    vector<vector<int> > m_quad7;
    vector<vector<int> > m_quad8;

    vector<int> m_id; //Points selected
    DatasetHelper* m_dh;

    int m_level;  //Lvl of subdivision
    int m_countPoints; //Nb of points from dataset
    float m_maxPointX; //Quad corner
    float m_maxPointY; //Quad corner
    float m_maxPointZ; //Quad corner
    float m_minPointX; //Quad corner
    float m_minPointY; //Quad corner
    float m_minPointZ; //Quad corner
    const vector< float > &m_pointArray; // Points (x,y,z)
    vector< float > m_boxMax; //SelectionObject
    vector< float > m_boxMin; //SelectionObject
    
    void findBoundingBox(); //BB of the points
    void classifyPoints(); //Classify the points according to their position
    void subClassifyPoints(vector<int> i_bigVolume, int i_xmin, int i_ymin, int i_zmin, int i_xmax, int i_ymax, int i_zmax, vector<vector<int > >& o_tree); //Subclassify for lvl 2
    
};

#endif /*OCTREE_H_*/
