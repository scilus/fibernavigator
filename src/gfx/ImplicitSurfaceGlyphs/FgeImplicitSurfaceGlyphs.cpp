//Copyright (c) 2008 Aaron Knoll, Younis Hijazi, Andrew Kensler, Mathias Schott, Charles Hansen and Hans Hagen
//
// edited 2009 by Mario Hlawitschka and Alexander Wiebel
//
//Permission is hereby granted, free of charge, to any person
//obtaining a copy of this software and associated documentation
//files (the "Software"), to deal in the Software without
//restriction, including without limitation the rights to use,
//copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the
//Software is furnished to do so, subject to the following
//conditions:
//
//The above copyright notice and this permission notice shall be
//included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
//OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//OTHER DEALINGS IN THE SOFTWARE.
#include "FgeImplicitSurfaceGlyphs.h"

#include "FgeGL.h"

#include <stdlib.h>
#include <math.h>

#include <iostream>
using namespace std;

#include <sstream>
#include "FgeImplicitSurfaceExpression.h"
#include "FNumberingSystem.h"
#include "FFixArray.hh"

#include <cg.h>
#include <cgGL.h>

#include <sstream>

#include "FgeGLFrustum.h"

template<typename T> std::string to_string( const T& i )
{
    std::ostringstream oss;
    oss << i;
    return oss.str();
}

typedef FFixArray<float, 3> Vector3;
typedef FFixArray<float, 4> Vector4;

#ifndef __UTIL_HXX__
#define __UTIL_HXX__

#include <iostream>
#include <fstream>
#include <sstream>

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#define SWAP(a,b,temp)   ((temp)=(a),(a)=(b),(b)=(temp))

#ifdef WIN32
#define FATAL(a)     { std::cout << "FATAL error: " << a << std::endl; exit(0); }
#else
#define FATAL(a)     { std::cout << "FATAL error in " << __PRETTY_FUNCTION__ << " : " << a << std::endl; exit(0); }
#endif

#define FAIL FATAL

#ifndef NULL
#define NULL 0
#endif

#endif

Vector3 cross( const Vector3 &a, const Vector3 &b )
{
    return crossProduct( a, b );
}

#define WINDOW_SIZE 800
#define MAX_KEYS 256
#define VOLUME_TEX_SIZE 128

#ifndef M_PI
#define M_PI 3.14159265358793
#endif

enum ARITH_TYPE
{
    ARITH_IA = 0, ARITH_RAA_FLOAT3, ARITH_RAA_FLOAT4,
};

enum ISO_TYPE
{
    ISO_SINGLE = 0, ISO_MULTI, ISO_RANGE,
};

using namespace std;

#ifdef WIN32
#define snprintf _snprintf
#define strtof(a,b) (atof((a)))
#endif

char volumeString[256];
char shaderBase[256] = "alex.cg";
char functionStringTmp[512];

string functionString = "x^2 - y^2 - z^2 - .25";
string dfdxString = "2*x";
string dfdyString = "-2*y";
string dfdzString = "-2*z";

float isovalue;
int depth = 10;
float epsilon = .001;
float delta = 0.f;
float old_delta = 0.f;
float translate_delta;
int arith_type = ARITH_IA;
int isovalue_type = ISO_SINGLE;
float iso0 = 0.f;
float iso1 = 0.f;
float iso2 = 0.f;
float iso3 = 0.f;
float iso_incr = 1.f;
float iso_color_offset = 0.f;
float iso_color_scale = 1.f;
int asymptotes = 0;
int animate = 0;
int use_cubemap = 0;//before enabling this, please uncomment stuff marked with ALEXS_OLD_CG_CMAP_COMMENT. But be aware that this still might be not working.
int actualAsymptotes = 1;
int shaderID = 0;
int actualShaderID = 0;
int bumpmapID = 0;
int actualBumpmapID = 0;
int centralDifferences = 1;
int actualCentralDifferences = 1;
float centralDifferencesDelta = 0.001f;
float bumpmapFrequency = 20.f;
float phong_exponent = 128.f;
float nearPlane = .01;
Vector3 front_color( .4, .1, .9 );
Vector3 back_color( .1, .6, .6 );

Vector4 domain_color( 0., 0., 0., 0. );

float shadow_eps_factor = 1.0;
float dp_eps_factor = 10.f;
float dp_alpha = 0.8f;

float var_w = 0.f;
float var_w_min = 0.f;
float var_w_max = 1.f;
float var_dw = 0.01;

float var_r0 = 1.0f;
float var_r1 = 1.0f;
float var_r2 = 1.0f;

const float fovy = 60.f;

float fixedrotate = 20;

int res_w = WINDOW_SIZE;
int res_h = WINDOW_SIZE;
bool gKeys[MAX_KEYS];
//bool toggle_visuals = true;
//bool showFps = true;
//R bool cameraSlowRotate = false;
//R bool cameraSet = false;
bool newfunction = true;
bool newdomain = false;
//R bool newcamera = false;
CGcontext context;
CGprofile vertexProfile, fragmentProfile;
CGparameter param1;
CGparameter param2;
CGparameter param3;
GLuint renderbuffer;
GLuint framebuffer;
CGprogram vertex_main, fragment_main; // the raycasting shader programs
GLuint permutations_x_tex, permutations_y_tex, permutations_z_tex,
                vectors_xyz_tex;
GLuint backface_buffer; // the FBO buffers
GLuint final_image;
GLuint hsv_colors;
#define HSV_SIZE 128
float hsv_buffer[HSV_SIZE][3];
//float stepsize = 1.0/50.0;

//R Camera camera;
//
Vector3 domain_min( -1, -1, -1 );
Vector3 domain_max( 1, 1, 1 );
Vector3 domain_width( 2, 2, 2 );
Vector3 inv_domain_width( .5, .5, .5 );
//Vector3 domain_min( 0, 0, 0 );
//Vector3 domain_max( 1, 1, 1 );
//Vector3 domain_width( 1, 1, 1 );
//Vector3 inv_domain_width( 1, 1, 1 );

//R Vector3 camera_u, camera_v;
float half_pixel_u, half_pixel_v;

GLuint cube_map;

struct ShadeOptions
{
    int eye_light;
    int shadows;
    int depth_peeling;
    int flip_color;
};

ShadeOptions shadeOptions;

//AARONBAD - maybe make a fixed number of point lights?
Vector3 light[4];

//Spline from_path, lookat_path, up_path;
bool path = false; // use a path
FILE* output_path = NULL;
float path_frames = 1.f;

#define FGLSLSHADERDIR "GLSL/"
#define MYSHADERDIR  FGLSLSHADERDIR "ImplicitSurfaceGlyphs/"

std::string getFileName( const std::string & file )
{
    std::ifstream firstGuess( file.c_str() );
    if( firstGuess )
        return file;

    std::string secondFile = std::string( "" ) + MYSHADERDIR + file;
    std::ifstream secondGuess( secondFile.c_str() );
    if( secondGuess )
        return secondFile;
    std::cerr << "Cannot find file " << file << " or " << secondFile
                    << std::endl;
    return "";
}

void cgErrorCallback()
{
    CGerror lastError = cgGetError();
    if( lastError )
    {
        cout << cgGetErrorString( lastError ) << endl;
        if( context != NULL )
            cout << cgGetLastListing( context ) << endl;
        exit( 0 );
    }
}

// load_vertex_program: loading a vertex program
void load_vertex_program( CGprogram &v_program, const char *shader_path,
                          const char *program_name )
{
    assert(cgIsContext(context));
    cgGLSetOptimalOptions( fragmentProfile );
    v_program = cgCreateProgramFromFile( context, CG_SOURCE, shader_path,
                    vertexProfile, program_name, NULL );
    if( !cgIsProgramCompiled( v_program ) )
        cgCompileProgram( v_program );

    cgGLEnableProfile( vertexProfile );
    cgGLLoadProgram( v_program );
    cgGLDisableProfile( vertexProfile );
}

// load_fragment_program: loading a fragment program
void load_fragment_program( CGprogram &f_program, const char *shader_path,
                            const char *program_name )
{
    assert(cgIsContext(context));
    cgGLSetOptimalOptions( fragmentProfile );
    f_program = cgCreateProgramFromFile( context, CG_SOURCE, shader_path,
                    fragmentProfile, program_name, NULL );
    if( !cgIsProgramCompiled( f_program ) )
        cgCompileProgram( f_program );

    cgGLEnableProfile( fragmentProfile );
    cgGLLoadProgram( f_program );
    cgGLDisableProfile( fragmentProfile );
}

