/*
 *  The Fibers class implementation.
 *
 */

#include "Fibers.h"

#include "Anatomy.h"
#include "DatasetManager.h"

#include "../main.h"
#include "../Logger.h"
#include "../gfx/ShaderHelper.h"
#include "../gui/MainFrame.h"
#include "../gui/SceneManager.h"
#include "../misc/Fantom/FMatrix.h"

#include <wx/file.h>
#include <wx/tokenzr.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string>

// TODO replace by const
#define LINEAR_GRADIENT_THRESHOLD 0.085f
#define MIN_ALPHA_VALUE 0.017f

Fibers::Fibers( DatasetHelper *pDatasetHelper )
    : DatasetInfo( pDatasetHelper ),
      m_isSpecialFiberDisplay( false ),
      m_isInitialized( false ),
      m_normalsPositive( false ),
      m_cachedThreshold( 0.0f ),
	  m_fibersInverted( false ),
	  m_useFakeTubes( false ),
	  m_useTransparency( false ),
	  m_isColorationUpdated( false ),
	  m_fiberColorationMode( NORMAL_COLOR ),
      m_pKdTree( NULL ),
      m_pOctree( NULL ),
      m_cfDrawDirty( true ),
      m_axialShown(    SceneManager::getInstance()->isAxialDisplayed() ),
      m_coronalShown(  SceneManager::getInstance()->isCoronalDisplayed() ),
      m_sagittalShown( SceneManager::getInstance()->isSagittalDisplayed() ),
      m_useCrossingFibers( false ),
      m_thickness( 2.5f )
{
    m_bufferObjects         = new GLuint[3];
}

Fibers::~Fibers()
{
    Logger::getInstance()->print( wxT( "Executing fibers destructor" ), LOGLEVEL_DEBUG );

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glDeleteBuffers( 3, m_bufferObjects );
    }

    if( m_pKdTree )
    {
        delete m_pKdTree;
        m_pKdTree = NULL;
    }

    if( m_pOctree )
    {
        delete m_pOctree;
        m_pOctree = NULL;
    }

    m_lineArray.clear();
    m_linePointers.clear();
    m_reverse.clear();
    m_pointArray.clear();
    m_normalArray.clear();
    m_colorArray.clear();
}

bool Fibers::load( const wxString &filename )
{
    bool res( false );

    wxString extension = filename.AfterLast( '.' );
    if( wxT( "fib" ) == extension )
    {
        if( loadVTK( filename ) )
        {
            res = true;
        }
        else
        {
            res = loadDmri( filename );
        }
    }
    else if( wxT( "bundlesdata" ) == extension )
    {
        res = loadPTK( filename );
    }
    else if( wxT( "Bfloat" ) == extension )
    {
        res = loadCamino( filename );
    }
    else if( wxT( "trk" ) == extension )
    {
        res = loadTRK( filename );
    }
    else if( wxT( "tck" ) == extension )
    {
        res = loadMRtrix( filename );
    }

    /* OcTree points classification */
    m_pOctree = new Octree( 2, m_pointArray, m_countPoints, m_dh );
    return res;
}

bool Fibers::loadTRK( const wxString &filename )
{
    stringstream ss;
    Logger::getInstance()->print( wxT( "Loading TRK file..." ), LOGLEVEL_MESSAGE );
    wxFile dataFile;
    wxFileOffset nSize( 0 );
    converterByteINT16 cbi;
    converterByteINT32 cbi32;
    converterByteFloat cbf;

    if( !dataFile.Open( filename ) )
    {
        return false;
    }

    nSize = dataFile.Length();

    if( nSize == wxInvalidOffset )
    {
        return false;
    }

    ////
    // READ HEADER
    ////
    //Read file header. [1000 bytes]
    wxUint8 *pBuffer = new wxUint8[1000];
    dataFile.Read( pBuffer, ( size_t )1000 );
    
    //ID String for track file. The first 5 characters must match "TRACK". [6 bytes]
    char idString[6];
    memcpy( idString, &pBuffer[0], 6 );
    ss.str( "" );
    ss << "Type: " << idString;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    if( strncmp( idString, "TRACK", 5 ) != 0 ) 
    {
        return false;
    }

    //Dimension of the image volume. [6 bytes]
    wxUint16 dim[3];

    for( int i = 0; i != 3; ++i )
    {
        memcpy( cbi.b, &pBuffer[6 + ( i * 2 )], 2 );
        dim[i] = cbi.i;
    }

    ss.str( "" );
    ss << "Dim: " << dim[0] << "x" << dim[1] << "x" << dim[2];
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    
    //Voxel size of the image volume. [12 bytes]
    float voxelSize[3];

    for( int i = 0; i != 3; ++i )
    {
        memcpy( cbf.b, &pBuffer[12 + ( i * 4 )], 4 );
        voxelSize[i] = cbf.f;
    }

    ss.str( "" );
    ss << "Voxel size: " << voxelSize[0] << "x" << voxelSize[1] << "x" << voxelSize[2];
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Origin of the image volume. [12 bytes]
    float origin[3];

    for( int i = 0; i != 3; ++i )
    {
        memcpy( cbf.b, &pBuffer[24 + ( i * 4 )], 4 );
        origin[i] = cbf.f;
    }

    ss.str( "" );
    ss << "Origin: (" << origin[0] << "," << origin[1] << "," << origin[2] << ")";
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    
    //Number of scalars saved at each track point. [2 bytes]
    wxUint16 nbScalars;
    memcpy( cbi.b, &pBuffer[36], 2 );
    nbScalars = cbi.i;
    ss.str( "" );
    ss << "Nb. scalars: " << nbScalars;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    
    //Name of each scalar. (20 characters max each, max 10 names) [200 bytes]
    char scalarNames[10][20];
    memcpy( scalarNames, &pBuffer[38], 200 );

    for( int i = 0; i != 10; ++i )
    {
        ss.str( "" );
        ss << "Scalar name #" << i << ": " << scalarNames[i];
        Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    }

    //Number of properties saved at each track. [2 bytes]
    wxUint16 nbProperties;
    memcpy( cbi.b, &pBuffer[238], 2 );
    nbProperties = cbi.i;
    ss.str( "" );
    ss << "Nb. properties: " << nbProperties;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    
    //Name of each property. (20 characters max each, max 10 names) [200 bytes]
    char propertyNames[10][20];
    memcpy( propertyNames, &pBuffer[240], 200 );

    for( int i = 0; i != 10; ++i )
    {
        ss.str( "" );
        ss << "Property name #" << i << ": " << propertyNames[i];
    }

    //4x4 matrix for voxel to RAS (crs to xyz) transformation.
    // If vox_to_ras[3][3] is 0, it means the matrix is not recorded.
    // This field is added from version 2. [64 bytes]
    float voxToRas[4][4];

    for( int i = 0; i != 4; ++i )
    {
        ss.str( "" );

        for( int j = 0; j != 4; ++j )
        {
            memcpy( cbf.b, &pBuffer[440 + ( i * 4 + j )], 4 );
            voxToRas[i][j] = cbf.f;
            ss << voxToRas[i][j] << " ";
        }

        Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    }

    //Reserved space for future version. [444 bytes]
    //char reserved[444];
    //pBuffer[504]...
    //Storing order of the original image data. [4 bytes]
    char voxelOrder[4];
    memcpy( voxelOrder, &pBuffer[948], 4 );
    ss.str( "" );
    ss << "Voxel order: " << voxelOrder;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    
    //Paddings [4 bytes]
    char pad2[4];
    memcpy( pad2, &pBuffer[952], 4 );
    ss.str( "" );
    ss << "Pad #2: " << pad2;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    
    //Image orientation of the original image. As defined in the DICOM header. [24 bytes]
    float imageOrientationPatient[6];
    ss.str( "" );
    ss << "Image orientation patient: ";

    for( int i = 0; i != 6; ++i )
    {
        memcpy( cbf.b, &pBuffer[956 + ( i * 4 )], 4 );
        imageOrientationPatient[i] = cbf.f;
        ss << imageOrientationPatient[i] << " ";
    }

    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    
    //Paddings. [2 bytes]
    char pad1[2];
    memcpy( pad1, &pBuffer[980], 2 );
    ss.str( "" );
    ss << "Pad #1: " << pad1;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    
    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool invertX = pBuffer[982] > 0;
    ss.str( "" );
    ss << "Invert X: " << invertX;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool invertY = pBuffer[983] > 0;
    ss.str( "" );
    ss << "Invert Y: " << invertY;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool invertZ = pBuffer[984] > 0;
    ss.str( "" );
    ss << "Invert Z: " << invertZ;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool swapXY = pBuffer[985] > 0;
    ss.str( "" );
    ss << "Swap XY: " << swapXY;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool swapYZ = pBuffer[986] > 0;
    ss.str( "" );
    ss << "Swap YZ: " << swapYZ;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Inversion/rotation flags used to generate this track file. [1 byte]
    bool swapZX = pBuffer[987] > 0;
    ss.str( "" );
    ss << "Swap ZX: " << swapZX;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Number of tracks stored in this track file. 0 means the number was NOT stored. [4 bytes]
    wxUint32 nbCount;
    memcpy( cbi32.b, &pBuffer[988], 4 );
    nbCount = cbi32.i;
    ss.str( "" );
    ss << "Nb. tracks: " << nbCount;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    //Version number. Current version is 2. [4 bytes]
    wxUint32 version;
    memcpy( cbi32.b, &pBuffer[992], 4 );
    version = cbi32.i;
    ss.str( "" );
    ss << "Version: " << version;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    
    //Size of the header. Used to determine byte swap. Should be 1000. [4 bytes]
    wxUint32 hdrSize;
    memcpy( cbi32.b, &pBuffer[996], 4 );
    hdrSize = cbi32.i;
    ss.str( "" );
    ss << "HDR size: " << hdrSize;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );

    ////
    // READ DATA
    ////
    delete[] pBuffer;
    pBuffer = NULL;
    vector<float> tmpPoints;

    //File size - header size (used when the number of fiber was not stored)
    int remainingBytes = nSize - hdrSize;

    vector< vector< float > > lines;
    m_countPoints = 0;
    vector< float > colors;

	 //if( nbCount == 0 )
    //{
    //   return false; //TODO: handle it. (0 means the number was NOT stored.)
    //}

    for( unsigned int i = 0; i < nbCount || remainingBytes > 0; ++i )
    //for( unsigned int i = 0; i != nbCount ; ++i )
	 {
        //Number of points in this track. [4 bytes]
        wxUint32 nbPoints;
        dataFile.Read( cbi32.b, ( size_t )4 );
        nbPoints = cbi32.i;
        
        //Read data of one track.
        size_t ptsSize = 3 + nbScalars;
        size_t tractSize = 4 * ( nbPoints * ( ptsSize ) + nbProperties );
        pBuffer = new wxUint8[tractSize];
        dataFile.Read( pBuffer, tractSize );
        vector< float > curLine;

        for( unsigned int j = 0; j != nbPoints; ++j )
        {
            //Read coordinates (x,y,z) and scalars associated to each point.
            for( unsigned int k = 0; k != ptsSize; ++k )
            {
                memcpy( cbf.b, &pBuffer[4 * ( j * ptsSize + k )], 4 );

                if( k >= 6 ) //TODO: incorporate other scalars in the navigator.
                {
                    break;
                }
                else if( k >= 3 ) //RGB color of each point.
                {
                    colors.push_back( cbf.f );
                }
                else
                {
                    curLine.push_back( cbf.f );
                }
            }
        }

        //TODO: incorporate properties in the navigator.
//         for( unsigned int j = 0; j != nbProperties; ++j ) 
//         {}

        m_countPoints += curLine.size() / 3;
        lines.push_back( curLine );
        delete[] pBuffer;
        pBuffer = NULL;

       //Adjust remainingBytes
       remainingBytes -= 1*4; //Number of points (4 bytes)
       remainingBytes -= 3*nbPoints * 4; //Coordinates (4 bytes)
       remainingBytes -= nbScalars*nbPoints * 4; //Scalars (4 bytes)
       remainingBytes -= nbProperties * 4; //Properties (4 bytes)
		 //cout <<  "i : " << i << ", remainingBytes : " << remainingBytes << "\n";
    }

    dataFile.Close();
    
    ////
    //POST PROCESS: set all the data in the right format for the navigator
    ////
    Logger::getInstance()->print( wxT( "Setting data in right format for the navigator..." ), LOGLEVEL_MESSAGE );
    m_countLines = lines.size();
    DatasetManager::getInstance()->setCountFibers( m_countLines );
    m_pointArray.max_size();
    m_colorArray.max_size();
    m_linePointers.resize( m_countLines + 1 );
    m_pointArray.resize( m_countPoints * 3 );
    m_colorArray.resize( m_countPoints * 3 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );
    ss.str( "" );
    ss << "m_countLines: " << m_countLines;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    ss.str( "" );
    ss << "m_countPoints: " << m_countPoints;
    Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
    m_linePointers[0] = 0;

    for( int i = 0; i < m_countLines; ++i )
    {
        m_linePointers[i + 1] = m_linePointers[i] + lines[i].size() / 3;
    }

    int lineCounter = 0;

    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[lineCounter + 1] )
        {
            ++lineCounter;
        }

        m_reverse[i] = lineCounter;
    }

    unsigned int pos( 0 );
    vector< vector< float > >::iterator it;

    for( it = lines.begin(); it < lines.end(); it++ )
    {
        vector< float >::iterator it2;

        for( it2 = ( *it ).begin(); it2 < ( *it ).end(); it2++ )
        {   
				if (colors.size() > 0)
				{
	            m_colorArray[pos] = colors[pos] / 255.;
				}
            m_pointArray[pos++] = *it2;
        }
    }

    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    if( voxelSize[0] == 0 && voxelSize[1] == 0 && voxelSize[2] == 0 )
    {
        ss.str( "" );
        ss << "Using anatomy's voxel size: [" << voxelX << ", " << voxelY << ", " << voxelZ << "]";
        Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
        voxelSize[0] = voxelX;
        voxelSize[1] = voxelY;
        voxelSize[2] = voxelZ;
        ss.str( "" );
        ss << "Centering with respect to the anatomy: [" << columns / 2 << "," << rows / 2 << "," << frames / 2 << "]";
        Logger::getInstance()->print( wxString( ss.str().c_str(), wxConvUTF8 ), LOGLEVEL_MESSAGE );
        origin[0] = columns / 2;
        origin[1] = rows / 2;
        origin[2] = frames / 2;
    }

    float flipX = ( invertX ) ? -1. : 1.;
    float flipY = ( invertY ) ? -1. : 1.;
    float flipZ = ( invertZ ) ? -1. : 1.;
    float anatomy[3];
    anatomy[0] = ( flipX - 1. ) * columns * voxelX / -2.;
    anatomy[1] = ( flipY - 1. ) * rows    * voxelY / -2.;
    anatomy[2] = ( flipZ - 1. ) * frames  * voxelZ / -2.;

    for( int i = 0; i < m_countPoints * 3; ++i )
    {
        m_pointArray[i] = flipX * ( m_pointArray[i] - origin[0] ) * voxelX / voxelSize[0] + anatomy[0];
        ++i;
        m_pointArray[i] = flipY * ( m_pointArray[i] - origin[1] ) * voxelY / voxelSize[1] + anatomy[1];
        ++i;
        m_pointArray[i] = flipZ * ( m_pointArray[i] - origin[2] ) * voxelZ / voxelSize[2] + anatomy[2];
    }

    Logger::getInstance()->print( wxT( "TRK file loaded" ), LOGLEVEL_MESSAGE );
    createColorArray( colors.size() > 0 );
    m_type = FIBERS;
    m_fullPath = filename;
    //m_pKdTree = new KdTree( m_countPoints, &m_pointArray[0], m_dh );
