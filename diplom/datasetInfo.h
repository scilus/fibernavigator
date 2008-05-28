#ifndef DATASETINFO_H_
#define DATASETINFO_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/laywin.h"
#include "wx/textfile.h"
#include "wx/file.h"
#include "wx/wfstream.h"
#include "wx/datstrm.h"

enum DatasetType {
	ERROR = -1,
	not_initialized = 0,
	Head_byte,
	Head_short,
	Overlay,
	RGB,		
};

class DatasetInfo
{
private:
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

public:	
	DatasetInfo();
	~DatasetInfo(){};
	
	bool load(wxString filename);
	
	wxString getInfoString();
	
	void setHighestValue(float value) {m_highest_value = value;};
	
	int getType() {return m_type;};
	int getLength() {return m_length;};
	int getBands() {return m_bands;};
	int getFrames() {return m_frames;};
	int getRows() {return m_rows;};
	int getColumns() {return m_columns;};
	wxString getRpn() {return m_repn;};
	double getXVoxel() {return m_xVoxel;};
	double getYVoxel() {return m_yVoxel;};;
	double getZVoxel() {return m_zVoxel;};;
	float getHighestValue() {return m_highest_value;};	
};
#endif /*DATASETINFO_H_*/