inline void FgeImplicitSurfaceGlyphs::build_shader( int control )
{
    //rebuild the shader here.
    char shaderOut[32] = "SHADER_TMP.cg";
    FILE* fin = fopen( getFileName( shaderBase ).c_str(), "rb" );
    if( !fin )
    {
        cerr << "Couldn't open " << shaderBase << " for reading! Aborting."
                        << endl;
        exit( 0 );
    }

    FILE* fout = fopen( shaderOut, "wb" );
    if( !fout )
    {
        cerr << "Couldn't open SHADER_TMP.cg for writing! Aborting." << endl;
        exit( 0 );
    }

    bool use_cgim = false;

    const int LINECHARS = 8192;
    char lineBuffer[LINECHARS];

    bool genIA, genAD, genADIA, genRAFA;
    bool use_cgim_gradient = false;
    genIA = genAD = genADIA = genRAFA = 0;

    genRAFA = (arith_type >= ARITH_RAA_FLOAT3);

    bool use_cgim_reject_test = false;
    string ia_reject_string = "";
    string raf_reject_string = "";
    string traverse_hit_string = "";
    string pretraverse_string = "";
    string eval_gradient_string = "";

    for( ;; )
    {
        if( !fgets( lineBuffer, LINECHARS, fin ) )
            break;

        if( strstr( lineBuffer, "GEN__IA" ) )
            genIA = true;
        else if( strstr( lineBuffer, "GEN__ADIFF" ) )
            genAD = true;
        else if( strstr( lineBuffer, "GEN__ADIA" ) )
            genADIA = true;
        //else if (strstr(lineBuffer, "GEN__RAFA"))
        //  genRAFA = true;
        else if( strstr( lineBuffer, "STUB__CUBEMAP_IFDEF" ) )
        {
            if( use_cubemap )
                fputs( "#if 1\n", fout );
            else
                fputs( "#if 0\n", fout );
        }
        else if( strstr( lineBuffer, "STUB__ARITHMETIC_INCLUDE" ) )
        {
            if( arith_type == ARITH_IA )
                //fputs("\n#include \"" MYSHADERDIR "iarith.cg\"\n", fout);
                fputs( "\n#include \"" MYSHADERDIR "raf1_float3.cg\"\n", fout );
            else if( arith_type == ARITH_RAA_FLOAT3 )
                fputs( "\n#include \"" MYSHADERDIR "raf1_float3.cg\"\n", fout );
            else if( arith_type == ARITH_RAA_FLOAT4 )
                fputs( "\n#include \"" MYSHADERDIR "raf1_float4.cg\"\n", fout );
        }

        else if( strstr( lineBuffer, "STUB__TINTERVAL_SETUP" ) )
        {
            if( arith_type == ARITH_IA )
                fputs( "\n  interval ix, iy, iz;\n  interval F_t;\n\n", fout );
            else if( arith_type == ARITH_RAA_FLOAT3 || arith_type
                            == ARITH_RAA_FLOAT4 )
                fputs( "\n  raf raft;\n  raf ix, iy, iz;\n  interval F_t;\n\n",
                                fout );
        }

        else if( strstr( lineBuffer, "STUB__TINTERVAL_COMPUTE_AND_TEST" ) )
        {
            if( arith_type == ARITH_IA )
            {
                fputs(
                                "\n  ix = iadd(orig.x, imul(t, dir.x));\n  iy = iadd(orig.y, imul(t, dir.y));\n  iz = iadd(orig.z, imul(t, dir.z));\n",
                                fout );
                if( use_cgim_reject_test )
                    fputs( ia_reject_string.c_str(), fout );
                else
                {
                    fputs(
                                    "  F_t = evaluate_interval(nan, inf, ix, iy, iz, w, r0, r1, r2);\n",
                                    fout );
                    if( isovalue_type == ISO_SINGLE )
                    {
                        fputs( "  if (icontains(F_t, isovalue.x))\n", fout );
                    }
                    else if( isovalue_type == ISO_MULTI )
                    {
                        fputs(
                                        "  bool4 contains4 = icontains4(F_t, isovalue);\n  if (any(contains4))\n",
                                        fout );
                    }
                    else if( isovalue_type == ISO_RANGE )
                    {
                        fputs( "  bool contains = false; float iso;\n", fout );
                        fputs(
                                        "  for(iso = isovalue.x; !contains || iso < isovalue.y; iso += iso_incr)\n     contains += icontains(F_t, iso);\n",
                                        fout );
                        fputs(
                                        "  for(iso = isovalue.z; !contains || iso < isovalue.w; iso += iso_incr)\n     contains += icontains(F_t, iso);\n",
                                        fout );
                        fputs( "  const float iso_hit = iso;\n", fout );
                        fputs( "  if (contains)\n", fout );
                    }
                }
            }
            else if( arith_type == ARITH_RAA_FLOAT3 || arith_type
                            == ARITH_RAA_FLOAT4 )
            {
                fputs(
                                "\n  raft = interval_to_raf(t);\n  ix = raf_add(orig.x, raf_mul(raft, dir.x));\n  iy = raf_add(orig.y, raf_mul(raft, dir.y));\n  iz = raf_add(orig.z, raf_mul(raft, dir.z));\n",
                                fout );
                if( use_cgim_reject_test )
                    fputs( raf_reject_string.c_str(), fout );
                else
                {
                    fputs(
                                    "  F_t = raf_to_interval(evaluate_raf(nan, inf, ix, iy, iz, w, r0, r1, r2));\n",
                                    fout );
                    if( isovalue_type == ISO_SINGLE )
                    {
                        fputs( "  if (icontains(F_t, isovalue.x))\n", fout );
                    }
                    else if( isovalue_type == ISO_MULTI )
                    {
                        fputs(
                                        "  bool4 contains4 = icontains4(F_t, isovalue);\n  if (any(contains4))\n",
                                        fout );
                    }
                    else if( isovalue_type == ISO_RANGE )
                    {
                        fputs( "  bool contains = false; float iso;\n", fout );
                        fputs(
                                        "  for(iso = isovalue.x; !contains || iso < isovalue.y; iso += iso_incr)\n     contains += icontains(F_t, iso);\n",
                                        fout );
                        fputs(
                                        "  for(iso = isovalue.z; !contains || iso < isovalue.w; iso += iso_incr)\n     contains += icontains(F_t, iso);\n",
                                        fout );
                        fputs( "  const float iso_hit = iso;\n", fout );
                        fputs( "  if (contains)\n", fout );
                    }
                }
            }
        }

        else if( strstr( lineBuffer, "STUB__EVALUATE" ) )
        {
            //search evalExpression for powI(
            string powersExpression, evalExpression, dfdxExpression,
                            dfdyExpression, dfdzExpression;

            if( use_cgim )
            {
                FILE * fimp = fopen( getFileName( functionString ).c_str(),
                                "rb" );
                if( !fimp )
                    return;
                const int readBufferSize = LINECHARS;
                char readBuffer[readBufferSize];

                bool waitForOPTIONS = false;
                bool waitForGRADIENT = false;
                bool waitForIA = false;
                bool waitForRAF = false;
                bool waitForPRETRAVERSE = false;
                bool waitForTRAVERSE_HIT = false;
                bool waitForIA_REJECT = false;
                bool waitForRAF_REJECT = false;
                bool waitForAD = false;
                bool waitForADIA = false;
                bool waitForEVAL_GRADIENT = false;

                for( ;; )
                {
                    if( !fgets( readBuffer, LINECHARS, fimp ) )
                        break;

                    if( strstr( readBuffer, "TRAVERSE_HIT_BEGIN" ) )
                    {
                        waitForTRAVERSE_HIT = true;
                        continue;
                    }
                    else if( strstr( readBuffer, "TRAVERSE_HIT_END" ) )
                    {
                        waitForTRAVERSE_HIT = false;
                        continue;
                    }
                    else if( strstr( readBuffer, "PRETRAVERSE_BEGIN" ) )
                    {
                        waitForPRETRAVERSE = true;
                        continue;
                    }
                    else if( strstr( readBuffer, "PRETRAVERSE_END" ) )
                    {
                        waitForPRETRAVERSE = false;
                        continue;
                    }
                    else if( strstr( readBuffer, "EVAL_GRADIENT_BEGIN" ) )
                    {
                        waitForEVAL_GRADIENT = true;
                        continue;
                    }
                    else if( strstr( readBuffer, "EVAL_GRADIENT_END" ) )
                    {
                        waitForEVAL_GRADIENT = false;
                        continue;
                    }
                    else if( strstr( readBuffer, "RAF_REJECT_TEST_BEGIN" ) )
                    {
                        use_cgim_reject_test = true;
                        waitForRAF_REJECT = true;
                        continue;
                    }
                    else if( strstr( readBuffer, "RAF_REJECT_TEST_END" ) )
                    {
                        waitForRAF_REJECT = false;
                        continue;
                    }
                    else if( strstr( readBuffer, "IA_REJECT_TEST_BEGIN" ) )
                    {
                        use_cgim_reject_test = true;
                        waitForIA_REJECT = true;
                        continue;
                    }
                    else if( strstr( readBuffer, "IA_REJECT_TEST_END" ) )
                    {
                        waitForIA_REJECT = false;
                        continue;
                    }
                    else if( strstr( readBuffer, "OPTIONS_BEGIN" ) )
                    {
                        waitForOPTIONS = true;
                        continue;
                    }
                    else if( strstr( readBuffer, "OPTIONS_END" ) )
                    {
                        waitForOPTIONS = false;
                        continue;
                    }
                    else if( strstr( readBuffer, "GRADIENT_BEGIN" ) )
                    {
                        use_cgim_gradient = true;
                        waitForGRADIENT = true;
                        continue;
                    }
                    else if( strstr( readBuffer, "IARITH_BEGIN" ) )
                    {
                        waitForIA = true;
                        continue;
                    }
                    else if( strstr( readBuffer, "RAF_BEGIN" ) )
                    {
                        waitForRAF = true;
                        continue;
                    }
                    else if( strstr( readBuffer, "AD_BEGIN" ) )
                    {
                        waitForAD = true;
                        continue;
                    }
                    else if( strstr( readBuffer, "ADIA_BEGIN" ) )
                    {
                        waitForADIA = true;
                        continue;
                    }
                    else if( strstr( readBuffer, "GRADIENT_END" ) )
                    {
                        waitForGRADIENT = false;
                        continue;
                    }
                    else if( strstr( readBuffer, "IARITH_END" ) )
                    {
                        waitForIA = false;
                        continue;
                    }
                    else if( strstr( readBuffer, "RAF_END" ) )
                    {
                        waitForRAF = false;
                        continue;
                    }
                    else if( strstr( readBuffer, "AD_END" ) )
                    {
                        waitForAD = false;
                        continue;
                    }
                    else if( strstr( readBuffer, "ADIA_END" ) )
                    {
                        waitForADIA = false;
                        continue;
                    }
                    else
                    {
                        if( (waitForIA && !genIA) || (waitForRAF && !genRAFA)
                                        || (waitForAD && !genAD)
                                        || (waitForADIA && !genADIA) )
                            continue;

                        if( waitForOPTIONS )
                        {
                            //parse options

                            char option_string[16];
                            if( newfunction )
                            {
                                if( strstr( readBuffer, "domain" ) )
                                {
#ifdef __GNUC__
#warning [alex, assert]
#endif
                                    assert(0);
                                    sscanf( readBuffer, "%s %f %f %f %f %f %f",
                                                    option_string,
                                                    &domain_min[0],
                                                    &domain_min[1],
                                                    &domain_min[2],
                                                    &domain_max[0],
                                                    &domain_max[1],
                                                    &domain_max[2] );
                                    // FIX domain_callback(0);
                                    newdomain = true;
                                }
                                else if( strstr( readBuffer, "colors" ) )
                                {
                                    sscanf( readBuffer, "%s %f %f %f %f %f %f",
                                                    option_string,
                                                    &front_color[0],
                                                    &front_color[1],
                                                    &front_color[2],
                                                    &back_color[0],
                                                    &back_color[1],
                                                    &back_color[2] );
                                }
                                else if( strstr( readBuffer, "wanim" ) )
                                {
                                    sscanf( readBuffer, "%s %f %f %f %f",
                                                    option_string, &var_w,
                                                    &var_w_min, &var_w_max,
                                                    &var_dw );
                                }
                                else if( strstr( readBuffer, "r012" ) )
                                {
                                    sscanf( readBuffer, "%s %f %f %f",
                                                    option_string, &var_r0,
                                                    &var_r1, &var_r2 );
                                }
                                else if( strstr( readBuffer, "phong" ) )
                                {
                                    sscanf( readBuffer, "%s %f", option_string,
                                                    &phong_exponent );
                                }
#if 0
                                else if (strstr(readBuffer, "camera"))
                                {
                                    char tmp0[4], tmp1[4], tmp2[4];
                                    sscanf(readBuffer, "%s %s %f %f %f %s %f %f %f %s %f %f %f", option_string,
                                                    tmp0, &camera.lookFrom[0], &camera.lookFrom[1], &camera.lookFrom[2],
                                                    tmp1, &camera.lookAt[0], &camera.lookAt[1], &camera.lookAt[2],
                                                    tmp2, &camera.lookUp[0], &camera.lookUp[1], &camera.lookUp[2]);
                                    newcamera = true;
                                    compute_camera_uv();
                                }
#endif
                            }
                        }
                        else
                        {
                            string tmp = readBuffer;
                            if( waitForIA_REJECT )
                                ia_reject_string += tmp;
                            else if( waitForRAF_REJECT )
                                raf_reject_string += tmp;
                            else if( waitForTRAVERSE_HIT )
                                traverse_hit_string += tmp;
                            else if( waitForPRETRAVERSE )
                                pretraverse_string += tmp;
                            else if( waitForEVAL_GRADIENT )
                                eval_gradient_string += tmp;
                            else
                                //just copy it verbatim into the shader
                                powersExpression += tmp;
                        }
                    }
                    // FIX GLUI_Master.sync_live_all();
                }

                newfunction = false;

                newdomain = false;
                //R newcamera = false;

            }
            else
            {
                evalExpression = "return " + evaluator_generate_expression(
                                functionString ) + ";";
                dfdxExpression = "\n  grad.x = "
                                + evaluator_generate_expression( dfdxString )
                                + ";";
                dfdyExpression = "\n  grad.y = "
                                + evaluator_generate_expression( dfdyString )
                                + ";";
                dfdzExpression = "\n  grad.z = "
                                + evaluator_generate_expression( dfdzString )
                                + ";";

                powersExpression = evalExpression + dfdxExpression
                                + dfdyExpression + dfdzExpression;
            }

            int powpos = 0;
            vector<int> powers_list;
            for( ;; )
            {
                powpos = powersExpression.find( "pow", powpos );
                if( powpos == string::npos )
                    break;
                int parenpos = powersExpression.find( "(", powpos );
                int ilen = parenpos - powpos;
                if( ilen > 3 )
                {
                    string istr =
                                    powersExpression.substr( powpos + 3, ilen
                                                    - 3 );
                    int intpower = atoi( istr.c_str() );
                    bool already = false;
                    for( int i = 0; i < powers_list.size(); i++ )
                    {
                        if( powers_list[i] == intpower )
                        {
                            already = true;
                            break;
                        }
                    }
                    if( !already )
                        powers_list.push_back( intpower );

                    if( genADIA && intpower > 2 )
                    {
                        int ipm1 = intpower - 1;
                        already = false;
                        for( int i = 0; i < powers_list.size(); i++ )
                        {
                            if( powers_list[i] == ipm1 )
                            {
                                already = true;
                                break;
                            }
                        }
                        if( !already )
                            powers_list.push_back( ipm1 );
                    }

                }
                powpos++;
            }

            {
                for( int i = 0; i < powers_list.size(); i++ )
                    fputs( make_pow( powers_list[i] ).c_str(), fout );
                fputs( "\n", fout );
            }

            //make the powers
            if( genAD )
            {
                for( int i = 0; i < powers_list.size(); i++ )
                    fputs( make_autodiff_pow( powers_list[i] ).c_str(), fout );
                fputs( "\n", fout );
            }

            if( genIA || genADIA )
            {
                for( int i = 0; i < powers_list.size(); i++ )
                    fputs( make_interval_pow( powers_list[i] ).c_str(), fout );
                fputs( "\n", fout );
            }

            if( genRAFA )
            {
                for( int i = 0; i < powers_list.size(); i++ )
                    fputs( make_affine_pow( powers_list[i] ).c_str(), fout );
                fputs( "\n", fout );
            }

            // FIXME: compiler option?
            if( genADIA )
            {
                for( int i = 0; i < powers_list.size(); i++ )
                    fputs( make_adia_pow( powers_list[i] ).c_str(), fout );
                fputs( "\n", fout );
            }
            // XXXXXXXXXXXXX
            int nbVars = 1;
            if( tex )
            {
                nbVars = tex->getWidth() * 4;
            }
            std::cout << "Texture " << tex->getWidth() << "x"
                            << tex->getHeight() << "\n";
            std::cout << "Allocating data lookup for " << nbVars
                            << " variables.\n";
            std::string dataLookup = lookupString( nbVars );
            fputs( dataLookup.c_str(), fout );

            if( use_cgim )
            {
                fputs( powersExpression.c_str(), fout );
            }
            else //generate from expression
            {

                /*
                 // local data lookup

                 std::string dataLookup =
                 "\n"
                 "\n float4 a0 = tex2D(mydata, float2(0.125,w))*2.0;"
                 "\n float4 a1 = tex2D(mydata, float2(0.375,w))*2.0;"
                 "\n float4 a2 = tex2D(mydata, float2(0.625,w))*2.0;"
                 "\n float4 a3 = tex2D(mydata, float2(0.875,w))*2.0;"
                 "\n"
                 "\n float A0  = a0.x;"
                 "\n float A1  = a0.y;"
                 "\n float A2  = a0.z;"
                 "\n float A3  = a0.w;"
                 "\n float A4  = a1.x;"
                 "\n float A5  = a1.y;"
                 "\n float A6  = a1.z;"
                 "\n float A7  = a1.w;"
                 "\n float A8  = a2.x;"
                 "\n float A9  = a2.y;"
                 "\n float A10 = a2.z;"
                 "\n float A11 = a2.w;"
                 "\n float A12 = a3.x;"
                 "\n float A13 = a3.y;"
                 "\n float A14 = a3.z;"
                 "\n float A15 = a3.w;"
                 "\n";
                 */
                fputs(
                                "\n\nfloat evaluate(float3 p, float w, float r0, float r1, float r2)",
                                fout );
                fputs( "\n{", fout );
                fputs( "\n  float x = p.x; float y = p.y; float z = p.z;\n  ",
                                fout );
                fputs( evalExpression.c_str(), fout );
                fputs( "\n}", fout );

                if( genAD )
                {
                    string evalAutodiffExpression = "\n  return "
                                    + postfix_generate_expression( "ad",
                                                    functionString ) + ";";
                    fputs(
                                    "\n\nautodiff evaluate_autodiff(autodiff x, autodiff y, autodiff z, float w, float r0, float r1, float r2)",
                                    fout );
                    fputs( "\n{", fout );
                    fputs( evalAutodiffExpression.c_str(), fout );
                    fputs( "\n}", fout );
                }

                if( genIA )
                {
                    string evalIntervalExpression = "\n  return "
                                    + postfix_generate_expression( "i",
                                                    functionString ) + ";";
                    fputs(
                                    "\n\ninterval evaluate_interval(float nan, float inf, interval x, interval y, interval z, float w, float r0, float r1, float r2)",
                                    fout );
                    fputs( "\n{", fout );
                    fputs( evalIntervalExpression.c_str(), fout );
                    fputs( "\n}", fout );
                }

                if( genADIA )
                {
                    string evalADIAExpression = "\n  return "
                                    + postfix_generate_expression( "adi",
                                                    functionString ) + ";";
                    fputs(
                                    "\n\nadi evaluate_adi(float nan, float inf, adi x, adi y, adi z, float w, float r0, float r1, float r2)",
                                    fout );
                    fputs( "\n{", fout );
                    fputs( evalADIAExpression.c_str(), fout );
                    fputs( "\n}", fout );
                }

                if( genRAFA )
                {
                    string evalRAFAExpression = "\n  return "
                                    + postfix_generate_expression( "raf_",
                                                    functionString ) + ";";
                    fputs(
                                    "\n\nraf evaluate_raf(float nan, float inf, raf x, raf y, raf z, float w, float r0, float r1, float r2)",
                                    fout );
                    fputs( "\n{", fout );
                    fputs( evalRAFAExpression.c_str(), fout );
                    fputs( "\n}", fout );
                }
                //end function expression generator section

                //gradient
                if( !use_cgim_gradient )
                {
                    if( centralDifferences )
                    {
                        fputs(
                                        "\n\nfloat3 evaluate_gradient(float3 p, float w, float r0, float r1, float r2)",
                                        fout );
                        fputs( "\n{", fout );
                        fputs( "\n  float3 grad, ptmp;\n  float evalL, evalR;",
                                        fout );
                        stringstream scdd;
                        scdd << centralDifferencesDelta;
                        dfdxExpression
                                        = "  ptmp.yz = p.yz;\n  ptmp.x = p.x - "
                                                        + scdd.str()
                                                        + ";\n  evalL = evaluate(ptmp, w, r0, r1, r2);\n  ptmp.x = p.x + "
                                                        + scdd.str()
                                                        + ";\n  evalR = evaluate(ptmp, w, r0, r1, r2);\n  grad.x = evalR - evalL;\n";
                        fputs( dfdxExpression.c_str(), fout );
                        dfdyExpression
                                        = "  ptmp.xz = p.xz;\n  ptmp.y = p.y - "
                                                        + scdd.str()
                                                        + ";\n  evalL = evaluate(ptmp, w, r0, r1, r2);\n  ptmp.y = p.y + "
                                                        + scdd.str()
                                                        + ";\n  evalR = evaluate(ptmp, w, r0, r1, r2);\n  grad.y = evalR - evalL;\n";
                        fputs( dfdyExpression.c_str(), fout );
                        dfdzExpression
                                        = "  ptmp.xy = p.xy;\n  ptmp.z = p.z - "
                                                        + scdd.str()
                                                        + ";\n  evalL = evaluate(ptmp, w, r0, r1, r2);\n  ptmp.z = p.z + "
                                                        + scdd.str()
                                                        + ";\n  evalR = evaluate(ptmp, w, r0, r1, r2);\n  grad.z = evalR - evalL;\n";
                        fputs( dfdzExpression.c_str(), fout );
                        string finalExpr = "  grad *= 1/(2*" + scdd.str()
                                        + ");\n  return grad;\n}\n";
                        //string finalExpr = "  return grad;\n}\n";
                        fputs( finalExpr.c_str(), fout );
                    }
                    else
                    {
                        fputs(
                                        "\n\nfloat3 evaluate_gradient(float3 p, float w, float r0, float r1, float r2)",
                                        fout );
                        fputs( "\n{", fout );
                        fputs(
                                        "\n  float x = p.x;\n  float y = p.y;\n  float z = p.z;\n  float3 grad;",
                                        fout );
                        fputs( dfdxExpression.c_str(), fout );
                        fputs( dfdyExpression.c_str(), fout );
                        fputs( dfdzExpression.c_str(), fout );
                        fputs( "\n  return grad; \n}", fout );
                    }
                }
            } //end if !use_cgim

            actualCentralDifferences = centralDifferences;
        }
        else if( strstr( lineBuffer, "STUB__BUMPMAP" ) )
        {
            if( bumpmapID )
            {
                stringstream ss;
                ss << bumpmapFrequency;
                string bms =
                                "  float3 normalized_gradient= normalize(gradient);\n"
                                    "  float3 offset = float3(0,0,0);\n"
                                    "  float3 up = normalized_gradient;\n"
                                    "  float3 right = float3(1,1,1);\n"
                                    "  float3 other = cross(right, up);\n"
                                    "  right = cross(other, up);\n"
                                    "  float scale = 1 / (bumpmap_frequency * length(gradient));\n"
                                    "  offset = sin(scale * phit.x) * right + sin(scale * phit.y + other);\n"
                                    "  float3 offset_scale = 0.25f;\n"
                                    "  offset*= offset_scale;\n"
                                    "  normal = normalize( normalized_gradient + offset);\n"
                                    "  if(dot(normal,normalized_gradient) < 0)\n"
                                    "	   normal = normalized_gradient;\n";

                fputs( bms.c_str(), fout );
            }
            else
            {
                fputs( "  normal = normalize(gradient);", fout );
            }
            actualBumpmapID = bumpmapID;
        }
        else if( strstr( lineBuffer, "STUB__TERMINATE_CONDITION" ) )
        {
            if( delta )
            {
                if( arith_type == ARITH_IA )
                    fputs( "        if (d==dlast && iwidth(F_t) < delta)", fout );
                else
                    fputs(
                                    "        if (d==dlast && 2*raf_radius(F_t) < delta)",
                                    fout );
            }
            else
            {
                fputs( "        if (d==dlast)", fout );
            }
        }
        else if( strstr( lineBuffer, "STUB__PRE_TRAVERSE" ) )
        {
            fputs( pretraverse_string.c_str(), fout );
        }
        else if( strstr( lineBuffer, "STUB__HIT" ) )
        {
            if( traverse_hit_string == "" )
            {
                if( asymptotes == 1 )
                    fputs( "          hr.opt.x = isinf(iwidth(F_t));\n", fout );
                else if( asymptotes == 2 )
                    fputs( "          if (isfinite(iwidth(F_t)))\n", fout );

                fputs( "          {\n", fout );

                if( shadeOptions.depth_peeling )
                {
                    if( isovalue_type == ISO_MULTI )
                    {
                        fputs(
                                        "            if (hr.thit.x == -1 ){ hr.thit.x = t.x; hr.opt.x = contains4.x ? isovalue.x : (contains4.y ? isovalue.y : (contains4.z ? isovalue.z : isovalue.w));}\n",
                                        fout );
                        fputs(
                                        "            else if (hr.thit.y == -1 && depth_peel_allow(t.x, hr.thit.x, orig, dir, w, r0, r1, r2)) {   hr.thit.y = t.x; hr.opt.y = contains4.x ? isovalue.x : (contains4.y ? isovalue.y : (contains4.z ? isovalue.z : isovalue.w)); }\n",
                                        fout );
                        fputs(
                                        "            else if (hr.thit.z == -1 && depth_peel_allow(t.x, hr.thit.y, orig, dir, w, r0, r1, r2)) {   hr.thit.z = t.x; hr.opt.z = contains4.x ? isovalue.x : (contains4.y ? isovalue.y : (contains4.z ? isovalue.z : isovalue.w)); }\n",
                                        fout );
                        fputs(
                                        "            else if (depth_peel_allow(t.x, hr.thit.z, orig, dir, w, r0, r1, r2)) { hr.thit.w = t.x; hr.opt.w = contains4.x ? isovalue.x : (contains4.y ? isovalue.y : (contains4.z ? isovalue.z : isovalue.w)); break; }\n",
                                        fout );
                    }
                    else if( isovalue_type == ISO_RANGE )
                    {
                        fputs(
                                        "            if (hr.thit.x == -1 ){ hr.thit.x = t.x; hr.opt.x = iso_hit;}\n",
                                        fout );
                        fputs(
                                        "            else if (hr.thit.y == -1 && depth_peel_allow(t.x, hr.thit.x, orig, dir, w, r0, r1, r2)) { hr.thit.y = t.x; hr.opt.y = iso_hit;}\n",
                                        fout );
                        fputs(
                                        "            else if (hr.thit.z == -1 && depth_peel_allow(t.x, hr.thit.y, orig, dir, w, r0, r1, r2)) { hr.thit.z = t.x; hr.opt.z = iso_hit;}\n",
                                        fout );
                        fputs(
                                        "            else if (depth_peel_allow(t.x, hr.thit.z, orig, dir, w, r0, r1, r2)) { hr.thit.w = t.x; break; hr.opt.w = iso_hit;}\n",
                                        fout );
                    }
                    else
                    {
                        fputs(
                                        "            if (hr.thit.x == -1 ){ hr.thit.x = t.x; }\n",
                                        fout );
                        fputs(
                                        "            else if (hr.thit.y == -1 && depth_peel_allow(t.x, hr.thit.x, orig, dir, w, r0, r1, r2)) { hr.thit.y = t.x; }\n",
                                        fout );
                        fputs(
                                        "            else if (hr.thit.z == -1 && depth_peel_allow(t.x, hr.thit.y, orig, dir, w, r0, r1, r2)) { hr.thit.z = t.x; }\n",
                                        fout );
                        fputs(
                                        "            else if (depth_peel_allow(t.x, hr.thit.z, orig, dir, w, r0, r1, r2)) { hr.thit.w = t.x; break; }\n",
                                        fout );
                    }
                }
                else
                {
                    fputs( "            hr.thit.x = t.x;\n", fout );
                    if( isovalue_type == ISO_MULTI )
                        fputs(
                                        "            hr.opt.x = contains4.x ? isovalue.x : (contains4.y ? isovalue.y : (contains4.z ? isovalue.z : isovalue.w));\n",
                                        fout );
                    else if( isovalue_type == ISO_RANGE )
                        fputs( "            hr.opt.x = iso_hit;", fout );
                    fputs( "            break;\n", fout );
                }

                fputs( "          }\n", fout );
            }
            else
            {
                fputs( traverse_hit_string.c_str(), fout );
            }

            actualAsymptotes = asymptotes;
        }
        else if( strstr( lineBuffer, "STUB__SHADOWS_IFDEF" ) )
        {
            if( shadeOptions.shadows )
                fputs( "#if 1\n", fout );
            else
                fputs( "#if 0\n", fout );
        }
        else if( strstr( lineBuffer, "STUB__DP_IFDEF" ) )
        {
            if( shadeOptions.depth_peeling )
                fputs( "#if 1\n", fout );
            else
                fputs( "#if 0\n", fout );
        }
        else if( strstr( lineBuffer, "STUB__EVAL_GRADIENT" ) )
        {
            if( eval_gradient_string == "" )
            {
                fputs(
                                "  const float3 gradient = evaluate_gradient(phit, w, r0, r1, r2);",
                                fout );
            }
            else
                fputs( eval_gradient_string.c_str(), fout );
        }
        else if( strstr( lineBuffer, "STUB__SHADE" ) )
        {
            fputs( "  float3 front, back;\n", fout );
            if( color_direction )
            {
                fputs( "  front = normalize(abs(phit));", fout );
                fputs( "  back  = front;", fout );
            }

            else if( asymptotes == 1 )
                fputs(
                                "  front = opt.x ? float3(1,1,1) - front_color : front_color; back = opt.x ? front : back_color;\n",
                                fout );
            else if( isovalue_type == ISO_SINGLE )
                fputs( "  front = front_color; back = back_color;\n", fout );
            else
                fputs(
                                "  front = tex1D(hsv_colors, (opt + iso_color_offset) * iso_color_scale); back = front;\n",
                                fout );

            switch( shaderID )
            {
                case 0:
                    fputs(
                                    "  float3 lookup_color = n_dot_v > 0 ? front : back;",
                                    fout );
                    break;
                case 1:
                    fputs(
                                    "  float3 lookup_color = n_dot_v > 0 ? front : back;\n  lookup_color = lerp( lookup_color, float3(1,1,1), sin(log(length(gradient))));",
                                    fout );
                    break;
                case 2:
                    fputs(
                                    "  float3 lookup_color = n_dot_v > 0 ? IN.TexCoord : 1 -IN.TexCoord;",
                                    fout );
                    break;
            }
            actualShaderID = shaderID;
        }
        else
        {
            fputs( lineBuffer, fout );
        }
    }
    fclose( fin );
    fclose( fout );

    // load the vertex and fragment raycasting programs
    load_vertex_program( vertex_main, shaderOut, "vertex_main" );
    // FIX cgErrorCallback();
    load_fragment_program( fragment_main, shaderOut, "fragment_main" );
    // FIX cgErrorCallback();
}