#ifdef __WXMSW__
    m_name = wxT( "-" ) + filename.AfterLast( '\\' );
#else
    m_name = wxT( "-" ) + filename.AfterLast( '/' );
#endif
    return true;
}

bool Fibers::loadCamino( const wxString &filename )
{
    Logger::getInstance()->print( wxT( "Loading Camino file" ), LOGLEVEL_MESSAGE );
    wxFile dataFile;
    wxFileOffset nSize = 0;

    if( dataFile.Open( filename ) )
    {
        nSize = dataFile.Length();

        if( nSize == wxInvalidOffset )
        {
            return false;
        }
    }

    wxUint8 *pBuffer = new wxUint8[nSize];
    dataFile.Read( pBuffer, nSize );
    dataFile.Close();
    m_countLines  = 0; // Number of lines.
    m_countPoints = 0; // Number of points.
    int cl = 0;
    int pc = 0;
    converterByteFloat cbf;
    vector< float > tmpPoints;

    while( pc < nSize )
    {
        ++m_countLines;
        cbf.b[3] = pBuffer[pc++];
        cbf.b[2] = pBuffer[pc++];
        cbf.b[1] = pBuffer[pc++];
        cbf.b[0] = pBuffer[pc++];
        cl = ( int )cbf.f;
        m_lineArray.push_back( cl );
        pc += 4;

        for( int i = 0; i < cl; ++i )
        {
            m_lineArray.push_back( m_countPoints );
            ++m_countPoints;
            cbf.b[3] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[0] = pBuffer[pc++];
            tmpPoints.push_back( cbf.f );
            cbf.b[3] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[0] = pBuffer[pc++];
            tmpPoints.push_back( cbf.f );
            cbf.b[3] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[0] = pBuffer[pc++];
            tmpPoints.push_back( cbf.f );

            if( pc > nSize )
            {
                break;
            }
        }
    }

    m_linePointers.resize( m_countLines + 1 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );
    m_pointArray.resize( tmpPoints.size() );

    for( size_t i = 0; i < tmpPoints.size(); ++i )
    {
        m_pointArray[i] = tmpPoints[i];
    }

    printf( "%d lines and %d points \n", m_countLines, m_countPoints );
    Logger::getInstance()->print( wxT( "Move vertices" ), LOGLEVEL_MESSAGE );

    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    for( int i = 0; i < m_countPoints * 3; ++i )
    {
        m_pointArray[i] = columns * voxelX - m_pointArray[i];
        ++i;
        m_pointArray[i] = rows    * voxelY - m_pointArray[i];
        ++i;
        m_pointArray[i] = frames  * voxelZ - m_pointArray[i];
    }

    calculateLinePointers();
    createColorArray( false );
    Logger::getInstance()->print( wxT( "Read all" ), LOGLEVEL_MESSAGE );
    delete[] pBuffer;
    pBuffer = NULL;
    
    DatasetManager::getInstance()->setCountFibers( m_countLines );
    m_type = FIBERS;
    m_fullPath = filename;
#ifdef __WXMSW__
    m_name = wxT( "-" ) + filename.AfterLast( '\\' );
#else
    m_name = wxT( "-" ) + filename.AfterLast( '/' );
#endif
    //m_pKdTree = new KdTree( m_countPoints, &m_pointArray[0], m_dh );
    return true;
}

bool Fibers::loadMRtrix( const wxString &filename )
{
    Logger::getInstance()->print( wxT( "Loading MRtrix file" ), LOGLEVEL_MESSAGE );
    wxFile dataFile;
    long int nSize = 0;
    long int pc = 0, nodes = 0;
    converterByteFloat cbf;
    float x, y, z, x2, y2, z2;
    std::vector< float > tmpPoints;
    vector< vector< float > > lines;

    //Open file
    FILE *pFs = fopen( filename.ToAscii(), "r" ) ;
    ////
    // read header
    ////
    char lineBuffer[200];

    for( int i = 0; i < 22; ++i )
    {
        fgets( lineBuffer, 200, pFs );
        std::string s0( lineBuffer );

        if( s0.find( "file" ) != std::string::npos )
        {
            sscanf( lineBuffer, "file: . %ld", &pc );
        }

        if( s0.find( "count" ) != std::string::npos )
        {
            sscanf( lineBuffer, "count: %i", &m_countLines );
        }
    }

    fclose( pFs );

    if( dataFile.Open( filename ) )
    {
        nSize = dataFile.Length();

        if( nSize < 1 )
        {
            return false;
        }
    }

    nSize -= pc;
    dataFile.Seek( pc );
    wxUint8 *pBuffer = new wxUint8[nSize];
    dataFile.Read( pBuffer, nSize );
    dataFile.Close();
    
    cout << " Read fibers:\n";
    pc = 0;
    m_countPoints = 0; // number of points

    for( int i = 0; i < m_countLines; i++ )
    {
        tmpPoints.clear();
        nodes = 0;
        // read one tract
        cbf.b[0] = pBuffer[pc++];
        cbf.b[1] = pBuffer[pc++];
        cbf.b[2] = pBuffer[pc++];
        cbf.b[3] = pBuffer[pc++];
        x = cbf.f;
        cbf.b[0] = pBuffer[pc++];
        cbf.b[1] = pBuffer[pc++];
        cbf.b[2] = pBuffer[pc++];
        cbf.b[3] = pBuffer[pc++];
        y = cbf.f;
        cbf.b[0] = pBuffer[pc++];
        cbf.b[1] = pBuffer[pc++];
        cbf.b[2] = pBuffer[pc++];
        cbf.b[3] = pBuffer[pc++];
        z = cbf.f;
        //add first point
        tmpPoints.push_back( x );
        tmpPoints.push_back( y );
        tmpPoints.push_back( z );
        ++nodes;
        x2 = x;
        cbf.f = x2;

        //Read points (x,y,z) until x2 equals NaN (0x0000C07F), meaning end of the tract.
        while( !( cbf.b[0] == 0x00 && cbf.b[1] == 0x00 && cbf.b[2] == 0xC0 && cbf.b[3] == 0x7F ) )
        {
            cbf.b[0] = pBuffer[pc++];   // get next float
            cbf.b[1] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[3] = pBuffer[pc++];
            x2 = cbf.f;
            cbf.b[0] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[3] = pBuffer[pc++];
            y2 = cbf.f;
            cbf.b[0] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[3] = pBuffer[pc++];
            z2 = cbf.f;

            // downsample fibers: take only points in distance of min 0.75 mm
            if( ( ( x - x2 ) * ( x - x2 ) + ( y - y2 ) * ( y - y2 ) + ( z - z2 ) * ( z - z2 ) ) >= 0.2 )
            {
                x = x2;
                y = y2;
                z = z2;
                tmpPoints.push_back( x );
                tmpPoints.push_back( y );
                tmpPoints.push_back( z );
                ++nodes;
            }

            cbf.f = x2;
        }

        // put the tract in the line array
        lines.push_back( tmpPoints );

        for( int i = 0; i < nodes ; i++ )
        {
            m_countPoints++;
        }
    }

    delete[] pBuffer;
    pBuffer = NULL;
    
    ////
    //POST PROCESS: set all the data in the right format for the navigator
    ////
    DatasetManager::getInstance()->setCountFibers( m_countLines );
    m_pointArray.max_size();
    m_linePointers.resize( m_countLines + 1 );
    m_pointArray.resize( m_countPoints * 3 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );
    m_linePointers[0] = 0;

    for( int i = 0; i < m_countLines; ++i )
    {
        m_linePointers[i + 1] = m_linePointers[i] + lines[i].size() / 3;
    }

    int lineCounter = 0;

    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[lineCounter + 1] )
        {
            ++lineCounter;
        }

        m_reverse[i] = lineCounter;
    }

    unsigned int pos = 0;
    vector< vector< float > >::iterator it;

    for( it = lines.begin(); it < lines.end(); it++ )
    {
        vector< float >::iterator it2;

        for( it2 = ( *it ).begin(); it2 < ( *it ).end(); it2++ )
        {
            m_pointArray[pos++] = *it2;
        }
    }
    
    // The MrTrix fibers are defined in the same geometric reference
    // as the anatomical file. That is, the fibers coordinates are related to 
    // the anatomy in world space. The transformation from local to world space
    // for the anatomy is encoded in the m_dh->m_niftiTransform member.
    // Since we do not consider this tranform when loading the anatomy, we must 
    // bring back the fibers in the same reference, using the inverse of the 
    // local to world transformation. A further problem arises when loading an
    // anatomy that has voxels with dimensions differing from 1x1x1. The 
    // scaling factor is encoded in the transformation matrix, but we do not,
    // for the moment, use this scaling. Therefore, we must remove it from the
    // the transformation matrix before computing its inverse.
    FMatrix localToWorld = FMatrix( DatasetManager::getInstance()->getNiftiTransform() );

    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    if( voxelX != 1.0 || voxelY != 1.0 || voxelZ != 1.0 )
    {
        FMatrix rotMat( 3, 3 );
        localToWorld.getSubMatrix( rotMat, 0, 0 );
        
        FMatrix scaleInversion( 3, 3 );
        scaleInversion( 0, 0 ) = 1.0 / voxelX;
        scaleInversion( 1, 1 ) = 1.0 / voxelY;
        scaleInversion( 2, 2 ) = 1.0 / voxelZ;
        
        rotMat = scaleInversion * rotMat;
        
        localToWorld.setSubMatrix( 0, 0, rotMat );
    }
       
    FMatrix invertedTransform( 4, 4 );
    invertedTransform = invert( localToWorld );

    for( int i = 0; i < m_countPoints * 3; ++i )
    {
        FMatrix curPoint( 4, 1 );
        curPoint( 0, 0 ) = m_pointArray[i];
        curPoint( 1, 0 ) = m_pointArray[i + 1];
        curPoint( 2, 0 ) = m_pointArray[i + 2];
        curPoint( 3, 0 ) = 1;
        
        FMatrix invertedPoint = invertedTransform * curPoint;
        
        m_pointArray[i] = invertedPoint( 0, 0 );
        m_pointArray[i + 1] = invertedPoint( 1, 0 );
        m_pointArray[i + 2] = invertedPoint( 2, 0 );
        
        i += 2;
    }

    Logger::getInstance()->print( wxT( "TCK file loaded" ), LOGLEVEL_MESSAGE );
    createColorArray( false );
    m_type = FIBERS;
    m_fullPath = filename;
    //m_pKdTree = new KdTree( m_countPoints, &m_pointArray[0], m_dh );
#ifdef __WXMSW__
    m_name = wxT( "-" ) + filename.AfterLast( '\\' );
#else
    m_name = wxT( "-" ) + filename.AfterLast( '/' );
#endif
    return true;
}

bool Fibers::loadPTK( const wxString &filename )
{
    Logger::getInstance()->print( wxT( "Loading PTK file" ), LOGLEVEL_MESSAGE );
    wxFile dataFile;
    wxFileOffset nSize = 0;
    int pc = 0;
    converterByteINT32 cbi;
    converterByteFloat cbf;
    vector< float > tmpPoints;

    if( dataFile.Open( filename ) )
    {
        nSize = dataFile.Length();

        if( nSize == wxInvalidOffset )
            return false;
    }

    wxUint8 *pBuffer = new wxUint8[nSize];
    dataFile.Read( pBuffer, nSize );
    m_countLines  = 0; // Number of lines.
    m_countPoints = 0; // Number of points.

    while( pc < nSize )
    {
        ++m_countLines;
        cbi.b[0] = pBuffer[pc++];
        cbi.b[1] = pBuffer[pc++];
        cbi.b[2] = pBuffer[pc++];
        cbi.b[3] = pBuffer[pc++];
        m_lineArray.push_back( cbi.i );

        for( size_t i = 0; i < cbi.i; ++i )
        {
            m_lineArray.push_back( m_countPoints );
            ++m_countPoints;
            cbf.b[0] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[3] = pBuffer[pc++];
            tmpPoints.push_back( cbf.f );
            cbf.b[0] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[3] = pBuffer[pc++];
            tmpPoints.push_back( cbf.f );
            cbf.b[0] = pBuffer[pc++];
            cbf.b[1] = pBuffer[pc++];
            cbf.b[2] = pBuffer[pc++];
            cbf.b[3] = pBuffer[pc++];
            tmpPoints.push_back( cbf.f );
        }
    }

    m_linePointers.resize( m_countLines + 1 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );
    m_pointArray.resize( tmpPoints.size() );

    for( size_t i = 0; i < tmpPoints.size(); ++i )
    {
        m_pointArray[i] = tmpPoints[i];
    }

    printf( "%d lines and %d points \n", m_countLines, m_countPoints );
    Logger::getInstance()->print( wxT( "Move vertices" ), LOGLEVEL_MESSAGE );

    /*for( int i = 0; i < m_countPoints * 3; ++i )
    {
    m_pointArray[i] = m_dh->m_columns - m_pointArray[i];
    ++i;
    m_pointArray[i] = m_dh->m_rows - m_pointArray[i];
    ++i;
    m_pointArray[i] = m_dh->m_frames - m_pointArray[i];
    }*/
    /********************************************************************
    * This is a fix for the visContest
    * Only tested on -visContest fibers
    *                -PGuevara datas
    *
    * Hypothesis: If bundles computed in ptk, coordinates (x,y,z) are
    * already in the space of the dataset. Good voxel size and origin
    *
    ********************************************************************/
    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();
    
    for( int i = 0; i < m_countPoints * 3; ++i )
    {
        m_pointArray[i] = columns * voxelX - m_pointArray[i];
        ++i;
        m_pointArray[i] = rows    * voxelY - m_pointArray[i];
        ++i;
        m_pointArray[i] = frames  * voxelZ - m_pointArray[i];
    }

    calculateLinePointers();
    createColorArray( false );
    Logger::getInstance()->print( wxT( "Read all" ), LOGLEVEL_MESSAGE );
    
    delete[] pBuffer;
    pBuffer = NULL;
    
    DatasetManager::getInstance()->setCountFibers( m_countLines );
    m_type = FIBERS;
    m_fullPath = filename;
#ifdef __WXMSW__
    m_name = wxT( "-" ) + filename.AfterLast( '\\' );
#else
    m_name = wxT( "-" ) + filename.AfterLast( '/' );
#endif
    //m_pKdTree = new KdTree( m_countPoints, &m_pointArray[0], m_dh );
    return true;
}

