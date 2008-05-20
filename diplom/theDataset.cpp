#include "theDataset.h"


TheDataset::TheDataset()
{
	is_loaded = false;
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
	int mode = -1;
	if (m_repn.Cmp(wxT("ubyte")) == 0) mode = 1;
	if (m_repn.Cmp(wxT("short")) == 0) mode = 2;
	if (m_repn.Cmp(wxT("float")) == 0) mode = 3;
	if (flag)
	{
		flag = false;
		wxFile dataFile;
		if (dataFile.Open(filename.BeforeLast('.')+ wxT(".ima")))
		{
			wxFileOffset nSize = dataFile.Length();
			if (nSize == wxInvalidOffset) return false;
			m_data = new float[nSize];

			//wxMessageBox(wxString::Format(wxT("%d"), sizeof(buffer)),  wxT(""), wxOK|wxICON_INFORMATION, NULL);
			switch (mode)
			{
			case 1: {
				wxUint8 *buffer = new wxUint8[nSize];
				if (dataFile.Read(buffer, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] buffer;
					return false;
				}
				else flag = true;
				for (int i = 0 ; i < nSize ; ++i)
				{
					m_data[i] = (float)buffer[i];
				}
				m_highest_value = 255.0; 
			} break;
			case 2: {
				wxUint16 *buffer = new wxUint16[nSize];
				if (dataFile.Read(buffer, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] buffer;
					return false;
				}
				else flag = true;
				for (int i = 0 ; i < nSize ; ++i)
				{
					m_data[i] = (float)buffer[i];
				}
				m_highest_value = 65536.0;
			} break;
			case 3: {
				if (dataFile.Read(m_data, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] m_data;
					return false;
				}
				else flag = true;
				m_highest_value = 1.0;
			} break;
			}
		}
		dataFile.Close();
	}
	is_loaded = flag;
	return flag;
}

wxString TheDataset::getInfoString()
{
	if (!is_loaded) return wxT("");
	wxString infoString1, infoString2;
	infoString1.Empty();
	infoString2.Empty();
	infoString1 = wxString::Format(wxT("Info:\nLength: %d\nBands: %d\nFrames: %d\nRows: %d\nColumns: %d\nRepn: "), 
			this->m_length, this->m_bands, this->m_frames, this->m_rows, this->m_columns) + this->m_repn;
	infoString2 = wxString::Format(wxT("\nx Voxel: %.2f\ny Voxel: %.2f\nz Voxel: %.2f"), this->m_xVoxel, this->m_yVoxel, this->m_zVoxel);
	return infoString1 + infoString2;
}
