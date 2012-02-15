// TriangleMesh.cpp
#include "TriangleMesh.h"

#include "LoopSubD.h"
#include "../../dataset/DatasetManager.h"

#include <algorithm>
using std::for_each;

#include <string>

namespace
{
    class TriangleTensorCalculator
    {
    public:
        TriangleTensorCalculator( TriangleMesh *pTriMesh, float columns, float rows, float frames )
        :   m_pTriMesh( pTriMesh ),
            m_columns( columns ),
            m_frames( frames ),
            m_rows( rows )
        {
        }

        int operator()( const int i )
        {
            using std::min;
            using std::max;

            Vector p = m_pTriMesh->getTriangleCenter( i );
            int x = min( (int)(m_columns - 1), max( 0, (int)( p[0] + 0.5 ) ) );
            int y = min( (int)(m_rows    - 1), max( 0, (int)( p[1] + 0.5 ) ) );
            int z = min( (int)(m_frames  - 1), max( 0, (int)( p[2] + 0.5 ) ) );
            return x + y * m_columns + z * m_columns * m_rows;
        }
    private:
        TriangleTensorCalculator();

    private:
        TriangleMesh *m_pTriMesh;
        const float m_columns;
        const float m_frames;
        const float m_rows;
    };
}


// Construction
TriangleMesh::TriangleMesh (DatasetHelper* dh)
{
    m_dh = dh;
    numVerts     = 0;
    numTris         = 0;

    isCleaned = false;

    m_vertNormalsCalculated = false;
    m_neighborsCalculated = false;
    m_triangleTensorsCalculated = false;

    defaultColor = wxColour(200, 200, 200, 255);
}

TriangleMesh::~TriangleMesh ()
{
    clearMesh();
}

void TriangleMesh::clearMesh()
{
    vertices.clear();
    vertNormals.clear();
    vertColors.clear();
    vIsInTriangle.clear();

    triangles.clear();
    triangleTensor.clear();
    triangleColor.clear();
    neighbors.clear();
    triNormals.clear();

    numVerts     = 0;
    numTris      = 0;

    isCleaned = false;
    m_vertNormalsCalculated = false;
    m_neighborsCalculated = false;
    m_triangleTensorsCalculated = false;
}


void TriangleMesh::addVert(const Vector newVert)
{
    vertices.push_back( newVert );
    numVerts = vertices.size();
    vIsInTriangle.resize(numVerts);
    vertColors.resize(numVerts);
}

void TriangleMesh::fastAddVert(const Vector newVert)
{
    vertices[numVerts] = newVert ;
    ++numVerts;
}

void TriangleMesh::addVert(const float x, const float y, const float z)
{
    addVert(Vector(x,y,z));
}

void TriangleMesh::reserveVerts(const int size)
{
    vertices.reserve(size);
    vertNormals.reserve(size);
}

void TriangleMesh::resizeVerts(const int size)
{
    vertices.resize(size);
    vIsInTriangle.resize(size);
}

void TriangleMesh::addTriangle(const int vertA, const int vertB, const int vertC)
{
    addTriangle(vertA, vertB, vertC, 0);
}

void TriangleMesh::addTriangle(const int vertA, const int vertB, const int vertC, const int tensorIndex)
{
    Triangle t = {{vertA, vertB, vertC}};
    triangles.push_back(t);
    triNormals.push_back(calcTriangleNormal(t));
    vIsInTriangle[vertA].push_back(numTris);
    vIsInTriangle[vertB].push_back(numTris);
    vIsInTriangle[vertC].push_back(numTris);
    std::vector<int> v(3,-1);
    neighbors.push_back( v );
    numTris = triangles.size();
    triangleTensor.push_back(tensorIndex);
    triangleColor.push_back(defaultColor);
}

void TriangleMesh::fastAddTriangle(const int vertA, const int vertB, const int vertC)
{
    Triangle t = {{vertA, vertB, vertC}};
    triangles[numTris] = t;
    triNormals[numTris] = calcTriangleNormal(t);
    vIsInTriangle[vertA].push_back(numTris);
    vIsInTriangle[vertB].push_back(numTris);
    vIsInTriangle[vertC].push_back(numTris);
    ++numTris;
}