bool Fibers::loadVTK( const wxString &filename )
{
    Logger::getInstance()->print( wxT( "Loading VTK file" ), LOGLEVEL_MESSAGE );
    wxFile dataFile;
    wxFileOffset nSize = 0;

    if( dataFile.Open( filename ) )
    {
        nSize = dataFile.Length();

        if( nSize == wxInvalidOffset )
            return false;
    }

    wxUint8 *pBuffer = new wxUint8[255];
    dataFile.Read( pBuffer, ( size_t )255 );
    
    int pointOffset         = 0;
    int lineOffset          = 0;
    int pointColorOffset    = 0;
    int lineColorOffset     = 0;
    int fileOffset          = 0;
    int j                   = 0;

    bool colorsLoadedFromFile( false );

    char *pTemp = new char[256];

    // Ignore the first 2 lines.
    while( pBuffer[fileOffset] != '\n' )
    {
        ++fileOffset;
    }

    ++fileOffset;

    while( pBuffer[fileOffset] != '\n' )
    {
        ++fileOffset;
    }

    ++fileOffset;

    // Check the file type.
    while( pBuffer[fileOffset] != '\n' )
    {
        pTemp[j] = pBuffer[fileOffset];
        ++fileOffset;
        ++j;
    }

    ++fileOffset;
    pTemp[j] = 0;
    wxString type( pTemp, wxConvUTF8 );

    if( type == wxT( "ASCII" ) )
    {
        // ASCII file, maybe later.
        return false;
    }

    if( type != wxT( "BINARY" ) )
    {
        // Something else, don't know what to do.
        return false;
    }

    // Ignore line DATASET POLYDATA.
    while( pBuffer[fileOffset] != '\n' )
    {
        ++fileOffset;
    }

    ++fileOffset;
    j = 0;

    // Read POINTS.
    while( pBuffer[fileOffset] != '\n' )
    {
        pTemp[j] = pBuffer[fileOffset];
        ++fileOffset;
        ++j;
    }

    ++fileOffset;
    pTemp[j] = 0;
    wxString points( pTemp, wxConvUTF8 );
    points = points.AfterFirst( ' ' );
    points = points.BeforeFirst( ' ' );
    long tempValue = 0;

    if( ! points.ToLong( &tempValue, 10 ) )
    {
        return false; // Can't read point count.
    }

    int countPoints = ( int )tempValue;
    // Start position of the point array in the file.
    pointOffset = fileOffset;
    // Jump to postion after point array.
    fileOffset += ( 12 * countPoints ) + 1;
    j = 0;
    dataFile.Seek( fileOffset );
    dataFile.Read( pBuffer, ( size_t ) 255 );

    while( pBuffer[j] != '\n' )
    {
        pTemp[j] = pBuffer[j];
        ++fileOffset;
        ++j;
    }

    ++fileOffset;
    pTemp[j] = 0;
    wxString sLines( pTemp, wxConvUTF8 );
    wxString sLengthLines = sLines.AfterLast( ' ' );

    if( ! sLengthLines.ToLong( &tempValue, 10 ) )
    {
        return false; // Can't read size of lines array.
    }

    int lengthLines = ( int( tempValue ) );
    sLines = sLines.AfterFirst( ' ' );
    sLines = sLines.BeforeFirst( ' ' );

    if( ! sLines.ToLong( &tempValue, 10 ) )
    {
        return false; // Can't read lines.
    }

    int countLines = ( int ) tempValue;
    // Start postion of the line array in the file.
    lineOffset = fileOffset;
    // Jump to postion after line array.
    fileOffset += ( lengthLines * 4 ) + 1;
    dataFile.Seek( fileOffset );
    dataFile.Read( pBuffer, ( size_t ) 255 );
    j = 0;
    int k = 0;

    // TODO test if there's really a color array.
    while( pBuffer[k] != '\n' )
    {
        pTemp[j] = pBuffer[k];
        ++fileOffset;
        ++j;
        ++k;
    }

    ++k;
    ++fileOffset;
    pTemp[j] = 0;
    wxString tmpString( pTemp, wxConvUTF8 );
    j = 0;

    while( pBuffer[k] != '\n' )
    {
        pTemp[j] = pBuffer[k];
        ++fileOffset;
        ++j;
        ++k;
    }

    ++fileOffset;
    pTemp[j] = 0;
    wxString tmpString2( pTemp, wxConvUTF8 );

    if( tmpString.BeforeFirst( ' ' ) == _T( "CELL_DATA" ) )
    {
        lineColorOffset = fileOffset;
        fileOffset += ( countLines * 3 ) + 1;
        dataFile.Seek( fileOffset );
        dataFile.Read( pBuffer, ( size_t ) 255 );
        // aa 2009/06/26 workaround if the pBuffer doesn't contain a string.
        pBuffer[254] = '\n';
        int k = j = 0;

        // TODO test if there's really a color array.
        while( pBuffer[k] != '\n' )
        {
            pTemp[j] = pBuffer[k];
            ++fileOffset;
            ++j;
            ++k;
        }

        ++k;
        ++fileOffset;
        pTemp[j] = 0;
        wxString tmpString3( pTemp, wxConvUTF8 );
        tmpString = tmpString3;
        j = 0;

        while( pBuffer[k] != '\n' )
        {
            pTemp[j] = pBuffer[k];
            ++fileOffset;
            ++j;
            ++k;
        }

        ++fileOffset;
        pTemp[j] = 0;
        wxString tmpString4( pTemp, wxConvUTF8 );
        tmpString2 = tmpString4;
    }

    if( tmpString.BeforeFirst( ' ' ) == _T( "POINT_DATA" ) && tmpString2.BeforeFirst( ' ' ) == _T( "COLOR_SCALARS" ) )
    {
        pointColorOffset = fileOffset;
    }

    Logger::getInstance()->print( wxString::Format( wxT( "Loading %d points and %d lines" ), countPoints, countLines ), LOGLEVEL_MESSAGE );
    m_countLines        = countLines;
    DatasetManager::getInstance()->setCountFibers( m_countLines );
    m_countPoints       = countPoints;
    
    m_linePointers.resize( m_countLines + 1 );
    m_linePointers[countLines] = countPoints;
    m_reverse.resize( countPoints );
    m_filtered.resize( countLines, false );
    m_selected.resize( countLines, false );
    m_pointArray.resize( countPoints * 3 );
    m_lineArray.resize( lengthLines * 4 );
    m_colorArray.resize( countPoints * 3 );
    
    dataFile.Seek( pointOffset );
    dataFile.Read( &m_pointArray[0], ( size_t )countPoints * 12 );
    dataFile.Seek( lineOffset );
    dataFile.Read( &m_lineArray[0], ( size_t )lengthLines * 4 );

    if( pointColorOffset != 0 )
    {
        vector< wxUint8 > tmpColorArray( countPoints * 3, 0 );
        dataFile.Seek( pointColorOffset );
        dataFile.Read( &tmpColorArray[0], ( size_t ) countPoints * 3 );

        for( size_t i = 0; i < tmpColorArray.size(); ++i )
        {
            m_colorArray[i] = tmpColorArray[i] / 255.;
        }

        colorsLoadedFromFile = true;
    }

    toggleEndianess();
    Logger::getInstance()->print( wxT( "Move vertices" ), LOGLEVEL_MESSAGE );

    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    for( int i = 0; i < countPoints * 3; ++i )
    {
        m_pointArray[i] = columns * voxelX - m_pointArray[i];
        ++i;
        m_pointArray[i] = rows    * voxelY - m_pointArray[i];
        ++i;
        //m_pointArray[i] = frames - m_pointArray[i];
    }

    calculateLinePointers();
    createColorArray( colorsLoadedFromFile );
    Logger::getInstance()->print( wxT( "Read all" ), LOGLEVEL_MESSAGE );
    m_type      = FIBERS;
    m_fullPath  = filename;
#ifdef __WXMSW__
    m_name = wxT( "-" ) + filename.AfterLast( '\\' );
#else
    m_name = wxT( "-" ) + filename.AfterLast( '/' );
#endif
    //m_pKdTree = new KdTree( m_countPoints, &m_pointArray[0], m_dh );
    delete[] pBuffer;
    delete[] pTemp;
    return true;
}

bool Fibers::loadDmri( const wxString &filename )
{
    FILE *pFile;
    pFile = fopen( filename.mb_str(), "r" );

    if( pFile == NULL ) 
    {   
        return false;
    }

    char *pS1 = new char[10];
    char *pS2 = new char[10];
    char *pS3 = new char[10];
    char *pS4 = new char[10];
    float f1, f2, f3, f4, f5;
    int res;
    
    // the header
    res = fscanf( pFile, "%f %s", &f1, pS1 );
    res = fscanf( pFile, "%f %s %s %s %s", &f1, pS1, pS2, pS3, pS4 );
    res = fscanf( pFile, "%f", &f1 );
    res = fscanf( pFile, "%f %f %f %f %f", &f1, &f2, &f3, &f4, &f5 );
    res = fscanf( pFile, "%f %f %f %f %f", &f1, &f2, &f3, &f4, &f5 );
    res = fscanf( pFile, "%f %f %f %f %f", &f1, &f2, &f3, &f4, &f5 );
    res = fscanf( pFile, "%d %f", &m_countLines, &f2 );
    
    delete[] pS1;
    delete[] pS2;
    delete[] pS3;
    delete[] pS4;
    
    pS1 = NULL;
    pS2 = NULL;
    pS3 = NULL;
    pS4 = NULL;
    
    // the list of points
    vector< vector< float > > lines;
    m_countPoints = 0;
    float back, front;

    for( int i = 0; i < m_countLines; i++ )
    {
        res = fscanf( pFile, "%f %f %f", &back, &front, &f1 );
        int nbpoints = back + front;

        if( back != 0 && front != 0 )
        {
            nbpoints--;
        }

        if( nbpoints > 0 )
        {
            vector< float > curLine;
            curLine.resize( nbpoints * 3 );

            //back
            for( int j = back - 1; j >= 0; j-- )
            {
                res = fscanf( pFile, "%f %f %f %f", &f1, &f2, &f3, &f4 );
                curLine[j * 3]  = f1;
                curLine[j * 3 + 1] = f2;
                curLine[j * 3 + 2] = f3;
            }

            if( back != 0 && front != 0 )
            {
                //repeated pts
                res = fscanf( pFile, "%f %f %f %f", &f1, &f2, &f3, &f4 );
            }

            //front
            for( int j = back; j < nbpoints; j++ )
            {
                res = fscanf( pFile, "%f %f %f %f", &f1, &f2, &f3, &f4 );
                curLine[j * 3]  = f1;
                curLine[j * 3 + 1] = f2;
                curLine[j * 3 + 2] = f3;
            }

            m_countPoints += curLine.size() / 3;
            lines.push_back( curLine );
        }
    }

    fclose( pFile );
    
    //set all the data in the right format for the navigator
    m_countLines = lines.size();
    DatasetManager::getInstance()->setCountFibers( m_countLines + 1 );
    m_pointArray.max_size();
    m_linePointers.resize( m_countLines + 1 );
    m_pointArray.resize( m_countPoints * 3 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );
    m_linePointers[0] = 0;

    for( int i = 0; i < m_countLines; ++i )
    {
        m_linePointers[i + 1] = m_linePointers[i] + lines[i].size() / 3;
    }

    int lineCounter = 0;

    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[lineCounter + 1] )
        {
            ++lineCounter;
        }

        m_reverse[i] = lineCounter;
    }

    unsigned int pos = 0;
    vector< vector< float > >::iterator it;

    for( it = lines.begin(); it < lines.end(); it++ )
    {
        vector< float >::iterator it2;

        for( it2 = ( *it ).begin(); it2 < ( *it ).end(); it2++ )
        {
            m_pointArray[pos++] = *it2;
        }
    }

    createColorArray( false );
    m_type = FIBERS;
    m_fullPath = filename;
    //m_pKdTree = new KdTree( m_countPoints, &m_pointArray[0], m_dh );
#ifdef __WXMSW__
    m_name = /*"-"+*/ filename.AfterLast( '\\' );
#else
    m_name = /*"-" +*/ filename.AfterLast( '/' );
#endif
    return true;
}

///////////////////////////////////////////////////////////////////////////
// This function was made for debug purposes, it will create a fake set of
// fibers with hardcoded value to be able to test different things.
///////////////////////////////////////////////////////////////////////////
void Fibers::loadTestFibers()
{
    m_countLines        = 2;  // The number of fibers you want to display.
    int lengthLines   = 10; // The number of points each fiber will have.
    int pos = 0;
    DatasetManager::getInstance()->setCountFibers( m_countLines );
    m_countPoints       = m_countLines * lengthLines;
    
    m_linePointers.resize( m_countLines + 1 );
    m_pointArray.resize( m_countPoints * 3 );
    m_linePointers[m_countLines] = m_countPoints;
    m_reverse.resize( m_countPoints );
    m_selected.resize( m_countLines, false );
    m_filtered.resize( m_countLines, false );
    
    // Because you need to load an anatomy file first in order to load this fake set of fibers,
    // the points composing your fibers have to be between [0,159] in x, [0,199] in y and [0,159] in z.
    // This is for a straight line.
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 110.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 120.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 130.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 140.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 150.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 160.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 170.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 180.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 60.0f;
    m_pointArray[pos++] = 190.0f;
    m_pointArray[pos++] = 60.0f;
    
    // This is for a circle in 2D (Z never changes).
    float circleRadius = 10.0f;
    float offset       = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI *  0.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI *  0.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 10.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 10.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 20.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 20.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 30.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 30.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 40.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 40.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 50.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 50.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 60.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 60.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 70.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 70.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 80.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 80.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;
    m_pointArray[pos++] = circleRadius * sin( M_PI * 90.0f / 180.0f ) + offset;
    m_pointArray[pos++] = circleRadius * cos( M_PI * 90.0f / 180.0f ) + offset;
    m_pointArray[pos++] = 100.0f;

    // No need to modify the rest of this function if you only want to add a test fiber.
    for( int i = 0; i < m_countLines; ++i )
    {
        m_linePointers[i] = i * lengthLines;
    }
    

    int lineCounter = 0;

    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[lineCounter + 1] )
        {
            ++lineCounter;
        }

        m_reverse[i] = lineCounter;
    }

    m_pointArray.resize( m_countPoints * 3 );
    createColorArray( false );
    m_type = FIBERS;
    //m_pKdTree = new KdTree( m_countPoints, &m_pointArray[0], m_dh );
}

