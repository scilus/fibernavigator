#include "fibers.h"

#include <iostream>
#include <fstream>

Fibers::Fibers(DatasetHelper* dh)
{
	m_dh = dh;
	m_type = not_initialized;
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
	m_threshold = 0.10f;
	m_alpha = 1.0;
	m_show = true;
	m_showFS = true;
	m_useTex = true;
	m_bufferObjects = new GLuint[3];
	m_hasTreeId = false;

	m_pointArray = NULL;
	m_lineArray = NULL;
	m_colorArray = NULL;
	m_normalArray = NULL;
}

Fibers::~Fibers()
{
	delete[] m_linePointers;
	delete[] m_pointArray;
	delete[] m_lineArray;
	delete[] m_reverse;
	delete m_kdTree;
	glDeleteBuffers(3, m_bufferObjects);
	m_dh->fibers_loaded = false;
}

bool Fibers::load(wxString filename)
{
	m_dh->printDebug(_T("start loading vtk file"), 1);
	wxFile dataFile;
	wxFileOffset nSize = 0;

	if (dataFile.Open(filename))
	{
		nSize = dataFile.Length();
		if (nSize == wxInvalidOffset) return false;
	}

	wxUint8* buffer = new wxUint8[255];
	dataFile.Read(buffer, (size_t) 255);


	char* temp = new char[256];
	int i = 0;
	int j = 0;
	while (buffer[i] != '\n') {
		++i;
	}
	++i;
	while (buffer[i] != '\n') {
		++i;
	}
	++i;
	while (buffer[i] != '\n') {
		temp[j] = buffer[i];
		++i;
		++j;
	}
	++i;
	temp[j] = 0;
	wxString type(temp, wxConvUTF8);
	if (type == wxT("ASCII")) {
		//ASCII file, maybe later
		return false;
	}

	if (type != wxT("BINARY")) {
		//somethingn else, don't know what to do
		return false;
	}

	j = 0;
	while (buffer[i] != '\n') {
		++i;
	}
	++i;
	while (buffer[i] != '\n') {
		temp[j] = buffer[i];
		++i;
		++j;
	}
	++i;
	temp[j] = 0;
	wxString points(temp, wxConvUTF8);
	points = points.AfterFirst(' ');
	points = points.BeforeFirst(' ');
	long tempValue;
	if(!points.ToLong(&tempValue, 10)) return false; //can't read point count
	int countPoints = (int)tempValue;

	// start position of the point array in the file
	int pc = i;

	i += (12 * countPoints) +1;
	j = 0;
	dataFile.Seek(i);
	dataFile.Read(buffer, (size_t) 255);
	while (buffer[j] != '\n') {
		temp[j] = buffer[j];
		++i;
		++j;
	}
	++i;
	temp[j] = 0;

	wxString sLines(temp, wxConvUTF8);
	wxString sLengthLines = sLines.AfterLast(' ');
	if(!sLengthLines.ToLong(&tempValue, 10)) return false; //can't read size of lines array
	int lengthLines = (int(tempValue));
	sLines = sLines.AfterFirst(' ');
	sLines = sLines.BeforeFirst(' ');
	if(!sLines.ToLong(&tempValue, 10)) return false; //can't read lines
	int countLines = (int)tempValue;
	// start postion of the line array in the file
	int lc = i;

	i += (lengthLines*4) +1;
	dataFile.Seek(i);
	dataFile.Read(buffer, (size_t) 255);
	j = 0;
	int k = 0;
	// TODO test if there's really a color array;
	while (buffer[k] != '\n') {
		++i;
		++k;
	}
	++k;
	++i;
	while (buffer[k] != '\n') {
		temp[j] = buffer[k];
		++i;
		++j;
		++k;
	}
	++i;
	temp[j] = 0;

	//int cc = i;

	m_dh->printDebug(wxString::Format(_T("loading %d points and %d lines."), countPoints, countLines), 1);

	m_countLines = countLines;
	m_dh->countFibers = m_countLines;
	m_countPoints = countPoints;
	m_linePointers = new int[countLines+1];
	m_linePointers[countLines] = countPoints;
	m_reverse = new int[countPoints];
	m_inBox.resize(countLines, sizeof(bool));
	for (int i = 0; i < countLines ; ++i)
	{
		m_inBox[i] = 0;
	}

	m_pointArray = new float[countPoints*3];
	m_lineArray = new int[lengthLines*4];
	m_lengthPoints = countPoints*3;
	m_lengthLines = lengthLines;

	dataFile.Seek(pc);
	dataFile.Read(m_pointArray, (size_t) countPoints*12);
	dataFile.Seek(lc);
	dataFile.Read(m_lineArray, (size_t) lengthLines*4);
	/*
	 * we don't use the color info saved here but calculate our own
	 *
	dataFile.Seek(cc);
	dataFile.Read(curves->m_colorArray, (size_t) countPoints*3);
	*/

	toggleEndianess();

	m_dh->printDebug(_T("move vertices"), 1);

	for (int i = 0; i < countPoints * 3 ; ++i) {
		m_pointArray[i] = m_dh->columns - m_pointArray[i];
		i += 2;
		m_pointArray[i] = m_dh->frames - m_pointArray[i];
	}

	calculateLinePointers();
	createColorArray();
	m_dh->printDebug(_T("read all"), 1);

	m_type = Fibers_;
	m_fullPath = filename;
#ifdef __WXMSW__
	m_name = filename.AfterLast('\\');
#else
	m_name = filename.AfterLast('/');
#endif
	initializeBuffer();

	m_kdTree = new KdTree(m_countPoints, m_pointArray, m_dh);

	return true;
}


