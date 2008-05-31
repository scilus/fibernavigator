#include "theDataset.h"

#include <wx/listimpl.cpp>

WX_DEFINE_LIST(DatasetList);

TheDataset::TheDataset()
{
	m_rows = 1;
	m_columns = 1;
	m_frames = 1;
	m_lastError = wxT("");
}

TheDataset::~TheDataset()
{
	
}

DatasetInfo* TheDataset::load(wxString filename)
{
	DatasetInfo *info = new DatasetInfo();
	bool flag = info->load(filename); 
	if (!flag)
	{
		m_lastError = wxT("couldn't load header file");
		return NULL;
	}
	
	if ((m_rows + m_columns + m_frames) == 3)
	{
		if ( info->getRows() <= 0 || info->getColumns() <= 0 || info->getFrames() <= 0 )
		{
			m_lastError = wxT("couldn't parse header file");
			return NULL;
		}
	}
	else
	{
		if ( info->getRows() != m_rows || info->getColumns() != m_columns || info->getFrames() != m_frames )
		{
			m_lastError = wxT("dimensions of loaded files must be the same");
			return NULL;
		}
	}
	
	if (flag)
	{
		flag = false;
		wxFile dataFile;
		if (dataFile.Open(filename.BeforeLast('.')+ wxT(".ima")))
		{
			wxFileOffset nSize = dataFile.Length();
			if (nSize == wxInvalidOffset) return NULL;
			
			switch (info->getType())
			{
			case Head_byte: {
				info->m_byteDataset = new wxUint8[nSize];
				if (dataFile.Read(info->m_byteDataset, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] info->m_byteDataset;
					return NULL;
				}
				flag = true;
			} break;

			case Head_short: {
				info->m_shortDataset = new wxUint16[nSize/2];
					if (dataFile.Read(info->m_shortDataset, (size_t) nSize) != nSize)
					{
						dataFile.Close();
						delete[] info->m_shortDataset;
						return NULL;
					}
					flag = true;
					wxUint16 max = 0;
					for ( int i = 0 ; i < sizeof(info->m_shortDataset) ; ++i)
					{
						max = wxMax(max, info->m_shortDataset[i]);
					}
					printf("max: %d\n", max);
				} break;
			
			case Overlay: {
				info->m_floatDataset = new float[nSize/4];
				if (dataFile.Read(info->m_floatDataset, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] info->m_floatDataset;
					return NULL;
				}
				flag = true;
			} break;
			
			case RGB: {
				wxUint8 *buffer = new wxUint8[nSize];
				info->m_rgbDataset = new wxUint8[nSize];
				if (dataFile.Read(buffer, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] buffer;
					return NULL;
				}
				flag = true;

				int offset = info->getColumns() * info->getRows() ;
				int startslize = 0;
				
				for (int i = 0 ; i < info->getFrames() ; ++i)
				{
					startslize = i * offset * 3;
					for (int j = 0 ; j < offset ; ++j)
					{
						info->m_rgbDataset[startslize + 3*j] = buffer[startslize + j];
						info->m_rgbDataset[startslize + 3*j + 1] = buffer[startslize + offset + j];
						info->m_rgbDataset[startslize + 3*j + 2] = buffer[startslize + 2*offset + j];
					}
				}
			} break;
			
			case ERROR:
			default:
				m_lastError = wxT("unsupported data file format");
				return NULL;
			}
		}
		dataFile.Close();
	}
	
	if (flag)
	{
		m_rows = info->getRows();
		m_columns = info->getColumns();
		m_frames = info->getFrames();
		return info;
	}
	return NULL;
}