///////////////////////////////////////////////////////////////////////////
// This function will call the proper coloring function for the fibers.
///////////////////////////////////////////////////////////////////////////
void Fibers::updateFibersColors()
{	
    if( m_fiberColorationMode == NORMAL_COLOR )
    {
        resetColorArray();
    }
    else
    {
		float *pColorData( NULL );
		
		if( SceneManager::getInstance()->isUsingVBO() )
		{
			glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
			pColorData = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
		}
        else
        {
            pColorData  = &m_colorArray[0];
        }

        if( m_fiberColorationMode == CURVATURE_COLOR )
        {
            colorWithCurvature( pColorData );
        }
        else if( m_fiberColorationMode == TORSION_COLOR )
        {
            colorWithTorsion( pColorData );
        }
        else if( m_fiberColorationMode == DISTANCE_COLOR )
        {
            colorWithDistance( pColorData );
        }
        else if( m_fiberColorationMode == MINDISTANCE_COLOR )
        {
            colorWithMinDistance( pColorData );
        }

        if( SceneManager::getInstance()->isUsingVBO() )
        {
            glUnmapBuffer( GL_ARRAY_BUFFER );
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will color the fibers depending on their torsion value.
//
// pColorData      : A pointer to the fiber color info.
///////////////////////////////////////////////////////////////////////////
void Fibers::colorWithTorsion( float *pColorData )
{
    if( pColorData == NULL )
    {
        return;
    }

    int    pc = 0;
    // TODO remove
    //Vector firstDerivative, secondDerivative, thirdDerivative;

    // For each fibers.
    for( int i = 0; i < getLineCount(); ++i )
    {
        double color        = 0.0f;
        int    index        = 0;
        float  progression  = 0.0f;
        int    pointPerLine = getPointsPerLine( i );

        // We cannot calculate the torsion for a fiber that as less that 5 points.
        // So we simply do not cange the color for this fiber
        if( pointPerLine < 5 )
        {
            continue;
        }

        // For each points of this fiber.
        for( int j = 0; j < pointPerLine; ++j )
        {
            if( j == 0 )
            {
                index = 6;                             // For the first point of each fiber.
                progression = 0.0f;
            }
            else if( j == 1 )
            {
                index = 6;                             // For the second point of each fiber.
                progression = 0.25f;
            }
            else if( j == pointPerLine - 2 )
            {
                index = ( pointPerLine - 2 ) * 3;    // For the before last point of each fiber.
                progression = 0.75f;
            }
            else if( j == pointPerLine - 1 )
            {
                index = ( pointPerLine - 2 ) * 3;    // For the last point of each fiber.
                progression = 1.0f;
            }
            else
            {
                progression = 0.5f;     // For every other points.
            }

            MyApp::frame->getLastSelectedObj()->getProgressionTorsion( 
                    Vector( m_pointArray[index - 6], m_pointArray[index - 5], m_pointArray[index - 4] ),
                    Vector( m_pointArray[index - 3], m_pointArray[index - 2], m_pointArray[index - 1] ),
                    Vector( m_pointArray[index],     m_pointArray[index + 1], m_pointArray[index + 2] ),
                    Vector( m_pointArray[index + 3], m_pointArray[index + 4], m_pointArray[index + 5] ),
                    Vector( m_pointArray[index + 6], m_pointArray[index + 7], m_pointArray[index + 8] ),
                    progression, color );
            
            // Lets apply a specific hard coded coloration for the torsion.
            float realColor;

            if( color <= 0.01f ) // Those points have no torsion so we simply but them pure blue.
            {
                pColorData[pc]     = 0.0f;
                pColorData[pc + 1] = 0.0f;
                pColorData[pc + 2] = 1.0f;
            }
            else if( color < 0.1f )  // The majority of the values are here.
            {
                double normalizedValue = ( color - 0.01f ) / ( 0.1f - 0.01f );
                realColor = ( pow( ( double )2.71828182845904523536, normalizedValue ) ) - 1.0f;
                pColorData[pc]     = 0.0f;
                pColorData[pc + 1] = realColor;
                pColorData[pc + 2] = 1.0f - realColor;
            }
            else // All the rest is simply pure green.
            {
                pColorData[pc]     = 0.0f;
                pColorData[pc + 1] = 1.0f;
                pColorData[pc + 2] = 0.0f;
            }

            pc    += 3;
            index += 3;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will color the fibers depending on their curvature value.
//
// pColorData      : A pointer to the fiber color info.
///////////////////////////////////////////////////////////////////////////
void Fibers::colorWithCurvature( float *pColorData )
{
    if( pColorData == NULL )
    {
        return;
    }

    int    pc = 0;
    // TODO remove
    //Vector firstDerivative, secondDerivative, thirdDerivative;

    // For each fibers.
    for( int i = 0; i < getLineCount(); ++i )
    {
        double color        = 0.0f;
        int    index        = 0;
        float  progression  = 0.0f;
        int    pointPerLine = getPointsPerLine( i );

        // We cannot calculate the curvature for a fiber that as less that 5 points.
        // So we simply do not cange the color for this fiber
        if( pointPerLine < 5 )
        {
            continue;
        }

        // For each point of this fiber.
        for( int j = 0; j < pointPerLine; ++j )
        {
            if( j == 0 )
            {
                index = 6;                             // For the first point of each fiber.
                progression = 0.0f;
            }
            else if( j == 1 )
            {
                index = 6;                             // For the second point of each fiber.
                progression = 0.25f;
            }
            else if( j == pointPerLine - 2 )
            {
                index = ( pointPerLine - 2 ) * 3;    // For the before last point of each fiber.
                progression = 0.75f;
            }
            else if( j == pointPerLine - 1 )
            {
                index = ( pointPerLine - 2 ) * 3;    // For the last point of each fiber.
                progression = 1.0f;
            }
            else
            {
                progression = 0.5f;     // For every other points.
            }

            MyApp::frame->getLastSelectedObj()->getProgressionCurvature(
                    Vector( m_pointArray[index - 6], m_pointArray[index - 5], m_pointArray[index - 4] ),
                    Vector( m_pointArray[index - 3], m_pointArray[index - 2], m_pointArray[index - 1] ),
                    Vector( m_pointArray[index],     m_pointArray[index + 1], m_pointArray[index + 2] ),
                    Vector( m_pointArray[index + 3], m_pointArray[index + 4], m_pointArray[index + 5] ),
                    Vector( m_pointArray[index + 6], m_pointArray[index + 7], m_pointArray[index + 8] ),
                    progression, color );
            
            // Lets apply a specific hard coded coloration for the curvature.
            float realColor;

            if( color <= 0.01f ) // Those points have no curvature so we simply but them pure blue.
            {
                pColorData[pc]     = 0.0f;
                pColorData[pc + 1] = 0.0f;
                pColorData[pc + 2] = 1.0f;
            }
            else if( color < 0.1f )  // The majority of the values are here.
            {
                double normalizedValue = ( color - 0.01f ) / ( 0.1f - 0.01f );
                realColor = ( pow( ( double )2.71828182845904523536, normalizedValue ) ) - 1.0f;
                pColorData[pc]     = 0.0f;
                pColorData[pc + 1] = realColor;
                pColorData[pc + 2] = 1.0f - realColor;
            }
            else // All the rest is simply pure green.
            {
                pColorData[pc]     = 0.0f;
                pColorData[pc + 1] = 1.0f;
                pColorData[pc + 2] = 0.0f;
            }

            pc    += 3;
            index += 3;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// This function will color the fibers depending on their distance to the
// flagged distance anchors voi.
//
// pColorData      : A pointer to the fiber color info.
///////////////////////////////////////////////////////////////////////////
void Fibers::colorWithDistance( float *pColorData )
{
	if( pColorData == NULL )
    {
        return;
    }
	
    SelectionObjList selObjs = SceneManager::getInstance()->getSelectionObjects();
    vector< SelectionObject* > simplifiedList;

    for( unsigned int i = 0; i < selObjs.size(); ++i )
    {
        for( unsigned int j = 0; j < selObjs[i].size(); ++j )
        {
            if( selObjs[i][j]->IsUsedForDistanceColoring() )
            {
                simplifiedList.push_back( selObjs[i][j] );
            }
        }
    }

    int   columns = DatasetManager::getInstance()->getColumns();
    int   rows    = DatasetManager::getInstance()->getRows();
    int   frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    for( int i = 0; i < getPointCount(); ++i )
    {
        float minDistance = FLT_MAX;
        int x     = std::min( columns - 1, std::max( 0, (int)( m_pointArray[i * 3 ]    / voxelX ) ) );
        int y     = std::min( rows    - 1, std::max( 0, (int)( m_pointArray[i * 3 + 1] / voxelY ) ) );
        int z     = std::min( frames  - 1, std::max( 0, (int)( m_pointArray[i * 3 + 2] / voxelZ ) ) );
        int index = x + y * columns + z * rows * columns;

        for( unsigned int j = 0; j < simplifiedList.size(); ++j )
        {
            if( simplifiedList[j]->m_sourceAnatomy != NULL )
            {
                float curValue = simplifiedList[j]->m_sourceAnatomy->at( index );

                if( curValue < minDistance )
                {
                    minDistance = curValue;
                }
            }
        }

        float thresh = m_threshold / 2.0f;

        if( minDistance > ( thresh ) && minDistance < ( thresh + LINEAR_GRADIENT_THRESHOLD ) )
        {
            float greenVal = ( minDistance - thresh ) / LINEAR_GRADIENT_THRESHOLD;
            float redVal = 1 - greenVal;
            pColorData[3 * i]      = redVal;
            pColorData[3 * i + 1]  = greenVal;
            pColorData[3 * i + 2]  = 0.0f;
        }
        else if( minDistance > ( thresh + LINEAR_GRADIENT_THRESHOLD ) )
        {
            pColorData[3 * i ]     = 0.0f;
            pColorData[3 * i + 1]  = 1.0f;
            pColorData[3 * i + 2]  = 0.0f;
        }
        else
        {
            pColorData[3 * i ]     = 1.0f;
            pColorData[3 * i + 1]  = 0.0f;
            pColorData[3 * i + 2]  = 0.0f;
        }
    }
}

void Fibers::colorWithMinDistance( float *pColorData )
{
	if( pColorData == NULL )
    {
        return;
    }
	
    SelectionObjList selObjs = SceneManager::getInstance()->getSelectionObjects();
    vector< SelectionObject* > simplifiedList;

    for( unsigned int i = 0; i < selObjs.size(); ++i )
    {
        for( unsigned int j = 0; j < selObjs[i].size(); ++j )
        {
            if( selObjs[i][j]->IsUsedForDistanceColoring() )
            {
                simplifiedList.push_back( selObjs[i][j] );
            }
        }
    }

    int   columns = DatasetManager::getInstance()->getColumns();
    int   rows    = DatasetManager::getInstance()->getRows();
    int   frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    for( int i = 0; i < getLineCount(); ++i )
    {
        int nbPointsInLine = getPointsPerLine( i );
        int index = getStartIndexForLine( i );
        float minDistance = FLT_MAX;

        for( int j = 0; j < nbPointsInLine; ++j )
        {
            int x     = std::min( columns - 1, std::max( 0, (int)( m_pointArray[( index + j ) * 3 ]    / voxelX ) ) ) ;
            int y     = std::min( rows    - 1, std::max( 0, (int)( m_pointArray[( index + j ) * 3 + 1] / voxelY ) ) ) ;
            int z     = std::min( frames  - 1, std::max( 0, (int)( m_pointArray[( index + j ) * 3 + 2] / voxelZ ) ) ) ;
            int index = x + y * columns + z * rows * columns;

            for( unsigned int k = 0; k < simplifiedList.size(); ++k )
            {
                float curValue = simplifiedList[k]->m_sourceAnatomy->at( index );

                if( curValue < minDistance )
                {
                    minDistance = curValue;
                }
            }
        }

        float thresh = m_threshold / 2.0f;
        Vector theColor;
        float theAlpha;

        if( m_localizedAlpha.size() != ( unsigned int ) getPointCount() )
        {
            m_localizedAlpha = vector< float >( getPointCount() );
        }

        if( minDistance > ( thresh ) && minDistance < ( thresh + LINEAR_GRADIENT_THRESHOLD ) )
        {
            float greenVal = ( minDistance - thresh ) / LINEAR_GRADIENT_THRESHOLD;
            float redVal = 1 - greenVal;
            theColor.x  = redVal;
            theColor.y  = 0.9f;
            theColor.z  = 0.0f;

            if( redVal < MIN_ALPHA_VALUE )
            {
                theAlpha = MIN_ALPHA_VALUE;
            }
            else
            {
                theAlpha = pow( redVal, 6.0f );
            }
        }
        else if( minDistance > ( thresh + LINEAR_GRADIENT_THRESHOLD ) )
        {
            theColor.x  = 0.0f;
            theColor.y  = 1.0f;
            theColor.z  = 0.0f;
            theAlpha = MIN_ALPHA_VALUE;
        }
        else
        {
            theColor.x  = 1.0f;
            theColor.y  = 0.0f;
            theColor.z  = 0.0f;
            theAlpha = 1.0;
        }

        for( int j = 0; j < nbPointsInLine; ++j )
        {
            pColorData[( index + j ) * 3]     = theColor.x;
            pColorData[( index + j ) * 3 + 1] = theColor.y;
            pColorData[( index + j ) * 3 + 2] = theColor.z;
            m_localizedAlpha[index + j] = theAlpha;
        }
    }
}

Anatomy* Fibers::generateFiberVolume()
{
	float* pColorData( NULL );
	if( SceneManager::getInstance()->isUsingVBO() )
    {
		glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
			
		#ifdef __WXMAC__
			//glBufferData(GL_ARRAY_BUFFER, getPointCount()*3 + 2, NULL, GL_STREAM_DRAW);
		#endif
		pColorData = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
    }
    else
    {
        pColorData  = &m_colorArray[0];
    }

    if( m_localizedAlpha.size() != ( unsigned int )getPointCount() )
    {
        m_localizedAlpha = vector< float >( getPointCount(), 1 );
    }

    int index = DatasetManager::getInstance()->createAnatomy( RGB );
    Anatomy *pTmpAnatomy = (Anatomy *)DatasetManager::getInstance()->getDataset( index );
    pTmpAnatomy->setName( m_name.BeforeFirst( '.' ) + wxT(" Fiber-Density Volume" ) );
    
    MyApp::frame->m_pListCtrl2->InsertItem( index );

    MyApp::frame->refreshAllGLWidgets();

    int   columns = DatasetManager::getInstance()->getColumns();
    int   rows    = DatasetManager::getInstance()->getRows();
    int   frames  = DatasetManager::getInstance()->getFrames();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    for( int i = 0; i < getPointCount(); ++i )
    {
        int x     = std::min( columns - 1, std::max( 0, (int)( m_pointArray[i * 3 ]    / voxelX ) ) ) ;
        int y     = std::min( rows    - 1, std::max( 0, (int)( m_pointArray[i * 3 + 1] / voxelY ) ) ) ;
        int z     = std::min( frames  - 1, std::max( 0, (int)( m_pointArray[i * 3 + 2] / voxelZ ) ) ) ;
        int index = x + y * columns + z * rows * columns;
        
        ( *pTmpAnatomy->getFloatDataset() )[index * 3]     += pColorData[i * 3]     * m_localizedAlpha[i];
        ( *pTmpAnatomy->getFloatDataset() )[index * 3 + 1] += pColorData[i * 3 + 1] * m_localizedAlpha[i];
        ( *pTmpAnatomy->getFloatDataset() )[index * 3 + 2] += pColorData[i * 3 + 2] * m_localizedAlpha[i];
    }

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glUnmapBuffer( GL_ARRAY_BUFFER );
    }
	
	return pTmpAnatomy;
}

void Fibers::getFibersInfoToSave( vector<float>& pointsToSave,  vector<int>& linesToSave, vector<int>& colorsToSave, int& countLines )
{
    int pointIndex( 0 );
    countLines = 0;

	float *pColorData( NULL );
	
	if( SceneManager::getInstance()->isUsingVBO() )
    {
		glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
		pColorData = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
    }
    else
    {
        pColorData = &m_colorArray[0];
    }

    float columns = DatasetManager::getInstance()->getColumns();
    float rows    = DatasetManager::getInstance()->getRows();
    float voxelX  = DatasetManager::getInstance()->getVoxelX();
    float voxelY  = DatasetManager::getInstance()->getVoxelY();
    float voxelZ  = DatasetManager::getInstance()->getVoxelZ();

    for( int l = 0; l < m_countLines; ++l )
    {
        if( m_selected[l] && !m_filtered[l] )
        {
            unsigned int pc = getStartIndexForLine( l ) * 3;
            linesToSave.push_back( getPointsPerLine( l ) );

            for( int j = 0; j < getPointsPerLine( l ); ++j )
            {
                pointsToSave.push_back( columns * voxelX - m_pointArray[pc] );
                colorsToSave.push_back( ( wxUint8 )( pColorData[pc] * 255 ) );
                ++pc;
                pointsToSave.push_back( rows * voxelY - m_pointArray[pc] );
                colorsToSave.push_back( ( wxUint8 )( pColorData[pc] * 255 ) );
                ++pc;
                pointsToSave.push_back( m_pointArray[pc] );
                colorsToSave.push_back( ( wxUint8 )( pColorData[pc] * 255 ) );
                ++pc;
                linesToSave.push_back( pointIndex );
                ++pointIndex;
            }
            ++countLines;
        }
    }

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glUnmapBuffer( GL_ARRAY_BUFFER );
    }
}

void Fibers::getNbLines( int& nbLines )
{
	nbLines = 0;

    for( int l = 0; l < m_countLines; ++l )
    {
        if( m_selected[l] && !m_filtered[l] )
        {
            nbLines++;
        }
    }
}

void Fibers::loadDMRIFibersInFile( ofstream& myfile )
{
	for( int l = 0; l < m_countLines; ++l )
    {
        if( m_selected[l] && !m_filtered[l] )
        {
            unsigned int pc = getStartIndexForLine( l ) * 3;
            myfile << getPointsPerLine( l ) << " 1\n1\n";

            for( int j = 0; j < getPointsPerLine( l ); ++j )
            {
                myfile <<  m_pointArray[pc] << " " <<  m_pointArray[pc + 1] << " " <<  m_pointArray[pc + 2] << " 0\n";
                pc += 3;
            }

            pc = getStartIndexForLine( l ) * 3;
            myfile <<  m_pointArray[pc] << " " <<  m_pointArray[pc + 1] << " " <<  m_pointArray[pc + 2] << " 0\n";
        }
    }
}

/**
 * Save using the VTK binary format.
 */
void Fibers::save( wxString filename )
{
	ofstream myfile;
    char *pFn;
	vector<char> vBuffer;
	converterByteINT32 c;
    converterByteFloat f;
	vector<float> pointsToSave;
	vector<int> linesToSave;
	vector<int> colorsToSave;
	int countLines = 0;

	if( filename.AfterLast( '.' ) != _T( "fib" ) )
    {
        filename += _T( ".fib" );
    }

    pFn = ( char * ) malloc( filename.length() );
    strcpy( pFn, ( const char * ) filename.mb_str( wxConvUTF8 ) );
    myfile.open( pFn, ios::binary );

	getFibersInfoToSave( pointsToSave, linesToSave, colorsToSave, countLines );

	string header1 = "# vtk DataFile Version 3.0\nvtk output\nBINARY\nDATASET POLYDATA\nPOINTS ";
	header1 += intToString( pointsToSave.size() / 3 );
	header1 += " float\n";
	for( unsigned int i = 0; i < header1.size(); ++i )
	{
		vBuffer.push_back( header1[i] );
	}
	for( unsigned int i = 0; i < pointsToSave.size(); ++i )
	{
		f.f = pointsToSave[i];
		vBuffer.push_back( f.b[3] );
		vBuffer.push_back( f.b[2] );
		vBuffer.push_back( f.b[1] );
		vBuffer.push_back( f.b[0] );
	}
	
	vBuffer.push_back( '\n' );
	string header2 = "LINES " + intToString( countLines ) + " " + intToString( linesToSave.size() ) + "\n";
    for( unsigned int i = 0; i < header2.size(); ++i )
    {
        vBuffer.push_back( header2[i] );
    }
	for( unsigned int i = 0; i < linesToSave.size(); ++i )
	{
		c.i = linesToSave[i];
		vBuffer.push_back( c.b[3] );
		vBuffer.push_back( c.b[2] );
		vBuffer.push_back( c.b[1] );
		vBuffer.push_back( c.b[0] );
	}
    
	vBuffer.push_back( '\n' );
    string header3 = "POINT_DATA ";
    header3 += intToString( pointsToSave.size() / 3 );
    header3 += " float\n";
    header3 += "COLOR_SCALARS scalars 3\n";
    for( unsigned int i = 0; i < header3.size(); ++i )
    {
        vBuffer.push_back( header3[i] );
    }
	for( unsigned int i = 0; i < colorsToSave.size(); ++i )
	{
		vBuffer.push_back( colorsToSave[i] );
	}
	vBuffer.push_back( '\n' );

	// Put the buffer vector into a char* array.
    char* pBuffer = new char[vBuffer.size()];

    for( unsigned int i = 0; i < vBuffer.size(); ++i )
    {
        pBuffer[i] = vBuffer[i];
    }
	
	myfile.write( pBuffer, vBuffer.size() );
    myfile.close();
    
    delete[] pBuffer;
    pBuffer = NULL;
}

void Fibers::saveDMRI( wxString filename )
{
    ofstream myfile;
	int nbrlines;
    char *pFn;
	float dist = 0.5;

	if( filename.AfterLast( '.' ) != _T( "fib" ) )
    {
        filename += _T( ".fib" );
    }

    pFn = ( char * ) malloc( filename.length() );
    strcpy( pFn, ( const char * ) filename.mb_str( wxConvUTF8 ) );
    myfile.open( pFn, ios::out );
   
	getNbLines( nbrlines );

	myfile << "1 FA\n4 min max mean var\n1\n4 0 0 0 0\n4 0 0 0 0\n4 0 0 0 0\n";
	myfile << nbrlines << " " << dist << "\n";
	loadDMRIFibersInFile( myfile);

    myfile.close();
}

string Fibers::intToString( const int number )
{
    stringstream out;
    out << number;
    return out.str();
}

void Fibers::toggleEndianess()
{
    Logger::getInstance()->print( wxT( "Toggle Endianess" ), LOGLEVEL_MESSAGE );
    wxUint8 temp = 0;
    wxUint8 *pPointBytes = ( wxUint8 * )&m_pointArray[0];

    for( int i = 0; i < m_countPoints * 12; i += 4 )
    {
        temp = pPointBytes[i];
        pPointBytes[i] = pPointBytes[i + 3];
        pPointBytes[i + 3] = temp;
        temp = pPointBytes[i + 1];
        pPointBytes[i + 1] = pPointBytes[i + 2];
        pPointBytes[i + 2] = temp;
    }

    // Toggle endianess for the line array.
    wxUint8 *pLineBytes = ( wxUint8 * )&m_lineArray[0];

    for( size_t i = 0; i < m_lineArray.size() * 4; i += 4 )
    {
        temp = pLineBytes[i];
        pLineBytes[i] = pLineBytes[i + 3];
        pLineBytes[i + 3] = temp;
        temp = pLineBytes[i + 1];
        pLineBytes[i + 1] = pLineBytes[i + 2];
        pLineBytes[i + 2] = temp;
    }
}

int Fibers::getPointsPerLine( const int lineId )
{
    return ( m_linePointers[lineId + 1] - m_linePointers[lineId] );
}

int Fibers::getStartIndexForLine( const int lineId )
{
    return m_linePointers[lineId];
}

int Fibers::getLineForPoint( const int pointIdx )
{
    return m_reverse[pointIdx];
}

void Fibers::calculateLinePointers()
{
    Logger::getInstance()->print( wxT( "Calculate line pointers" ), LOGLEVEL_MESSAGE );
    int pc = 0;
    int lc = 0;
    int tc = 0;

    for( int i = 0; i < m_countLines; ++i )
    {
        m_linePointers[i] = tc;
        lc = m_lineArray[pc];
        tc += lc;
        pc += ( lc + 1 );
    }

    lc = 0;
    pc = 0;

    for( int i = 0; i < m_countPoints; ++i )
    {
        if( i == m_linePointers[lc + 1] )
        {
            ++lc;
        }

        m_reverse[i] = lc;
    }
}

void Fibers::createColorArray( const bool colorsLoadedFromFile )
{
    Logger::getInstance()->print( wxT( "Create color arrays" ), LOGLEVEL_MESSAGE );

    if( !colorsLoadedFromFile )
    {
        m_colorArray.clear();
        m_colorArray.resize( m_countPoints * 3 );
    }

    m_normalArray.clear();
    m_normalArray.resize( m_countPoints * 3 );
    int   pc = 0;
    
    float x1, x2, y1, y2, z1, z2 = 0.0f;
    float r, g, b, rr, gg, bb    = 0.0f;
    float lastX, lastY, lastZ          = 0.0f;

    for( int i = 0; i < getLineCount(); ++i )
    {
        x1 = m_pointArray[pc];
        y1 = m_pointArray[pc + 1];
        z1 = m_pointArray[pc + 2];
        x2 = m_pointArray[pc + getPointsPerLine( i ) * 3 - 3];
        y2 = m_pointArray[pc + getPointsPerLine( i ) * 3 - 2];
        z2 = m_pointArray[pc + getPointsPerLine( i ) * 3 - 1];
        r = ( x1 ) - ( x2 );
        g = ( y1 ) - ( y2 );
        b = ( z1 ) - ( z2 );

        if( r < 0.0 )
        {
            r *= -1.0;
        }

        if( g < 0.0 )
        {
            g *= -1.0;
        }

        if( b < 0.0 )
        {
            b *= -1.0;
        }

        float norm = sqrt( r * r + g * g + b * b );
        r *= 1.0 / norm;
        g *= 1.0 / norm;
        b *= 1.0 / norm;
        
        lastX = m_pointArray[pc]     + ( m_pointArray[pc]     - m_pointArray[pc + 3] );
        lastY = m_pointArray[pc + 1] + ( m_pointArray[pc + 1] - m_pointArray[pc + 4] );
        lastZ = m_pointArray[pc + 2] + ( m_pointArray[pc + 2] - m_pointArray[pc + 5] );

        for( int j = 0; j < getPointsPerLine( i ); ++j )
        {
            rr = lastX - m_pointArray[pc];
            gg = lastY - m_pointArray[pc + 1];
            bb = lastZ - m_pointArray[pc + 2];
            lastX = m_pointArray[pc];
            lastY = m_pointArray[pc + 1];
            lastZ = m_pointArray[pc + 2];

            if( rr < 0.0 )
            {
                rr *= -1.0;
            }

            if( gg < 0.0 )
            {
                gg *= -1.0;
            }

            if( bb < 0.0 )
            {
                bb *= -1.0;
            }

            float norm = sqrt( rr * rr + gg * gg + bb * bb );
            rr *= 1.0 / norm;
            gg *= 1.0 / norm;
            bb *= 1.0 / norm;
            m_normalArray[pc]     = rr;
            m_normalArray[pc + 1] = gg;
            m_normalArray[pc + 2] = bb;

            if( ! colorsLoadedFromFile )
            {
                m_colorArray[pc]     = r;
                m_colorArray[pc + 1] = g;
                m_colorArray[pc + 2] = b;
            }

            pc += 3;
        }
    }
}

void Fibers::resetColorArray()
{
    Logger::getInstance()->print( wxT( "Reset color arrays" ), LOGLEVEL_MESSAGE );
    float *pColorData( NULL );
    float *pColorData2( NULL );
	
	if( SceneManager::getInstance()->isUsingVBO() )
    {
		glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
		pColorData = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
		pColorData2 = &m_colorArray[0];
    }
    else
    {
        pColorData  = &m_colorArray[0];
        pColorData2 = &m_colorArray[0];
    }

    int pc = 0;
    float r, g, b, x1, x2, y1, y2, z1, z2, lastX, lastY, lastZ = 0.0f;

    for( int i = 0; i < getLineCount(); ++i )
    {
        x1 = m_pointArray[pc];
        y1 = m_pointArray[pc + 1];
        z1 = m_pointArray[pc + 2];
        x2 = m_pointArray[pc + getPointsPerLine( i ) * 3 - 3];
        y2 = m_pointArray[pc + getPointsPerLine( i ) * 3 - 2];
        z2 = m_pointArray[pc + getPointsPerLine( i ) * 3 - 1];
        r = ( x1 ) - ( x2 );
        g = ( y1 ) - ( y2 );
        b = ( z1 ) - ( z2 );

        if( r < 0.0 )
        {
            r *= -1.0;
        }

        if( g < 0.0 )
        {
            g *= -1.0;
        }

        if( b < 0.0 )
        {
            b *= -1.0;
        }

        float norm = sqrt( r * r + g * g + b * b );
        r *= 1.0 / norm;
        g *= 1.0 / norm;
        b *= 1.0 / norm;
        
        lastX = m_pointArray[pc] + ( m_pointArray[pc] - m_pointArray[pc + 3] );
        lastY = m_pointArray[pc + 1] + ( m_pointArray[pc + 1] - m_pointArray[pc + 4] );
        lastZ = m_pointArray[pc + 2] + ( m_pointArray[pc + 2] - m_pointArray[pc + 5] );

        for( int j = 0; j < getPointsPerLine( i ); ++j )
        {
            pColorData[pc] = r;
            pColorData[pc + 1] = g;
            pColorData[pc + 2] = b;
            pColorData2[pc] = r;
            pColorData2[pc + 1] = g;
            pColorData2[pc + 2] = b;
            pc += 3;
        }
    }

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glUnmapBuffer( GL_ARRAY_BUFFER );
    }

    m_fiberColorationMode = NORMAL_COLOR;
}


void Fibers::resetLinesShown()
{
    m_selected.assign( m_countLines, false );
}

void Fibers::updateLinesShown()
{
    SelectionObjList selectionObjects = SceneManager::getInstance()->getSelectionObjects();

    m_selected.assign( m_countLines, true );

    int activeCount = 0;

    //First pass to make sure there is at least one intersection volume active;
    for( unsigned int i = 0; i < selectionObjects.size(); ++i )
    {
        if( selectionObjects[i][0]->getIsActive() )
        {
            activeCount++;

            for( unsigned int j = 1; j < selectionObjects[i].size(); ++j )
            {
                if( selectionObjects[i][j]->getIsActive() )
                {
                    activeCount++;
                }
            }
        }
    }

    if( activeCount == 0 )
    {
        return;
    }

    // For all the master selection objects.
    for( unsigned int i = 0; i < selectionObjects.size(); ++i )
    {
        if( selectionObjects[i][0]->getIsActive() )
        {
            // NOTE: this is currently commented while waiting for JF's
            // refactoring and JC's selection branch.
            //if( selectionObjects[i][0]->getIsDirty() )
            {
                selectionObjects[i][0]->m_inBox.clear();
                selectionObjects[i][0]->m_inBox.resize( m_countLines );
                selectionObjects[i][0]->m_inBranch.clear();
                selectionObjects[i][0]->m_inBranch.resize( m_countLines );

                // Sets the fibers that are inside this object to true in the m_inBox vector.
                selectionObjects[i][0]->m_inBox = getLinesShown( selectionObjects[i][0] );
            }

            for( int k = 0; k < m_countLines; ++k )
            {
                selectionObjects[i][0]->m_inBranch[k] = selectionObjects[i][0]->m_inBox[k];
            }

            // For all its child box.
            for( unsigned int j = 1; j < selectionObjects[i].size(); ++j )
            {
                if( selectionObjects[i][j]->getIsActive() )
                {
                    // NOTE: this is currently commented while waiting for JF's
                    // refactoring and JC's selection branch.                  
                    //if( selectionObjects[i][j]->getIsDirty() )
                    {
                        selectionObjects[i][j]->m_inBox.clear();
                        selectionObjects[i][j]->m_inBox.resize( m_countLines );
                        
                        // Sets the fibers that are inside this object to true in the m_inBox vector.
                        selectionObjects[i][j]->m_inBox = getLinesShown( selectionObjects[i][j] );
                    }

                    // Sets the fibers that are INSIDE this child object and INSIDE its master to be in branch.
                    if( ! selectionObjects[i][j]->getIsNOT() )
                    {
                        for( int k = 0; k < m_countLines; ++k )
                        {
                            selectionObjects[i][0]->m_inBranch[k] = selectionObjects[i][0]->m_inBranch[k] & selectionObjects[i][j]->m_inBox[k];
                        }
                    }
                    else // Sets the fibers that are NOT INSIDE this child object and INSIDE its master to be in branch.
                    {
                        for( int k = 0; k < m_countLines; ++k )
                        {
                            selectionObjects[i][0]->m_inBranch[k] = selectionObjects[i][0]->m_inBranch[k] & !selectionObjects[i][j]->m_inBox[k];
                        }
                    }
                }
            }
        }

        if( selectionObjects[i].size() > 0 && selectionObjects[i][0]->isColorChanged() )
        {
			float *pColorData( NULL );
			float *pColorData2( NULL );
			
			if( SceneManager::getInstance()->isUsingVBO() )
			{
				glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
				pColorData = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
				pColorData2 = &m_colorArray[0];
			}
            else
            {
                pColorData  = &m_colorArray[0];
                pColorData2 = &m_colorArray[0];
            }

            wxColour col = selectionObjects[i][0]->getFiberColor();

            for( int l = 0; l < m_countLines; ++l )
            {
                if( selectionObjects[i][0]->m_inBranch[l] )
                {
                    unsigned int pc = getStartIndexForLine( l ) * 3;

                    for( int j = 0; j < getPointsPerLine( l ); ++j )
                    {
                        pColorData[pc]      = ( ( float ) col.Red() )   / 255.0f;
                        pColorData[pc + 1]  = ( ( float ) col.Green() ) / 255.0f;
                        pColorData[pc + 2]  = ( ( float ) col.Blue() )  / 255.0f;
                        pColorData2[pc]     = ( ( float ) col.Red() )   / 255.0f;
                        pColorData2[pc + 1] = ( ( float ) col.Green() ) / 255.0f;
                        pColorData2[pc + 2] = ( ( float ) col.Blue() )  / 255.0f;
                        pc += 3;
                    }
                }
            }

            if( SceneManager::getInstance()->isUsingVBO() )
            {
                glUnmapBuffer( GL_ARRAY_BUFFER );
            }

            selectionObjects[i][0]->setColorChanged( false );
        }
    }

    resetLinesShown();
    bool boxWasUpdated( false );

    for( unsigned int i = 0; i < selectionObjects.size(); ++i )
    {
        if( selectionObjects[i].size() > 0 && selectionObjects[i][0]->getIsActive() )
        {
            for( int k = 0; k < m_countLines; ++k )
            {
                m_selected[k] = m_selected[k] | selectionObjects[i][0]->m_inBranch[k];
            }
        }

        boxWasUpdated = true;
    }

    if( m_fibersInverted )
    {
        for( int k = 0; k < m_countLines; ++k )
        {
            m_selected[k] = ! m_selected[k];
        }
    }

    SelectionObject * pLastSelObj = MyApp::frame->getLastSelectedObj();

    // This is to update the information display in the fiber grid info and the mean fiber
    if( boxWasUpdated && pLastSelObj != NULL )
    {
        pLastSelObj->SetFiberInfoGridValues();
        pLastSelObj->computeMeanFiber();
        pLastSelObj->computeConvexHull();
    }
}

///////////////////////////////////////////////////////////////////////////
// Will return the fibers that are inside the selection object passed in argument.
//
// pSelectionObject        : The selection object to test with.
//
// Return a vector of bool, a value of TRUE indicates that this fiber is inside the selection object passed in argument.
// A value of false, indicate that this fiber is not inside the selection object.
///////////////////////////////////////////////////////////////////////////
vector< bool > Fibers::getLinesShown( SelectionObject *pSelectionObject )
{
    if( ! pSelectionObject->isSelectionObject() && ! pSelectionObject->m_sourceAnatomy )
    {
        return pSelectionObject->m_inBox;
    }

    resetLinesShown();

    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    if( pSelectionObject->getSelectionType() == BOX_TYPE || pSelectionObject->getSelectionType() == ELLIPSOID_TYPE )
    {
        Vector center = pSelectionObject->getCenter();
        Vector size   = pSelectionObject->getSize();
        m_boxMin.resize( 3 );
        m_boxMax.resize( 3 );
        m_boxMin[0] = center.x - size.x * 0.5 * voxelX;
        m_boxMax[0] = center.x + size.x * 0.5 * voxelX;
        m_boxMin[1] = center.y - size.y * 0.5 * voxelY;
        m_boxMax[1] = center.y + size.y * 0.5 * voxelY;
        m_boxMin[2] = center.z - size.z * 0.5 * voxelZ;
        m_boxMax[2] = center.z + size.z * 0.5 * voxelZ;

        //Get and Set selected lines to visible
        objectTest( pSelectionObject );
    }
    else
    {
        int columns = DatasetManager::getInstance()->getColumns();
        int rows    = DatasetManager::getInstance()->getRows();
        int frames  = DatasetManager::getInstance()->getFrames();

        for( int i = 0; i < m_countPoints; ++i )
        {
            if( m_selected[getLineForPoint( i )] != 1 )
            {
                int x     = std::min( columns - 1, std::max( 0, (int)( m_pointArray[i * 3 ]    / voxelX ) ) );
                int y     = std::min( rows    - 1, std::max( 0, (int)( m_pointArray[i * 3 + 1] / voxelY ) ) );
                int z     = std::min( frames  - 1, std::max( 0, (int)( m_pointArray[i * 3 + 2] / voxelZ ) ) );
                int index = x + y * columns + z * rows * columns;

                if( ( pSelectionObject->m_sourceAnatomy->at( index ) > pSelectionObject->getThreshold() ) )
                {
                    m_selected[getLineForPoint( i )] = 1;
                }
            }
        }
    }

    return m_selected;
}

///////////////////////////////////////////////////////////////////////////
// Get points that are inside the selection object and
// set selected fibers according to those points.
///////////////////////////////////////////////////////////////////////////
void Fibers::objectTest( SelectionObject *pSelectionObject )
{
    vector<int> pointsInside = m_pOctree->getPointsInside( pSelectionObject ); //Get points inside the selection object
    int indice, id;

    for( unsigned int i = 0; i < pointsInside.size(); i++ )
    {
        indice = pointsInside[i];
        id = m_reverse[indice];//Fiber ID according to current point
        m_selected[id] = 1; //Fiber to be in bundle (TRUE)
    }
}

///////////////////////////////////////////////////////////////////////////
//Fill KdTree
///////////////////////////////////////////////////////////////////////////
void Fibers::generateKdTree()
{
    m_pKdTree = new KdTree( m_countPoints, &m_pointArray[0], m_dh );
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
//
// i_point      :
///////////////////////////////////////////////////////////////////////////
bool Fibers::getBarycenter( SplinePoint *pPoint )
{
    // Number of fibers needed to keep a i_point.
    int threshold = 20;
    
    // Multiplier for moving the i_point towards the barycenter.
    m_boxMin.resize( 3 );
    m_boxMax.resize( 3 );
    m_boxMin[0] = pPoint->X() - 25.0 / 2;
    m_boxMax[0] = pPoint->X() + 25.0 / 2;
    m_boxMin[1] = pPoint->Y() - 5.0 / 2;
    m_boxMax[1] = pPoint->Y() + 5.0 / 2;
    m_boxMin[2] = pPoint->Z() - 5.0 / 2;
    m_boxMax[2] = pPoint->Z() + 5.0 / 2;
    m_barycenter.x = m_barycenter.y = m_barycenter.z = m_count = 0;
    barycenterTest( 0, m_countPoints - 1, 0 );

    if( m_count > threshold )
    {
        m_barycenter.x /= m_count;
        m_barycenter.y /= m_count;
        m_barycenter.z /= m_count;
        float x1 = ( m_barycenter.x - pPoint->X() );
        float y1 = ( m_barycenter.y - pPoint->Y() );
        float z1 = ( m_barycenter.z - pPoint->Z() );

        Vector vector( x1, y1, z1 );
        pPoint->setOffsetVector( vector );
        pPoint->setX( pPoint->X() + x1 );
        pPoint->setY( pPoint->Y() + y1 );
        pPoint->setZ( pPoint->Z() + z1 );
        return true;
    }
    else
    {
        return false;
    }
}

void Fibers::barycenterTest( int left, int right, int axis )
{
    // Abort condition.
    if( left > right )
    {
        return;
    }

    int root  = left + ( ( right - left ) / 2 );
    int axis1 = ( axis + 1 ) % 3;
    int pointIndex = m_pKdTree->m_tree[root] * 3;

    if( m_pointArray[pointIndex + axis] < m_boxMin[axis] )
    {
        barycenterTest( root + 1, right, axis1 );
    }
    else if( m_pointArray[pointIndex + axis] > m_boxMax[axis] )
    {
        barycenterTest( left, root - 1, axis1 );
    }
    else
    {
        int axis2 = ( axis + 2 ) % 3;

        if( m_selected[getLineForPoint( m_pKdTree->m_tree[root] )] == 1 &&
            m_pointArray[pointIndex + axis1] <= m_boxMax[axis1]    &&
            m_pointArray[pointIndex + axis1] >= m_boxMin[axis1]    &&
            m_pointArray[pointIndex + axis2] <= m_boxMax[axis2]    &&
            m_pointArray[pointIndex + axis2] >= m_boxMin[axis2] )
        {
            m_barycenter[0] += m_pointArray[m_pKdTree->m_tree[root] * 3];
            m_barycenter[1] += m_pointArray[m_pKdTree->m_tree[root] * 3 + 1];
            m_barycenter[2] += m_pointArray[m_pKdTree->m_tree[root] * 3 + 2];
            m_count++;
        }

        barycenterTest( left, root - 1, axis1 );
        barycenterTest( root + 1, right, axis1 );
    }
}

void Fibers::initializeBuffer()
{
    if( m_isInitialized || !SceneManager::getInstance()->isUsingVBO()  )
    {
        return;
    }

    m_isInitialized = true;
    bool isOK = true;
    
    glGenBuffers( 3, m_bufferObjects );
    glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * m_countPoints * 3, &m_pointArray[0], GL_STATIC_DRAW );

    isOK = !Logger::getInstance()->printIfGLError( wxT( "initialize vbo points" ) );
    
    if( isOK )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * m_countPoints * 3, &m_colorArray[0], GL_STATIC_DRAW );

        isOK = !Logger::getInstance()->printIfGLError( wxT( "initialize vbo colors" ) );
    }

    if( isOK )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * m_countPoints * 3, &m_normalArray[0], GL_STATIC_DRAW );

        isOK = !Logger::getInstance()->printIfGLError( wxT( "initialize vbo normals" ) );
    }

    SceneManager::getInstance()->setUsingVBO( isOK );

    if( isOK )
    {
        freeArrays();
    }
    else
    {
        Logger::getInstance()->print( wxT( "Not enough memory on your gfx card. Using vertex arrays." ),            LOGLEVEL_ERROR );
        Logger::getInstance()->print( wxT( "This shouldn't concern you. Perfomance just will be slightly worse." ), LOGLEVEL_ERROR );
        Logger::getInstance()->print( wxT( "Get a better graphics card if you want more juice." ),                  LOGLEVEL_ERROR );
        glDeleteBuffers( 3, m_bufferObjects );
    }
}

void Fibers::draw()
{
    setShader();

    if( m_cachedThreshold != m_threshold )
    {
        updateFibersColors();
        m_cachedThreshold = m_threshold;
    }

    initializeBuffer();

    if( m_useFakeTubes )
    {
        drawFakeTubes();
        return;
    }

    if( m_useTransparency )
    {
        glPushAttrib( GL_ALL_ATTRIB_BITS );
        glEnable( GL_BLEND );
        glBlendFunc( GL_ONE, GL_ONE );
        glDepthMask( GL_FALSE );
        drawSortedLines();
        glPopAttrib();
        return;
    }

    // If geometry shaders are supported, the shader will take care of the filtering
    // Otherwise, use the drawCrossingFibers
    if ( !SceneManager::getInstance()->isFibersGeomShaderActive() && m_useCrossingFibers )
    {
        drawCrossingFibers();
        return;
    }

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_COLOR_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );

    if( !SceneManager::getInstance()->isUsingVBO() )
    {
        glVertexPointer( 3, GL_FLOAT, 0, &m_pointArray[0] );

        if( m_showFS )
        {
            glColorPointer( 3, GL_FLOAT, 0, &m_colorArray[0] );  // Global colors.
        }
        else
        {
            glColorPointer( 3, GL_FLOAT, 0, &m_normalArray[0] ); // Local colors.
        }

        glNormalPointer( GL_FLOAT, 0, &m_normalArray[0] );
    }
    else
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[0] );
        glVertexPointer( 3, GL_FLOAT, 0, 0 );

        if( m_showFS )
        {
            glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
            glColorPointer( 3, GL_FLOAT, 0, 0 );
        }
        else
        {
            glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
            glColorPointer( 3, GL_FLOAT, 0, 0 );
        }

        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
        glNormalPointer( GL_FLOAT, 0, 0 );
    }

    for( int i = 0; i < m_countLines; ++i )
    {
        if( ( m_selected[i] || !SceneManager::getInstance()->getActivateAllSelObj() ) && !m_filtered[i] )
        {
            glDrawArrays( GL_LINE_STRIP, getStartIndexForLine( i ), getPointsPerLine( i ) );
        }
    }

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );

    releaseShader();
}