void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
{
    int i;
    float f, p, q, t;
    if( s == 0 )
    {
        // achromatic (grey)
        *r = *g = *b = v;
        return;
    }
    h /= 60; // sector 0 to 5
    i = (int)floor( h );
    f = h - i; // factorial part of h
    p = v * (1 - s);
    q = v * (1 - s * f);
    t = v * (1 - s * (1 - f));
    switch( i )
    {
        case 0:
            *r = v;
            *g = t;
            *b = p;
            break;
        case 1:
            *r = q;
            *g = v;
            *b = p;
            break;
        case 2:
            *r = p;
            *g = v;
            *b = t;
            break;
        case 3:
            *r = p;
            *g = q;
            *b = v;
            break;
        case 4:
            *r = t;
            *g = p;
            *b = v;
            break;
        default: // case 5:
            *r = v;
            *g = p;
            *b = q;
            break;
    }
}

void generate_hsv_colors()
{
    const float hsvs = HSV_SIZE;
    const float inv_hsvs = 1.f / hsvs;
    for( int i = 0; i < HSV_SIZE; i++ )
    {
        float h = i * inv_hsvs;
        float r, g, b;
        HSVtoRGB( &r, &g, &b, h, 1.f, 1.f );
        hsv_buffer[i][0] = r;
        hsv_buffer[i][1] = g;
        hsv_buffer[i][2] = b;
    }
}

