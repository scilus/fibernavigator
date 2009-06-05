#ifndef DATASETINFO_H_
#define DATASETINFO_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/treectrl.h"
#include "DatasetHelper.h"

#include "../misc/IsoSurface/triangleMesh.h"

#include <GL/glew.h>


enum DatasetType {
	TERROR = -1,
	not_initialized = 0,
	// Textures
	Head_byte,
	Head_short,
	Overlay,
	RGB,
	// everything higher can't be used as a texture
	Mesh_,
	Vectors_,
	Tensors_,
	Fibers_,
	Surface_,
	IsoSurface_,
	MasterBox,
	ChildBox,
	Point_,
	Label_points,
	Label_selBoxes,
};

union converterByteINT32 {
	wxUint8 b[4];
	wxUint32 i;
};

union converterByteFloat {
	wxUint8 b[4];
	float f;
};

class DatasetHelper;
class TriangleMesh;

class DatasetInfo
{
public:
	DatasetInfo(DatasetHelper* dh) :
	m_dh(dh),

    m_length(0),
    m_bands(0),
    m_frames(0),
    m_rows(0),
    m_columns(0),
    m_type(not_initialized),
    m_repn(_T("")),
    is_loaded(false),
    m_highest_value(1.0),

    m_name(_T("")),
    m_fullPath(_T("")),

    m_threshold(0.00f),
    m_alpha(1.0f),
    m_oldMax(1.0),
    m_newMax(1.0),

    m_color( wxColour(255, 255, 0)),
    m_GLuint(0),

    m_show(true),
    m_showFS(true),
    m_useTex(true),

    m_isGlyph(false),

    licCalculated(false),
    m_useLIC(false),
    m_bufferObjects(0)
	{};
	virtual ~DatasetInfo() {};

	virtual bool load(wxString filename)=0;
	virtual bool save(wxString filename)const{ return false;}
	virtual void draw()=0;
	virtual void clean()=0;
	virtual void smooth()=0;
	virtual void activateLIC()=0;
	virtual GLuint getGLuint()=0;


	wxString getName() {return m_name;};
	wxString getPath() {return m_fullPath;};
	void setName(wxString name) {m_name = name;};
	int getType() {return m_type;};
	void setType(int type) {m_type = type;};
	float getHighestValue() {return m_highest_value;};
	void setHighestValue(float value) {m_highest_value = value;};
	float getThreshold() {return m_threshold;};
	float getOldMax() {return m_oldMax;};
	void setOldMax(float v) {m_oldMax = v;};
	float getNewMax() {return m_newMax;};
	void setThreshold(float value) {m_threshold = value;};
	float getAlpha() {return m_alpha;};
	void setAlpha(float v) {m_alpha = v;};

	int getLength() {return m_length;};
	int getBands() {return m_bands;};
	int getFrames() {return m_frames;};
	int getRows() {return m_rows;};
	int getColumns() {return m_columns;};

	wxString getRpn() {return m_repn;};

	bool toggleShow() {m_show = !m_show; return m_show;};
	bool toggleShowFS() {m_showFS = !m_showFS; return m_showFS;};
	bool toggleUseTex() {m_useTex = !m_useTex; return m_useTex;};
	bool toggleUseLIC() {m_useLIC = !m_useLIC; return m_useLIC;};

	void setShow(bool v) {m_show = v;};
	void setShowFS(bool v) {m_showFS = v;};
	void setuseTex(bool v) {m_useTex = v;};
	bool getShow() {return m_show;};
	bool getShowFS() {return m_showFS;};
	bool getUseTex() {return m_useTex;};
	bool getUseLIC() {return m_useLIC;};
	bool getIsGlyph() {return m_isGlyph;};

	void setGLuint(GLuint value) {m_GLuint = value;};
	void setColor(wxColour color) {m_color = color;};
	wxColour getColor() {return m_color;};

	// Trianglemesh
	TriangleMesh* m_tMesh;

protected:
	virtual void generateTexture()=0;
	virtual void generateGeometry()=0;
	virtual void initializeBuffer()=0;

	DatasetHelper* m_dh;

	int m_length;
	int m_bands;
	int m_frames;
	int m_rows;
	int m_columns;
	int m_type;
	wxString m_repn;
	bool is_loaded;
	float m_highest_value;
	wxString m_name;
	wxString m_fullPath;
	float m_threshold;
	float m_alpha;
	float m_oldMax;
	float m_newMax;

	wxColour m_color;
	GLuint m_GLuint;

	bool m_show;
	bool m_showFS;	// show front sector for meshs
	bool m_useTex; 	// color mesh with textures loaded,
					// if false use colormap on threshold value
	bool m_isGlyph;
	bool licCalculated;
	bool m_useLIC;
	GLuint *m_bufferObjects;
};

#endif /*DATASETINFO_H_*/
