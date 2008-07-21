#ifndef DATASETINFO_H_
#define DATASETINFO_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/treectrl.h"

#include <GL/glew.h>


enum DatasetType {
	ERROR = -1,
	not_initialized = 0,
	Head_byte,
	Head_short,
	Overlay,
	RGB,
	Mesh_,
	Curves_,
	Surface_,
};

class DatasetInfo
{
public:
	DatasetInfo() {};
	virtual ~DatasetInfo() {};

	virtual bool load(wxString filename)=0;
	virtual void draw()=0;
	virtual void generateTexture()=0;
	virtual void generateGeometry()=0;
	virtual void initializeBuffer()=0;

	wxTreeItemId getTreeId() const {return m_treeId;};
    void setTreeId(wxTreeItemId treeId) {m_treeId = treeId;m_hasTreeId = true;};
	wxString getName() {return m_name;};
	wxString getPath() {return m_fullPath;};
	void setName(wxString name) {m_name = name;};
	int getType() {return m_type;};
	void setType(int type) {m_type = type;};
	float getHighestValue() {return m_highest_value;};
	void setHighestValue(float value) {m_highest_value = value;};
	float getThreshold() {return m_threshold;};
	void setThreshold(float value) {m_threshold = value;};

	int getLength() {return m_length;};
	int getBands() {return m_bands;};
	int getFrames() {return m_frames;};
	int getRows() {return m_rows;};
	int getColumns() {return m_columns;};

	wxString getRpn() {return m_repn;};
	double getXVoxel() {return m_xVoxel;};
	double getYVoxel() {return m_yVoxel;};
	double getZVoxel() {return m_zVoxel;};

	bool toggleShow() {m_show = !m_show; return m_show;};
	bool toggleShowFS() {m_showFS = !m_showFS; return m_showFS;};
	bool toggleUseTex() {m_useTex = !m_useTex; return m_useTex;};
	bool getShow() {return m_show;};
	bool getShowFS() {return m_showFS;};
	bool getUseTex() {return m_useTex;};
	bool hasTreeId() {return m_hasTreeId;};

	void setDisplayList(GLuint value) {m_displayList = value;};
	GLuint getDisplayList() {return m_displayList;};

protected:
	int m_length;
	int m_bands;
	int m_frames;
	int m_rows;
	int m_columns;
	int m_type;
	wxString m_repn;
	double m_xVoxel;
	double m_yVoxel;
	double m_zVoxel;
	bool is_loaded;
	float m_highest_value;
	wxString m_name;
	wxString m_fullPath;
	float m_threshold;
	wxTreeItemId m_treeId;
	GLuint m_displayList;

	bool m_hasTreeId;
	bool m_show;
	bool m_showFS;	// show front sector for meshs
	bool m_useTex; 	// color mesh with textures loaded,
					// if false use colormap on threshold value
	GLuint *m_bufferObjects;
};

#endif /*DATASETINFO_H_*/
