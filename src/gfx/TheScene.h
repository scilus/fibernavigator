/////////////////////////////////////////////////////////////////////////////
// Name:            TheScene.h
// Author:          ---
// Creation Date:   ---
//
// Description: theScene class.
//
// Last modifications:
//      by : Imagicien - 12/11/2009
/////////////////////////////////////////////////////////////////////////////

#ifndef THESCENE_H_
#define THESCENE_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "../gui/ArcBall.h"
#include "../dataset/DatasetInfo.h"
#include "../dataset/DatasetHelper.h"
#include "wx/glcanvas.h"

//////////////////////////////////////////////////////////////////////////////////
// COMMENT
//////////////////////////////////////////////////////////////////////////////////
enum 
{
    AXIAL = 1,
    CORONAL,
    SAGITTAL,
    MAIN_VIEW
};

class DatasetHelper;
class FgeImageSpaceLIC;

class TheScene
{
public:
    // Constructor/Destructor
    TheScene( DatasetHelper* );
    ~TheScene();

    // Functions
    void         bindTextures    ();
    void         drawSphere      ( float, float, float, float );
    wxGLContext* getMainGLContext()                                 { return m_mainGLContext; };
    void         initGL          ( int );
    void         renderScene     ();
    void         setMainGLContext( wxGLContext* i_context )         { m_mainGLContext = i_context; };
	bool         m_isRotateZ;
    bool         m_isRotateY;
    bool         m_isRotateX;
    bool         m_isNavSagital;
    bool         m_isNavAxial;
    bool         m_isNavCoronal;
    float        m_rotAngleX;
    float        m_rotAngleY;
    float        m_rotAngleZ;
    float        m_posSagital;
    float        m_posCoronal;
    float        m_posAxial;


private:
    // Functions
    void extractFrustum         ();
    void drawColorMapLegend     ();
    void drawGraph              ();
    void drawPoints             ();
    void drawSelectionObjects   ();
    void drawVectors            ();
    void lightsOff              ();
    void lightsOn               ();
    void renderFakeTubes        ();
    void renderFibers           ();
    void renderMesh             ();
    void renderODFs             ();
    void renderSlizes           ();
    void renderSplineSurface    ();
    void renderTensors          ();

    // Variables
    DatasetHelper*  m_datasetHelper;
    wxGLContext*    m_mainGLContext;
    float           m_modelview[16];  // Variable for the frustum calculation.
    float           m_projection[16]; // Variable for the frustum calculation.
};

#endif /*THESCENE_H_*/
