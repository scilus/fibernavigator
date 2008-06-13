#include "curves.h"

Curves::Curves(int lines)
{
	m_lineCount = lines;
	m_pointsPerLine = new int[lines];
	m_pointsPerLineCalculated = false;
}

Curves::~Curves()
{
	delete[] m_pointsPerLine;
	delete[] m_pointArray;
	delete[] m_lineArray;
}

int Curves::getPointsPerLine(int line) 
{
	if (!m_pointsPerLineCalculated) calculatePointsPerLine();
	return m_pointsPerLine[line];
}

void Curves::calculatePointsPerLine()
{
	int pc = 0;
	int lc = 0;
	for (int i = 0 ; i < m_lineCount ; ++i)
	{
		lc = m_lineArray[pc];
		m_pointsPerLine[i] = lc;
		pc += (lc + 1);
	}
	m_pointsPerLineCalculated = true;
}

void Curves::toggleEndianess()
{
	wxUint8 *pointbytes = (wxUint8*)m_pointArray;
	wxUint8 temp;
	for ( int i = 0 ; i < m_lengthPoints*4; i +=4)
	{
		temp  = pointbytes[i];
		pointbytes[i] = pointbytes[i+3]; 
		pointbytes[i+3] = temp;
		temp  = pointbytes[i+1];
		pointbytes[i+1] = pointbytes[i+2]; 
		pointbytes[i+2] = temp;
	}

	wxUint8 *linebytes = (wxUint8*)m_lineArray;
	for ( int i = 0 ; i < m_lengthLines*4; i +=4)
	{
		temp  = linebytes[i];
		linebytes[i] = linebytes[i+3]; 
		linebytes[i+3] = temp;
		temp  = linebytes[i+1];
		linebytes[i+1] = linebytes[i+2]; 
		linebytes[i+2] = temp;
	}
}
