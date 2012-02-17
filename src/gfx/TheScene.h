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

class TheScene
{
public:
    // Constructor/Destructor
    TheScene( DatasetHelper* );
    ~TheScene();

    // Functions
    void         bindTextures    ();
    void         drawSphere      ( float xPos, float yPos, float zPos, float ray );
    wxGLContext* getMainGLContext() { return m_pMainGLContext; };
    void         initGL          ( int whichView );
    void         renderScene     ();
    void         setMainGLContext( wxGLContext* pContext ) { m_pMainGLContext = pContext; };
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
	void renderFibers           ();
    void renderMesh             ();
    void renderODFs             ();
    void renderSlices           ();
    void renderSplineSurface    ();
    void renderTensors          ();

    // Variables
    DatasetHelper*  m_pDatasetHelper;
    wxGLContext*    m_pMainGLContext;
    float           m_modelview[16];  // Variable for the frustum calculation.
    float           m_projection[16]; // Variable for the frustum calculation.
};

#endif /*THESCENE_H_*/
