#include "theDataset.h"

#include <wx/listimpl.cpp>

WX_DEFINE_LIST(DatasetList);

TheDataset::TheDataset()
{
	m_rows = 1;
	m_columns = 1;
	m_frames = 1;
	m_dsList = new DatasetList();
	m_lastError = wxT("");
}

bool TheDataset::load(wxString filename)
{
	DatasetInfo *info = new DatasetInfo();
	bool flag = info->load(filename);
	if (!flag)
	{
		printf("ERROR couldn't load header file\n");
		m_lastError = wxT("couldn't load header file");
		return false;
	}
	
	if (m_dsList->size() == 0)
	{
		if ( info->getRows() <= 0 || info->getColumns() <= 0 || info->getFrames() <= 0 )
		{
			printf("ERROR couldn't parse header file\n");
			m_lastError = wxT("couldn't parse header file");
			return false;
		}
		m_rows = info->getRows();
		m_columns = info->getColumns();
		m_frames = info->getFrames();
	}
	else
	{
		if ( info->getRows() != m_rows || info->getColumns() != m_columns || info->getFrames() != m_frames )
		{
			printf("ERROR dimensions of loaded files must be the same!\n");
			m_lastError = wxT("dimensions of loaded files must be the same");
			return false;
		}
	}
	
	if (flag)
	{
		flag = false;
		wxFile dataFile;
		if (dataFile.Open(filename.BeforeLast('.')+ wxT(".ima")))
		{
			wxFileOffset nSize = dataFile.Length();
			if (nSize == wxInvalidOffset) return false;
			
			switch (info->getType())
			{
			case Head_byte: {
				
				info->m_byteDataset = new wxUint8[nSize];
				if (dataFile.Read(info->m_byteDataset, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] info->m_byteDataset;
					return false;
				}
				else flag = true;
				m_dsList->Append(info);
				
				
			} break;

			case Head_short: {
				/*
				m_dataHead = new float[nSize];
				wxFileInputStream input(filename.BeforeLast('.') + wxT(".ima"));
				if (!input.Ok()) return false;
				wxDataInputStream data( input );
				float temp;
				float max = 0.0;
				for (int i = 0 ; i < nSize/2 ; ++ i)
				{
					temp =  data.Read16();
				    m_dataHead[2*i] = temp;
				    m_dataHead[2*i + 1] = temp;
				    max = wxMax(max, (float)temp);
				}
				for (int i = 0 ; i < nSize/2 ; ++ i)
				{
				    m_dataHead[2*i] /= max;
				    m_dataHead[2*i + 1] /= max;
				}
				m_headLoaded = flag;
				m_headInfo = info;
				flag = true;
				*/
			} break;
			
			case Overlay: {
				info->m_floatDataset = new float[nSize/4];
				if (dataFile.Read(info->m_floatDataset, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] info->m_floatDataset;
					return false;
				}
				else flag = true;
				m_dsList->Append(info);
			} break;
			
			case RGB: {
				wxUint8 *buffer = new wxUint8[nSize];
				info->m_rgbDataset = new wxUint8[nSize];
				if (dataFile.Read(buffer, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] buffer;
					return false;
				}
				else flag = true;

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
				m_dsList->Append(info);
			} break;
			
			case ERROR:
			default:
				printf("ERROR unsupported data file format\n");
				m_lastError = wxT("unsupported data file format");
				return false;
			}
		}
		dataFile.Close();
	}
	return flag;
}