void FgeImplicitSurfaceGlyphs::cubeVertex( float x, float y, float z, float tx,
                                       float ty, float tz, float glyphPos )
{
    pglMultiTexCoord4f( GL_TEXTURE7_ARB, tx, ty, tz, glyphPos ); //this HAS to be [0,1]
    glVertex3f( x, y, z );
}

Vector3 unitVector( const Vector3&p )
{
    return Vector3( p ).normalize();
}

float dot( const Vector3&a, const Vector3&b )
{
    return a * b;
}

// ok let's start things up

void FgeImplicitSurfaceGlyphs::init()
{
    /* FIX
     cout << "glew init " << endl;
     GLenum err = glewInit();

     // initialize all the OpenGL extensions
     glewGetExtension("glMultiTexCoord2fvARB");
     if(glewGetExtension("GL_EXT_framebuffer_object") ) cout << "GL_EXT_framebuffer_object support " << endl;
     if(glewGetExtension("GL_EXT_renderbuffer_object")) cout << "GL_EXT_renderbuffer_object support " << endl;
     if(glewGetExtension("GL_ARB_vertex_buffer_object")) cout << "GL_ARB_vertex_buffer_object support" << endl;
     if(GL_ARB_multitexture)cout << "GL_ARB_multitexture support " << endl;

     if (glewGetExtension("GL_ARB_fragment_shader")      != GL_TRUE ||
     glewGetExtension("GL_ARB_vertex_shader")        != GL_TRUE ||
     glewGetExtension("GL_ARB_shader_objects")       != GL_TRUE ||
     glewGetExtension("GL_ARB_shading_language_100") != GL_TRUE)
     {
     cout << "Driver does not support OpenGL Shading Language" << endl;
     exit(1);
     }
     */
    //	glEnable(GL_CULL_FACE);
    //FIX	glClearColor(background_color[0], background_color[1], background_color[2], 0);

    //create_volumetexture();
    //create_bisect_array();

    // CG init
    cgSetErrorCallback( cgErrorCallback );
    context = cgCreateContext();
    if( cgGLIsProfileSupported( CG_PROFILE_VP40 ) )
    {
        vertexProfile = CG_PROFILE_VP40;
        cout << "CG_PROFILE_VP40 supported." << endl;
    }
    //#define TRY_GLSL
#ifdef TRY_GLSL
    else if (cgGLIsProfileSupported(CG_PROFILE_GLSLV))
    {
        vertexProfile = CG_PROFILE_GLSLV;
        cout << "CG_PROFILE_GLSLV supported." << endl;
    }
#endif
    else
        if( cgGLIsProfileSupported( CG_PROFILE_VP30 ) )
        {
            vertexProfile = CG_PROFILE_VP30;
            cout << "CG_PROFILE_VP30 supported." << endl;
        }
        else
            if( cgGLIsProfileSupported( CG_PROFILE_ARBVP1 ) )
            {
                cout << "Using CG_PROFILE_ARBVP1." << endl;
                vertexProfile = CG_PROFILE_ARBVP1;
            }
            else
            {
                cout
                                << "Neither arbvp1 or vp40 vertex profiles supported on this system."
                                << endl;
                exit( 1 );
            }

    if( cgGLIsProfileSupported( CG_PROFILE_FP40 ) )
    {
        fragmentProfile = CG_PROFILE_FP40;
        cout << "CG_PROFILE_FP40 supported." << endl;
    }
#ifdef TRY_GLSL
    else if (cgGLIsProfileSupported(CG_PROFILE_GLSLF))
    {
        fragmentProfile = CG_PROFILE_GLSLF;
        cout << "CG_PROFILE_GLSLF supported." << endl;
    }
#endif
    else
        if( cgGLIsProfileSupported( CG_PROFILE_FP30 ) )
        {
            fragmentProfile = CG_PROFILE_FP30;
            cout << "CG_PROFILE_FP30 supported." << endl;
        }
        else
            if( cgGLIsProfileSupported( CG_PROFILE_ARBFP1 ) )
            {
                cout << "Using CG_PROFILE_ARBFP1." << endl;
                fragmentProfile = CG_PROFILE_ARBFP1;
            }
            else
            {
                cout
                                << "Neither arbfp1 or fp40 fragment profiles supported on this system."
                                << endl;
                exit( 1 );
            }

    vertexProfile = cgGLGetLatestProfile( CG_GL_VERTEX );
    fragmentProfile = cgGLGetLatestProfile( CG_GL_FRAGMENT );

    cout << cgGetProfileString( vertexProfile ) << endl;
    cout << cgGetProfileString( fragmentProfile ) << endl;

    cgGLSetOptimalOptions( fragmentProfile );
    cgGLSetOptimalOptions( vertexProfile );

    //build_shader( 0 );

    /*
     // load the vertex and fragment raycasting programs
     load_vertex_program(vertex_main,shaderBase,"vertex_main");
     cgErrorCallback();
     load_fragment_program(fragment_main,shaderBase,"fragment_main");
     cgErrorCallback();
     */

    // Create the to FBO's one for the backside of the volumecube and one for the finalimage rendering
    glext.pglGenFramebuffersEXT( 1, &framebuffer );
    glext.pglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, framebuffer );

    glGenTextures( 1, &backface_buffer );
    glBindTexture( GL_TEXTURE_2D, backface_buffer );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, res_w, res_h, 0, GL_RGBA,
                    GL_FLOAT, NULL );

    glext.pglFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT,
                    GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, backface_buffer, 0 );

    glGenTextures( 1, &final_image );
    glBindTexture( GL_TEXTURE_2D, final_image );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, res_w, res_h, 0, GL_RGBA,
                    GL_FLOAT, NULL );

    glext.pglGenRenderbuffersEXT( 1, &renderbuffer );
    glext.pglBindRenderbufferEXT( GL_RENDERBUFFER_EXT, renderbuffer );
    glext.pglRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,
                    res_w, res_h );
    glext.pglFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT,
                    GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, renderbuffer );
    glext.pglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

    generate_hsv_colors();
    // FIXME: do I need this? create_pseudorandom_tables();
    glGenTextures( 1, &hsv_colors );
    glBindTexture( GL_TEXTURE_1D, hsv_colors );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    //glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F_ARB, HSV_SIZE, 0, GL_RGBA, GL_FLOAT, &hsv_buffer[0][0]);
    glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA16F_ARB, HSV_SIZE, 0, GL_RGBA,
                    GL_FLOAT, &hsv_buffer[0][0] );

    // FIX cube_map = gltutCubeLoadTexture("simple_%c%s.tga",true);
    // FIX cube_map = gltutCubeLoadTexture("desert_%s%c.tga",false);

    //initialize the camera
    // FIX  if (!cameraSet)
    //  {
    //    reset_camera();
    //  }

}

