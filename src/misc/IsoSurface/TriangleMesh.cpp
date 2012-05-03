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
TriangleMesh::TriangleMesh()
:   m_numVerts( 0 ),
    m_numTris( 0 ),
    m_isCleaned( false ),
    m_vertNormalsCalculated( false ),
    m_neighborsCalculated( false ),
    m_triangleTensorsCalculated( false ),
    m_defaultColor( 200, 200, 200, 255 )
{
}

TriangleMesh::~TriangleMesh ()
{
    clearMesh();
}

void TriangleMesh::clearMesh()
{
    m_vertices.clear();
    m_vertNormals.clear();
    m_vertColors.clear();
    m_vIsInTriangle.clear();

    m_triangles.clear();
    m_triangleTensor.clear();
    m_triangleColor.clear();
    m_neighbors.clear();
    m_triNormals.clear();

    m_numVerts     = 0;
    m_numTris      = 0;

    m_isCleaned = false;
    m_vertNormalsCalculated = false;
    m_neighborsCalculated = false;
    m_triangleTensorsCalculated = false;
}


void TriangleMesh::addVert(const Vector newVert)
{
    m_vertices.push_back( newVert );
    m_numVerts = m_vertices.size();
    m_vIsInTriangle.resize(m_numVerts);
    m_vertColors.resize(m_numVerts);
}

void TriangleMesh::fastAddVert(const Vector newVert)
{
    m_vertices[m_numVerts] = newVert ;
    ++m_numVerts;
}

void TriangleMesh::addVert(const float x, const float y, const float z)
{
    addVert(Vector(x,y,z));
}

void TriangleMesh::reserveVerts(const int size)
{
    m_vertices.reserve(size);
    m_vertNormals.reserve(size);
}

void TriangleMesh::resizeVerts(const int size)
{
    m_vertices.resize(size);
    m_vIsInTriangle.resize(size);
}

void TriangleMesh::addTriangle(const int vertA, const int vertB, const int vertC)
{
    addTriangle(vertA, vertB, vertC, 0);
}

void TriangleMesh::addTriangle(const int vertA, const int vertB, const int vertC, const int tensorIndex)
{
    Triangle t = {{vertA, vertB, vertC}};
    m_triangles.push_back(t);
    m_triNormals.push_back(calcTriangleNormal(t));
    m_vIsInTriangle[vertA].push_back(m_numTris);
    m_vIsInTriangle[vertB].push_back(m_numTris);
    m_vIsInTriangle[vertC].push_back(m_numTris);
    std::vector<int> v(3,-1);
    m_neighbors.push_back( v );
    m_numTris = m_triangles.size();
    m_triangleTensor.push_back(tensorIndex);
    m_triangleColor.push_back(m_defaultColor);
}

void TriangleMesh::fastAddTriangle(const int vertA, const int vertB, const int vertC)
{
    Triangle t = {{vertA, vertB, vertC}};
    m_triangles[m_numTris] = t;
    m_triNormals[m_numTris] = calcTriangleNormal(t);
    m_vIsInTriangle[vertA].push_back(m_numTris);
    m_vIsInTriangle[vertB].push_back(m_numTris);
    m_vIsInTriangle[vertC].push_back(m_numTris);
    ++m_numTris;
}

void TriangleMesh::reserveTriangles(const int size)
{
    m_triangles.reserve(size);
    m_triNormals.reserve(size);
    m_triangleTensor.reserve(size);
    m_triangleColor.reserve(size);
    m_neighbors.reserve(size);
}

void TriangleMesh::resizeTriangles(const int size)
{
    m_triangles.resize(size);
    m_triNormals.resize(size);
    m_triangleTensor.resize(size,0);
    m_triangleColor.resize(size, m_defaultColor);
    std::vector<int> v(3,-1);
    m_neighbors.resize(size, v);
}

void TriangleMesh::setTriangleColor(const unsigned int triNum, const float r, const float g, const float b, const float a)
{
    unsigned char red = (unsigned char)(r * 255);
    unsigned char green = (unsigned char)(g * 255);
    unsigned char blue= (unsigned char)(b * 255);
    unsigned char alpha = (unsigned char)(a * 255);

    m_triangleColor[triNum].Set(red, green, blue, alpha);
}

void TriangleMesh::setTriangleColor(const unsigned int triNum, const float r, const float g, const float b)
{
    unsigned char red = (unsigned char)(r * 255);
    unsigned char green = (unsigned char)(g * 255);
    unsigned char blue= (unsigned char)(b * 255);
    m_triangleColor[triNum].Set(red, green, blue, m_triangleColor[triNum].Alpha());
}

