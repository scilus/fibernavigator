/////////////////////////////////////////////////////////////////////////////
// Name:            Helper.h
// Author:          Imagicien ->LAMIRANDE-NADEAU Julien & NAZRATI Réda<-
// Creation Date:   11/14/2009
//
// Description: Helper class.
//
// Last modifications:
//      by : Imagicien - 12/11/2009
/////////////////////////////////////////////////////////////////////////////

#ifndef HELPER_H_
#define HELPER_H_

#include <vector>
#include "../IsoSurface/Vector.h"

#define EPSILON 0.000001f

///////////////////////////////////////////////////////////////////////////
// Enum representing the different data type in the application.
///////////////////////////////////////////////////////////////////////////
enum DatasetType 
{
    BOT_INITIALIZED = 0,

    // Textures
    HEAD_BYTE,
    HEAD_SHORT,
    OVERLAY,
    RGB,
    TENSOR_FIELD,

    // Everything higher can't be used as a texture.
    MESH,
    VECTORS,
    TENSORS,
    ODFS,
    FIBERS,
    SURFACE,
    ISO_SURFACE,
    MASTER_OBJECT,
    CHILD_OBJECT,
    POINT_DATASET, //POINT is already defined in WinDef.h so we cannot use this name in this enum.
    LABEL_POINTS,
    LABEL_SELECTION_OBJECTS,
    NOT_INITIALIZED
};

///////////////////////////////////////////////////////////////////////////
// Enum representing the different type of selection object that we can create.
///////////////////////////////////////////////////////////////////////////
enum ObjectType
{
    ELLIPSOID_TYPE    = 0,
    BOX_TYPE          = 1,
    CISO_SURFACE_TYPE = 2,
    DEFAULT_TYPE      = 3
};

///////////////////////////////////////////////////////////////////////////
// Enum representing the different LOD level available to draw a glyph.
///////////////////////////////////////////////////////////////////////////
enum LODChoices
{
    LOD_0     = 0, // Lowest LOD.
    LOD_1     = 1,
    LOD_2     = 2,
    LOD_3     = 3,
    LOD_4     = 4,
    LOD_5     = 5,
    LOD_6     = 6,
    NB_OF_LOD = 7
};

///////////////////////////////////////////////////////////////////////////
// Enum representing the 3 x, y, z axis.
///////////////////////////////////////////////////////////////////////////
enum AxisType
{
    X_AXIS          = 0,
    Y_AXIS          = 1,
    Z_AXIS          = 2,
    AXIS_UNDEFINED  = 3
};

///////////////////////////////////////////////////////////////////////////
// Enum representing the different color modifier for a glyph.
///////////////////////////////////////////////////////////////////////////
enum GlyphColorModifier
{
    MIN_HUE    = 0,
    MAX_HUE    = 1,
    SATURATION = 2,
    LUMINANCE  = 3,
};

///////////////////////////////////////////////////////////////////////////
// Enum representing the different type of fiber coloration mode.
///////////////////////////////////////////////////////////////////////////
enum FibersColorationMode
{
    NORMAL_COLOR        = 0,
    CURVATURE_COLOR     = 1,
    TORSION_COLOR       = 2,
    DISTANCE_COLOR      = 3,
    MINDISTANCE_COLOR   = 4,
};

///////////////////////////////////////////////////////////////////////////
// Enum representing the different type of display associated to the cross sections.
///////////////////////////////////////////////////////////////////////////
enum CrossSectionsDisplay
{
    CS_NOTHING       = 0,
    CS_MIN_MAX_ONLY  = 1,
    CS_EVERYTHING    = 2,
    CS_NB_OF_CHOICES = 3
};

///////////////////////////////////////////////////////////////////////////
// Enum representing the different type of display associated to the dispersion cone.
///////////////////////////////////////////////////////////////////////////
enum DispersionConeDisplay
{
    DC_NOTHING        = 0,
    DC_MIN_MAX_ONLY   = 1,
    DC_EVERYTHING     = 2,
    DC_FULL_LINE_CONE = 3,
    DC_FULL_CONE      = 4,
    DC_NB_OF_CHOICES  = 5
};


class Helper  
{  
private:  
    Helper(){};    
    ~Helper(){};

public:
    
    // Color conversion
    static void     HSLtoRGB                 ( float  i_hue, 
                                               float  i_saturation, 
                                               float  i_luminance, 
                                               float &o_r, 
                                               float &o_g, 
                                               float &o_b );

    // Math functions
    static double   calculateCurvature       ( Vector &i_d1, Vector &i_d2 );
    static double   calculateTorsion         ( Vector &i_d1, Vector &i_d2, Vector &i_d3 );
    static void     cartesianToSpherical     ( float i_catesianDir[3], float o_sphericalDir[3] );
    static bool     convert2DPlanePointsTo3D ( const std::vector< Vector > &i_original3Dpoints, 
                                               const std::vector< Vector > &i_2Dpoints, 
                                                     std::vector< Vector > &o_3Dpoints );
    static bool     convert3DPlanePointsTo2D ( const Vector &i_planeNormal, std::vector< Vector > &io_pointsOnThePlane );
    static void     createSphere             ( int                   i_latsFullSphere, 
                                               int                   i_longsFullSphere,
                                               int                   i_latsThisSphere,
                                               int                   i_longsThisSphere,
                                               std::vector< float > &o_spherePoints  );
    static void     getAssociatedLegendrePlm ( const int i_l, const int i_m, const double i_x, double &o_value );
    static void     getCirclePoints          ( const Vector                &i_center, 
                                               const Vector                &i_normal, 
                                                     float                  i_radius, 
                                                     int                    i_nmPoints, 
                                                     std::vector< Vector > &o_circlePoints );
    static bool     getIntersectionPoint     ( const Vector &i_la, 
                                               const Vector &i_lb,
                                               const Vector &i_p0,
                                               const Vector &i_n,
                                                     Vector &o_intersectionPoint );
    static double   getLegendrePlm           ( int i_m, double i_x );
    static float    getFactorial             ( int i_n );
    static double   getFAFromEigenValues     ( double i_eigenValue1, double i_eigenValue2, double i_eigenValue3 ); 

    // Comparaison functions
    template< class T >
    static bool     areEquals( T i_firstArray[], T i_secondArray[], int i_arraysSize );
};

///////////////////////////////////////////////////////////////////////////
// Simple function that will compare the elements in 2 arrays and return if they are equals or not.
// WARNING : The 2 arrays must be of the same size (i_arraysSize) for this function to work correctly.
//
// i_firstArray     : The first array to compare.
// i_secondArray    : The second array to compare.
// i_arraysSize     : The size of both array.
//
// Returns TRUE if all the elements in the 2 arrays are the same, FALSE otherwise.
///////////////////////////////////////////////////////////////////////////
template< class T >
bool Helper::areEquals( T i_firstArray[], T i_secondArray[], int i_arraysSize )
{
    for( int i = 0; i < i_arraysSize; ++i )
        if( i_firstArray[i] != i_secondArray[i] )
            return false;

    return true;
}

#endif // HELPER_H_