///////////////////////////////////////////////////////////////////////////
// COMMENT
///////////////////////////////////////////////////////////////////////////
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


void Fibers::drawFakeTubes()
{
    if( ! m_normalsPositive )
    {
        switchNormals( false );
    }

    GLfloat *pColors  = NULL;
    GLfloat *pNormals = NULL;
    pColors  = &m_colorArray[0];
    pNormals = &m_normalArray[0];

    if( SceneManager::getInstance()->isPointMode() )
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    else
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    if ( m_useCrossingFibers )
    {
        findCrossingFibers();

        for( unsigned int i = 0; i < m_cfStartOfLine.size(); ++i )
        {
            if ( 3 < m_cfPointsPerLine[i] )
            {
                int index = m_cfStartOfLine[i] * 3;
                glBegin( GL_QUAD_STRIP );

                for( unsigned int k = 0; k < m_cfPointsPerLine[i]; ++k, index += 3 )
                {
                    glNormal3f( m_normalArray[index], m_normalArray[index + 1], m_normalArray[index + 2] );
                    glColor3f( m_colorArray[index],  m_colorArray[index + 1],  m_colorArray[index + 2] );
                    glTexCoord2f( -1.0f, 0.0f );
                    glVertex3f( m_pointArray[index], m_pointArray[index + 1], m_pointArray[index + 2] );
                    glTexCoord2f( 1.0f, 0.0f );
                    glVertex3f( m_pointArray[index], m_pointArray[index + 1], m_pointArray[index + 2] );
                }

                glEnd();
            }
        }
    }
    else
    {
        for( int i = 0; i < m_countLines; ++i )
        {
            if( m_selected[i] && !m_filtered[i] )
            {
                int idx = getStartIndexForLine( i ) * 3;
                glBegin( GL_QUAD_STRIP );

                for( int k = 0; k < getPointsPerLine( i ); ++k )
                {
                    glNormal3f( pNormals[idx], pNormals[idx + 1], pNormals[idx + 2] );
                    glColor3f( pColors[idx],  pColors[idx + 1],  pColors[idx + 2] );
                    glTexCoord2f( -1.0f, 0.0f );
                    glVertex3f( m_pointArray[idx], m_pointArray[idx + 1], m_pointArray[idx + 2] );
                    glTexCoord2f( 1.0f, 0.0f );
                    glVertex3f( m_pointArray[idx], m_pointArray[idx + 1], m_pointArray[idx + 2] );
                    idx += 3;
                }

                glEnd();
            }
        }
    }
}