void TriangleMesh::setTriangleAlpha(const unsigned int triNum, const float a)
{
    unsigned char alpha = (unsigned char)(a * 255);
    m_triangleColor[triNum].Set(m_triangleColor[triNum].Red(), m_triangleColor[triNum].Green(), m_triangleColor[triNum].Blue(), alpha);
}

void TriangleMesh::setTriangleRed(const unsigned int triNum, const float r)
{
    unsigned char red = (unsigned char)(r * 255);
    m_triangleColor[triNum].Set(red, m_triangleColor[triNum].Green(), m_triangleColor[triNum].Blue(), m_triangleColor[triNum].Alpha());
}

void TriangleMesh::setTriangleGreen(const unsigned int triNum, const float g)
{
    unsigned char green = (unsigned char)(g * 255);
    m_triangleColor[triNum].Set(m_triangleColor[triNum].Red(), green, m_triangleColor[triNum].Blue(), m_triangleColor[triNum].Alpha());
}

void TriangleMesh::setTriangleBlue(const unsigned int triNum, const float b)
{
    unsigned char blue = (unsigned char)(b * 255);
    m_triangleColor[triNum].Set(m_triangleColor[triNum].Red(), m_triangleColor[triNum].Green(), blue, m_triangleColor[triNum].Alpha());
}

Vector TriangleMesh::calcTriangleNormal(const Triangle t)
{
    Vector v1 = m_vertices[t.pointID[1]] - m_vertices[t.pointID[0]];
    Vector v2 = m_vertices[t.pointID[2]] - m_vertices[t.pointID[0]];

    Vector tempNormal = v1.Cross(v2);
    tempNormal.normalize();
    return tempNormal;
}

Vector TriangleMesh::calcTriangleNormal(const int triNum)
{

    Vector v1 = m_vertices[m_triangles[triNum].pointID[1]] - m_vertices[m_triangles[triNum].pointID[0]];
    Vector v2 = m_vertices[m_triangles[triNum].pointID[2]] - m_vertices[m_triangles[triNum].pointID[0]];

    Vector tempNormal = v1.Cross(v2);
    tempNormal.normalize();
    return tempNormal;
}

Vector TriangleMesh::getVertex (const int triNum, int pos)
{
    if (pos < 0 || pos > 2) pos = 0;
    return m_vertices[m_triangles[triNum].pointID[pos]];
}

Vector TriangleMesh::getVertNormal(const int vertNum)
{
    if ( !m_vertNormalsCalculated )
        calcVertNormals();
    return m_vertNormals[vertNum];
}

Vector TriangleMesh::calcVertNormal(const int vertNum)
{
    Vector sum(0,0,0);

    for(size_t i = 0 ; i < m_vIsInTriangle[vertNum].size() ; ++i)
    {
        sum = sum + m_triNormals[m_vIsInTriangle[vertNum][i]];
    }
    sum.normalize();
    return sum;
}

void TriangleMesh::setVertColor( const int vertNum, const wxColour color)
{
    m_vertColors[vertNum] = color;
}

wxColour TriangleMesh::getVertColor(const int vertNum)
{
    return m_vertColors[vertNum];
}

void TriangleMesh::calcVertNormals()
{
    m_vertNormals.clear();
    m_vertNormals.resize(m_numVerts);
    for ( int i = 0 ; i < m_numVerts ; ++i)
    {
        m_vertNormals[i] = calcVertNormal(i);
    }

    m_vertNormalsCalculated = true;
}

int TriangleMesh::getNeighbor(const unsigned int coVert1, const unsigned int coVert2, const unsigned int triangleNum)
{
    std::vector<unsigned int>candidates = m_vIsInTriangle[coVert1];
    std::vector<unsigned int>compares   = m_vIsInTriangle[coVert2];

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
    for( int i = 0 ; i < m_numTris ; ++i)
    {
        int coVert0 = m_triangles[i].pointID[0];
        int coVert1 = m_triangles[i].pointID[1];
        int coVert2 = m_triangles[i].pointID[2];

        m_neighbors[i][0] = getNeighbor(coVert0, coVert1, i);
        m_neighbors[i][1] = getNeighbor(coVert1, coVert2, i);
        m_neighbors[i][2] = getNeighbor(coVert2, coVert0, i);
    }
    m_neighborsCalculated = true;
}

void TriangleMesh::calcNeighbor(const int triangleNum)
{
    int coVert0 = m_triangles[triangleNum].pointID[0];
    int coVert1 = m_triangles[triangleNum].pointID[1];
    int coVert2 = m_triangles[triangleNum].pointID[2];

    m_neighbors[triangleNum][0] = getNeighbor(coVert0, coVert1, triangleNum);
    m_neighbors[triangleNum][1] = getNeighbor(coVert1, coVert2, triangleNum);
    m_neighbors[triangleNum][2] = getNeighbor(coVert2, coVert0, triangleNum);
}