void FgeImplicitSurfaceGlyphs::mymain(/*int argc, char**argv*/)
{

    //default shade options
    shadeOptions.eye_light = true;
    shadeOptions.shadows = false;
    shadeOptions.depth_peeling = false;
    shadeOptions.flip_color = false;

    light[0] = Vector3( 0, 5, 10 );
    if( strcmp( functionStringTmp, "" ) )
        functionString.assign( functionStringTmp );

    //		const float transspeed = .05;

    if( strcmp( functionStringTmp, "" ) )
    {
        assert(0);
        build_shader( 0 );
        //	    reset_camera();
    }
}

FgeImplicitSurfaceGlyphs::FgeImplicitSurfaceGlyphs()
:   color_direction(true),tex(0), data(0), maxNumberOfGlyphs(0)
{
    vertices.clear();

    dList = 0;
    recreateList = true;

    // some default properties
    unifiedScaling = false;
    boxScale = 0.2;

    // use Superquadrics by default
    renderMode = 0;
}

FgeImplicitSurfaceGlyphs::~FgeImplicitSurfaceGlyphs()
{
    glDeleteTextures(1, &backface_buffer);
    glDeleteTextures(1, &final_image);
    glDeleteTextures(1, &hsv_colors);
    if( tex )
    {
        delete tex;
        tex = 0;
    }
    if( data )
    {
        delete[] data;
        data = 0;
    }
}