void TriangleMesh::reserveTriangles(const int size)
{
    triangles.reserve(size);
    triNormals.reserve(size);
    triangleTensor.reserve(size);
    triangleColor.reserve(size);
    neighbors.reserve(size);
}

void TriangleMesh::resizeTriangles(const int size)
{
    triangles.resize(size);
    triNormals.resize(size);
    triangleTensor.resize(size,0);
    triangleColor.resize(size, defaultColor);
    std::vector<int> v(3,-1);
    neighbors.resize(size, v);
}

void TriangleMesh::setTriangleColor(const unsigned int triNum, const float r, const float g, const float b, const float a)
{
    unsigned char red = (unsigned char)(r * 255);
    unsigned char green = (unsigned char)(g * 255);
    unsigned char blue= (unsigned char)(b * 255);
    unsigned char alpha = (unsigned char)(a * 255);

    triangleColor[triNum].Set(red, green, blue, alpha);
}

void TriangleMesh::setTriangleColor(const unsigned int triNum, const float r, const float g, const float b)
{
    unsigned char red = (unsigned char)(r * 255);
    unsigned char green = (unsigned char)(g * 255);
    unsigned char blue= (unsigned char)(b * 255);
    triangleColor[triNum].Set(red, green, blue, triangleColor[triNum].Alpha());
}

void TriangleMesh::setTriangleAlpha(const unsigned int triNum, const float a)
{
    unsigned char alpha = (unsigned char)(a * 255);
    triangleColor[triNum].Set(triangleColor[triNum].Red(), triangleColor[triNum].Green(), triangleColor[triNum].Blue(), alpha);
}

void TriangleMesh::setTriangleRed(const unsigned int triNum, const float r)
{
    unsigned char red = (unsigned char)(r * 255);
    triangleColor[triNum].Set(red, triangleColor[triNum].Green(), triangleColor[triNum].Blue(), triangleColor[triNum].Alpha());
}

void TriangleMesh::setTriangleGreen(const unsigned int triNum, const float g)
{
    unsigned char green = (unsigned char)(g * 255);
    triangleColor[triNum].Set(triangleColor[triNum].Red(), green, triangleColor[triNum].Blue(), triangleColor[triNum].Alpha());
}

void TriangleMesh::setTriangleBlue(const unsigned int triNum, const float b)
{
    unsigned char blue = (unsigned char)(b * 255);
    triangleColor[triNum].Set(triangleColor[triNum].Red(), triangleColor[triNum].Green(), blue, triangleColor[triNum].Alpha());
}

Vector TriangleMesh::calcTriangleNormal(const Triangle t)
{
    Vector v1 = vertices[t.pointID[1]] - vertices[t.pointID[0]];
    Vector v2 = vertices[t.pointID[2]] - vertices[t.pointID[0]];

    Vector tempNormal = v1.Cross(v2);
    tempNormal.normalize();
    return tempNormal;
}

Vector TriangleMesh::calcTriangleNormal(const int triNum)
{

    Vector v1 = vertices[triangles[triNum].pointID[1]] - vertices[triangles[triNum].pointID[0]];
    Vector v2 = vertices[triangles[triNum].pointID[2]] - vertices[triangles[triNum].pointID[0]];

    Vector tempNormal = v1.Cross(v2);
    tempNormal.normalize();
    return tempNormal;
}

Vector TriangleMesh::getVertex (const int triNum, int pos)
{
    if (pos < 0 || pos > 2) pos = 0;
    return vertices[triangles[triNum].pointID[pos]];
}

Vector TriangleMesh::getVertNormal(const int vertNum)
{
    if ( !m_vertNormalsCalculated )
        calcVertNormals();
    return vertNormals[vertNum];
}

Vector TriangleMesh::calcVertNormal(const int vertNum)
{
    Vector sum(0,0,0);

    for(size_t i = 0 ; i < vIsInTriangle[vertNum].size() ; ++i)
    {
        sum = sum + triNormals[vIsInTriangle[vertNum][i]];
    }
    sum.normalize();
    return sum;
}

void TriangleMesh::setVertColor( const int vertNum, const wxColour color)
{
    vertColors[vertNum] = color;
}

wxColour TriangleMesh::getVertColor(const int vertNum)
{
    return vertColors[vertNum];
}