void TriangleMesh::getEdgeNeighbor( const FIndex& triNum, int pos, std::vector< FIndex >& neigh )
{
    if ( !m_neighborsCalculated )
        calcNeighbors();
    neigh.clear();
    neigh.push_back(FIndex((int)m_neighbors[triNum][pos]));
}

void TriangleMesh::getNeighbors( const FIndex& vertId, std::vector< FIndex >& neighs )
{
    if ( !m_neighborsCalculated )
        calcNeighbors();
    neighs.clear();
    std::vector<unsigned int>neighbors = m_vIsInTriangle[vertId];
    for (size_t i = 0 ; i < neighbors.size() ; ++i)
    {
        neighs.push_back(FIndex(neighbors[i]));
    }
}

int TriangleMesh::getTriangleTensor(const int triNum)
{
    if (!m_triangleTensorsCalculated)
        calcTriangleTensors();
    return m_triangleTensor[triNum];
}

void TriangleMesh::calcTriangleTensors()
{
    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();
    
    m_triangleTensor.resize(m_numTris);
    for_each( m_triangleTensor.begin(), m_triangleTensor.end(), TriangleTensorCalculator( this, columns, rows, frames ) );
    m_triangleTensorsCalculated = true;
}

Vector TriangleMesh::getTriangleCenter(const int triNum)
{
    Vector v0 = m_vertices[m_triangles[triNum].pointID[0]];
    Vector v1 = m_vertices[m_triangles[triNum].pointID[1]];
    Vector v2 = m_vertices[m_triangles[triNum].pointID[2]];
    Vector p = ( v0 + v1 + v2 )/3.0;
    return p;
}

bool TriangleMesh::hasEdge(const unsigned int coVert1, const unsigned int coVert2, const unsigned int triangleNum){

    return(isInTriangle(coVert1,triangleNum) && isInTriangle(coVert2,triangleNum));

}

bool TriangleMesh::isInTriangle(const unsigned int vertNum, const unsigned int triangleNum)
{
    return ( (vertNum == m_triangles[triangleNum].pointID[0]) ||
             (vertNum == m_triangles[triangleNum].pointID[1]) ||
             (vertNum == m_triangles[triangleNum].pointID[2]) );
}

int TriangleMesh::getThirdVert(const unsigned int coVert1, const unsigned int coVert2, const unsigned int triangleNum)
{
    int index = 0;
    bool found = false;

    while((index < 2) && !found){
        if((m_triangles[triangleNum].pointID[index] == coVert1) || (m_triangles[triangleNum].pointID[index] == coVert2)){
            index++;
        } else {
            found = true;
        }
    }

    return m_triangles[triangleNum].pointID[index];
}


void TriangleMesh::setTriangle(const unsigned int triNum, const unsigned int vertA, const unsigned int vertB, const unsigned int vertC)
{
    eraseTriFromVert(triNum, m_triangles[triNum].pointID[1]);
    eraseTriFromVert(triNum, m_triangles[triNum].pointID[2]);

    m_triangles[triNum].pointID[0] = vertA;
    m_triangles[triNum].pointID[1] = vertB;
    m_triangles[triNum].pointID[2] = vertC;

    m_vIsInTriangle[vertB].push_back(triNum);
    m_vIsInTriangle[vertC].push_back(triNum);

    m_triNormals[triNum] = calcTriangleNormal(triNum);
}

void TriangleMesh::eraseTriFromVert(const unsigned int triNum, const unsigned int vertNum)
{
    std::vector<unsigned int>temp;
    for ( size_t i = 0 ; i < m_vIsInTriangle[vertNum].size() ; ++i)
    {
        if ( triNum != m_vIsInTriangle[vertNum][i])
            temp.push_back(m_vIsInTriangle[vertNum][i]);
    }
    m_vIsInTriangle[vertNum] = temp;
}

int TriangleMesh::getNextVertex(const unsigned int triNum, const unsigned int vertNum)
{
    int answer = -1;

    if(m_triangles[triNum].pointID[0] == vertNum){
        answer = m_triangles[triNum].pointID[1];
    } else if(m_triangles[triNum].pointID[1] == vertNum){
        answer = m_triangles[triNum].pointID[2];
    } else {
        answer = m_triangles[triNum].pointID[0];
    }

    return answer;
}