void FgeImplicitSurfaceGlyphs::initiate()
{

    FgeOpenGL::lookupGLFuncPtrWarn( pglMultiTexCoord3f, "glMultiTexCoord3f",
                    "glMultiTexCoord3fARB" );
    FgeOpenGL::lookupGLFuncPtrWarn( pglMultiTexCoord4f, "glMultiTexCoord4f",
                    "glMultiTexCoord4fARB" );
    //FgeOpenGL::lookupGLFuncPtrWarn( pglGenFramebuffers, "glGenFramebuffers" );
    //FgeOpenGL::lookupGLFuncPtrWarn( pglBindFramebuffer, "glBindFramebuffer" );
    //FgeOpenGL::lookupGLFuncPtrWarn( pglBindRenderbuffer, "glBindRenderbuffer", "glBindRenderbufferEXT" );
    //FgeOpenGL::lookupGLFuncPtrWarn( pglFramebufferRenderbuffer, "glFramebufferRenderbuffer", "glFramebufferRenderbufferEXT" );
    glext.initialize();

    //	char* argv="main";
    //mymain(1,&argv);

    mymain();

    init();
}

/**
 * \par Description
 * Adds a Tensor.
 * \param x X coordinate.
 * \param y Y coordinate.
 * \param z Z coordinate
 * \param tensor the tensor to add.
 */
void FgeImplicitSurfaceGlyphs::setNewVertex( GLfloat x, GLfloat y, GLfloat z )
{
    assert( vertices.size() <= maxNumberOfGlyphs );

    std::vector<double> myvertex(3);
    myvertex[0] = x;
    myvertex[1] = y;
    myvertex[2] = z;
    vertices.push_back( myvertex );

    // force display list recreation
    recreateList = true;
}

/**
 * \par Description
 * Reloads all needed shaders, compiles them, links them, dumps errors.
 */