void Fibers::drawSortedLines()
{
    // Only sort those lines we see.
    unsigned int *pSnippletSort = NULL;
    unsigned int *pLineIds      = NULL;
    
    int nbSnipplets = 0;

    // Estimate memory required for arrays.
    for( int i = 0; i < m_countLines; ++i )
    {
        if( m_selected[i] && !m_filtered[i] )
        {
            nbSnipplets += getPointsPerLine( i ) - 1;
        }
    }

    pSnippletSort = new unsigned int[nbSnipplets + 1]; // +1 just to be sure because of fancy problems with some sort functions.
    pLineIds      = new unsigned int[nbSnipplets * 2];
    // Build data structure for sorting.
    int snp = 0;

    for( int i = 0; i < m_countLines; ++i )
    {
        if( !( m_selected[i] && !m_filtered[i] ) )
        {
            continue;
        }

        const unsigned int p = getPointsPerLine( i );

        // TODO: update pLineIds and pSnippletSort size only when fiber selection changes.
        for( unsigned int k = 0; k < p - 1; ++k )
        {
            pLineIds[snp << 1] = getStartIndexForLine( i ) + k;
            pLineIds[( snp << 1 ) + 1] = getStartIndexForLine( i ) + k + 1;
            pSnippletSort[snp] = snp;
            snp++;
        }
    }

    GLfloat projMatrix[16];
    glGetFloatv( GL_PROJECTION_MATRIX, projMatrix );
    
    // Compute z values of lines (in our case: starting points only).
    vector< float > zVals( nbSnipplets );

    for( int i = 0; i < nbSnipplets; ++i )
    {
        const int id = pLineIds[i << 1] * 3;
        zVals[i] = ( m_pointArray[id + 0] * projMatrix[2] + m_pointArray[id + 1] * projMatrix[6]
                      + m_pointArray[id + 2] * projMatrix[10] + projMatrix[14] ) / ( m_pointArray[id + 0] * projMatrix[3]
                              + m_pointArray[id + 1] * projMatrix[7] + m_pointArray[id + 2] * projMatrix[11] + projMatrix[15] );
    }

    sort( &pSnippletSort[0], &pSnippletSort[nbSnipplets], IndirectComp< vector< float > > ( zVals ) );
    
    float *pColors  = NULL;
    float *pNormals = NULL;

    if( SceneManager::getInstance()->isUsingVBO() )
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
        pColors = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_ONLY );
        glUnmapBuffer( GL_ARRAY_BUFFER );
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
        pNormals = ( float * ) glMapBuffer( GL_ARRAY_BUFFER, GL_READ_ONLY );
        glUnmapBuffer( GL_ARRAY_BUFFER );
    }
    else
    {
        pColors  = &m_colorArray[0];
        pNormals = &m_normalArray[0];
    }

    if( SceneManager::getInstance()->isPointMode() )
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    else
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glBegin( GL_LINES );

    if( m_fiberColorationMode == MINDISTANCE_COLOR )
    {
        int i = 0;

        for( int c = 0; c < nbSnipplets; ++c )
        {
            i = c;
            int idx  = pLineIds[pSnippletSort[i] << 1];
            int idx3 = idx * 3;
            int id2  = pLineIds[( pSnippletSort[i] << 1 ) + 1];
            int id23 = id2 * 3;
            glColor4f(  pColors[idx3 + 0],       pColors[idx3 + 1],       pColors[idx3 + 2],   m_localizedAlpha[idx] * m_alpha );
            glNormal3f( pNormals[idx3 + 0],      pNormals[idx3 + 1],      pNormals[idx3 + 2] );
            glVertex3f( m_pointArray[idx3 + 0],  m_pointArray[idx3 + 1],  m_pointArray[idx3 + 2] );
            glColor4f(  pColors[id23 + 0],       pColors[id23 + 1],       pColors[id23 + 2],   m_localizedAlpha[id2] * m_alpha );
            glNormal3f( pNormals[id23 + 0],      pNormals[id23 + 1],      pNormals[id23 + 2] );
            glVertex3f( m_pointArray[id23 + 0],  m_pointArray[id23 + 1],  m_pointArray[id23 + 2] );
        }
    }
    else
    {
        int i = 0;

        for( int c = 0; c < nbSnipplets; ++c )
        {
            i = c;
            int idx  = pLineIds[pSnippletSort[i] << 1];
            int idx3 = idx * 3;
            int id2  = pLineIds[( pSnippletSort[i] << 1 ) + 1];
            int id23 = id2 * 3;
            glColor4f(  pColors[idx3 + 0],       pColors[idx3 + 1],       pColors[idx3 + 2],   m_alpha );
            glNormal3f( pNormals[idx3 + 0],      pNormals[idx3 + 1],      pNormals[idx3 + 2] );
            glVertex3f( m_pointArray[idx3 + 0],  m_pointArray[idx3 + 1],  m_pointArray[idx3 + 2] );
            glColor4f(  pColors[id23 + 0],       pColors[id23 + 1],       pColors[id23 + 2],   m_alpha );
            glNormal3f( pNormals[id23 + 0],      pNormals[id23 + 1],      pNormals[id23 + 2] );
            glVertex3f( m_pointArray[id23 + 0],  m_pointArray[id23 + 1],  m_pointArray[id23 + 2] );
        }
    }

    glEnd();
    glDisable( GL_BLEND );
    
    // FIXME: store these later on!
    delete[] pSnippletSort;
    delete[] pLineIds;
}