int Fibers::getPointsPerLine(int line)
{
	return (m_linePointers[line+1] - m_linePointers[line]) ;
}

int Fibers::getStartIndexForLine(int line)
{
	return m_linePointers[line];
}


void Fibers::calculateLinePointers()
{
	m_dh->printDebug(_T("calculate line pointers"), 1);
	int pc = 0;
	int lc = 0;
	int tc = 0;
	for (int i = 0 ; i < m_countLines ; ++i)
	{
		m_linePointers[i] = tc;
		lc = m_lineArray[pc];
		tc += lc;
		pc += (lc + 1);
	}

	lc = 0;
	pc = 0;


	for ( int i = 0 ; i < m_countPoints ; ++i)
	{
		if ( i == m_linePointers[lc+1]) ++lc;
		m_reverse[i] = lc;
	}
}

int Fibers::getLineForPoint(int point)
{
	return m_reverse[point];
}

void Fibers::toggleEndianess()
{
	m_dh->printDebug(_T("toggle Endianess"), 1);;

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

void Fibers::createColorArray()
{
	m_dh->printDebug(_T("create color arrays"), 1);

	if (m_colorArray) delete[] m_colorArray;
	if (m_normalArray) delete[] m_normalArray;

	m_colorArray = new float[m_countPoints*3];
	m_normalArray = new float[m_countPoints*3];

	int pc = 0;
	float r,g,b, rr, gg, bb;
	float x1,x2,y1,y2,z1,z2;
	float lastx, lasty, lastz;
	for ( int i = 0 ; i < getLineCount() ; ++i )
	{
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

        lastx = m_pointArray[pc]   + (m_pointArray[pc]   - m_pointArray[pc+3]);
		lasty = m_pointArray[pc+1] + (m_pointArray[pc+1] - m_pointArray[pc+4]);
		lastz = m_pointArray[pc+2] + (m_pointArray[pc+2] - m_pointArray[pc+5]);

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

void Fibers::resetColorArray()
{
	m_dh->printDebug(_T("reset color arrays"), 1);

	float *colorData;
	if (m_dh->useVBO)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[1]);
		colorData = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	}
	else
	{
		colorData = m_colorArray;
	}

	int pc = 0;
	float r,g,b;
	float x1,x2,y1,y2,z1,z2;

	for ( int i = 0 ; i < getLineCount() ; ++i )
	{
		x1 = m_pointArray[pc];
		y1 = m_pointArray[pc+1];
		z1 = m_pointArray[pc+2];
		x2 = m_pointArray[pc + getPointsPerLine(i)*3 - 3];
		y2 = m_pointArray[pc + getPointsPerLine(i)*3 - 2];
		z2 = m_pointArray[pc + getPointsPerLine(i)*3 - 1];

		r = (x1) - (x2);
		g = (y1) - (y2);
		b = (z1) - (z2);

		float norm = sqrt(r*r + g*g + b*b);
		r *= 1.0/norm;
        g *= 1.0/norm;
        b *= 1.0/norm;

		for (int j = 0; j < getPointsPerLine(i) ; ++j )
		{
	        colorData[pc] = r;
	        colorData[pc+1] = g;
	        colorData[pc+2] = b;
            pc += 3;
		}
	}

	if (m_dh->useVBO)
	{
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
}


void Fibers::resetLinesShown()
{
	for (int i = 0; i < m_countLines ; ++i)
	{
		m_inBox[i] = 0;
	}
}

void Fibers::updateLinesShown(std::vector<std::vector<SelectionBox*> > boxes)
{
	for (unsigned int i = 0 ; i < boxes.size() ; ++i)
	{
		bool dirty = false;
		for (unsigned int j = 0 ; j < boxes[i].size() ; ++j)
		{
			dirty |= boxes[i][j]->isDirty();
		}
		if (dirty)
		{
			boxes[i][0]->m_inBox = getLinesShown(boxes[i][0]);
			boxes[i][0]->notDirty();

			for (unsigned int j = 1 ; j < boxes[i].size() ; ++j)
			{
				if  (boxes[i][j]->isDirty()) {
					boxes[i][j]->m_inBox = getLinesShown(boxes[i][j]);
					boxes[i][j]->notDirty();
				}
				if ( boxes[i][j]->m_isActive) {
					for (int k = 0 ; k <m_countLines ; ++k)
					boxes[i][0]->m_inBox[k] = boxes[i][0]->m_inBox[k] & ( (boxes[i][j]->m_inBox[k] | boxes[i][j]->m_isNOT) &
																			!(boxes[i][j]->m_inBox[k] & boxes[i][j]->m_isNOT));
				}
			}
		}

		if (boxes[i].size() > 0 && boxes[i][0]->colorChanged())
		{
			float *colorData;
			if (m_dh->useVBO)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[1]);
				colorData = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
			}
			else
			{
				colorData = m_colorArray;
			}
			wxColour col = boxes[i][0]->getColor();

			for ( int l = 0 ; l < m_countLines ; ++l )
			{
				if (boxes[i][0]->m_inBox[l])
				{
					unsigned int pc = getStartIndexForLine(l)*3;

					for (int j = 0; j < getPointsPerLine(l) ; ++j )
					{
						colorData[pc] = ((float)col.Red())/255.0;
						colorData[pc+1] = ((float)col.Green())/255.0;
						colorData[pc+2] = ((float)col.Blue())/255.0;
						pc += 3;
					}
				}
			}
			if (m_dh->useVBO)
			{
				glUnmapBuffer(GL_ARRAY_BUFFER);
			}
			boxes[i][0]->setColorChanged(false);
		}
	}
	resetLinesShown();
	for (unsigned int i = 0 ; i < boxes.size() ; ++i)
	{
		if ( boxes[i].size() > 0 && boxes[i][0]->m_isActive) {
			for (int k = 0 ; k <m_countLines ; ++k)
				m_inBox[k] = m_inBox[k] | boxes[i][0]->m_inBox[k];
		}
	}
	if (m_dh->fibersInverted)
	{
		for (int k = 0 ; k <m_countLines ; ++k)
		{
			m_inBox[k] = !m_inBox[k];
		}
	}
}