void FgeImplicitSurfaceGlyphs::setData( const unsigned int maxNumberOfGlyphs, GLfloat *tensor, unsigned int width, unsigned int height)
{

    vertices.clear();
    std::cout<<this->maxNumberOfGlyphs<<" --> "<< maxNumberOfGlyphs << std::endl;
    //assert( this->maxNumberOfGlyphs <= maxNumberOfGlyphs );
    this->maxNumberOfGlyphs = maxNumberOfGlyphs;
    if( data != 0 )
    {
        delete[] data;
        data = 0;
    }
    data = new GLfloat[width*height];
    for( unsigned int i=0; i < width*height; ++i )
    {
	data[i]=tensor[i];
    }
//     for( unsigned int tensId = 0; tensId < width; ++ tensId )
//         for( unsigned int i=0; i < height; ++i )
//         {
//             data[tensId*8+i] = tensor[tensId*6+i];
//         }
}

void FgeImplicitSurfaceGlyphs::dataToTexture( unsigned int width, unsigned int height )
{
    if( tex )
    {
        delete tex;
        tex = 0;
    }

    if( data != 0 )
        tex = new FgeGLTexture( GL_TEXTURE_2D,  width/4, height, false, GL_FLOAT, data); // width/4 because of RGBA
    else
        assert( 0 );

    if( tex )
    {
        tex->initialize();
    }
    else
        cout << "TEXTURE POINTER EMPTY ---> TEXTURE WILL NOT BE INTIALIZED"
                        << endl;
    build_shader(0);
}

void FgeImplicitSurfaceGlyphs::drawBox( double x, double y, double z, double s, int glyph )
{
//
//    std::cout << "glyph: " << glyph << " tex->height: " << tex->getHeight() << std::endl;
    float p = 1. / (float)(2. * tex->getHeight()) + (float)glyph / (float)tex->getHeight();
//    std::cout << "p: " << p << " xyz: "<<x<<" "<<y<<" "<<z<<std::endl;
    FFixArray<double, 3> box_domain_min( x - s, y - s, z - s );
    FFixArray<double, 3> box_domain_max( x + s, y + s, z + s );
    pglMultiTexCoord4f( GL_TEXTURE3_ARB, x, y, z, p ); //store centerpoint of glyph in TEXCOORD3
//    std::cout << "box_domain_min" << box_domain_min << std::endl;
//    std::cout << "box_domain_max" << box_domain_max << std::endl;


    glBegin( GL_QUADS );

    // here we give the vertex coordinates and texture coordiantes for the
    // vertices of the cube side faces. min-coord correposnds to texure coord 0
    // max-coord to texture coord 1

    /* Back side */
    glNormal3f( 0.0, 0.0, -1.0 );
    cubeVertex( box_domain_min[0], box_domain_min[1], box_domain_min[2], 0., 0., 0., p );
    cubeVertex( box_domain_min[0], box_domain_max[1], box_domain_min[2], 0., 1., 0., p );
    cubeVertex( box_domain_max[0], box_domain_max[1], box_domain_min[2], 1., 1., 0., p );
    cubeVertex( box_domain_max[0], box_domain_min[1], box_domain_min[2], 1., 0., 0., p );

    /* Front side */
    glNormal3f( 0.0, 0.0, 1.0 );
    cubeVertex( box_domain_min[0], box_domain_min[1], box_domain_max[2], 0., 0., 1., p );
    cubeVertex( box_domain_max[0], box_domain_min[1], box_domain_max[2], 1., 0., 1., p );
    cubeVertex( box_domain_max[0], box_domain_max[1], box_domain_max[2], 1., 1., 1., p );
    cubeVertex( box_domain_min[0], box_domain_max[1], box_domain_max[2], 0., 1., 1., p );

    /* Top side */
    glNormal3f( 0.0, 1.0, 0.0 );
    cubeVertex( box_domain_min[0], box_domain_max[1], box_domain_min[2], 0., 1., 0., p );
    cubeVertex( box_domain_min[0], box_domain_max[1], box_domain_max[2], 0., 1., 1., p );
    cubeVertex( box_domain_max[0], box_domain_max[1], box_domain_max[2], 1., 1., 1., p );
    cubeVertex( box_domain_max[0], box_domain_max[1], box_domain_min[2], 1., 1., 0., p );

    /* Bottom side */
    glNormal3f( 0.0, -1.0, 0.0 );
    cubeVertex( box_domain_min[0], box_domain_min[1], box_domain_min[2], 0., 0., 0., p );
    cubeVertex( box_domain_max[0], box_domain_min[1], box_domain_min[2], 1., 0., 0., p );
    cubeVertex( box_domain_max[0], box_domain_min[1], box_domain_max[2], 1., 0., 1., p );
    cubeVertex( box_domain_min[0], box_domain_min[1], box_domain_max[2], 0., 0., 1., p );

    /* Left side */
    glNormal3f( -1.0, 0.0, 0.0 );
    cubeVertex( box_domain_min[0], box_domain_min[1], box_domain_min[2], 0., 0., 0., p );
    cubeVertex( box_domain_min[0], box_domain_min[1], box_domain_max[2], 0., 0., 1., p );
    cubeVertex( box_domain_min[0], box_domain_max[1], box_domain_max[2], 0., 1., 1., p );
    cubeVertex( box_domain_min[0], box_domain_max[1], box_domain_min[2], 0., 1., 0., p );

    /* Right side */
    glNormal3f( 1.0, 0.0, 0.0 );
    cubeVertex( box_domain_max[0], box_domain_min[1], box_domain_min[2], 1., 0., 0., p );
    cubeVertex( box_domain_max[0], box_domain_max[1], box_domain_min[2], 1., 1., 0., p );
    cubeVertex( box_domain_max[0], box_domain_max[1], box_domain_max[2], 1., 1., 1., p );
    cubeVertex( box_domain_max[0], box_domain_min[1], box_domain_max[2], 1., 0., 1., p );
    glEnd();

}

// Sets a uniform texture parameter
void set_tex_param( const char* par, GLuint tex, const CGprogram &program )
{
    CGparameter param = cgGetNamedParameter( program, par );
    cgGLSetTextureParameter( param, tex );
    cgGLEnableTextureParameter( param );
}

void FgeImplicitSurfaceGlyphs::raycasting_pass()
{
//    glLineWidth(4);
//    glBegin(GL_LINES);
//    glColor3f(0,0,1);
//    glVertex3f(0,0,0);
//    glVertex3f(150,150,150);
//    glEnd();


    glext.pglFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, final_image, 0 );

#ifdef __GNUC__
#warning [alex] deactivated option ....... depth
#endif
//    if( use_cubemap )
        glClear( GL_DEPTH_BUFFER_BIT );
//    else
//        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    cgGLEnableProfile( vertexProfile );
    cgGLEnableProfile( fragmentProfile );
    cgGLBindProgram( vertex_main );
    cgGLBindProgram( fragment_main );


    float zero0 = 0.f;
    float zero1 = 0.f;
    set_tex_param( "hsv_colors", hsv_colors, fragment_main );
    set_tex_param( "permutations_x_tex", permutations_x_tex, fragment_main );
    set_tex_param( "permutations_y_tex", permutations_y_tex, fragment_main );
    set_tex_param( "permutations_z_tex", permutations_z_tex, fragment_main );
    set_tex_param( "vectors_xyz_tex", vectors_xyz_tex, fragment_main );
    tex->bind();
    set_tex_param( "mydata", tex->getId(), fragment_main );