void Fibers::useFakeTubes()
{
	m_useFakeTubes = ! m_useFakeTubes;
	switchNormals( m_useFakeTubes );
}

void Fibers::useTransparency()
{
	m_useTransparency = ! m_useTransparency;
}

void Fibers::drawCrossingFibers()
{
    findCrossingFibers();

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_COLOR_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );

    if( !SceneManager::getInstance()->isUsingVBO() )
    {
        glVertexPointer( 3, GL_FLOAT, 0, &m_pointArray[0] );

        if( m_showFS )
        {
            glColorPointer( 3, GL_FLOAT, 0, &m_colorArray[0] );  // Global colors.
        }
        else
        {
            glColorPointer( 3, GL_FLOAT, 0, &m_normalArray[0] ); // Local colors.
        }

        glNormalPointer( GL_FLOAT, 0, &m_normalArray[0] );
    }
    else
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[0] );
        glVertexPointer( 3, GL_FLOAT, 0, 0 );

        if( m_showFS )
        {
            glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[1] );
            glColorPointer( 3, GL_FLOAT, 0, 0 );
        }
        else
        {
            glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
            glColorPointer( 3, GL_FLOAT, 0, 0 );
        }

        glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[2] );
        glNormalPointer( GL_FLOAT, 0, 0 );
    }

    for( unsigned int i = 0; i < m_cfStartOfLine.size(); ++i )
    {
        if ( 1 < m_cfPointsPerLine[i] )
        {
            glDrawArrays( GL_LINE_STRIP, m_cfStartOfLine[i], m_cfPointsPerLine[i] );
        }
    }

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
}

void Fibers::switchNormals( bool positive )
{
    float *pNormals = NULL;
    pNormals = &m_normalArray[0];

    if( positive )
    {
        int pc = 0;
        float rr, gg, bb, lastX, lastY, lastZ = 0.0f;

        for( int i = 0; i < getLineCount(); ++i )
        {
            lastX = m_pointArray[pc] + ( m_pointArray[pc] - m_pointArray[pc + 3] );
            lastY = m_pointArray[pc + 1] + ( m_pointArray[pc + 1] - m_pointArray[pc + 4] );
            lastZ = m_pointArray[pc + 2] + ( m_pointArray[pc + 2] - m_pointArray[pc + 5] );

            for( int j = 0; j < getPointsPerLine( i ); ++j )
            {
                rr = lastX - m_pointArray[pc];
                gg = lastY - m_pointArray[pc + 1];
                bb = lastZ - m_pointArray[pc + 2];
                lastX = m_pointArray[pc];
                lastY = m_pointArray[pc + 1];
                lastZ = m_pointArray[pc + 2];

                if( rr < 0.0 )
                {
                    rr *= -1.0;
                }

                if( gg < 0.0 )
                {
                    gg *= -1.0;
                }

                if( bb < 0.0 )
                {
                    bb *= -1.0;
                }

                float norm = sqrt( rr * rr + gg * gg + bb * bb );
                rr *= 1.0 / norm;
                gg *= 1.0 / norm;
                bb *= 1.0 / norm;
                pNormals[pc] = rr;
                pNormals[pc + 1] = gg;
                pNormals[pc + 2] = bb;
                pc += 3;
            }
        }

        m_normalsPositive = true;
    }
    else
    {
        int pc = 0;
        float rr, gg, bb, lastX, lastY, lastZ = 0.0f;

        for( int i = 0; i < getLineCount(); ++i )
        {
            lastX = m_pointArray[pc] + ( m_pointArray[pc] - m_pointArray[pc + 3] );
            lastY = m_pointArray[pc + 1] + ( m_pointArray[pc + 1] - m_pointArray[pc + 4] );
            lastZ = m_pointArray[pc + 2] + ( m_pointArray[pc + 2] - m_pointArray[pc + 5] );

            for( int j = 0; j < getPointsPerLine( i ); ++j )
            {
                rr = lastX - m_pointArray[pc];
                gg = lastY - m_pointArray[pc + 1];
                bb = lastZ - m_pointArray[pc + 2];
                lastX = m_pointArray[pc];
                lastY = m_pointArray[pc + 1];
                lastZ = m_pointArray[pc + 2];
                pNormals[pc] = rr;
                pNormals[pc + 1] = gg;
                pNormals[pc + 2] = bb;
                pc += 3;
            }
        }

        m_normalsPositive = false;
    }
}

void Fibers::freeArrays()
{
    // Disabled for now, due to problems with glMapBuffer.
    //m_colorArray.clear();
    //m_normalArray.clear();
}

float Fibers::getPointValue( int ptIndex )
{
    return m_pointArray[ptIndex];
}

int Fibers::getLineCount()
{
    return m_countLines;
}

int Fibers::getPointCount()
{
    return m_countPoints;
}

bool Fibers::isSelected( int fiberId )
{
    return m_selected[fiberId];
}

float Fibers::getLocalizedAlpha( int index )
{
	return m_localizedAlpha[index];
}

void Fibers::setFibersLength()
{
    m_length.resize( m_countLines, false );
    
    vector< Vector >           currentFiberPoints;
    vector< vector< Vector > > fibersPoints;

    for( int i = 0; i < m_countLines; i++ )
    {
        if( getFiberCoordValues( i, currentFiberPoints ) )
        {
            fibersPoints.push_back( currentFiberPoints );
            currentFiberPoints.clear();
        }
    }

    float dx, dy, dz;
    m_maxLength = 0;
    m_minLength = 1000000;

    float voxelX = DatasetManager::getInstance()->getVoxelX();
    float voxelY = DatasetManager::getInstance()->getVoxelY();
    float voxelZ = DatasetManager::getInstance()->getVoxelZ();

    for( unsigned int j = 0 ; j < fibersPoints.size(); j++ )
    {
        currentFiberPoints = fibersPoints[j];
        m_length[j] = 0;

        for( unsigned int i = 1; i < currentFiberPoints.size(); ++i )
        {
            // The values are in pixel, we need to set them in millimeters using the spacing
            // specified in the anatomy file ( m_datasetHelper->xVoxel... ).
            dx = ( currentFiberPoints[i].x - currentFiberPoints[i - 1].x ) * voxelX;
            dy = ( currentFiberPoints[i].y - currentFiberPoints[i - 1].y ) * voxelY;
            dz = ( currentFiberPoints[i].z - currentFiberPoints[i - 1].z ) * voxelZ;
            FArray currentVector( dx, dy, dz );
            m_length[j] += ( float )currentVector.norm();
        }

        if( m_length[j] > m_maxLength ) m_maxLength = m_length[j];

        if( m_length[j] < m_minLength ) m_minLength = m_length[j];
    }
}

bool Fibers::getFiberCoordValues( int fiberIndex, vector< Vector > &fiberPoints )
{
    int index = getStartIndexForLine( fiberIndex ) * 3;
    Vector point3D;

    for( int i = 0; i < getPointsPerLine( fiberIndex ); ++i )
    {
        point3D.x = getPointValue( index );
        point3D.y = getPointValue( index + 1 );
        point3D.z = getPointValue( index + 2 );
        fiberPoints.push_back( point3D );
        index += 3;
    }

    return true;
}

void Fibers::updateFibersFilters()
{
    m_cfDrawDirty = true;
    int min = m_pSliderFibersFilterMin->GetValue();
    int max = m_pSliderFibersFilterMax->GetValue();
    int subSampling = m_pSliderFibersSampling->GetValue();
    int maxSubSampling = m_pSliderFibersSampling->GetMax() + 1;

	updateFibersFilters(min, max, subSampling, maxSubSampling);
}

void Fibers::updateFibersFilters(int minLength, int maxLength, int minSubsampling, int maxSubsampling)
{
	for( int i = 0; i < m_countLines; ++i )
    {
        m_filtered[i] = !( ( i % maxSubsampling ) >= minSubsampling && m_length[i] >= minLength && m_length[i] <= maxLength );
    }
    
    SelectionObject * pLastSelObj = MyApp::frame->getLastSelectedObj();

    //Update stats, mean fiber and convexhull only if an object is selected.
    if( pLastSelObj != NULL )
    {
        pLastSelObj->SetFiberInfoGridValues();
        pLastSelObj->computeMeanFiber();
        pLastSelObj->computeConvexHull();
    }

}

vector< bool > Fibers::getFilteredFibers()
{
    return m_filtered;
}