void TriangleMesh::calcVertNormals()
{
    vertNormals.clear();
    vertNormals.resize(numVerts);
    for ( int i = 0 ; i < numVerts ; ++i)
    {
        vertNormals[i] = calcVertNormal(i);
    }

    m_vertNormalsCalculated = true;
}

int TriangleMesh::getNeighbor(const unsigned int coVert1, const unsigned int coVert2, const unsigned int triangleNum)
{
    std::vector<unsigned int>candidates = vIsInTriangle[coVert1];
    std::vector<unsigned int>compares   = vIsInTriangle[coVert2];

    for (size_t i = 0 ; i < candidates.size() ; ++i)
        for (size_t k = 0 ; k < compares.size() ; ++k)
        {
            if ( (candidates[i] != triangleNum) && (candidates[i] == compares[k]))
                return candidates[i];
        }
    return triangleNum;
}

void TriangleMesh::calcNeighbors()
{
    for( int i = 0 ; i < numTris ; ++i)
    {
        int coVert0 = triangles[i].pointID[0];
        int coVert1 = triangles[i].pointID[1];
        int coVert2 = triangles[i].pointID[2];

        neighbors[i][0] = getNeighbor(coVert0, coVert1, i);
        neighbors[i][1] = getNeighbor(coVert1, coVert2, i);
        neighbors[i][2] = getNeighbor(coVert2, coVert0, i);
    }
    m_neighborsCalculated = true;
}

void TriangleMesh::calcNeighbor(const int triangleNum)
{
    int coVert0 = triangles[triangleNum].pointID[0];
    int coVert1 = triangles[triangleNum].pointID[1];
    int coVert2 = triangles[triangleNum].pointID[2];

    neighbors[triangleNum][0] = getNeighbor(coVert0, coVert1, triangleNum);
    neighbors[triangleNum][1] = getNeighbor(coVert1, coVert2, triangleNum);
    neighbors[triangleNum][2] = getNeighbor(coVert2, coVert0, triangleNum);
}

void TriangleMesh::getEdgeNeighbor( const FIndex& triNum, int pos, std::vector< FIndex >& neigh )
{
    if ( !m_neighborsCalculated )
        calcNeighbors();
    neigh.clear();
    neigh.push_back(FIndex((int)neighbors[triNum][pos]));
}

void TriangleMesh::getNeighbors( const FIndex& vertId, std::vector< FIndex >& neighs )
{
    if ( !m_neighborsCalculated )
        calcNeighbors();
    neighs.clear();
    std::vector<unsigned int>neighbors = vIsInTriangle[vertId];
    for (size_t i = 0 ; i < neighbors.size() ; ++i)
    {
        neighs.push_back(FIndex(neighbors[i]));
    }
}

int TriangleMesh::getTriangleTensor(const int triNum)
{
    if (!m_triangleTensorsCalculated)
        calcTriangleTensors();
    return triangleTensor[triNum];
}

void TriangleMesh::calcTriangleTensors()
{
    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();
    
    triangleTensor.resize(numTris);
    for_each( triangleTensor.begin(), triangleTensor.end(), TriangleTensorCalculator( this, columns, rows, frames ) );
    m_triangleTensorsCalculated = true;
}

Vector TriangleMesh::getTriangleCenter(const int triNum)
{
    Vector v0 = vertices[triangles[triNum].pointID[0]];
    Vector v1 = vertices[triangles[triNum].pointID[1]];
    Vector v2 = vertices[triangles[triNum].pointID[2]];
    Vector p = ( v0 + v1 + v2 )/3.0;
    return p;
}

bool TriangleMesh::hasEdge(const unsigned int coVert1, const unsigned int coVert2, const unsigned int triangleNum){

    return(isInTriangle(coVert1,triangleNum) && isInTriangle(coVert2,triangleNum));

}

bool TriangleMesh::isInTriangle(const unsigned int vertNum, const unsigned int triangleNum)
{
    return ( (vertNum == triangles[triangleNum].pointID[0]) ||
             (vertNum == triangles[triangleNum].pointID[1]) ||
             (vertNum == triangles[triangleNum].pointID[2]) );
}

