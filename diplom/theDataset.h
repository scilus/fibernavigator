#ifndef THEDATASET_H_
#define THEDATASET_H_

class TheDataset
{
private:
	int length;
	int bands;
	int frames;
	int rows;
	int columns;
	wxString repn;
	double xVoxel;
	double yVoxel;
	double zVoxel;

	wxUint8 *data;
	
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
	wxImage* getXSlize(int);
	wxImage* getYSlize(int);
	wxImage* getZSlize(int);
	
};

#endif /*THEDATASET_H_*/
