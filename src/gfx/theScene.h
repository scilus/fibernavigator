#ifndef THESCENE_H_
#define THESCENE_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "../gui/ArcBall.h"
#include "../dataset/datasetInfo.h"
#include "../dataset/DatasetHelper.h"
#include <vector>

//#include "lic/FgeImageSpaceLIC.h"

#ifdef CG_GLYPHS
#include "ImplicitSurfaceGlyphs/FgeImplicitSurfaceGlyphs.h"
#endif //CG_GLYPHS

#include "wx/glcanvas.h"

enum {
	axial = 1,
	coronal,
	sagittal,
	mainView
};

class DatasetHelper;
class FgeImageSpaceLIC;

class TheScene {

public:
	TheScene(DatasetHelper*);
	~TheScene();

	void initGL(int);

	void bindTextures();

	void setMainGLContext(wxGLContext* context) {m_mainGLContext = context;};
	wxGLContext* getMainGLContext() {return m_mainGLContext;};

	void renderScene();

	void drawSphere(float, float, float, float);

private:
    DatasetHelper* m_dh;
	wxGLContext* m_mainGLContext;
#ifdef CG_GLYPHS
	float m_old_xSlice;
    float m_old_ySlice;
    float m_old_zSlice;
    FgeImplicitSurfaceGlyphs* m_myGlyphs;
    bool m_glyphsInitialized;
    unsigned int m_old_nbGlyphPositions;
    std::vector<Vector> m_positions;
#endif //CG_GLYPHS

	void lightsOn();
	void lightsOff();

	void renderSlizes();
	void renderMesh();
	void renderFibers();
	void renderFakeTubes();
	void renderSplineSurface();

	void drawVectors();

#ifdef CG_GLYPHS
	void drawSingleGlyph();
	void drawGlyphsOnSurface();
    void drawGlyphs( std::vector<Vector> glyphPositions );
#endif //CG_GLYPHS

	void drawColorMapLegend();

	void drawSelectionBoxes();
	void drawPoints();
};

#endif /*THESCENE_H_*/