std::vector<bool> Fibers::getLinesShown(SelectionBox* box)
{
	Vector vpos = box->getCenter();
	Vector vsize = box->getSize();
	resetLinesShown();
	m_boxMin = new float[3];
	m_boxMax = new float[3];
	m_boxMin[0] = vpos.x - vsize.x/2;
	m_boxMax[0] = vpos.x + vsize.x/2;
	m_boxMin[1] = vpos.y - vsize.y/2;
	m_boxMax[1] = vpos.y + vsize.y/2;
	m_boxMin[2] = vpos.z - vsize.z/2;
	m_boxMax[2] = vpos.z + vsize.z/2;

	boxTest(0, m_countPoints-1, 0);
	return m_inBox;
}

void Fibers::boxTest(int left, int right, int axis)
{
	// abort condition
	if (left > right) return;

	int root = left + ((right-left)/2);
	int axis1 = (axis+1) % 3;
	int pointIndex = m_kdTree->m_tree[root]*3;

	if (m_pointArray[pointIndex + axis] < m_boxMin[axis]) {
		boxTest(root +1, right, axis1);
	}
	else if (m_pointArray[pointIndex + axis] > m_boxMax[axis]) {
		boxTest(left, root-1, axis1);
	}
	else {
		int axis2 = (axis+2) % 3;
		if (	m_pointArray[pointIndex + axis1] <= m_boxMax[axis1] &&
				m_pointArray[pointIndex + axis1] >= m_boxMin[axis1] &&
				m_pointArray[pointIndex + axis2] <= m_boxMax[axis2] &&
				m_pointArray[pointIndex + axis2] >= m_boxMin[axis2] )
		{
			m_inBox[getLineForPoint(m_kdTree->m_tree[root])] = 1;
		}
		boxTest(left, root -1, axis1);
		boxTest(root+1, right, axis1);
	}
}