// cgGLSetupSampler( cgGetNamedParameter( fragment_main, "hsv_colors" ), //ALEXS_OLD_CG_HSV_COMMENT.
//                    hsv_colors );                                      //ALEXS_OLD_CG_HSV_COMMENT.

    // set current camera here
    GLFrustum<GLfloat> frustum;
    frustum.setFromGL();

    /* Set the eye position as the starting point for all cast rays: */
    GLFrustum<float>::HVector eye = frustum.getEye();
    //std::cout << "Eye is at: " << eye << std::endl;
    //if(fabs(eye[3]) >1e-9) eye*= 1./eye[3];
    //else eye *= 1000.; // FIXME !!! we need something better for orthographic projection
    //std::cout << "Eye is h : " << eye << std::endl;
    cgGLSetParameter4f( cgGetNamedParameter( fragment_main, "origin" ), eye[0],
                    eye[1], eye[2], eye[3] );

    //cgGLSetParameter3f( cgGetNamedParameter( fragment_main, "origin") , camera.lookFrom[0], camera.lookFrom[1], camera.lookFrom[2]);
    cgGLSetParameter3f( cgGetNamedParameter( fragment_main, "camera_u" ), 0.f,
                    0.f, 0.f );
    cgGLSetParameter3f( cgGetNamedParameter( fragment_main, "camera_v" ), 0.f,
                    0.f, 0.f );
    cgGLSetParameter3f( cgGetNamedParameter( fragment_main, "camera_w" ), 0.f,
                    0.f, 0.f );

    cgGLSetParameter2f( cgGetNamedParameter( fragment_main, "half_pixel_uv" ),
                    half_pixel_u, half_pixel_v );
    cgGLSetParameter1f( cgGetNamedParameter( fragment_main, "nan" ), (zero0
                    / zero1) );
    cgGLSetParameter1f( cgGetNamedParameter( fragment_main, "inf" ), (1.f
                    / zero1) );
    cgGLSetParameter1f( cgGetNamedParameter( fragment_main, "max_depth" ),
                    (float)depth );
    cgGLSetParameter1f( cgGetNamedParameter( fragment_main, "epsilon" ),
                    epsilon );
    cgGLSetParameter1f( cgGetNamedParameter( fragment_main, "delta" ), delta );
    cgGLSetParameter1f(
                    cgGetNamedParameter( fragment_main, "shadow_eps_factor" ),
                    shadow_eps_factor );
    //cgGLSetParameter1f( cgGetNamedParameter( fragment_main, "dp_eps_factor") , dp_eps_factor);
    cgGLSetParameter1f( cgGetNamedParameter( fragment_main, "dp_alpha" ),
                    dp_alpha );
    //cgGLSetParameter1f( cgGetNamedParameter( fragment_main, "w") , var_w);
    cgGLSetParameter1f( cgGetNamedParameter( fragment_main, "r0" ), var_r0 );
    cgGLSetParameter1f( cgGetNamedParameter( fragment_main, "r1" ), var_r1 );
    cgGLSetParameter1f( cgGetNamedParameter( fragment_main, "r2" ), var_r2 );


    cgGLSetParameter4f( cgGetNamedParameter( fragment_main, "isovalue" ), iso0,
                    iso1, iso2, iso3 );
    cgGLSetParameter1f( cgGetNamedParameter( fragment_main, "iso_incr" ),
                    iso_incr );
    cgGLSetParameter1f(
                    cgGetNamedParameter( fragment_main, "bumpmap_frequency" ),
                    bumpmapFrequency );
    cgGLSetParameter1f( cgGetNamedParameter( fragment_main, "phong_exponent" ),
                    phong_exponent );
    cgGLSetParameter3f( cgGetNamedParameter( fragment_main, "domain_min" ),
                    domain_min[0], domain_min[1], domain_min[2] );
    cgGLSetParameter3f( cgGetNamedParameter( fragment_main, "domain_max" ),
                    domain_max[0], domain_max[1], domain_max[2] );
//    cgGLSetParameter3f( cgGetNamedParameter( fragment_main, "light_pos" ),
//                    light[0][0], light[0][1], light[0][2] );
    cgGLSetParameter3f( cgGetNamedParameter( fragment_main, "light_pos" ),
                    eye[0], eye[1], eye[2] );
    cgGLSetParameter3f( cgGetNamedParameter( fragment_main, "front_color" ),
                    front_color[0], front_color[1], front_color[2] );
    cgGLSetParameter3f( cgGetNamedParameter( fragment_main, "back_color" ),
                    back_color[0], back_color[1], back_color[2] );
    cgGLSetParameter4f( cgGetNamedParameter( fragment_main, "domain_color" ),
                    domain_color[0], domain_color[1], domain_color[2],
                    domain_color[3] );
    //cgGLSetupSampler( cgGetNamedParameter( fragment_main, "cube_map" ), //ALEXS_OLD_CG_CMAP_COMMENT
     //               cube_map );                                         //ALEXS_OLD_CG_CMAP_COMMENT
    cgGLSetParameter1f(
                    cgGetNamedParameter( fragment_main, "iso_color_offset" ),
                    iso_color_offset );
    cgGLSetParameter1f(
                    cgGetNamedParameter( fragment_main, "iso_color_scale" ),
                    iso_color_scale );
//
//    Vector3 domain_dia = domain_min - domain_max;//for debugging output only
//    std::cout << "==================================" << std::endl;
//    std::cout << "== origin "<< eye << std::endl;
//    std::cout << "== origin "<< eye[0]/eye[3] << std::endl;
//    std::cout << "== origin "<< eye[1]/eye[3] << std::endl;
//    std::cout << "== origin "<< eye[2]/eye[3] << std::endl;
//    std::cout << "== r0     "<< var_r0 << std::endl;
//    std::cout << "== r1     "<< var_r1 << std::endl;
//    std::cout << "== r2     "<< var_r2 << std::endl;
//    std::cout << "== domain_min " << domain_min[0] << " " << domain_min[1] << " " << domain_min[2] <<std::endl;
//    std::cout << "== domain_max " << domain_max[0] << " " << domain_max[1] << " " << domain_max[2] <<std::endl;
//    std::cout << "== domain_dia " << domain_dia[0] << " " << domain_dia[1] << " " << domain_dia[2] <<std::endl;
//    std::cout << "==================================" << std::endl << std::endl << std::endl;


    //needed for Kruger and Westermann technique
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    int i=0;
    std::vector<std::vector<double> >::iterator vertIt = vertices.begin();
    while( vertIt != vertices.end() )
    {
//        std::cout << "render cube" << i << " at " << (*vertIt)[0] << " " << (*vertIt)[1] << " " << (*vertIt)[2] << "   scale: "<<boxScale<<std::endl;
//        std::cout << "eyeorigin "<< eye[0] - (*vertIt)[0] << " "
//        << eye[1] - (*vertIt)[1] << " "
//        << eye[2] - (*vertIt)[2] << " " << std::endl;
//        std::cout<< "center(max,min): " << (domain_min + domain_max)*.5 << std::endl;
        drawBox( (*vertIt)[0], (*vertIt)[1], (*vertIt)[2], boxScale, i);
//        std::cout << "GlyphPosition: " << (*vertIt)[0] << " " << (*vertIt)[1] << " " << (*vertIt)[2] << std::endl << std::endl << std::endl << std::endl;
        ++vertIt;
        ++i;
    }

    glDisable( GL_CULL_FACE );

    cgGLDisableProfile( vertexProfile );
    cgGLDisableProfile( fragmentProfile );

}

void FgeImplicitSurfaceGlyphs::render()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    glEnable( GL_CULL_FACE );
    raycasting_pass();
    glDisable( GL_CULL_FACE );

    glPopAttrib();
}

void FgeImplicitSurfaceGlyphs::setScaling( float scale_ )
{
    boxScale = scale_;
}

void FgeImplicitSurfaceGlyphs::setUnifiedScaling( bool ulv )
{
    unifiedScaling = ulv;
}

void FgeImplicitSurfaceGlyphs::setRenderMode( unsigned int i )
{
    if( i == renderMode )
        return;
    if( i >= getNbRenderModes() )
        return;

    renderMode = i;


    std::cout << "mode = " << RENDERMODES[renderMode] << std::endl;
}

unsigned int FgeImplicitSurfaceGlyphs::getNbRenderModes()
{
    return 4;
}

std::string FgeImplicitSurfaceGlyphs::getRenderModeName( unsigned int i )
{
    if( i < getNbRenderModes() )
    {
        return RENDERMODES[i];
    }
    else
        return RENDERMODES[getNbRenderModes()];
}

char const * const FgeImplicitSurfaceGlyphs::RENDERMODES[] = {
                "HigherOrderTensors", "DUMMY" };

void FgeImplicitSurfaceGlyphs::setFunction( const std::string &f, float iso,
                                            const std::string &dx,
                                            const std::string &dy,
                                            const std::string &dz )
{
    functionString = f;
    std::cout << " painting isosurface for 0 == " << functionString
                    << std::endl;
    isovalue = iso;
    if( dx != "" )
    {
        centralDifferences = 0;
        dfdxString = dx;
        dfdyString = dy;
        dfdzString = dz;
    }
    else
    {
        centralDifferences = 1;
    }
}

std::string FgeImplicitSurfaceGlyphs::lookupString( int nb ) const
{
    static char const table[] = { 'x', 'y', 'z', 'w' };

    std::ostringstream oss;
    oss << "\n// data storage"
        "\n";

    for( int i = 0; i < nb; ++i )
        oss << "\n float A" << i << ";";

    oss << "\nvoid lookupData(float w)"
        "\n{";

    for( int i = 0; i < ceil( (float)nb / 4.f ); ++i )
    {
        oss << "\n  float4 a" << i << " = tex2D( mydata, float2( "
                        << (float)0.5f / ceil( (float)nb / 4.f ) + (float)i
                                        / (ceil( (float)nb / 4. )) << ", "
                        << "w ));";
    }
    oss << "\n";
    for( int i = 0; i < nb; ++i )
        oss << "\n  A" << i << " = a" << i / 4 << "." << table[i % 4] << "*2.-1.;";
    oss << "\n}";
    return oss.str();
}