int TriangleMesh::getThirdVert(const unsigned int coVert1, const unsigned int coVert2, const unsigned int triangleNum)
{
    int index = 0;
    bool found = false;

    while((index < 2) && !found){
        if((triangles[triangleNum].pointID[index] == coVert1) || (triangles[triangleNum].pointID[index] == coVert2)){
            index++;
        } else {
            found = true;
        }
    }

    return triangles[triangleNum].pointID[index];
}


void TriangleMesh::setTriangle(const unsigned int triNum, const unsigned int vertA, const unsigned int vertB, const unsigned int vertC)
{
    eraseTriFromVert(triNum, triangles[triNum].pointID[1]);
    eraseTriFromVert(triNum, triangles[triNum].pointID[2]);

    triangles[triNum].pointID[0] = vertA;
    triangles[triNum].pointID[1] = vertB;
    triangles[triNum].pointID[2] = vertC;

    vIsInTriangle[vertB].push_back(triNum);
    vIsInTriangle[vertC].push_back(triNum);

    triNormals[triNum] = calcTriangleNormal(triNum);
}

void TriangleMesh::eraseTriFromVert(const unsigned int triNum, const unsigned int vertNum)
{
    std::vector<unsigned int>temp;
    for ( size_t i = 0 ; i < vIsInTriangle[vertNum].size() ; ++i)
    {
        if ( triNum != vIsInTriangle[vertNum][i])
            temp.push_back(vIsInTriangle[vertNum][i]);
    }
    vIsInTriangle[vertNum] = temp;
}

int TriangleMesh::getNextVertex(const unsigned int triNum, const unsigned int vertNum)
{
    int answer = -1;

    if(triangles[triNum].pointID[0] == vertNum){
        answer = triangles[triNum].pointID[1];
    } else if(triangles[triNum].pointID[1] == vertNum){
        answer = triangles[triNum].pointID[2];
    } else {
        answer = triangles[triNum].pointID[0];
    }

    return answer;
}

void TriangleMesh::cleanUp()
{
    if ( isCleaned ) return;

    if ( !m_neighborsCalculated )
        calcNeighbors();

    std::vector<int> queue;
    std::vector<bool> visited(numTris, false);
    std::vector< std::vector<int> >objects;
    queue.push_back(0);
    std::vector<int>n;

    while (!queue.empty())
    {
        std::vector<int>newObject;
        newObject.clear();
        while (!queue.empty())
        {
            int index = queue.back();
            visited[index] = true;
            queue.pop_back();
            newObject.push_back(index);
            n = neighbors[index];
            for ( int i = 0 ; i < 3 ; ++i)
            {
                if ( (n[i] != -1) && !visited[n[i]])
                    queue.push_back(n[i]);
            }
        }

        for (int i = 0 ; i < numTris ; ++i)
        {
            if (!visited[i])
            {
                queue.push_back(i);
                break;
            }
        }
        objects.push_back(newObject);
    }
    if (objects.size() == 1) return;
    size_t biggest = 0;
    size_t sizeBiggest = objects[0].size();
    for ( size_t i = 0 ; i < objects.size() ; ++i)
    {
        if (sizeBiggest < objects[i].size() )
        {
            biggest = i;
            sizeBiggest = objects[i].size();
        }
    }

    std::vector<int>obj = objects[biggest];
    std::vector<Triangle>tempTriangles;
    for (size_t i = 0 ; i < obj.size() ; ++i)
    {
        tempTriangles.push_back(triangles[obj[i]]);
    }

    vertNormals.clear();
    triangles.clear();
    triNormals.clear();
    vIsInTriangle.clear();
    neighbors.clear();
    numTris = 0;

    std::vector<unsigned int> v;
    for (int i = 0 ; i < numVerts ; ++i)
    {
        vIsInTriangle.push_back( v );
    }

    for (size_t i = 0 ; i < tempTriangles.size() ; ++i)
    {
        Triangle t = tempTriangles[i];
        addTriangle(t.pointID[0], t.pointID[1], t.pointID[2]);
    }
    calcNeighbors();
    calcVertNormals();
    calcTriangleTensors();
    isCleaned = true;
}