void Fibers::initializeBuffer()
{
	if (!m_dh->useVBO) return;
	bool isOK = true;
	glGenBuffers(3, m_bufferObjects);
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*m_countPoints*3, m_pointArray, GL_STATIC_DRAW );
	if (m_dh->GLError())
	{
		m_dh->printGLError(wxT("initialize vbo points"));
		isOK = false;
	}
	if (isOK)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*m_countPoints*3, m_colorArray, GL_STATIC_DRAW );
		if (m_dh->GLError())
		{
			m_dh->printGLError(wxT("initialize vbo colors"));
			isOK = false;
		}
	}
	if (isOK)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*m_countPoints*3, m_normalArray, GL_STATIC_DRAW );
		if (m_dh->GLError())
		{
			m_dh->printGLError(wxT("initialize vbo normals"));
			isOK = false;
		}
	}
	m_dh->useVBO = isOK;
	if (isOK)
	{
		freeArrays();
	}
	else
	{
		m_dh->printDebug(_T("***ERROR***: Not enough memory on your gfx card. Using vertex arrays."), 2);
		glDeleteBuffers(3, m_bufferObjects);
	}
}

void Fibers::draw()
{
	if (m_dh->useFakeTubes)
	{
		drawFakeTubes();
		return;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	if (!m_dh->useVBO)
	{
	    glVertexPointer(3, GL_FLOAT, 0, m_pointArray);
	    if (m_showFS)
	    	glColorPointer (3, GL_FLOAT, 0, m_colorArray); // global colors
	    else
	    	glColorPointer (3, GL_FLOAT, 0, m_normalArray); // local colors
	    glNormalPointer (GL_FLOAT, 0, m_normalArray);
	}
	else
	{
	    glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[0]);
	    glVertexPointer(3, GL_FLOAT, 0, 0);
	    if (m_showFS) {
			glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[1]);
			glColorPointer (3, GL_FLOAT, 0, 0);
	    }
	    else {
	    	glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[2]);
	    	glColorPointer (3, GL_FLOAT, 0, 0);
	    }

	    glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[2]);
	    glNormalPointer (GL_FLOAT, 0, 0);
	}

	for ( int i = 0 ; i < m_countLines ; ++i )
	{
		if (m_inBox[i] == 1)
			glDrawArrays(GL_LINE_STRIP, getStartIndexForLine(i), getPointsPerLine(i));
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

bool Fibers::getBarycenter(SplinePoint* point)
{
	// number of fibers needed to keep a point
	int threshold = 20;
	// multiplier for moving the point towards the barycenter

	m_boxMin = new float[3];
	m_boxMax = new float[3];
	m_boxMin[0] = point->X() - 25.0/2;
	m_boxMax[0] = point->X() + 25.0/2;
	m_boxMin[1] = point->Y() - 5.0/2;
	m_boxMax[1] = point->Y() + 5.0/2;
	m_boxMin[2] = point->Z() - 5.0/2;
	m_boxMax[2] = point->Z() + 5.0/2;
	m_barycenter.x = 0;
	m_barycenter.y = 0;
	m_barycenter.z = 0;
	m_count = 0;

	barycenterTest(0, m_countPoints-1, 0);
	if (m_count > threshold) {
		m_barycenter.x /= m_count;
		m_barycenter.y /= m_count;
		m_barycenter.z /= m_count;

		float x1 = ( m_barycenter.x - point->X() );
		float y1 = ( m_barycenter.y - point->Y() );
		float z1 = ( m_barycenter.z - point->Z() );

		Vector v (x1, y1, z1 );
		point->setOffsetVector(v);

		point->setX(point->X() + x1);
		point->setY(point->Y() + y1);
		point->setZ(point->Z() + z1);
		return true;
	}
	else {
		return false;

	}

}

void Fibers::barycenterTest(int left, int right, int axis)
{
	// abort condition
	if (left > right) return;

	int root = left + ((right-left)/2);
	int axis1 = (axis+1) % 3;
	int pointIndex = m_kdTree->m_tree[root]*3;

	if (m_pointArray[pointIndex + axis] < m_boxMin[axis]) {
		barycenterTest(root +1, right, axis1);
	}
	else if (m_pointArray[pointIndex + axis] > m_boxMax[axis]) {
		barycenterTest(left, root-1, axis1);
	}
	else {
		int axis2 = (axis+2) % 3;
		if (	m_inBox[getLineForPoint(m_kdTree->m_tree[root])] == 1 &&
				m_pointArray[pointIndex + axis1] <= m_boxMax[axis1] &&
				m_pointArray[pointIndex + axis1] >= m_boxMin[axis1] &&
				m_pointArray[pointIndex + axis2] <= m_boxMax[axis2] &&
				m_pointArray[pointIndex + axis2] >= m_boxMin[axis2] )
		{
			m_barycenter[0] += m_pointArray[m_kdTree->m_tree[root]*3];
			m_barycenter[1] += m_pointArray[m_kdTree->m_tree[root]*3+1];
			m_barycenter[2] += m_pointArray[m_kdTree->m_tree[root]*3+2];
			m_count++;
		}
		barycenterTest(left, root -1, axis1);
		barycenterTest(root+1, right, axis1);
	}
}

void Fibers::save(wxString filename)
{
	if (filename.AfterLast('.') != _T("fib"))
		filename += _T(".fib");
	std::vector<float>pointsToSave;
	std::vector<int>linesToSave;
	std::vector<float>colorsToSave;
	int pointIndex = 0;
	int countLines = 0;

	float redVal = 0.5f;
	float greenVal = 0.5;
	float blueVal = 0.5f;

	for ( int l = 0 ; l < m_countLines ; ++l )
	{
		if (m_inBox[l])
		{
			unsigned int pc = getStartIndexForLine(l)*3;

			linesToSave.push_back(getPointsPerLine(l));

			for (int j = 0; j < getPointsPerLine(l) ; ++j )
			{
				pointsToSave.push_back(m_dh->columns - m_pointArray[pc]);
				++pc;
				pointsToSave.push_back(m_pointArray[pc]);
				++pc;
				pointsToSave.push_back(m_dh->columns - m_pointArray[pc]);
				++pc;

				linesToSave.push_back(pointIndex);
				++pointIndex;

				colorsToSave.push_back(redVal);
				colorsToSave.push_back(greenVal);
				colorsToSave.push_back(blueVal);
			}
			++countLines;
		}
	}

	converterByteINT32 c;
	converterByteFloat f;

	std::ofstream myfile;
	std::vector<char>vBuffer;

	std::string header1 = "# vtk DataFile Version 3.0\nvtk output\nBINARY\nDATASET POLYDATA\nPOINTS ";
	header1 += intToString(pointsToSave.size()/3);
	header1 +=  " float\n";

	for (unsigned int i = 0 ; i < header1.size() ; ++i)
	{
		vBuffer.push_back(header1[i]);
	}

	for (unsigned int i = 0 ; i < pointsToSave.size() ; ++i)
	{
		f.f = pointsToSave[i];
		vBuffer.push_back(f.b[3]);
		vBuffer.push_back(f.b[2]);
		vBuffer.push_back(f.b[1]);
		vBuffer.push_back(f.b[0]);
	}

	vBuffer.push_back('\n');
	std::string header2 = "LINES " + intToString(countLines) + " " + intToString(linesToSave.size()) + "\n";

	for (unsigned int i = 0 ; i < header2.size() ; ++i)
	{
		vBuffer.push_back(header2[i]);
	}

	for (unsigned int i = 0 ; i < linesToSave.size() ; ++i)
	{
		c.i = linesToSave[i];
		vBuffer.push_back(c.b[3]);
		vBuffer.push_back(c.b[2]);
		vBuffer.push_back(c.b[1]);
		vBuffer.push_back(c.b[0]);
	}

	vBuffer.push_back('\n');

	std::string header3 = "CELL_DATA 0\n";
	header3 += "COLOR_SCALARS scalars 3\n";

	for (unsigned int i = 0 ; i < header3.size() ; ++i)
	{
		vBuffer.push_back(header3[i]);
	}

	for (unsigned int i = 0 ; i < colorsToSave.size() ; ++i)
	{
		f.f = colorsToSave[i];
		vBuffer.push_back(f.b[0]);
		vBuffer.push_back(f.b[1]);
		vBuffer.push_back(f.b[2]);
		vBuffer.push_back(f.b[3]);
	}

	vBuffer.push_back('\n');

	// finally put the buffer vector into a char* array
	char * buffer;
	buffer = new char [vBuffer.size()];

	for (unsigned int i = 0 ; i < vBuffer.size() ; ++i)
	{
		buffer[i] = vBuffer[i];
	}

	char* fn;
	fn = (char*)malloc(filename.length());
	strcpy(fn, (const char*)filename.mb_str(wxConvUTF8));

	myfile.open ( fn, std::ios::binary);
	myfile.write(buffer, vBuffer.size());

	myfile.close();
}

std::string Fibers::intToString(int number)
{
	std::stringstream out;
	out << number;
	return out.str();
}

void Fibers::drawFakeTubes()
{
	float *colors;
	float *normals;
	if (m_dh->useVBO)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[1]);
		colors = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[2]);
		normals = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	else
	{
		colors = m_colorArray;
		normals = m_normalArray;
	}

	if (m_dh->getPointMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for ( int i = 0 ; i < m_countLines ; ++i )
	{
		if (m_inBox[i] == 1)
		{
			glBegin(GL_QUAD_STRIP);
			int idx = getStartIndexForLine(i)*3;
			for (int k = 0 ; k < getPointsPerLine(i) ; ++k)
			{
				glNormal3f( normals[idx], normals[idx+1], normals[idx+2] );
				//glColor3f ( normals[idx], normals[idx+1], normals[idx+2] );
				//glNormal3f( colors[idx], colors[idx+1], colors[idx+2] );
				glColor3f ( colors[idx], colors[idx+1], colors[idx+2]);
				glMultiTexCoord2f(GL_TEXTURE0, -1.0f, 0.0f);
				glVertex3f (m_pointArray[idx], m_pointArray[idx+1], m_pointArray[idx+2]);
				glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f);
				glVertex3f (m_pointArray[idx], m_pointArray[idx+1], m_pointArray[idx+2]);
				idx += 3;
				//
			}
			glEnd();
		}
	}
}

