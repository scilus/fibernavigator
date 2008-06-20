#include "curves.h"

Curves::Curves(int lines, int points)
{
	m_lineCount = lines;
	m_pointCount = points;
	m_pointsPerLine = new int[lines*2];
	m_pointsPerLineCalculated = false;
}

Curves::~Curves()
{
	delete[] m_pointsPerLine;
	delete[] m_pointArray;
	delete[] m_lineArray;
	delete[] m_colorArray;
	delete[] m_normalArray;
}

int Curves::getPointsPerLine(int line) 
{
	if (!m_pointsPerLineCalculated) calculatePointsPerLine();
	return m_pointsPerLine[line*2];
}

int Curves::getStartIndexForLine(int line) 
{
	if (!m_pointsPerLineCalculated) calculatePointsPerLine();
	return m_pointsPerLine[line*2+1];
}


void Curves::calculatePointsPerLine()
{
	int pc = 0;
	int lc = 0;
	int tc = 0;
	for (int i = 0 ; i < m_lineCount ; ++i)
	{
		lc = m_lineArray[pc];
		m_pointsPerLine[2*i] = lc;
		m_pointsPerLine[2*i+1] = tc;
		tc += lc;
		pc += (lc + 1);
	}
	m_pointsPerLineCalculated = true;
}

void Curves::toggleEndianess()
{
	printf("toggle Endianess\n");
	
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

void Curves::createColorArray()
{
	printf("create color arrays\n");

	int pc = 0;
    float r,g,b, rr, gg, bb;
    float x1,x2,y1,y2,z1,z2;
    float lastx, lasty, lastz;
    for ( int i = 0 ; i < getLineCount() ; ++i )
    {
    	//pc = getStartIndexForLine(i)*3;
        x1 = m_pointArray[pc];
        y1 = m_pointArray[pc+1];
        z1 = m_pointArray[pc+2];
        x2 = m_pointArray[pc + getPointsPerLine(i)*3 - 3];
        y2 = m_pointArray[pc + getPointsPerLine(i)*3 - 2];
        z2 = m_pointArray[pc + getPointsPerLine(i)*3 - 1];
        
        r = (x1) - (x2);
        g = (y1) - (y2);
        b = (z1) - (z2);
        if (r < 0.0) r *= -1.0 ;
        if (g < 0.0) g *= -1.0 ;
        if (b < 0.0) b *= -1.0 ;
        
        float norm = sqrt(r*r + g*g + b*b);
        r *= 1.0/norm;
        g *= 1.0/norm;
        b *= 1.0/norm;

        lastx = lasty = lastz = 0.0;
        
        for (int j = 0; j < getPointsPerLine(i) ; ++j )
        {
        	rr = lastx - m_pointArray[pc];
            gg = lasty - m_pointArray[pc+1];
            bb = lastz - m_pointArray[pc+2];
            lastx = m_pointArray[pc];
            lasty = m_pointArray[pc+1];
            lastz = m_pointArray[pc+2];
            if (rr < 0.0) rr *= -1.0 ;
            if (gg < 0.0) gg *= -1.0 ;
            if (bb < 0.0) bb *= -1.0 ;
            float norm = sqrt(rr*rr + gg*gg + bb*bb);
            rr *= 1.0/norm;
            gg *= 1.0/norm;
            bb *= 1.0/norm;

        	m_normalArray[pc] = rr;
        	m_normalArray[pc+1] = gg;
        	m_normalArray[pc+2] = bb;
        	
        	m_colorArray[pc] = r;
	        m_colorArray[pc+1] = g;
	        m_colorArray[pc+2] = b;
	        pc += 3;
        }
    }

}

void Curves::buildkDTree()
{
	m_kdTree = new KdTree(m_pointCount, m_pointArray);
}
