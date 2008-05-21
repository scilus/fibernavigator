#include "theDataset.h"


TheDataset::TheDataset()
{
	is_headLoaded = false;
}

bool TheDataset::loadHead(wxString filename)
{
	m_headInfo = new DatasetInfo();
	bool flag =m_headInfo->load(filename);
	
	if (flag)
	{
		flag = false;
		wxFile dataFile;
		if (dataFile.Open(filename.BeforeLast('.')+ wxT(".ima")))
		{
			wxFileOffset nSize = dataFile.Length();
			if (nSize == wxInvalidOffset) return false;
			m_headData = new float[nSize];

			//wxMessageBox(wxString::Format(wxT("%d"), sizeof(buffer)),  wxT(""), wxOK|wxICON_INFORMATION, NULL);
			switch (m_headInfo->getMode())
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
					m_headData[i] = (float)buffer[i];
				}
				m_headInfo->setHighestValue(255.0); 
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
					m_headData[i] = (float)buffer[i];
				}
				m_headInfo->setHighestValue(65536.0); 
			} break;
			case 3: {
				if (dataFile.Read(m_headData, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] m_headData;
					return false;
				}
				else flag = true;
				m_headInfo->setHighestValue(1.0);
			} break;
			}
		}
		dataFile.Close();
	}
	is_headLoaded = flag;
	return flag;
}

