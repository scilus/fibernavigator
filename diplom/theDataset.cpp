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
				this->m_length = (int)lTmpValue;
			}
			if (sLabel.Contains(wxT("nbands:"))) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				this->m_bands = (int)lTmpValue;
			}
			if (sLabel.Contains(wxT("nframes:"))) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				this->m_frames = (int)lTmpValue;
			}
			if (sLabel.Contains(wxT("nrows:"))) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				this->m_rows = (int)lTmpValue;
			}
			if (sLabel.Contains(wxT("ncolumns:"))) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				this->m_columns = (int)lTmpValue;
			}
			if (sLabel.Contains(wxT("repn:"))) 
			{
				this->m_repn = sValue;
			}
			if (sLabel.Contains(wxT("voxel:"))) 
			{
				wxString sNumber;
				sValue = sLine.AfterLast(':');
				sValue = sValue.BeforeLast('\"');
				sNumber = sValue.AfterLast(' ');
				flag = sNumber.ToDouble(&this->m_zVoxel); 
				sValue = sValue.BeforeLast(' ');
				sNumber = sValue.AfterLast(' ');
				flag = sNumber.ToDouble(&this->m_yVoxel);
				sValue = sValue.BeforeLast(' ');
				sNumber = sValue.AfterLast('\"');
				flag = sNumber.ToDouble(&this->m_xVoxel);
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
			m_data = new wxUint8[nSize];
			if (dataFile.Read(m_data, (size_t) nSize) != nSize)
			{
				delete[] m_data;
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
	return this->m_length;
}
int TheDataset::getBands()
{
	return this->m_bands;
}
int TheDataset::getFrames()
{
	return this->m_frames;
}
int TheDataset::getRows()
{
	return this->m_rows;
}
int TheDataset::getColumns()
{
	return this->m_columns;
}
wxString TheDataset::getRpn()
{
	return this->m_repn;
}
double TheDataset::getXVoxel()
{
	return this->m_xVoxel;
}
double TheDataset::getYVoxel()
{
	return this->m_yVoxel;
}
double TheDataset::getZVoxel()
{
	return this->m_zVoxel;
}

wxUint8* TheDataset::getData()
{
	return this->m_data;
}

wxString TheDataset::getInfoString()
{
	if (m_data == NULL) return wxT("");
	wxString infoString1, infoString2;
	infoString1.Empty();
	infoString2.Empty();
	infoString1 = wxString::Format(wxT("Info:\nLength: %d\nBands: %d\nFrames: %d\nRows: %d\nColumns: %d\nRepn: "), 
			this->m_length, this->m_bands, this->m_frames, this->m_rows, this->m_columns) + this->m_repn;
	infoString2 = wxString::Format(wxT("\nx Voxel: %.2f\ny Voxel: %.2f\nz Voxel: %.2f"), this->m_xVoxel, this->m_yVoxel, this->m_zVoxel);
	return infoString1 + infoString2;
}