void TriangleMesh::cleanUp()
{
    if ( m_isCleaned ) return;

    if ( !m_neighborsCalculated )
        calcNeighbors();

    std::vector<int> queue;
    std::vector<bool> visited(m_numTris, false);
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
            n = m_neighbors[index];
            for ( int i = 0 ; i < 3 ; ++i)
            {
                if ( (n[i] != -1) && !visited[n[i]])
                    queue.push_back(n[i]);
            }
        }

        for (int i = 0 ; i < m_numTris ; ++i)
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
        tempTriangles.push_back(m_triangles[obj[i]]);
    }

    m_vertNormals.clear();
    m_triangles.clear();
    m_triNormals.clear();
    m_vIsInTriangle.clear();
    m_neighbors.clear();
    m_numTris = 0;

    std::vector<unsigned int> v;
    for (int i = 0 ; i < m_numVerts ; ++i)
    {
        m_vIsInTriangle.push_back( v );
    }

    for (size_t i = 0 ; i < tempTriangles.size() ; ++i)
    {
        Triangle t = tempTriangles[i];
        addTriangle(t.pointID[0], t.pointID[1], t.pointID[2]);
    }
    calcNeighbors();
    calcVertNormals();
    calcTriangleTensors();
    m_isCleaned = true;
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
    //assert( triNum < m_numTris );
    vertices.clear();

    vertices.push_back( FIndex(m_triangles[triNum].pointID[0]) );
    vertices.push_back( FIndex(m_triangles[triNum].pointID[1]) );
    vertices.push_back( FIndex(m_triangles[triNum].pointID[2]) );
}

void TriangleMesh::getPosition( FPosition& resultPos, const FIndex& pIndex )
{
    positive ind = pIndex.getIndex();
    //assert( ind < m_numVerts );
    resultPos.resize(3);
    resultPos[0] = m_vertices[ind].x;
    resultPos[1] = m_vertices[ind].y;
    resultPos[2] = m_vertices[ind].z;
}

void TriangleMesh::flipNormals()
{
    for(int i = 0; i < getNumVertices(); ++i)
    {
        m_vertNormals[i] = -1 * m_vertNormals[i];
    }
}

void TriangleMesh::printInfo()
{
    int bytes = 0;
    wxDateTime dt1 = wxDateTime::Now();
    printf("[%02d:%02d:%02d] ", dt1.GetHour(), dt1.GetMinute(), dt1.GetSecond());
    printf("Triangle Mesh contains %d Vertices and %d Triangles.\n", m_numVerts, m_numTris);
    bytes += ( 6 * sizeof(float) * m_numVerts ) + ( 12 * sizeof(float) * m_numTris );
        for ( size_t i = 0 ; i < m_vIsInTriangle.size() ; ++i)
    {
        bytes += (int)(m_vIsInTriangle[i].size()) * sizeof(int);
    }

    for ( size_t i = 0 ; i < m_neighbors.size() ; ++i)
    {
        bytes += (int)(m_neighbors[i].size()) * sizeof(int);
    }
    printf("[%02d:%02d:%02d] ", dt1.GetHour(), dt1.GetMinute(), dt1.GetSecond());
    printf("Triangle Mesh uses %d bytes.\n", bytes);

#if defined(DEBUG) || defined(_DEBUG)
    printf("m_vertices: %d\n",(int)m_vertices.size());
    printf("m_vertNormals: %d\n",(int)m_vertNormals.size());
    printf("m_triangles: %d\n",(int)m_triangles.size());
    printf("m_triangleTensor: %d\n",(int)m_triangleTensor.size());
    printf("m_triangleColor: %d\n",(int)m_triangleColor.size());
    printf("m_neighbors: %d\n",(int)m_neighbors.size());
    printf("m_triNormals: %d\n",(int)m_triNormals.size());
    printf("m_vIsInTriangle: %d\n",(int)m_vIsInTriangle.size());

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
    return m_numVerts;
}

int TriangleMesh::getNumTriangles()
{
    return m_numTris;
}

Vector TriangleMesh::getVertex(const int vertNum)
{
    return m_vertices[vertNum];
}

Vector TriangleMesh::getNormal(const int triNum)
{
    return m_triNormals[triNum];
}

Triangle TriangleMesh::getTriangle(const int triNum)
{
    return m_triangles[triNum];
}

wxColour TriangleMesh::getTriangleColor(const int triNum)
{
    return m_triangleColor[triNum];
}

std::vector<unsigned int> TriangleMesh::getStar(const int vertNum)
{
    return m_vIsInTriangle[vertNum];
}

std::vector<Vector> TriangleMesh::getVerts()
{
    return m_vertices;
}

void TriangleMesh::setVertex(const unsigned int vertNum, const Vector nPos)
{
    m_vertices[vertNum] = nPos;
}
