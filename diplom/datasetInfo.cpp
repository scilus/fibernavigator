#include "datasetInfo.h"

DatasetInfo::DatasetInfo()
{
	m_length = 0;
	m_bands = 0;
	m_frames = 0;
	m_rows = 0;
	m_columns = 0;
	m_repn = wxT("");
	m_xVoxel = 0.0;
	m_yVoxel = 0.0;
	m_zVoxel = 0.0;
	is_loaded = false;
	m_highest_value = 1.0;
}

bool DatasetInfo::load(wxString filename)
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
			sLabel.Trim(false);
			sLabel.Trim();
			if (sLabel.Contains(wxT("length:"))) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				this->m_length = (int)lTmpValue;
			}
			if (sLabel == wxT("nbands:")) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				this->m_bands = (int)lTmpValue;
			}
			if (sLabel == wxT("nframes:")) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				this->m_frames = (int)lTmpValue;
			}
			if (sLabel == wxT("nrows:")) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				this->m_rows = (int)lTmpValue;
			}
			if (sLabel == wxT("ncolumns:")) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				this->m_columns = (int)lTmpValue;
			}
			if (sLabel == wxT("repn:"))
			//if (sLabel.Contains(wxT("repn:"))) 
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
	is_loaded = flag;
	return flag;
}

wxString DatasetInfo::getInfoString()
{
	if (!is_loaded) return wxT("not loaded");
	wxString infoString1, infoString2;
	infoString1.Empty();
	infoString2.Empty();
	infoString1 = wxString::Format(wxT("Length: %d\nBands: %d\nFrames: %d\nRows: %d\nColumns: %d\nRepn: "), 
			this->m_length, this->m_bands, this->m_frames, this->m_rows, this->m_columns) + this->m_repn;
	infoString2 = wxString::Format(wxT("\nx Voxel: %.2f\ny Voxel: %.2f\nz Voxel: %.2f"), this->m_xVoxel, this->m_yVoxel, this->m_zVoxel);
	return infoString1 + infoString2;
}

int DatasetInfo::getMode()
{
	if (m_repn.Cmp(wxT("ubyte")) == 0) return 1;
	if (m_repn.Cmp(wxT("short")) == 0) return 2;
	if (m_repn.Cmp(wxT("float")) == 0) return 3;
	return 0;
}

void DatasetInfo::setHighestValue(float value)
{
	m_highest_value = value;
}
