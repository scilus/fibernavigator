#include "wx/wx.h"
#include "wx/laywin.h"
#include "wx/textfile.h"
#include "wx/file.h"
#include "wx/image.h"

#include "theDataset.h"


TheDataset::TheDataset()
{
	
}

bool TheDataset::load(wxString filename)
{
	// check file extension
	wxString ext = filename.substr(filename.Length()-3,3);
	if (ext != wxT("hea")) return false;
	// read header file
	wxTextFile headerFile;
	bool flag = false;
	if (headerFile.Open(filename))
	{
		size_t i;
		wxString sLine;
		wxString sValue;
		wxString sLabel;
		long lTmpValue;
		for (i = 3 ; i < headerFile.GetLineCount() ; ++i)
		{
			sLine = headerFile.GetLine(i);
			sLabel = sLine.BeforeLast(' ');
			sValue = sLine.AfterLast(' ');
			if (sLabel.Contains(wxT("length:"))) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				this->length = (int)lTmpValue;
			}
			if (sLabel.Contains(wxT("nbands:"))) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				this->bands = (int)lTmpValue;
			}
			if (sLabel.Contains(wxT("nframes:"))) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				this->frames = (int)lTmpValue;
			}
			if (sLabel.Contains(wxT("nrows:"))) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				this->rows = (int)lTmpValue;
			}
			if (sLabel.Contains(wxT("ncolumns:"))) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				this->columns = (int)lTmpValue;
			}
			if (sLabel.Contains(wxT("repn:"))) 
			{
				this->repn = sValue;
			}
			if (sLabel.Contains(wxT("voxel:"))) 
			{
				wxString sNumber;
				sValue = sLine.AfterLast(':');
				sValue = sValue.BeforeLast('\"');
				sNumber = sValue.AfterLast(' ');
				flag = sNumber.ToDouble(&this->zVoxel); 
				sValue = sValue.BeforeLast(' ');
				sNumber = sValue.AfterLast(' ');
				flag = sNumber.ToDouble(&this->yVoxel);
				sValue = sValue.BeforeLast(' ');
				sNumber = sValue.AfterLast('\"');
				flag = sNumber.ToDouble(&this->xVoxel);
			}
		}
	}
	headerFile.Close();
	if (flag)
	{
		flag = false;
		wxFile dataFile;
		if (dataFile.Open(filename.BeforeLast('.')+ wxT(".ima")))
		{
			wxFileOffset nSize = dataFile.Length();
			if (nSize == wxInvalidOffset) return false;
			data = new wxUint8[nSize];
			if (dataFile.Read(data, (size_t) nSize) != nSize)
			{
				delete[] data;
				return false;
			}
			else flag = true;
		}
		
	}
	return flag;
}

/* getter methods */
int TheDataset::getLength()
{
	return this->length;
}
int TheDataset::getBands()
{
	return this->bands;
}
int TheDataset::getFrames()
{
	return this->frames;
}
int TheDataset::getRows()
{
	return this->rows;
}
int TheDataset::getColumns()
{
	return this->columns;
}
wxString TheDataset::getRpn()
{
	return this->repn;
}
double TheDataset::getXVoxel()
{
	return this->xVoxel;
}
double TheDataset::getYVoxel()
{
	return this->yVoxel;
}
double TheDataset::getZVoxel()
{
	return this->zVoxel;
}

wxUint8* TheDataset::getData()
{
	return this->data;
}

wxString TheDataset::getInfoString()
{
	wxString infoString1, infoString2;
	infoString1.Empty();
	infoString2.Empty();
	infoString1 = wxString::Format(wxT("Info:\nLength: %d\nBands: %d\nFrames: %d\nRows: %d\nColumns: %d\nRepn: "), 
			this->length, this->bands, this->frames, this->rows, this->columns) + this->repn;
	infoString2 = wxString::Format(wxT("\nx Voxel: %.2f\ny Voxel: %.2f\nz Voxel: %.2f"), this->xVoxel, this->yVoxel, this->zVoxel);
	return infoString1 + infoString2;
}
