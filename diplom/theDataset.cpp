#include "theDataset.h"

TheDataset::TheDataset()
{
	m_headLoaded = false;
	m_overlayLoaded = false;
	m_rgbLoaded = false;
	
	m_rows = 0;
	m_columns = 0;
	m_frames = 0;
	
	m_headInfo = new DatasetInfo();
	m_overlayInfo = new DatasetInfo();
	m_rgbInfo = new DatasetInfo();
}

bool TheDataset::load(wxString filename)
{
	DatasetInfo *info = new DatasetInfo();
	bool flag = info->load(filename);
	
	if (m_rows == 0)
	{
		if ( info->getRows() <= 0 || info->getColumns() <= 0 || info->getFrames() <= 0 )
		{
			printf("ERROR while trying to load data file\nheader file corrupt!\n");
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
			printf("ERROR while trying to load data file\ndimensions of loaded files must be the same!\n");
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
				wxUint8 *buffer = new wxUint8[nSize];
				m_dataHead = new float[2*nSize];
				if (dataFile.Read(buffer, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] buffer;
					return false;
				}
				else flag = true;
				for (int i = 0 ; i < nSize ; ++i)
				{
					m_dataHead[2*i] = (float)buffer[i]/255.0;
					m_dataHead[2*i + 1] = m_dataHead[2*i];
				}
				m_headLoaded = flag;
				m_headInfo = info;
				m_headInfo->setHighestValue(255.0);
			} break;

			case Head_short: {
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
			} break;
			
			case Overlay: {
				m_dataOverlay = new float[nSize/4];
				if (dataFile.Read(m_dataOverlay, (size_t) nSize) != nSize)
				{
					dataFile.Close();
					delete[] m_dataOverlay;
					return false;
				}
				else flag = true;
				m_overlayLoaded = flag;
				m_overlayInfo = info;
				m_overlayInfo->setHighestValue(1.0);
			} break;
			
			case RGB: {
				wxUint8 *buffer = new wxUint8[nSize];
				m_dataRGB = new float[(nSize/3)*4];
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
						m_dataRGB[startslize + 3*j] = (float)buffer[startslize + j]/255.0;
						m_dataRGB[startslize + 3*j + 1] = (float)buffer[startslize + offset + j]/255.0;
						m_dataRGB[startslize + 3*j + 2] = (float)buffer[startslize + 2*offset + j]/255.0;
					}
				}
				m_rgbLoaded = flag;
				m_rgbInfo = info;
				m_rgbInfo->setHighestValue(255.0); 
			} break;
			
			case ERROR:
			default:
				printf("ERROR while trying to load data file\ndata file corrupt!\n");
				return false;
			}
		}
		dataFile.Close();
	}
	return flag;
}