void Fibers::flipAxis( AxisType i_axe )
{
    unsigned int i = 0;

    switch ( i_axe )
    {
        case X_AXIS:
            i = 0;
            m_pOctree->flipX();
            break;
        case Y_AXIS:
            i = 1;
            m_pOctree->flipY();
            break;
        case Z_AXIS:
            i = 2;
            m_pOctree->flipZ();
            break;
        default:
            Logger::getInstance()->print( wxT( "Cannot flip fibers. The specified axis is undefined" ), LOGLEVEL_ERROR );
            return; //No axis specified - Cannot flip
    }

    //Computing mesh center for the given axis
    float center;
    float maxVal= -9999999;
    float minVal = 9999999;
    for ( unsigned int j(i); j < m_pointArray.size(); j += 3 )
    {
        minVal = min( m_pointArray[j], minVal );
        maxVal = max( m_pointArray[j], maxVal );
    }
    center = ( minVal + maxVal ) / 2; 

    //Translate mesh at origin, flip it and move it back;
    for ( ; i < m_pointArray.size(); i += 3 )
    {
        m_pointArray[i] = -( m_pointArray[i] - center ) + center;
    }

    glBindBuffer( GL_ARRAY_BUFFER, m_bufferObjects[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * m_countPoints * 3, &m_pointArray[0], GL_STATIC_DRAW );

    SceneManager::getInstance()->updateAllSelectionObjects();
}

void Fibers::createPropertiesSizer( PropertiesWindow *pParent )
{
    setFibersLength();
    DatasetInfo::createPropertiesSizer( pParent );
    
    wxSizer *pSizer;
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( new wxStaticText( pParent, wxID_ANY , wxT( "Min Length" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_CENTRE ), 0, wxALIGN_CENTER );
    
    m_pSliderFibersFilterMin = new wxSlider( pParent, wxID_ANY, getMinFibersLength(), getMinFibersLength(), getMaxFibersLength(), wxDefaultPosition, wxSize( 140, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    pSizer->Add( m_pSliderFibersFilterMin, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pSliderFibersFilterMin->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( new wxStaticText( pParent, wxID_ANY , wxT( "Max Length" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_CENTRE ), 0, wxALIGN_CENTER );
    m_pSliderFibersFilterMax = new wxSlider( pParent, wxID_ANY, getMaxFibersLength(), getMinFibersLength(), getMaxFibersLength(), wxDefaultPosition, wxSize( 140, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    pSizer->Add( m_pSliderFibersFilterMax, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pSliderFibersFilterMax->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( new wxStaticText( pParent, wxID_ANY , wxT( "Subsampling" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_CENTRE ), 0, wxALIGN_CENTER );
    m_pSliderFibersSampling = new wxSlider( pParent, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxSize( 140, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    pSizer->Add( m_pSliderFibersSampling, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pSliderFibersSampling->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler( PropertiesWindow::OnFibersFilter ) );

    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( new wxStaticText( pParent, wxID_ANY , wxT( "Thickness" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_CENTRE ), 0, wxALIGN_CENTER );
    m_pSliderCrossingFibersThickness = new wxSlider( pParent, wxID_ANY, m_thickness * 4, 1, 20, wxDefaultPosition, wxSize( 140, -1 ), wxSL_HORIZONTAL | wxSL_AUTOTICKS );
    pSizer->Add( m_pSliderCrossingFibersThickness, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pSliderCrossingFibersThickness->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler( PropertiesWindow::OnCrossingFibersThicknessChange ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    m_pGeneratesFibersDensityVolume = new wxButton( pParent, wxID_ANY, wxT( "New Density Volume" ), wxDefaultPosition, wxSize( 145, -1 ) );
    pSizer->Add( m_pGeneratesFibersDensityVolume, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pGeneratesFibersDensityVolume->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnGenerateFiberVolume ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    m_pToggleLocalColoring = new wxToggleButton( pParent, wxID_ANY, wxT( "Local Coloring" ), wxDefaultPosition, wxSize( 145, -1 ) );
    pSizer->Add( m_pToggleLocalColoring, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pToggleLocalColoring->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( PropertiesWindow::OnToggleUseTex ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    m_pToggleNormalColoring = new wxToggleButton( pParent, wxID_ANY, wxT( "Color With Overlay" ), wxDefaultPosition, wxSize( 145, -1 ) );
    pSizer->Add( m_pToggleNormalColoring, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pToggleNormalColoring->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleShowFS ) );

    pSizer = new wxBoxSizer( wxHORIZONTAL );
    m_pToggleCrossingFibers = new wxToggleButton( pParent, wxID_ANY, wxT( "Intersected Fibers" ), wxDefaultPosition, wxSize( 140, -1 ) );
    pSizer->Add( m_pToggleCrossingFibers, 0, wxALIGN_CENTER );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pToggleCrossingFibers->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxEventHandler( PropertiesWindow::OnToggleCrossingFibers ) );

    m_propertiesSizer->AddSpacer( 8 );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( new wxStaticText( pParent, wxID_ANY, _T( "Coloring" ), wxDefaultPosition, wxSize( 60, -1 ), wxALIGN_RIGHT ), 0, wxALIGN_CENTER );
    pSizer->Add( 8, 1, 0 );
    m_pRadioNormalColoring = new wxRadioButton( pParent, wxID_ANY, _T( "Normal" ), wxDefaultPosition, wxSize( 145, -1 ) );
    pSizer->Add( m_pRadioNormalColoring );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    m_pRadioDistanceAnchoring  = new wxRadioButton( pParent, wxID_ANY, _T( "Dist. Anchoring" ), wxDefaultPosition, wxSize( 145, -1 ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( 68, 1, 0 );
    pSizer->Add( m_pRadioDistanceAnchoring );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    m_pRadioMinDistanceAnchoring  = new wxRadioButton( pParent, wxID_ANY, _T( "Min Dist. Anchoring" ), wxDefaultPosition, wxSize( 145, -1 ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( 68, 1, 0 );
    pSizer->Add( m_pRadioMinDistanceAnchoring );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    m_pRadioCurvature  = new wxRadioButton( pParent, wxID_ANY, _T( "Curvature" ), wxDefaultPosition, wxSize( 145, -1 ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( 68, 1, 0 );
    pSizer->Add( m_pRadioCurvature );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    m_pRadioTorsion  = new wxRadioButton( pParent, wxID_ANY, _T( "Torsion" ), wxDefaultPosition, wxSize( 145, -1 ) );
    
    pSizer = new wxBoxSizer( wxHORIZONTAL );
    pSizer->Add( 68, 1, 0 );
    pSizer->Add( m_pRadioTorsion );
    m_propertiesSizer->Add( pSizer, 0, wxALIGN_CENTER );
    pParent->Connect( m_pRadioNormalColoring->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnNormalColoring ) );
    pParent->Connect( m_pRadioDistanceAnchoring->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnListMenuDistance ) );
    pParent->Connect( m_pRadioMinDistanceAnchoring->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnListMenuMinDistance ) );
    pParent->Connect( m_pRadioTorsion->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnColorWithTorsion ) );
    pParent->Connect( m_pRadioCurvature->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PropertiesWindow::OnColorWithCurvature ) );
    m_pRadioNormalColoring->SetValue( true );
}

void Fibers::updatePropertiesSizer()
{
    DatasetInfo::updatePropertiesSizer();
    m_pToggleFiltering->Enable( false );
    m_pToggleFiltering->SetValue( false );
    m_pToggleCrossingFibers->Enable( true );
	m_pToggleCrossingFibers->SetValue( m_useCrossingFibers );
    m_psliderOpacity->SetValue( m_psliderOpacity->GetMin() );
    m_psliderOpacity->Enable( false );
    m_pSliderCrossingFibersThickness->Enable( m_useCrossingFibers );
    m_pToggleNormalColoring->SetValue( !getShowFS() );
    m_pRadioNormalColoring->Enable( getShowFS() );
    m_pRadioCurvature->Enable( getShowFS() );
    m_pRadioDistanceAnchoring->Enable( getShowFS() );
    m_pRadioMinDistanceAnchoring->Enable( getShowFS() );
    m_pRadioTorsion->Enable( getShowFS() );
	m_psliderThresholdIntensity->SetValue( getThreshold()*100 );
	m_psliderOpacity->SetValue( getAlpha()*100 );

	// Hide temporarily opacity functionality
	m_psliderOpacity->Hide();
	m_pOpacityText->Hide();
	
	if(m_isColorationUpdated)
	{
		m_pRadioNormalColoring->SetValue( m_fiberColorationMode == NORMAL_COLOR );
		m_pRadioCurvature->SetValue( m_fiberColorationMode == CURVATURE_COLOR );
		m_pRadioDistanceAnchoring->SetValue( m_fiberColorationMode == DISTANCE_COLOR );
		m_pRadioMinDistanceAnchoring->SetValue( m_fiberColorationMode == MINDISTANCE_COLOR );
		m_pRadioTorsion->SetValue( m_fiberColorationMode == TORSION_COLOR );
		m_isColorationUpdated = false;
	}
	
	DatasetInfo* pDatasetInfo = NULL;
	
	long nextItemId = MyApp::frame->getCurrentListItem();

	if( nextItemId >= 0)
	{
		pDatasetInfo = DatasetManager::getInstance()->getDataset( MyApp::frame->m_pListCtrl2->GetItem( nextItemId ) );
		if( pDatasetInfo != NULL)
		{
			if(pDatasetInfo->getType() != FIBERS)
			{
				DatasetInfo::m_pbtnDown->Disable();
			}
			else
			{
				DatasetInfo::m_pbtnDown->Enable();
			}
		}
		else
		{
			DatasetInfo::m_pbtnDown->Disable();
		}
	}
	else
	{
		DatasetInfo::m_pbtnDown->Disable();
	}
	
	long prevItemId = MyApp::frame->getCurrentListItem() - 1;
	
	if( prevItemId != -1)
	{
		pDatasetInfo = DatasetManager::getInstance()->getDataset( MyApp::frame->m_pListCtrl2->GetItem( prevItemId ) );
		if( pDatasetInfo != NULL)
		{
			if(pDatasetInfo->getType() != FIBERS)
			{
				DatasetInfo::m_pbtnUp->Disable();
			}
			else
			{
				DatasetInfo::m_pbtnUp->Enable();
			}
		}
		else
		{
			DatasetInfo::m_pbtnUp->Disable();
		}
	}
	
}

//////////////////////////////////////////////////////////////////////////

void Fibers::updateCrossingFibersThickness() 
{
    if ( NULL != m_pSliderCrossingFibersThickness )
    {
        m_thickness = m_pSliderCrossingFibersThickness->GetValue() * 0.25f; 
        m_cfDrawDirty = true;
    }
}

//////////////////////////////////////////////////////////////////////////

void Fibers::findCrossingFibers() 
{
    if (   m_cfDrawDirty
        || m_xDrawn != SceneManager::getInstance()->getSliceX()
        || m_yDrawn != SceneManager::getInstance()->getSliceY()
        || m_zDrawn != SceneManager::getInstance()->getSliceZ()
        || m_axialShown    != SceneManager::getInstance()->isAxialDisplayed()
        || m_coronalShown  != SceneManager::getInstance()->isCoronalDisplayed()
        || m_sagittalShown != SceneManager::getInstance()->isSagittalDisplayed() )
    {
        m_xDrawn = SceneManager::getInstance()->getSliceX();
        m_yDrawn = SceneManager::getInstance()->getSliceY();
        m_zDrawn = SceneManager::getInstance()->getSliceZ();
        m_axialShown    = SceneManager::getInstance()->isAxialDisplayed();
        m_coronalShown  = SceneManager::getInstance()->isCoronalDisplayed();
        m_sagittalShown = SceneManager::getInstance()->isSagittalDisplayed();

        m_cfDrawDirty = true;

        // Determine X, Y and Z range
        const float xMin( m_xDrawn + 0.5f - m_thickness );
        const float xMax( m_xDrawn + 0.5f + m_thickness );
        const float yMin( m_yDrawn + 0.5f - m_thickness );
        const float yMax( m_yDrawn + 0.5f + m_thickness );
        const float zMin( m_zDrawn + 0.5f - m_thickness );
        const float zMax( m_zDrawn + 0.5f + m_thickness );

        bool lineStarted(false);

        m_cfStartOfLine.clear();
        m_cfPointsPerLine.clear();

        unsigned int index( 0 );
        unsigned int point( 0 );
        for ( unsigned int line( 0 ); line < static_cast<unsigned int>(m_countLines); ++line )
        {
            if ( m_selected[line] && !m_filtered[line] )
            {
                for ( unsigned int i( 0 ); i < static_cast<unsigned int>(getPointsPerLine(line)); ++i, ++point, index += 3 )
                {
                    if ( m_sagittalShown && xMin <= m_pointArray[index] && xMax >= m_pointArray[index] )
                    {
                        if ( !lineStarted )
                        {
                            m_cfStartOfLine.push_back(point);
                            m_cfPointsPerLine.push_back(0);
                            lineStarted = true;
                        }
                        ++m_cfPointsPerLine.back();
                    }
                    else if ( m_coronalShown && yMin <= m_pointArray[index + 1] && yMax >= m_pointArray[index + 1] )
                    {
                        if ( !lineStarted )
                        {
                            m_cfStartOfLine.push_back(point);
                            m_cfPointsPerLine.push_back(0);
                            lineStarted = true;
                        }
                        ++m_cfPointsPerLine.back();
                    }
                    else if ( m_axialShown && zMin <= m_pointArray[index + 2] && zMax >= m_pointArray[index + 2] )
                    {
                        if ( !lineStarted )
                        {
                            m_cfStartOfLine.push_back(point);
                            m_cfPointsPerLine.push_back(0);
                            lineStarted = true;
                        }
                        ++m_cfPointsPerLine.back();
                    }
                    else
                    {
                        lineStarted = false;
                    }
                }
                lineStarted = false;
            }
            else
            {
                point += getPointsPerLine(line);
                index += getPointsPerLine(line) * 3;
            }
        }
    }
}

void Fibers::setShader()
{
    DatasetInfo *pDsInfo = (DatasetInfo*) this;

    if( m_useFakeTubes )
    {
        ShaderHelper::getInstance()->getFakeTubesShader()->bind();
        ShaderHelper::getInstance()->getFakeTubesShader()->setUniInt  ( "globalColor", getShowFS() );
        ShaderHelper::getInstance()->getFakeTubesShader()->setUniFloat( "dimX", (float)MyApp::frame->m_pMainGL->GetSize().x );
        ShaderHelper::getInstance()->getFakeTubesShader()->setUniFloat( "dimY", (float)MyApp::frame->m_pMainGL->GetSize().y );
        ShaderHelper::getInstance()->getFakeTubesShader()->setUniFloat( "thickness", GLfloat( 3.175 ) );
    }
    else if( SceneManager::getInstance()->isFibersGeomShaderActive() && m_useCrossingFibers )
    {
        // Determine X, Y and Z range
        const float xMin( SceneManager::getInstance()->getSliceX() + 0.5f - m_thickness );
        const float xMax( SceneManager::getInstance()->getSliceX() + 0.5f + m_thickness );
        const float yMin( SceneManager::getInstance()->getSliceY() + 0.5f - m_thickness );
        const float yMax( SceneManager::getInstance()->getSliceY() + 0.5f + m_thickness );
        const float zMin( SceneManager::getInstance()->getSliceZ() + 0.5f - m_thickness );
        const float zMax( SceneManager::getInstance()->getSliceZ() + 0.5f + m_thickness );

        ShaderHelper::getInstance()->getCrossingFibersShader()->bind();

        ShaderHelper::getInstance()->getCrossingFibersShader()->setUniFloat("xMin", SceneManager::getInstance()->isSagittalDisplayed() ? xMin : 0 );
        ShaderHelper::getInstance()->getCrossingFibersShader()->setUniFloat("xMax", SceneManager::getInstance()->isSagittalDisplayed() ? xMax : 0 );
		
        ShaderHelper::getInstance()->getCrossingFibersShader()->setUniFloat("yMin", SceneManager::getInstance()->isCoronalDisplayed() ? yMin : 0 );
        ShaderHelper::getInstance()->getCrossingFibersShader()->setUniFloat("yMax", SceneManager::getInstance()->isCoronalDisplayed() ? yMax : 0 );

        ShaderHelper::getInstance()->getCrossingFibersShader()->setUniFloat("zMin", SceneManager::getInstance()->isAxialDisplayed() ? zMin : 0 );
        ShaderHelper::getInstance()->getCrossingFibersShader()->setUniFloat("zMax", SceneManager::getInstance()->isAxialDisplayed() ? zMax : 0 );
    }
    else if ( !m_useTex )
    {
        ShaderHelper::getInstance()->getFibersShader()->bind();
        ShaderHelper::getInstance()->setFiberShaderVars();
        ShaderHelper::getInstance()->getFibersShader()->setUniInt( "useTex", !pDsInfo->getUseTex() );
        ShaderHelper::getInstance()->getFibersShader()->setUniInt( "useColorMap", SceneManager::getInstance()->getColorMap() );
        ShaderHelper::getInstance()->getFibersShader()->setUniInt( "useOverlay", pDsInfo->getShowFS() );
    }
}

void Fibers::releaseShader()
{
    if( m_useFakeTubes )
    {
        ShaderHelper::getInstance()->getFakeTubesShader()->release();
    }
    else if( SceneManager::getInstance()->isFibersGeomShaderActive() && m_useCrossingFibers )
    {
        ShaderHelper::getInstance()->getCrossingFibersShader()->release();
    }
    else if( !m_useTex )
    {
        ShaderHelper::getInstance()->getFibersShader()->release();
    }
}
