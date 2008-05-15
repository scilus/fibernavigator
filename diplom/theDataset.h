#ifndef THEDATASET_H_
#define THEDATASET_H_

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
	
	
public:
	TheDataset();
	~TheDataset(){};
	
	bool load(wxString filename);
	
	/* getter methods*/
	int getLength();
	int getBands();
	int getFrames();
	int getRows();
	int getColumns();
	wxString getRpn();
	double getXVoxel();
	double getYVoxel();
	double getZVoxel();
	
	wxString getInfoString();
	float* getData();
	
};

#endif /*THEDATASET_H_*/