void TriangleMesh::doLoopSubD()
{
    if ( !m_neighborsCalculated )
        calcNeighbors();

    if ( !m_triangleTensorsCalculated )
        calcTriangleTensors();

    loopSubD loop(this);
    m_neighborsCalculated = false;
    m_vertNormalsCalculated = false;
    m_triangleTensorsCalculated = false;

    calcVertNormals();
    calcNeighbors();
    calcTriangleTensors();
}


void TriangleMesh::getCellVerticesIndices( const FIndex& triNum, std::vector< FIndex >& vertices )
{
    //assert( triNum < numTris );
    vertices.clear();

    vertices.push_back( FIndex(triangles[triNum].pointID[0]) );
    vertices.push_back( FIndex(triangles[triNum].pointID[1]) );
    vertices.push_back( FIndex(triangles[triNum].pointID[2]) );
}

void TriangleMesh::getPosition( FPosition& resultPos, const FIndex& pIndex )
{
    positive ind = pIndex.getIndex();
    //assert( ind < numVerts );
    resultPos.resize(3);
    resultPos[0] = vertices[ind].x;
    resultPos[1] = vertices[ind].y;
    resultPos[2] = vertices[ind].z;
}

void TriangleMesh::flipNormals()
{
    for(int i = 0; i < getNumVertices(); ++i)
    {
        vertNormals[i] = -1 * vertNormals[i];
    }
}

void TriangleMesh::printInfo()
{
    int bytes = 0;
    wxDateTime dt1 = wxDateTime::Now();
    printf("[%02d:%02d:%02d] ", dt1.GetHour(), dt1.GetMinute(), dt1.GetSecond());
    printf("Triangle Mesh contains %d Vertices and %d Triangles.\n", numVerts, numTris);
    bytes += ( 6 * sizeof(float) * numVerts ) + ( 12 * sizeof(float) * numTris );
        for ( size_t i = 0 ; i < vIsInTriangle.size() ; ++i)
    {
        bytes += (int)(vIsInTriangle[i].size()) * sizeof(int);
    }

    for ( size_t i = 0 ; i < neighbors.size() ; ++i)
    {
        bytes += (int)(neighbors[i].size()) * sizeof(int);
    }
    printf("[%02d:%02d:%02d] ", dt1.GetHour(), dt1.GetMinute(), dt1.GetSecond());
    printf("Triangle Mesh uses %d bytes.\n", bytes);

#if defined(DEBUG) || defined(_DEBUG)
    printf("vertices: %d\n",(int)vertices.size());
    printf("vertNormals: %d\n",(int)vertNormals.size());
    printf("triangles: %d\n",(int)triangles.size());
    printf("triangleTensor: %d\n",(int)triangleTensor.size());
    printf("triangleColor: %d\n",(int)triangleColor.size());
    printf("neighbors: %d\n",(int)neighbors.size());
    printf("triNormals: %d\n",(int)triNormals.size());
    printf("vIsInTriangle: %d\n",(int)vIsInTriangle.size());

    printf("vertNormalsCalculated = ");
    (m_vertNormalsCalculated) ? printf("true\n") : printf("false\n");
    printf("neighborsCalculated = ");
    (m_neighborsCalculated) ? printf("true\n") : printf("false\n");
    printf("triangleTensorsCalculated = ");
    (m_triangleTensorsCalculated) ? printf("true\n") : printf("false\n");
#endif
}

int TriangleMesh::getNumVertices()
{
    return numVerts;
}

int TriangleMesh::getNumTriangles()
{
    return numTris;
}

Vector TriangleMesh::getVertex(const int vertNum)
{
    return vertices[vertNum];
}

Vector TriangleMesh::getNormal(const int triNum)
{
    return triNormals[triNum];
}

Triangle TriangleMesh::getTriangle(const int triNum)
{
    return triangles[triNum];
}

wxColour TriangleMesh::getTriangleColor(const int triNum)
{
    return triangleColor[triNum];
}

std::vector<unsigned int> TriangleMesh::getStar(const int vertNum)
{
    return vIsInTriangle[vertNum];
}

std::vector<Vector> TriangleMesh::getVerts()
{
    return vertices;
}

void TriangleMesh::setVertex(const unsigned int vertNum, const Vector nPos)
{
    vertices[vertNum] = nPos;
}