void Fibers::switchNormals(bool positive)
{
	float *normals;
	if (m_dh->useVBO)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferObjects[2]);
		normals = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	}
	else
	{
		normals = m_normalArray;
	}

	if (positive)
	{
		int pc = 0;
		float rr, gg, bb;
		float lastx, lasty, lastz;
		for ( int i = 0 ; i < getLineCount() ; ++i )
		{
			lastx = m_pointArray[pc]   + (m_pointArray[pc]   - m_pointArray[pc+3]);
			lasty = m_pointArray[pc+1] + (m_pointArray[pc+1] - m_pointArray[pc+4]);
			lastz = m_pointArray[pc+2] + (m_pointArray[pc+2] - m_pointArray[pc+5]);

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

				normals[pc] = rr;
				normals[pc+1] = gg;
				normals[pc+2] = bb;

				pc += 3;
			}
		}
	}
	else
	{
		int pc = 0;
		float rr, gg, bb;
		float lastx, lasty, lastz;
		for ( int i = 0 ; i < getLineCount() ; ++i )
		{
			lastx = m_pointArray[pc]   + (m_pointArray[pc]   - m_pointArray[pc+3]);
			lasty = m_pointArray[pc+1] + (m_pointArray[pc+1] - m_pointArray[pc+4]);
			lastz = m_pointArray[pc+2] + (m_pointArray[pc+2] - m_pointArray[pc+5]);

			for (int j = 0; j < getPointsPerLine(i) ; ++j )
			{
				rr = lastx - m_pointArray[pc];
				gg = lasty - m_pointArray[pc+1];
				bb = lastz - m_pointArray[pc+2];
				lastx = m_pointArray[pc];
				lasty = m_pointArray[pc+1];
				lastz = m_pointArray[pc+2];

				normals[pc] = rr;
				normals[pc+1] = gg;
				normals[pc+2] = bb;

				pc += 3;
			}
		}
	}
	if (m_dh->useVBO)
	{
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
}
