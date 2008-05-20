#ifndef THEDATASET_H_
#define THEDATASET_H_

#include "wx/wx.h"
#include "wx/laywin.h"
#include "wx/textfile.h"
#include "wx/file.h"
#include "wx/image.h"
#include "wx/wfstream.h"
#include "wx/datstrm.h"

class TheDataset
{
private:
	int m_length;
	int m_bands;
	int m_frames;
	int m_rows;
	int m_columns;
	wxString m_repn;
	double m_xVoxel;
	double m_yVoxel;
	double m_zVoxel;
	bool is_loaded;
	float *m_data;
	
	float m_highest_value;
	
public:
	TheDataset();
	~TheDataset(){};
	
	bool load(wxString filename);
	wxString getInfoString();
	
	/* getter methods*/
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
	float* getData() {return m_data;};
};

#endif /*THEDATASET_H_*/
